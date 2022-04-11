// Qx_opencv.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include <iostream>
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>
#include <omp.h>
#include <time.h>
#include "opencv2/imgproc/imgproc_c.h"

using namespace cv;
using namespace std;

Mat Changethesize(Mat& src, double mul)   //如果图示大小不适合 可根据第二个参数调整图片大小
{
	// 让我们调节图像使用新的宽度和高度
	int down_width = 300;
	int down_height = 200;
	Mat resize_down;
	double scale_down = mul;
	Mat scaled_f_down;
	//resize
	resize(src, scaled_f_down, Size(), scale_down, scale_down, INTER_LINEAR);
	// 使用插值方法改变原图size
	Mat res_inter_linear, res_inter_nearest, res_inter_area;
	resize(src, res_inter_linear, Size(), scale_down, scale_down, INTER_LINEAR);
	vconcat(res_inter_linear, src);
	imshow("改变尺寸后图像", src);
	return src;
}

static int picnum = 0;
//对图像进行旋转，不crop的方法，对旋转矩阵的参数修改，以及输出Size的确认
//被旋转过后的图将存到dst
void Rotate(Mat& src, vector<Mat>& dst)
{
	Mat m;
	Point2f center = Point(src.cols / 2, src.rows / 2);
	m = getRotationMatrix2D(center, 90 * (picnum), 0.8);   
	double cos = abs(m.at<double>(0, 0));
	double sin = abs(m.at<double>(0, 1));
	int bound_w = cos * src.cols + sin * src.rows;
	int bound_h = sin * src.cols + cos * src.rows;
	m.at<double>(0, 2) += static_cast<float>(bound_w / 2 - src.cols / 2);  
	m.at<double>(1, 2) += static_cast<float>(bound_h / 2 - src.rows / 2);
	warpAffine(src, dst[picnum], m, Size(bound_w, bound_h));
	picnum++;
}


int height;
int width;
Mat Back;
//四个线程四张图像
DWORD WINAPI pic1_thread(PVOID pParam)
{
	Mat dst = *(Mat*)pParam;
	for (int i = 0; i < height; i++)   
	{
		for (int j = 0; j < width; j++)
		{
			Vec3b bgr = dst.at<Vec3b>(i, j);
			Back.at<Vec3b>(i, j + height)[0] = bgr[0];
			Back.at<Vec3b>(i, j + height)[1] = bgr[1];
			Back.at<Vec3b>(i, j + height)[2] = bgr[2];
		}
	}
	return 0;
}

DWORD WINAPI pic2_thread(PVOID pParam)
{
	Mat dst = *(Mat*)pParam;
	for (int i = 0; i < width; i++)   
	{
		for (int j = 0; j < height; j++)
		{
			Vec3b bgr = dst.at<Vec3b>(i, j);
			Back.at<Vec3b>(i + height, j)[0] = bgr[0];
			Back.at<Vec3b>(i + height, j)[1] = bgr[1];
			Back.at<Vec3b>(i + height, j)[2] = bgr[2];
		}
	}
	return 0;
}

DWORD WINAPI pic3_thread(PVOID pParam)
{
	Mat dst = *(Mat*)pParam;
	for (int i = 0; i < height; i++)   
	{
		for (int j = 0; j < width; j++)
		{
			Vec3b bgr = dst.at<Vec3b>(i, j);
			Back.at<Vec3b>(i + height + width, j + height)[0] = bgr[0];
			Back.at<Vec3b>(i + height + width, j + height)[1] = bgr[1];
			Back.at<Vec3b>(i + height + width, j + height)[2] = bgr[2];
		}
	}
	return 0;
}


DWORD WINAPI pic4_thread(PVOID pParam)
{
	Mat dst = *(Mat*)pParam;
	for (int i = 0; i < width; i++)   
	{
		for (int j = 0; j < height; j++)
		{
			Vec3b bgr = dst.at<Vec3b>(i, j);
			Back.at<Vec3b>(i + height, j + height + width)[0] = bgr[0];
			Back.at<Vec3b>(i + height, j + height + width)[1] = bgr[1];
			Back.at<Vec3b>(i + height, j + height + width)[2] = bgr[2];
		}
	}
	return 0;
}

void Merge(vector<Mat>& src, Mat& dst)
{
	height = src[0].rows;
	width = src[0].cols;
	int newSize = 2 * height + width;
	dst = Mat::zeros(Size(newSize, newSize), CV_8UC3);
	Back = Mat::zeros(Size(newSize, newSize), CV_8UC3);

	//多线程处理图层合并
	HANDLE hthread[4];
	hthread[0] = CreateThread(NULL, 0, pic1_thread, (PVOID)&src[0], 0, NULL);
	hthread[1] = CreateThread(NULL, 0, pic2_thread, (PVOID)&src[1], 0, NULL);
	hthread[2] = CreateThread(NULL, 0, pic3_thread, (PVOID)&src[2], 0, NULL);
	hthread[3] = CreateThread(NULL, 0, pic4_thread, (PVOID)&src[3], 0, NULL);
	if (hthread[0] == NULL|| hthread[1] == NULL|| hthread[2] == NULL|| hthread[3] == NULL) {
		cout << "CreateThread error()\n";
		return;
	}
	WaitForMultipleObjects(4, hthread, TRUE, INFINITE);

	copyTo(Back, dst, Mat());
	imshow("合成后的图像", dst);
	imwrite("QXTY.png", dst);
}

int main()
{
	// img1 原图   img2 旋转90度   img3 180度  img270度
	//为了防便观察 上下左右取同一张图展示人像
	Mat img1 = imread("123.png");
	Mat img2 = imread("123.png");
	Mat img3 = imread("123.png");
	Mat img4 = imread("123.png");
	imshow("原图像", img1);//显示p1

	img1 = Changethesize(img1, 0.4);
	img2 = Changethesize(img2, 0.4);
	img3 = Changethesize(img3, 0.4);
	img4 = Changethesize(img4, 0.4);
	vector<Mat> four(4);
	Rotate(img1, four);
	Rotate(img2, four);
	Rotate(img3, four);
	Rotate(img4, four);
	Merge(four, img1);
	waitKey(0);
}


// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
