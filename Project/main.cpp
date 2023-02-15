#include <iostream>
#include "yolo.h"
#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"
#include "draw.h"
#include "draw_interface.h"
#include <windows.h>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
using namespace std;

enum location {
    Middle,
    North,
    South,
    East,
    West,
    NorthEast,
    NorthWest,
    SouthEast,
    SouthWest
};

void showBox(cv::Rect* box) {
    cout << "box: " << box->x << " " << box->y << " " << box->width << " " << box->height << "\t";
    cout << "center: (" << box->x + box->width / 2 << ", " << box->y + box->height / 2 << ") " << "\t";
    cout << "Hand is" << (box->width > 150 ? " open " : " closed ") << "in the frame" << endl;
}

// void addLogo(cv::Mat frame){
//     cv::Mat logo = cv::imread("C:\\Users\\admin\\Desktop\\opencv_logo.png");
//     cv::Mat gray = cv::imread("C:\\Users\\admin\\Desktop\\opencv_logo.png", 0);
//     cv::Mat imageROI,mask;
//     imageROI = image(Rect(0, 0, logo.cols, logo.rows));
//     threshold(gray, mask, 10, 255, THRESH_BINARY);
//     imshow("img2graybin", mask);
//     logo.copyTo(imageROI, mask);
//     imshow("result", image);
// }

location getLocation(cv::Rect* box, cv::Mat& frame) {
    int center_x = box->x + box->width / 2;
    int center_y = box->y + box->height / 2;
    int center_x_frame = frame.size().width / 2;
    int center_y_frame = frame.size().height / 2;
    int bound = frame.size().height / 7;
    cv::Rect center_box = cv::Rect(center_x_frame - bound, center_y_frame - bound, bound * 2, bound * 2);
    //cv::rectangle(frame, center_box, cv::Scalar(0, 0, 255), 2);

    if(center_x < center_x_frame - bound) {
        if(center_y < center_y_frame - bound) {
            return NorthWest;
        } else if(center_y > center_y_frame + bound) {
            return SouthWest;
        } else {
            return West;
        }
    }else if (center_x > center_x_frame + bound) {
        if(center_y < center_y_frame - bound) {
            return NorthEast;
        } else if(center_y > center_y_frame + bound) {
            return SouthEast;
        } else {
            return East;
        }
    } else {
        if(center_y < center_y_frame - bound) {
            return North;
        } else if(center_y > center_y_frame + bound) {
            return South;
        } else {
            return Middle;
        }
    }
}

void showLocation(location l) {
    switch(l) {
        case Middle:
            cout << "Middle" << endl;
            break;
        case North:
            cout << "North" << endl;
            break;
        case South:
            cout << "South" << endl;
            break;
        case East:
            cout << "East" << endl;
            break;
        case West:
            cout << "West" << endl;
            break;
        case NorthEast:
            cout << "NorthEast" << endl;
            break;
        case NorthWest:
            cout << "NorthWest" << endl;
            break;
        case SouthEast:
            cout << "SouthEast" << endl;
            break;
        case SouthWest:
            cout << "SouthWest" << endl;
            break;
    }
}

cv::Rect getBox(deque<cv::Rect*> & boxes) {
    bool has_box = false;
    //judge whether all boxes are empty
    for(int i = 0; i < boxes.size(); i++) {
        if(boxes[i]->width > 0) {
            has_box = true;
            break;
        }
    }
    if(!has_box) {
        return cv::Rect(0, 0, 0, 0);
    }
    //get the box with the average width and height without empty box
    int width = 0;
    int height = 0;
    int count = 0;
    for(int i = 0; i < boxes.size(); i++) {
        if(boxes[i]->width > 0) {
            width += boxes[i]->width;
            height += boxes[i]->height;
            count++;
        }
    }
    width /= count;
    height /= count;
    //get the box with the average width and height without empty box
    int x = 0;
    int y = 0;
    for(int i = 0; i < boxes.size(); i++) {
        if(boxes[i]->width > 0) {
            x += boxes[i]->x;
            y += boxes[i]->y;
        }
    }
    x /= count;
    y /= count;
    return cv::Rect(x, y, width, height);
}

void moveMouse(bool has_hand, bool is_open, location l) {
    if (has_hand){            
            if(!is_open) {
                
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); //按下左键 
               
            }else {
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); //释放左键 
            }
            
            showLocation(l);
            POINT p;
            GetCursorPos(&p);
            switch (l)
            {
            case North:
                SetCursorPos(p.x, p.y - 10);
                break;
            case South:
                SetCursorPos(p.x, p.y + 10);
                break;
            case East:
                SetCursorPos(p.x + 10, p.y);
                break;
            case West:
                SetCursorPos(p.x - 10, p.y);
                break;
            case NorthEast:
                SetCursorPos(p.x + 10, p.y - 10);
                break; 
            case NorthWest:
                SetCursorPos(p.x - 10, p.y - 10);
                break;
            case SouthEast:
                SetCursorPos(p.x + 10, p.y + 10);
                break;
            case SouthWest:
                SetCursorPos(p.x - 10, p.y + 10);
                break;                       
            default:
                break;
            }
        }
}


int main() {
    //color choices
    cv::Scalar light_green=cv::Scalar(0, 255, 0);
	cv::Scalar blue=cv::Scalar(255,215,0);
	cv::Scalar yellow=cv::Scalar(0,215,255);
	cv::Scalar red=cv::Scalar(80,23,227);

    static const cv::String cfg_path = "models/cross-hands-tiny.cfg";
    static const cv::String weight_path = "models/cross-hands-tiny.weights";
    static int state = 0;//0:initial 1:drawBoard 2:exist
    static int count=0;//buffer to avoid frequently jumping between modes

    const int screen_height = 1080;
    const int screen_width = 1920;
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Cannot open camera" << endl;
        return -1;
    }
    /////draw interface//////
    Draw draw;
    Draw_Interface draw_interface(draw);
    /////DNN mode loading/////
    Yolo yolo(cfg_path, weight_path);
    deque<cv::Rect*> box_queue;

    //////////introduce and guidance/////////////////
	cv::Mat gui = cv::imread("Design/1.png"); // initial------WELCOME
	cv::imshow("Look Here", gui);
	cv::resizeWindow("Look Here", 310, 480);
	cv::moveWindow("Look Here", 100, 100);
	cv::waitKey(1000);

    while(true) {
        cv::Rect* box = new cv::Rect();
        cv::Mat frame;
        cap.read(frame);
        cv::flip(frame,frame,1);
        //use DNN to locate user's hand
        yolo.inference(frame, box);
        int iw = frame.size().width;
        int ih = frame.size().height;
        box_queue.push_back(box);
        //buffer of functional enhancing according your computer performance
        if(box_queue.size() > 5) {
            delete box_queue.front();
            box_queue.pop_front();
        }
        cv::Rect box_avg = getBox(box_queue);
        bool has_hand = (box_avg.width > 0 && box_avg.height > 0);
        bool is_open = (box_avg.width > 150);
        bool is_pressed = false;
        cv::Scalar box_color = is_open ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::rectangle(frame, box_avg, box_color, 2);
        cv::waitKey(1);
        count++;
        if (state == 0){
            using namespace cv;
            gui = cv::imread("Design/2.png"); // Cursor mode------Here we go!
			cv::imshow("Look Here", gui);
			


			location l = getLocation(&box_avg, frame);//cursor location
            moveMouse(has_hand, is_open, l);
 Mat img = frame;
    if (img.empty())
    {
        cout << "Can't read image." << endl;
        return 0;
    }

    Mat overlay = imread("1.png", -1);

    if (overlay.empty())
    {
        cout << "Can't read overlay image." << endl;
        return 0;
    }


    Rect target_roi(225, 150, img.cols, img.rows); // Set here, where to place overlay.

    //cv::resize(overlay, overlay, Size(target_roi.width, target_roi.height));

    Mat mask;
    if (overlay.channels() == 4)
    {
        vector<Mat> ch;
        split(overlay, ch);
        mask = 255 - ch[3].clone();
        mask.convertTo(mask, CV_32FC1, 1.0 / 255.0);
        ch.erase(ch.begin() + 3);
        merge(ch, overlay);
    }

    else
    {
        if (overlay.channels() == 3)
        {
            cvtColor(overlay, overlay, COLOR_BGR2GRAY);
        }

        overlay.convertTo(mask, CV_32FC1, 1.0 / 255.0);
    }


    for (int i = 0; i < overlay.rows; ++i)
    {
        for (int j = 0; j < overlay.cols; ++j)
        {
            float blending_coeff = mask.at<float>(i, j);

            Vec3b v1 = img.at<Vec3b>(i + target_roi.y, j + target_roi.x);
            Vec3b v2;
            if (overlay.channels() == 1)
            {
                int v = overlay.at<uchar>(i, j);
                v2 = (v, v, v);
            }
            else
            {
                v2 = overlay.at<Vec3b>(i, j);
            }

            Vec3f v1f(v1[0], v1[1], v1[2]);
            Vec3f v2f(v2[0], v2[1], v2[2]);

            Vec3f r = v1f * blending_coeff + (1.0 - blending_coeff) * v2f;
            img.at<Vec3b>(i + target_roi.y, j + target_roi.x) = r;
        }
    }









            //make a fist to drawBoard//////////////////////
			cv::rectangle(frame,cv::Point(450,10),cv::Point(630,180),light_green);
			cv::putText(frame, "Make fist to", cv::Point(460,70), 0, 0.7, light_green, 1, 8);// 
			cv::putText(frame, "Draw Board", cv::Point(460,100), 0, 0.7, light_green, 1, 8);

			//make a fist to exist//////////////////////
			cv::rectangle(frame,cv::Point(450,300),cv::Point(630,470),light_green);
			cv::putText(frame, "Make fist to", cv::Point(460,370), 0, 0.7, light_green, 1, 8);//
			cv::putText(frame, "Exist", cv::Point(460,400), 0, 0.7, light_green, 1, 8);// 
            cv::imshow("frame", frame);
			cv::moveWindow("frame", 460, 100);

            int center_x = box->x + box->width / 2;
            int center_y = box->y + box->height / 2;

            if(!is_open&&count>10){
                if(center_x > 450&&center_x < 630) {
                    if(center_y > 10&&center_y < 180) {
                        count=0;
                        state = 1;
                    }
                    else if(center_y > 350&&center_y < 450) {//to exist
                        count=0;
                        state = 2;
                    }
                }
            }
        }
       
        if (state==1){
            gui = cv::imread("Design/3.png"); // DrawBoard mode------DrawBoard
			cv::imshow("Look Here", gui);
			cv::waitKey(1);

			//Blue//////////////////////经过它画笔就变颜色
			cv::rectangle(frame,cv::Point(10,10),cv::Point(90,90),blue,cv::FILLED);
			//Yellow//////////////////////
			cv::rectangle(frame,cv::Point(100,10),cv::Point(180,90),yellow,cv::FILLED);
			//Red//////////////////////
			cv::rectangle(frame,cv::Point(190,10),cv::Point(270,90),red,cv::FILLED);
			
            int center_x = box->x + box->width / 2;
            int center_y = box->y + box->height / 2; 
            if(!is_open){
                if(center_x>10&&center_x<90){
                    if(center_y>10&&center_y<190){
                        draw_interface.m_draw.color=blue;
                    }
                }
                else if(center_x>100&&center_x<180){
                    if(center_y>10&&center_y<190){
                        draw_interface.m_draw.color=yellow;
                    }
                }
                else if(center_x>190&&center_x<270){
                    if(center_y>10&&center_y<190){
                        draw_interface.m_draw.color=red;
                    }
                }
            }

			//make a fist to cursor//////////////////////
			cv::rectangle(frame,cv::Point(450,10),cv::Point(630,180),light_green);
			cv::putText(frame, "Make fist to", cv::Point(460,70), 0, 0.7, light_green, 1, 8);
			cv::putText(frame, "Cursor Mode", cv::Point(460,100), 0, 0.7, light_green, 1, 8);

			//make a fist to exist//////////////////////
			cv::rectangle(frame,cv::Point(450,300),cv::Point(630,470),light_green);
			cv::putText(frame, "Make fist to", cv::Point(460,370), 0, 0.7, light_green, 1, 8);
			cv::putText(frame, "Exist", cv::Point(460,400), 0, 0.7, light_green, 1, 8);

            draw_interface.set_frame(frame);
            draw_interface.set_hand_status(has_hand, is_open);
            draw_interface.start(&box_avg);
            cv::imshow("frame", frame);
			cv::moveWindow("frame", 460, 100);
          
            if(!is_open&&count>10){
                if(center_x > 450&&center_x < 630) {
                    if(center_y > 10&&center_y < 180) {
                        count=0;
                        state = 0;
                    }
                    else if(center_y > 350&&center_y < 450) {//to exist
                        count=0;
                        state = 2;
                    }
                }
            }
        }

        else if(state==2){
            cv::destroyWindow("frame");
            gui = cv::imread("Design/4.png"); // Exist------Good Bye!
			cv::imshow("Look Here", gui);
			cv::waitKey(2000);
			return 0;
        }
        showBox(&box_avg);//type information in console
                
    }
}



