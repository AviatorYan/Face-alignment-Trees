#include "Utils.h"

using cv::Mat;
using cv::FileStorage;

#ifndef eps
#define eps	0.0001f
#endif

//��ͨ��Mat����ɶ�ά����
//�豣֤�����ַ����
void saveMat(const char *fileName, Mat mat){
	FileStorage fs(fileName,FileStorage::WRITE);
	if (!fs.open(fileName, FileStorage::WRITE)){
		fprintf(stderr, "Error:The model file cannot be open.\n");
		return;
	}
	if (!mat.isContinuous()){
		fprintf(stderr, "Error:The matrix is not continue.\n");
		return ;
	}
	Mat t = Mat(mat.rows,mat.cols*mat.channels(),mat.depth(),mat.data);
	fs << "mat" << t;

	fs.release();
	return ;

}

Mat readMat(const char *fileName,int channels){
	FileStorage fs(fileName, FileStorage::READ);
	if (!fs.open(fileName, FileStorage::READ)){
		fprintf(stderr, "The model file cannot be open.\n");
	}
	Mat result;
	fs["mat"]>>result;
	result=result.reshape(channels,result.rows);
	fs.release();
	return result;
}

//�ȽϾ���Ķ�Ӧλ�õĸ���Ԫ�أ�����һ�����󣬴������������Ԫ�ص�λ��x,y,c
//tolerance��Ԫ�ز���ȵ����̶ȣ��Ǹ�
Mat compare(Mat a, Mat b,double tolerance){
	int adims[3] = { a.rows, a.cols, a.channels() };
	int bdims[3] = { b.rows, b.cols, b.channels() };
	if (adims[0] != bdims[0] || adims[1] != bdims[1] ||
		adims[2] != bdims[2]){
		fprintf(stderr, "Error:The input matrix cannot be compared.\n");
	}

	//��ͨ�����ֱ�Ϊx,y,c
	Mat result = Mat(1, adims[0] * adims[1] * adims[2] * 3, CV_32S);
	int len = 0;
	if (a.depth() == CV_64F && b.depth() == CV_64F){
		if (tolerance < 0){
			fprintf(stderr, "Error:tolerance should be positive.\n");
			return Mat();
		}
		double *as = (double *)a.data;
		double *bs = (double *)b.data;
		int *rs = (int *)result.data;
		len = 0;
		for (int y = 0; y < adims[0]; ++y){
			for (int x = 0; x < adims[1]; ++x){
				for (int c = 0; c < adims[2]; ++c){

					if (*as - *bs>tolerance || *bs - *as>tolerance){
						*(rs++) = x;
						*(rs++) = y;
						*(rs++) = c;
						len += 3;
					}
					as++;
					bs++;
				}
			}
		}
	}
	else if (a.depth() == CV_32S && b.depth() == CV_32S){
		int *as = (int *)a.data;
		int *bs = (int *)b.data;
		int *rs = (int *)result.data;
		len = 0;
		for (int y = 0; y < adims[0]; ++y){
			for (int x = 0; x < adims[1]; ++x){
				for (int c = 0; c < adims[2]; ++c){

					if (*as - *bs>tolerance || *bs - *as>tolerance){
						*(rs++) = x;
						*(rs++) = y;
						*(rs++) = c;
						len += 3;
					}
					as++;
					bs++;
				}
			}
		}
	}
	else{

		fprintf(stderr, "Error:The element type of the matrix is wrong.\n");
	}



	
	return result.colRange(0,len);
}

//��������ȥ��padding����֤��ַ����
//padx:x����padding����
//pady:y����padding����
Mat rmPadding(Mat src, int padx, int pady){
	int h = src.rows - 2 * padx;
	int w = src.cols - 2 * pady;
	int channels = src.channels();
	if (src.depth() != CV_64F){
		fprintf(stderr, "Error:The element type of the input matrix must be double.\n");
	}
	Mat result = Mat(h, w, CV_64FC(channels));
	double *r = (double *)result.data;
	int step = src.step1();
	double *s = (double *)src.data + pady *step + padx*channels;
	for (int y = 0; y < h; ++y){
		double *ss = s + y*step;
		//����ÿһ��
		for (int x = 0; x < w; ++x){
			for (int c = 0; c < channels; ++c){
				*(r++) = *(ss++);
			}
		}
	}

	return result;
}
