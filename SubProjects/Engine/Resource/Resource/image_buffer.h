#pragma once
/**********************************************************************************************
Project:		Resource
File:			resouce_defines.h
Language:		C++11
Library:		Qt 5.8.0
author:			Seo Seok Man
First date:		2017-11-23
Last modify:	2017-11-23
**********************************************************************************************/
#include "resource_defines.h"

#include "resource_global.h"

namespace resource {
using namespace image;

class RESOURCE_EXPORT ImageBuffer {
public:
	ImageBuffer();
	ImageBuffer(uint w, uint h, const ImageFormat& format);
	~ImageBuffer();

	ImageBuffer(const ImageBuffer&) = delete;
	ImageBuffer& operator=(const ImageBuffer&) = delete;

public:
	inline uchar* data() { return data_; }
	inline const uint size() const noexcept { return size_; }
	void Resize(uint w, uint h, const ImageFormat& format);
	void CopyData(const uchar* data, const ImageFormat& format,
				  uint w = 0, uint h = 0);
	void Clear();

private:
	uint size_ = 0;
	uchar* data_ = nullptr;
};
} // end of namespace resource
