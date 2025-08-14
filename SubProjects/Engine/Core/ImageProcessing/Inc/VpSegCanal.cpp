#include "VpSegCanal.h"

#include "../../../Common/Common/W3Memory.h"
#include "VpPathTracking.h"
#include "VpThreshold.h"
#include "VpImageAPI.inl"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVpSegCanal::CVpSegCanal()
	: m_nWidth(0), m_nHeight(0), m_nDepth(0), m_nWH(0), m_ppVolData(NULL) {
	memset(&m_pntOrigStart, 0, sizeof(TPoint3D<int>));
	memset(&m_pntOrigEnd, 0, sizeof(TPoint3D<int>));
	memset(&m_pntStart, 0, sizeof(TPoint3D<int>));
	memset(&m_pntEnd, 0, sizeof(TPoint3D<int>));

	m_nOrigWidth = 0;
	m_nOrigHeight = 0;
	m_nOrigDepth = 0;
	m_nOrigWH = 0;
	m_ppOrigVolData = NULL;

	m_nScale = 1;
	m_shVolThresh = 0;

	m_nOrigKernelSize = 0;
	m_nKernelSize = 0;

	m_fPixelSize = .0f;
}

CVpSegCanal::~CVpSegCanal() {}

// release
void CVpSegCanal::Release() {
	SAFE_DELETE_VOLUME(m_ppVolData, m_nDepth);
}

// 초기화 함수
void CVpSegCanal::Init(int w, int h, int d, usvoltype** ppVolData, float pixelsize, usvoltype thresh) {
	// 오리지날 볼륨 정보 저장
	m_nOrigWidth = w;
	m_nOrigHeight = h;
	m_nOrigDepth = d;
	m_nOrigWH = w * h;
	m_ppOrigVolData = ppVolData;
	m_shVolThresh = thresh;
	m_fPixelSize = pixelsize;

	// pixel size를 통해 canal의 크기 설정하고 kernel사이즈로 설정함
	m_nKernelSize = Max(3, ((int)(DEF_KERNEL / (pixelsize*10.0f)) + 1));

	// kernal 사이즈는 홀수로 셋팅
	if ((m_nKernelSize % 2) == 0) ++m_nKernelSize;

	// pixel size가 0.3이하면 다운 샘플링을 하고
	if (pixelsize < DEF_THRESH_PIXELSIZE) {
		m_nScale = kDownScale;
		CImage3D<usvoltype> shimg3d;
		shimg3d.Set(m_nOrigWidth, m_nOrigHeight, m_nOrigDepth, m_ppOrigVolData);

		// 다운 샘플링하고 진행
		m_ppVolData = shimg3d.ExportScale(m_nScale, m_nScale, m_nScale, &m_nWidth, &m_nHeight, &m_nDepth);
		m_nWH = m_nWidth * m_nHeight;
		m_nOrigKernelSize = DEF_KERNEL;
	}
	// pixel size가 0.3보다 크면 원래 크기로 진행한다.
	else {
		m_nScale = 1;
		m_nWidth = m_nOrigWidth;
		m_nHeight = m_nOrigHeight;
		m_nDepth = m_nOrigDepth;
		m_nWH = m_nOrigWH;

		// 원래 크기로 진행
		SafeNew2D(m_ppVolData, m_nWH, m_nDepth);
		for (int z = 0; z < m_nDepth; z++) {
			memcpy(m_ppVolData[z], m_ppOrigVolData[z], sizeof(usvoltype)*m_nWH);
		}

		m_nOrigKernelSize = m_nKernelSize;
	}
}

// 파노라마 2차원 시드를 설정하는 함수
void CVpSegCanal::SetSeeds2D(int startx, int starty, int endx, int endy) {
	m_pntOrigStart.x = startx;
	m_pntOrigStart.y = starty;
	m_pntOrigStart.z = -1;

	m_pntOrigEnd.x = endx;
	m_pntOrigEnd.y = endy;
	m_pntOrigEnd.z = -1;

	if (m_nScale == 1) {
		memcpy(&m_pntStart, &m_pntOrigStart, sizeof(TPoint3D<int>));
		memcpy(&m_pntEnd, &m_pntOrigEnd, sizeof(TPoint3D<int>));
	} else {
		m_pntStart.x = (int)(m_pntOrigStart.x / (float)m_nScale + .5f);
		m_pntStart.y = (int)(m_pntOrigStart.y / (float)m_nScale + .5f);
		m_pntStart.z = -1;

		m_pntEnd.x = (int)(m_pntOrigEnd.x / (float)m_nScale + .5f);
		m_pntEnd.y = (int)(m_pntOrigEnd.y / (float)m_nScale + .5f);
		m_pntEnd.z = -1;
	}
}

// 3차원 복셀값 시드를 설정하는 함수
void CVpSegCanal::SetSeeds3D(int startx, int starty, int startz, int endx, int endy, int endz) {
	m_pntOrigStart.x = startx;
	m_pntOrigStart.y = starty;
	m_pntOrigStart.z = startz;

	m_pntOrigEnd.x = endx;
	m_pntOrigEnd.y = endy;
	m_pntOrigEnd.z = endz;

	if (m_nScale == 1) {
		memcpy(&m_pntStart, &m_pntOrigStart, sizeof(TPoint3D<int>));
		memcpy(&m_pntEnd, &m_pntOrigEnd, sizeof(TPoint3D<int>));
	} else {
		m_pntStart.x = (int)(m_pntOrigStart.x / (float)m_nScale + .5f);
		m_pntStart.y = (int)(m_pntOrigStart.y / (float)m_nScale + .5f);
		m_pntStart.z = (int)(m_pntOrigStart.z / (float)m_nScale + .5f);

		m_pntEnd.x = (int)(m_pntOrigEnd.x / (float)m_nScale + .5f);
		m_pntEnd.y = (int)(m_pntOrigEnd.y / (float)m_nScale + .5f);
		m_pntEnd.z = (int)(m_pntOrigEnd.z / (float)m_nScale + .5f);
	}
}

// kernel size의 sphere를 저장하는 마스크
// sphere영역은 255 그 밖에 영역은 0 저장
void CVpSegCanal::GetKernelMask(int kernel, unsigned char** ppKernelMask) {
	// fill mask
	int nRadius = (kernel >> 1);
	int r2 = nRadius * nRadius;
	int nKernelCnt = 0;

	//#pragma omp parallel for
	for (int rz = 0; rz < kernel; rz++) {
		int z2 = (rz - nRadius)*(rz - nRadius);
		for (int ry = 0; ry < kernel; ry++) {
			int y2z2 = (ry - nRadius)*(ry - nRadius) + z2;
			for (int rx = 0; rx < kernel; rx++) {
				int x2y2z2 = (rx - nRadius)*(rx - nRadius) + y2z2;
				if (x2y2z2 <= r2) {
					ppKernelMask[rz][rx + ry * kernel] = 255;
					++nKernelCnt;
				}
			}
		}
	}
}

// path를 tracking하며 kernel mask를 이용하여 ppExpandMask에 sphere를 그려줌
void CVpSegCanal::ExpandPath(int w, int kernel, unsigned char** ppKernelMask, std::vector<int>* vecpathxy, std::vector<int>* vecpathz, unsigned char** ppExpandMask) {
	int nRadius = (kernel >> 1);
	int nSize = (int)vecpathxy->size();
	for (int n = 0; n < nSize; n++) {
		int x = 0, y = 0, z = 0;
		GetIdx2DTo3D(vecpathxy->at(n), vecpathz->at(n), w, &x, &y, &z);

		//#pragma omp parallel for
		for (int n = -nRadius; n <= nRadius; n++) {
			int nTempZ = z + n;
			int nMaskZ = n + nRadius;
			for (int m = -nRadius; m <= nRadius; m++) {
				int nTempY = y + m;
				int nMaskY = m + nRadius;
				for (int l = -nRadius; l <= nRadius; l++) {
					int nTempX = x + l;
					int nMaskX = l + nRadius;
					if (ppKernelMask[nMaskZ][nMaskX + nMaskY * kernel]) {
						ppExpandMask[nTempZ][nTempX + nTempY * w] = 255;
					}
				}
			}
		}
	}
}

// canal을 찾는 함수
// ppOrigOutCanal : canal 영역에 255 저장 나머지는 0
// bSmooth : path smoothing 여부. default는 true
// fEnhance : 뼈 영역만 강조할때 사용하는 factor로 default는 0.2
void CVpSegCanal::Do(unsigned char** ppOrigOutCanal, bool bSmooth, float fEnhance) {
	// 뼈 영역만 강조
	usvoltype threshmax = USHRT_MAX;
	if (fEnhance > .01f) {
		unsigned char** ppThresh = NULL;
		SafeNew2D(ppThresh, m_nWH, m_nDepth);

		// 1. otsu thresh 값을 구함
		CVpThreshold thresh;
		thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
		threshmax = thresh.OtsuEx(ppThresh, NULL);

		// 2. enhance only bone region
		// otsu threshold로 대강의 뼈 영역을 구하고 뼈 영역만 다시 enhance함
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				//  뼈 영역이면 더 밝게하고
				if (ppThresh[z][xy]) {
					m_ppVolData[z][xy] = (usvoltype)(m_ppVolData[z][xy] * (1 + fEnhance) + .5f);
				}
				// 아니면 더 흐리게 하여 뼈와 뼈가 아닌 부분의 밝기값 차이를 크게 함
				else {
					m_ppVolData[z][xy] = (usvoltype)(m_ppVolData[z][xy] * (1 - fEnhance) + .5f);
				}
			}
		}
		SAFE_DELETE_VOLUME(ppThresh, m_nDepth);
	}

	// dijkstra 를 이용하여 두 시드 사이의 path를 구함
	std::vector<int> vecpathxy;
	std::vector<int> vecpathz;
	CVpPathTracking dijks;
	dijks.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData, CVpPathTracking::DIJKSTRA, m_nKernelSize);
	dijks.SetPoints(m_pntStart.x, m_pntStart.y, m_pntStart.z, m_pntEnd.x, m_pntEnd.y, m_pntEnd.z);
	dijks.Do(&vecpathxy, &vecpathz);
	dijks.Release();

	// 다운 샘플링이 되었을 경우 path의 원래 좌표를 구함
	int n = (int)vecpathxy.size();
	for (int i = 0; i < n; i++) {
		int xy = vecpathxy[i];
		int z = vecpathz[i];
		TPoint3D<int> tmppnt;
		GetIdx2DTo3D(xy, z, m_nWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);
		vecpathxy[i] = (tmppnt.x + tmppnt.y*m_nOrigWidth)*m_nScale;
		vecpathz[i] = z * m_nScale;
	}

	// curve로 smoothing함
	if (bSmooth) {
		float offsett = m_fPixelSize / 5.0f;
		int sampling = (int)(1.0f / offsett + .5f);
		CVpPathTracking curvesmoothing;
		curvesmoothing.Curve(m_nOrigWidth, &vecpathxy, &vecpathz, sampling, offsett, CVpPathTracking::CRCURVE);
	}

	// path를 tracking하며 sphere를 그려줌
	unsigned char** ppKernelMask = NULL;
	SafeNew2D(ppKernelMask, m_nOrigKernelSize*m_nOrigKernelSize, m_nOrigKernelSize);
	GetKernelMask(m_nOrigKernelSize, ppKernelMask);
	ExpandPath(m_nOrigWidth, m_nOrigKernelSize, ppKernelMask, &vecpathxy, &vecpathz, ppOrigOutCanal);

	SAFE_DELETE_VOLUME(ppKernelMask, m_nOrigKernelSize);
}

void CVpSegCanal::Do(std::vector<glm::vec3>* pvOrigOutCanal, bool bSmooth, float fEnhance) {
	// 뼈 영역만 강조
	usvoltype threshmax = SHRT_MAX;
	if (fEnhance > .01f) {
		unsigned char** ppThresh = NULL;
		SafeNew2D(ppThresh, m_nWH, m_nDepth);

		// 1. otsu thresh 값을 구함
		CVpThreshold thresh;
		thresh.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData);
		threshmax = thresh.OtsuEx(ppThresh, NULL);	// ppThresh: otsu threshold 보다 높은 값은 255, 아닌 것은 0

		// 2. enhance only bone region
		// otsu threshold로 대강의 뼈 영역을 구하고 뼈 영역만 다시 enhance함: 단순 값을 올리고 내리기
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				//  뼈 영역이면 더 밝게하고
				if (ppThresh[z][xy]) {
					m_ppVolData[z][xy] = (usvoltype)(m_ppVolData[z][xy] * (1 + fEnhance) + .5f);
				}
				// 아니면 더 흐리게 하여 뼈와 뼈가 아닌 부분의 밝기값 차이를 크게 함
				else {
					m_ppVolData[z][xy] = (usvoltype)(m_ppVolData[z][xy] * (1 - fEnhance) + .5f);
				}
			}
		}
		SAFE_DELETE_VOLUME(ppThresh, m_nDepth);
	}

	// dijkstra 를 이용하여 두 시드 사이의 path를 구함
	std::vector<int> vecpathxy;
	std::vector<int> vecpathz;
	CVpPathTracking dijks;
	dijks.Init(m_nWidth, m_nHeight, m_nDepth, m_ppVolData, CVpPathTracking::DIJKSTRA, m_nKernelSize); // m_ppVolData : Bone intensity 가 커진 volume

	printf("---- Init finished\n");

	dijks.SetPoints(m_pntStart.x, m_pntStart.y, m_pntStart.z, m_pntEnd.x, m_pntEnd.y, m_pntEnd.z);

	printf("---- SetPoints finished\n");

	dijks.Do(&vecpathxy, &vecpathz);

	printf("---- Do finished\n");

	// 다운 샘플링이 되었을 경우 path의 원래 좌표를 구함
	int n = (int)vecpathxy.size();
	int step = std::max((n - 1) / 3, 1);

	//printf("step: %d\n", step);

	//if (step == 1)
	//{
	//	vecpathxy.clear();
	//	vecpathz.clear();

	//	dijks.Do(&vecpathxy, &vecpathz);
	//}

	dijks.Release();

	printf("---- Release finished\n");

	for (int i = 0; i < n - step; i += step) {
		int xy = vecpathxy[i];
		int z = vecpathz[i];
		TPoint3D<int> tmppnt;
		GetIdx2DTo3D(xy, z, m_nWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);

		pvOrigOutCanal->push_back(glm::vec3(tmppnt.x*m_nScale, tmppnt.y*m_nScale, tmppnt.z*m_nScale));

		//vecpathxy[i] = (tmppnt.x + tmppnt.y*m_nOrigWidth)*m_nScale;
		//vecpathz[i] = z*m_nScale;
	}

	int xy = vecpathxy[n - 1];
	int z = vecpathz[n - 1];
	TPoint3D<int> tmppnt;
	GetIdx2DTo3D(xy, z, m_nWidth, &tmppnt.x, &tmppnt.y, &tmppnt.z);

	pvOrigOutCanal->push_back(glm::vec3(tmppnt.x*m_nScale, tmppnt.y*m_nScale, tmppnt.z*m_nScale));

	// curve로 smoothing함
	//if (bSmooth)
	//{
	//	float offsett = m_fPixelSize / 5.0f;
	//	int sampling = (int)(1.0f / offsett + .5f);
	//	CVpPathTrackingCanal curvesmoothing;
	//	curvesmoothing.Curve(m_nOrigWidth, &vecpathxy, &vecpathz, sampling, offsett, CVpPathTrackingCanal::CRCURVE);
	//}

	// path를 tracking하며 sphere를 그려줌
	//unsigned char** ppKernelMask = NULL;
	//SafeNew2D(ppKernelMask, m_nOrigKernelSize*m_nOrigKernelSize, m_nOrigKernelSize);
	//GetKernelMask(m_nOrigKernelSize, ppKernelMask);
	//ExpandPath(m_nOrigWidth, m_nOrigKernelSize, ppKernelMask, &vecpathxy, &vecpathz, ppOrigOutCanal);
	//SafeDelete2D(ppKernelMask, m_nOrigKernelSize*m_nOrigKernelSize, m_nOrigKernelSize);
}

// 파노라마 시드로 부터 canal을 찾음
void CVpSegCanal::DoFromPano(unsigned char** ppOutCanal, bool bSmooth, float fEnhance) {
	// 2차원 시드점을 이용하여 3차원의 z좌표 를 찾음
	DetectSeedSliceEx(m_pntStart.x, m_pntStart.y, m_pntEnd.x, m_pntEnd.y, &m_pntStart.z, &m_pntEnd.z);

	// 3차원 시드점으로 canal을 찾음
	Do(ppOutCanal, bSmooth, fEnhance);
}

void CVpSegCanal::DoFromPano(std::vector<glm::vec3>* pvOutCanal, bool bSmooth, float fEnhance) {
	// 2차원 시드점을 이용하여 3차원의 z좌표 를 찾음
	DetectSeedSliceEx(m_pntStart.x, m_pntStart.y, m_pntEnd.x, m_pntEnd.y, &m_pntStart.z, &m_pntEnd.z);

	// 3차원 시드점으로 canal을 찾음
	Do(pvOutCanal, bSmooth, fEnhance);
}

// 3차원 시드의 z좌표를 반환함
void CVpSegCanal::GetSeedSlice(int* startslice, int* endslice) {
	if (m_nScale == 1) {
		*startslice = m_pntStart.z;
		*endslice = m_pntEnd.z;
	} else {
		*startslice = m_pntStart.z * m_nScale;
		*endslice = m_pntEnd.z * m_nScale;
	}
}

// 3차원 시드의 z좌표를 찾는 함수
void CVpSegCanal::DetectSeedSliceEx(int seed1x, int seed1y, int seed2x, int seed2y, int* slice1, int* slice2) {
	// 1. 대강의 뼈 영역을 찾고 canal이 존재할 슬라이스(z좌표)의 대강의 영역을 찾음
	// 입력 볼륨
	usvoltype** ppInputVol = m_ppVolData;

	// 1.1 canal이 존재하는걸로 예상되는 대강의 초기영역 설정
	int midz = m_nDepth >> 1;
	int stepz = m_nDepth >> 3;
	int startz = stepz;
	int endz = m_nDepth - stepz;

	usvoltype threshmin = 300;
	usvoltype threshmax = 1500;

	// 1.2 local otsu thresh
	CImage3D<usvoltype> orig3d;
	orig3d.Set(m_nWidth, m_nHeight, m_nDepth, ppInputVol);

	// 시드1 점을 중심으로 일정 크기의 부분 볼륨을 잘라내고 그 안에서 threshold를 찾아 뼈 영역을 찾음
	CVpThreshold thresh;
	int partsize = m_nKernelSize << 1;

	// 부분 볼륨의 영역을 저장하기 위한 변수 초기화
	TPoint3D<int> seed1min, seed1max;
	seed1min.x = Max(0, seed1x - partsize);
	seed1min.y = Max(0, seed1y - partsize);
	seed1min.z = startz;
	seed1max.x = Min(m_nWidth - 1, seed1x + partsize);
	seed1max.y = Min(m_nHeight - 1, seed1y + partsize);
	seed1max.z = endz;

	// 부분 볼륨의  크기를 구하고
	TPoint3D<int> seed1partsize;
	seed1partsize.x = seed1max.x - seed1min.x + 1;
	seed1partsize.y = seed1max.y - seed1min.y + 1;
	seed1partsize.z = seed1max.z - seed1min.z + 1;

	// 부분 볼륨을 추출한다.
	usvoltype** ppSeed1Part = orig3d.ExportPart(seed1min.x, seed1max.x, seed1min.y, seed1max.y, seed1min.z, seed1max.z);

	// 부분 볼륨으로 부터 threshold값을 구한다. 단 구하는 threshold는 영상의 특성상
	// 300보다 작으면 안되고 1500보다 커서는 안된다.
	thresh.Init(seed1partsize.x, seed1partsize.y, seed1partsize.z, ppSeed1Part);
	usvoltype threshseed1 = thresh.Otsu((unsigned char**)NULL);
	if (threshseed1 < threshmin)
		threshseed1 = threshmin;
	else if (threshseed1 > threshmax)
		threshseed1 = threshmax;

	SAFE_DELETE_VOLUME(ppSeed1Part, seed1partsize.z);

	// 시드2 점을 중심으로 일정 크기의 부분 볼륨을 잘라내고 그 안에서 threshold를 찾아 뼈 영역을 찾음
	// 위의 프로세스와 같다.

	// 부분 볼륨의 영역을 저장하기 위한 변수 초기화
	TPoint3D<int> seed2min, seed2max;
	seed2min.x = Max(0, seed2x - partsize);
	seed2min.y = Max(0, seed2y - partsize);
	seed2min.z = startz;
	seed2max.x = Min(m_nWidth - 1, seed2x + partsize);
	seed2max.y = Min(m_nHeight - 1, seed2y + partsize);
	seed2max.z = endz;

	// 부분 볼륨의  크기를 구하고
	TPoint3D<int> seed2partsize;
	seed2partsize.x = seed2max.x - seed2min.x + 1;
	seed2partsize.y = seed2max.y - seed2min.y + 1;
	seed2partsize.z = seed2max.z - seed2min.z + 1;

	// 부분 볼륨을 추출한다.
	usvoltype** ppSeed2Part = orig3d.ExportPart(seed2min.x, seed2max.x, seed2min.y, seed2max.y, seed2min.z, seed2max.z);

	// 부분 볼륨으로 부터 threshold값을 구한다. 단 구하는 threshold는 영상의 특성상
	// 300보다 작으면 안되고 1500보다 커서는 안된다.
	thresh.Init(seed2partsize.x, seed2partsize.y, seed2partsize.z, ppSeed2Part);
	usvoltype threshseed2 = thresh.Otsu((unsigned char**)NULL);
	if (threshseed2 < threshmin)
		threshseed2 = threshmin;
	else if (threshseed2 > threshmax)
		threshseed2 = threshmax;

	SAFE_DELETE_VOLUME(ppSeed2Part, seed2partsize.z);

	// 두 시드를 이용하여 canal의 2차원 방향 벡터를 구한다.
	float dirx = (float)(seed2x - seed1x);
	float diry = (float)(seed2y - seed1y);
	float dirlen = sqrt(dirx*dirx + diry * diry);
	dirx /= dirlen;
	diry /= dirlen;

	// canal의 2차원 방향 벡터와 수직인 벡터를 구한다.
	float perdirx = diry;
	float perdiry = -dirx;

	int curx = 0;
	int cury = 0;
	int nLenPerProfile1 = 0;
	int nLenPerProfile2 = 0;
	int nMaxLenPerProfile1 = 0;
	int nMaxLenPerProfile2 = 0;
	int margin = (m_nKernelSize << 1);
	int halfkernel = (m_nKernelSize + margin) >> 1;

	int tempslice1 = midz;
	int tempslice2 = midz;
	bool bIsDetected1 = false;
	bool bIsDetected2 = false;
	float minratio = .3f;

	// canal의 시드가 존재할것으로 예상되는 슬라이스의 범위 안에서 실제 시드의 z값을 찾는다.
	// canal을 찾는 방법은 시드가 되기 위한 조건을 만족하는 점 중에 가장 빈 공간이 큰 곳을 찾는다.
	// 빈 공간이 크다는 얘기는 시드간의 방향벡터와 수직인 방향으로 profile을 조사하여 뼈까지의 거리가 가장 긴
	// 슬라이스를 찾는다.
	for (int z = stepz; z < m_nDepth - stepz; z++) {
		bool bIsBone1Up = false;
		bool bIsBone1Dn = false;
		bool bIsBone2Up = false;
		bool bIsBone2Dn = false;

		// 시드간 벡터의 수직은 방향으로 뼈까지의 거리를 저장하는 변수
		nLenPerProfile1 = 0;
		nLenPerProfile2 = 0;

		// 시드점 조건을 만족하는지 저장
		bool bSeed1Validation = true;
		bool bSeed2Validation = true;

		int nProfileCnt = 3;
		int nHalfProfileCnt = nProfileCnt >> 1;

		// 각 슬라이스에서 2차원 시드 점으로 부터 시드간 벡터의 수직방향 위아래로 조사를 한다.
		// 시드점을 만족하는 조건은
		// 1. 우선 수직 위아래 방향으로 밝기값을 조사하는데 반드시 지정된 범위 안에뼈 영역이 있어야 하고
		// 2. 위 아래로 뼈 영역이 있는 슬라이스에서 길이가 가장 긴 점이 시드점이 되며
		// 3. 위 아래 방향으로의 뼈간 거리가 너무 다르면 안된다. (두 뼈까지의 거리가 비슷해야 한다.)
		//==========================================================================================================
		// seed1
		int nLenPerProfile1Up = 0;
		int nLenPerProfile1Dn = 0;
		for (int n = -nHalfProfileCnt; n <= nHalfProfileCnt; n++) {
			int tempseed1x = seed1x + (int)floor(dirx*n);
			int tempseed1y = seed1y + (int)floor(diry*n);

			//up
			for (int t = 0; t <= halfkernel; t++) {
				curx = tempseed1x + (int)floor(perdirx*t);
				cury = tempseed1y + (int)floor(perdiry*t);
				if (ppInputVol[z][curx + cury * m_nWidth] < threshseed1) {
					++nLenPerProfile1Up;
				} else {
					bIsBone1Up = true;
					break;
				}
			}
			// down
			for (int t = 0; t >= -halfkernel; t--) {
				curx = tempseed1x + (int)floor(perdirx*t);
				cury = tempseed1y + (int)floor(perdiry*t);
				if (ppInputVol[z][curx + cury * m_nWidth] < threshseed1) {
					++nLenPerProfile1Dn;
				} else {
					bIsBone1Dn = true;
					break;
				}
			}

			//=====================================================================================================
			// validation
			// 시드간 벡터의 수직 방향 양쪽으로 뼈 영역이 있어야 한다.
			if (!(bIsBone1Up && bIsBone1Dn)) { bSeed1Validation = false; break; }
			if ((nLenPerProfile1Up == 0) || (nLenPerProfile1Dn == 0)) { bSeed1Validation = false; break; }
			// 위 아래 방향으로 뼈간 거리가 비슷해야 한다.
			float ratio = (nLenPerProfile1Up < nLenPerProfile1Dn) ? (((float)nLenPerProfile1Up) / ((float)nLenPerProfile1Dn)) : (((float)nLenPerProfile1Dn) / ((float)nLenPerProfile1Up));
			if (ratio < minratio) { bSeed1Validation = false; break; }
		}
		nLenPerProfile1 = nLenPerProfile1Up + nLenPerProfile1Dn;

		// 시드 1에서의 프로세스와 동일하다
		//==========================================================================================================
		// seed2
		int nLenPerProfile2Up = 0;
		int nLenPerProfile2Dn = 0;
		for (int n = -nHalfProfileCnt; n <= nHalfProfileCnt; n++) {
			int tempseed2x = seed2x + (int)floor(dirx*n);
			int tempseed2y = seed2y + (int)floor(diry*n);

			//up
			for (int t = 0; t <= halfkernel; t++) {
				curx = tempseed2x + (int)floor(perdirx*t);
				cury = tempseed2y + (int)floor(perdiry*t);
				if (ppInputVol[z][curx + cury * m_nWidth] < threshseed2) {
					++nLenPerProfile2Up;
				} else {
					bIsBone2Up = true;
					break;
				}
			}
			// down
			for (int t = 0; t >= -halfkernel; t--) {
				curx = tempseed2x + (int)floor(perdirx*t);
				cury = tempseed2y + (int)floor(perdiry*t);
				if (ppInputVol[z][curx + cury * m_nWidth] < threshseed2) {
					++nLenPerProfile2Dn;
				} else {
					bIsBone2Dn = true;
					break;
				}
			}
			//=====================================================================================================
			// validation
			if (!(bIsBone2Up && bIsBone2Dn)) { bSeed2Validation = false; break; }
			if ((nLenPerProfile2Up == 0) || (nLenPerProfile2Dn == 0)) { bSeed2Validation = false; break; }
			float ratio = (nLenPerProfile2Up < nLenPerProfile2Dn) ? (((float)nLenPerProfile2Up) / ((float)nLenPerProfile2Dn)) : (((float)nLenPerProfile2Dn) / ((float)nLenPerProfile2Up));
			if (ratio < minratio) { bSeed2Validation = false; break; }
		}
		nLenPerProfile2 = nLenPerProfile2Up + nLenPerProfile2Dn;

		// 시드의 조건을 만족한 점들 중 뼈간 거리가 가장 긴 점을 찾아 시드로 설정한다.
		// check max profile
		if (bSeed1Validation && bIsBone1Up && bIsBone1Dn) {
			if (nMaxLenPerProfile1 < nLenPerProfile1) {
				nMaxLenPerProfile1 = nLenPerProfile1;
				tempslice1 = z;
				bIsDetected1 = true;
			}
		}
		if (bSeed2Validation && bIsBone2Up && bIsBone2Dn) {
			if (nMaxLenPerProfile2 < nLenPerProfile2) {
				nMaxLenPerProfile2 = nLenPerProfile2;
				tempslice2 = z;
				bIsDetected2 = true;
			}
		}
	}
	*slice1 = tempslice1;
	*slice2 = tempslice2;
}
