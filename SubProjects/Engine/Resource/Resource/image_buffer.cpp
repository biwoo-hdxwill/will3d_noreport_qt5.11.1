#include "image_buffer.h"
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File: image_buffer.cpp
Desc: In header.
=========================================================================*/
// multithreading 버전을 사용할 때 주석을 해제하여 쓴다.
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include "../../Common/Common/W3Memory.h"

namespace resource {
ImageBuffer::ImageBuffer() {}
ImageBuffer::ImageBuffer(uint w, uint h, const ImageFormat& format) {
	Resize(w, h, format);
}

ImageBuffer::~ImageBuffer() {
	SAFE_DELETE_ARRAY(data_);
}

void ImageBuffer::Resize(uint w, uint h, const ImageFormat& format) {
	if (data_)
		SAFE_DELETE_ARRAY(data_);

	uint image_size = w*h;
	switch (format) {
	case ImageFormat::RGBA32:
		size_ = image_size * sizeof(image::RGBA32);
		break;
	case ImageFormat::GRAY16:
	case ImageFormat::GRAY16UI:
		size_ = image_size * sizeof(ushort);
		break;
	case ImageFormat::GRAY8:
		size_ = image_size * sizeof(uchar);
		break;
	default:
		size_ = 0u;
		data_ = nullptr;
		break;
	}
	data_ = new uchar[size_];
	memset(data_, 0, size_ * sizeof(uchar));
}

void ImageBuffer::CopyData(const uchar* source, const ImageFormat& format,
						   uint width, uint height) {
	std::memcpy(data_, source, size_);
}

void ImageBuffer::Clear() {
	std::memset(data_, 0, size_);
}
} // end of namespace resource
