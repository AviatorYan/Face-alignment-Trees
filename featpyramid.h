#pragma once
#include <opencv2/opencv.hpp>
#include "Utils.h"

cv::Mat	features2(const cv::Mat &image, const int sbin, const int padx, const int pady);

void features(cv::Mat image, int sbin, int padx, int pady, cv::Mat &feat_mat);

//�����˲�����Ӧ��
//ÿ���˲��������������ֱ��˲���
std::vector<cv::Mat> fconv(cv::Mat feature, std::vector<cv::Mat> filters);

//�����˲�����Ӧ����һ���˲����˲�
cv::Mat fconv(cv::Mat feature, cv::Mat filter);
