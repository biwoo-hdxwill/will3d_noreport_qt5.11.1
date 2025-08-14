#pragma once

#include "VpProcBase.h"

#include "../../../Common/Common/W3Types.h"
#include "../imageprocessing_global.h"

#define THINDIR		6
#define THINNEIGH	26

class IMAGEPROCESSING_EXPORT CVpMorph : public CVpProcBase {
private:
	unsigned char m_bytImageValue;
	TPoint3D<int> m_pntDir[THINDIR];

	typedef struct {
		int *_pIdx = nullptr;
		int _nCnt = 0;
	} SLUT;
	SLUT m_S26[THINNEIGH];
	SLUT m_S18[THINNEIGH];

	void StartThinning();
	void SetLUT();
	void EndThinning();

public:
	enum eMaskOperator { ADD, SUBTRACT };

	enum eDirection { U = 0, D, N, S, E, W };

	CVpMorph();
	virtual ~CVpMorph();

	inline void SetValue(unsigned char val = 1) { m_bytImageValue = val; }

	bool Dilation2D(int nSESize = 3);
	bool Erosion2D(int nSESize = 3);
	bool Dilation3D(int nSESize = 3);
	bool Erosion3D(int nSESize = 3);
	bool Dilation3DperSlice(int nSESize = 3);
	bool Erosion3DperSlice(int nSESize = 3);
	//	 unsigned char** Thining3D();

	bool Open2D(int nSESize = 3);
	bool Close2D(int nSESize = 3);

	bool Open3D(int nSESize = 3);
	bool Close3D(int nSESize = 3);

	bool Open2D(int nErosSize, int nDilSize);
	bool Close2D(int nErosSize, int nDilSize);

	void Union3D(int zmin, int zmax, unsigned char** ppBinData2);
	void Union3D(unsigned char** ppBinData2);
	void Substract3D(unsigned char** ppSubData);

	void Mask2D(unsigned char* pMask, eMaskOperator emaskop = ADD);
	void Mask3D(unsigned char** pMask, eMaskOperator emaskop = ADD);

	// morphology operation only mask's value is val
	bool Erosion2D(unsigned char val, int nSESize);
	bool Dilation2D(unsigned char val, int nSESize);
	bool Erosion3D(unsigned char val, int nSESize);
	bool Dilation3D(unsigned char val, int nSESize);
	bool Erosion3DperSlice(unsigned char val, int nSESize);
	bool Dilation3DperSlice(unsigned char val, int nSESize);

	// thinning for 3d skeletonize - (Z3, 26, 6, B)
	bool Thinning3D(unsigned char** ppSkeleton);
	int SubIter(unsigned char** ppMask, eDirection direction);
	bool IsBorderPoint(unsigned char** ppMask, eDirection direction, int x, int y, int z);
	void Collect26Neighbors(unsigned char** ppMask, int x, int y, int z, TPoint3D<int>* pOutPnt);
	bool IsEndPoint(unsigned char** ppMask, TPoint3D<int>* pPnt);
	bool IsSimple(unsigned char** ppMask, TPoint3D<int>* pPnt);
	bool IsCond2Satisfied(unsigned char** ppMask, TPoint3D<int>* pPnt);
	bool IsCond4Satisfied(unsigned char** ppMask, TPoint3D<int>* pPnt);

	void Copy(unsigned char** ppSrc);
	void Inverse();

	void LeaveOne(int kernel = 3);

private:
	int countNonZeroNeighbors(std::vector<int>& neighbors);
	int countTransitionPatterns(std::vector<int>& neighbors);
	bool checkCondition(std::vector<int>& p);
	std::vector<int> getNeighbors(unsigned char* pGray, int x, int y);
};
