project(yolo11ncnn)

cmake_minimum_required(VERSION 3.10)

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mobile-4.11.0-android/sdk/native/jni)
find_package(OpenCV REQUIRED core imgproc)

set(ncnn_DIR ${CMAKE_SOURCE_DIR}/ncnn-20250428-android-vulkan/${ANDROID_ABI}/lib/cmake/ncnn)
find_package(ncnn REQUIRED)

add_library(myncnn SHARED
        main_ncnn.cpp
        myncnn/myncnn.cpp
        myncnn/type/detection.cpp
        socket/TcpClient.cpp
        camera/ndkcamera.cpp)

target_link_libraries(myncnn ncnn ${OpenCV_LIBS} camera2ndk mediandk)
