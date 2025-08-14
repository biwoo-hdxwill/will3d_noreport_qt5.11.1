#pragma once
/*=========================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File:			image_2d_impl.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2017-08-31
Last modify:	2017-08-31

=========================================================================*/
#include <map>
#include <vector>
#include <memory>

#include "resource_defines.h"
#include "W3Resource.h"
#include "image_buffer.h"
#include "dicom_header.h"
#include "resource_global.h"

namespace resource {

/**************************************************************************************************
* @class	DicomInfo
*
* @brief	Information about the dicom.
*
* @author	Seo Seok Man
* @date	2017-08-31
**************************************************************************************************/
class DicomInfo {
public:
	explicit DicomInfo(std::map<std::string, std::string>& list = std::map<std::string, std::string>()) :
		header_(list) {}
	~DicomInfo();

	DicomInfo(const DicomInfo&) = delete;
	DicomInfo& operator=(const DicomInfo&) = delete;

public:
	void CopyData(DicomInfo* info);
	const DicomImageHeader& header() const;
	inline const float pixel_spacing() const noexcept { return pixel_spacing_; }
	inline const float window_level() const noexcept { return window_level_; }
	inline const float window_width() const noexcept { return window_width_; }
	inline const float intercept() const noexcept { return intercept_; }
	inline const float slope() const noexcept { return slope_; }

	void set_header(const DicomImageHeader& header);
	inline void set_pixel_spacing(const float ps) noexcept { pixel_spacing_ = ps; }
	inline void set_window_level(const float level) noexcept { window_level_ = level; }
	inline void set_window_width(const float width) noexcept { window_width_ = width; }
	inline void set_intercept(const float intercept) noexcept { intercept_ = intercept; }
	inline void set_slope(const float slope) noexcept { slope_ = slope; }

private:
	DicomImageHeader header_;

	float pixel_spacing_ = 0.0f;
	float window_level_  = 0.0f;
	float window_width_  = 0.0f;
	float intercept_     = 0.0f;
	float slope_         = 0.0f;
};

/**************************************************************************************************
 * @class	RESOURCE_EXPORT
 *
 * @brief	A resource export.
 *
 * @author	Seo Seok Man
 * @date	2017-08-31
 **************************************************************************************************/
class RESOURCE_EXPORT Image2DImpl : public CW3Resource {
public:
	Image2DImpl();
	Image2DImpl(uint width, uint height, const image::ImageFormat& format);
	virtual ~Image2DImpl() {}

	Image2DImpl(const Image2DImpl& impl) = delete;
	Image2DImpl& operator=(const Image2DImpl& impl) = delete;

public:
	void SetBuffer(uint width, uint height, const uchar* data, const image::ImageFormat& format);
	void Resize(int width, int height);
	void ClearData();
	void Copy(Image2DImpl* impl);
	void CopyData(const uchar* data);
	uchar* Data();

	// 2d image interface
	inline const image::ImageFormat format() const noexcept { return format_; }
	const float pitch() const;
	
	inline void set_format(const image::ImageFormat& format) noexcept { format_ = format; }
	void set_pitch(const float pitch) noexcept;
	
	// virtual functions.
	virtual const uint width() noexcept { return width_; }
	virtual const uint height() noexcept { return height_; }

	// dicom image's interface
	void SetDicomHeader(std::map<std::string, std::string>& header_list);
	inline const bool IsHeaderSet() const { return dicom_info_.get() ? true : false; }
	inline void SetPixelSpacing(const float ps) noexcept { dicom_info_->set_pixel_spacing(ps); }
	inline void SetWindowLevel(const float level) noexcept { dicom_info_->set_window_level(level); }
	inline void SetWindowWidth(const float width) noexcept { dicom_info_->set_window_width(width); }
	inline void SetIntercept(const float intercept) noexcept { dicom_info_->set_intercept(intercept); }
	inline void SetSlope(const float slope) noexcept { dicom_info_->set_slope(slope); }

	const DicomImageHeader& DicomHeader() const { return dicom_info_->header(); }
	inline const float PixelSpacing() const noexcept { return dicom_info_->pixel_spacing(); }
	inline const float WindowWidth() const noexcept { return dicom_info_->window_width(); }
	inline const float WindowLevel() const noexcept { return dicom_info_->window_level(); }
	inline const float Intercept() const noexcept { return dicom_info_->intercept(); }
	inline const float Slope() const noexcept { return dicom_info_->slope(); }

private:
	DicomInfo* dicom_info();

private:
	ImageBuffer data_;

	uint	width_  = 0;
	uint	height_ = 0;
	float	pitch_  = 0.0f;

	image::ImageFormat format_ = image::ImageFormat::UNKNOWN;

	std::unique_ptr<DicomInfo> dicom_info_;
};
} // end of namespace resource
