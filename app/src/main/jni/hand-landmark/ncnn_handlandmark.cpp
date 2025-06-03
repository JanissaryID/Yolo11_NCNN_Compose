//
// Created by Krisn on 02/06/2025.
//
int NCNN_Hand::detect(const cv::Mat& rgb, std::vector<PalmObject>& objects, float prob_threshold, float nms_threshold)
{
    int width = rgb.cols;
    int height = rgb.rows;

    int w = width;
    int h = height;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)target_size / w;
        w = target_size;
        h = h * scale;
    }
    else
    {
        scale = (float)target_size / h;
        h = target_size;
        w = w * scale;
    }

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(rgb.data, ncnn::Mat::PIXEL_RGB, width, height, w, h);

    int wpad = target_size - w;
    int hpad = target_size - h;
    ncnn::Mat in_pad;
    ncnn::copy_make_border(in, in_pad, hpad / 2, hpad - hpad / 2, wpad / 2, wpad - wpad / 2, ncnn::BORDER_CONSTANT, 0.f);
    const float norm_vals[3] = { 1 / 255.f, 1 / 255.f, 1 / 255.f };
    in_pad.substract_mean_normalize(0, norm_vals);

    ncnn::Extractor ex = blazepalm_net.create_extractor();
    ncnn::Mat cls, reg;
    ex.input("input", in_pad);
    ex.extract("cls", cls);
    ex.extract("reg", reg);

    float* scores = (float*)cls.data;
    float* bboxes = (float*)reg.data;

    std::list<DetectRegion> region_list, region_nms_list;
    std::vector<DetectRegion> detect_results;
    decode_bounds(region_list, prob_threshold, target_size, target_size, scores, bboxes, anchors);
    non_max_suppression(region_list, region_nms_list, nms_threshold);
    objects.clear();
    pack_detect_result(detect_results, region_nms_list, target_size, objects);

    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].hand_pos[0].x = (objects[i].hand_pos[0].x * target_size - (wpad / 2)) / scale;
        objects[i].hand_pos[0].y = (objects[i].hand_pos[0].y * target_size - (hpad / 2)) / scale;
        objects[i].hand_pos[1].x = (objects[i].hand_pos[1].x * target_size - (wpad / 2)) / scale;
        objects[i].hand_pos[1].y = (objects[i].hand_pos[1].y * target_size - (hpad / 2)) / scale;
        objects[i].hand_pos[2].x = (objects[i].hand_pos[2].x * target_size - (wpad / 2)) / scale;
        objects[i].hand_pos[2].y = (objects[i].hand_pos[2].y * target_size - (hpad / 2)) / scale;
        objects[i].hand_pos[3].x = (objects[i].hand_pos[3].x * target_size - (wpad / 2)) / scale;
        objects[i].hand_pos[3].y = (objects[i].hand_pos[3].y * target_size - (hpad / 2)) / scale;

        //for (int j = 0; j < 7; j++)
        //{
        //    objects[i].landmarks[j].x = (objects[i].landmarks[j].x * target_size - (wpad / 2)) / scale;
        //    objects[i].landmarks[j].y = (objects[i].landmarks[j].y * target_size - (hpad / 2)) / scale;
        //}

        cv::Point2f srcPts[4];
        srcPts[0] = objects[i].hand_pos[0];
        srcPts[1] = objects[i].hand_pos[1];
        srcPts[2] = objects[i].hand_pos[2];
        srcPts[3] = objects[i].hand_pos[3];

        cv::Point2f dstPts[4];
        dstPts[0] = cv::Point2f(0, 0);
        dstPts[1] = cv::Point2f(224, 0);
        dstPts[2] = cv::Point2f(224, 224);
        dstPts[3] = cv::Point2f(0, 224);

        cv::Mat trans_mat = cv::getAffineTransform(srcPts, dstPts);
        cv::warpAffine(rgb, objects[i].trans_image, trans_mat, cv::Size(224, 224), 1, 0);

        cv::Mat trans_mat_inv;
        cv::invertAffineTransform(trans_mat, trans_mat_inv);

        float score = landmark.detect(objects[i].trans_image, trans_mat_inv, objects[i].skeleton);
    }

    return 0;
}

int NCNN_Hand::draw(cv::Mat& rgb, const std::vector<PalmObject>& objects)
{
    for (int i = 0; i < objects.size(); i++)
    {
        objects[i].trans_image.copyTo(rgb(cv::Rect(0,0,224,224)));
        for(int j = 0; j < objects[i].skeleton.size(); j++)
        {
            cv::Scalar color1(10, 215, 255);
            cv::Scalar color2(255, 115, 55);
            cv::Scalar color3(5, 255, 55);
            cv::Scalar color4(25, 15, 255);
            cv::Scalar color5(225, 15, 55);
            for(size_t j = 0; j < 21; j++)
            {
                cv::circle(rgb, objects[i].skeleton[j],4,cv::Scalar(255,0,0),-1);
                if (j < 4)
                {
                    cv::line(rgb, objects[i].skeleton[j], objects[i].skeleton[j+1], color1, 2, 8);
                }
                if (j < 8 && j > 4)
                {
                    cv::line(rgb, objects[i].skeleton[j], objects[i].skeleton[j+1], color2, 2, 8);
                }
                if (j < 12 && j > 8)
                {
                    cv::line(rgb, objects[i].skeleton[j], objects[i].skeleton[j+1], color3, 2, 8);
                }
                if (j < 16 && j > 12)
                {
                    cv::line(rgb, objects[i].skeleton[j], objects[i].skeleton[j+1], color4, 2, 8);
                }
                if (j < 20 && j > 16)
                {
                    cv::line(rgb, objects[i].skeleton[j], objects[i].skeleton[j+1], color5, 2, 8);
                }
            }
            cv::line(rgb, objects[i].skeleton[0], objects[i].skeleton[5], color2, 2, 8);
            cv::line(rgb, objects[i].skeleton[0], objects[i].skeleton[9], color3, 2, 8);
            cv::line(rgb, objects[i].skeleton[0], objects[i].skeleton[13], color4, 2, 8);
            cv::line(rgb, objects[i].skeleton[0], objects[i].skeleton[17], color5, 2, 8);
        }

        cv::line(rgb, objects[i].hand_pos[0], objects[i].hand_pos[1], cv::Scalar(0, 0, 255), 2, 8, 0);
        cv::line(rgb, objects[i].hand_pos[1], objects[i].hand_pos[2], cv::Scalar(0, 0, 255), 2, 8, 0);
        cv::line(rgb, objects[i].hand_pos[2], objects[i].hand_pos[3], cv::Scalar(0, 0, 255), 2, 8, 0);
        cv::line(rgb, objects[i].hand_pos[3], objects[i].hand_pos[0], cv::Scalar(0, 0, 255), 2, 8, 0);

    }

    return 0;
}