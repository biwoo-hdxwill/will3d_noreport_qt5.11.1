#pragma once
/*=========================================================================

File:		class CW3Image2D
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/
#include "W3Resource.h"

#include "resource_global.h"

class CW3ImageHeader;
/*
	* Image Slice Data Structure.
	* Descriptions.
		- super class : CW3Resource
*/
class RESOURCE_EXPORT CW3Image2D : public CW3Resource {
public:
	CW3Image2D(void) : m_pHeader(nullptr), m_nWidth(0), m_nHeight(0), m_fPixelSpacing(0.0f), m_pusData(nullptr) {}
	CW3Image2D(CW3ImageHeader* header) : m_pHeader(header), m_nWidth(0), m_nHeight(0), m_fPixelSpacing(0.0f), m_pusData(nullptr) {}
	explicit CW3Image2D(unsigned int width, unsigned int height);
	virtual ~CW3Image2D(void) override;

public:
	// public functions.
	inline unsigned short* getData(void) const { return m_pusData; }
	inline CW3ImageHeader* getHeader(void) const { return m_pHeader; }
	inline void setHeader(CW3ImageHeader *pHeader) { m_pHeader = pHeader; }
	void copyFrom(unsigned short* data, unsigned int sz);
	void setZeros(void);
	void resize(int width, int height);

	inline void setWidth(unsigned int n) noexcept { m_nWidth = n; }
	inline void setHeight(unsigned int n) noexcept { m_nHeight = n; }

	inline const unsigned int width(void) const noexcept { return m_nWidth; }
	inline const unsigned int height(void) const noexcept { return m_nHeight; }
	inline const unsigned int sizeSlice(void) const noexcept { return m_nWidth * m_nHeight; }
	inline const float pixelSpacing(void) const noexcept { return m_fPixelSpacing; }
	inline void setPixelSpacing(const float pSpacing) noexcept { m_fPixelSpacing = pSpacing; }

private:
	// private member fields.
	CW3ImageHeader * m_pHeader;
	unsigned int m_nWidth;
	unsigned int m_nHeight;
	float m_fPixelSpacing;
	unsigned short* m_pusData;
};
