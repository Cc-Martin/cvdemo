// CVDemo.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <opencv2\opencv.hpp>

#pragma warning(disable:4996)

using namespace cv;
using namespace std;

Mat addGaussianNoise(Mat &srcImag);
Mat addSaltNoise(const Mat srcImage, int n);
int dynamicStretch(const Mat& srcImg, Mat& dstImg);

int main(int argc, wchar_t* argv[])
{

	

	namedWindow("win1", 0);
	namedWindow("win2", 0);
	namedWindow("win3", 0);

	//Mat inImage = imread("lena.jpg", IMREAD_GRAYSCALE);
	//Mat inImage = imread("timg1.jpg", IMREAD_GRAYSCALE);
	Mat inImage = imread("timg.jpg", IMREAD_GRAYSCALE);
	

	Mat srcImage = inImage.clone();
	Mat dstImage(inImage.rows, inImage.cols, CV_8UC1, Scalar(0));

	Mat kern = (Mat_<float>(3, 3)<< 1.0f, 2.0f, 1.0f,
									2.0f, 4.0f, 2.0f,
									1.0f, 2.0f, 1.0f) / 16.0f;
		 
	
	int r = 1;
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(2 * r + 1, 2 * r + 1));  //创建结构元
	

	int thrvalue = 50;
	int channels = 0;
	MatND dstHist;  //定义存储直方图变量 
	float hranges[] = { 0, 256 };  //范围[0,256)注意是最大值加1
	const float* ranges[] = { hranges };
	int bins = 256;
	int scale = 1;
	Mat HistImg(bins * scale, bins * 1, CV_8UC3, Scalar(0));  //定义直方图输出图像
	double minValue = 0, maxValue = 0;	
	int hpt = saturate_cast<int>(0.9*bins);  //设置最大值并防止溢出
	cout <<"“0”为恢复原图，“1”添加高斯噪声，“2”添加椒盐噪声" << endl;
	cout << "" << endl;
	cout <<"“3”filter2D，“4”均值滤波，“5”中值滤波，“6”高斯滤波" << endl;
	cout << "" << endl;
	cout <<"“t”固定阈值，“a”灰度直方图，“d”灰度均衡，“e”灰度拉伸" << endl;
	cout << "" << endl;
	cout <<"“p”膨胀，“f”腐蚀，“k”开运算，“b”闭运算" << endl;
	cout << "" << endl;
	int key;

	
	while (1) 
	{
		imshow("win1", srcImage);
		imshow("win2", dstImage);
		imshow("win3", HistImg);
		key = waitKey(1000);
		if (key == 27)
		{
			cout << "Exit!" << endl;
			return 0;
		}
	
		switch (key) 
		{
		case '0':
			srcImage = inImage.clone();
			cout << "恢复!" << endl;
			break;
		case '1':
			srcImage = addGaussianNoise(srcImage);
			cout<<"高斯噪声!"<<endl;
			break;
		case '2':
			srcImage = addSaltNoise(srcImage, 2000);
			cout << "椒盐噪声!" << endl;
			break;
		case '3':
			filter2D(srcImage, dstImage, srcImage.depth(), kern);
			cout << "filter2D!" << endl;
			break;
		case '4':
			blur(srcImage, dstImage, Size(3, 3)); //均值
			cout << "均值滤波!" << endl;
			break;
		case '5':
			medianBlur(srcImage, dstImage, 3); //中值
			cout << "中值滤波!" << endl;
			break;
		case '6':
			GaussianBlur(srcImage, dstImage, Size(5, 5), 0.8f);// 高斯
			cout << "高斯滤波!" << endl;
			break;
		case 't':
			threshold(srcImage, dstImage, thrvalue, 255, THRESH_BINARY);//  
			cout << "固定阈值!" << endl;
			break;

		case 'a':
			calcHist(&srcImage, 1, &channels, Mat(), dstHist, 1, &bins, ranges);
			minMaxLoc(dstHist, &minValue, &maxValue, 0, 0);
			for (int i = 0; i<256; i++)
			{
				float binValue = dstHist.at<float>(i);
				int realValue = saturate_cast<int>( binValue * hpt / maxValue);  //归一化数据
				line(HistImg, Point(i*scale, bins - 1), Point(i*scale, bins - realValue), Scalar(0, 255, 0));
			}
			
			cout << "直方图!!" << endl;
			break;
		case 'd':
			equalizeHist(srcImage, dstImage);//  
			cout << "灰度均衡!" << endl;
			break;
		case 'e':
			dynamicStretch(srcImage, dstImage);//  
			cout << "灰度拉伸!" << endl;
			break;
		case 'p':
			threshold(srcImage, dstImage, thrvalue, 255, THRESH_BINARY);//  
			morphologyEx(dstImage, dstImage, MORPH_DILATE, kernel);
			cout << "膨胀!" << endl;
			break;
		case 'f':
			threshold(srcImage, dstImage, thrvalue, 255, THRESH_BINARY);//  
			morphologyEx(dstImage, dstImage, MORPH_ERODE, kernel);
			cout << "腐蚀!" << endl;
			break;
		case 'k':
			threshold(srcImage, dstImage, thrvalue, 255, THRESH_BINARY);//  
			morphologyEx(dstImage, dstImage, MORPH_OPEN, kernel);     //  
			cout << "开!" << endl;
			break;
		case 'b':
			threshold(srcImage, dstImage, thrvalue, 255, THRESH_BINARY);//  
			morphologyEx(dstImage, dstImage, MORPH_CLOSE, kernel);
			cout << "闭!" << endl;
			break;
		default:
			break;
		}
	
	}


    return 0;
}

//动态灰度拉伸
int dynamicStretch(const Mat& srcImg, Mat& dstImg)
{
	assert(srcImg.type() == CV_8UC1);
	if (&srcImg != &dstImg)    // 当输入和输出相同时，对输出矩阵不需要重新分配空间
	{
		dstImg = Mat(srcImg.rows, srcImg.cols, CV_8UC1);
	}

	double maxVal = -std::numeric_limits<double>::max();
	double minVal = 0;
	
	minMaxIdx(srcImg, &minVal, &maxVal);  // 寻找图像的最大最小值

	for (auto i = 0; i < srcImg.rows; i++)
	{
		for (auto j = 0; j < srcImg.cols; j++)
		{
			uchar val = srcImg.at<uchar>(i, j);
			dstImg.at<uchar>(i, j) = static_cast<uchar>(255 * (val - minVal) * 1.0 / (maxVal - minVal));
		}
	}

	return 0;
}



//生成高斯噪声
double generateGaussianNoise(double mu, double sigma)
{
	//定义小值
	const double epsilon = numeric_limits<double>::min();
	static double z0, z1;
	static bool flag = false;
	flag = !flag;
	//flag为假构造高斯随机变量X
	if (!flag)
		return z1 * sigma + mu;
	double u1, u2;
	//构造随机变量
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);
	//flag为真构造高斯随机变量
	z0 = sqrt(-2.0*log(u1))*cos(2 * CV_PI*u2);
	z1 = sqrt(-2.0*log(u1))*sin(2 * CV_PI*u2);
	return z0*sigma + mu;
}


//为图像添加高斯噪声
Mat addGaussianNoise(Mat &srcImag)
{
	Mat dstImage = srcImag.clone();
	int channels = dstImage.channels();
	int rowsNumber = dstImage.rows;
	int colsNumber = dstImage.cols*channels;
	//判断图像的连续性
	if (dstImage.isContinuous())
	{
		colsNumber *= rowsNumber;
		rowsNumber = 1;
	}
	for (int i = 0; i < rowsNumber; i++)
	{
		for (int j = 0; j < colsNumber; j++)
		{
			//添加高斯噪声
			int val = dstImage.ptr<uchar>(i)[j] + generateGaussianNoise(0,2.0) * 16;
			if (val < 0)
				val = 0;
			if (val>255)
				val = 255;
			dstImage.ptr<uchar>(i)[j] = (uchar)val;
		}
	}
	return dstImage;
}

//为图像添加椒盐噪声
Mat addSaltNoise(const Mat srcImage, int n)
{
	Mat dstImage = srcImage.clone();

	for (int k = 0; k < n; k++)
	{
		//随机取值行列，得到像素点(i,j)
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;

		//图像通道判定
		if (dstImage.channels() == 1)//修改像素点(i,j)的像素值
		{
			dstImage.at<uchar>(i, j) = 255;     //盐噪声
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 255;
			dstImage.at<Vec3b>(i, j)[1] = 255;
			dstImage.at<Vec3b>(i, j)[2] = 255;
		}
	}

	for (int k = 0; k < n; k++)
	{
		//随机取值行列
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;
		//图像通道判定
		if (dstImage.channels() == 1)
		{
			dstImage.at<uchar>(i, j) = 0;       
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 0;
			dstImage.at<Vec3b>(i, j)[1] = 0;
			dstImage.at<Vec3b>(i, j)[2] = 0;
		}
	}
	return dstImage;
}
