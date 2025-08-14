#include "VpMorph.h"

#include <numeric>
#include "../../../Common/Common/W3Memory.h"
#include "VpImageAPI.inl"
#include "GxConnectedComponentLabeling.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVpMorph::CVpMorph()
	: m_bytImageValue(255) {}

CVpMorph::~CVpMorph() {}

bool CVpMorph::Dilation3D(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

	unsigned char **ppTemp = NULL;
	if (!SafeNew2D< unsigned char>(ppTemp, w*h, d)) return false;

	// 2. loop all voxel
	int nHalfSESize = nSESize >> 1;
#pragma omp parallel for
	for (int z = nHalfSESize; z < d - nHalfSESize; z++) {
		for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
				if (m_ppbytData[z][x + y * w] == 0)
					continue;

				//int sum = 0;
				for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
					for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
						for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
							int idxx = x + l;
							int idxy = y + m;
							int idxz = z + n;
							int idxxy = idxx + idxy * w;

							if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

							//sum += m_ppbytData[idxz][idxxy];
							ppTemp[idxz][idxxy] = m_bytImageValue;
						}
					}
				}
				// http://homepages.inf.ed.ac.uk/rbf/HIPR2/dilate.htm
				// 1. 3x3 안의 픽셀들의 합이 0이 아니면 모두 m_bytImageValue로 채우는가?
				// 2. 현재 픽셀(m_ppbytData[z][xy])이 0이 아닐 때만 적용해야 하는 것 아닌지?
				// 1과 2의 결과 별 차이 없음

#if 0
				if (sum > 0) {
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * w;

								if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

								ppTemp[idxz][idxxy] = m_bytImageValue;
							}
						}
					}
				}
#endif
			}
		}
	}
	for (int z = 0; z < d; z++) {
		memcpy(m_ppbytData[z], ppTemp[z], sizeof(unsigned char)*w*h);
	}

	SAFE_DELETE_VOLUME(ppTemp, d);

	return true;
}

bool CVpMorph::Erosion3D(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

	unsigned char **ppTemp = NULL;
	if (!SafeNew2D< unsigned char>(ppTemp, w*h, d)) return false;

	CImage3D< unsigned char> img3d;
	img3d.Set(w, h, d, ppTemp);
	img3d.Copy(m_ppbytData);

	// 2. loop all voxel
	int nHalfSESize = nSESize >> 1;
	int nFullSESize = nSESize * nSESize*nSESize;
	int nFullSEValue = m_bytImageValue * nFullSESize;

#pragma omp parallel for
	for (int z = nHalfSESize; z < d - nHalfSESize; z++) {
		for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
				int sum = 0;
				for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
					for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
						for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
							int idxx = x + l;
							int idxy = y + m;
							int idxz = z + n;
							int idxxy = idxx + idxy * w;

							if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

							sum += m_ppbytData[idxz][idxxy];
						}
					}
				}

				if (sum != nFullSEValue) {
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * w;

								if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

								ppTemp[idxz][idxxy] = 0;
							}
						}
					}
				}
			}
		}
	}
	for (int z = 0; z < d; z++) {
		memcpy(m_ppbytData[z], ppTemp[z], sizeof(unsigned char)*w*h);
	}

	SAFE_DELETE_VOLUME(ppTemp, d);

	return true;
}

bool CVpMorph::Dilation2D(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;

	unsigned char *pTemp = NULL;
	if (!SafeNews< unsigned char>(pTemp, w*h)) return false;

	int nHalfSESize = nSESize >> 1;

#pragma omp parallel for
	for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
		for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
			int sum = 0;
			//
			for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
				for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
					int idxx = x + l;
					int idxy = y + m;
					int idxxy = idxx + idxy * w;

					if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

					sum += m_pbytData[idxxy];
				}
			}
			//
			if (sum > 0) {
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * w;

						if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

						pTemp[idxxy] = m_bytImageValue;
					}
				}
			}
		}
	}
	memcpy(m_pbytData, pTemp, sizeof(unsigned char)*w*h);
	SAFE_DELETE_ARRAY(pTemp);

	return true;
}

bool CVpMorph::Erosion2D(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;

	unsigned char *pTemp = NULL;
	if (!SafeNews< unsigned char>(pTemp, w*h)) return false;
	memcpy(pTemp, m_pbytData, sizeof(unsigned char)*w*h);

	int nHalfSESize = nSESize >> 1;
	int nFullSESize = nSESize * nSESize;
	int nFullSEValue = m_bytImageValue * nFullSESize;

#pragma omp parallel for
	for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
		for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
			int sum = 0;
			//
			for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
				for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
					int idxx = x + l;
					int idxy = y + m;
					int idxxy = idxx + idxy * w;

					if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

					sum += m_pbytData[idxxy];
				}
			}
			//
			if (sum != nFullSEValue) {
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * w;

						if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

						pTemp[idxxy] = 0;
					}
				}
			}
		}
	}
	memcpy(m_pbytData, pTemp, sizeof(unsigned char)*w*h);
	SAFE_DELETE_ARRAY(pTemp);

	return true;
}

bool CVpMorph::Dilation3DperSlice(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

#pragma omp parallel for
	for (int z = 0; z < d; z++) {
		CVpMorph morph;
		morph.Init2D(w, h, m_ppbytData[z]);
		morph.Dilation2D(nSESize);
	}

	return true;
}

bool CVpMorph::Erosion3DperSlice(int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

#pragma omp parallel for
	for (int z = 0; z < d; z++) {
		CVpMorph morph;
		morph.Init2D(w, h, m_ppbytData[z]);
		morph.Erosion2D(nSESize);
	}
	return true;
}

void CVpMorph::Union3D(int zmin, int zmax, unsigned char** ppBinData2) {
	int wh = m_nWidth * m_nHeight;
	for (int z = zmin; z <= zmax; z++) {
		memcpy(m_ppbytData[z], ppBinData2[z], wh);
	}
}

void CVpMorph::Union3D(unsigned char** ppBinData2) {
	int wh = m_nWidth * m_nHeight;
	for (int z = 0; z < m_nDepth; z++) {
		for (int xy = 0; xy < wh; xy++) {
			if ((m_ppbytData[z][xy] != 0) || (ppBinData2[z][xy] != 0))	m_ppbytData[z][xy] = m_bytImageValue;
		}
	}
}

void CVpMorph::Substract3D(unsigned char** ppSubData) {
	int wh = m_nWidth * m_nHeight;
	for (int z = 0; z < m_nDepth; z++) {
		for (int xy = 0; xy < wh; xy++) {
			if (ppSubData[z][xy] != 0) {
				m_ppbytData[z][xy] = 0;
			}
		}
	}
}

void CVpMorph::Mask2D(unsigned char* pMask, eMaskOperator emaskop) {
	int wh = m_nWidth * m_nHeight;

	switch (emaskop) {
	case ADD:
		for (int xy = 0; xy < wh; xy++) {
			if (pMask[xy] != 0)
				m_pbytData[xy] = m_bytImageValue;
		}
		break;

	case SUBTRACT:
		for (int xy = 0; xy < wh; xy++) {
			if (pMask[xy] != 0)
				m_pbytData[xy] = 0;
		}
		break;

	default:
		break;
	}
}

void CVpMorph::Mask3D(unsigned char** pMask, eMaskOperator emaskop) {}

bool CVpMorph::Open2D(int nSESize) {
	if (!Erosion2D(nSESize)) return false;
	if (!Dilation2D(nSESize)) return false;

	return true;
}

bool CVpMorph::Close2D(int nSESize) {
	if (!Dilation2D(nSESize)) return false;
	if (!Erosion2D(nSESize)) return false;

	return true;
}

bool CVpMorph::Open3D(int nSESize) {
	if (!Erosion3D(nSESize)) return false;
	if (!Dilation3D(nSESize)) return false;

	return true;
}

bool CVpMorph::Close3D(int nSESize) {
	if (!Dilation3D(nSESize)) return false;
	if (!Erosion3D(nSESize)) return false;

	return true;
}

bool CVpMorph::Open2D(int nErosSize, int nDilSize) {
	if (!Erosion2D(nErosSize)) return false;
	if (!Dilation2D(nDilSize)) return false;

	return true;
}

bool CVpMorph::Close2D(int nErosSize, int nDilSize) {
	if (!Dilation2D(nDilSize)) return false;
	if (!Erosion2D(nErosSize)) return false;

	return true;
}

bool CVpMorph::Erosion2D(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;

	unsigned char *pTemp = NULL;
	if (!SafeNews< unsigned char>(pTemp, w*h)) return false;
	memcpy(pTemp, m_pbytData, sizeof(unsigned char)*w*h);

	int nHalfSESize = nSESize >> 1;
	int nFullSESize = nSESize * nSESize;
	//	int nFullSEValue = m_bytImageValue * nFullSESize;
	int nFullSEValue = val * nFullSESize;

#pragma omp parallel for
	for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
		for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
			int sum = 0;
			//
			if (m_pbytData[x + y * w] != val) {
				pTemp[x + y * w] = m_pbytData[x + y * w];
				continue;
			}
			for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
				for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
					int idxx = x + l;
					int idxy = y + m;
					int idxxy = idxx + idxy * w;

					if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

					sum += m_pbytData[idxxy];
				}
			}
			//
			if (sum != nFullSEValue) {
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * w;

						if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

						pTemp[idxxy] = 0;
					}
				}
			}
		}
	}
	memcpy(m_pbytData, pTemp, sizeof(unsigned char)*w*h);
	SAFE_DELETE_ARRAY(pTemp);

	return true;
}

bool CVpMorph::Dilation2D(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;

	unsigned char *pTemp = NULL;
	if (!SafeNews< unsigned char>(pTemp, w*h)) return false;

	int nHalfSESize = nSESize >> 1;

#pragma omp parallel for
	for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
		for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
			int sum = 0;
			if (m_pbytData[x + y * w] != val) {
				pTemp[x + y * w] = m_pbytData[x + y * w];
				continue;
			}
			for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
				for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
					int idxx = x + l;
					int idxy = y + m;
					int idxxy = idxx + idxy * w;

					if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

					sum += m_pbytData[idxxy];
				}
			}

			if (sum > 0) {
				for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
					for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
						int idxx = x + l;
						int idxy = y + m;
						int idxxy = idxx + idxy * w;

						if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1)) continue;

						//	pTemp[idxxy] = m_bytImageValue;
						pTemp[idxxy] = val;
					}
				}
			}
		}
	}
	memcpy(m_pbytData, pTemp, sizeof(unsigned char)*w*h);
	SAFE_DELETE_ARRAY(pTemp);

	return true;
}

bool CVpMorph::Erosion3D(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

	unsigned char **ppTemp = NULL;
	if (!SafeNew2D< unsigned char>(ppTemp, w*h, d))
		return false;

	CImage3D< unsigned char> img3d;
	img3d.Set(w, h, d, ppTemp);
	img3d.Copy(m_ppbytData);

	// 2. loop all voxel
	int nHalfSESize = nSESize >> 1;
	int nFullSESize = nSESize * nSESize*nSESize;
	//	int nFullSEValue = m_bytImageValue * nFullSESize;
	int nFullSEValue = val * nFullSESize;

#pragma omp parallel for
	for (int z = nHalfSESize; z < d - nHalfSESize; z++) {
		for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
				int sum = 0;
				if (m_ppbytData[z][x + y * w] != val) {
					ppTemp[z][x + y * w] = m_ppbytData[z][x + y * w];
					continue;
				}
				for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
					for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
						for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
							int idxx = x + l;
							int idxy = y + m;
							int idxz = z + n;
							int idxxy = idxx + idxy * w;

							if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

							sum += m_ppbytData[idxz][idxxy];
						}
					}
				}
				//
				if (sum != nFullSEValue) {
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * w;

								if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

								ppTemp[idxz][idxxy] = 0;
							}
						}
					}
				}
			}
		}
	}
	for (int z = 0; z < d; z++) {
		memcpy(m_ppbytData[z], ppTemp[z], sizeof(unsigned char)*w*h);
	}
	SAFE_DELETE_VOLUME(ppTemp, d);

	return true;
}

bool CVpMorph::Dilation3D(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

	unsigned char **ppTemp = NULL;
	if (!SafeNew2D< unsigned char>(ppTemp, w*h, d)) return false;

	// 2. loop all voxel
	int nHalfSESize = nSESize >> 1;
#pragma omp parallel for
	for (int z = nHalfSESize; z < d - nHalfSESize; z++) {
		for (int y = nHalfSESize; y < h - nHalfSESize; y++) {
			for (int x = nHalfSESize; x < w - nHalfSESize; x++) {
				int sum = 0;
				if (m_ppbytData[z][x + y * w] != val) {
					ppTemp[z][x + y * w] = m_ppbytData[z][x + y * w];
					continue;
				}
				//
				for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
					for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
						for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
							int idxx = x + l;
							int idxy = y + m;
							int idxz = z + n;
							int idxxy = idxx + idxy * w;

							if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

							sum += m_ppbytData[idxz][idxxy];
						}
					}
				}
				//
				if (sum > 0) {
					for (int n = -nHalfSESize; n <= nHalfSESize; n++) {
						for (int m = -nHalfSESize; m <= nHalfSESize; m++) {
							for (int l = -nHalfSESize; l <= nHalfSESize; l++) {
								int idxx = x + l;
								int idxy = y + m;
								int idxz = z + n;
								int idxxy = idxx + idxy * w;

								if ((idxx < 0) || (idxx > w - 1) || (idxy < 0) || (idxy > h - 1) || (idxz < 0) || (idxz > d - 1)) continue;

								//	ppTemp[idxz][idxxy] = m_bytImageValue;
								ppTemp[idxz][idxxy] = val;
							}
						}
					}
				}
			}
		}
	}
	for (int z = 0; z < d; z++) {
		memcpy(m_ppbytData[z], ppTemp[z], sizeof(unsigned char)*w*h);
	}

	SAFE_DELETE_VOLUME(ppTemp, d);

	return true;
}

bool CVpMorph::Dilation3DperSlice(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

#pragma omp parallel for
	for (int z = 0; z < d; z++) {
		CVpMorph morph;
		morph.Init2D(w, h, m_ppbytData[z]);
		morph.Dilation2D(val, nSESize);
	}

	return true;
}

bool CVpMorph::Erosion3DperSlice(unsigned char val, int nSESize) {
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;

#pragma omp parallel for
	for (int z = 0; z < d; z++) {
		CVpMorph morph;
		morph.Init2D(w, h, m_ppbytData[z]);
		morph.Erosion2D(val, nSESize);
	}

	return true;
}

int CVpMorph::countNonZeroNeighbors(std::vector<int>& neighbors) {
	int init = 0;
	return std::accumulate(neighbors.begin() + 1, neighbors.end() - 1, init);
}

int CVpMorph::countTransitionPatterns(std::vector<int>& neighbors) {
	int count = 0;
	for (int i = 1; i < 9; i++) {
		if (neighbors[i + 1] - neighbors[i] == 1)	++count;
	}
	return count;
}

bool CVpMorph::checkCondition(std::vector<int>& p) {
	if (p[0] == 1 && /* Only consider if P1 is 1 */
		(p[1] * p[3] * p[5] == 0) && /* if P2 * P4 * P6 == 0 */
		(p[3] * p[5] * p[7] == 0) && /* if P4 * P6 * P8 == 0 */
		(countTransitionPatterns(p) == 1)) {
		int non_zeros = countNonZeroNeighbors(p);
		if (non_zeros >= 2 && non_zeros <= 6)  /* if 2 <= B(P1) <= 6 */
			return true;
	}
	return false;
}

std::vector<int> CVpMorph::getNeighbors(unsigned char* pGray, int x, int y) {
	/*
		 unsigned char* row1 = &pGray[(y-1)*m_nWidth];
		 unsigned char* row2 = &pGray[(y)*m_nWidth];
		 unsigned char* row3 = &pGray[(y+1)*m_nWidth];

		 vector<int> neighbors;
		 neighbors.push_back(row2[x]);
		 neighbors.push_back(row1[x]);
		 neighbors.push_back(row1[x+1]);
		 neighbors.push_back(row2[x+1]);
		 neighbors.push_back(row3[x+1]);
		 neighbors.push_back(row3[x]);
		 neighbors.push_back(row3[x-1]);
		 neighbors.push_back(row2[x-1]);
		 neighbors.push_back(row1[x-1]);
		 neighbors.push_back(row1[x]);
		 */

	std::vector<int> neighbors;
	neighbors.push_back(pGray[x + y * m_nWidth]);
	neighbors.push_back(pGray[x + (y - 1)*m_nWidth]);
	neighbors.push_back(pGray[x + 1 + (y - 1)*m_nWidth]);
	neighbors.push_back(pGray[x + 1 + y * m_nWidth]);
	neighbors.push_back(pGray[x + 1 + (y + 1)*m_nWidth]);
	neighbors.push_back(pGray[x + (y + 1)*m_nWidth]);
	neighbors.push_back(pGray[x - 1 + (y + 1)*m_nWidth]);
	neighbors.push_back(pGray[x - 1 + y * m_nWidth]);
	neighbors.push_back(pGray[x - 1 + (y - 1)*m_nWidth]);
	neighbors.push_back(pGray[x + (y - 1)*m_nWidth]);

	// 	{
	// 		/* P1  , P2     , P3       , P4       , P5       , P6     , P7       , P8       , P9       , P2 */
	// 		row2[x], row1[x], row1[x+1], row2[x+1], row3[x+1], row3[x], row3[x-1], row2[x-1], row1[x-1], row1[x]
	// 	};
	return neighbors;
}

void CVpMorph::StartThinning() {
	// set the distance of neighbor - U, D, N, S, E, W
	m_pntDir[CVpMorph::U].x = 0;	m_pntDir[CVpMorph::U].y = 0;	m_pntDir[CVpMorph::U].z = -1;
	m_pntDir[CVpMorph::D].x = 0;	m_pntDir[CVpMorph::D].y = 0;	m_pntDir[CVpMorph::D].z = 1;
	m_pntDir[CVpMorph::N].x = 0;	m_pntDir[CVpMorph::N].y = -1;	m_pntDir[CVpMorph::N].z = 0;
	m_pntDir[CVpMorph::S].x = 0;	m_pntDir[CVpMorph::S].y = 1;	m_pntDir[CVpMorph::S].z = 0;
	m_pntDir[CVpMorph::E].x = 1;	m_pntDir[CVpMorph::E].y = 0;	m_pntDir[CVpMorph::E].z = 0;
	m_pntDir[CVpMorph::W].x = -1;	m_pntDir[CVpMorph::W].y = 0;	m_pntDir[CVpMorph::W].z = 0;

	// set look up table;
	SetLUT();
}

void CVpMorph::EndThinning() {
	for (int i = 0; i < THINNEIGH; i++) {
		SAFE_DELETE_ARRAY(m_S26[i]._pIdx);
		SAFE_DELETE_ARRAY(m_S18[i]._pIdx);
	}
}

bool CVpMorph::Thinning3D(unsigned char** ppSkeleton) {
	StartThinning();
	//
	CImage3D< unsigned char> img8;
	img8.Set(m_nWidth, m_nHeight, m_nDepth, ppSkeleton);
	img8.Copy(m_ppbytData);

	while (true) {
		int modified = 0;
		//
		modified += SubIter(ppSkeleton, CVpMorph::U);
		modified += SubIter(ppSkeleton, CVpMorph::D);
		modified += SubIter(ppSkeleton, CVpMorph::N);
		modified += SubIter(ppSkeleton, CVpMorph::S);
		modified += SubIter(ppSkeleton, CVpMorph::E);
		modified += SubIter(ppSkeleton, CVpMorph::W);
		//
		if (!modified) break;
	}
	//
	EndThinning();

	return true;
}

int CVpMorph::SubIter(unsigned char** ppMask, eDirection direction) {
	int modified = 0;
	std::vector<TPoint3D<int>> list;

	// 1st phase
	for (int z = 1; z < m_nDepth - 1; z++) {
		for (int y = 1; y < m_nHeight - 1; y++) {
			for (int x = 1; x < m_nWidth - 1; x++) {
				int idxxy = x + y * m_nWidth;
				if (!ppMask[z][idxxy]) continue;

				if (IsBorderPoint(ppMask, direction, x, y, z)) {
					TPoint3D<int> Np[THINNEIGH];
					Collect26Neighbors(ppMask, x, y, z, Np);
					if (!IsEndPoint(ppMask, Np)) {
						if (IsSimple(ppMask, Np)) {
							TPoint3D<int> pnt;
							pnt.x = x;	pnt.y = y;	pnt.z = z;
							list.push_back(pnt);
						}
					}
				}
			}
		}
	}

	// 2nd phase
	std::vector<TPoint3D<int>>::iterator itr;
	for (itr = list.begin(); itr != list.end(); itr++) {
		TPoint3D<int> p = (*itr);
		TPoint3D<int> Np[THINNEIGH];
		Collect26Neighbors(ppMask, p.x, p.y, p.z, Np);

		if (!IsEndPoint(ppMask, Np)) {
			if (IsSimple(ppMask, Np)) {
				int idxxy = p.x + p.y*m_nWidth;
				ppMask[p.z][idxxy] = 0;
				++modified;
			}
		}
	}

	return modified;
}

bool CVpMorph::IsBorderPoint(unsigned char** ppMask, eDirection direction, int x, int y, int z) {
	//	for (int i = 0 ; i < THINDIR ; i++)
	//	{
	int idxxy = (x + m_pntDir[direction].x) + (y + m_pntDir[direction].y)*m_nWidth;
	int idxz = (z + m_pntDir[direction].z);
	if (ppMask[idxz][idxxy] == 0) return true;
	//	}
	return false;
}

void CVpMorph::Collect26Neighbors(unsigned char** ppMask, int x, int y, int z, TPoint3D<int>* pOutPnt) {
	// top
	pOutPnt[0].x = x - 1;	pOutPnt[0].y = y - 1;	pOutPnt[0].z = z - 1;
	pOutPnt[1].x = x;		pOutPnt[1].y = y - 1;	pOutPnt[1].z = z - 1;
	pOutPnt[2].x = x + 1;	pOutPnt[2].y = y - 1;	pOutPnt[2].z = z - 1;

	pOutPnt[3].x = x - 1;	pOutPnt[3].y = y;		pOutPnt[3].z = z - 1;
	pOutPnt[4].x = x;		pOutPnt[4].y = y;		pOutPnt[4].z = z - 1;
	pOutPnt[5].x = x + 1;	pOutPnt[5].y = y;		pOutPnt[5].z = z - 1;

	pOutPnt[6].x = x - 1;	pOutPnt[6].y = y + 1;	pOutPnt[6].z = z - 1;
	pOutPnt[7].x = x;		pOutPnt[7].y = y + 1;	pOutPnt[7].z = z - 1;
	pOutPnt[8].x = x + 1;	pOutPnt[8].y = y + 1;	pOutPnt[8].z = z - 1;

	// middle
	pOutPnt[9].x = x - 1;	pOutPnt[9].y = y - 1;	pOutPnt[9].z = z;
	pOutPnt[10].x = x;		pOutPnt[10].y = y - 1;	pOutPnt[10].z = z;
	pOutPnt[11].x = x + 1;	pOutPnt[11].y = y - 1;	pOutPnt[11].z = z;

	pOutPnt[12].x = x - 1;	pOutPnt[12].y = y;		pOutPnt[12].z = z;
	pOutPnt[13].x = x + 1;	pOutPnt[13].y = y;		pOutPnt[13].z = z;

	pOutPnt[14].x = x - 1;	pOutPnt[14].y = y + 1;	pOutPnt[14].z = z;
	pOutPnt[15].x = x;		pOutPnt[15].y = y + 1;	pOutPnt[15].z = z;
	pOutPnt[16].x = x + 1;	pOutPnt[16].y = y + 1;	pOutPnt[16].z = z;

	// bottom
	pOutPnt[17].x = x - 1;	pOutPnt[17].y = y - 1;	pOutPnt[17].z = z + 1;
	pOutPnt[18].x = x;		pOutPnt[18].y = y - 1;	pOutPnt[18].z = z + 1;
	pOutPnt[19].x = x + 1;	pOutPnt[19].y = y - 1;	pOutPnt[19].z = z + 1;

	pOutPnt[20].x = x - 1;	pOutPnt[20].y = y;		pOutPnt[20].z = z + 1;
	pOutPnt[21].x = x;		pOutPnt[21].y = y;		pOutPnt[21].z = z + 1;
	pOutPnt[22].x = x + 1;	pOutPnt[22].y = y;		pOutPnt[22].z = z + 1;

	pOutPnt[23].x = x - 1;	pOutPnt[23].y = y + 1;	pOutPnt[23].z = z + 1;
	pOutPnt[24].x = x;		pOutPnt[24].y = y + 1;	pOutPnt[24].z = z + 1;
	pOutPnt[25].x = x + 1;	pOutPnt[25].y = y + 1;	pOutPnt[25].z = z + 1;
}

bool CVpMorph::IsEndPoint(unsigned char** ppMask, TPoint3D<int>* pPnt) {
	int sum = 0;
	for (int i = 0; i < THINNEIGH; i++) {
		int idxxy = pPnt[i].x + pPnt[i].y*m_nWidth;
		sum += ppMask[pPnt[i].z][idxxy];
	}
	if (sum > m_bytImageValue) return false;

	return true;
}

bool CVpMorph::IsSimple(unsigned char** ppMask, TPoint3D<int>* pPnt) {
	if (IsCond2Satisfied(ppMask, pPnt) && IsCond4Satisfied(ppMask, pPnt))		return true;
	return false;
}

bool CVpMorph::IsCond2Satisfied(unsigned char** ppMask, TPoint3D<int>* pPnt) {
	unsigned char blackpnt[THINNEIGH];
	memset(blackpnt, 0, sizeof(unsigned char)*THINNEIGH);
	unsigned char connected[THINNEIGH];
	memset(connected, 0, sizeof(unsigned char)*THINNEIGH);

	for (int i = 0; i < THINNEIGH; i++) {
		int idxxy = pPnt[i].x + pPnt[i].y*m_nWidth;
		if (ppMask[pPnt[i].z][idxxy]) blackpnt[i] = 1;
	}

	for (int i = 0; i < THINNEIGH; i++) {
		if (!blackpnt[i]) continue;
		for (int n = 0; n < m_S26[i]._nCnt; n++) {
			int neigh = m_S26[i]._pIdx[n];
			if (blackpnt[neigh]) connected[neigh] = 1;
		}
	}

	for (int i = 0; i < THINNEIGH; i++) {
		if (blackpnt[i] && !connected[i])	return false;
	}
	return true;
}

bool CVpMorph::IsCond4Satisfied(unsigned char** ppMask, TPoint3D<int>* pPnt) {
	unsigned char whitepnt[THINNEIGH];
	memset(whitepnt, 0, sizeof(unsigned char)*THINNEIGH);
	unsigned char connected[THINNEIGH];
	memset(connected, 0, sizeof(unsigned char)*THINNEIGH);

	// set the distance of neighbor - U, D, N, S, E, W
	int idxneigh[THINDIR] = { 4, 21, 10, 15, 13, 12 };

	for (int i = 0; i < THINDIR; i++) {
		int idxxy = pPnt[idxneigh[i]].x + pPnt[idxneigh[i]].y*m_nWidth;
		if (!ppMask[pPnt[idxneigh[i]].z][idxxy]) whitepnt[idxneigh[i]] = 1;
	}

	for (int i = 0; i < THINNEIGH; i++) {
		if (!whitepnt[i]) continue;
		for (int n = 0; n < m_S18[i]._nCnt; n++) {
			int neigh = m_S18[i]._pIdx[n];
			if (whitepnt[neigh]) connected[neigh] = 1;
		}
	}

	for (int i = 0; i < THINNEIGH; i++) {
		if (whitepnt[i] && !connected[i])	return false;
	}

	return true;
}

void CVpMorph::SetLUT() {
	int pntidx;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 1. set S26
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 0
	pntidx = 0;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 1;
	m_S26[pntidx]._pIdx[1] = 3;
	m_S26[pntidx]._pIdx[2] = 4;
	m_S26[pntidx]._pIdx[3] = 9;
	m_S26[pntidx]._pIdx[4] = 10;
	m_S26[pntidx]._pIdx[5] = 12;

	// 1
	pntidx = 1;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 2;
	m_S26[pntidx]._pIdx[2] = 3;
	m_S26[pntidx]._pIdx[3] = 4;
	m_S26[pntidx]._pIdx[4] = 5;
	m_S26[pntidx]._pIdx[5] = 9;
	m_S26[pntidx]._pIdx[6] = 10;
	m_S26[pntidx]._pIdx[7] = 11;
	m_S26[pntidx]._pIdx[8] = 12;
	m_S26[pntidx]._pIdx[9] = 13;

	// 2
	pntidx = 2;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 1;
	m_S26[pntidx]._pIdx[1] = 4;
	m_S26[pntidx]._pIdx[2] = 5;
	m_S26[pntidx]._pIdx[3] = 10;
	m_S26[pntidx]._pIdx[4] = 11;
	m_S26[pntidx]._pIdx[5] = 13;

	// 3
	pntidx = 3;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 1;
	m_S26[pntidx]._pIdx[2] = 4;
	m_S26[pntidx]._pIdx[3] = 6;
	m_S26[pntidx]._pIdx[4] = 7;
	m_S26[pntidx]._pIdx[5] = 9;
	m_S26[pntidx]._pIdx[6] = 10;
	m_S26[pntidx]._pIdx[7] = 12;
	m_S26[pntidx]._pIdx[8] = 14;
	m_S26[pntidx]._pIdx[9] = 15;

	// 4
	pntidx = 4;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 1;
	m_S26[pntidx]._pIdx[2] = 2;
	m_S26[pntidx]._pIdx[3] = 3;
	m_S26[pntidx]._pIdx[4] = 5;
	m_S26[pntidx]._pIdx[5] = 6;
	m_S26[pntidx]._pIdx[6] = 7;
	m_S26[pntidx]._pIdx[7] = 8;
	m_S26[pntidx]._pIdx[8] = 9;
	m_S26[pntidx]._pIdx[9] = 10;
	m_S26[pntidx]._pIdx[10] = 11;
	m_S26[pntidx]._pIdx[11] = 12;
	m_S26[pntidx]._pIdx[12] = 13;
	m_S26[pntidx]._pIdx[13] = 14;
	m_S26[pntidx]._pIdx[14] = 15;
	m_S26[pntidx]._pIdx[15] = 16;

	// 5
	pntidx = 5;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 1;
	m_S26[pntidx]._pIdx[1] = 2;
	m_S26[pntidx]._pIdx[2] = 4;
	m_S26[pntidx]._pIdx[3] = 7;
	m_S26[pntidx]._pIdx[4] = 8;
	m_S26[pntidx]._pIdx[5] = 10;
	m_S26[pntidx]._pIdx[6] = 11;
	m_S26[pntidx]._pIdx[7] = 13;
	m_S26[pntidx]._pIdx[8] = 15;
	m_S26[pntidx]._pIdx[9] = 16;

	// 6
	pntidx = 6;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 3;
	m_S26[pntidx]._pIdx[1] = 4;
	m_S26[pntidx]._pIdx[2] = 7;
	m_S26[pntidx]._pIdx[3] = 12;
	m_S26[pntidx]._pIdx[4] = 14;
	m_S26[pntidx]._pIdx[5] = 15;

	// 7
	pntidx = 7;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 3;
	m_S26[pntidx]._pIdx[1] = 4;
	m_S26[pntidx]._pIdx[2] = 5;
	m_S26[pntidx]._pIdx[3] = 6;
	m_S26[pntidx]._pIdx[4] = 8;
	m_S26[pntidx]._pIdx[5] = 12;
	m_S26[pntidx]._pIdx[6] = 13;
	m_S26[pntidx]._pIdx[7] = 14;
	m_S26[pntidx]._pIdx[8] = 15;
	m_S26[pntidx]._pIdx[9] = 16;

	// 8
	pntidx = 8;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 4;
	m_S26[pntidx]._pIdx[1] = 5;
	m_S26[pntidx]._pIdx[2] = 7;
	m_S26[pntidx]._pIdx[3] = 13;
	m_S26[pntidx]._pIdx[4] = 15;
	m_S26[pntidx]._pIdx[5] = 16;

	// 9
	pntidx = 9;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 1;
	m_S26[pntidx]._pIdx[2] = 3;
	m_S26[pntidx]._pIdx[3] = 4;
	m_S26[pntidx]._pIdx[4] = 10;
	m_S26[pntidx]._pIdx[5] = 12;
	m_S26[pntidx]._pIdx[6] = 17;
	m_S26[pntidx]._pIdx[7] = 18;
	m_S26[pntidx]._pIdx[8] = 20;
	m_S26[pntidx]._pIdx[9] = 21;

	// 10
	pntidx = 10;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 1;
	m_S26[pntidx]._pIdx[2] = 2;
	m_S26[pntidx]._pIdx[3] = 3;
	m_S26[pntidx]._pIdx[4] = 4;
	m_S26[pntidx]._pIdx[5] = 5;
	m_S26[pntidx]._pIdx[6] = 9;
	m_S26[pntidx]._pIdx[7] = 11;
	m_S26[pntidx]._pIdx[8] = 12;
	m_S26[pntidx]._pIdx[9] = 13;
	m_S26[pntidx]._pIdx[10] = 17;
	m_S26[pntidx]._pIdx[11] = 18;
	m_S26[pntidx]._pIdx[12] = 19;
	m_S26[pntidx]._pIdx[13] = 20;
	m_S26[pntidx]._pIdx[14] = 21;
	m_S26[pntidx]._pIdx[15] = 22;

	// 11
	pntidx = 11;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 1;
	m_S26[pntidx]._pIdx[1] = 2;
	m_S26[pntidx]._pIdx[2] = 4;
	m_S26[pntidx]._pIdx[3] = 5;
	m_S26[pntidx]._pIdx[4] = 10;
	m_S26[pntidx]._pIdx[5] = 13;
	m_S26[pntidx]._pIdx[6] = 18;
	m_S26[pntidx]._pIdx[7] = 19;
	m_S26[pntidx]._pIdx[8] = 21;
	m_S26[pntidx]._pIdx[9] = 22;

	// 12
	pntidx = 12;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 0;
	m_S26[pntidx]._pIdx[1] = 1;
	m_S26[pntidx]._pIdx[2] = 3;
	m_S26[pntidx]._pIdx[3] = 4;
	m_S26[pntidx]._pIdx[4] = 6;
	m_S26[pntidx]._pIdx[5] = 7;
	m_S26[pntidx]._pIdx[6] = 9;
	m_S26[pntidx]._pIdx[7] = 10;
	m_S26[pntidx]._pIdx[8] = 14;
	m_S26[pntidx]._pIdx[9] = 15;
	m_S26[pntidx]._pIdx[10] = 17;
	m_S26[pntidx]._pIdx[11] = 18;
	m_S26[pntidx]._pIdx[12] = 20;
	m_S26[pntidx]._pIdx[13] = 21;
	m_S26[pntidx]._pIdx[14] = 23;
	m_S26[pntidx]._pIdx[15] = 24;

	// 13
	pntidx = 13;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 1;
	m_S26[pntidx]._pIdx[1] = 2;
	m_S26[pntidx]._pIdx[2] = 4;
	m_S26[pntidx]._pIdx[3] = 5;
	m_S26[pntidx]._pIdx[4] = 7;
	m_S26[pntidx]._pIdx[5] = 8;
	m_S26[pntidx]._pIdx[6] = 10;
	m_S26[pntidx]._pIdx[7] = 11;
	m_S26[pntidx]._pIdx[8] = 15;
	m_S26[pntidx]._pIdx[9] = 16;
	m_S26[pntidx]._pIdx[10] = 18;
	m_S26[pntidx]._pIdx[11] = 19;
	m_S26[pntidx]._pIdx[12] = 21;
	m_S26[pntidx]._pIdx[13] = 22;
	m_S26[pntidx]._pIdx[14] = 24;
	m_S26[pntidx]._pIdx[15] = 25;

	// 14
	pntidx = 14;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 3;
	m_S26[pntidx]._pIdx[1] = 4;
	m_S26[pntidx]._pIdx[2] = 6;
	m_S26[pntidx]._pIdx[3] = 7;
	m_S26[pntidx]._pIdx[4] = 12;
	m_S26[pntidx]._pIdx[5] = 15;
	m_S26[pntidx]._pIdx[6] = 20;
	m_S26[pntidx]._pIdx[7] = 21;
	m_S26[pntidx]._pIdx[8] = 23;
	m_S26[pntidx]._pIdx[9] = 24;

	// 15
	pntidx = 15;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 3;
	m_S26[pntidx]._pIdx[1] = 4;
	m_S26[pntidx]._pIdx[2] = 5;
	m_S26[pntidx]._pIdx[3] = 6;
	m_S26[pntidx]._pIdx[4] = 7;
	m_S26[pntidx]._pIdx[5] = 8;
	m_S26[pntidx]._pIdx[6] = 12;
	m_S26[pntidx]._pIdx[7] = 13;
	m_S26[pntidx]._pIdx[8] = 14;
	m_S26[pntidx]._pIdx[9] = 16;
	m_S26[pntidx]._pIdx[10] = 20;
	m_S26[pntidx]._pIdx[11] = 21;
	m_S26[pntidx]._pIdx[12] = 22;
	m_S26[pntidx]._pIdx[13] = 23;
	m_S26[pntidx]._pIdx[14] = 24;
	m_S26[pntidx]._pIdx[15] = 25;

	// 16
	pntidx = 16;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 4;
	m_S26[pntidx]._pIdx[1] = 5;
	m_S26[pntidx]._pIdx[2] = 7;
	m_S26[pntidx]._pIdx[3] = 8;
	m_S26[pntidx]._pIdx[4] = 13;
	m_S26[pntidx]._pIdx[5] = 15;
	m_S26[pntidx]._pIdx[6] = 21;
	m_S26[pntidx]._pIdx[7] = 22;
	m_S26[pntidx]._pIdx[8] = 24;
	m_S26[pntidx]._pIdx[9] = 25;

	// 17
	pntidx = 17;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 9;
	m_S26[pntidx]._pIdx[1] = 10;
	m_S26[pntidx]._pIdx[2] = 12;
	m_S26[pntidx]._pIdx[3] = 18;
	m_S26[pntidx]._pIdx[4] = 20;
	m_S26[pntidx]._pIdx[5] = 21;

	// 18
	pntidx = 18;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 9;
	m_S26[pntidx]._pIdx[1] = 10;
	m_S26[pntidx]._pIdx[2] = 11;
	m_S26[pntidx]._pIdx[3] = 12;
	m_S26[pntidx]._pIdx[4] = 13;
	m_S26[pntidx]._pIdx[5] = 17;
	m_S26[pntidx]._pIdx[6] = 19;
	m_S26[pntidx]._pIdx[7] = 20;
	m_S26[pntidx]._pIdx[8] = 21;
	m_S26[pntidx]._pIdx[9] = 22;

	// 19
	pntidx = 19;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 10;
	m_S26[pntidx]._pIdx[1] = 11;
	m_S26[pntidx]._pIdx[2] = 13;
	m_S26[pntidx]._pIdx[3] = 18;
	m_S26[pntidx]._pIdx[4] = 21;
	m_S26[pntidx]._pIdx[5] = 22;

	// 20
	pntidx = 20;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 9;
	m_S26[pntidx]._pIdx[1] = 10;
	m_S26[pntidx]._pIdx[2] = 12;
	m_S26[pntidx]._pIdx[3] = 14;
	m_S26[pntidx]._pIdx[4] = 15;
	m_S26[pntidx]._pIdx[5] = 17;
	m_S26[pntidx]._pIdx[6] = 18;
	m_S26[pntidx]._pIdx[7] = 21;
	m_S26[pntidx]._pIdx[8] = 23;
	m_S26[pntidx]._pIdx[9] = 24;

	// 21
	pntidx = 21;
	m_S26[pntidx]._nCnt = 16;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 9;
	m_S26[pntidx]._pIdx[1] = 10;
	m_S26[pntidx]._pIdx[2] = 11;
	m_S26[pntidx]._pIdx[3] = 12;
	m_S26[pntidx]._pIdx[4] = 13;
	m_S26[pntidx]._pIdx[5] = 14;
	m_S26[pntidx]._pIdx[6] = 15;
	m_S26[pntidx]._pIdx[7] = 16;
	m_S26[pntidx]._pIdx[8] = 17;
	m_S26[pntidx]._pIdx[9] = 18;
	m_S26[pntidx]._pIdx[10] = 19;
	m_S26[pntidx]._pIdx[11] = 20;
	m_S26[pntidx]._pIdx[12] = 22;
	m_S26[pntidx]._pIdx[13] = 23;
	m_S26[pntidx]._pIdx[14] = 24;
	m_S26[pntidx]._pIdx[15] = 25;

	// 22
	pntidx = 22;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 10;
	m_S26[pntidx]._pIdx[1] = 11;
	m_S26[pntidx]._pIdx[2] = 13;
	m_S26[pntidx]._pIdx[3] = 15;
	m_S26[pntidx]._pIdx[4] = 16;
	m_S26[pntidx]._pIdx[5] = 18;
	m_S26[pntidx]._pIdx[6] = 19;
	m_S26[pntidx]._pIdx[7] = 21;
	m_S26[pntidx]._pIdx[8] = 24;
	m_S26[pntidx]._pIdx[9] = 25;

	// 23
	pntidx = 23;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 12;
	m_S26[pntidx]._pIdx[1] = 14;
	m_S26[pntidx]._pIdx[2] = 15;
	m_S26[pntidx]._pIdx[3] = 20;
	m_S26[pntidx]._pIdx[4] = 21;
	m_S26[pntidx]._pIdx[5] = 24;

	// 24
	pntidx = 24;
	m_S26[pntidx]._nCnt = 10;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 12;
	m_S26[pntidx]._pIdx[1] = 13;
	m_S26[pntidx]._pIdx[2] = 14;
	m_S26[pntidx]._pIdx[3] = 15;
	m_S26[pntidx]._pIdx[4] = 16;
	m_S26[pntidx]._pIdx[5] = 20;
	m_S26[pntidx]._pIdx[6] = 21;
	m_S26[pntidx]._pIdx[7] = 22;
	m_S26[pntidx]._pIdx[8] = 23;
	m_S26[pntidx]._pIdx[9] = 25;

	// 25
	pntidx = 25;
	m_S26[pntidx]._nCnt = 6;
	m_S26[pntidx]._pIdx = new int[m_S26[pntidx]._nCnt];
	m_S26[pntidx]._pIdx[0] = 13;
	m_S26[pntidx]._pIdx[1] = 15;
	m_S26[pntidx]._pIdx[2] = 16;
	m_S26[pntidx]._pIdx[3] = 21;
	m_S26[pntidx]._pIdx[4] = 22;
	m_S26[pntidx]._pIdx[5] = 24;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 2. set S18
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 0
	pntidx = 0;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 1;
	m_S18[pntidx]._pIdx[1] = 3;
	m_S18[pntidx]._pIdx[2] = 4;
	m_S18[pntidx]._pIdx[3] = 9;
	m_S18[pntidx]._pIdx[4] = 10;
	m_S18[pntidx]._pIdx[5] = 12;

	// 1
	pntidx = 1;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 2;
	m_S18[pntidx]._pIdx[2] = 3;
	m_S18[pntidx]._pIdx[3] = 4;
	m_S18[pntidx]._pIdx[4] = 5;
	m_S18[pntidx]._pIdx[5] = 9;
	m_S18[pntidx]._pIdx[6] = 10;
	m_S18[pntidx]._pIdx[7] = 11;

	// 2
	pntidx = 2;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 1;
	m_S18[pntidx]._pIdx[1] = 4;
	m_S18[pntidx]._pIdx[2] = 5;
	m_S18[pntidx]._pIdx[3] = 10;
	m_S18[pntidx]._pIdx[4] = 11;
	m_S18[pntidx]._pIdx[5] = 13;

	// 3
	pntidx = 3;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 1;
	m_S18[pntidx]._pIdx[2] = 4;
	m_S18[pntidx]._pIdx[3] = 6;
	m_S18[pntidx]._pIdx[4] = 7;
	m_S18[pntidx]._pIdx[5] = 9;
	m_S18[pntidx]._pIdx[6] = 12;
	m_S18[pntidx]._pIdx[7] = 14;

	// 4
	pntidx = 4;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 1;
	m_S18[pntidx]._pIdx[2] = 2;
	m_S18[pntidx]._pIdx[3] = 3;
	m_S18[pntidx]._pIdx[4] = 5;
	m_S18[pntidx]._pIdx[5] = 6;
	m_S18[pntidx]._pIdx[6] = 7;
	m_S18[pntidx]._pIdx[7] = 8;
	m_S18[pntidx]._pIdx[8] = 10;
	m_S18[pntidx]._pIdx[9] = 12;
	m_S18[pntidx]._pIdx[10] = 13;
	m_S18[pntidx]._pIdx[11] = 15;

	// 5
	pntidx = 5;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 1;
	m_S18[pntidx]._pIdx[1] = 2;
	m_S18[pntidx]._pIdx[2] = 4;
	m_S18[pntidx]._pIdx[3] = 7;
	m_S18[pntidx]._pIdx[4] = 8;
	m_S18[pntidx]._pIdx[5] = 11;
	m_S18[pntidx]._pIdx[6] = 13;
	m_S18[pntidx]._pIdx[7] = 16;

	// 6
	pntidx = 6;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 3;
	m_S18[pntidx]._pIdx[1] = 4;
	m_S18[pntidx]._pIdx[2] = 7;
	m_S18[pntidx]._pIdx[3] = 12;
	m_S18[pntidx]._pIdx[4] = 14;
	m_S18[pntidx]._pIdx[5] = 15;

	// 7
	pntidx = 7;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 3;
	m_S18[pntidx]._pIdx[1] = 4;
	m_S18[pntidx]._pIdx[2] = 5;
	m_S18[pntidx]._pIdx[3] = 6;
	m_S18[pntidx]._pIdx[4] = 8;
	m_S18[pntidx]._pIdx[5] = 14;
	m_S18[pntidx]._pIdx[6] = 15;
	m_S18[pntidx]._pIdx[7] = 16;

	// 8
	pntidx = 8;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 4;
	m_S18[pntidx]._pIdx[1] = 5;
	m_S18[pntidx]._pIdx[2] = 7;
	m_S18[pntidx]._pIdx[3] = 13;
	m_S18[pntidx]._pIdx[4] = 15;
	m_S18[pntidx]._pIdx[5] = 16;

	// 9
	pntidx = 9;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 1;
	m_S18[pntidx]._pIdx[2] = 3;
	m_S18[pntidx]._pIdx[3] = 10;
	m_S18[pntidx]._pIdx[4] = 12;
	m_S18[pntidx]._pIdx[5] = 17;
	m_S18[pntidx]._pIdx[6] = 18;
	m_S18[pntidx]._pIdx[7] = 20;

	// 10
	pntidx = 10;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 1;
	m_S18[pntidx]._pIdx[2] = 2;
	m_S18[pntidx]._pIdx[3] = 4;
	m_S18[pntidx]._pIdx[4] = 9;
	m_S18[pntidx]._pIdx[5] = 11;
	m_S18[pntidx]._pIdx[6] = 12;
	m_S18[pntidx]._pIdx[7] = 13;
	m_S18[pntidx]._pIdx[8] = 17;
	m_S18[pntidx]._pIdx[9] = 18;
	m_S18[pntidx]._pIdx[10] = 19;
	m_S18[pntidx]._pIdx[11] = 21;

	// 11
	pntidx = 11;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 1;
	m_S18[pntidx]._pIdx[1] = 2;
	m_S18[pntidx]._pIdx[2] = 5;
	m_S18[pntidx]._pIdx[3] = 10;
	m_S18[pntidx]._pIdx[4] = 13;
	m_S18[pntidx]._pIdx[5] = 18;
	m_S18[pntidx]._pIdx[6] = 19;
	m_S18[pntidx]._pIdx[7] = 22;

	// 12
	pntidx = 12;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 0;
	m_S18[pntidx]._pIdx[1] = 3;
	m_S18[pntidx]._pIdx[2] = 4;
	m_S18[pntidx]._pIdx[3] = 6;
	m_S18[pntidx]._pIdx[4] = 9;
	m_S18[pntidx]._pIdx[5] = 10;
	m_S18[pntidx]._pIdx[6] = 14;
	m_S18[pntidx]._pIdx[7] = 15;
	m_S18[pntidx]._pIdx[8] = 17;
	m_S18[pntidx]._pIdx[9] = 20;
	m_S18[pntidx]._pIdx[10] = 21;
	m_S18[pntidx]._pIdx[11] = 23;

	// 13
	pntidx = 13;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 2;
	m_S18[pntidx]._pIdx[1] = 4;
	m_S18[pntidx]._pIdx[2] = 5;
	m_S18[pntidx]._pIdx[3] = 8;
	m_S18[pntidx]._pIdx[4] = 10;
	m_S18[pntidx]._pIdx[5] = 11;
	m_S18[pntidx]._pIdx[6] = 15;
	m_S18[pntidx]._pIdx[7] = 16;
	m_S18[pntidx]._pIdx[8] = 19;
	m_S18[pntidx]._pIdx[9] = 21;
	m_S18[pntidx]._pIdx[10] = 22;
	m_S18[pntidx]._pIdx[11] = 25;

	// 14
	pntidx = 14;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 3;
	m_S18[pntidx]._pIdx[1] = 6;
	m_S18[pntidx]._pIdx[2] = 7;
	m_S18[pntidx]._pIdx[3] = 12;
	m_S18[pntidx]._pIdx[4] = 15;
	m_S18[pntidx]._pIdx[5] = 20;
	m_S18[pntidx]._pIdx[6] = 23;
	m_S18[pntidx]._pIdx[7] = 24;

	// 15
	pntidx = 15;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 4;
	m_S18[pntidx]._pIdx[1] = 6;
	m_S18[pntidx]._pIdx[2] = 7;
	m_S18[pntidx]._pIdx[3] = 8;
	m_S18[pntidx]._pIdx[4] = 12;
	m_S18[pntidx]._pIdx[5] = 13;
	m_S18[pntidx]._pIdx[6] = 14;
	m_S18[pntidx]._pIdx[7] = 16;
	m_S18[pntidx]._pIdx[8] = 21;
	m_S18[pntidx]._pIdx[9] = 23;
	m_S18[pntidx]._pIdx[10] = 24;
	m_S18[pntidx]._pIdx[11] = 25;

	// 16
	pntidx = 16;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 5;
	m_S18[pntidx]._pIdx[1] = 7;
	m_S18[pntidx]._pIdx[2] = 8;
	m_S18[pntidx]._pIdx[3] = 13;
	m_S18[pntidx]._pIdx[4] = 15;
	m_S18[pntidx]._pIdx[5] = 22;
	m_S18[pntidx]._pIdx[6] = 24;
	m_S18[pntidx]._pIdx[7] = 25;

	// 17
	pntidx = 17;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 9;
	m_S18[pntidx]._pIdx[1] = 10;
	m_S18[pntidx]._pIdx[2] = 12;
	m_S18[pntidx]._pIdx[3] = 18;
	m_S18[pntidx]._pIdx[4] = 20;
	m_S18[pntidx]._pIdx[5] = 21;

	// 18
	pntidx = 18;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 9;
	m_S18[pntidx]._pIdx[1] = 10;
	m_S18[pntidx]._pIdx[2] = 11;
	m_S18[pntidx]._pIdx[3] = 17;
	m_S18[pntidx]._pIdx[4] = 19;
	m_S18[pntidx]._pIdx[5] = 20;
	m_S18[pntidx]._pIdx[6] = 21;
	m_S18[pntidx]._pIdx[7] = 22;

	// 19
	pntidx = 19;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 10;
	m_S18[pntidx]._pIdx[1] = 11;
	m_S18[pntidx]._pIdx[2] = 13;
	m_S18[pntidx]._pIdx[3] = 18;
	m_S18[pntidx]._pIdx[4] = 21;
	m_S18[pntidx]._pIdx[5] = 22;

	// 20
	pntidx = 20;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 9;
	m_S18[pntidx]._pIdx[1] = 12;
	m_S18[pntidx]._pIdx[2] = 14;
	m_S18[pntidx]._pIdx[3] = 17;
	m_S18[pntidx]._pIdx[4] = 18;
	m_S18[pntidx]._pIdx[5] = 21;
	m_S18[pntidx]._pIdx[6] = 23;
	m_S18[pntidx]._pIdx[7] = 24;

	// 21
	pntidx = 21;
	m_S18[pntidx]._nCnt = 12;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 10;
	m_S18[pntidx]._pIdx[1] = 12;
	m_S18[pntidx]._pIdx[2] = 13;
	m_S18[pntidx]._pIdx[3] = 15;
	m_S18[pntidx]._pIdx[4] = 17;
	m_S18[pntidx]._pIdx[5] = 18;
	m_S18[pntidx]._pIdx[6] = 19;
	m_S18[pntidx]._pIdx[7] = 20;
	m_S18[pntidx]._pIdx[8] = 22;
	m_S18[pntidx]._pIdx[9] = 23;
	m_S18[pntidx]._pIdx[10] = 24;
	m_S18[pntidx]._pIdx[11] = 25;

	// 22
	pntidx = 22;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 11;
	m_S18[pntidx]._pIdx[1] = 13;
	m_S18[pntidx]._pIdx[2] = 16;
	m_S18[pntidx]._pIdx[3] = 18;
	m_S18[pntidx]._pIdx[4] = 19;
	m_S18[pntidx]._pIdx[5] = 21;
	m_S18[pntidx]._pIdx[6] = 24;
	m_S18[pntidx]._pIdx[7] = 25;

	// 23
	pntidx = 23;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 12;
	m_S18[pntidx]._pIdx[1] = 14;
	m_S18[pntidx]._pIdx[2] = 15;
	m_S18[pntidx]._pIdx[3] = 20;
	m_S18[pntidx]._pIdx[4] = 21;
	m_S18[pntidx]._pIdx[5] = 24;

	// 24
	pntidx = 24;
	m_S18[pntidx]._nCnt = 8;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 14;
	m_S18[pntidx]._pIdx[1] = 15;
	m_S18[pntidx]._pIdx[2] = 16;
	m_S18[pntidx]._pIdx[3] = 20;
	m_S18[pntidx]._pIdx[4] = 21;
	m_S18[pntidx]._pIdx[5] = 22;
	m_S18[pntidx]._pIdx[6] = 23;
	m_S18[pntidx]._pIdx[7] = 25;

	// 25
	pntidx = 25;
	m_S18[pntidx]._nCnt = 6;
	m_S18[pntidx]._pIdx = new int[m_S18[pntidx]._nCnt];
	m_S18[pntidx]._pIdx[0] = 13;
	m_S18[pntidx]._pIdx[1] = 15;
	m_S18[pntidx]._pIdx[2] = 16;
	m_S18[pntidx]._pIdx[3] = 21;
	m_S18[pntidx]._pIdx[4] = 22;
	m_S18[pntidx]._pIdx[5] = 24;
}

void CVpMorph::Copy(unsigned char** ppSrc) {
	for (int z = 0; z < m_nDepth; z++) {
		memcpy(m_ppbytData[z], ppSrc[z], sizeof(unsigned char)*m_nWH);
	}
}

void CVpMorph::Inverse() {
	for (int z = 0; z < m_nDepth; z++) {
		for (int xy = 0; xy < m_nWH; xy++)
			if (m_ppbytData[z][xy])	m_ppbytData[z][xy] = 0;
			else					m_ppbytData[z][xy] = 255;
	}
}

void CVpMorph::LeaveOne(int kernel) {
	if (kernel != 0)	Erosion3D(kernel);

	// leave only one
	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, m_ppbytData);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();
	gxccl.GetVolData(1, m_ppbytData);
	gxccl.Release();

	if (kernel != 0)	Dilation3D(kernel);
}

#if 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 1. create structure element
// 	 unsigned char*** pppSE = new  unsigned char**[nSESize];
// 	for (int n = 0 ; n < nSESize ; n++)
// 	{
// 		pppSE[n] = new  unsigned char*[nSESize];
// 		for (int m = 0 ; m < nSESize ; m++)
// 		{
// 			pppSE[n][m] = new  unsigned char[nSESize];
// 			memset(pppSE[n][m], 1, sizeof( unsigned char)*nSESize);
// 		}
// 	}

// 3 delete
// 	if (pppSE)
// 	{
// 		for (int n = 0 ; n < nSESize ; n++)
// 		{
// 			for (int m = 0 ; m < nSESize ; m++)
// 			{
// 				delete[] (pppSE[n][m]);
//
// 			}
// 			delete[] (pppSE[n]);
// 		}
// 		delete[] pppSE;
// 	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
