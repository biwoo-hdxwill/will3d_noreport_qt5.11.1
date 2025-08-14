#pragma once
#include <vector>

#include "../../../Core/ImageProcessing/Inc/VpGlobalDefine.h"

#define	OFFSETDEPTH		10
#define MAXDIST			INT_MAX

// Dijkstra 알고리즘을 수행하는 클래스
class CVpPathTrackingEx {
public:
	// DIJKSTRA를 사용합니다.
	enum EMethod { DIJKSTRA, MAXMETHOD };

	// path를 구한후 path를 샘플링할때 curve를 사용합니다.
	enum ECurve { CRCURVE, MAXCURVE };

public:
	CVpPathTrackingEx(void);
	~CVpPathTrackingEx(void);

	// 초기화 함수
	bool Init(int w, int h, int d, usvoltype** ppVolume, unsigned char** ppFeatureMap, EMethod emethod, int kernelsize = 1);

	// 시드 설정함수
	void SetPoints(int startx, int starty, int startz, int endx, int endy, int endz);

	// DIJKSTRA 수행함수
	float Do(unsigned char** ppOutMask);
	float Do(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz);

	// 샘플링된 점을 curve로 smoothing 해주는 함수
	bool Curve(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett, ECurve eCurve);
	bool Curve(int w, std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett, ECurve eCurve);

	//release function
	void Release();

private:
	// DIJKSTRA 실행할때 사용하는 에너지 텀입니다. 실제로 사용하는 텀은
	// DISTINTENSITY, DISTANCE, DIRECTION 세가지 입니다.
	enum EEnergyFtr {
		AVGINTENSITY = 0,
		AVGDISTINTENSITY,
		GRADIENT,
		STD,
		INTENSITY,
		DISTINTENSITY,
		DISTANCE,
		DISTANCEFROMSTART,
		DIRECTION,
		ENERGYFTRMAX
	};

	// 입력영상 정보
	int m_nWidth;
	int m_nHeight;
	int m_nWH;
	int m_nDepth;
	usvoltype** m_ppVolume;

	// DIJKSTRA 에 사용되는 그래프의 엣지와 노드 수
	int m_nEdge;
	int m_nNode;

	int m_nKernelSize = 0;
	int m_nScale;

	// 시드 1
	int m_nStartX = 0;
	int m_nStartY = 0;
	int m_nStartZ = 0;

	// 시드 2
	int m_nEndX = 0;
	int m_nEndY = 0;
	int m_nEndZ = 0;

	EMethod m_eMethod = EMethod::DIJKSTRA;

	unsigned char** m_ppFeatureMap = nullptr;

	float DoDijkstra(unsigned char** ppOutMask);
	float DoDijkstra(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz);

	bool Curve_CRCurve(int w, std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett);
};
