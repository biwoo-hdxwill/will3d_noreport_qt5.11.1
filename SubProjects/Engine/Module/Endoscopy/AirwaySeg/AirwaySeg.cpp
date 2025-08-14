#include "AirwaySeg.h"

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include <ctime>
#include <iostream>
#include <queue>
#include <qmath.h>

#include "../../../Engine/Common/Common/W3Types.h"
#include "../../../Engine/Core/Surfacing/MarchingCube.h"

CAirwaySeg::CAirwaySeg(void) {}
CAirwaySeg::~CAirwaySeg(void) {}

bool CAirwaySeg::doRun(unsigned short** ppVolData3D, unsigned char** ppMaskData3D, int nW, int nH, int nD, int nDataMin, float fPixelSpacing, float fIntercept,
					   std::vector<glm::vec3> vecCP, std::vector<glm::vec3> vecPath, int nDownFactor, int nAirwayMax, bool bClosing) {
	int nCPnum = static_cast<int>(vecCP.size());
	int nWH = nW * nH;
	float fPS_SQ = fPixelSpacing * fPixelSpacing;

	//path point interpolation
	std::vector<glm::vec3> vecCopy(vecPath);
	vecPath.clear();
	std::vector<glm::vec3> vecInterp;
	int idxVAt = 0;
	for (int idx = 0; idx < vecCopy.size() - 1; ++idx) {
		glm::vec3 vF = vecCopy.at(idx);
		glm::vec3 vB = vecCopy.at(idx + 1);
		glm::vec3 vInterp = (vB + vF)*0.5f;
		vecInterp.push_back(vInterp);
	}

	bool bOdd = true;
	for (int idx = 0, idxInterp = 0; idx < (int)vecCopy.size() - 1; ++idx) {
		glm::vec3 vAt = vecCopy.at(idx);
		vecPath.push_back(vAt);
		vecPath.push_back(vecInterp.at(idxInterp++));
	}
	vecPath.push_back(vecCopy.at((int)vecCopy.size() - 1));
	vecCopy.clear();
	vecInterp.clear();

	vecCopy = std::vector<glm::vec3>(vecPath);
	for (int idx = 1; idx < vecCopy.size() - 2; ++idx) {
		glm::vec3 vInterp = (vecCopy.at(idx + 1) + vecCopy.at(idx - 1))*0.5f;
		glm::vec3 vAt = (vInterp + vecCopy.at(idx))*0.5f;
		vecPath[idx] = vAt;
	}

	////////////////////////////////////////////////////////////////////////
	//////////////////// 2. 양 끝 normal vector 구하기 //////////////////////
	////////////////////////////////////////////////////////////////////////

	glm::vec3 vecNormalS = vecCP[0] - vecCP[1];
	//vecNormalS.Normalize();
	vecNormalS = glm::normalize(vecNormalS);
	glm::vec3 vecNormalE = vecCP[nCPnum - 1] - vecCP[nCPnum - 2];
	//vecNormalE.Normalize();
	vecNormalE = glm::normalize(vecNormalE);

	////////////////////////////////////////////////////////////////////////
	//////////////////////// 3. Protect 하기 ///////////////////////////////
	////////////////////////////////////////////////////////////////////////
	std::clock_t total_start;
	total_start = std::clock();

	std::clock_t start3;
	double duration3 = 0;
	start3 = std::clock();

	bool** ppProtect = new bool*[nD];
	for (int i = 0; i < nD; ++i) {
		ppProtect[i] = new bool[nWH];
		memset(ppProtect[i], 0, nWH);
	}

	// input grid size
	int nGridSize = 3000;
	float fPlaneWidth = static_cast<float>(nGridSize);
	float fPlaneHeight = static_cast<float>(nGridSize);
	int nPlaneWidth = nGridSize;
	int nPlaneHeight = nGridSize;

	glm::vec3 vR1, vR2;
	if (vecNormalS.z != 0.0f)
		vR1 = glm::vec3(1, 1, (-vecNormalS.x - vecNormalS.y) / vecNormalS.z);
	else if (vecNormalS.y != 0.0f)
		vR1 = glm::vec3(1, (-vecNormalS.x - vecNormalS.z) / vecNormalS.y, 1);
	else if (vecNormalS.x != 0.0f)
		vR1 = glm::vec3((-vecNormalS.y - vecNormalS.z) / vecNormalS.x, 1, 1);
	else {
		std::cerr << "normal vector at start position is (0,0,0)\n";
		return false;
	}

	//vR1.Normalize();
	vR1 = glm::normalize(vR1);
	if (vecNormalE.z != 0.0f)
		vR2 = glm::vec3(1, 1, (-vecNormalE.x - vecNormalE.y) / vecNormalE.z);
	else if (vecNormalE.y != 0.0f)
		vR2 = glm::vec3(1, (-vecNormalE.x - vecNormalE.z) / vecNormalE.y, 1);
	else if (vecNormalE.x != 0.0f)
		vR2 = glm::vec3((-vecNormalE.y - vecNormalE.z) / vecNormalE.x, 1, 1);
	else {
		std::cerr << "normal vector at end position is (0,0,0)\n";
		return false;
	}
	//vR2.Normalize();
	vR2 = glm::normalize(vR2);
	//glm::vec3 vB1 = glm::vec3::CrossProduct(vR1, vecNormalS).Normalized();
	//glm::vec3 vB2 = glm::vec3::CrossProduct(vR2, vecNormalE).Normalized();
	glm::vec3 vB1 = glm::normalize(glm::cross(vR1, vecNormalS));
	glm::vec3 vB2 = glm::normalize(glm::cross(vR2, vecNormalE));

	//float fD1 = glm::vec3::DotProduct(vecNormalS, vR1);
	//float fD2 = glm::vec3::DotProduct(vecNormalE, vR2);
	float fD1 = glm::dot(vecNormalS, vR1);
	float fD2 = glm::dot(vecNormalE, vR2);

	float fMinSpacing = 0.5f;
	vR1 = vR1 * fMinSpacing;
	vB1 = vB1 * fMinSpacing;
	vR2 = vR2 * fMinSpacing;
	vB2 = vB2 * fMinSpacing;

	glm::vec3 ptPlaneStart1 = -fPlaneWidth * vR1*0.5f - fPlaneHeight * vB1*0.5f + vecCP[0];
	glm::vec3 ptPlaneStart2 = -fPlaneWidth * vR2*0.5f - fPlaneHeight * vB2*0.5f + vecCP[nCPnum - 1];
	for (int v = 0; v < nPlaneHeight; ++v) {
		glm::vec3 ptRowStart1 = ptPlaneStart1 + vB1 * static_cast<float>(v);
		glm::vec3 ptPlane1 = ptRowStart1;

		glm::vec3 ptRowStart2 = ptPlaneStart2 + vB2 * static_cast<float>(v);
		glm::vec3 ptPlane2 = ptRowStart2;

		for (int u = 0; u < nPlaneWidth; ++u) {
			glm::vec3 ptVol1 = glm::vec3(ptPlane1.x, ptPlane1.y, ptPlane1.z/*/m_SpacingZ*/); // spacing 고려되는지
			int iX1 = static_cast<int>(ptVol1.x + 0.5f);
			int iY1 = static_cast<int>(ptVol1.y + 0.5f);
			int iZ1 = static_cast<int>(ptVol1.z + 0.5f);
			glm::vec3 ptVol2 = glm::vec3(ptPlane2.x, ptPlane2.y, ptPlane2.z/*/m_SpacingZ*/); // spacing 고려되는지
			int iX2 = static_cast<int>(ptVol2.x + 0.5f);
			int iY2 = static_cast<int>(ptVol2.y + 0.5f);
			int iZ2 = static_cast<int>(ptVol2.z + 0.5f);
			// Nearest-neighbor sampling.
			if (iX1 >= 0 && iX1 < nW &&
				iY1 >= 0 && iY1 < nH &&
				iZ1 >= 0 && iZ1 < nD) {
				ppProtect[iZ1][iY1*nW + iX1] = true;
			}
			if (iX2 >= 0 && iX2 < nW &&
				iY2 >= 0 && iY2 < nH &&
				iZ2 >= 0 && iZ2 < nD) {
				ppProtect[iZ2][iY2*nW + iX2] = true;
			}
			ptPlane1 += vR1;
			ptPlane2 += vR2;
		}
	}

	duration3 = (std::clock() - start3) / (double)CLOCKS_PER_SEC;
	std::cout << "protect time = " << duration3 << '\n';

	////////////////////////////////////////////////////////////////////////
	/////////// 4. Region growing seed 및 intensity distance 구하기 /////////
	////////////////////////////////////////////////////////////////////////

	std::clock_t start4;
	double duration4 = 0;
	start4 = std::clock();

	int nCenterIndex = static_cast<int>(nCPnum / 2);
	int nDistanceMin = 10000;
	int nDistanceMax = -10000;
	int nDistanceTemp = 0;

	// 사용자가 찍은 control point들을 기준으로 region growing 알고리즘의 intensity distance의 min/max를 구함.
	for (int i = 0; i < nCPnum; ++i) {
		glm::vec3 vecTemp = vecCP[i];
		nDistanceTemp = ppVolData3D[static_cast<int>(vecTemp.z)][static_cast<int>(vecTemp.x) + static_cast<int>(vecTemp.y)*nW] + fIntercept;
		if (nDistanceTemp < nDistanceMin)
			nDistanceMin = nDistanceTemp;

		if (nDistanceTemp > nDistanceMax)
			nDistanceMax = nDistanceTemp;
	}
	std::queue<TPoint3D<int>> Q;
	unsigned char** ppLabel = new unsigned char*[nD]; // 분할된 mask가 임시로 저장될 label volume
	for (int i = 0; i < nD; ++i) {
		ppLabel[i] = new unsigned char[nWH];
		memset(ppLabel[i], 0, sizeof(unsigned char)*nWH);
	}

#pragma omp parallel for
	for (int k = 0; k < nD; ++k) {
		for (int j = 0; j < nWH; ++j) {
			if (ppProtect[k][j] == true)
				ppLabel[k][j] |= LABEL_PROTECTED;
		}
	}

	// 사용자가 찍은 control point들 중 가운데 control point의 intensity를 기준으로 intensity distance를 정함.
	TPoint3D<int> pnt;
	pnt.x = static_cast<int>(vecCP[nCenterIndex].x);
	pnt.y = static_cast<int>(vecCP[nCenterIndex].y);
	pnt.z = static_cast<int>(vecCP[nCenterIndex].z);

	register int nIndex;
	register short nData = ppVolData3D[pnt.z][pnt.x + pnt.y*nW] + fIntercept;
	int nDistance = 0;
	if (abs(nData - nDistanceMin) > abs(nData - nDistanceMax))
		nDistance = abs(nData - nDistanceMin);
	else
		nDistance = abs(nData - nDistanceMax);

	// Sagital 에서 control point 찍는걸 기준으로 Region growing 알고리즘 적용할 VOI 구함
	int nRG_MinX = 1000, nRG_MaxX = -1000, nRG_MinY = 1000, nRG_MaxY = -1000, nRG_MinZ = 1000, nRG_MaxZ = -1000;
	for (int k = 0; k < nCPnum; ++k) {
		TPoint3D<int> pnt;
		pnt.y = static_cast<int>(vecCP[k].y);
		pnt.z = static_cast<int>(vecCP[k].z);

		if (nRG_MinY > pnt.y)
			nRG_MinY = pnt.y;
		if (nRG_MinZ > pnt.z)
			nRG_MinZ = pnt.z;
		if (nRG_MaxY < pnt.y)
			nRG_MaxY = pnt.y;
		if (nRG_MaxZ < pnt.z)
			nRG_MaxZ = pnt.z;
	}

	int nRG_Margin = 50;
	if (nRG_MinY - nRG_Margin >= 0)
		nRG_MinY = nRG_MinY - nRG_Margin;
	if (nRG_MinZ - nRG_Margin >= 0)
		nRG_MinZ = nRG_MinZ - nRG_Margin;
	if (nRG_MaxY + nRG_Margin < nH)
		nRG_MaxY = nRG_MaxY + nRG_Margin;
	if (nRG_MaxZ + nRG_Margin < nD)
		nRG_MaxZ = nRG_MaxZ + nRG_Margin;

	nRG_MinX = 1;
	nRG_MaxX = nW - 1;

	duration4 = (std::clock() - start4) / (double)CLOCKS_PER_SEC;
	std::cout << "calculating seed + distance time = " << duration4 << '\n';

	////////////////////////////////////////////////////////////////////////
	// 5. 3D Region growing 수행
	////////////////////////////////////////////////////////////////////////

	std::clock_t start5;
	double duration5 = 0;
	start5 = std::clock();

	int nMin = nDataMin + fIntercept - 1;
	int nMax = nData + nDistance;
	nAirwayMax = nAirwayMax/* - fIntercept*/; //  기도 절대 파라미터 세팅. nAirwayMax = -900 ==> Airway는 절대 -800 이상이 될수 없다

	if (nMax > nAirwayMax)
		nMax = nAirwayMax;
	nMax = nAirwayMax;

	//  첫번째와 마지막 control point를 제외한 control point들을 region growing seed로 push 한다.
	for (int i = 1; i < nCPnum - 1; ++i) {
		TPoint3D<int> pnt;
		pnt.x = static_cast<int>(vecCP[i].x);
		pnt.y = static_cast<int>(vecCP[i].y);
		pnt.z = static_cast<int>(vecCP[i].z);
		short nData = ppVolData3D[pnt.z][pnt.x + pnt.y*nW] + fIntercept;
		if (nData > nMin && nData < nMax)
			Q.push(pnt);
	}

	if (Q.size() == 0) {
		std::cout << "Choose correct control points again 1" << std::endl;
		return false;
	}

	TPoint3D<int> PointTemp;

	//  Region growing 수행
	do {
		TPoint3D<int> Q_Point = Q.front();
		Q.pop();

		int x_pos = Q_Point.x;
		int y_pos = Q_Point.y;
		int z_pos = Q_Point.z;
		nIndex = y_pos * nW;

		bool bTop_Connectivity = false;
		bool bBottom_Connectivity = false;
		bool bUp_Connectivity = false;
		bool bDown_Connectivity = false;

		bool bTop_Check = true;
		bool bBottom_Check = true;
		bool bUp_Check = true;
		bool bDown_Check = true;

		if (y_pos <= nRG_MinY) bTop_Check = false;
		if (y_pos >= nRG_MaxY) bBottom_Check = false;

		for (int x = x_pos; x >= nRG_MinX; x--) {
			int nIndexNeighbor = nIndex + x;
			if (!CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nIndexNeighbor])) break;

			nData = ppVolData3D[z_pos][nIndexNeighbor] + fIntercept;

			if (nMin <= nData && nData <= nMax) {
				ppLabel[z_pos][nIndexNeighbor] |= LABEL_SEGMENTING; //그 점에 레이블을 준다.

				if (bTop_Check) {
					int nRefIndex = nIndexNeighbor - nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						int nRefData = ppVolData3D[z_pos][nRefIndex] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bTop_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos - 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bTop_Connectivity = true;
							}
						} else bTop_Connectivity = false;
					} else bTop_Connectivity = false;
				}
				if (bBottom_Check) {
					int nRefIndex = nIndexNeighbor + nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						int nRefData = ppVolData3D[z_pos][nRefIndex] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bBottom_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos + 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bBottom_Connectivity = true;
							}
						} else bBottom_Connectivity = false;
					} else bBottom_Connectivity = false;
				}
				if (bUp_Check) {
					if ((z_pos - 1 >= nRG_MinZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos - 1][nIndexNeighbor])) {
						int nRefData = ppVolData3D[z_pos - 1][nIndexNeighbor] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bUp_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos - 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bUp_Connectivity = true;
							}
						} else bUp_Connectivity = false;
					} else bUp_Connectivity = false;
				}
				if (bDown_Check) {
					if ((z_pos + 1 < nRG_MaxZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos + 1][nIndexNeighbor])) {
						int nRefData = ppVolData3D[z_pos + 1][nIndexNeighbor] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bDown_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos + 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bDown_Connectivity = true;
							}
						} else bDown_Connectivity = false;
					} else bDown_Connectivity = false;
				}
			} else break;
		}

		bUp_Connectivity = bDown_Connectivity = bTop_Connectivity = bBottom_Connectivity = false;

		for (int x = x_pos + 1; x < nRG_MaxX; x++) {
			int nIndexNeighbor = nIndex + x;
			if (!CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nIndexNeighbor])) break;

			nData = ppVolData3D[z_pos][nIndexNeighbor] + fIntercept;

			if (nMin <= nData && nData <= nMax) {
				ppLabel[z_pos][nIndexNeighbor] |= LABEL_SEGMENTING; //그 점에 레이블을 준다.

				if (bTop_Check) {
					int nRefIndex = nIndexNeighbor - nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						int nRefData = ppVolData3D[z_pos][nRefIndex] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bTop_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos - 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bTop_Connectivity = true;
							}
						} else bTop_Connectivity = false;
					} else bTop_Connectivity = false;
				}
				if (bBottom_Check) {
					int nRefIndex = nIndexNeighbor + nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						int nRefData = ppVolData3D[z_pos][nRefIndex] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bBottom_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos + 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bBottom_Connectivity = true;
							}
						} else bBottom_Connectivity = false;
					} else bBottom_Connectivity = false;
				}
				if (bUp_Check) {
					if ((z_pos - 1 >= nRG_MinZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos - 1][nIndexNeighbor])) {
						int nRefData = ppVolData3D[z_pos - 1][nIndexNeighbor] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bUp_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos - 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bUp_Connectivity = true;
							}
						} else bUp_Connectivity = false;
					} else bUp_Connectivity = false;
				}
				if (bDown_Check) {
					if ((z_pos + 1 < nRG_MaxZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos + 1][nIndexNeighbor])) {
						int nRefData = ppVolData3D[z_pos + 1][nIndexNeighbor] + fIntercept;
						if (nMin <= nRefData && nRefData <= nMax) {
							if (!bDown_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos + 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bDown_Connectivity = true;
							}
						} else bDown_Connectivity = false;
					} else bDown_Connectivity = false;
				}
			} else break;
		}
	} while (!Q.empty()); // Q에 들어간 point가 소진될 때까지

#pragma omp parallel for
	for (int k = 0; k < nD; ++k) {
		for (int i = 0; i < nWH; ++i) {
			if (ppLabel[k][i] & LABEL_SEGMENTING)
				ppMaskData3D[k][i] = 1;
		}
	}

	duration5 = (std::clock() - start5) / (double)CLOCKS_PER_SEC;
	std::cout << "SRG time = " << duration5 << '\n';

	///////////////////////////////////////////////////////////////////////
	// Region growing 에 의해 initial 분할된 mask기준으로 VOI 계산
	///////////////////////////////////////////////////////////////////////

	std::clock_t startROI;
	double durationROI = 0;
	startROI = std::clock();

	for (int i = 0; i < nD; ++i) {
		memset(ppLabel[i], 0, sizeof(unsigned char)*nWH);
	}

	int nMinX = 1000, nMaxX = -1000, nMinY = 1000, nMaxY = -1000, nMinZ = 1000, nMaxZ = -1000;

	bool bNoSeg = true;
	for (int k = 0; k < nD; ++k) {
		for (int j = 0; j < nH; ++j) {
			for (int i = 0; i < nW; ++i) {
				int nIdx = nW * j + i;
				if (ppMaskData3D[k][nIdx] == 1) {
					bNoSeg = false;
					ppLabel[k][nIdx] |= LABEL_PROTECTED; // 다음 step 인 Hole filling 위한 protection

					if (nMinX > i)
						nMinX = i;
					if (nMinY > j)
						nMinY = j;
					if (nMinZ > k)
						nMinZ = k;
					if (nMaxX < i)
						nMaxX = i;
					if (nMaxY < j)
						nMaxY = j;
					if (nMaxZ < k)
						nMaxZ = k;
				}
			}
		}
	}

	if (bNoSeg) {
		std::cout << "Choose correct control points again 2" << std::endl;
		return false;
	}

	int nMargin = 3;
	if (nMinX - nMargin >= 0)
		nMinX = nMinX - nMargin;
	if (nMinY - nMargin >= 0)
		nMinY = nMinY - nMargin;
	if (nMinZ - nMargin >= 0)
		nMinZ = nMinZ - nMargin;
	if (nMaxX + nMargin < nW)
		nMaxX = nMaxX + nMargin;
	if (nMaxY + nMargin < nH)
		nMaxY = nMaxY + nMargin;
	if (nMaxZ + nMargin < nD)
		nMaxZ = nMaxZ + nMargin;

	durationROI = (std::clock() - startROI) / (double)CLOCKS_PER_SEC;
	std::cout << "VOI calculation time = " << durationROI << '\n';

	std::cout << "X  :  " << nMinX << " , " << nMaxX << std::endl;
	std::cout << "Y  :  " << nMinY << " , " << nMaxY << std::endl;
	std::cout << "Z  :  " << nMinZ << " , " << nMaxZ << std::endl;

	////////////////////////////////////////////////////////////////////////
	///////////////////// 6. 3D Closing 연산 수행하기 ///////////////////////
	////////////////////////////////////////////////////////////////////////
	if (bClosing) {
		// 6-1. Dilation Step
		std::clock_t start6;
		double duration6 = 0;
		start6 = std::clock();
		unsigned char** ppTempMask = new unsigned char*[nD];
#pragma omp parallel for
		for (int i = 0; i < nD; ++i) {
			ppTempMask[i] = new unsigned char[nWH];
			memset(ppTempMask[i], 0, sizeof(unsigned char)*nWH);
		}

		// 1. make binary
#pragma omp parallel for
		for (int z = 0; z < nD; ++z) {
			for (int xy = 0; xy < nWH; ++xy) {
				if (ppMaskData3D[z][xy] == 1) {
					ppTempMask[z][xy] = 1;
				}
			}
		}

		// 2. loop all voxel
#pragma omp parallel for
		for (int z = nMinZ; z < nMaxZ; ++z) {
			for (int y = nMinY; y < nMaxY; ++y) {
				for (int x = nMinX; x < nMaxX; ++x) {
					int sum = 0;
					int xy = y * nW + x;

					if (x > 0 && x < nW - 1 && y > 0 && y < nH - 1 && z > 0 && z < nD - 1) {
						sum += ppTempMask[z][xy];
						sum += ppTempMask[z - 1][xy];
						sum += ppTempMask[z + 1][xy];
						sum += ppTempMask[z][xy - nW];
						sum += ppTempMask[z][xy + nW];
						sum += ppTempMask[z][xy - 1];
						sum += ppTempMask[z][xy + 1];
					} else {
						sum += ppTempMask[z][xy];
						if (x != 0)
							sum += ppTempMask[z][xy - 1];

						if (y != 0)
							sum += ppTempMask[z][xy - nW];

						if (z != 0)
							sum += ppTempMask[z - 1][xy];

						if (x != nW - 1)
							sum += ppTempMask[z][xy + 1];

						if (y != nH - 1)
							sum += ppTempMask[z][xy + nW];

						if (z != nD - 1)
							sum += ppTempMask[z + 1][xy];
					}

					if (sum > 0) {
						ppMaskData3D[z][xy] = 1;
					}
				}
			}
		}

		// 6-2. Erosion Step
#pragma omp parallel for
		for (int i = 0; i < nD; ++i)
			memset(ppTempMask[i], 0, sizeof(unsigned char)*nWH);

		// 1. make binary
#pragma omp parallel for
		for (int z = 0; z < nD; ++z) {
			for (int xy = 0; xy < nWH; ++xy) {
				if (ppMaskData3D[z][xy] == 1) {
					ppTempMask[z][xy] = 1;
				}
			}
		}

		// 2. loop all voxel
#pragma omp parallel for
		for (int z = nMinZ; z < nMaxZ; ++z) {
			for (int y = nMinY; y < nMaxY; ++y) {
				for (int x = nMinX; x < nMaxX; ++x) {
					int sum = 0;
					int xy = y * nW + x;
					int nBoundary = 0;

					if (x > 0 && x < nW - 1 && y > 0 && y < nH - 1 && z > 0 && z < nD - 1) {
						sum += ppTempMask[z][xy];
						sum += ppTempMask[z - 1][xy];
						sum += ppTempMask[z + 1][xy];
						sum += ppTempMask[z][xy - nW];
						sum += ppTempMask[z][xy + nW];
						sum += ppTempMask[z][xy - 1];
						sum += ppTempMask[z][xy + 1];
						if (sum != 7) {
							ppMaskData3D[z][xy] = 0;
						}
					} else {
						nBoundary = 1;
						sum += ppTempMask[z][xy];
						if (x != 0) {
							sum += ppTempMask[z][xy - 1];
							++nBoundary;
						}

						if (y != 0) {
							sum += ppTempMask[z][xy - nW];
							++nBoundary;
						}

						if (z != 0) {
							sum += ppTempMask[z - 1][xy];
							++nBoundary;
						}

						if (x != nW - 1) {
							sum += ppTempMask[z][xy + 1];
							++nBoundary;
						}

						if (y != nH - 1) {
							sum += ppTempMask[z][xy + nW];
							++nBoundary;
						}

						if (z != nD - 1) {
							sum += ppTempMask[z + 1][xy];
							++nBoundary;
						}

						if (sum != nBoundary) {
							ppMaskData3D[z][xy] = 0;
						}
					}
				}
			}
		}

		for (int i = 0; i < nD; ++i)
			delete[] ppTempMask[i];
		delete[] ppTempMask;

		duration6 = (std::clock() - start6) / (double)CLOCKS_PER_SEC;
		std::cout << "Closing time = " << duration6 << '\n';
	}

	////////////////////////////////////////////////////////////////////////
	///////////////////////// 7-1. 3D Hole filling /////////////////////////
	////////////////////////////////////////////////////////////////////////

	std::clock_t start7;
	double duration7 = 0;
	start7 = std::clock();

	// VOI 좌측 상단 끝을 seed로 push
	if (CHECK_FOR_SEGMENTABLE(ppLabel[nMinZ][nMinY*nW + nMinX])) {
		pnt.x = nMinX;
		pnt.y = nMinY;
		pnt.z = nMinZ;
		Q.push(pnt); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
	}

	// VOI 우측 하단 끝을 seed로 push
	if (CHECK_FOR_SEGMENTABLE(ppLabel[nMaxZ][nMaxY*nW + nMaxX])) {
		pnt.x = nMaxX - 1;
		pnt.y = nMaxY - 1;
		pnt.z = nMaxZ - 1;
		Q.push(pnt); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
	}

	register unsigned char nDataByte;

	// inverted region growing으로 3D hole filling 수행
	do {
		TPoint3D<int> Q_Point = Q.front();
		Q.pop();

		int x_pos = Q_Point.x;
		int y_pos = Q_Point.y;
		int z_pos = Q_Point.z;
		nIndex = y_pos * nW;

		bool bTop_Connectivity = false;
		bool bBottom_Connectivity = false;
		bool bUp_Connectivity = false;
		bool bDown_Connectivity = false;

		bool bTop_Check = true;
		bool bBottom_Check = true;
		bool bUp_Check = true;
		bool bDown_Check = true;

		if (y_pos <= nMinY) bTop_Check = false;
		if (y_pos >= nMaxY - 1) bBottom_Check = false;

		for (int x = x_pos; x >= nMinX; --x) {
			int nIndexNeighbor = nIndex + x;
			if (!CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nIndexNeighbor])) break;

			nDataByte = ppMaskData3D[z_pos][nIndexNeighbor];

			if (nDataByte == 0) {
				ppLabel[z_pos][nIndexNeighbor] |= LABEL_SEGMENTING; //그 점에 레이블을 준다.

				if (bTop_Check) {
					int nRefIndex = nIndexNeighbor - nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						unsigned char nRefData = ppMaskData3D[z_pos][nRefIndex];
						if (nRefData == 0) {
							if (!bTop_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos - 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bTop_Connectivity = true;
							}
						} else bTop_Connectivity = false;
					} else bTop_Connectivity = false;
				}
				if (bBottom_Check) {
					int nRefIndex = nIndexNeighbor + nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						unsigned char nRefData = ppMaskData3D[z_pos][nRefIndex];
						if (nRefData == 0) {
							if (!bBottom_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos + 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bBottom_Connectivity = true;
							}
						} else bBottom_Connectivity = false;
					} else bBottom_Connectivity = false;
				}
				if (bUp_Check) {
					if ((z_pos - 1 >= nMinZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos - 1][nIndexNeighbor])) {
						unsigned char nRefData = ppMaskData3D[z_pos - 1][nIndexNeighbor];
						if (nRefData == 0) {
							if (!bUp_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos - 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bUp_Connectivity = true;
							}
						} else bUp_Connectivity = false;
					} else bUp_Connectivity = false;
				}
				if (bDown_Check) {
					if ((z_pos + 1 < nMaxZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos + 1][nIndexNeighbor])) {
						unsigned char nRefData = ppMaskData3D[z_pos + 1][nIndexNeighbor];
						if (nRefData == 0) {
							if (!bDown_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos + 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bDown_Connectivity = true;
							}
						} else bDown_Connectivity = false;
					} else bDown_Connectivity = false;
				}
			} else
				break;
		}

		bUp_Connectivity = bDown_Connectivity = bTop_Connectivity = bBottom_Connectivity = false;

		for (int x = x_pos + 1; x < nMaxX; ++x) {
			int nIndexNeighbor = nIndex + x;
			if (!CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nIndexNeighbor])) break;

			nDataByte = ppMaskData3D[z_pos][nIndexNeighbor];

			if (nDataByte == 0) {
				ppLabel[z_pos][nIndexNeighbor] |= LABEL_SEGMENTING; //그 점에 레이블을 준다.

				if (bTop_Check) {
					int nRefIndex = nIndexNeighbor - nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						unsigned char nRefData = ppMaskData3D[z_pos][nRefIndex];
						if (nRefData == 0) {
							if (!bTop_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos - 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bTop_Connectivity = true;
							}
						} else bTop_Connectivity = false;
					} else bTop_Connectivity = false;
				}
				if (bBottom_Check) {
					int nRefIndex = nIndexNeighbor + nW;
					if (CHECK_FOR_SEGMENTABLE(ppLabel[z_pos][nRefIndex])) {
						unsigned char nRefData = ppMaskData3D[z_pos][nRefIndex];
						if (nRefData == 0) {
							if (!bBottom_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos + 1;
								PointTemp.z = z_pos;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bBottom_Connectivity = true;
							}
						} else bBottom_Connectivity = false;
					} else bBottom_Connectivity = false;
				}
				if (bUp_Check) {
					if ((z_pos - 1 >= nMinZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos - 1][nIndexNeighbor])) {
						unsigned char nRefData = ppMaskData3D[z_pos - 1][nIndexNeighbor];
						if (nRefData == 0) {
							if (!bUp_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos - 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bUp_Connectivity = true;
							}
						} else bUp_Connectivity = false;
					} else bUp_Connectivity = false;
				}
				if (bDown_Check) {
					if ((z_pos + 1 < nMaxZ) && CHECK_FOR_SEGMENTABLE(ppLabel[z_pos + 1][nIndexNeighbor])) {
						unsigned char nRefData = ppMaskData3D[z_pos + 1][nIndexNeighbor];
						if (nRefData == 0) {
							if (!bDown_Connectivity) {
								PointTemp.x = x;
								PointTemp.y = y_pos;
								PointTemp.z = z_pos + 1;
								Q.push(PointTemp); // 이웃한 점이 임계치 이내이면 q 에 입력 (Seed 확장)
								bDown_Connectivity = true;
							}
						} else bDown_Connectivity = false;
					} else bDown_Connectivity = false;
				}
			} else break;
		}
	} while (!Q.empty()); // Q에 들어간 point가 소진될 때까지

#pragma omp parallel for
	for (int k = nMinZ; k < nMaxZ; ++k) {
		for (int j = nMinY; j < nMaxY; ++j) {
			for (int i = nMinX; i < nMaxX; ++i) {
				int idx = j * nW + i;
				if (ppMaskData3D[k][idx] == 1) continue;

				if (ppLabel[k][idx] != LABEL_SEGMENTING) {
					ppMaskData3D[k][idx] = 1;
				}
			}
		}
	}

#pragma omp parallel for
	for (int i = 0; i < nD; ++i) {
		delete[] ppLabel[i];
		delete[] ppProtect[i];
	}
	delete[] ppLabel;
	delete[] ppProtect;

	duration7 = (std::clock() - start7) / (double)CLOCKS_PER_SEC;
	std::cout << "hole filling time = " << duration7 << '\n';

	// 8. center line 기준으로 단면적 구해서 각 voxel에 저장 (단면적 구할때 2D CCL로 각 component별로 단면적 구하기)
	std::clock_t start8;
	double duration8 = 0;
	start8 = std::clock();

	//단면적 값 저장용 볼륨 데이터 생성 및 초기화
	AreaType** ppAreaVol = new AreaType*[nD];
#pragma omp parallel for
	for (int i = 0; i < nD; ++i) {
		ppAreaVol[i] = new AreaType[nWH];
		memset(ppAreaVol[i], 0, sizeof(AreaType)*nWH);
	}

	//center line 기준으로 500x500 크기의 image plane
	int nPathNum = static_cast<int>(vecPath.size());
	int nImageSize = 500;
	fPlaneWidth = static_cast<float>(nImageSize);
	fPlaneHeight = static_cast<float>(nImageSize);
	nPlaneWidth = nImageSize;
	nPlaneHeight = nImageSize;
	int nSliceSize = nImageSize * nImageSize;

	//input center line이 정수단위이므로, 부드러운 곡선을 위해
	//input point 사이 사이에 laplacian 방법으로 interpolation point들을 추가하여
	//보다 촘촘하고 부드럽게 계산 가능
	std::vector<glm::vec3> vecNormalLin;
	int nIdxNormal = 0;
	for (int idx = 1; idx < vecCP.size(); ++idx) {
		glm::vec3 vNorm = vecCP.at(idx) - vecCP.at(idx - 1);
		//vecNormalLin.push_back(vNorm.Normalized());
		vecNormalLin.push_back(glm::normalize(vNorm));
	}
	std::vector<glm::vec3> vecNormal_s;
	for (int i = 0; i < nPathNum; ++i) {
		vecNormal_s.push_back(vecNormalLin.at(nIdxNormal));
		glm::vec3 vDiff = vecCP.at(nIdxNormal + 1) - vecCopy.at(i);
		if (vDiff.x < 0.00001f && vDiff.y < 0.00001f && vDiff.z < 0.00001f) {
			if (nIdxNormal < vecNormalLin.size() - 1)
				++nIdxNormal;
		}
	}
	vecNormalLin.clear();

#if defined(_OPENMP)
	int numThread = omp_get_max_threads();
#else
	int numThread = 1;
#endif
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int i = 0; i < nPathNum; ++i) {
		//center line 기준으로 500x500 크기의 image plane 생성
		//parallelize를 위해 for문 내부에 image plane 변수들을 생성
		unsigned char* inputImage = new unsigned char[nSliceSize];
		unsigned int* outputImage = new unsigned int[nSliceSize];
		AreaType* AreaImage = new AreaType[nSliceSize];
		memset(inputImage, 0, sizeof(unsigned char)*nSliceSize);
		memset(outputImage, 0, sizeof(unsigned int)*nSliceSize);
		memset(AreaImage, 0, sizeof(AreaType)*nSliceSize);

		//interpolation point들을 포함한 input vertex를 기준으로 plane's normal vector 정의
		// 해당 normal vector를 기준으로 plane's right, back vector를 추가로 정의함
		glm::vec3 vecNormal = vecNormal_s.at(i);
		glm::vec3 vR;
		if (vecNormal.z != 0.0f)
			vR = glm::vec3(1, 1, (-vecNormal.x - vecNormal.y) / vecNormal.z);
		else if (vecNormal.y != 0.0f)
			vR = glm::vec3(1, (-vecNormal.x - vecNormal.z) / vecNormal.y, 1);
		else if (vecNormalS.x != 0.0f)
			vR = glm::vec3((-vecNormal.y - vecNormal.z) / vecNormal.x, 1, 1);
		else {
			continue;
		}
		//vR.Normalize();
		vR = glm::normalize(vR);

		// 가상의 plane상에서 unit distance 만큼씩 이동(Right, Back direction)하여 volume 의 값을 얻어올 수 있음
		//glm::vec3 vB = glm::vec3::CrossProduct(vR, vecNormal).Normalized();
		glm::vec3 vB = glm::normalize(glm::cross(vR, vecNormal));
		glm::vec3 ptPlaneStart = -fPlaneWidth * vR*0.5f - fPlaneHeight * vB*0.5f + vecPath.at(i);
		for (int v = 0; v < nPlaneHeight; ++v) {
			glm::vec3  ptPlane = ptPlaneStart + vB * static_cast<float>(v);
			for (int u = 0; u < nPlaneWidth; ++u) {
				int iX = static_cast<int>(ptPlane.x + 0.5f);
				int iY = static_cast<int>(ptPlane.y + 0.5f);
				int iZ = static_cast<int>(ptPlane.z + 0.5f);
				if (iX >= 0 && iX < nW &&
					iY >= 0 && iY < nH &&
					iZ >= 0 && iZ < nD) {
					inputImage[v*nPlaneWidth + u] = ppMaskData3D[iZ][iY*nW + iX];
				}
				ptPlane += vR;
			}
		}

		//reconstruction 된 image plane으로, CCL -> 단면적 계산을 진행
		int nLabelNum = run2DCCL(inputImage, outputImage, nImageSize, nImageSize);
		CountArea(outputImage, AreaImage, nLabelNum, nImageSize, nImageSize, fPS_SQ);

		for (int v = 0; v < nPlaneHeight; ++v) {
			glm::vec3 ptPlane = ptPlaneStart + vB * static_cast<float>(v);
			for (int u = 0; u < nPlaneWidth; ++u) {
				int iX = static_cast<int>(ptPlane.x + 0.5f);
				int iY = static_cast<int>(ptPlane.y + 0.5f);
				int iZ = static_cast<int>(ptPlane.z + 0.5f);

				if (iX >= 0 && iX < nW &&
					iY >= 0 && iY < nH &&
					iZ >= 0 && iZ < nD) {
					int nVolIdx = iY * nW + iX;
					AreaType valAt = AreaImage[v*nPlaneWidth + u];
					if (ppAreaVol[iZ][nVolIdx] == 0 && valAt < 800 && valAt > 5)
						ppAreaVol[iZ][nVolIdx] = valAt;
				}
				ptPlane += vR;
			}
		}

		delete[] inputImage;
		delete[] outputImage;
		delete[] AreaImage;
	}
	vecNormal_s.clear();

	// control point들이 급격하게 꺾이는 지점에서 값을 보간해주기 위한 코드
	// 위의 과정과 유사함
	for (int j = 1; j < (int)vecCP.size() - 1; ++j) {
		int idxAt;
		bool bIdxFind = false;
		for (idxAt = 0; idxAt < nPathNum; ++idxAt) {
			glm::vec3 vDiff = vecCP.at(j) - vecCopy.at(idxAt);
			if (std::abs(vDiff.x) < 0.0001f && std::abs(vDiff.y) < 0.0001f && std::abs(vDiff.z) < 0.0001f) {
				bIdxFind = true;
				break;
			}
		}

		if (!bIdxFind)
			continue;

		omp_set_num_threads(numThread);

#pragma omp parallel for
		for (int i = idxAt - 15; i < idxAt + 15; ++i) {
			if (i < 0 || i > nPathNum)
				continue;

			unsigned char* inputImage = new unsigned char[nSliceSize];
			unsigned int* outputImage = new unsigned int[nSliceSize];
			AreaType* AreaImage = new AreaType[nSliceSize];
			memset(inputImage, 0, sizeof(unsigned char)*nSliceSize);
			memset(outputImage, 0, sizeof(unsigned int)*nSliceSize);
			memset(AreaImage, 0, sizeof(AreaType)*nSliceSize);

			glm::vec3 vecNormal;
			vecNormal = vecPath.at(i + 4) - vecPath.at(i - 4);

			glm::vec3 vR;
			if (vecNormal.z != 0.0f)
				vR = glm::vec3(1, 1, (-vecNormal.x - vecNormal.y) / vecNormal.z);
			else if (vecNormal.y != 0.0f)
				vR = glm::vec3(1, (-vecNormal.x - vecNormal.z) / vecNormal.y, 1);
			else if (vecNormalS.x != 0.0f)
				vR = glm::vec3((-vecNormal.y - vecNormal.z) / vecNormal.x, 1, 1);
			else {
				continue;
			}
			//vR.Normalize();
			vR = glm::normalize(vR);

			//glm::vec3 vB = glm::vec3::CrossProduct(vR, vecNormal).Normalized();
			glm::vec3 vB = glm::normalize(glm::cross(vR, vecNormal));
			glm::vec3 ptPlaneStart = -fPlaneWidth * vR*0.5f - fPlaneHeight * vB*0.5f + vecPath.at(i);

			for (int v = 0; v < nPlaneHeight; ++v) {
				glm::vec3  ptPlane = ptPlaneStart + vB * static_cast<float>(v);
				for (int u = 0; u < nPlaneWidth; ++u) {
					int iX = static_cast<int>(ptPlane.x + 0.5f);
					int iY = static_cast<int>(ptPlane.y + 0.5f);
					int iZ = static_cast<int>(ptPlane.z + 0.5f);
					// Nearest-neighbor sampling.
					if (iX >= 0 && iX < nW &&
						iY >= 0 && iY < nH &&
						iZ >= 0 && iZ < nD) {
						inputImage[v*nPlaneWidth + u] = ppMaskData3D[iZ][iY*nW + iX];
					}
					ptPlane += vR;
				}
			}

			int nLabelNum = run2DCCL(inputImage, outputImage, nImageSize, nImageSize);
			CountArea(outputImage, AreaImage, nLabelNum, nImageSize, nImageSize, fPS_SQ);

			for (int v = 0; v < nPlaneHeight; ++v) {
				glm::vec3 ptPlane = ptPlaneStart + vB * static_cast<float>(v);
				for (int u = 0; u < nPlaneWidth; ++u) {
					int iX = static_cast<int>(ptPlane.x + 0.5f);
					int iY = static_cast<int>(ptPlane.y + 0.5f);
					int iZ = static_cast<int>(ptPlane.z + 0.5f);

					if (iX >= 0 && iX < nW &&
						iY >= 0 && iY < nH &&
						iZ >= 0 && iZ < nD) {
						AreaType valAt = AreaImage[v*nPlaneWidth + u];
						if (valAt < 800 && valAt > 5)
							ppAreaVol[iZ][iY*nW + iX] = valAt;
					}
					ptPlane += vR;
				}
			}

			delete[] inputImage;
			delete[] outputImage;
			delete[] AreaImage;
		}
	}

	duration8 = (std::clock() - start8) / (double)CLOCKS_PER_SEC;
	std::cout << "Area calculation time = " << duration8 << '\n';

	glm::vec3 vMin(static_cast<float>(nMinX), static_cast<float>(nMinY), static_cast<float>(nMinZ));
	glm::vec3 vMax(static_cast<float>(nMaxX), static_cast<float>(nMaxY), static_cast<float>(nMaxZ));
	int nWSub = nMaxX - nMinX + 2;
	int nHSub = nMaxY - nMinY + 2;
	int nDSub = nMaxZ - nMinZ + 2;
	int nWHSub = nWSub * nHSub;
	float fSigma = 1.0f;

	// 9. Smoothing 수행
	m_meshSTL.clear();

	//create subvolume for smoothing
	unsigned char** ppIsoVol = new unsigned char*[nDSub];
	for (int i = 0; i < nDSub; ++i) {
		ppIsoVol[i] = new unsigned char[nWHSub];
		memset(ppIsoVol[i], 0, sizeof(unsigned char)*nWHSub);
	}

	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int k = 1; k < nDSub - 1; ++k) {
		for (int j = 1; j < nHSub - 1; ++j) {
			for (int i = 1; i < nWSub - 1; ++i) {
				if (ppMaskData3D[nMinZ + k - 1][(nMinY + j - 1)*nW + (nMinX + i - 1)] != 0)
					ppIsoVol[k][j*nWSub + i] = 255;
			}
		}
	}

	std::clock_t start11;
	double duration11 = 0;
	start11 = std::clock();

	//smoothing 된 mesh extaction 을 위해 input volume 에 gaussian smoothing을 적용함.
	getGaussianVolume(ppIsoVol, nWSub, nHSub, nDSub, fPixelSpacing);

	duration11 = (std::clock() - start11) / (double)CLOCKS_PER_SEC;
	std::cout << "getGaussianVolume time = " << duration11 << '\n';
	std::clock_t start12;
	double duration12 = 0;
	start12 = std::clock();

	std::clock_t start9;
	double duration9 = 0;
	start9 = std::clock();

	// 10. 단면적 color mapping 된 Marching Cube 알고리즘 수행
	// marching cube algorithm은 http://paulbourke.net/geometry/polygonise/ 참조
	// 결과물 mesh는 (STL형태 + color value) 로 m_meshSTL 에 저장된다.
	//CMC MCCube;
	MarchingCube MCCube;

#if 1
	glm::vec3 vStart(0.0f, 0.0f, 0.0f);
	float fSzCube = fPixelSpacing / 1.0f; //fPixelSpacing / fScale
	float fIsoVal = 128.0f;
	glm::vec3 vCubeSZ(fSzCube, fSzCube, fSzCube);
	MCCube.initializeAirway(vStart, vCubeSZ, fIsoVal);

	AreaType vTempSum = 0;
	for (int idxSubD = 0, d = nMinZ - 1; idxSubD < nDSub - 1; ++d, ++idxSubD) {
		for (int idxSubH = 0, h = nMinY - 1; idxSubH < nHSub - 1; ++h, ++idxSubH) {
			for (int idxSubW = 0, w = nMinX - 1; idxSubW < nWSub - 1; ++w, ++idxSubW) {
				int Y0 = idxSubH * nWSub;
				int Y1 = (idxSubH + 1)*nWSub;
				MCCube.setCubeVerticesPos(w, h, d);
				MCCube.setVertexVal(0, (float)ppIsoVol[idxSubD][Y0 + idxSubW]);
				MCCube.setVertexVal(1, (float)ppIsoVol[idxSubD][Y0 + idxSubW + 1]);
				MCCube.setVertexVal(2, (float)ppIsoVol[idxSubD][Y1 + idxSubW + 1]);
				MCCube.setVertexVal(3, (float)ppIsoVol[idxSubD][Y1 + idxSubW]);
				MCCube.setVertexVal(4, (float)ppIsoVol[idxSubD + 1][Y0 + idxSubW]);
				MCCube.setVertexVal(5, (float)ppIsoVol[idxSubD + 1][Y0 + idxSubW + 1]);
				MCCube.setVertexVal(6, (float)ppIsoVol[idxSubD + 1][Y1 + idxSubW + 1]);
				MCCube.setVertexVal(7, (float)ppIsoVol[idxSubD + 1][Y1 + idxSubW]);

				int nIdxOutside = MCCube.calculateIndex();
				if (MCCube.isSurfaceCube()) {     // if the cube includes the surface
					// get area val
					unsigned short areaVal = 0;
					for (int idxInside = 0; idxInside < 8; ++idxInside) {
						if (!(nIdxOutside & bitFlags[idxInside])) {
							switch (idxInside) {
							case 0:	areaVal = ppAreaVol[d][h*nW + w];			break;
							case 1:	areaVal = ppAreaVol[d][h*nW + w + 1];		break;
							case 2:	areaVal = ppAreaVol[d][(h + 1)*nW + w + 1];	break;
							case 3:	areaVal = ppAreaVol[d][(h + 1)*nW + w];		break;
							case 4:	areaVal = ppAreaVol[d + 1][h*nW + w];			break;
							case 5: areaVal = ppAreaVol[d + 1][h*nW + w + 1];		break;
							case 6:	areaVal = ppAreaVol[d + 1][(h + 1)*nW + w + 1];	break;
							case 7:	areaVal = ppAreaVol[d + 1][(h + 1)*nW + w];		break;
							}
							if (areaVal != 0)
								break;
						}
					}
					int cntValues = 0;
					unsigned int vSum = 0;
					for (int z = -3; z < 4; ++z) {
						int pX, pY, pZ;
						for (int y = -3; y < 4; ++y) {
							for (int x = -3; x < 4; ++x) {
								pX = w + x;
								pY = h + y;
								pZ = d + z;
								if (pX >= 0 && pX < nW && pY >= 0 && pY < nH && pZ >= 0 && pZ < nD) {
									if (ppAreaVol[pZ][pY*nW + pX] != 0) {
										++cntValues;
										vSum += ppAreaVol[pZ][pY*nW + pX];
									}
								}
							}
						}
					}
					if (cntValues == 0)
						continue;

					//MCCube.polygonize(m_meshSTL, vSum/cntValues);
					if (vSum != 0) {
						MCCube.polygonize(m_meshSTL, vSum / cntValues);
						vTempSum = vSum / cntValues;
					} else
						MCCube.polygonize(m_meshSTL, vTempSum);
				}
			}
		}
	}

	for (int i = 0; i < nD; ++i) {
		delete[] ppAreaVol[i];
	}
	delete[] ppAreaVol;
#else
#endif

	for (int i = 0; i < nDSub; i++)
		delete[] ppIsoVol[i];
	delete[] ppIsoVol;

	// 단면적 값을 가시화 하기 위한 color mapping code.
	// black - red - orange - yellow - green - blue - white 순으로 mapped
	for (int idx = 0; idx < m_meshSTL.size(); ++idx) {
		m_meshSTL.at(idx).nColorVal *= pow(nDownFactor, 2);
		AreaType colorVal = m_meshSTL.at(idx).nColorVal;
		//f3 vColor;
		glm::vec3 vColor;
		if (colorVal <= 100) {
			vColor.x = 0.25f;
			vColor.y = 0.1f;
			vColor.z = 0.1f;
		} else if (colorVal > 700) {
			vColor.x = 1.0f;
			vColor.y = 1.0f;
			vColor.z = 1.0f;
		} else {
			float fColorVal = static_cast<float>(colorVal);
			if (colorVal > 100 && colorVal <= 200) {			//black -> red
				vColor.x = 1.0f - (200.0f - fColorVal) / 100.0f;
				vColor.y = 0.0f;
				vColor.z = 0.0f;
			} else if (colorVal > 200 && colorVal <= 300) {		//red -> orange
				vColor.x = 1.0f;
				vColor.y = 0.5f*(1.0f - (300.0f - fColorVal) / 100.0f);
				vColor.z = 0.0f;
			} else if (colorVal > 300 && colorVal <= 400) {		//orange -> yellow
				vColor.x = 1.0f;
				vColor.y = 0.5f*(1.0f - (400.0f - fColorVal) / 100.0f) + 0.5f;
				vColor.z = 0.0f;
			} else if (colorVal > 400 && colorVal <= 500) {		//yellow -> green
				vColor.x = (500.0f - fColorVal) / 100.0f;
				vColor.y = 1.0f;
				vColor.z = 0.0f;
			} else if (colorVal > 500 && colorVal <= 600) {		//green -> blue
				vColor.x = 0.0f;
				vColor.y = (600.0f - fColorVal) / 100.0f;
				vColor.z = 1.0f - (600.0f - fColorVal) / 100.0f;
			} else if (colorVal > 600 && colorVal <= 700) {		//blue ->white
				vColor.x = 1.0f - (700.0f - fColorVal) / 100.0f;
				vColor.y = 1.0f - (700.0f - fColorVal) / 100.0f;
				vColor.z = 1.0f;
			}
		}
		m_meshSTL.at(idx).fColor = vColor;
	}

	duration9 = (std::clock() - start9) / (double)CLOCKS_PER_SEC;
	std::cout << "Surface extract time = " << duration9 << '\n';

	double total_duration = (std::clock() - total_start) / (double)CLOCKS_PER_SEC;
	std::cout << "====================================" << '\n';
	std::cout << "total time = " << total_duration << '\n';

	return true;
}

int CAirwaySeg::run2DCCL(unsigned char* input, unsigned int* output, int nW, int nH) {
	unsigned short* STACK = new unsigned short[3 * (nW*nH + 1)];

	int labelNo = 0;
	int index = -1;
	for (unsigned short y = 0; y < nH; y++) {
		for (unsigned short x = 0; x < nW; x++) {
			++index;
			if (input[index] == 0) continue;   /* This pixel is not part of a component */
			if (output[index] != 0) continue;   /* This pixel has already been labelled  */
			/* New component found */
			++labelNo;
			LabelComponent(STACK, input, output, nW, nH, labelNo, x, y);
		}
	}

	delete[] STACK;

	return labelNo;
}

#define CALL_LabelComponent(x,y,returnLabel) { STACK[SP] = x; STACK[SP+1] = y; STACK[SP+2] = returnLabel; SP += 3; goto START; }
#define RETURN { SP -= 3;  switch (STACK[SP+2])    {    case 1 : goto RETURN1;  case 2 : goto RETURN2;  case 3 : goto RETURN3;  case 4 : goto RETURN4;  default: return;} }
#define X (STACK[SP-3])
#define Y (STACK[SP-2])

void CAirwaySeg::LabelComponent(unsigned short* STACK, unsigned char* input, unsigned int* output, int nW, int nH, int labelNo, unsigned short x, unsigned short y) {
	STACK[0] = x;
	STACK[1] = y;
	STACK[2] = 0;  /* return - component is labelled */
	int SP = 3;
	int index;

START: /* Recursive routine starts here */

	index = X + nW * Y;
	if (input[index] == 0) RETURN;   /* This pixel is not part of a component */
	if (output[index] != 0) RETURN;   /* This pixel has already been labelled  */
	output[index] = labelNo;

	if (X > 0) CALL_LabelComponent(X - 1, Y, 1);   /* left  pixel */
RETURN1:

	if (X < nW - 1) CALL_LabelComponent(X + 1, Y, 2);   /* right pixel */
RETURN2:

	if (Y > 0) CALL_LabelComponent(X, Y - 1, 3);   /* upper pixel */
RETURN3:

	if (Y < nH - 1) CALL_LabelComponent(X, Y + 1, 4);   /* lower pixel */
RETURN4:

	RETURN;
}

void CAirwaySeg::CountArea(unsigned int* outputImage, AreaType* AreaImage, int nLabelNum, int nW, int nH, float fPS_SQ) {
	unsigned int* pLabelCount = new unsigned int[nLabelNum + 1];
	memset(pLabelCount, 0, sizeof(unsigned int)*(nLabelNum + 1));
	int nWH = nW * nH;
	for (int i = 0; i < nWH; ++i) {
		if (outputImage[i] != 0)
			++pLabelCount[outputImage[i]];
	}

	for (int i = 0; i < nWH; ++i) {
		if (outputImage[i] != 0)
			AreaImage[i] = static_cast<AreaType>(static_cast<float>(pLabelCount[outputImage[i]])*fPS_SQ);
	}

	delete[] pLabelCount;
}
void CAirwaySeg::getGaussianVolume(unsigned char** ppInput, int nW, int nH, int nD, float fPixelSpacing) {
	// min : 1.3f, max : 3.0, kSize : max 11x11x11
	float fSigma = 1.3f;
	fSigma = (fPixelSpacing < 0.2f) ? 3.0f : ((fPixelSpacing > 0.8f) ? 1.3f : -2.83f*fPixelSpacing + 3.56f);
	//fSigma = (nD<=180)? 1.3f :((nD<800)? (1.3f+(3.0f-1.3f)*(nD-800)/(nD-800)):3.0f);
	//std::cout<<"gaussian sigma : "<<fSigma<<std::endl;

	// 1. get 1D Gaussian Mask
	int dim = static_cast<int>(std::max(3.0f, 2 * 4 * fSigma + 1.0f));
	//dim = dim > 11 ? 11 : dim;
	if (dim % 2 == 0) dim++;
	int dim2 = static_cast<int>(dim / 2);

	float* pMask = new float[dim];
	for (int i = 0; i < dim; ++i) {
		int x = i - dim2;
		pMask[i] = (float)(exp(-(x*x) / (2 * fSigma*fSigma)) / (sqrt(2.0*M_PI)*fSigma));
	}

	// 2. create temp memory buffer
	float** ppTempBuff = new float*[nD];
	float** ppTempBuff2 = new float*[nD];
	unsigned char** ppVolFiltered = new unsigned char*[nD];
	int nWHTmp = nW * nH;
	for (int d = 0; d < nD; ++d) {
		ppTempBuff[d] = new float[nWHTmp];
		ppTempBuff2[d] = new float[nWHTmp];
		ppVolFiltered[d] = new unsigned char[nWHTmp];
		std::memset(ppTempBuff[d], 0, sizeof(float)*nWHTmp);
		std::memset(ppTempBuff2[d], 0, sizeof(float)*nWHTmp);
		std::memset(ppVolFiltered[d], 0, sizeof(unsigned char)*nWHTmp);
	}

	// 3. mask operation : vertically
#if defined(_OPENMP)
	int numThread = omp_get_max_threads();
#else
	int numThread = 1;
#endif
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int m = 1; m < nD - 1; ++m) {
		float sum1, sum2;
		int x, idx;
		for (int i = 1; i < nW - 1; ++i) {
			for (int j = 1; j < nH - 1; ++j) {
				sum1 = sum2 = 0.0f;
				for (int k = 0; k < dim; ++k) {
					x = k - dim2 + j;
					idx = i + x * nW;
					if (x >= 0 && x < nH) {
						sum1 += pMask[k];
						sum2 += (pMask[k] * ppInput[m][idx]);
					}
				}
				idx = i + j * nW;
				ppTempBuff[m][idx] = (sum2 / sum1);
			}
		}
	}

	// 5. mask operation : horizontally
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int m = 1; m < nD - 1; ++m) {
		float sum1, sum2;
		int x, idx;
		for (int j = 1; j < nH - 1; ++j) {
			for (int i = 1; i < nW - 1; ++i) {
				sum1 = sum2 = 0.0f;
				for (int k = 0; k < dim; ++k) {
					x = k - dim2 + i;
					idx = x + j * nW;
					if (x >= 0 && x < nW) {
						sum1 += pMask[k];
						sum2 += (pMask[k] * ppTempBuff[m][idx]);
					}
				}
				idx = i + j * nW;
				ppTempBuff2[m][idx] = (sum2 / sum1);
			}
		}
	}

	// 6. mask operation : depth
	omp_set_num_threads(numThread);

#pragma omp parallel for
	for (int j = 1; j < nH - 1; ++j) {
		float sum1, sum2;
		int x, idx;
		for (int i = 1; i < nW - 1; ++i) {
			for (int m = 1; m < nD - 1; ++m) {
				sum1 = sum2 = 0.0f;
				for (int k = 0; k < dim; ++k) {
					x = k - dim2 + m;
					idx = i + j * nW;
					if (x >= 0 && x < nD) {
						sum1 += pMask[k];
						sum2 += (pMask[k] * ppTempBuff2[x][idx]);
					}
				}
				idx = i + j * nW;
				ppVolFiltered[m][idx] = (unsigned char)(sum2 / sum1);
			}
		}
	}

	for (int d = 1; d < nD - 1; ++d) {
		unsigned char* pVolFiltered = ppVolFiltered[d];
		unsigned char* pIsoVol = ppInput[d];
		std::memcpy(pIsoVol, pVolFiltered, sizeof(unsigned char)*nH*nW);
	}

	for (int i = 0; i < nD; ++i) {
		delete[] ppTempBuff[i];
		delete[] ppTempBuff2[i];
		delete[] ppVolFiltered[i];
	}
	delete[] ppTempBuff;
	delete[] ppTempBuff2;
	delete[] ppVolFiltered;
	delete[] pMask;
}
