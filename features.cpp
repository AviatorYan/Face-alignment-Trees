#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "featpyramid.h"

using cv::Mat;
// small value, used to avoid division by zero
#define eps 0.0001
#define	round(x)	((x-floor(x))>0.5 ? ceil(x) : floor(x))


// unit vectors used to compute gradient orientation
double uu[9] = { 1.0000,
0.9397,
0.7660,
0.500,
0.1736,
-0.1736,
-0.5000,
-0.7660,
-0.9397 };
double vv[9] = { 0.0000,
0.3420,
0.6428,
0.8660,
0.9848,
0.9848,
0.8660,
0.6428,
0.3420 };


//����ͼ��Ϊ��ɫͼ������Ϊdouble����
void features(Mat image,int sbin,int padx,int pady,Mat &feat_mat){

	if (image.channels() != 3 || image.depth() != CV_32F){
		fprintf(stderr, "Invalid image input.\n");
	}
	const int dims[3] = {image.rows,image.cols,image.channels()};
	// memory for caching orientation histograms & their norms
	int blocks[2];

	//������block����ʾһ��8x8��С�飬paper������cell����ʾ�������ˣ������������H��W�����Ի��ֶ���cell 
	blocks[0] = (int)round((float)dims[0] / (float)sbin);	//h������ٸ�cell  
	blocks[1] = (int)round((float)dims[1] / (float)sbin);	//w������ٸ�cell  
	//���ݶȵ�ֱ��ͼ��ÿ������һҳ���ܹ�18�����򣬹�18ҳ  
	//�����ʼ��
	float *hist = (float *)calloc(blocks[0] * blocks[1] * 18 , sizeof(float));
	float *norm = (float *)calloc(blocks[0] * blocks[1] , sizeof(float));

	// memory for HOG features
	int out[3];
	out[0] = MAX(blocks[0] - 2, 0);	//��2��ԭ����Ϊͼ��û����չ��ֱ��ͼ�ĵ�һ�У���һ�к����һ�У����һ�в�������㣬���Կ��߸���ȥһ 
	out[1] = MAX(blocks[1] - 2, 0);
	out[2] = 27 + 4 + 1;			//ÿ��cell���������ά��Ϊ32ά����ͬ��ԭʼHOG��36ά 
	
	//���������,32ά��Ӧ������������32��ͨ��
	feat_mat = Mat(out[0] + pady * 2, out[1] + padx * 2, CV_32FC(out[2]));


	cv::Vec<float, 32>init_val;
	init_val = 0;
	init_val[out[2] - 1] = 1;    //write boundary occlusion feature
	feat_mat.setTo(init_val);

	int visible[2];//����ͼ��һ����cell��С�������������Ҫ���вü����������ǲü����H,W�� 
	visible[0] = blocks[0] * sbin;//h����
	visible[1] = blocks[1] * sbin;//w����

	float *im = (float *)image.data;
	int step = image.step1();
	//���ѭ�������ݶȷ���ͷ�ֵ����ͶӰ����Ӧ���ݶ�ֱ��ͼ�� 
	for (int y = 1; y < visible[0] - 1; y++) {
		float *imgpt = im + MIN(y, dims[0] - 2)*step;
		for (int x = 1; x < visible[1] - 1; x++) {
			// first color channel
			float *s = imgpt + MIN(x, dims[1] - 2)*dims[2];
			float dy = *(s + step) - *(s - step);
			float dx = *(s + dims[2]) - *(s - dims[2]);
			float v = dx*dx + dy*dy;

			// second color channel
			s++;
			float dy2 = *(s + step) - *(s - step);
			float dx2 = *(s + dims[2]) - *(s - dims[2]);
			float v2 = dx2*dx2 + dy2*dy2;

			// third color channel
			s++;
			float dy3 = *(s + step) - *(s - step);
			float dx3 = *(s + dims[2]) - *(s - dims[2]);
			float v3 = dx3*dx3 + dy3*dy3;

			// pick channel with strongest gradient
			if (v2 > v) {
				v = v2;
				dx = dx2;
				dy = dy2;
			}
			if (v3 > v) {
				v = v3;
				dx = dx3;
				dy = dy3;
			}

			//�ҵ���ǰ���ݶ�Ӧ��ͶӰ���ĸ�����[0, 2xPI]�ܹ�18��
			// snap to one of 18 orientations
			float best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) {
				float dot = uu[o] * dx + vv[o] * dy;
				if (dot > best_dot) {
					best_dot = dot;
					best_o = o;
				}
				else if (-dot > best_dot) {
					best_dot = -dot;
					best_o = o + 9;
				}
			}

			//�±��⼸�д�������������Բ�ֵ�ģ�ע������û��ʹ�������Բ�ֵ��ԭʼHOG��һ��  
			//ʡ�����ݶȵĲ�ֵ  
			// add to 4 histograms around pixel using linear interpolation
			float xp = ((float)x + 0.5) / (float)sbin - 0.5;
			float yp = ((float)y + 0.5) / (float)sbin - 0.5;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			float vx0 = xp - ixp;
			float vy0 = yp - iyp;
			float vx1 = 1.0 - vx0;
			float vy1 = 1.0 - vy0;
			v = sqrt(v);

			float *ttt = hist + ixp*blocks[0] + iyp + best_o*blocks[0] * blocks[1];
			//��ǰ���ض����½�cell�й���
			if (ixp >= 0 && iyp >= 0) {
				*(hist + ixp*blocks[0] + iyp + best_o*blocks[0] * blocks[1]) +=
					vx1*vy1*v;
			}

			if (ixp + 1 < blocks[1] && iyp >= 0) {
				*(hist + (ixp + 1)*blocks[0] + iyp + best_o*blocks[0] * blocks[1]) +=
					vx0*vy1*v;
			}

			if (ixp >= 0 && iyp + 1 < blocks[0]) {
				*(hist + ixp*blocks[0] + (iyp + 1) + best_o*blocks[0] * blocks[1]) +=
					vx1*vy0*v;
			}

			if (ixp + 1 < blocks[1] && iyp + 1 < blocks[0]) {
				*(hist + (ixp + 1)*blocks[0] + (iyp + 1) + best_o*blocks[0] * blocks[1]) +=
					vx0*vy0*v;
			}
		}
	}

	// ��Ϊ�ϱ��ǰ�[0�� 2PI]��Ϊ18�����򣬾ٸ�����10�Ⱥ�190��������������  
	// �����һ����ʱ��Ҫ��10�Ⱥ�190��������������һ���������Ҫ����һ��Ȼ����ƽ��  
	// norm��blocks[0]*blocks[1]��С�ģ�ÿһ��λ�ô���������ݶȷ����ƽ����  
	// compute energy in each block by summing over orientations
	for (int o = 0; o < 9; o++) {
		float *src1 = hist + o*blocks[0] * blocks[1];
		float *src2 = hist + (o + 9)*blocks[0] * blocks[1];
		float *dst = norm;
		float *end = norm + blocks[1] * blocks[0];
		while (dst < end) {
			*(dst++) += (*src1 + *src2) * (*src1 + *src2);
			src1++;
			src2++;
		}
	}

	// compute features
	//����������out[0] = blocks[0] - 2, out[1] = blocks[1] - 2; ��ֹԽ��  
	float *feat_data = feat_mat.ptr<float>(padx, pady);
	const int feat_step = feat_mat.step1();					//ÿ�и���
	for (int y = 0; y < out[0]; y++) {
		float *dst = feat_data + y*feat_step;	//���д���
		for (int x = 0; x < out[1]; x++) {
			
			float *src, *p, n1, n2, n3, n4;
			//�����ϱ߼������energy�����һ������  
			//ÿ��cell�����ĸ�block�����block��2x2��cell���Ǹ�block���������,���Ҫ��һ���ĴΣ��±߾������ĸ���һ������  
			p = norm + (x + 1)*blocks[0] + y + 1;
			n1 = 1.0 / sqrt(*p + *(p + 1) + *(p + blocks[0]) + *(p + blocks[0] + 1) + eps);
			p = norm + (x + 1)*blocks[0] + y;
			n2 = 1.0 / sqrt(*p + *(p + 1) + *(p + blocks[0]) + *(p + blocks[0] + 1) + eps);
			p = norm + x*blocks[0] + y + 1;
			n3 = 1.0 / sqrt(*p + *(p + 1) + *(p + blocks[0]) + *(p + blocks[0] + 1) + eps);
			p = norm + x*blocks[0] + y;
			n4 = 1.0 / sqrt(*p + *(p + 1) + *(p + blocks[0]) + *(p + blocks[0] + 1) + eps);

			float t1 = 0;
			float t2 = 0;
			float t3 = 0;
			float t4 = 0;

			// contrast-sensitive features
			//�����18��������Ϊ18��������Ҳ����10�Ⱥ�190���ǲ�ͬ������ 
			src = hist + (x + 1)*blocks[0] + (y + 1);
			for (int o = 0; o < 18; o++) {
				float h1 = MIN(*src * n1, 0.2);//clip, ����0.2������ֵ�ض� 
				float h2 = MIN(*src * n2, 0.2);
				float h3 = MIN(*src * n3, 0.2);
				float h4 = MIN(*src * n4, 0.2);
				*(dst++) = 0.5 * (h1 + h2 + h3 + h4);//�ĸ���һ��֮�������ֵ��ͳ���2
				t1 += h1;//��ǰcell���ڵ��ĸ�block��һ���������ֵ�ֱ������
				t2 += h2;
				t3 += h3;
				t4 += h4;
				src += blocks[0] * blocks[1];
			}

			// contrast-insensitive features
			//�����10�Ⱥ�190������һ������������Ҫ��һ��sumȻ���ٹ�һ���Ĵ�  
			src = hist + (x + 1)*blocks[0] + (y + 1);
			for (int o = 0; o < 9; o++) {
				float sum = *src + *(src + 9 * blocks[0] * blocks[1]);
				float h1 = MIN(sum * n1, 0.2);
				float h2 = MIN(sum * n2, 0.2);
				float h3 = MIN(sum * n3, 0.2);
				float h4 = MIN(sum * n4, 0.2);
				*(dst++) = 0.5 * (h1 + h2 + h3 + h4);
				src += blocks[0] * blocks[1];
			}

			// texture features
			//����������cell���ڵ��ĸ�block������ֵ�ĺͳ���һ��ϵ��
			*(dst++) = 0.2357 * t1;
			*(dst++) = 0.2357 * t2;
			*(dst++) = 0.2357 * t3;
			*(dst++) = 0.2357 * t4;

			// truncation feature
			//���һ��������0
			*(dst++) = 0;
		}
	}

	free(hist);
	free(norm);

	return ;
}