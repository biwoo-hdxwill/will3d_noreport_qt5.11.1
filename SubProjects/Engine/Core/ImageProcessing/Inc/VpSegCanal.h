#pragma once

#include "VpGlobalDefine.h"

#define DEF_KERNEL				13
#define kDownScale				2
#define DEF_SCALEFACTOR			.4f
#define	DEF_EPS					.0001f
#define DEF_THRESH_PIXELSIZE	.3f
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "../../../Common/Common/W3Types.h"
#include "../imageprocessing_global.h"

class IMAGEPROCESSING_EXPORT CVpSegCanal {
public:
	CVpSegCanal();
	virtual ~CVpSegCanal();

	// 초기화
	void Init(int w, int h, int d, usvoltype** ppVolData, float pixelsize, usvoltype thresh = 0);

	// 시드 설정 함수
	void SetSeeds2D(int startx, int starty, int endx, int endy);
	void SetSeeds3D(int startx, int starty, int startz, int endx, int endy, int endz);

	// 3차원 시드로 부터 canal 영역을 찾는 함수
	void Do(unsigned char** ppOrigOutCanal, bool bSmooth = true, float fEnhance = .2f);
	void Do(std::vector<glm::vec3>* pvOrigOutCanal, bool bSmooth = true, float fEnhance = .2f);

	// 2차원 시드로 부터 canal 영역을 찾는 함수
	void DoFromPano(unsigned char** ppOutCanal, bool bSmooth = true, float fEnhance = .2f);
	void DoFromPano(std::vector<glm::vec3>* pvOutCanal, bool bSmooth = true, float fEnhance = .2f);

	// release
	void Release();

	void GetSeedSlice(int* startslice, int* endslice);

private:
	enum {
		ID_NC,
		ID_IN = 1,
		ID_OUT = 255
	};

	// 원래 입력 볼륨 정보
	int m_nOrigWidth;
	int m_nOrigHeight;
	int m_nOrigDepth;
	int m_nOrigWH;
	usvoltype** m_ppOrigVolData;

	// scale된 입력 볼륨 정보
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;
	int m_nWH;
	usvoltype** m_ppVolData;

	// 시드 1, 2
	TPoint3D<int> m_pntStart;
	TPoint3D<int> m_pntEnd;

	// 시드 1, 2
	TPoint3D<int> m_pntOrigStart;
	TPoint3D<int> m_pntOrigEnd;

	int m_nScale;
	int m_nOrigKernelSize;
	int m_nKernelSize;
	usvoltype m_shVolThresh;

	// pixel size
	float m_fPixelSize;

	// path로 부터 canal 영역을 채워주는 함수들
	void GetKernelMask(int kernel, unsigned char** ppKernelMask);
	void ExpandPath(int w, int kernel, unsigned char** ppKernelMask, std::vector<int>* vecpathxy, std::vector<int>* vecpathz, unsigned char** ppExpandMask);

	// panorama 2차원 (x,y) 시드 점으로부터 3차원 시드를 찾아 시드의 z좌표를 반환해 주는 함수
	void DetectSeedSliceEx(int seed1x, int seed1y, int seed2x, int seed2y, int* slice1, int* slice2);
};
