#include "W3Image2D.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
//CW3Image2D::CW3Image2D(void) : 
//	m_pHeader(nullptr),
//	width_(0),
//	height_(0),
//	m_fPixelSpacing(0.0f),
//	m_psData(nullptr)
//{
//}

CW3Image2D::CW3Image2D(unsigned int width, unsigned int height) :
	m_pHeader(nullptr),
	m_nWidth(width),
	m_nHeight(height),
	m_fPixelSpacing(0.0f),
	m_pusData(nullptr)
{
	W3::p_allocate_2D(&m_pusData, m_nWidth, m_nHeight);
}

CW3Image2D::~CW3Image2D(void)
{
	SAFE_DELETE_ARRAY(m_pusData);
}

void CW3Image2D::copyFrom(unsigned short* data, unsigned int sz)
{
	if (sz != this->sizeSlice()){
		common::Logger::instance()->Print(common::LogType::ERR, 
			"CW3Image2D::copyFrom : Different size of buffer");
		return;
	}
	std::memcpy(m_pusData, data, m_nWidth*m_nHeight*sizeof(unsigned short));
}

void CW3Image2D::setZeros(void)
{
	std::memset(m_pusData, 0, sizeof(unsigned short) * this->sizeSlice());
}

void CW3Image2D::resize(int width, int height)
{
	m_nWidth = width;
	m_nHeight = height;
	SAFE_DELETE_ARRAY(m_pusData);
	W3::p_allocate_1D(&m_pusData, width*height);
}
