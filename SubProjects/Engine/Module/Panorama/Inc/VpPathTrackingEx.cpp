#include "VpPathTrackingEx.h"

#include <queue>

#include "../../../Common/Common/W3Memory.h"
#include "../../../Core/ImageProcessing/Inc/VpImageAPI.inl"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
CVpPathTrackingEx::CVpPathTrackingEx(void)
	: m_nWidth(0), m_nHeight(0), m_nWH(0), m_nDepth(0),
	m_ppVolume(NULL), m_nEdge(0), m_nNode(0), m_nScale(0) {
}

CVpPathTrackingEx::~CVpPathTrackingEx(void) {}

// release
void CVpPathTrackingEx::Release() {
	m_nWidth = 0;
	m_nHeight = 0;
	m_nWH = 0;
	m_ppVolume = NULL;

	m_nDepth = 0;
	m_nEdge = 0;
	m_nNode = 0;
	m_nKernelSize = 0;
	m_nScale = 0;

	m_nStartX = 0;
	m_nStartY = 0;
	m_nStartZ = 0;

	m_nEndX = 0;
	m_nEndY = 0;
	m_nEndZ = 0;
}

// 초기화 함수
bool CVpPathTrackingEx::Init(int w, int h, int d, usvoltype** ppVolume,
							 unsigned char** ppFeatureMap, EMethod emethod, int kernelsize) {
	m_nWidth = w;
	m_nHeight = h;
	m_nWH = w * h;
	m_nDepth = d;
	m_ppVolume = ppVolume;
	m_ppFeatureMap = ppFeatureMap;
	m_nKernelSize = kernelsize;
	m_eMethod = emethod;

	return false;
}

// 시드 설정 함수
void CVpPathTrackingEx::SetPoints(int startx, int starty, int startz,
								  int endx, int endy, int endz) {
	m_nStartX = startx;	m_nStartY = starty;	m_nStartZ = startz;
	m_nEndX = endx;	m_nEndY = endy;	m_nEndZ = endz;
}

// DIJKSTRA 실행
float CVpPathTrackingEx::Do(unsigned char** ppOutMask) {
	float distance = .0f;
	switch (m_eMethod) {
	case DIJKSTRA:
		distance = DoDijkstra(ppOutMask);
		break;

	default:
		break;
	}

	return distance;
}

// DIJKSTRA 실행
// pvecPathxy : path의 x, y 좌표 std::vector에 저장 (x, y index로 저장. 즉 x, y는 x + y*width 로 저장)
// pvecPathz : path의 z 좌표 std::vector에 저장
float CVpPathTrackingEx::Do(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz) {
	float distance = .0f;
	switch (m_eMethod) {
	case DIJKSTRA:
		// 실제로 DIJKSTRA 실행
		distance = DoDijkstra(pvecPathxy, pvecPathz);
		break;

	default:
		break;
	}

	return distance;
}

#define	PP	std::pair<int, float>
typedef struct _pri {
	int operator() (const std::pair<int, float>&p1, const std::pair<int, float>&p2) {
		return p1.second > p2.second;
	}
}PRI;

// DIJKSTRA 실행
// ppOutMaskd에 path인 부분만 255로 저장
float CVpPathTrackingEx::DoDijkstra(unsigned char** ppOutMask) {
	std::vector<int> vecpathxy;
	std::vector<int> vecpathz;
	//
	float fPathDist = DoDijkstra(&vecpathxy, &vecpathz);
	//
	int nSize = (int)vecpathxy.size();
	for (int n = 0; n < nSize; n++) {
		ppOutMask[vecpathz[n]][vecpathxy[n]] = 255;
	}

	return fPathDist;
}

// 실제로 DIJKSTRA 실행하는 함수
float CVpPathTrackingEx::DoDijkstra(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz) {
	int sourcexyz = m_nStartX + m_nStartY * m_nWidth + m_nStartZ * m_nWH;
	int sourcexy = m_nStartX + m_nStartY * m_nWidth;
	int sourcex = m_nStartX;
	int sourcey = m_nStartY;
	int sourcez = m_nStartZ;

	int targetxyz = m_nEndX + m_nEndY * m_nWidth + m_nEndZ * m_nWH;
	int targetxy = m_nEndX + m_nEndY * m_nWidth;
	int targetx = m_nEndX;
	int targety = m_nEndY;
	int targetz = m_nEndZ;

	// DIJKSTRA에 사용되는 임시 변수들
	float** dist = NULL;	// 최단 거리 저장변수
	int** prevxy = NULL;	// 이전 복셀 저장 (xy좌표)
	short** prevz = NULL;	// 이전 복셀 저장 (z좌표)
	bool** selected = NULL;	// 한번 체크했던 복셀여부 저장

	// 변수들 2차원 배열로 메모리 할당
	SafeNew2D(dist, m_nWH, m_nDepth);
	SafeNew2D(prevxy, m_nWH, m_nDepth);
	SafeNew2D(prevz, m_nWH, m_nDepth);
	SafeNew2D(selected, m_nWH, m_nDepth);

	// DIJKSTRA에 사용되는 임시 변수들 초기화
	for (int z = 0; z < m_nDepth; z++) {
		for (int xy = 0; xy < m_nWH; xy++) {
			dist[z][xy] = FLT_MAX;
			prevxy[z][xy] = -1;
			prevz[z][xy] = -1;
		}
	}
	int startxyz = sourcexyz;
	int startxy = sourcexy;
	int startx = sourcex;
	int starty = sourcey;
	int startz = sourcez;
	selected[startz][startxy] = true;
	dist[startz][startxy] = .0f;

	int nRadius = m_nKernelSize >> 1;

	// 비어있는 priority 큐에 시드1을 push함
	std::priority_queue<PP, std::vector<PP>, PRI>	q;
	q.push(PP(startxyz, dist[startz][startxy]));

	// 현재 복셀의 이웃을 참조하기 위한 오프셋
	int neiborcnt = 26;
	TPoint3D<int> neighbordist[26];
	neighbordist[0].x = -1;	neighbordist[0].y = -1;	neighbordist[0].z = -1;
	neighbordist[1].x = 0;	neighbordist[1].y = -1;	neighbordist[1].z = -1;
	neighbordist[2].x = 1;	neighbordist[2].y = -1;	neighbordist[2].z = -1;

	neighbordist[3].x = -1;	neighbordist[3].y = 0;	neighbordist[3].z = -1;
	neighbordist[4].x = 0;	neighbordist[4].y = 0;	neighbordist[4].z = -1;
	neighbordist[5].x = 1;	neighbordist[5].y = 0;	neighbordist[5].z = -1;

	neighbordist[6].x = -1;	neighbordist[6].y = 1;	neighbordist[6].z = -1;
	neighbordist[7].x = 0;	neighbordist[7].y = 1;	neighbordist[7].z = -1;
	neighbordist[8].x = 1;	neighbordist[8].y = 1;	neighbordist[8].z = -1;

	neighbordist[9].x = -1;	neighbordist[9].y = -1;	neighbordist[9].z = 0;
	neighbordist[10].x = 0;	neighbordist[10].y = -1; neighbordist[10].z = 0;
	neighbordist[11].x = 1;	neighbordist[11].y = -1; neighbordist[11].z = 0;

	neighbordist[12].x = -1; neighbordist[12].y = 0;	neighbordist[12].z = 0;
	neighbordist[13].x = 1;	neighbordist[13].y = 0;	neighbordist[13].z = 0;

	neighbordist[14].x = -1; neighbordist[14].y = 1;	neighbordist[14].z = 0;
	neighbordist[15].x = 0;	neighbordist[15].y = 1;	neighbordist[15].z = 0;
	neighbordist[16].x = 1;	neighbordist[16].y = 1;	neighbordist[16].z = 0;

	neighbordist[17].x = -1; neighbordist[17].y = -1; neighbordist[17].z = 1;
	neighbordist[18].x = 0;	neighbordist[18].y = -1; neighbordist[18].z = 1;
	neighbordist[19].x = 1;	neighbordist[19].y = -1; neighbordist[19].z = 1;

	neighbordist[20].x = -1; neighbordist[20].y = 0;	neighbordist[20].z = 1;
	neighbordist[21].x = 0;	neighbordist[21].y = 0;	neighbordist[21].z = 1;
	neighbordist[22].x = 1;	neighbordist[22].y = 0;	neighbordist[22].z = 1;

	neighbordist[23].x = -1; neighbordist[23].y = 1;	neighbordist[23].z = 1;
	neighbordist[24].x = 0;	neighbordist[24].y = 1;	neighbordist[24].z = 1;
	neighbordist[25].x = 1;	neighbordist[25].y = 1;	neighbordist[25].z = 1;

	// 각 이웃까지의 노말벡터 미리 저장. 나중에 현재 복셀과 이웃까지의 direction 구할때 사용
	TPoint3D<float> neighnor[26];
	for (int n = 0; n < neiborcnt; n++) {
		float fneighlen = (float)sqrt((float)(neighbordist[n].x*neighbordist[n].x + neighbordist[n].y*neighbordist[n].y + neighbordist[n].z*neighbordist[n].z));
		neighnor[n].x = neighbordist[n].x / fneighlen;
		neighnor[n].y = neighbordist[n].y / fneighlen;
		neighnor[n].z = neighbordist[n].z / fneighlen;
	}

	// 시드1에서 시드2로 가는 방향 벡터를 미리 계산. canal은 이 방향일거라고 가정하고 움직이게 된다.
	// 실제로 dijkstra의 energy function에서 direction이라는 feature에 큰 weight로 사용된다.
	TPoint3D<float> directionvec;
	directionvec.x = (float)(m_nEndX - m_nStartX);
	directionvec.y = (float)(m_nEndY - m_nStartY);
	directionvec.z = (float)(m_nEndZ - m_nStartZ);
	float flen = (float)sqrt(directionvec.x*directionvec.x + directionvec.y*directionvec.y + directionvec.z*directionvec.z);
	directionvec.x /= flen;
	directionvec.y /= flen;
	directionvec.z /= flen;

	// dijkstra energy function에 사용되는 feature들의 weight값 설정
	const float weightftr[ENERGYFTRMAX] = {
		.0f,	//AVGINTENSITY
		.0f, 	//AVGDISTINTENSITY
		.0f, 	//GRADIENT
		.0f,	//STD
		0.45f, 	//INTENSITY
		.25f, 	//DISTINTENSITY
		.2f, 	//DISTANCE
		.0f, 	//DISTANCEFROMSTART
		.0f 	//DIRECTION
	};

	// Que가 empty일때까지 que로부터 값을 하나씩 pop하여 최단거리를 계산
	while ((!q.empty()) && (selected[targetz][targetxy] == false)) {
		// 현재 큐에 저장되어 있는 복셀을 하나 꺼내오고
		// first : xyz 좌표 인덱스 값 (x + y*width + z*width*height)
		int curxyz = q.top().first;
		// second : energy 값
		float curcost = q.top().second;
		q.pop();

		// first 값에서 x, y, z 좌표를 얻어오고
		int curx, cury, curz, curxy;
		GetIdx1DTo3D(curxyz, m_nWidth, m_nHeight, m_nDepth, &curx, &cury, &curz);
		curxy = curx + cury * m_nWidth;

		// 현재 복셀을 체크 했다고 저장
		selected[curz][curxy] = true;

		// 범위를 넘어가면 밑에는 실행하지 말고 다음 복셀로 넘어간다.
		if ((curx < nRadius + 1) || (curx > m_nWidth - nRadius - 2) ||
			(cury < nRadius + 1) || (cury > m_nHeight - nRadius - 2) ||
			(curz < nRadius + 1) || (curz > m_nDepth - nRadius - 2)) continue;

		// 현재 복셀의 이웃들을 검사한다.
		for (int i = 0; i < neiborcnt; i++) {
			int nz = curz + neighbordist[i].z;
			int ny = cury + neighbordist[i].y;
			int nx = curx + neighbordist[i].x;

			int nidxz = nz * m_nWH;
			int nidxy = ny * m_nWidth;
			int nxy = nx + nidxy;

			// 이미 체크를 한 복셀이면 넘어가고
			if (selected[nz][nxy]) continue;

			// energy function 값 계산
			float ftrval[ENERGYFTRMAX];
			memset(ftrval, 0, sizeof(float)*ENERGYFTRMAX);

			//// 현재 복셀과 이웃 복셀의 밝기값 차이
			ftrval[DISTINTENSITY] = std::fabsf(m_ppVolume[curz][curxy] - m_ppVolume[nz][nxy]);

			// 전처리에서 구한 feature map.
			ftrval[INTENSITY] = (float)(m_ppFeatureMap[curz][curxy] / 255);

			//현재 복셀 위치의 Sagittal 평면에서 주변 원 모양의 structure
			//if (nx % 20 == 0)
			//	ftrval[DISTINTENSITY] = sagittalCircleFeature(nx, ny, nz, nRadius, nRadius - 1);
			//else
			//	ftrval[DISTINTENSITY] = 1.0f;

			// 현재 복셀과 이웃 복셀의 거리
			ftrval[DISTANCE] = (float)sqrt((double)(curx - nx)*(curx - nx) + (cury - ny)*(cury - ny) + (curz - nz)*(curz - nz));

			// 현재 복셀과 이웃복셀간의 벡터 와 시드간 방향벡터의 내적을 이용하여 0~1까지의 값으로 정규화함
			// 이 값이 0 : 두 벡터의 방향이 같다
			// 이 값이 1 : 두 벡터의 방향이 반대다
			// dijkstra는 energy 값이 작은 방향으로 가기 때문에 두 벡터의 방향이 같은쪽으로 향하려는 성질을 갖는다.
			//ftrval[DIRECTION] = (1.0f - (neighnor[i].x*directionvec.x + neighnor[i].y*directionvec.y + neighnor[i].z*directionvec.z)) / 2.0f; // 1 ~ 0

			// cost (energy) 값 계산
			float cost = .0f;
			for (int n = 0; n < ENERGYFTRMAX; n++) cost += (weightftr[n] * ftrval[n]);

			// 현재 cost와 이웃까지의 cost를 합함
			float d = curcost + cost;

			// 현재 이웃의 cost보다 작으면 저장하고 que에 저장함
			if (d < dist[nz][nxy]) {
				dist[nz][nxy] = d;
				prevxy[nz][nxy] = curxy;
				prevz[nz][nxy] = curz;
				q.push(PP(nxy + nidxz, d));
			}
		}
	}
	float fPathDist = dist[targetz][targetxy];

	SAFE_DELETE_VOLUME(dist, m_nDepth);
	SAFE_DELETE_VOLUME(selected, m_nDepth);

	startxy = targetxy;
	startz = targetz;

	// 위의 while문에서 저장한 prev배열로 부터 시드 두점 사이의 path를 구한다.
	// 시드2로부터 거꾸로 path를 탐색하여 시드1에 도착할때까지 pvecPath에 저장한다.
	while ((startxy != -1) && (startz != -1)) {
		pvecPathxy->push_back(startxy);
		pvecPathz->push_back(startz);

		int tempxy = startxy;
		int tempz = startz;
		startxy = prevxy[tempz][tempxy];
		startz = prevz[tempz][tempxy];
	}

	// pvecPath에 path가 거꾸로 저장되어 있기때문에 reverse하면 우리가 구하고자 하는 path가 구해진다.
	reverse(pvecPathxy->begin(), pvecPathxy->end());
	reverse(pvecPathz->begin(), pvecPathz->end());

	SAFE_DELETE_VOLUME(prevxy, m_nDepth);
	SAFE_DELETE_VOLUME(prevz, m_nDepth);

	return fPathDist;
}

// path로부터 샘플링된 복셀을 얻어 그 복셀들 사이를 CR curve로 smoothing한다.
bool CVpPathTrackingEx::Curve(std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett, ECurve eCurve) {
	return Curve(m_nWidth, pvecPathxy, pvecPathz, sampling, offsett, eCurve);
}

bool CVpPathTrackingEx::Curve(int w, std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett, ECurve eCurve) {
	bool bRet = false;
	switch (eCurve) {
	case CRCURVE:
		bRet = Curve_CRCurve(w, pvecPathxy, pvecPathz, sampling, offsett);
		break;

	default:
		break;
	}

	return bRet;
}

// 실제로 CR curve를 구하는 함수
bool CVpPathTrackingEx::Curve_CRCurve(int w, std::vector<int>* pvecPathxy, std::vector<int>* pvecPathz, int sampling, float offsett) {
	// 1. sampling path
	int nPntCnt = (int)pvecPathxy->size();
	std::vector<TPoint3D<int>> vecSample;
	for (int i = 0; i < nPntCnt; i += sampling) {
		TPoint3D<int> pnt3d;
		GetIdx2DTo3D(pvecPathxy->at(i), pvecPathz->at(i), w, &pnt3d.x, &pnt3d.y, &pnt3d.z);
		vecSample.push_back(pnt3d);
	}
	// add last
	TPoint3D<int> lastpnt3d;
	GetIdx2DTo3D(pvecPathxy->at(nPntCnt - 1), pvecPathz->at(nPntCnt - 1), w, &lastpnt3d.x, &lastpnt3d.y, &lastpnt3d.z);

	vecSample.push_back(lastpnt3d);

	int nSamplePntCntPP = (int)vecSample.size();
	if (nSamplePntCntPP < 4) return false;

	// 2. add start(1) and end(1) for curve
	TPoint3D<int> addstart, addend;
	addstart.x = vecSample[0].x + (vecSample[0].x - vecSample[1].x);
	addstart.y = vecSample[0].y + (vecSample[0].y - vecSample[1].y);
	addstart.z = vecSample[0].z + (vecSample[0].z - vecSample[1].z);

	addend.x = vecSample[nSamplePntCntPP - 1].x + (vecSample[nSamplePntCntPP - 1].x - vecSample[nSamplePntCntPP - 2].x);
	addend.y = vecSample[nSamplePntCntPP - 1].y + (vecSample[nSamplePntCntPP - 1].y - vecSample[nSamplePntCntPP - 2].y);
	addend.z = vecSample[nSamplePntCntPP - 1].z + (vecSample[nSamplePntCntPP - 1].z - vecSample[nSamplePntCntPP - 2].z);

	vecSample.insert(vecSample.begin(), addstart);
	vecSample.push_back(addend);
	nSamplePntCntPP += 2;

	pvecPathxy->clear();
	pvecPathz->clear();

	// 3. generate curve
	int nPntCntPP = (int)(1.0f / offsett + .5f);
	//#pragma omp parallel for
	for (int i = 1; i < nSamplePntCntPP - 2; i++) {
		for (int k = 0; k < nPntCntPP; k++) {
			float t = k * offsett;
			int xcr = (int)((vecSample[i].x + 0.5*t*(-vecSample[i - 1].x + vecSample[i + 1].x)
							 + t * t*(vecSample[i - 1].x - 2.5*vecSample[i].x + 2 * vecSample[i + 1].x - 0.5*vecSample[i + 2].x)
							 + t * t*t*(-0.5*vecSample[i - 1].x + 1.5*vecSample[i].x - 1.5*vecSample[i + 1].x + 0.5*vecSample[i + 2].x)) + .5f);
			int ycr = (int)((vecSample[i].y + 0.5*t*(-vecSample[i - 1].y + vecSample[i + 1].y)
							 + t * t*(vecSample[i - 1].y - 2.5*vecSample[i].y + 2 * vecSample[i + 1].y - 0.5*vecSample[i + 2].y)
							 + t * t*t*(-0.5*vecSample[i - 1].y + 1.5*vecSample[i].y - 1.5*vecSample[i + 1].y + 0.5*vecSample[i + 2].y)) + .5f);
			int zcr = (int)((vecSample[i].z + 0.5*t*(-vecSample[i - 1].z + vecSample[i + 1].z)
							 + t * t*(vecSample[i - 1].z - 2.5*vecSample[i].z + 2 * vecSample[i + 1].z - 0.5*vecSample[i + 2].z)
							 + t * t*t*(-0.5*vecSample[i - 1].z + 1.5*vecSample[i].z - 1.5*vecSample[i + 1].z + 0.5*vecSample[i + 2].z)) + .5f);

			pvecPathxy->push_back(xcr + ycr * w);
			pvecPathz->push_back(zcr);
		}
	}

	return true;
}
