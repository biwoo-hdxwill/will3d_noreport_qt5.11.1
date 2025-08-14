#pragma once
/**=================================================================================================
Copyright (c) 2017 All rights reserved by HDXWILL.

File:			image_2d.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2017-08-31
Last modify:	2017-08-31

*===============================================================================================**/
#include <memory>
#include <vector>

#include "resource_defines.h"
#include "resource_global.h"

class QDateTime;

namespace resource {
class Image2DImpl;
class DicomImageHeader;
/**********************************************************************************************//**
 * @class	Image2D
 *
 * @brief	2d image's interface class
 *			값을 입력받는 함수들에는 값의 유효성을 점검하는 항목이 없음.
 *			따라서 값을 검사하고 해당 클래스에 입력해야 함.
 *
 * @author	Seo Seok Man
 * @date	2017-08-31
 **************************************************************************************************/
class RESOURCE_EXPORT Image2D {
public:
	Image2D();
	Image2D(uint width, uint height,
			const image::ImageFormat& format);

	Image2D(const Image2D&);
	Image2D& operator=(const Image2D& image);

	~Image2D();

public:
	// public interfaces
	bool IsReady() const;
	void Resize(uint width, uint height);
	void ClearData();
	void CopyData(const uchar* data);
	uchar* Data() const;

	//bool ImportImage(const QString& path);
	//bool ExportImage(const QString& path);

	const image::ImageFormat Format() const;

	const uint Width() const;
	const uint Height() const;

	/**********************************************************************************************
	Gets the pitch.
	
	@author	Seo Seok Man
	@date	2017-11-24
	
	@return	Dicom 영상인 경우 pixel spacing 을, 그 이외의 경우 pitch 를 리턴한다.
	 **********************************************************************************************/
	const float Pitch() const;

	const bool IsHeaderSet() const;
	const DicomImageHeader& DicomHeader() const;
	const float PixelSpacing() const;
	const float WindowWidth() const;
	const float WindowLevel() const;
	const float Intercept() const;
	const float Slope() const;

	void SetFormat(const image::ImageFormat& format);
	void SetPitch(const float pitch);

	void SetPixelSpacing(const float ps);
	void SetWindowWidth(const float ww);
	void SetWindowLevel(const float wl);
	void SetIntercept(const float intercept);
	void SetSlope(const float slope);

private:
	Image2DImpl* impl_ = nullptr;
};
} // end of namespace resource
