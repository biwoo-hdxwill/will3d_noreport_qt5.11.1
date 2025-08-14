#include "Inc/VpProcBase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVpProcBase::CVpProcBase()
	: m_nWidth(0), m_nHeight(0), m_nDepth(0), m_nWH(0), m_pbytData(NULL)
{
}

CVpProcBase::~CVpProcBase()
{
}

void CVpProcBase::Init2D(int w, int h,  unsigned char* pData)
{
	m_nWidth = w; m_nHeight = h; m_pbytData = pData;
	m_nWH = w*h;
}

void CVpProcBase::Init3D(int w, int h, int d,  unsigned char** ppData)
{
	m_nWidth = w; m_nHeight = h; m_nDepth = d; m_ppbytData = ppData;
	m_nWH = w*h;
}

void CVpProcBase::Init2D(int w, int h, voltype* pData)
{
	m_nWidth = w; m_nHeight = h; m_pvolData = pData;
	m_nWH = w*h;
}

void CVpProcBase::Init3D(int w, int h, int d, voltype** ppData)
{
	m_nWidth = w; m_nHeight = h; m_nDepth = d; m_ppvolData = ppData;
	m_nWH = w*h;
}
