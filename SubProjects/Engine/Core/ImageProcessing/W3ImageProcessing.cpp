#include "W3ImageProcessing.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

CW3ImageProcessing::CW3ImageProcessing() {
}

CW3ImageProcessing::~CW3ImageProcessing() {
}
void CW3ImageProcessing::GaussianBlur(ushort * buffer, 
									  int buffer_width, int buffer_height,
									  SharpenLevel level) {
	float sigma;
	double weight;
	GetFilterParameter(level, &sigma, &weight);

	cv::Mat ori_buffer = cv::Mat(buffer_height, buffer_width, CV_16U);
	memcpy(ori_buffer.ptr<unsigned short>(0), buffer, sizeof(unsigned short)*buffer_width*buffer_height);

	int ksize = (int)MAX(3.0f, 2.0f * 4.0f * (sigma)+1.0f);
	if (ksize % 2 == 0) ksize++; // 홀수

	cv::Mat blurred;
	cv::GaussianBlur(ori_buffer, blurred, cv::Size(ksize, ksize), 0.0, 0.0, cv::BORDER_REPLICATE);

	memcpy(buffer, blurred.ptr<unsigned short>(0), sizeof(unsigned short)*buffer_width*buffer_height);
}
void CW3ImageProcessing::Sharpen(ushort * buffer,
								 int buffer_width, int buffer_height,
								 SharpenLevel level) {
	float sigma;
	double weight;
	GetFilterParameter(level, &sigma, &weight);

	cv::Mat ori_buffer = cv::Mat(buffer_height, buffer_width, CV_16U);
	memcpy(ori_buffer.ptr<unsigned short>(0), buffer, sizeof(unsigned short)*buffer_width*buffer_height);

	int ksize = (int)MAX(3.0f, 2.0f * 4.0f * (sigma)+1.0f);
	if (ksize % 2 == 0) ksize++; // 홀수

	cv::Mat blurred;
	cv::GaussianBlur(ori_buffer, blurred, cv::Size(ksize, ksize), 0.0, 0.0, cv::BORDER_REPLICATE);

	double max, min;
	max = min = 0.0;

	cv::minMaxLoc(ori_buffer, &min, &max);

	double inv_1_w = 1.0 / (1.0 - weight);

#pragma omp parallel for
	for (int j = 0; j < buffer_height; j++) {
		unsigned short* ori_data = ori_buffer.ptr<unsigned short>(0) + j*buffer_width;
		unsigned short* blur_data = blurred.ptr<unsigned short>(0) + j*buffer_width;
		for (int i = 0; i < buffer_width; i++) {
			double wsub = (static_cast<double>(*ori_data) -
				(weight*(static_cast<double>(*blur_data++))));

			double tmp = (wsub / (1.0 - weight));

			tmp = (tmp > max) ? max : (tmp < min) ? min : tmp;
			*ori_data++ = static_cast<unsigned short>(tmp);
		}
	}

	memcpy(buffer, ori_buffer.ptr<unsigned short>(0), sizeof(unsigned short)*buffer_width*buffer_height);
}

void CW3ImageProcessing::GetFilterParameter(const SharpenLevel & level, float * sigma, double * weight) {
	switch (level) {
	case SHARPEN_OFF:
		return;
	case SHARPEN_LEVEL_1:
		*sigma = 0.25f;
		*weight = 0.5;
		break;
	case SHARPEN_LEVEL_2:
		*sigma = 0.25f;
		*weight = 0.62;
		break;
	case SHARPEN_LEVEL_3:
		*sigma = 0.25f;
		*weight = 0.7;
		break;
	default:
		assert(false);
		break;
	}
}
