project(yolo11ncnn)

cmake_minimum_required(VERSION 3.10)

set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/opencv-mobile-4.11.0-android/sdk/native/jni)
find_package(OpenCV REQUIRED core imgproc)

set(ncnn_DIR ${CMAKE_SOURCE_DIR}/ncnn-20250428-android-vulkan/${ANDROID_ABI}/lib/cmake/ncnn)
find_package(ncnn REQUIRED)

#yolo
#add_library(yolo11ncnn SHARED yolo/yolo11ncnn.cpp yolo/yolo11.cpp yolo/yolo11_det.cpp ndkcamera.cpp)
#
#target_link_libraries(yolo11ncnn ncnn ${OpenCV_LIBS} camera2ndk mediandk)

# HandLandmark
#add_library(mediapipehand SHARED hand-landmark/mediapipehand.cpp hand-landmark/hand.cpp hand-landmark/landmark.cpp camera/ndkcamera.cpp)
#
#target_link_libraries(mediapipehand ncnn ${OpenCV_LIBS} camera2ndk mediandk)

#ncnn
add_library(myncnn SHARED ncnn/myncnn.cpp yolo/yolo11ncnn.cpp yolo/yolo11_det.cpp camera/ndkcamera.cpp hand-landmark/landmark.cpp hand-landmark/mediapipehand.cpp)

target_link_libraries(myncnn ncnn ${OpenCV_LIBS} camera2ndk mediandk)
