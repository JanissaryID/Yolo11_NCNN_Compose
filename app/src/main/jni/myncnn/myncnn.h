#ifndef YOLO11_NCNN_COMPOSE_MYNCNN_H
#define YOLO11_NCNN_COMPOSE_MYNCNN_H

#include <opencv2/core/core.hpp>

#include <net.h>

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>

struct KeyPoint
{
    cv::Point2f p;
    float prob;
};

struct Object
{
    cv::Rect_<float> rect;
    cv::RotatedRect rrect;
    int label;
    float prob;
    int gindex;
    cv::Mat mask;
    std::vector<KeyPoint> keypoints;
};

// #################################################################################################



// #################################################################################################

class MYNCNN
{
public:
    int myNumber = 0;

    virtual ~MYNCNN();

    int load(const char* parampath, const char* modelpath);
    int load(AAssetManager* mgr, const char* parampath, const char* modelpath);

    void set_det_target_size(int target_size);
    void set_number(int number);
    int get_number();

    virtual int detect(const cv::Mat& rgb, std::vector<Object>& objects) = 0;
    virtual int draw(cv::Mat& rgb, const std::vector<Object>& objects) = 0;

protected:
    ncnn::Net myncnn;
    int det_target_size;
};

class Detection : public MYNCNN
{
public:
    virtual int detect(const cv::Mat& rgb, std::vector<Object>& objects);
    int getLabel();
    virtual int draw(cv::Mat& rgb, const std::vector<Object>& objects);
};

class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    // Asynchronous connect (callback untuk hasil)
    void asyncConnect(const std::string& ip, int port, std::function<void(bool)> callback);

    // Kirim data asinkron
    void asyncSendData(const std::string& data);

    // Tutup koneksi dan stop semua thread
    void closeConnection();

    bool isConnected() const;

private:
    // Blocking connect internal
    bool connectToServer(const std::string& ip, int port);

    // Blocking send internal
    bool sendData(const std::string& data);

    // Worker thread untuk pengiriman data
    void sendWorker();

private:
    int sockfd;
    std::atomic<bool> connected;

    // Thread worker untuk send async
    std::thread sendThread;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::queue<std::string> sendQueue;
    std::atomic<bool> running;

    std::mutex connectMutex;
    std::thread connectThread;
};

#endif //YOLO11_NCNN_COMPOSE_MYNCNN_H
