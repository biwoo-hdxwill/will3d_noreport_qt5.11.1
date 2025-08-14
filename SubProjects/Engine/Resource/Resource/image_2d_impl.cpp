#include "image_2d_impl.h"
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File: image_2d_impl.cpp
Desc: In header.
=========================================================================*/

namespace {
const int kIDUnAssigned = -1;
} // end of namespace

namespace resource {
Image2DImpl::Image2DImpl() :
	width_(0u), height_(0u) {}
Image2DImpl::Image2DImpl(uint width, uint height, const image::ImageFormat & format) :
	width_(width), height_(height), format_(format), data_(width, height, format) {
	// smseo : 일단 RGBA32 또는 RGB32가 아닐 경우에 Dicom 영상이라고 판단하고 dicom_info 를 만든다.
	// 추후 Dicom 영상에 대한 조건이 만들어지면 변경한다.
	if (format != image::ImageFormat::RGBA32 ||
		format != image::ImageFormat::GRAY16UI )
#if 0
		dicom_info_ = std::make_unique<DicomInfo>();
#else
		dicom_info_.reset(new DicomInfo());
#endif
}

void Image2DImpl::SetBuffer(uint width, uint height, const uchar * data,
							const image::ImageFormat & format) {
	width_ = width;
	height_ = height;
	format_ = format;
	data_.Resize(width, height, format_);
	data_.CopyData(data, format_, width, height);
}

void Image2DImpl::Resize(int width, int height) {
	width_ = width;
	height_ = height;
	data_.Resize(width_, height_, format_);
}
void Image2DImpl::ClearData() {
	data_.Clear();
}
void Image2DImpl::Copy(Image2DImpl* impl) {
	width_        = impl->width();
	height_       = impl->height();
	pitch_        = impl->pitch();
	format_       = impl->format();
	data_.CopyData(impl->Data(), format_, width_, height_);

	if (dicom_info_)
		dicom_info_->CopyData(impl->dicom_info());

	//BaseResource::CopyData(impl);
}
void Image2DImpl::CopyData(const uchar * data) {
	data_.CopyData(data, format_, width_, height_);
}
uchar* Image2DImpl::Data() {
	return data_.data();
}

// DICOM image 인 경우 pitch 대신 pixel spacing 을 사용한다.
const float Image2DImpl::pitch() const {
	return dicom_info_ ? dicom_info_->pixel_spacing() : pitch_;
}
void Image2DImpl::set_pitch(const float pitch) noexcept {
	pitch_ = pitch;
}

void Image2DImpl::SetDicomHeader(std::map<std::string, std::string>& header_list) {
	//dicom_info_.reset(new DicomInfo(header_list));
	
	float pixel_spacing = atof(header_list["PixelSpacing"].c_str());
	SetPixelSpacing(pixel_spacing);
	
	float slope = atof(header_list["RescaleSlope"].c_str());
	SetSlope(slope);

	float intercept = atof(header_list["RescaleIntercept"].c_str());
	SetIntercept(intercept);

	float window_center = atof(header_list["WindowCenter"].c_str());
	SetWindowLevel(window_center);

	float window_width = atof(header_list["WindowWidth"].c_str());
	SetWindowWidth(window_width);
}

DicomInfo * Image2DImpl::dicom_info() {
	return dicom_info_.get();
}


DicomInfo::~DicomInfo() {}

void DicomInfo::CopyData(DicomInfo * info) {
	header_        = info->header();
	pixel_spacing_ = info->pixel_spacing();
	window_level_  = info->window_level();
	window_width_  = info->window_width();
	intercept_     = info->intercept();
	slope_         = info->slope();
}

/**************************************************************************************************
*	class DicomImageHeader
**************************************************************************************************/
const DicomImageHeader& DicomInfo::header() const {
	return header_;
}
void DicomInfo::set_header(const DicomImageHeader& header) {
	header_ = header;
}
}// end of namespace resource
