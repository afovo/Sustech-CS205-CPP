//
// Created by 卓著 on 2022/6/7.
//

#ifndef HAND_TRACKING_YOLO_H
#define HAND_TRACKING_YOLO_H

#include <iostream>

#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"

#include <vector>

using namespace std;


struct result {
    int x;
    int y;
    int w;
    int h;
    float confidence;
};

class Yolo {
public:
    int m_size;
    float m_confidence;
    float m_threshold;
    cv::dnn::Net m_net;
    vector<cv::String> output_names;
    Yolo(const std::string& cfg_path, const std::string& weight_path, int size = 416, float conf_threshold = 0.5, float nms_threshold = 0.3);
    void inference(cv::Mat &frame, cv::Rect* _box);
};

Yolo::Yolo(const std::string& cfg_path, const std::string& weight_path, int size, float conf_threshold, float nms_threshold) {
    m_size = size;
    m_confidence = conf_threshold;
    m_threshold = nms_threshold;
    m_net = cv::dnn::readNetFromDarknet(cfg_path,weight_path);
    vector<cv::String> ln = m_net.getLayerNames();
    for(auto i : m_net.getUnconnectedOutLayers()) {
        output_names.push_back(ln[int(i)-1]);
    }
}

void Yolo::inference(cv::Mat &frame, cv::Rect* _box) {
    int ih = frame.size().height;
    int iw = frame.size().width;
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(m_size, m_size), cv::Scalar(0, 0, 0), true, false);
    m_net.setInput(blob);
    vector<cv::Mat> outs;
    m_net.forward(outs, output_names);
    vector<int> class_ids;
    vector<float> confidences;
    vector<cv::Rect> boxes;
    for(auto output : outs) {
        for(int i = 0;i < output.rows;i++) {
            cv::Mat detections = output.row(i);
            float confidence = detections.at<float>(0,5);
            if(confidence > m_confidence) {
                int center_x = detections.at<float>(0,0) * iw;
                int center_y = detections.at<float>(0,1) * ih;
                int width = detections.at<float>(0,2) * iw;
                int height = detections.at<float>(0,3) * ih;
                int left = center_x - width / 2;
                int top = center_y - height / 2;
                int right = center_x + width / 2;
                int bottom = center_y + height / 2;
                boxes.push_back(cv::Rect(left,top,right-left,bottom-top));
                confidences.push_back(confidence);
            }
        }
    }

    vector<int> idx;
    cv::dnn::NMSBoxes(boxes, confidences, m_confidence, m_threshold, idx);



    //get the box with the max confidence
    float max_confidence = 0;
    int max_index = 0;
    for(int i = 0;i < idx.size();i++) {
        if(confidences[i] > max_confidence) {
            max_confidence = confidences[i];
            max_index = i;
        }
    }
    if(max_confidence > 0) {
        _box->x = boxes[max_index].x;
        _box->y = boxes[max_index].y;
        _box->width = boxes[max_index].width;
        _box->height = boxes[max_index].height;
    }
}


#endif //HAND_TRACKING_YOLO_H
