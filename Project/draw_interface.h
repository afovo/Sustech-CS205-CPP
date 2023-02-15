//
// Created by 卓著 on 2022/6/18.
//

#ifndef HAND_TRACKING_DRAW_INTERFACE_H
#define HAND_TRACKING_DRAW_INTERFACE_H

#include <opencv2/opencv.hpp>
#include "draw.h"


class Draw_Interface {
public:
    bool is_drawing;
    bool has_saved;
    bool m_has_hand;
    bool m_is_open;
    cv::Mat m_frame;
    Draw m_draw;
    Draw_Interface(Draw &draw);
    void start(cv::Rect* box);
    void set_hand_status(bool has_hand, bool is_open);
    void set_frame(cv::Mat& frame);

};

Draw_Interface::Draw_Interface(Draw &draw) {
    this->is_drawing = false;
    this->has_saved = false;
    this->m_has_hand = false;
    this->m_is_open = false;
    this->m_draw = std::move(draw);
}

void Draw_Interface::set_hand_status(bool has_hand, bool is_open) {
    this->m_has_hand = has_hand;
    this->m_is_open = is_open;
}
void Draw_Interface::start(cv::Rect* box) {
    if(m_has_hand) {
        if(!m_is_open) {
            if(!is_drawing) {
                m_draw.clearPoint();
                is_drawing = true;
            }
            cv::circle(m_frame, cv::Point(box->x + box->width / 2, box->y + box->height / 2), 5, cv::Scalar(0,0,255), -1);
            m_draw.pushPoint(cv::Point(box->x + box->width / 2, box->y + box->height / 2));
            m_draw.drawOnCanvas(m_draw.newPoints, m_frame);
        }else {
            if(m_draw.newPoints.size() > 0) {
                cout << "拟合" << endl;
                m_draw.drawOnCanvas(m_draw.newPoints, m_frame);
                m_draw.fit(m_frame);
                is_drawing = false;
            }
        }
    }else {
        m_draw.clearPoint();
    }
}

void Draw_Interface::set_frame(cv::Mat& frame) {
    this->m_frame = frame;
}

#endif //HAND_TRACKING_DRAW_INTERFACE_H
