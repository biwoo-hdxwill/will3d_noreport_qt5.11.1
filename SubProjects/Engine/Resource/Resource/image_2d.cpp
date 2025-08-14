#include "image_2d.h"
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File: image_2d.cpp
Desc: In header.
=========================================================================*/
#include "../../Common/Common/W3Memory.h"
#include "image_2d_impl.h"

namespace resource {
Image2D::Image2D() :
	impl_(new Image2DImpl()) {}
Image2D::Image2D(uint width, uint height, const image::ImageFormat& format) :
	impl_(new Image2DImpl(width, height, format)) {}

Image2D::Image2D(const Image2D& image) {
	if (impl_)
		SAFE_DELETE_OBJECT(impl_);
	impl_ = new Image2DImpl(image.Width(), image.Height(), image.Format());
	impl_->Copy(image.impl_);
}
Image2D & Image2D::operator=(const Image2D& image) {
	if (impl_)
		SAFE_DELETE_OBJECT(impl_);
	impl_ = new Image2DImpl(image.Width(), image.Height(), image.Format());
	impl_->Copy(image.impl_);
	return *this;
}

Image2D::~Image2D() {
	SAFE_DELETE_OBJECT(impl_);
}

bool Image2D::IsReady() const {
	return impl_->width() == 0 || impl_->height() == 0 ? false : true;
}

void Image2D::Resize(uint width, uint height) {
	impl_->Resize(width, height);
}

void Image2D::ClearData() {
	impl_->ClearData();
}

void Image2D::CopyData(const uchar * data) {
	impl_->CopyData(data);
}

uchar * Image2D::Data() const {
	return impl_->Data();
}

const image::ImageFormat Image2D::Format() const {
	return impl_->format();
}

//bool Image2D::ImportImage(const QString & path) {
//	return ImageIO::ImportImage(path, impl_);
//}
//
//bool Image2D::ExportImage(const QString & path) {
//	return ImageIO::ExportImage(path, impl_);
//}

const uint Image2D::Width() const {
	return impl_->width();
}

const uint Image2D::Height() const {
	return impl_->height();
}

const float Image2D::Pitch() const {
	return impl_->pitch();
}

const bool Image2D::IsHeaderSet() const {
	return impl_->IsHeaderSet();
}

const DicomImageHeader& Image2D::DicomHeader() const {
	return impl_->DicomHeader();
}

const float Image2D::PixelSpacing() const {
	return impl_->PixelSpacing();
}

const float Image2D::WindowWidth() const {
	return impl_->WindowWidth();
}

const float Image2D::WindowLevel() const {
	return impl_->WindowLevel();
}

const float Image2D::Intercept() const {
	return impl_->Intercept();
}

const float Image2D::Slope() const {
	return impl_->Slope();
}

void Image2D::SetFormat(const image::ImageFormat & format) {
	impl_->set_format(format);
}

void Image2D::SetPitch(const float pitch) {
	impl_->set_pitch(pitch);
}

void Image2D::SetPixelSpacing(const float ps) {
	impl_->SetPixelSpacing(ps);
}

void Image2D::SetWindowWidth(const float ww) {
	impl_->SetWindowWidth(ww);
}

void Image2D::SetWindowLevel(const float wl) {
	impl_->SetWindowLevel(wl);
}

void Image2D::SetIntercept(const float intercept) {
	impl_->SetIntercept(intercept);
}

void Image2D::SetSlope(const float slope) {
	impl_->SetSlope(slope);
}
} // end of namespace resource
