#include "../myncnn/myncnn.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <android/log.h>
#include <cerrno>

#define LOG_TAG "TcpClient"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

TcpClient::TcpClient()
        : sockfd(-1), connected(false), running(false)
{}

TcpClient::~TcpClient()
{
    closeConnection();
}

void TcpClient::asyncConnect(const std::string& ip, int port, std::function<void(bool)> callback)
{
    // Jika koneksi lama masih ada, close dulu
    closeConnection();

    running = true;

    connectThread = std::thread([=]() {
        bool success = connectToServer(ip, port);
        if (success) {
            // Mulai worker thread untuk kirim data async
            sendThread = std::thread(&TcpClient::sendWorker, this);
        }
        callback(success);
    });

    connectThread.detach();
}

bool TcpClient::connectToServer(const std::string& ip, int port)
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOGE("Failed to create socket, errno=%d", errno);
        return false;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0) {
        LOGE("Invalid IP address: %s", ip.c_str());
        close(sockfd);
        sockfd = -1;
        return false;
    }

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        LOGE("Connect failed to %s:%d, errno=%d", ip.c_str(), port, errno);
        close(sockfd);
        sockfd = -1;
        return false;
    }

    connected = true;
    LOGI("Connected to %s:%d", ip.c_str(), port);
    return true;
}

void TcpClient::asyncSendData(const std::string& data)
{
    if (!connected) {
        LOGE("Not connected, cannot queue send");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        sendQueue.push(data);
    }
    cv.notify_one();
}

void TcpClient::sendWorker()
{
    while (running) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this]() { return !sendQueue.empty() || !running; });

        while (!sendQueue.empty()) {
            std::string data = sendQueue.front();
            sendQueue.pop();
            lock.unlock();

            bool sent = sendData(data);
            if (!sent) {
                LOGE("Failed to send data, closing connection");
                closeConnection();
                return;
            }

            lock.lock();
        }
    }
}

bool TcpClient::sendData(const std::string& data)
{
    if (!connected) {
        LOGE("Not connected");
        return false;
    }

    size_t totalSent = 0;
    size_t dataLen = data.size();
    const char* dataPtr = data.c_str();

    while (totalSent < dataLen) {
        ssize_t sent = send(sockfd, dataPtr + totalSent, dataLen - totalSent, 0);
        if (sent <= 0) {
            LOGE("Send failed or connection closed, errno=%d", errno);
            connected = false;
            closeConnection();
            return false;
        }
        totalSent += sent;
    }

    LOGI("Sent data: %s", data.c_str());
    return true;
}

void TcpClient::closeConnection()
{
    running = false;

    cv.notify_all();

    if (sendThread.joinable()) {
        sendThread.join();
    }
    if (connectThread.joinable()) {
        connectThread.join();
    }

    if (connected) {
        close(sockfd);
        sockfd = -1;
        connected = false;
        LOGI("Connection closed");
    }
}

bool TcpClient::isConnected() const
{
    return connected;
}

