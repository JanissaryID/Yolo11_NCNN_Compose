// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2021 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

#include <android/log.h>

#include <jni.h>

#include <string>
#include <vector>

#include <platform.h>
#include <benchmark.h>

#include "../ncnn/myncnn.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#if __ARM_NEON
#include <arm_neon.h>
#endif // __ARM_NEON

static MYNCNN* g_hand = 0;
static ncnn::Mutex lock;

extern "C" {

// public native boolean loadModel(AssetManager mgr, int modelid, int cpugpu);

JNIEXPORT jboolean JNICALL Java_com_polytron_yolo11ncnncompose_Ncnn_loadModelHand(JNIEnv* env, jobject thiz, jobject assetManager)
{
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr)
    {
        __android_log_print(ANDROID_LOG_ERROR, "ncnn", "AAssetManager is NULL");
        return JNI_FALSE;
    }

    __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "loadModel mgr=%p", mgr);

    const char* modeltypes[] =
    {
        "palm-lite",
        "palm-full"
    };

    const float mean_vals[3] = {0.f,0.f,0.f};

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};

    {
        ncnn::MutexLockGuard g(lock);

        delete g_hand;
        g_hand = nullptr;

        g_hand = new NCNN_Hand;
        if (!g_hand)
        {
            __android_log_print(ANDROID_LOG_ERROR, "ncnn", "Failed to alloc YOLO11_det");
            return JNI_FALSE;
        }

        int ret = g_hand->load(mgr, modeltypes[1], mean_vals, norm_vals);
        if (ret != 0)
        {
            __android_log_print(ANDROID_LOG_ERROR, "ncnn", "g_yolo11->load failed: %d", ret);
            delete g_hand;
            g_hand = nullptr;
            return JNI_FALSE;
        }

        g_hand->set_det_target_size(640);

//        __android_log_print(ANDROID_LOG_DEBUG, "ncnn", "Model loaded: %s / %s", parampath, modelpath);
    }

    // reload

    return JNI_TRUE;
}

}
