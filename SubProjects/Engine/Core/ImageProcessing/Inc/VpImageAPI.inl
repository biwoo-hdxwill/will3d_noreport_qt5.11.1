#pragma once

#include "VpGlobalDefine.h"
#include "../../../Common/Common/W3Types.h"
#include "../../../Common/Common/W3Memory.h"

template<typename T>
class CImage3D {
public:
	enum eInterpolation {
		NEAREST = 0,
		BILINEAR,
		BICUBIC,
		TRILINEAR,
		TRICUBIC
	};

private:
	T** m_ppImage;
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;
	int m_nWH;
	TPoint3D<int> m_pntObjCenter;

	T m_valMin;
	T m_valMax;
	float m_fScale;

	double* m_pPDF;
	int m_nLevel;

public:
	CImage3D() : m_ppImage(NULL), m_nWidth(0), m_nHeight(0), m_nDepth(0), m_nWH(0), m_valMax(0), m_valMin(0), m_fScale(.0f) {
		memset(&m_pntObjCenter, 0, sizeof(TPoint3D<int>));

		m_pPDF = NULL;
		m_nLevel = 0;
	}
	~CImage3D() {}

	inline int GetWidth() { return m_nWidth; }
	inline int GetHeight() { return m_nHeight; }
	inline int GetDepth() { return m_nDepth; }
	inline int GetWH() { return m_nWH; }
	inline T GetValue(int x, int y, int z) { return m_ppImage[z][x + y * m_nWidth]; }
	inline T GetValue(int xy, int z) { return m_ppImage[z][xy]; }
	inline void SetValue(int x, int y, int z, T val) { m_ppImage[z][x + y * m_nWidth] = val; }
	inline void SetValue(int xy, int z, T val) { m_ppImage[z][xy] = val; }
	inline float GetNorValue(int x, int y, int z) { return ((m_ppImage[z][x*y*m_nWidth] - m_valMin) / m_fScale); }
	inline float GetNorValue(int xy, int z) { return ((m_ppImage[z][xy] - m_valMin) / m_fScale); }
	inline T GetMinVal() { return m_valMin; }
	inline T GetMaxVal() { return m_valMax; }
	inline void SetObjCenter(int x, int y, int z) { m_pntObjCenter.x = x; m_pntObjCenter.y = y; m_pntObjCenter.z = z; }
	inline void GetObjCenter(int *x, int *y, int *z) { *x = m_pntObjCenter.x; *y = m_pntObjCenter.y; *z = m_pntObjCenter.z; }

	inline	T** GetImage() { return m_ppImage; }
	inline	void SetImage(T** ppImg) { m_ppImage = ppImg; }

	// for mpr
	T** CreateCoronal(int *outw, int *outh, int *outd) {
		int corw = m_nWidth;
		int corh = m_nDepth;
		int cord = m_nHeight;
		int corwh = corw * corh;

		T** ppCoronal = NULL;
		SafeNew2D(ppCoronal, corwh, cord);

		for (int z = 0; z < cord; z++) {
			for (int y = 0; y < corh; y++) {
				for (int x = 0; x < corw; x++) {
					int xy = x + y * corw;
					ppCoronal[(cord - 1) - z][xy] = m_ppImage[y][x + z * m_nWidth];
				}
			}
		}

		*outw = corw;
		*outh = corh;
		*outd = cord;

		return ppCoronal;
	}

	T** CreateSagittal(int *outw, int *outh, int *outd) {
		int sagw = m_nHeight;
		int sagh = m_nDepth;
		int sagd = m_nWidth;
		int sagwh = sagw * sagh;

		T** ppSagittal = NULL;
		SafeNew2D(ppSagittal, sagwh, sagd);

		for (int z = 0; z < sagd; z++) {
			for (int y = 0; y < sagh; y++) {
				for (int x = 0; x < sagw; x++) {
					int xy = ((sagw - 1) - x) + y * sagw;
					ppSagittal[(sagd - 1) - z][xy] = m_ppImage[y][z + x * m_nHeight];
				}
			}
		}

		*outw = sagw;
		*outh = sagh;
		*outd = sagd;

		return ppSagittal;
	}

	// equal or greater than zero : 0 ~ xxxx
	void Normalize() {
		CalcMinMax();

#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				m_ppImage[z][xy] -= m_valMin;
			}
		}
	}

	// tmin ~ tmax
	void Normalize(T tmin, T tmax) {
		// 0. get min, max
		CalcMinMax();

		// 1. normalize (0 ~ 1)
		float** ppNormal = NULL;
		SafeNew2D(ppNormal, m_nWH, m_nDepth);
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				ppNormal[z][xy] = (float)((m_ppImage[z][xy] - m_valMin) / m_fScale);
			}
		}

		//#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				m_ppImage[z][xy] = (T)(ppNormal[z][xy] * tmax);
			}
		}
		SafeDelete2D(ppNormal, m_nWH, m_nDepth);
	}

	// equal or greater than zero : 0 ~ xxxx
	// if an intensity is less than minvalue, set it to minvalue
	void NormalizeWithRange(voltype minvalue = -1024) {
		// set minvalue
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy] < minvalue) m_ppImage[z][xy] = minvalue;
			}
		}

		CalcMinMax();

#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				m_ppImage[z][xy] -= m_valMin;
			}
		}
	}

	void SafeSetImage(T** ppImg) {
		SafeDelete2D<T>(m_ppImage, m_nWidth*m_nHeight, m_nDepth);
		m_ppImage = ppImg;
	}

	bool Create(int w, int h, int d, bool bIsZeo = true) {
		if (!SafeNew2D<T>(m_ppImage, w*h, d)) return false;

		m_nWidth = w;
		m_nHeight = h;
		m_nDepth = d;
		m_nWH = w * h;

		if (bIsZeo) {
			for (int z = 0; z < d; z++) {
				memset(m_ppImage[z], 0, sizeof(T)*m_nWH);
			}
		}

		return true;
	}

	void Set(int xmin, int xmax, int ymin, int ymax, int zmin, int zmax, T val) {
		if (!m_ppImage) return;

		for (int z = zmin; z <= zmax; z++) {
			for (int y = ymin; y <= ymax; y++) {
				for (int x = xmin; x <= xmax; x++) {
					int xy = x + y * m_nWidth;
					m_ppImage[z][xy] = val;
				}
			}
		}
	}

	void Set(int x, int y, int z, T val) {
		if (!m_ppImage) return;
		m_ppImage[z][x + y * m_nWidth] = val;
	}

	void Set(int w, int h, int d, T** ppData) {
		m_ppImage = ppData;
		m_nWidth = w;
		m_nHeight = h;
		m_nDepth = d;
		m_nWH = w * h;

		SAFE_DELETE_ARRAY(m_pPDF);
		m_nLevel = 0;
		m_valMin = 0;
		m_valMax = 0;
	}

	void Release() {
		SafeDelete2D<T>(m_ppImage, m_nWidth*m_nHeight, m_nDepth);
		m_nWidth = 0;
		m_nHeight = 0;
		m_nDepth = 0;
		m_nWH = 0;

		//		ReleasePDF();
	}

	void Clear() {
		int wh = sizeof(T)*m_nWidth*m_nHeight;
		for (int z = 0; z < m_nDepth; z++) {
			memset(m_ppImage[z], 0, wh);
		}
	}

	void Copy(T** ppSrc) {
		int wh = sizeof(T)*m_nWidth*m_nHeight;
		for (int z = 0; z < m_nDepth; z++) {
			memcpy(m_ppImage[z], ppSrc[z], wh);
		}
	}

	void Export(T** ppDst) {
		int wh = sizeof(T)*m_nWidth*m_nHeight;
		for (int z = 0; z < m_nDepth; z++) {
			memcpy(ppDst[z], m_ppImage[z], wh);
		}
	}

	void Copy(CImage3D<T>* pSrc) {
		m_nWidth = pSrc->GetWidth();
		m_nHeight = pSrc->GetHeight();
		m_nDepth = pSrc->GetDepth();
		m_nWH = pSrc->GetWH();

		for (int z = 0; z < m_nDepth; z++) {
			memcpy(m_ppImage[z], pSrc->GetImage()[z], m_nWH);
		}
	}

	void CopyEx(T** ppSrc, unsigned char** ppMustNotBeIncluded) {
#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (ppMustNotBeIncluded[z][xy] == 0) {
					m_ppImage[z][xy] = ppSrc[z][xy];
				} else {
					m_ppImage[z][xy] = 0;
				}
			}
		}
	}

	void CopyDontCareSize(int w, int h, int d, T** ppSrc) {
		int copyw = Min(m_nWidth, w);
		int copyh = Min(m_nHeight, h);
		int copyd = Min(m_nDepth, d);

#pragma omp parallel for
		for (int z = 0; z < copyd; z++) {
			for (int y = 0; y < copyh; y++) {
				for (int x = 0; x < copyw; x++) {
					int xy = x + y * m_nWidth;
					int srcxy = x + y * w;

					m_ppImage[z][xy] = ppSrc[z][srcxy];
				}
			}
		}
	}

	void ConvertGradMap() {
		unsigned char** ppGrad = NULL;
		SafeNew2D(ppGrad, m_nWH, m_nDepth);

		// 1st scan
		for (int z = 0; z < m_nDepth - 1; z++) {
			for (int y = 0; y < m_nHeight - 1; y++) {
				for (int x = 0; x < m_nWidth - 1; x++) {
					int idx = x + y * m_nWidth;
					int gradx = Abs(m_ppImage[z][idx + 1] - m_ppImage[z][idx]);
					int grady = Abs(m_ppImage[z][idx + m_nWidth] - m_ppImage[z][idx]);
					int gradz = Abs(m_ppImage[z + 1][idx] - m_ppImage[z][idx]);
					if ((gradx + grady + gradz) != 0)	ppGrad[z][idx] = 255;
				}
			}
		}

		// 2nd scan
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				m_ppImage[z][xy] = (T)ppGrad[z][xy];
			}
		}

		SafeDelete2D(ppGrad, m_nWH, m_nDepth);
	}

	T** ExportScale(int scale, int* outw = NULL, int* outh = NULL, int* outd = NULL) {
		if (m_ppImage == NULL) return NULL;

		T** ppPart = NULL;
		int finalw = (int)(((float)m_nWidth / scale) + .5f);
		int finalh = (int)(((float)m_nHeight / scale) + .5f);
		int finald = (int)(((float)m_nDepth / scale) + .5f);
		finald = Max(1, finald);
		int finalwh = finalw * finalh;

		if (!SafeNew2D<T>(ppPart, finalwh, finald)) return NULL;

		for (int z = 0; z < finald; z++) {
			for (int y = 0; y < finalh; y++) {
				for (int x = 0; x < finalw; x++) {
					ppPart[z][x + y * finalw] = m_ppImage[z*scale][(x + y * m_nWidth)*scale];
				}
			}
		}

		if (outw) *outw = finalw;
		if (outh) *outh = finalh;
		if (outd) *outd = finald;

		return ppPart;
	}

	T** ExportScale(int offsetx, int offsety, int offsetz, int* outw = NULL, int* outh = NULL, int* outd = NULL) {
		if (m_ppImage == NULL) return NULL;

		T** ppPart = NULL;
		int finalw = (int)(((float)m_nWidth / offsetx) + .5f);
		int finalh = (int)(((float)m_nHeight / offsety) + .5f);
		int finald = (int)(((float)m_nDepth / offsetz) + .5f);
		finald = Max(1, finald);
		int finalwh = finalw * finalh;

		if (!SafeNew2D<T>(ppPart, finalwh, finald)) return NULL;

		for (int z = 0; z < finald; z++) {
			for (int y = 0; y < finalh; y++) {
				for (int x = 0; x < finalw; x++) {
					ppPart[z][x + y * finalw] = m_ppImage[z*offsetz][(x*offsetx) + (y*offsety)*m_nWidth];
				}
			}
		}

		if (outw) *outw = finalw;
		if (outh) *outh = finalh;
		if (outd) *outd = finald;

		return ppPart;
	}

	T** ExportPart(int fromx, int tox, int fromy, int toy, int fromz, int toz) {
		if (m_ppImage == NULL) return NULL;

		T** ppPart = NULL;
		int w = tox - fromx + 1;
		int h = toy - fromy + 1;
		int d = toz - fromz + 1;
		int wh = w * h;
		if (!SafeNew2D<T>(ppPart, wh, d)) return NULL;

		for (int z = fromz; z <= toz; z++) {
			for (int y = fromy; y <= toy; y++) {
				for (int x = fromx; x <= tox; x++) {
					ppPart[z - fromz][(x - fromx) + (y - fromy)*w] = m_ppImage[z][x + y * m_nWidth];
				}
			}
		}

		return ppPart;
	}

	T** ExportPartScale(int fromx, int tox, int fromy, int toy, int fromz, int toz, int offsetx, int offsety, int offsetz, int* outw = NULL, int*outh = NULL, int* outd = NULL) {
		if (m_ppImage == NULL) return NULL;

		T** ppPart = NULL;
		int w = tox - fromx + 1;
		int h = toy - fromy + 1;
		int d = toz - fromz + 1;

		int finalw = (int)(((float)w / offsetx) + .5f);
		int finalh = (int)(((float)h / offsety) + .5f);
		int finald = (int)(((float)d / offsetz) + .5f);
		int finalwh = finalw * finalh;

		if (!SafeNew2D<T>(ppPart, finalwh, finald)) return NULL;
		/*
		for (int z = fromz, k = 0 ; z <= toz ; z+=offsetz, k++)
		{
		for (int y = fromy, j = 0 ; y <= toy ; y+=offsety, j++)
		{
		for (int x = fromx, i = 0 ; x <= tox ; x+=offsetx, i++)
		{
		ppPart[k][i+j*finalw] = m_ppImage[z][x+y*m_nWidth];
		}
		}
		}
		*/
		for (int z = 0; z < finald; z++) {
			for (int y = 0; y < finalh; y++) {
				for (int x = 0; x < finalw; x++) {
					ppPart[z][x + y * finalw] = m_ppImage[z*offsetz][(x*offsetx) + (y*offsety)*m_nWidth];
				}
			}
		}

		if (outw) *outw = finalw;
		if (outh) *outh = finalh;
		if (outd) *outd = finald;

		return ppPart;
	}

	T** Expansion(int destw, int desth, int destd) {
		if (m_ppImage == NULL) return NULL;

		T** ppExpansion = NULL;
		//	int scale = m_nWidth/destw;
		int scale = destw / m_nWidth;
		int destwh = destw * desth;
		if (!SafeNew2D<T>(ppExpansion, destwh, destd)) return NULL;

#pragma omp parallel for
		for (int z = 0; z < destd; z++) {
			for (int y = 0; y < desth; y++) {
				for (int x = 0; x < destw; x++) {
					int scaledx = Min((x / scale), m_nWidth - 1);
					int scaledy = Min((y / scale), m_nHeight - 1);
					int scaledz = Min((z / scale), m_nDepth - 1);

					int scaleidxxy = scaledx + scaledy * m_nWidth;
					int idxxy = x + y * destw;

					ppExpansion[z][idxxy] = m_ppImage[scaledz][scaleidxxy];
				}
			}
		}
		return ppExpansion;
	}

	T** ExpansionEx(int destw, int desth, int destd) {
		if (m_ppImage == NULL) return NULL;

		T** ppExpansion = NULL;
		int scalew = destw / m_nWidth;
		int scaleh = desth / m_nHeight;
		int scaled = destd / m_nDepth;
		int destwh = destw * desth;
		if (!SafeNew2D<T>(ppExpansion, destwh, destd)) return NULL;

#pragma omp parallel for
		for (int z = 0; z < destd; z++) {
			for (int y = 0; y < desth; y++) {
				for (int x = 0; x < destw; x++) {
					int scaledx = Min((x / scalew), m_nWidth - 1);
					int scaledy = Min((y / scaleh), m_nHeight - 1);
					int scaledz = Min((z / scaled), m_nDepth - 1);

					int scaleidxxy = scaledx + scaledy * m_nWidth;
					int idxxy = x + y * destw;

					ppExpansion[z][idxxy] = m_ppImage[scaledz][scaleidxxy];
				}
			}
		}
		return ppExpansion;
	}

	void ExpansionEx(int destw, int desth, int destd, T** ppExpansion) {
		if (m_ppImage == NULL) return;
		int scalew = destw / m_nWidth;
		int scaleh = desth / m_nHeight;
		int scaled = destd / m_nDepth;
		int destwh = destw * desth;

#pragma omp parallel for
		for (int z = 0; z < destd; z++) {
			for (int y = 0; y < desth; y++) {
				for (int x = 0; x < destw; x++) {
					int scaledx = Min((x / scalew), m_nWidth - 1);
					int scaledy = Min((y / scaleh), m_nHeight - 1);
					int scaledz = Min((z / scaled), m_nDepth - 1);

					int scaleidxxy = scaledx + scaledy * m_nWidth;
					int idxxy = x + y * destw;

					ppExpansion[z][idxxy] = m_ppImage[scaledz][scaleidxxy];
				}
			}
		}
	}

	void ExpansionEx(int scale, int destw, int desth, int destd, T** ppExpansion) {
		if (m_ppImage == NULL) return;
		int destwh = destw * desth;

#pragma omp parallel for
		for (int z = 0; z < destd; z++) {
			for (int y = 0; y < desth; y++) {
				for (int x = 0; x < destw; x++) {
					int scaledx = Min((x / scale), m_nWidth - 1);
					int scaledy = Min((y / scale), m_nHeight - 1);
					int scaledz = Min((z / scale), m_nDepth - 1);

					int scaleidxxy = scaledx + scaledy * m_nWidth;
					int idxxy = x + y * destw;

					ppExpansion[z][idxxy] = m_ppImage[scaledz][scaleidxxy];
				}
			}
		}
	}

	int CalcCenterPoint(TPoint3D<int>* pCenter) {
		TPoint3D<double> pnt;
		pnt.x = 0;
		pnt.y = 0;
		pnt.z = 0;
		int nCnt = 0;
		for (int z = 0; z < m_nDepth; z++) {
			for (int y = 0; y < m_nHeight; y++) {
				for (int x = 0; x < m_nWidth; x++) {
					int idxxy = x + y * m_nWidth;
					if (m_ppImage[z][idxxy] != 0) {
						pnt.x += x;
						pnt.y += y;
						pnt.z += z;
						++nCnt;
					}
				}
			}
		}
		if (nCnt == 0) return 0;
		pCenter->x = (int)(pnt.x / nCnt);
		pCenter->y = (int)(pnt.y / nCnt);
		pCenter->z = (int)(pnt.z / nCnt);
		return nCnt;
	}

	int CalcCenterPoint() {
		return CalcCenterPoint(&m_pntObjCenter);
	}

	int CalcCenterPoint(unsigned char** ppMask) {
		TPoint3D<double> pnt;
		pnt.x = 0;
		pnt.y = 0;
		pnt.z = 0;
		int nCnt = 0;
		//
		for (int z = 0; z < m_nDepth; z++) {
			for (int y = 0; y < m_nHeight; y++) {
				for (int x = 0; x < m_nWidth; x++) {
					int idxxy = x + y * m_nWidth;
					if (ppMask[z][idxxy] != 0) {
						pnt.x += x;
						pnt.y += y;
						pnt.z += z;
						++nCnt;
					}
				}
			}
		}
		if (nCnt == 0)
			return 0;

		m_pntObjCenter.x = (int)(pnt.x / nCnt);
		m_pntObjCenter.y = (int)(pnt.y / nCnt);
		m_pntObjCenter.z = (int)(pnt.z / nCnt);

		return nCnt;
	}

	double CalcMean(unsigned char** ppMask) {
		double mean = 0;

		int nCnt = 0;
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (ppMask[z][xy] != 0) {
					mean += m_ppImage[z][xy];
					++nCnt;
				}
			}
		}
		if (nCnt == 0) return 0;

		mean /= nCnt;

		return mean;
	}

	float CalcMean(float** ppWeight = NULL) {
		float mean = 0;
		float thresh = .5f;

		if (ppWeight == NULL) {
			for (int z = 0; z < m_nDepth; z++) {
				for (int xy = 0; xy < m_nWH; xy++) {
					mean += (float)m_ppImage[z][xy];
				}
			}
			mean /= (float)(m_nWH*m_nDepth);
		} else {
			int nCnt = 0;
			for (int z = 0; z < m_nDepth; z++) {
				for (int xy = 0; xy < m_nWH; xy++) {
					if (ppWeight[z][xy] > thresh) {
						mean += (float)m_ppImage[z][xy];
						++nCnt;
					}
				}
			}
			mean /= (float)(nCnt);
		}

		return mean;
	}

	void CalcMeanInOut(float** ppWeight, float* cin, float* cout, float thresh = .5f) {
		float c1 = 0, c2 = 0;
		int nC1Cnt = 0, nC2Cnt = 0;

#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (ppWeight[z][xy] > thresh) {
					c1 += (float)m_ppImage[z][xy];
					++nC1Cnt;
				} else {
					c2 += (float)m_ppImage[z][xy];
					++nC2Cnt;
				}
			}
		}

		*cin = c1 / (float)(nC1Cnt);
		*cout = c2 / (float)(nC2Cnt);
	}

	void CalcMeanInOut(float** ppWeight, unsigned char** ppMask, float* cin, float* cout, float thresh = .5f) {
		float c1 = 0, c2 = 0;
		int nC1Cnt = 0, nC2Cnt = 0;

#pragma omp parallel for
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (ppMask[z][xy] == 0) continue;
				if (ppWeight[z][xy] > thresh) {
					c1 += (float)m_ppImage[z][xy];
					++nC1Cnt;
				} else {
					c2 += (float)m_ppImage[z][xy];
					++nC2Cnt;
				}
			}
		}

		*cin = c1 / (float)(nC1Cnt);
		*cout = c2 / (float)(nC2Cnt);
	}

	void CalcMinMax(T* pMin, T* pMax) {
		double valMin = 999999999;
		double valMax = -999999999;

		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (valMin > m_ppImage[z][xy]) valMin = m_ppImage[z][xy];
				if (valMax < m_ppImage[z][xy]) valMax = m_ppImage[z][xy];
			}
		}
		*pMin = (T)valMin;
		*pMax = (T)valMax;
	}

	void CalcMinMax() {
		CalcMinMax(&m_valMin, &m_valMax);
		m_fScale = (float)(m_valMax - m_valMin);
	}

	void CalcMinMaxWithMask(T* pMin, T* pMax, unsigned char** ppMask) {
		double valMin = 999999999;
		double valMax = -999999999;

		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (ppMask[z][xy] == 0) continue;
				if (valMin > m_ppImage[z][xy]) valMin = m_ppImage[z][xy];
				if (valMax < m_ppImage[z][xy]) valMax = m_ppImage[z][xy];
			}
		}
		*pMin = (T)valMin;
		*pMax = (T)valMax;
	}

	void Thresholding(T minnumb, T outnumb) {
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy] > minnumb)		m_ppImage[z][xy] = outnumb;
				else								m_ppImage[z][xy] = 0;
			}
		}
	}

	void Thresholding(unsigned char** ppMask, T minnumb, T outnumb) {
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				//
				if (ppMask[z][xy] == 0) continue;
				//
				if (m_ppImage[z][xy] > minnumb)		m_ppImage[z][xy] = outnumb;
				else								m_ppImage[z][xy] = 0;
			}
		}
	}

	void Thresholding(unsigned char** ppMask, T minnumb, unsigned char** ppOut, T outnumb) {
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				//
				if (ppMask[z][xy] == 0) continue;
				//
				if (m_ppImage[z][xy] > minnumb)		ppOut[z][xy] = (unsigned char)outnumb;
				else								ppOut[z][xy] = 0;
			}
		}
	}

	unsigned char** ThresholdingEx(T minnumb, unsigned char outnumb = 255) {
		unsigned char** ppOut = NULL;
		SafeNew2D(ppOut, m_nWH, m_nDepth);

		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy] > minnumb)		ppOut[z][xy] = outnumb;
			}
		}

		return ppOut;
	}

	// threshold 적용하여 생긴 마스크를 한 평면에 프로젝션시켜 프로젝션 마스크 생성
	unsigned char* CreateProjImageFromVolume() {
		unsigned char* pProjMask = new  unsigned char[m_nWH];
		memset(pProjMask, 0, sizeof(unsigned char)*m_nWH);

		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy]) {
					pProjMask[xy] = 255;
				}
			}
		}

		return pProjMask;
	}

	// threshold 적용하여 생긴 마스크를 한 평면에 프로젝션시켜 프로젝션 마스크값 저장
	void CreateProjImageFromVolume(unsigned char* pProjMask) {
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy]) {
					pProjMask[xy] = 255;
				}
			}
		}
	}

	// threshold 적용하여 생긴 마스크를 한 평면에 프로젝션시켜 프로젝션 마스크 생성
	void ThreshAndCreateProjImageFromVolume(int threshold, unsigned char* pProjMask) {
		for (int z = 0; z < m_nDepth; z++) {
			for (int xy = 0; xy < m_nWH; xy++) {
				if (m_ppImage[z][xy] > threshold) {
					pProjMask[xy] = 255;
				}
			}
		}
	}
};