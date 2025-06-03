//
// Created by Krisn on 02/06/2025.
//

#ifndef YOLO11_NCNN_COMPOSE_MYNCNN_H
#define YOLO11_NCNN_COMPOSE_MYNCNN_H

#include <opencv2/core/core.hpp>

#include <net.h>
#include "../hand-landmark/landmark.h"

struct KeyPoint
{
    cv::Point2f p;
    float prob;
};

struct Object_Det
{
    cv::Rect_<float> rect;
    cv::RotatedRect rrect;
    int label;
    float prob;
    int gindex;
    cv::Mat mask;
    std::vector<KeyPoint> keypoints;
};

// Palm

struct Object_Palm
{
    cv::Rect_<float> rect;
    int label;
    float prob;
    cv::Point2f pts[21];

};

struct PalmObject
{
    float  score;
    cv::Rect rect;
    cv::Point2f  landmarks[7];
    float  rotation;

    float  hand_cx;
    float  hand_cy;
    float  hand_w;
    float  hand_h;
    cv::Point2f  hand_pos[4];

    cv::Mat trans_image;
    std::vector<cv::Point2f> skeleton;
};
struct DetectRegion
{
    float score;
    cv::Point2f topleft;
    cv::Point2f btmright;
    cv::Point2f landmarks[7];

    float  rotation;
    cv::Point2f  roi_center;
    cv::Point2f  roi_size;
    cv::Point2f  roi_coord[4];
};
struct Anchor
{
    float x_center, y_center, w, h;
};

struct AnchorsParams
{
    int input_size_width;
    int input_size_height;

    float min_scale;
    float max_scale;

    float anchor_offset_x;
    float anchor_offset_y;

    int num_layers;
    std::vector<int> feature_map_width;
    std::vector<int> feature_map_height;
    std::vector<int>   strides;
    std::vector<float> aspect_ratios;

};

class MYNCNN
{
public:
    virtual ~MYNCNN();

    int load(const char* parampath, const char* modelpath);
    int load(AAssetManager* mgr, const char* parampath, const char* modelpath);
    int load(AAssetManager* mgr, const char* modeltype, const float* mean_vals, const float* norm_vals);

    void set_det_target_size(int target_size);

    virtual int detect(const cv::Mat& rgb, std::vector<Object_Det>& objects) = 0;
    virtual int draw(cv::Mat& rgb, const std::vector<Object_Det>& objects) = 0;

protected:
    ncnn::Net myncnn;
    int det_target_size;

    ncnn::Net blazepalm_net;
    std::vector<Anchor> anchors;
    LandmarkDetect landmark;
private:
    ncnn::UnlockedPoolAllocator blob_pool_allocator;
    ncnn::PoolAllocator workspace_pool_allocator;
    float mean_vals[3];
    float norm_vals[3];
};

class NCNN_det : public MYNCNN
{
public:
    virtual int detect(const cv::Mat& rgb, std::vector<Object_Det>& objects);
    virtual int draw(cv::Mat& rgb, const std::vector<Object_Det>& objects);
};

class NCNN_Hand : public MYNCNN
{
public:
    int detect(const cv::Mat& rgb, std::vector<PalmObject>& objects, float prob_threshold = 0.55f, float nms_threshold = 0.3f);
    int draw(cv::Mat& rgb, const std::vector<PalmObject>& objects);

private:

    int target_size;

};

#endif //YOLO11_NCNN_COMPOSE_MYNCNN_H
