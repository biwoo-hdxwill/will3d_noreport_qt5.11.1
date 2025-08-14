#pragma once

#include "../../../Common/Common/W3Types.h"
#include "VpGlobalDefine.h"
#include "../imageprocessing_global.h"

typedef  int	conncomtype;

typedef struct _SCompInfo {
	conncomtype	_nLabel;
	int			_nArea = 0;
	TPoint3D<int>	_pntCenter;
	TPoint3D<int>	_pntMin;
	TPoint3D<int>	_pntMax;
} SCompInfo;

class IMAGEPROCESSING_EXPORT CVpConnComponent {
public:
	enum eSortType {
		AREA,
		BNDBOX,
		LENGTH,
		XCOORD,
		YCOORD,
		XCOORDUP,
		YCOORDUP,
	};

	enum eMorphology {
		EROSION,
		DILATION,
		OPEN,
		CLOSE,
	};

private:
	conncomtype** m_ppComponent;
	conncomtype* m_pComponent;
	std::vector<SCompInfo> m_vecCompInfo;

	int m_nWidth;
	int m_nHeight;
	int m_nDepth;

	unsigned char m_bytExpValue;
	//	void Union(int x, int y, int** lab);

public:
	CVpConnComponent();
	virtual ~CVpConnComponent();

	bool Create(int w, int h, int d);
	void Release();
	int Do(unsigned char** ppVolData, int minsize = 0);
	void LeaveOneComponent(int idx);
	unsigned char** ExportComponent3D(int idx, int* pVol = NULL);
	unsigned char* ExportComponent2D(int idx, int* pArea = NULL);
	unsigned char* ExportComponents2D(std::vector<int>& vecidx, int* pArea = NULL);
	unsigned char** ExportComponent3DFilter(int minsize, int* pCompCnt = NULL);
	unsigned char* ExportComponent2DFilter(int minsize, int* pCompCnt = NULL);
	unsigned char* ExportComponent2DFilterOnlyCenter(int minsize, int* pCompCnt = NULL);

	// export component based its size
	unsigned char* ExportComponent2DRect(int idx, int* outw = NULL, int* outh = NULL, int* outarea = NULL);

	unsigned char** Export3D(int* pComp = NULL);
	unsigned char* Export2D(int* pComp = NULL);
	unsigned char* Export2DEx(int* pComp = NULL);

	bool Create(int w, int h);
	int Do(unsigned char* pImgData, int minsize = 0);
	int DoAfterMorph(unsigned char* pImgData, eMorphology emorph = CLOSE, int kernel = 3, int minsize = 0);
	int DoAfterMorphEx(unsigned char* pImgData, eMorphology emorph, int kernel1 = 3, int kernel2 = 3, int minsize = 0);

	inline int GetWidth() { return m_nWidth; }
	inline int GetHeight() { return m_nHeight; }
	inline int GetDepth() { return m_nDepth; }
	inline void SetExportValue(unsigned char val) { m_bytExpValue = val; }

	inline	std::vector<SCompInfo> GetComponentInfo() { return m_vecCompInfo; }
	inline	SCompInfo* GetComponentInfo(int n) { return &m_vecCompInfo[n]; }
	inline conncomtype** GetComponent() { return m_ppComponent; }

	void Sort(eSortType esort = AREA);
	bool Remove(int idx);
	bool RemoveFromPoint(int x, int y);
	bool RemoveFromPoint(int x, int y, int z);
	bool LeaveFromPoint(int x, int y);
	bool LeaveFromPoint(int x, int y, int z);

	bool LeaveOnly(int* pIdx, int nCnt);
	bool LeaveOnly(std::vector<int>& vecIdx);

	TPoint3D<int> GetImmediatePoint(int idx);
	TPoint3D<int> GetCenterXImmediatePoint(int idx);
	TPoint3D<int> GetCenterYImmediatePoint(int idx);

	int CalcMeanIntensity(int idx, voltype* pData);
	int CalcMeanIntensity(int idx, voltype** ppData);

	// 2010.11.05
	int GetComponentResult(unsigned char** ppResult);
	int GetComponentResult(int idx, unsigned char** ppResult);

	// 2011.07.06
	bool LeaveOnly();
	inline	int GetComponentCnt() { return (int)m_vecCompInfo.size(); }
	inline	bool IsEmpty() { if (GetComponentCnt() == 0) return true;	return false; }
};
