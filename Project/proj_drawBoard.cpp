#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "conio.h"
#include <windows.h>

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
cv::Point getContours(cv::Mat imgDil)
{
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	findContours(imgDil, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	vector<vector<cv::Point>> conPoly(contours.size());
	vector<cv::Rect> boundRect(contours.size());

	cv::Point myPoint(0, 0); //笔尖坐标
	int max = -1;		 //储存面积最大的区域的下标

	for (int i = 0; i < contours.size(); i++)
	{
		if (cv::contourArea(contours[i]) > 0)
		{
			//绘制边缘
			float peri = cv::arcLength(contours[i], true);
			cv::approxPolyDP(contours[i], conPoly[i], 0.01 * peri, true);
			// drawContours(img, conPoly, i, Scalar(255, 0, 255), 3);
			// cout << conPoly[i].size() << endl;
			//绘制矩形框
			//  boundRect[i] = boundingRect(conPoly[i]);
			//  rectangle(img, boundRect[i].tl(), boundRect[i].br(), Scalar(0, 255, 0));
			//找出面积最大的区域
			if (max != -1)
			{
				if (cv::contourArea(contours[i]) > cv::contourArea(contours[max]))
					max = i;
			}
			else
				max = i;
		}
	}

	if (max != -1)
	{
		//绘制边缘
		float peri = cv::arcLength(contours[max], true);
		cv::approxPolyDP(contours[max], conPoly[max], 0.01 * peri, true);
		// drawContours(img, conPoly, max, Scalar(255, 0, 255), 3);
		//  cout << conPoly[i].size() << endl;
		//绘制矩形框
		boundRect[max] = cv::boundingRect(conPoly[max]);
		// rectangle(img, boundRect[max].tl(), boundRect[max].br(), Scalar(0, 255, 0));
		//获取面积最大的区域（视为笔尖）的坐标
		myPoint.x = boundRect[max].x + boundRect[max].width / 2;
		myPoint.y = boundRect[max].y;
	}
	return myPoint;
}

vector<cv::Point> findColor(cv::Mat img)
{
	cv::Mat imgHSV;
	cvtColor(img, imgHSV, cv::COLOR_BGR2HLS);
	int max = 0;
	for (int i = 0; i < myColors.size(); i++)
	{
		cv::Scalar lower(myColors[i][0], myColors[i][1], myColors[i][2]);
		cv::Scalar upper(myColors[i][3], myColors[i][4], myColors[i][5]);
		cv::Mat mask;
		inRange(imgHSV, lower, upper, mask); //筛选出特定颜色的区域
		// imshow(to_string(i), mask);
		cv::Point myPoint = getContours(mask);
		if (myPoint.x != 0 && myPoint.y != 0)
		{
			newPoints.push_back(myPoint); //将符合条件的笔尖坐标存入newPoints
		}
	}
	return newPoints;
}

cv::Point prepoint; //储存前一个点的坐标
void drawOnCanvas(vector<cv::Point> newPoints, cv::Mat img)
{
	for (int i = 0; i < newPoints.size(); i++)
	{
		// draw a point
		circle(img, newPoints[i], 5, cv::Scalar(0, 0, 255), cv::FILLED);
		// if (i > 0 &&
		// 	// the distance between 2 points <50
		// 	sqrt((newPoints[i].x - prepoint.x) * (newPoints[i].x - prepoint.x) + (newPoints[i].y - prepoint.y) * (newPoints[i].y - prepoint.y)) < 50)
		line(img, prepoint, newPoints[i], cv::Scalar(0, 0, 255), 10);
		prepoint = cv::Point(newPoints[i].x, newPoints[i].y);
	}
}
//绘制轮廓函数
void drawapp(cv::Mat result, cv::Mat img2)
{
	for (int i = 0; i < result.rows; i++)
	{
		//最后一个坐标点与第一个坐标点连接
		if (i == result.rows - 1)
		{
			cv::Vec2i point1 = result.at<cv::Vec2i>(i);
			cv::Vec2i point2 = result.at<cv::Vec2i>(0);
			cv::line(img2, point1, point2, cv::Scalar(0, 0, 255), 2, 8, 0);
			break;
		}
		cv::Vec2i point1 = result.at<cv::Vec2i>(i);
		cv::Vec2i point2 = result.at<cv::Vec2i>(i + 1);
		line(img2, point1, point2, cv::Scalar(0, 0, 255), 2, 8, 0);
	}
}

static int state = 0;
int main(int argc, const char *argv[])
{
	//color choices
	cv::Scalar blue=cv::Scalar(255,215,0);
	cv::Scalar yellow=cv::Scalar(0,215,255);
	cv::Scalar red=cv::Scalar(80,23,227);

	cv::VideoCapture capture(0);
	//////////introduce and guidance/////////////////
	cv::Mat gui = cv::imread("Design/1.png"); // initial------WELCOME
	cv::imshow("Look Here", gui);
	cv::resizeWindow("Look Here", 310, 480);
	cv::moveWindow("Look Here", 100, 100);
	cv::waitKey(1000);

	while (true)
	{
		capture.read(video);
		findColor(video);
		drawVideo = cv::Mat::zeros(video.size(), video.type());
		image = cv::Mat::zeros(video.size(), video.type());
		res = cv::Mat::zeros(video.size(), video.type());
		if (state == 0) // Cursor Mode
		{
			gui = cv::imread("Design/2.png"); // Cursor mode------Here we go!
			cv::imshow("Look Here", gui);
			cv::imshow("read video", video);
			cv::moveWindow("read video", 460, 100);
			cv::waitKey(1);
			if (_kbhit())
			{
				if (_getch() == 't') // to drawBoard
				{
					state = 1;
					newPoints.clear();
				}
				if (_getch() == 'e') // to exist
				{
					state = 2;
				}
			}
		}
		else if (state == 1) // DrawBoard Mode
		{
			gui = cv::imread("Design/3.png"); // Cursor mode------Here we go!
			cv::imshow("Look Here", gui);
			cv::waitKey(1);

			drawOnCanvas(newPoints, image); //////////////generate the initial route
			// 将原始轨迹加到video并由drawVideo输出
			addWeighted(image, 0.3, video, 0.7, 0.0, drawVideo);

			//Blue//////////////////////经过它画笔就变颜色
			cv::rectangle(drawVideo,cv::Point(10,10),cv::Point(90,90),blue,cv::FILLED);//cv::Scalar(158, 168, 3)
			// cv::putText(drawVideo, "Blue", cv::Point(40,50), 0, 0.7,blue, 1, 8);// (3, 168, 158)

			//Yellow//////////////////////
			cv::rectangle(drawVideo,cv::Point(100,10),cv::Point(180,90),yellow,cv::FILLED);//cv::Scalar(255,125,64)
			
			//Red//////////////////////
			cv::rectangle(drawVideo,cv::Point(190,10),cv::Point(270,90),red,cv::FILLED);//cv::Scalar(255,125,64)
			
			//make a fist to cursor//////////////////////
			cv::rectangle(drawVideo,cv::Point(450,10),cv::Point(630,180),cv::Scalar(0, 255, 0));
			cv::putText(drawVideo, "Make fist to", cv::Point(460,70), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);// 
			cv::putText(drawVideo, "Cursor Mode", cv::Point(460,100), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);

			//make a fist to exist//////////////////////
			cv::rectangle(drawVideo,cv::Point(450,300),cv::Point(630,470),cv::Scalar(0, 255, 0));
			cv::putText(drawVideo, "Make fist to", cv::Point(460,370), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);//
			cv::putText(drawVideo, "Exist", cv::Point(460,400), 0, 0.7, cv::Scalar(0, 255, 0), 1, 8);// 

			cv::imshow("drawBoard", drawVideo);
			cv::moveWindow("drawBoard", 460, 100);
			if (_kbhit())
			{
				if (_getch() == 'o')//拟合信号
				{
					cv::RotatedRect rrect = minAreaRect(newPoints);
					cv::Point2f center = rrect.center;
					cv::circle(res, center, 2, cv::Scalar(0, 255, 0), 2, 8, 0);

					cv::Mat result;
					cv::approxPolyDP(newPoints, result, 50, true); //多边形拟合
					drawapp(result, res);					   //绘制拟合后的轮廓
					cout << "corners : " << result.rows << endl;
					if (result.rows == 3)
					{
						cv::putText(res, "triangle", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
					}
					if (result.rows == 4)
					{
						cv::putText(res, "rectangle", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
					}
					if (result.rows == 8)
					{
						cv::putText(res, "poly-8", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
					}
					if (result.rows > 12)
					{
						cv::putText(res, "circle", center, 0, 1, cv::Scalar(0, 255, 0), 1, 8);
					}
					cv::addWeighted(res, 0.3, video, 0.7, 0.0, res);
					for (double w = 0; w <= 1; w += 0.05)
					{
						cv::addWeighted(res, w, drawVideo, (1 - w), 0, drawVideo);
						cv::imshow("drawBoard", drawVideo);
						cv::waitKey(100);
					}
					cv::waitKey(1000);
				}
				if (_getch() == 's')//保存图片
				{
					sprintf_s(imagename, 200, "D:\\ARTS\\%d.jpg", artCount);
					cv::imwrite(imagename, drawVideo);
					newPoints.clear();
					artCount++;
				}
				if (_getch() == 't')//区域to cursor mode 抓握
				{
					state = 0;
				}
				if (_getch() == 'e')//区域 exist 抓握
				{
					state = 2;
				}
			}
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
