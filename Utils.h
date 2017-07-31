#pragma once
#include <opencv2/opencv.hpp>

void saveMat(const char *fileName,cv::Mat mat);

cv::Mat readMat(const char *fileName,int channels);

cv::Mat compare(cv::Mat a, cv::Mat b,double tolerance);

//��������ȥ��padding����֤��ַ����
//padx:x����padding����
//pady:y����padding����
cv::Mat rmPadding(cv::Mat src, int padx, int pady);
