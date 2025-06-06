#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

#include <platform.h>
#include <benchmark.h>

#include "myncnn/myncnn.h"

#include "camera/ndkcamera.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if __ARM_NEON
#include <arm_neon.h>
#endif // __ARM_NEON

static int draw_unsupported(cv::Mat& rgb)
{
    const char text[] = "unsupported";

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 1.0, 1, &baseLine);

    int y = (rgb.rows - label_size.height) / 2;
    int x = (rgb.cols - label_size.width) / 2;

    cv::rectangle(rgb, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(rgb, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0));

    return 0;
}

static int draw_fps(cv::Mat& rgb)
{
    // resolve moving average
    float avg_fps = 0.f;
    {
        static double t0 = 0.f;
        static float fps_history[10] = {0.f};

        double t1 = ncnn::get_current_time();
        if (t0 == 0.f)
        {
            t0 = t1;
            return 0;
        }

        float fps = 1000.f / (t1 - t0);
        t0 = t1;

        for (int i = 9; i >= 1; i--)
        {
            fps_history[i] = fps_history[i - 1];
        }
        fps_history[0] = fps;

        if (fps_history[9] == 0.f)
        {
            return 0;
        }

        for (int i = 0; i < 10; i++)
        {
            avg_fps += fps_history[i];
        }
        avg_fps /= 10.f;
    }

    char text[32];
    sprintf(text, "FPS=%.2f", avg_fps);

    int baseLine = 0;
    cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

    int y = 0;
    int x = rgb.cols - label_size.width;

    cv::rectangle(rgb, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                  cv::Scalar(255, 255, 255), -1);

    cv::putText(rgb, text, cv::Point(x, y + label_size.height),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

    return 0;
}

static MYNCNN* g_ncnn = 0;
static ncnn::Mutex lock;

class MyNdkCamera : public NdkCameraWindow
{
public:
    virtual void on_image_render(cv::Mat& rgb) const;
};

void MyNdkCamera::on_image_render(cv::Mat& rgb) const
{
    // yolo11
    {
        ncnn::MutexLockGuard g(lock);

        if (g_ncnn)
        {
            std::vector<Object> objects;
            g_ncnn->detect(rgb, objects);

            g_ncnn->draw(rgb, objects);
        }
        else
        {
            draw_unsupported(rgb);
        }
    }

    draw_fps(rgb);
}

static MyNdkCamera* g_camera = 0;

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "JNI_OnLoad");

    g_camera = new MyNdkCamera;

    ncnn::create_gpu_instance();

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "JNI_OnUnload");

    {
        ncnn::MutexLockGuard g(lock);

        delete g_ncnn;
        g_ncnn = 0;
    }

    ncnn::destroy_gpu_instance();

    delete g_camera;
    g_camera = 0;
}

// public native boolean loadModel(AssetManager mgr, int taskid, int modelid, int cpugpu);
JNIEXPORT jboolean JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_loadModel(JNIEnv* env, jobject thiz, jobject assetManager)
{
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "ncnn", "AAssetManager is NULL");
        return JNI_FALSE;
    }

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "loadModel mgr=%p", mgr);

//    const char* parampath = "best_Damage.ncnn.param";
//    const char* modelpath = "best_Damage.ncnn.bin";

//    const char* parampath = "yolo11n.ncnn.param";
//    const char* modelpath = "yolo11n.ncnn.bin";

//    const char* parampath = "yolo11n_pose.ncnn.param";
//    const char* modelpath = "yolo11n_pose.ncnn.bin";

//    const char* parampath = "hand_full-op.param";
//    const char* modelpath = "hand_full-op.bin";

    const char* parampath = "rps.ncnn.param";
    const char* modelpath = "rps.ncnn.bin";

    {
        ncnn::MutexLockGuard g(lock);

        delete g_ncnn;
        g_ncnn = nullptr;

        g_ncnn = new Detection;
        if (!g_ncnn)
        {
            __android_log_print(ANDROID_LOG_ERROR, "ncnn", "Failed to alloc YOLO11_det");
            return JNI_FALSE;
        }

        int ret = g_ncnn->load(mgr, parampath, modelpath);
        if (ret != 0)
        {
            __android_log_print(ANDROID_LOG_ERROR, "ncnn", "g_yolo11->load failed: %d", ret);
            delete g_ncnn;
            g_ncnn = nullptr;
            return JNI_FALSE;
        }

        g_ncnn->set_det_target_size(640);

        __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "Model loaded: %s / %s", parampath, modelpath);
    }

    return JNI_TRUE;
}

// public native boolean openCamera(int facing);
JNIEXPORT jboolean JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_openCamera(JNIEnv* env, jobject thiz, jint facing)
{
    if (facing < 0 || facing > 3)
        return JNI_FALSE;

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "openCamera %d", facing);

    g_camera->open((int)facing);

    return JNI_TRUE;
}

// public native boolean closeCamera();
JNIEXPORT jboolean JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_closeCamera(JNIEnv* env, jobject thiz)
{
    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "closeCamera");

    g_camera->close();

    return JNI_TRUE;
}

// public native boolean setOutputWindow(Surface surface);
JNIEXPORT jboolean JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_setOutputWindow(JNIEnv* env, jobject thiz, jobject surface)
{
    ANativeWindow* win = ANativeWindow_fromSurface(env, surface);

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "setOutputWindow %p", win);

    g_camera->set_window(win);

    return JNI_TRUE;
}

JNIEXPORT jint JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_getNumber(JNIEnv* env, jobject thiz)
{
    return g_ncnn->get_number();
}

extern TcpClient* gTcpClient;

static void onConnectResult(bool success) {
    __android_log_print(ANDROID_LOG_DEBUG, "socket", "Connect result: %d", success);
}

JNIEXPORT jint JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_connect(JNIEnv* env, jobject thiz, jstring ip_, jint port)
{
    const char *ip = env->GetStringUTFChars(ip_, nullptr);

    if (gTcpClient != nullptr) {
        delete gTcpClient;
        gTcpClient = nullptr;
    }

    gTcpClient = new TcpClient();

    gTcpClient->asyncConnect(std::string(ip), port, onConnectResult);

    env->ReleaseStringUTFChars(ip_, ip);

    return 0;
}

JNIEXPORT void JNICALL Java_com_polytron_yolo11ncnncompose_MyNcnn_close(JNIEnv* env, jobject thiz) {
    if (gTcpClient != nullptr) {
        gTcpClient->closeConnection();
        delete gTcpClient;
        gTcpClient = nullptr;
        __android_log_print(ANDROID_LOG_DEBUG, "socket", "Connection closed and client deleted");
    }
}

}