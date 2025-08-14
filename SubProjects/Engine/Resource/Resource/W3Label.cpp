#include "W3Label.h"
#include "W3Image3D.h"
#include "W3Mask.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Point3D.h"

CW3Label::CW3Label(void) :
	m_pParent(nullptr),	
	m_nWidth(0),
	m_nHeight(0),
	m_nDepth(0),
	m_ppushData(nullptr),	
	m_ushMaxLabel(0),
	m_ushCurLabel(0),
	m_pbytActionFlag(nullptr),
	m_pDrawMask(nullptr),
	m_eCurrentActType(ACT4CNTR_NONE),
	m_ppMaskFlag(nullptr)
{
}

CW3Label::CW3Label(CW3Image3D* pParent) :
	m_pParent(pParent),
	m_nWidth(0),
	m_nHeight(0),
	m_nDepth(0),
	m_ppushData(nullptr),
	m_ushMaxLabel(0),
	m_ushCurLabel(0),
	m_pbytActionFlag(nullptr),
	m_pDrawMask(nullptr),
	m_eCurrentActType(ACT4CNTR_NONE),
	m_ppMaskFlag(nullptr)
{
	allocBuffer();
}

CW3Label::~CW3Label(void)
{
	cleanUp();
}

void CW3Label::cleanUp(void)
{
	if (m_ppushData)
	{
		for( int i=0; i<m_nDepth; i++ )
		{
			SAFE_DELETE_ARRAY(m_ppushData[i]);
		}
		SAFE_DELETE_ARRAY(m_ppushData);
	}	

	SAFE_DELETE_ARRAY(m_pbytActionFlag);
	m_vecActLabel.clear();

	SAFE_DELETE_VOLUME(m_ppMaskFlag, m_nDepth);

/*
	std::vector<W3BYTE**>::iterator itr;
	for (itr = m_vecppLabelToMask.begin() ; itr != m_vecppLabelToMask.end() ; itr++)
	{
		for( int i=0; i<m_nDepth; i++ )
		{
			SAFE_DELETE_ARRAY((*itr)[i]);
		}
		SAFE_DELETE_ARRAY((*itr));
	}
*/
}

W3USHORT** CW3Label::createBuffer(W3INT width, W3INT height, W3INT depth)
{
    if(m_ppushData)    cleanUp();
    int szSlice = width*height;
    m_nWidth = width;
    m_nHeight = height;
    m_nDepth = depth;
	m_ppushData = SAFE_ALLOC_VOLUME(W3USHORT, m_nDepth, szSlice);
	for(int i = 0; i < m_nDepth; i++)	memset(m_ppushData[i], 0, sizeof(W3USHORT)*szSlice);
    return m_ppushData;
}

void CW3Label::allocBuffer(void)
{
	if(m_pParent)
	{
        m_nWidth  = m_pParent->width();
        m_nHeight = m_pParent->height();
        m_nDepth  = m_pParent->depth();
	}
	createBuffer(m_nWidth, m_nHeight, m_nDepth);
}

void CW3Label::resetBuffer(void)
{
	for(int i = 0; i < m_nDepth; i++)
	{
		if(m_ppushData)
			std::memset(m_ppushData[i], 0, sizeof(W3SHORT)*m_nWidth*m_nHeight);
		if (m_ppMaskFlag)
			std::memset(m_ppMaskFlag[i], 0, sizeof(W3BOOL)*m_nWidth*m_nHeight);
	}
}

void CW3Label::setValue(W3INT x, W3INT y, W3INT z, W3USHORT val)
{
	m_ppushData[z][x+y*m_nWidth] = val;
}

W3USHORT CW3Label::addValue(W3INT x, W3INT y, W3INT z, W3USHORT addval)
{
	m_ppushData[z][x+y*m_nWidth] += addval;
	return m_ppushData[z][x+y*m_nWidth];
}

W3USHORT CW3Label::addValue(W3INT xy, W3INT z, W3USHORT addval)
{
	m_ppushData[z][xy] += addval;
	return m_ppushData[z][xy];
}


void CW3Label::init(CW3Image3D* pParent)
{
	m_pParent = pParent;
	allocBuffer();
	
	SAFE_DELETE_ARRAY(m_pbytActionFlag);
	m_pbytActionFlag = new W3BYTE[USHRT_MAX];
	memset(m_pbytActionFlag, 0, sizeof(W3BYTE)*USHRT_MAX);

	m_vecActLabel.clear();
}

void CW3Label::reset()
{
	resetBuffer();
	memset(m_pbytActionFlag, 0, sizeof(W3BYTE)*USHRT_MAX);
	m_vecActLabel.clear();

	m_ushMaxLabel = 0;
	m_ushCurLabel = 0;
	m_eCurrentActType = ACT4CNTR_NONE;
}

W3BOOL CW3Label::isState(W3INT x, W3INT y, W3INT z, EACT4CNTR_TYPE type)
{
	if (m_pbytActionFlag[(W3INT)(m_ppushData[z][x+y*m_nWidth])] == (W3INT)type) return true;
	return false;

}


void CW3Label::getCoord(W3INT inx, W3INT iny, W3INT* outx, W3INT* outy, W3INT* outz)
{
	CW3Vector3D ptPlane = m_vMaskPtStart + m_vMaskDirX*inx + m_vMaskDirY*iny;

	W3INT x = static_cast<W3INT>(ptPlane.x() + 0.5f);
	W3INT y = static_cast<W3INT>(ptPlane.y() + 0.5f);
	W3INT z = static_cast<W3INT>(ptPlane.z()/m_SpacingZ + 0.5f);

	if (x>=0 && x<m_nWidth && y>=0 && y<m_nHeight && z>=0 && z<m_nDepth) 
	{
		*outx = x;
		*outy = y;
		*outz = z;
	}
	else
	{
		*outx = 0;
		*outy = 0;
		*outz = 0;
	}

}

void CW3Label::update(W3INT x, W3INT y)
{
	W3INT outx, outy, outz;
	getCoord(x, y, &outx, &outy, &outz);

	W3INT z = outz;
	W3INT xy = outx + outy*m_nWidth;
	if (!m_ppMaskFlag[z][xy])
	{
		W3USHORT ushvalue = addValue(outx, outy, outz, m_ushCurLabel);
		if (m_ushMaxLabel < ushvalue) m_ushMaxLabel = ushvalue;
		m_pbytActionFlag[ushvalue] = static_cast<W3BYTE>(m_eCurrentActType);
		m_ppMaskFlag[z][xy] = true;
	}
}

void CW3Label::updateCorrection(W3INT x, W3INT y, W3BOOL bIsErase)
{
	W3INT outx, outy, outz;
	getCoord(x, y, &outx, &outy, &outz);

	W3INT z = outz;
	W3INT xy = outx + outy*m_nWidth;
	if (!m_ppMaskFlag[z][xy])
	{
		W3USHORT ushvalue = addValue(outx, outy, outz, m_ushCurLabel);
		if (m_ushMaxLabel < ushvalue) m_ushMaxLabel = ushvalue;
		//
		if (bIsErase)
			m_pbytActionFlag[ushvalue] = static_cast<W3BYTE>(ACT4CNTR_MASKERASE);
		else
			m_pbytActionFlag[ushvalue] = static_cast<W3BYTE>(ACT4CNTR_MASKFILL);
		//
		m_ppMaskFlag[z][xy] = true;
	}
}


void CW3Label::updateDirect(W3INT x, W3INT y, W3INT z)
{
	W3INT xy = x + y*m_nWidth;
	if (!m_ppMaskFlag[z][xy])
	{
		W3USHORT ushvalue = addValue(x, y, z, m_ushCurLabel);
		if (m_ushMaxLabel < ushvalue) m_ushMaxLabel = ushvalue;
		m_pbytActionFlag[ushvalue] = static_cast<W3BYTE>(m_eCurrentActType);
		m_ppMaskFlag[z][xy] = true;
	}
}

void CW3Label::updateDirect(W3INT xy, W3INT z)
{
	if (!m_ppMaskFlag[z][xy])
	{
		W3USHORT ushvalue = addValue(xy, z, m_ushCurLabel);
		if (m_ushMaxLabel < ushvalue) m_ushMaxLabel = ushvalue;
		m_pbytActionFlag[ushvalue] = static_cast<W3BYTE>(m_eCurrentActType);
		m_ppMaskFlag[z][xy] = true;
	}
}


void CW3Label::doAct(EACT4CNTR_TYPE eType)
{
	m_ushCurLabel = m_ushMaxLabel + 1;
	m_vecActLabel.push_back(m_ushCurLabel);
	m_pbytActionFlag[m_ushCurLabel] = static_cast<W3BYTE>(eType);
	m_eCurrentActType = eType;

	// create mask flag
	SAFE_DELETE_VOLUME(m_ppMaskFlag, m_nDepth);
	int szSlice = m_nWidth*m_nHeight;
    m_ppMaskFlag = SAFE_ALLOC_VOLUME(W3BOOL, m_nDepth, szSlice);
	for(int i = 0; i < m_nDepth; i++) memset(m_ppMaskFlag[i], 0, sizeof(W3BOOL)*szSlice);
}

void CW3Label::finishAct()
{
	SAFE_DELETE_VOLUME(m_ppMaskFlag, m_nDepth);
}


void CW3Label::undoAct()
{
	W3INT nSize = (W3INT) m_vecActLabel.size();
	if (nSize == 0) return;

	W3USHORT lastLabel = m_vecActLabel.at(nSize-1);
	m_vecActLabel.pop_back();

	// undo
	W3INT wh = m_nWidth*m_nHeight;
#pragma omp parallel for
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			// 1. label
			if (m_ppushData[z][xy] >= lastLabel)	
				m_ppushData[z][xy] -= lastLabel;

			// 2. mask
			W3USHORT curlabel = m_ppushData[z][xy];
			if (curlabel != 0 && m_pbytActionFlag[curlabel] == (W3BYTE) EACT4CNTR_TYPE::ACT4CNTR_MASKFILL)
			{
				m_pDrawMask->setValue(xy, z, 255);
			}
			else if (curlabel != 0 && m_pbytActionFlag[curlabel] == (W3BYTE) EACT4CNTR_TYPE::ACT4CNTR_MASKLEAVE)
			{
					
			}
			else
			{
				m_pDrawMask->setValue(xy, z, 0);
			}
		}
	}	
}

