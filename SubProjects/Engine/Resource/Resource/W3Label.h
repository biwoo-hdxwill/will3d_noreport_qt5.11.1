#pragma once
/*=========================================================================

File:		class CW3Label
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/

#include "resource_global.h"
#include "W3Resource.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Vector3D.h"

class CW3Image3D;
class CW3Mask;
class RESOURCE_EXPORT CW3Label
{
public:
	CW3Label(void);
	CW3Label(CW3Image3D* pParent);
	virtual ~CW3Label(void);

public:
	// public functions.
	void cleanUp(void);
	void resetBuffer();
	W3USHORT**  createBuffer(W3INT width, W3INT height, W3INT depth);
	void init(CW3Image3D* pParent);
	void reset();
		
private:
	// private functions.
	void allocBuffer(void);

private:
	// private member fields.
	CW3Image3D* m_pParent;
	W3UINT		m_nWidth;
    W3UINT		m_nHeight;
    W3UINT		m_nDepth;
	W3USHORT**	m_ppushData;
	W3USHORT	m_ushMaxLabel;
	W3USHORT	m_ushCurLabel;

	W3BYTE*					m_pbytActionFlag;
	std::vector<W3USHORT>	m_vecActLabel;

	// for mask draw
	CW3Mask*		m_pDrawMask;
	W3BOOL**		m_ppMaskFlag;
	W3FLOAT			m_SpacingZ;		
	CW3Vector3D		m_vMaskDirX;
	CW3Vector3D		m_vMaskDirY;
	CW3Vector3D		m_vMaskPtStart;
	EACT4CNTR_TYPE  m_eCurrentActType;
	
public:
	inline void initParam(W3FLOAT SpacingZ, CW3Vector3D vMaskPtStart, CW3Vector3D vMaskDirX, CW3Vector3D vMaskDirY)
	{ m_SpacingZ = SpacingZ; m_vMaskPtStart = vMaskPtStart; m_vMaskDirX = vMaskDirX; m_vMaskDirY = vMaskDirY; }
	inline	W3BOOL incValue()	{if (m_ushMaxLabel < USHRT_MAX) { m_ushMaxLabel++; return true; } return false;}
	inline	W3BOOL decValue()	{if (m_ushMaxLabel > 0)			{ m_ushMaxLabel--; return true; } return false;}
	void setValue(W3INT x, W3INT y, W3INT z, W3USHORT val);
	W3USHORT addValue(W3INT x, W3INT y, W3INT z, W3USHORT addval);
	W3USHORT addValue(W3INT xy, W3INT z, W3USHORT addval);
	void update(W3INT x, W3INT y);								// for current plane
	void updateCorrection(W3INT x, W3INT y, W3BOOL bIsErase);	// for correction
	void updateDirect(W3INT x, W3INT y, W3INT z);		// for volume
	void updateDirect(W3INT xy, W3INT z);		// for volume
	void getCoord(W3INT inx, W3INT iny, W3INT* outx, W3INT* outy, W3INT* outz);
	void doAct(EACT4CNTR_TYPE eType);
	void undoAct();
	void finishAct();

	W3BOOL isState(W3INT x, W3INT y,W3INT z, EACT4CNTR_TYPE type);
	inline void setMask(CW3Mask* pMask) {m_pDrawMask = pMask;}
		
};
