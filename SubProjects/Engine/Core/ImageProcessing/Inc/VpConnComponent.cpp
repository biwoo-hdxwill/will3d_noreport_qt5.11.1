
// TODO: 오류많음, 새 코드로 교체하거나 opencv 사용

#include "VpConnComponent.h"

#include "../../../Common/Common/W3Memory.h"
#include "VpMorph.h"

namespace {
// area
bool CompareCompAreaDn(SCompInfo& a, SCompInfo& b) { return (a._nArea > b._nArea); }
bool CompareCompXCoordDn(SCompInfo& a, SCompInfo& b) { return (a._pntCenter.x > b._pntCenter.x); }
bool CompareCompYCoordDn(SCompInfo& a, SCompInfo& b) { return (a._pntCenter.y > b._pntCenter.y); }
bool CompareCompXCoordUp(SCompInfo& a, SCompInfo& b) { return (a._pntCenter.x < b._pntCenter.x); }
bool CompareCompYCoordUp(SCompInfo& a, SCompInfo& b) { return (a._pntCenter.y < b._pntCenter.y); }

bool CompareCompBndBox2DDn(SCompInfo& a, SCompInfo& b) {
	int aBBX = (a._pntMax.x - a._pntMin.x)*(a._pntMax.y - a._pntMin.y);
	int bBBX = (b._pntMax.x - b._pntMin.x)*(b._pntMax.y - b._pntMin.y);
	return (aBBX > bBBX);
}
bool CompareCompBndBox3DDn(SCompInfo& a, SCompInfo& b) {
	int aBBX = (a._pntMax.x - a._pntMin.x)*(a._pntMax.y - a._pntMin.y)*(a._pntMax.z - a._pntMin.z);
	int bBBX = (b._pntMax.x - b._pntMin.x)*(b._pntMax.y - b._pntMin.y)*(b._pntMax.z - b._pntMin.z);
	return (aBBX > bBBX);
}
bool CompareCompLength2DDn(SCompInfo& a, SCompInfo& b) {
	int alength = Max((a._pntMax.x - a._pntMin.x), (a._pntMax.y - a._pntMin.y));
	int blength = Max((b._pntMax.x - b._pntMin.x), (b._pntMax.y - b._pntMin.y));
	return (alength > blength);
}
bool CompareCompLength3DDn(SCompInfo& a, SCompInfo& b) {
	int alength = Max(Max((a._pntMax.x - a._pntMin.x), (a._pntMax.y - a._pntMin.y)), (a._pntMax.z - a._pntMin.z));
	int blength = Max(Max((b._pntMax.x - b._pntMin.x), (b._pntMax.y - b._pntMin.y)), (b._pntMax.z - b._pntMin.z));
	return (alength > blength);
}

bool CompareByteValueUp(conncomtype& a, conncomtype& b) { return (a < b); }
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVpConnComponent::CVpConnComponent()
	: m_ppComponent(NULL), m_pComponent(NULL), m_nWidth(0), m_nHeight(0), m_nDepth(0), m_bytExpValue(255) {}

CVpConnComponent::~CVpConnComponent() {}

bool CVpConnComponent::Create(int w, int h, int d) {
	Release();

	int wh = w * h;
	if (!SafeNew2D<conncomtype>(m_ppComponent, wh, d))		return false;
	for (int z = 0; z < d; z++) {
		memset(m_ppComponent[z], 0, sizeof(conncomtype)*wh);
	}

	m_nWidth = w;
	m_nHeight = h;
	m_nDepth = d;

	return true;
}

bool CVpConnComponent::Create(int w, int h) {
	Release();

	int wh = w * h;
	if (!SafeNews<conncomtype>(m_pComponent, wh))
		return false;	// ccl을 진행할 이미지 크기와 같은 label 맵 생성

	memset(m_pComponent, 0, sizeof(conncomtype)*wh);

	m_nWidth = w;
	m_nHeight = h;

	return true;
}

void CVpConnComponent::Release() {
	SAFE_DELETE_VOLUME(m_ppComponent, m_nDepth);
	SAFE_DELETE_ARRAY(m_pComponent);
	m_vecCompInfo.clear();

	m_nWidth = 0;
	m_nHeight = 0;
	m_nDepth = 0;
}

int CVpConnComponent::Do(unsigned char** ppVolData, int minsize) {
	if ((ppVolData == NULL) || (m_ppComponent == NULL)) return 0;

	int i, j, k;
	conncomtype nLabel = 1;
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;
	int nMaxUF = 1024 * 1024;
	conncomtype* pEQTbl = new conncomtype[nMaxUF];
	memset(pEQTbl, 0, sizeof(conncomtype)*nMaxUF);

	// 0. init data
	memset(ppVolData[0], 0, sizeof(unsigned char)*w*h);
	for (k = 0; k < d; k++) {
		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				if ((i == 0) || (j == 0)) {
					int idxxy = i + j * w;
					ppVolData[k][idxxy] = 0;
				}
			}
		}
	}

	// 1. first scan
	for (k = 1; k < d; k++) {
		for (j = 1; j < h; j++) {
			for (i = 1; i < w; i++) {
				int xy = i + j * w;
				unsigned char bytCenter = ppVolData[k][xy];

				unsigned char bytWest = ppVolData[k][xy - 1];
				unsigned char bytNorth = ppVolData[k][xy - w];
				unsigned char bytUp = ppVolData[k - 1][xy];

				conncomtype nWestLabel = m_ppComponent[k][xy - 1];
				conncomtype nNorthLabel = m_ppComponent[k][xy - w];
				conncomtype nUpLabel = m_ppComponent[k - 1][xy];

				std::vector<conncomtype> vecLabel;
				if (bytWest != 0) vecLabel.push_back(nWestLabel);
				if (bytNorth != 0) vecLabel.push_back(nNorthLabel);
				if (bytUp != 0) vecLabel.push_back(nUpLabel);
				int nSize = (int)vecLabel.size();
				if (nSize == 1) {
					m_ppComponent[k][xy] = vecLabel[0];
				} else if (nSize > 1) {
					std::sort(vecLabel.begin(), vecLabel.end(), CompareByteValueUp);
					conncomtype nSmallestLabel = vecLabel[0];
					while (pEQTbl[nSmallestLabel] != nSmallestLabel) {
						nSmallestLabel = pEQTbl[nSmallestLabel];
					}
					m_ppComponent[k][xy] = nSmallestLabel;
					for (int n = 0; n < nSize; n++) {
						pEQTbl[vecLabel[n]] = nSmallestLabel;
					}
				} else if (bytCenter != 0) {
					m_ppComponent[k][xy] = nLabel;
					pEQTbl[nLabel] = nLabel;
					++nLabel;
				}
			}
		}
	}

	// 2. second scan
	for (k = 0; k < d; k++) {
		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				int xy = i + j * w;
				conncomtype n = m_ppComponent[k][xy];
				if (n != 0) {
					while (pEQTbl[n] != n) n = pEQTbl[n];
					m_ppComponent[k][xy] = pEQTbl[n];
				}
			}
		}
	}
	delete[] pEQTbl;

	// 3. third scan
	int nSize;
	for (k = 0; k < d; k++) {
		for (j = 0; j < h; j++) {
			for (i = 0; i < w; i++) {
				int xy = i + j * w;
				if (m_ppComponent[k][xy] != 0) {
					int m = 0;
					bool bExist = false;
					nSize = (int)m_vecCompInfo.size();
					for (m = 0; m < nSize; m++) {
						if (m_ppComponent[k][xy] == m_vecCompInfo[m]._nLabel) {
							bExist = true;
							break;
						}
					}
					if (bExist) {
						++m_vecCompInfo[m]._nArea;
						int idxx, idxy, idxz;
						GetIdx2DTo3D(xy, k, m_nWidth, &idxx, &idxy, &idxz);
						m_vecCompInfo[m]._pntCenter.x += idxx;
						m_vecCompInfo[m]._pntCenter.y += idxy;
						m_vecCompInfo[m]._pntCenter.z += idxz;

						if (m_vecCompInfo[m]._pntMin.x > idxx) m_vecCompInfo[m]._pntMin.x = idxx;
						if (m_vecCompInfo[m]._pntMin.y > idxy) m_vecCompInfo[m]._pntMin.y = idxy;
						if (m_vecCompInfo[m]._pntMin.z > idxz) m_vecCompInfo[m]._pntMin.z = idxz;

						if (m_vecCompInfo[m]._pntMax.x < idxx) m_vecCompInfo[m]._pntMax.x = idxx;
						if (m_vecCompInfo[m]._pntMax.y < idxy) m_vecCompInfo[m]._pntMax.y = idxy;
						if (m_vecCompInfo[m]._pntMax.z < idxz) m_vecCompInfo[m]._pntMax.z = idxz;
					} else {
						SCompInfo temp;
						temp._nLabel = m_ppComponent[k][xy];
						temp._nArea = 1;
						temp._pntCenter.x = 0;
						temp._pntCenter.y = 0;
						temp._pntCenter.z = 0;

						temp._pntMin.x = INT_MAX;
						temp._pntMin.y = INT_MAX;
						temp._pntMin.z = INT_MAX;

						temp._pntMax.x = INT_MIN;
						temp._pntMax.y = INT_MIN;
						temp._pntMax.z = INT_MIN;

						m_vecCompInfo.push_back(temp);
					}
				}
			}
		}
	}

	// min size
	nSize = (int)m_vecCompInfo.size();
	if (minsize > 0) {
		std::vector<SCompInfo> vecTemp;
		for (int n = 0; n < nSize; n++) {
			if (m_vecCompInfo[n]._nArea > minsize) {
				vecTemp.push_back(m_vecCompInfo[n]);
			}
		}
		vecTemp.swap(m_vecCompInfo);
	}

	//
	// min size delete
	// 	vector<int>vecDeleteIdx;
	// 	for (int n = 0 ; n < nSize ; n++)
	// 	{
	// 		vecDeleteIdx.push_back(n);
	// 	}
	// 	LeaveOnly(vecDeleteIdx);
	//

	// calc. center points
	nSize = (int)m_vecCompInfo.size();
	for (int n = 0; n < nSize; n++) {
		if (m_vecCompInfo[n]._nArea) {
			m_vecCompInfo[n]._pntCenter.x /= m_vecCompInfo[n]._nArea;
			m_vecCompInfo[n]._pntCenter.y /= m_vecCompInfo[n]._nArea;
			m_vecCompInfo[n]._pntCenter.z /= m_vecCompInfo[n]._nArea;
		}
	}

	// sort by area
	sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompAreaDn);

	return (int)m_vecCompInfo.size();
}

int CVpConnComponent::Do(unsigned char* pImgData, int minsize) {
	// by jdk 170131 comments https://en.wikipedia.org/wiki/Connected-component_labeling

	if ((pImgData == NULL) || (m_pComponent == NULL)) return 0;

	int i, j;	// 2D 탐색을 위한 index
	conncomtype nLabel = 1;
	int w = m_nWidth;
	int h = m_nHeight;
	int nMaxUF = 1024 * 1024;
	conncomtype* pEQTbl = new conncomtype[nMaxUF];	// equivalence table
	memset(pEQTbl, 0, sizeof(conncomtype)*nMaxUF);

	// 0. init data
	memset(pImgData, 0, sizeof(unsigned char)*w);	// width 방향 첫 줄 0으로 채움
	for (j = 0; j < h; j++)	// height 방향 첫 줄 0으로 채움
	{
		int idx = 0 + j * w;
		pImgData[idx] = 0;
	}

	// 1. first scan
	for (j = 1; j < h; j++)	// height 방향 1부터 시작
	{
		for (i = 1; i < w; i++)	// width 방향 1부터 시작
		{
			// 현재 위치가 배경(0)이 아닐 경우 west 픽셀과 north 픽셀의 확인하여
			// 1. 둘 중 하나에 label 번호가 있을 경우 현재 픽셀에 같은 값을 부여함
			// 2. 두 픽셀 모두 label 번호가 있을 경우 작은 번호로 부여하고 2번 째 스캔 때 합침
			// 3. 두 픽셀 모두 label 번호가 없을(0 일) 경우 새로운 번호 부여
			int xy = i + j * w;
			unsigned char bytCenter = pImgData[xy]; // current 픽셀
			unsigned char bytWest = pImgData[xy - 1]; // west 픽셀
			unsigned char bytNorth = pImgData[xy - w]; // north 픽셀

			conncomtype nWestLabel = m_pComponent[xy - 1];
			conncomtype nNorthLabel = m_pComponent[xy - w];

			std::vector<conncomtype> vecLabel;
			if (bytWest != 0) vecLabel.push_back(nWestLabel);
			if (bytNorth != 0) vecLabel.push_back(nNorthLabel);
			int nSize = (int)vecLabel.size();
			if (nSize == 1)	// 1번의 경우
			{
				m_pComponent[xy] = vecLabel[0]; // 해당 픽셀의 번호 부여
			} else if (nSize > 1) // 2번의 경우
			{
				sort(vecLabel.begin(), vecLabel.end(), CompareByteValueUp);
				conncomtype nSmallestLabel = vecLabel[0]; // 둘 중 작은 label 번호
				while (pEQTbl[nSmallestLabel] != nSmallestLabel) // EQTbl index와 label번호가 다른 경우(EQTbl 번호가 한번 변경된 경우)
				{
					nSmallestLabel = pEQTbl[nSmallestLabel]; // west, north 중 작은 label 번호 대신 그 label의 EQTbl 값을 사용
				}
				m_pComponent[xy] = nSmallestLabel; // 현재 픽셀에 label 부여
				for (int n = 0; n < nSize; n++) {
					pEQTbl[vecLabel[n]] = nSmallestLabel; // west, north label의 EQTbl을 현제 픽셀에 부여한 label 값으로 변경
				}
			} else if (bytCenter != 0) // 3번의 경우
			{
				m_pComponent[xy] = nLabel;
				pEQTbl[nLabel] = nLabel;
				++nLabel;
			}
		}
	}

	// 2. second scan
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			int xy = i + j * w;
			conncomtype n = m_pComponent[xy];
			if (n != 0) {
				while (pEQTbl[n] != n) n = pEQTbl[n]; // ?
				m_pComponent[xy] = pEQTbl[n]; // EQTbl을 참고하여 연결된 component 들의 label 재정리
			}
		}
	}
	delete[] pEQTbl;

	// 3. third scan
	// SCompInfo 객체를 만들기 위한 작업 : area, min pos, max pos를 구하기 위해
	int nSize;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			int xy = i + j * w;
			if (m_pComponent[xy] != 0) {
				int m = 0;
				bool bExist = false;
				nSize = (int)m_vecCompInfo.size();
				for (m = 0; m < nSize; m++) {
					if (m_pComponent[xy] == m_vecCompInfo[m]._nLabel) {
						bExist = true;
						break;
					}
				}
				if (bExist) {
					++m_vecCompInfo[m]._nArea;
					m_vecCompInfo[m]._pntCenter.x += i;
					m_vecCompInfo[m]._pntCenter.y += j;

					if (m_vecCompInfo[m]._pntMin.x > i) m_vecCompInfo[m]._pntMin.x = i;
					if (m_vecCompInfo[m]._pntMin.y > j) m_vecCompInfo[m]._pntMin.y = j;

					if (m_vecCompInfo[m]._pntMax.x < i) m_vecCompInfo[m]._pntMax.x = i;
					if (m_vecCompInfo[m]._pntMax.y < j) m_vecCompInfo[m]._pntMax.y = j;
				} else {
					SCompInfo temp;
					temp._nLabel = m_pComponent[xy];
					temp._nArea = 1;
					temp._pntCenter.x = 0;
					temp._pntCenter.y = 0;

					temp._pntMin.x = INT_MAX;
					temp._pntMin.y = INT_MAX;

					temp._pntMax.x = INT_MIN;
					temp._pntMax.y = INT_MIN;

					m_vecCompInfo.push_back(temp);
				}
			}
		}
	}

	// min size
	nSize = (int)m_vecCompInfo.size();
	if (minsize > 0) {
		std::vector<SCompInfo> vecTemp;
		for (int n = 0; n < nSize; n++) {
			if (m_vecCompInfo[n]._nArea > minsize) {
				vecTemp.push_back(m_vecCompInfo[n]);
			}
		}
		vecTemp.swap(m_vecCompInfo);
	}

	// min size delete
	// 	std::vector<int>vecDeleteIdx;
	// 	for (int n = 0 ; n < nSize ; n++)
	// 	{
	// 		vecDeleteIdx.push_back(n);
	// 	}
	// 	LeaveOnly(vecDeleteIdx);

	// calc. center points
	nSize = (int)m_vecCompInfo.size();
	for (int n = 0; n < nSize; n++) {
		if (m_vecCompInfo[n]._nArea) {
			m_vecCompInfo[n]._pntCenter.x /= m_vecCompInfo[n]._nArea;
			m_vecCompInfo[n]._pntCenter.y /= m_vecCompInfo[n]._nArea;
		}
	}

	// sort by area
	sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompAreaDn);

	return (int)m_vecCompInfo.size();
}

int CVpConnComponent::DoAfterMorph(unsigned char* pImgData, eMorphology emorph, int kernel, int minsize) {
	CVpMorph morph;
	morph.Init2D(m_nWidth, m_nHeight, pImgData);
	switch (emorph) {
	case EROSION:
		morph.Erosion2D(kernel);
		break;

	case DILATION:
		morph.Dilation2D(kernel);
		break;

	case OPEN:
		morph.Open2D(kernel);
		break;

	case CLOSE:
		morph.Close2D(kernel);
		break;

	default:
		break;
	}

	return Do(pImgData, minsize);
}

int CVpConnComponent::DoAfterMorphEx(unsigned char* pImgData, eMorphology emorph, int kernel1, int kernel2, int minsize) {
	CVpMorph morph;
	morph.Init2D(m_nWidth, m_nHeight, pImgData);
	switch (emorph) {
	case EROSION:
		morph.Erosion2D(kernel1);
		break;

	case DILATION:
		morph.Dilation2D(kernel1);
		break;

	case OPEN:
		morph.Open2D(kernel1, kernel2);
		break;

	case CLOSE:
		morph.Close2D(kernel1, kernel2);
		break;

	default:
		break;
	}

	return Do(pImgData, minsize);
}

void CVpConnComponent::LeaveOneComponent(int idx) {
	if ((int)m_vecCompInfo.size() <= idx) return;

	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;

	// 2D
	if (m_ppComponent == NULL) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;
				if (m_pComponent[xy] != nLabel) m_pComponent[xy] = 0;
			}
		}
	}
	// 3D
	else {
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;
					if (m_ppComponent[k][xy] != nLabel) m_ppComponent[k][xy] = 0;
				}
			}
		}
	}
}

unsigned char** CVpConnComponent::ExportComponent3D(int idx, int* pVol) {
	unsigned char** ppOut = NULL;
	if (!SafeNew2D(ppOut, m_nWidth*m_nHeight, m_nDepth)) return NULL;

	int nVol = 0;
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	for (int k = 0; k < m_nDepth; k++) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;

				if (m_ppComponent[k][xy] == nLabel) {
					ppOut[k][xy] = m_bytExpValue;
					++nVol;
				} else	ppOut[k][xy] = 0;
			}
		}
	}
	if (pVol) *pVol = nVol;

	return ppOut;
}

unsigned char* CVpConnComponent::ExportComponent2D(int idx, int* pArea) {
	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	int nArea = 0;
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;

	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;

			if (m_pComponent[xy] == nLabel) {
				pOut[xy] = m_bytExpValue;
				++nArea;
			} else	pOut[xy] = 0;
		}
	}

	if (pArea) *pArea = nArea;

	return pOut;
}

unsigned char* CVpConnComponent::ExportComponents2D(std::vector<int>& vecidx, int* pArea) {
	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	int nArea = 0;
	std::vector<conncomtype> vecLabel;
	int nSize = (int)vecidx.size();
	for (int i = 0; i < nSize; i++)	vecLabel.push_back(m_vecCompInfo[vecidx[i]]._nLabel);

	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;
			for (int n = 0; n < nSize; n++) {
				if (m_pComponent[xy] == vecLabel[n]) {
					pOut[xy] = m_bytExpValue;
					++nArea;
				}
			}
		}
	}

	if (pArea) *pArea = nArea;

	return pOut;
}

unsigned char** CVpConnComponent::ExportComponent3DFilter(int minsize, int* pCompCnt) {
	if (m_ppComponent == NULL) return NULL;

	unsigned char** ppOut = NULL;
	if (!SafeNew2D< unsigned char>(ppOut, m_nWidth*m_nHeight, m_nDepth)) return NULL;

	std::vector<conncomtype> vecLabel;
	int nComp = (int)m_vecCompInfo.size();
	for (int n = 0; n < nComp; n++) {
		if (m_vecCompInfo[n]._nArea > minsize) vecLabel.push_back(m_vecCompInfo[n]._nLabel);
	}
	nComp = (int)vecLabel.size();
	if (pCompCnt) *pCompCnt = nComp;

	for (int k = 0; k < m_nDepth; k++) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;
				bool bIsExist = false;
				for (int n = 0; n < nComp; n++) {
					if (m_ppComponent[k][xy] == vecLabel[n]) {
						bIsExist = true;
						break;
					}
				}
				if (bIsExist)	ppOut[k][xy] = m_bytExpValue;
			}
		}
	}

	return ppOut;
}

unsigned char* CVpConnComponent::ExportComponent2DFilter(int minsize, int* pCompCnt) {
	if (m_pComponent == NULL) return NULL;

	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	std::vector<conncomtype> vecLabel;
	int nComp = (int)m_vecCompInfo.size();
	for (int n = 0; n < nComp; n++) {
		if (m_vecCompInfo[n]._nArea > minsize) vecLabel.push_back(m_vecCompInfo[n]._nLabel);
	}
	nComp = (int)vecLabel.size();
	if (pCompCnt) *pCompCnt = nComp;

	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;
			bool bIsExist = false;
			for (int n = 0; n < nComp; n++) {
				if (m_pComponent[xy] == vecLabel[n]) {
					bIsExist = true;
					break;
				}
			}
			if (bIsExist)	pOut[xy] = m_bytExpValue;
		}
	}

	return pOut;
}

unsigned char* CVpConnComponent::ExportComponent2DFilterOnlyCenter(int minsize, int* pCompCnt) {
	if (m_pComponent == NULL) return NULL;

	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	std::vector<conncomtype> vecLabel;
	int nComp = (int)m_vecCompInfo.size();
	for (int n = 0; n < nComp; n++) {
		int diffx = m_nWidth - m_vecCompInfo[n]._pntCenter.x;
		int diffy = m_nHeight - m_vecCompInfo[n]._pntCenter.y;
		int offset = 15;

		if ((m_vecCompInfo[n]._nArea > minsize) && ((diffx > offset) || (diffy > offset))) {
			vecLabel.push_back(m_vecCompInfo[n]._nLabel);
		}
	}

	nComp = (int)vecLabel.size();
	if (pCompCnt) *pCompCnt = nComp;

	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;
			bool bIsExist = false;
			for (int n = 0; n < nComp; n++) {
				if (m_pComponent[xy] == vecLabel[n]) {
					bIsExist = true;
					break;
				}
			}
			if (bIsExist)	pOut[xy] = m_bytExpValue;
		}
	}

	return pOut;
}

unsigned char* CVpConnComponent::ExportComponent2DRect(int idx, int* outw, int* outh, int* outarea) {
	if (m_pComponent == NULL) return NULL;

	unsigned char* pOut = NULL;

	int w = m_vecCompInfo[idx]._pntMax.x - m_vecCompInfo[idx]._pntMin.x + 1;
	int h = m_vecCompInfo[idx]._pntMax.y - m_vecCompInfo[idx]._pntMin.y + 1;
	int area = 0;

	if (!SafeNews< unsigned char>(pOut, w*h)) return NULL;

	for (int j = m_vecCompInfo[idx]._pntMin.y; j <= m_vecCompInfo[idx]._pntMax.y; j++) {
		for (int i = m_vecCompInfo[idx]._pntMin.x; i <= m_vecCompInfo[idx]._pntMax.x; i++) {
			int xy = i + j * m_nWidth;
			if (m_pComponent[xy] == m_vecCompInfo[idx]._nLabel) {
				int x = i - m_vecCompInfo[idx]._pntMin.x;
				int y = j - m_vecCompInfo[idx]._pntMin.y;
				pOut[x + y * w] = m_bytExpValue;
				++area;
			}
		}
	}

	if (outw)		*outw = w;
	if (outh)		*outh = h;
	if (outarea)	*outarea = area;

	return pOut;
}

unsigned char** CVpConnComponent::Export3D(int* pComp) {
	unsigned char** ppOut = NULL;
	if (!SafeNew2D(ppOut, m_nWidth*m_nHeight, m_nDepth)) return NULL;

	int nVol = 0;
	for (int k = 0; k < m_nDepth; k++) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;

				if (m_ppComponent[k][xy] != 0) {
					ppOut[k][xy] = m_bytExpValue;
					++nVol;
				} else	ppOut[k][xy] = 0;
			}
		}
	}
	if (pComp) *pComp = (int)m_vecCompInfo.size();

	return ppOut;
}

unsigned char* CVpConnComponent::Export2D(int* pComp) {
	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	int nArea = 0;
	//#pragma omp parallel for
	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;

			if (m_pComponent[xy] > 0) {
				pOut[xy] = m_bytExpValue;
				++nArea;
			}
			//	else	pOut[xy] = 0;
		}
	}

	if (pComp) *pComp = (int)m_vecCompInfo.size();

	return pOut;
}

unsigned char* CVpConnComponent::Export2DEx(int* pComp) {
	unsigned char* pOut = NULL;
	if (!SafeNews< unsigned char>(pOut, m_nWidth*m_nHeight)) return NULL;

	int nArea = 0;
	int nComCont = (int)m_vecCompInfo.size();
#pragma omp parallel for
	for (int j = 0; j < m_nHeight; j++) {
		for (int i = 0; i < m_nWidth; i++) {
			int xy = i + j * m_nWidth;
			if (m_pComponent[xy] > 0) {
				for (int n = 0; n < nComCont; n++) {
					if (m_pComponent[xy] == m_vecCompInfo[n]._nLabel) {
						pOut[xy] = m_bytExpValue;
						++nArea;
					}
				}
			}
			//	else	pOut[xy] = 0;
		}
	}

	if (pComp) *pComp = nComCont;

	return pOut;
}

void CVpConnComponent::Sort(eSortType esort) {
	switch (esort) {
	case AREA:
		// sort by area
		sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompAreaDn);
		break;

	case BNDBOX:
		if (m_ppComponent == NULL)
			sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompBndBox2DDn);
		else
			sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompBndBox3DDn);
		break;

	case LENGTH:
		if (m_ppComponent == NULL)
			sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompLength2DDn);
		else
			sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompLength3DDn);
		break;

	case XCOORD:
		sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompXCoordDn);
		break;

	case YCOORD:
		sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompYCoordDn);
		break;

	case XCOORDUP:
		sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompXCoordUp);
		break;

	case YCOORDUP:
		sort(m_vecCompInfo.begin(), m_vecCompInfo.end(), CompareCompYCoordUp);
		break;

	default:
		break;
	}
}

bool CVpConnComponent::RemoveFromPoint(int x, int y) {
	if (m_pComponent == NULL) return false;
	int xy = x + y * m_nWidth;
	int nCnt = (int)m_vecCompInfo.size();
	int idx = -1;

	for (int n = 0; n < nCnt; n++) {
		if (m_vecCompInfo[n]._nLabel == m_pComponent[xy]) {
			idx = n;
			break;
		}
	}
	if (idx != -1) return Remove(idx);

	return false;
}

bool CVpConnComponent::RemoveFromPoint(int x, int y, int z) {
	if (m_ppComponent == NULL) return false;
	int xy = x + y * m_nWidth;
	int nCnt = (int)m_vecCompInfo.size();
	int idx = -1;

	for (int n = 0; n < nCnt; n++) {
		if (m_vecCompInfo[n]._nLabel == m_ppComponent[z][xy]) {
			idx = n;
			break;
		}
	}
	if (idx != -1) return Remove(idx);

	return false;
}

bool CVpConnComponent::Remove(int idx) {
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;

	// 2D
	if (m_ppComponent == NULL) {
#pragma omp parallel for
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;

				if (m_pComponent[xy] == nLabel) {
					m_pComponent[xy] = 0;
				}
			}
		}
	}
	// 3D
	else {
#pragma omp parallel for
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;

					if (m_ppComponent[k][xy] == nLabel) {
						m_ppComponent[k][xy] = 0;
					}
				}
			}
		}
	}

	return true;
}

bool CVpConnComponent::LeaveFromPoint(int x, int y) {
	if (m_pComponent == NULL) return false;
	int xy = x + y * m_nWidth;
	int nCnt = (int)m_vecCompInfo.size();
	int idx = -1;

	for (int n = 0; n < nCnt; n++) {
		if (m_vecCompInfo[n]._nLabel == m_pComponent[xy]) {
			idx = n;
			break;
		}
	}
	if (idx != -1)	return LeaveOnly(&idx, 1);

	return false;
}

bool CVpConnComponent::LeaveFromPoint(int x, int y, int z) {
	if (m_ppComponent == NULL) return false;
	int xy = x + y * m_nWidth;
	int nCnt = (int)m_vecCompInfo.size();
	int idx = -1;

	for (int n = 0; n < nCnt; n++) {
		if (m_vecCompInfo[n]._nLabel == m_ppComponent[z][xy]) {
			idx = n;
			break;
		}
	}
	if (idx != -1) return LeaveOnly(&idx, 1);

	return false;
}

bool CVpConnComponent::LeaveOnly(int* pIdx, int nCnt) {
	int nTotal = Min(nCnt, (int)m_vecCompInfo.size());
	if (nTotal < nCnt) return false;

	// 2D
	if (m_ppComponent == NULL) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;
				bool bIsDeleted = true;
				for (int n = 0; n < nTotal; n++) {
					conncomtype nLabel = m_vecCompInfo[pIdx[n]]._nLabel;
					if (m_pComponent[xy] == nLabel) {
						bIsDeleted = false;
						break;
					}
				}
				if (bIsDeleted) m_pComponent[xy] = 0;
			}
		}
	}
	// 3D
	else {
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;
					bool bIsDeleted = true;
					for (int n = 0; n < nTotal; n++) {
						conncomtype nLabel = m_vecCompInfo[pIdx[n]]._nLabel;
						if (m_ppComponent[k][xy] == nLabel) {
							bIsDeleted = false;
							break;
						}
					}
					if (bIsDeleted) m_ppComponent[k][xy] = 0;
				}
			}
		}
	}

	return true;
}

bool CVpConnComponent::LeaveOnly(std::vector<int>& vecIdx) {
	int nSize = (int)vecIdx.size();
	if (nSize == 0) return false;

	int* pIdx = new int[nSize];
	for (int i = 0; i < nSize; i++) {
		pIdx[i] = vecIdx[i];
	}

	bool ret = LeaveOnly(pIdx, nSize);
	SAFE_DELETE_ARRAY(pIdx);

	return ret;
}

bool CVpConnComponent::LeaveOnly() {
	int nCnt = (int)m_vecCompInfo.size();

	// 2D
	if (m_ppComponent == NULL) {
		for (int j = 0; j < m_nHeight; j++) {
			for (int i = 0; i < m_nWidth; i++) {
				int xy = i + j * m_nWidth;
				bool bIsDeleted = true;
				for (int n = 0; n < nCnt; n++) {
					conncomtype nLabel = m_vecCompInfo[n]._nLabel;
					if (m_pComponent[xy] == nLabel) {
						bIsDeleted = false;
						break;
					}
				}
				if (bIsDeleted) m_pComponent[xy] = 0;
			}
		}
	}
	// 3D
	else {
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;
					bool bIsDeleted = true;
					for (int n = 0; n < nCnt; n++) {
						conncomtype nLabel = m_vecCompInfo[n]._nLabel;
						if (m_ppComponent[k][xy] == nLabel) {
							bIsDeleted = false;
							break;
						}
					}
					if (bIsDeleted) m_ppComponent[k][xy] = 0;
				}
			}
		}
	}

	return true;
}

TPoint3D<int> CVpConnComponent::GetImmediatePoint(int idx) {
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	TPoint3D<int> pnt3d;
	pnt3d.x = 0;
	pnt3d.y = 0;
	pnt3d.z = 0;
	int nHalfSESize = 2;

	// 2D
	if (m_ppComponent == NULL) {
		for (int y = nHalfSESize; y < m_nHeight - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++) {
				//
				bool bIsSeed = true;
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * m_nWidth;
						if (m_pComponent[idxxy] != nLabel) {
							bIsSeed = false;
							break;
						}
					}
					if (!bIsSeed) break;
				}
				if (bIsSeed) {
					pnt3d.x = x;
					pnt3d.y = y;
					pnt3d.z = 0;
					return pnt3d;
				}
			}
		}
	}
	// 3D
	else {
		for (int z = nHalfSESize; z < m_nDepth - nHalfSESize; z++) {
			for (int y = nHalfSESize; y < m_nHeight - nHalfSESize; y++) {
				for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++) {
					//
					bool bIsSeed = true;
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * m_nWidth;

								if (m_ppComponent[idxz][idxxy] != nLabel) {
									bIsSeed = false;
									break;
								}
							}
							if (!bIsSeed) break;
						}
						if (!bIsSeed) break;
					}
					if (bIsSeed) {
						pnt3d.x = x;
						pnt3d.y = y;
						pnt3d.z = z;
						return pnt3d;
					}
				}
			}
		}
	}

	return pnt3d;
}

TPoint3D<int> CVpConnComponent::GetCenterXImmediatePoint(int idx) {
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	TPoint3D<int> pnt3d;
	pnt3d.x = 0;
	pnt3d.y = 0;
	pnt3d.z = 0;
	int nHalfSESize = 2;

	//// 2D
	//int centerX = (m_vecCompInfo[idx]._pntMax.x + m_vecCompInfo[idx]._pntMin.x) >> 1;
	//int startX = ((centerX - nHalfSESize) < 0) ? nHalfSESize : centerX;
	//if (m_ppComponent == NULL)
	//{
	//	for (int y = startX; y < m_nHeight - nHalfSESize; y++)
	//	{
	//		for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++)
	//		{
	//			//
	//			bool bIsSeed = true;
	//			for (int m = -nHalfSESize; m <= nHalfSESize; m++)
	//			{
	//				for (int l = -nHalfSESize; l <= nHalfSESize; l++)
	//				{
	//					int idxx = x + l;
	//					int idxy = y + m;
	//					int idxxy = idxx + idxy*m_nWidth;
	//					if (m_pComponent[idxxy] != nLabel)
	//					{
	//						bIsSeed = false;
	//						break;
	//					}
	//				}
	//				if (!bIsSeed) break;
	//			}
	//			if (bIsSeed)
	//			{
	//				pnt3d.x = x;
	//				pnt3d.y = y;
	//				pnt3d.z = 0;
	//				return pnt3d;
	//			}
	//		}
	//	}

	//}
	//// 3D
	//else
	//{
	//	for (int z = nHalfSESize; z < m_nDepth - nHalfSESize; z++)
	//	{
	//		for (int y = startY; y < m_nHeight - nHalfSESize; y++)
	//		{
	//			for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++)
	//			{
	//				//
	//				bool bIsSeed = true;
	//				for (int n = -nHalfSESize; n <= nHalfSESize; n++)
	//				{
	//					for (int m = -nHalfSESize; m <= nHalfSESize; m++)
	//					{
	//						for (int l = -nHalfSESize; l <= nHalfSESize; l++)
	//						{
	//							int idxx = x + l;
	//							int idxy = y + m;
	//							int idxz = z + n;
	//							int idxxy = idxx + idxy*m_nWidth;

	//							if (m_ppComponent[idxz][idxxy] != nLabel)
	//							{
	//								bIsSeed = false;
	//								break;
	//							}

	//						}
	//						if (!bIsSeed) break;
	//					}
	//					if (!bIsSeed) break;
	//				}
	//				if (bIsSeed)
	//				{
	//					pnt3d.x = x;
	//					pnt3d.y = y;
	//					pnt3d.z = z;
	//					return pnt3d;
	//				}
	//			}
	//		}
	//	}
	//}

	return pnt3d;
}

TPoint3D<int> CVpConnComponent::GetCenterYImmediatePoint(int idx) {
	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	TPoint3D<int> pnt3d;
	pnt3d.x = 0;
	pnt3d.y = 0;
	pnt3d.z = 0;
	int nHalfSESize = 2;

	// 2D
	int centerY = (m_vecCompInfo[idx]._pntMax.y + m_vecCompInfo[idx]._pntMin.y) >> 1;
	int startY = ((centerY - nHalfSESize) < 0) ? nHalfSESize : centerY;
	if (m_ppComponent == NULL) {
		for (int y = startY; y < m_nHeight - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++) {
				//
				bool bIsSeed = true;
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * m_nWidth;
						if (m_pComponent[idxxy] != nLabel) {
							bIsSeed = false;
							break;
						}
					}
					if (!bIsSeed) break;
				}
				if (bIsSeed) {
					pnt3d.x = x;
					pnt3d.y = y;
					pnt3d.z = 0;
					return pnt3d;
				}
			}
		}
	}
	// 3D
	else {
		for (int z = nHalfSESize; z < m_nDepth - nHalfSESize; z++) {
			for (int y = startY; y < m_nHeight - nHalfSESize; y++) {
				for (int x = nHalfSESize; x < m_nWidth - nHalfSESize; x++) {
					bool bIsSeed = true;
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * m_nWidth;

								if (m_ppComponent[idxz][idxxy] != nLabel) {
									bIsSeed = false;
									break;
								}
							}
							if (!bIsSeed) break;
						}
						if (!bIsSeed) break;
					}
					if (bIsSeed) {
						pnt3d.x = x;
						pnt3d.y = y;
						pnt3d.z = z;
						return pnt3d;
					}
				}
			}
		}
	}

	return pnt3d;
}

int CVpConnComponent::CalcMeanIntensity(int idx, voltype* pData) {
	if (m_pComponent == NULL) return 0;

	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	int nMeanintensity = 0;
	int wh = m_nWidth * m_nHeight;

	for (int xy = 0; xy < wh; xy++) {
		if (m_pComponent[xy] == nLabel)	nMeanintensity += pData[xy];
	}
	nMeanintensity /= m_vecCompInfo[idx]._nArea;

	return nMeanintensity;
}

int CVpConnComponent::CalcMeanIntensity(int idx, voltype** ppData) {
	if (m_ppComponent == NULL) return 0;

	conncomtype nLabel = m_vecCompInfo[idx]._nLabel;
	int nMeanintensity = 0;
	int wh = m_nWidth * m_nHeight;
	for (int z = 0; z < m_nDepth; z++) {
		for (int xy = 0; xy < wh; xy++) {
			if (m_ppComponent[z][xy] == nLabel)	nMeanintensity += ppData[z][xy];
		}
		nMeanintensity /= m_vecCompInfo[idx]._nArea;
	}

	return nMeanintensity;
}

int CVpConnComponent::GetComponentResult(unsigned char** ppResult) {
	// 2D
	if (m_ppComponent == NULL) {
		// not yet
	}
	// 3D
	else {
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;
					if (m_ppComponent[k][xy] != 0) {
						ppResult[k][xy] = 255;
					} else {
						ppResult[k][xy] = 0;
					}
				}
			}
		}
	}

	return 0;
}

int CVpConnComponent::GetComponentResult(int idx, unsigned char** ppResult) {
	// 2D
	if (m_ppComponent == NULL) {
		// not yet
	}
	// 3D
	else {
		int nLabel = m_vecCompInfo[idx]._nLabel;
		for (int k = 0; k < m_nDepth; k++) {
			for (int j = 0; j < m_nHeight; j++) {
				for (int i = 0; i < m_nWidth; i++) {
					int xy = i + j * m_nWidth;
					if (m_ppComponent[k][xy] == nLabel) {
						ppResult[k][xy] = 255;
					} else {
						ppResult[k][xy] = 0;
					}
				}
			}
		}
	}

	return 0;
}
