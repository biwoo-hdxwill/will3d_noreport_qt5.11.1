#pragma once

#include "VpGlobalDefine.h"
#include "../imageprocessing_global.h"

#define DEF_SCALE					2
//#define DEF_SCALEFACTOR			.4f
#define	DEF_EPS						.0001f
#define DEF_THRESH_PIXELSIZE_L		.35f
#define DEF_THRESH_PIXELSIZE_H		.75f

//#if !defined UNSIGNEDDATA
#define DEF_BONETHRESH_L			800
#define DEF_BONETHRESH_H			1200
#define DEF_BONETHRESH_MH			1500
//#else
//	#define UNSIGNEDSHORT				32767
//	#define DEF_BONETHRESH_L			UNSIGNEDSHORT+800
//	#define DEF_BONETHRESH_H			UNSIGNEDSHORT+1200
//	#define DEF_BONETHRESH_MH			UNSIGNEDSHORT+1500
//#endif

#define DEF_SKINTHRESH				0
#define DEF_NOSE_RATIO_WIDTH		.07f

class IMAGEPROCESSING_EXPORT CVpSegTMJ
{
public:
	CVpSegTMJ();
	virtual ~CVpSegTMJ();

	void Init(int w, int h, int d, voltype** ppVolData, float pixelsize, voltype boneThreshold = 0, voltype tissueThreshold = 0);
	void Do(unsigned char** ppOutMask, bool bThick = true);
	void DoEx(unsigned char** ppOutMask);
	void DoEx2(unsigned char** ppOutMask);
	void Release();

private:
	int m_nOrigWidth;
	int m_nOrigHeight;
	int m_nOrigDepth;
	int m_nOrigWH;
	voltype** m_ppOrigVolData;

	int m_nWidth;
	int m_nHeight;
	int m_nDepth;
	int m_nWH;
	voltype** m_ppVolData;

	float m_fPixelSize;
	int m_nScale;

	voltype bone_threshold_ = DEF_BONETHRESH_H;
	voltype tissue_threshold_ = DEF_SKINTHRESH;
};
