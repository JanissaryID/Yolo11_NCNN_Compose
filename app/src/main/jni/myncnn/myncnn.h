#ifndef YOLO11_NCNN_COMPOSE_MYNCNN_H
#define YOLO11_NCNN_COMPOSE_MYNCNN_H

#include <opencv2/core/core.hpp>

#include <net.h>

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

struct ObjectHand
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

class Pose : public MYNCNN
{
public:
    virtual int detect(const cv::Mat& rgb, std::vector<Object>& objects);
    virtual int draw(cv::Mat& rgb, const std::vector<Object>& objects);
};

class Hand
{
public:
    int load(AAssetManager* mgr, const char* modeltype, int target_size, const float* mean_vals, const float* norm_vals, bool use_gpu = false);
    int detect(const cv::Mat& bgr, std::vector<cv::Rect>& hands, std::vector<cv::Mat>& trans_mats, std::vector<cv::Mat>& hand_crops);

private:
    ncnn::Net hand_net;
    int target_size;
    float mean_vals[3];
    float norm_vals[3];
};

#endif //YOLO11_NCNN_COMPOSE_MYNCNN_H
