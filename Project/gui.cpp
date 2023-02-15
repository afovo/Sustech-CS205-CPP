//
// Created by 卓著 on 2022/6/18.
//

#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
using namespace std;
//现状：一直读图一直画同一个三角形并且在另一个窗口显示

// Mat video(原始视频流),Mat image (轨迹), Mat res(拟合后图)
// image=image叠video，res=res叠video
// Mat median===》render a window named drawBoard(把原始视频流和image&res叠加后不断刷新)
//目标：视频流(限时/手势辨别）读mat video， 图像在mat image, image叠video=median     show median
//                                        拟合图像在mat res, mat image遍历淡化, mat res 遍历强化
cv::Mat video;	   // Mat video(原始视频流，永不间断读入)
cv::Mat image;	   //轨迹
cv::Mat res;	   //拟合轨迹
cv::Mat drawVideo; //不断刷新的画板

int artCount = 0;
char *imagename = new char[200];

vector<cv::Point> newPoints;									// input point set
vector<vector<int>> myColors{{130, 55, 85, 205, 160, 210}}; //更加宽松的红色
//要输出的颜色
vector<cv::Scalar> myColorValues{{0, 0, 255}};



static int state = 1;
int main(int argc, const char *argv[])
{
    cv::VideoCapture capture(0);
    //////////introduce and guidance/////////////////
    cv::Mat gui = cv::imread("/Users/zhuozhu/CLionProjects/Hand-Tracking_0618/Design/1.png"); // initial------WELCOME
    cv::imshow("Look Here", gui);
    cv::resizeWindow("Look Here", 310, 480);
    cv::moveWindow("Look Here", 100, 100);
    cv::waitKey(1000);

    while (true)
    {
        capture.read(video);
        drawVideo = cv::Mat::zeros(video.size(), video.type());
        image = cv::Mat::zeros(video.size(), video.type());
        res = cv::Mat::zeros(video.size(), video.type());
        if (state == 0) // Cursor Mode
        {
            gui = cv::imread("/Users/zhuozhu/CLionProjects/Hand-Tracking_0618/Design/2.png"); // Cursor mode------Here we go!
            cv::imshow("Look Here", gui);
            cv::imshow("read video", video);
            cv::moveWindow("read video", 460, 100);
            cv::waitKey(1);
        }
        else if (state == 1) // DrawBoard Mode
        {
            gui = cv::imread("/Users/zhuozhu/CLionProjects/Hand-Tracking_0618/Design/3.png"); // Cursor mode------Here we go!
            cv::imshow("Look Here", gui);
            cv::waitKey(1);

            // 将原始轨迹加到video并由drawVideo输出
            addWeighted(image, 0.3, video, 0.7, 0.0, drawVideo);

            //Blue//////////////////////经过它画笔就变颜色
            rectangle(drawVideo,cv::Point(10,10),cv::Point(90,90),cv::Scalar(158, 168, 3));
            putText(drawVideo, "Blue", cv::Point(40,50), 0, 0.7, cv::Scalar(158, 168, 3), 1, 8);// (3, 168, 158)

            //Yellow//////////////////////
            rectangle(drawVideo,cv::Point(100,10),cv::Point(180,90),cv::Scalar(3, 168, 158));
            putText(drawVideo, "Yellow", cv::Point(100,50), 0, 0.7, cv::Scalar(3, 168, 158), 1, 8);//

            //Red//////////////////////
            // rectangle(drawVideo,Point(10,10),Point(90,90),Scalar(158, 168, 3));
            // putText(drawVideo, "Blue", Point(40,50), 0, 0.7, Scalar(255,0, 0), 1, 8);//

            //make a fist to cursor//////////////////////
            rectangle(drawVideo,cv::Point(450,10),cv::Point(630,180),cv::Scalar(0, 255, 0));
            putText(drawVideo, "Make fist to", cv::Point(460,70), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);//
            putText(drawVideo, "Cursor Mode", cv::Point(460,100), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);

            //make a fist to exist//////////////////////
            rectangle(drawVideo,cv::Point(450,300),cv::Point(630,470),cv::Scalar(0, 255, 0));
            putText(drawVideo, "Make fist to", cv::Point(460,370), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);//
            putText(drawVideo, "Exist", cv::Point(460,400), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);//

            imshow("drawBoard", drawVideo);
            cv::moveWindow("drawBoard", 460, 100);
        }
        else if (state == 2)
        {
            gui = cv::imread("Design/4.png"); // Exist------Good Bye!
            cv::imshow("Look Here", gui);
            cv::waitKey(2000);
            return 0;
        }
    }
    cv::waitKey(0);
    return 0;
}
