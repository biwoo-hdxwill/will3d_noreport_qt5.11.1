#pragma once

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "../../../Common/Common/W3Types.h"
#include "../../../Core/ImageProcessing/Inc/VpGlobalDefine.h"

class CVpSegCanalEx {
public:
	CVpSegCanalEx();
	virtual ~CVpSegCanalEx();

	// 초기화
	void Init(int w, int h, int d, usvoltype ** ppVolData, float pixelsize, usvoltype thdBone);

	// 시드 설정 함수
	void SetSeeds3D(int x1, int y1, int z1, int x2, int y2, int z2);
	void SetSeeds2D(int x1, int y1, int x2, int y2);

	// 3차원 시드로 부터 canal 영역을 찾는 함수
	void Do(unsigned char** ppOrigOutCanal, bool bSmooth = true);
	void Do(std::vector<glm::vec3>* pvOrigOutCanal, bool bSmooth = true);

	// 3차원 시드의 z좌표를 반환함
	void GetSeedSlice(int* startslice, int* endslice);
	// release
	void Release();

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

	//(seed1 - 6.5mm, seed2 + 6.5mm)를 크기로 하는 부분 볼륨
	int m_nPartWidth;
	int m_nPartHeight;
	int m_nPartDepth;
	int m_nPartWH;
	usvoltype** m_ppPartVolData;

	unsigned char**	m_ppPartCanalFeature;

	TPoint3D<int> m_nPartSeed1;
	TPoint3D<int> m_nPartSeed2;

	// 시드 1, 2
	TPoint3D<int> m_pntSeed1;
	TPoint3D<int> m_pntSeed2;

	// 시드 1, 2
	TPoint3D<int> m_pntOrigSeed1;
	TPoint3D<int> m_pntOrigSeed2;

	int m_nScale;
	int m_nOrigKernelSize;
	int m_nKernelSize;
	usvoltype m_shVolThresh;

	// pixel size
	float m_fPixelSize;

	usvoltype m_thdBone;

	float sagittalCanalFeature(const glm::vec3 & point, const int & Nangle, const int & detectRadius,
							   const int & volMaskWidth, const std::vector<std::vector<unsigned char>>& volMask);
	float sphereCanalFeature(const glm::vec3 & point, const int & Nangle, const int & outerRadius, const int& innerRadius,
							 const int & volMaskWidth, const std::vector<std::vector<unsigned char>>& volMask);

	//thyoo. canal영역을 찾기 위한 전처리 작업
	void postProcessing();
	void postProcessing2();
	void postProcessing3();

	//thyoo. 2차원 시드 점을 받아서 3차원 시드 좌표를 찾는 함수.
	void detectSeedCoordZ(int x1, int y1, int x2, int y2, int* d_z1, int* d_z2);

	// path로 부터 canal 영역을 채워주는 함수들
	void GetKernelMask(int kernel, unsigned char** ppKernelMask);
	void ExpandPath(int w, int kernel, unsigned char** ppKernelMask,
					std::vector<int>* vecpathxy, std::vector<int>* vecpathz, unsigned char** ppExpandMask);
};
