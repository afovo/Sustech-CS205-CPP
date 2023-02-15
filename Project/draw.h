//
// Created by 卓著 on 2022/6/17.
//

#ifndef HAND_TRACKING_DRAW_H
#define HAND_TRACKING_DRAW_H


#include <opencv2/opencv.hpp>
#include <sys/time.h>
#include <unistd.h>
#include <utility>
#include <vector>
using namespace std;
class Draw {
public:
    cv::Scalar color;// color of the line
    vector<cv::Point> newPoints;
    cv::Mat drawVideo;
    cv::Mat image;
    cv::Mat res;
    bool has_saved = false;
    void drawOnCanvas(vector<cv::Point> newPoints, cv::Mat img);
    void drawapp(cv::Mat result, cv::Mat img2);
    void pushPoint(cv::Point point);
    void clearPoint();
    void enclose();
    void fit(cv::Mat& frame);
    void save(cv::Mat& frame);
};

void Draw::drawOnCanvas(vector<cv::Point> newPoints, cv::Mat img)
{
    cv::Point prepoint = newPoints[0];
    for (int i = 0; i < newPoints.size(); i++)
    {
        circle(img, newPoints[i], 5,color, cv::FILLED);
        line(img, prepoint, newPoints[i], color, 10);
        prepoint = cv::Point(newPoints[i].x, newPoints[i].y);
    }
}

void Draw::drawapp(cv::Mat result, cv::Mat img2)
{
    if(result.rows <= 6) {
        cv::RotatedRect rrect = minAreaRect(newPoints);
        cv::Point2f center = rrect.center;
        for (int i = 0; i < result.rows; i++)
        {
            //最后一个坐标点与第一个坐标点连接
            if (i == result.rows - 1)
            {
                cv::Vec2i point1 = result.at<cv::Vec2i>(i);
                cv::Vec2i point2 = result.at<cv::Vec2i>(0);
                line(img2, point1, point2, cv::Scalar(255, 0, 0), 10, 8, 0);
                break;
            }
            cv::Vec2i point1 = result.at<cv::Vec2i>(i);
            cv::Vec2i point2 = result.at<cv::Vec2i>(i + 1);
            line(img2, point1, point2, cv::Scalar(255, 0, 0), 10, 8, 0);
        }
        if (result.rows == 3)
        {
            putText(res, "triangle", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
        }else if(result.rows == 4) {
            putText(res, "quadrilateral", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
        }else if(result.rows == 5) {
            putText(res, "pentagon", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
        }else if(result.rows == 6) {
            putText(res, "hexagon", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
        }else if(result.rows == 2) {
            putText(res, "line", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
        }
    }else {
        //calculate the mean of the points
        cv::Point meanPoint;
        for (int i = 0; i < result.rows; i++)
        {
            cv::Vec2i point = result.at<cv::Vec2i>(i);
            meanPoint.x += point[0];
            meanPoint.y += point[1];
        }
        meanPoint.x /= result.rows;
        meanPoint.y /= result.rows;

        //calculate the variance of the points
        double variance = 0;
        for (int i = 0; i < result.rows; i++)
        {
            cv::Vec2i point = result.at<cv::Vec2i>(i);
            variance += (point[0] - meanPoint.x) * (point[0] - meanPoint.x) + (point[1] - meanPoint.y) * (point[1] - meanPoint.y);
        }
        variance /= result.rows;
        //draw the circle accoding to the mean and variance
        circle(img2, meanPoint, sqrt(variance), cv::Scalar(255, 0, 0), 10);
    }

}



void Draw::pushPoint(cv::Point point) {
    newPoints.push_back(point);
}

void Draw::clearPoint() {
    newPoints.clear();
}

void Draw::enclose() {
    pushPoint(newPoints[0]);
}

void Draw::fit(cv::Mat& frame) {
    res = cv::Mat::zeros(frame.size(), frame.type());
    cv::Mat result;
    cv::approxPolyDP(newPoints, result, 50, true);
    drawapp(result, res);
    addWeighted(res, 0.7, frame, 0.3, 0, frame);
}

void Draw::save(cv::Mat& frame){
    if(!has_saved) {
        struct timeval now_time;
        gettimeofday(&now_time, NULL);
        time_t tt = now_time.tv_sec;
        tm *temp = localtime(&tt);
        char time_str[40]={NULL};
        sprintf(time_str,"%04d-%02d-%02d-%02d:%02d:%02d",temp->tm_year+ 1900,temp->tm_mon+1,temp->tm_mday,temp->tm_hour,temp->tm_min, temp->tm_sec);
        string time_str_str(time_str);
        string file_name = "/save/" + time_str_str + ".jpg";
        cout << file_name << endl;
        imwrite(file_name, frame);
        has_saved = true;
    }
}
#endif //HAND_TRACKING_DRAW_H
