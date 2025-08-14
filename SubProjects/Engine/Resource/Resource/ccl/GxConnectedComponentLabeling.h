#pragma once

// smseo : 안쓰이는 코드라 막아둠
#if 0

#include "../resource_global.h"
#include "../../../Common/Common/W3Types.h"

#define BKValue 0


typedef int					GxVolumetype;
typedef GxVolumetype**		GxVolume16s;

typedef struct 
{
	int x;
	int y;
	int z;
} SCCLPnt3D;

class GxConnectedComponentLabeling
{
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;

	GxVolume16s m_volMap;
	struct LabelTable {
		int * pTable;
		int nTableCnt;
	} m_stLabelTable;
	int m_nLabelCnt;

	void SetLabel( int x, int y, int z );
	void SetLabel(int idx, int x, int y, int z );
	int FindFinalLabel( int nNowLabel);

	int MinLabel( int a, int b) {
		return (a<b) ? a : b;
	}
	int MaxLabel( int a, int b) {
		return (a>b) ? a : b;
	}

	int MinLabel( int a, int b, int c)
	{
		if( a < b)
			return (a<c) ? a : c;
		else
			return (b<c) ? b : c;
	}
	int MaxLabel( int a, int b, int c)
	{
		if( a > b)
			return (a>c) ? a : c;
		else
			return (b>c) ? b : c;
	}
	int MidLabel( int a, int b, int c)	
	{
		if( a >= b && b >= c)
			return b;
		else if( a >= c && c >= b)
			return c;
		else if( b >= a && a >= c)
			return a;
		else if( b >= c && c >= a)
			return c;
		else if( c >= a && a >= b)
			return a;
		else if( c >= b && b >= a)
			return b;
		else
			return -1;
	}

	int MapFillingWithSortedLabel();

public:
	GxConnectedComponentLabeling(void);
	~GxConnectedComponentLabeling(void);

	int Get3DConnectedComponent();
	int Get3DConnectedComponent(int idx);
	bool SortVolumeSize();

	bool Create(int w, int h, int d, unsigned char** ppBinData);
	int GetVolData(unsigned char** ppOutBinData);
	int GetVolData(int sizerank, unsigned char** ppOutBinData);
	int GetVolData(int* pSizeRank, int nCnt, unsigned char** ppOutBinData);
	int GetVolData(int sizerank, unsigned char** ppOutBinData, SCCLPnt3D* pCenterPnt);
	int GetMaxVolData(unsigned char** ppOutBinData);
	int GetUpVolData(unsigned char** ppOutBinData);
	int GetDnVolData(unsigned char** ppOutBinData);
	void GetUpVolDataWithMeanZ(unsigned char** ppOutBinData);
	void GetDnVolDataWithMeanZ(unsigned char** ppOutBinData);
	void Release();

inline	int GetLabelCnt() {return m_nLabelCnt;}
};
#endif
