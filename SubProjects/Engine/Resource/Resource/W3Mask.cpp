#include "W3Mask.h"
#include "W3Image3D.h"
#include "../../Common/Common/W3Memory.h"
#include "ccl/GxConnectedComponentLabeling.h"

CW3Mask::CW3Mask(void) :
	CW3Resource(ERESOURCE_TYPE::MASK),
	m_pParent(nullptr),
	m_bVisible(false),
	m_nWidth(0),
	m_nHeight(0),
	m_nDepth(0),
	m_ppcData(nullptr),
	m_fPixelSpacing(1.0f),
	m_fSliceSpacing(1.0f),
	m_foregroundColor(Qt::red),
	m_backgroundColor(Qt::transparent)
{
//	m_ushMaxLabel = 0;
//	m_ppLabelToImage = NULL;
	m_bytImageValue = 255;
}

CW3Mask::CW3Mask(CW3Image3D* pParent) :
	CW3Resource(ERESOURCE_TYPE::MASK),
	m_pParent(pParent),
	m_bVisible(false),
	m_nWidth(0),
	m_nHeight(0),
	m_nDepth(0),
	m_ppcData(nullptr),
	m_fPixelSpacing(1.0f),
	m_fSliceSpacing(1.0f),
	m_foregroundColor(Qt::red),
	m_backgroundColor(Qt::transparent)
{
//	m_ushMaxLabel = 0;
//	m_ppLabelToImage = NULL;
	m_bytImageValue = 255;

	this->allocBuffer();
}

CW3Mask::~CW3Mask(void)
{
	this->cleanUp();
}

void CW3Mask::cleanUp(void)
{
	if (m_ppcData)
	{
		for( int i=0; i<m_nDepth; i++ )
		{
			SAFE_DELETE_ARRAY(m_ppcData[i]);
		//	SAFE_DELETE_ARRAY(m_ppLabelToImage[i]);
		}
		SAFE_DELETE_ARRAY(m_ppcData);
	//	SAFE_DELETE_ARRAY(m_ppLabelToImage);
	}	

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

W3UCHAR** CW3Mask::createBuffer(W3INT width, W3INT height, W3INT depth)
{
    if(isValid())
        cleanUp();
    int szSlice = width*height;
    m_nWidth = width;
    m_nHeight = height;
    m_nDepth = depth;
	m_ppcData = SAFE_ALLOC_VOLUME(W3UCHAR, m_nDepth, szSlice);
	for(int i = 0; i < m_nDepth; i++)
		std::memset(m_ppcData[i], 0, szSlice);
    return m_ppcData;
}

void CW3Mask::allocBuffer(void)
{
    if(m_pParent)
    {
        m_fPixelSpacing = m_pParent->pixelSpacing();
        m_fSliceSpacing = m_pParent->sliceSpacing();

        m_nWidth  = m_pParent->width();
        m_nHeight = m_pParent->height();
        m_nDepth  = m_pParent->depth();

		if(m_nWidth <= 0 || m_nHeight <= 0 || m_nDepth <= 0) 
			throw CW3MaskException(CW3MaskException::EID::INVALID_SIZE, "Invalid parent's size.");
    }
    else
		throw CW3MaskException(CW3MaskException::EID::PARENT_NULLPTR, "Parent nullptr.");

    this->createBuffer(m_nWidth, m_nHeight, m_nDepth);
}

void CW3Mask::resetBuffer(void)
{
    if(!isValid()) return;

	for(int i = 0; i < m_nDepth; i++)
		std::memset(m_ppcData[i], 0, m_nWidth*m_nHeight);
}

/*
void CW3Mask::setLabel(W3INT x, W3INT y, W3INT z)
{
}

void CW3Mask::eraseLabel(W3UCHAR label)
{
}

void CW3Mask::eraseMaxLabel()
{
}
*/


void CW3Mask::copy(W3BYTE** ppSrc)
{
	if (m_ppcData)
	{
		for (int z = 0 ; z < m_nDepth ; z++)
			memcpy(m_ppcData[z], ppSrc[z], sizeof(W3BYTE)*m_nWidth*m_nHeight);
	}
}

void CW3Mask::erosion(W3INT nSESize)
{
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;
	int wh = w*h;

	W3BYTE **ppTemp = SAFE_ALLOC_VOLUME(W3BYTE, d, wh);
	for (int z = 0 ; z < d ; z++)	memcpy(ppTemp[z], m_ppcData[z], sizeof(W3BYTE)*wh);
	
	// loop all voxel
	int nHalfSESize = nSESize >> 1;
	int nFullSESize = nSESize*nSESize*nSESize;
	int nFullSEValue = m_bytImageValue * nFullSESize;

#pragma omp parallel for
	for (int z = nHalfSESize ; z < d-nHalfSESize ; z++)
	{
		for (int y = nHalfSESize ; y < h-nHalfSESize ; y++)
		{
			for (int x = nHalfSESize ; x < w-nHalfSESize ; x++)
			{
				int sum = 0;
				//
				for (int n = -nHalfSESize ; n <= nHalfSESize ; n++)
				{
					for (int m = -nHalfSESize ; m <= nHalfSESize ; m++)
					{
						for (int l = -nHalfSESize ; l <= nHalfSESize ; l++)
						{
							int idxx = x+l;
							int idxy = y+m;
							int idxz = z+n;
							int idxxy = idxx + idxy*w;

							if ((idxx < 0) || (idxx > w-1) || (idxy < 0) || (idxy > h-1) || (idxz < 0) || (idxz > d-1)) continue;

							sum += m_ppcData[idxz][idxxy];
						}						
					}
				}
				//
				if (sum != nFullSEValue)
				{
					for (int n = -nHalfSESize ; n <= nHalfSESize ; n++)
					{
						for (int m = -nHalfSESize ; m <= nHalfSESize ; m++)
						{
							for (int l = -nHalfSESize ; l <= nHalfSESize ; l++)
							{
								int idxx = x+l;
								int idxy = y+m;
								int idxz = z+n;
								int idxxy = idxx + idxy*w;

								if ((idxx < 0) || (idxx > w-1) || (idxy < 0) || (idxy > h-1) || (idxz < 0) || (idxz > d-1)) continue;

								ppTemp[idxz][idxxy] = 0;
							}						
						}
					}
				}
			}
		}
	}

	for (int z = 0 ; z < d ; z++)
	{
		memcpy(m_ppcData[z], ppTemp[z], sizeof(W3BYTE)*wh);
	}
	SAFE_DELETE_VOLUME(ppTemp, d);	
}


void CW3Mask::dilation(W3INT nSESize)
{
	int w = m_nWidth;
	int h = m_nHeight;
	int d = m_nDepth;
	int wh = w*h;
	
	W3BYTE **ppTemp = SAFE_ALLOC_VOLUME(W3BYTE, d, wh);
	for (int z = 0 ; z < d ; z++)	memset(ppTemp[z], 0, sizeof(W3BYTE)*wh);

	// 2. loop all voxel
	int nHalfSESize = nSESize >> 1;
#pragma omp parallel for
	for (int z = nHalfSESize ; z < d-nHalfSESize ; z++)
	{
		for (int y = nHalfSESize ; y < h-nHalfSESize ; y++)
		{
			for (int x = nHalfSESize ; x < w-nHalfSESize ; x++)
			{
				int sum = 0;
				//
				for (int n = -nHalfSESize ; n <= nHalfSESize ; n++)
				{
					for (int m = -nHalfSESize ; m <= nHalfSESize ; m++)
					{
						for (int l = -nHalfSESize ; l <= nHalfSESize ; l++)
						{
							int idxx = x+l;
							int idxy = y+m;
							int idxz = z+n;
							int idxxy = idxx + idxy*w;

							if ((idxx < 0) || (idxx > w-1) || (idxy < 0) || (idxy > h-1) || (idxz < 0) || (idxz > d-1)) continue;

							sum += m_ppcData[idxz][idxxy];
						}						
					}
				}
				//
				if (sum > 0)
				{
					for (int n = -nHalfSESize ; n <= nHalfSESize ; n++)
					{
						for (int m = -nHalfSESize ; m <= nHalfSESize ; m++)
						{
							for (int l = -nHalfSESize ; l <= nHalfSESize ; l++)
							{
								int idxx = x+l;
								int idxy = y+m;
								int idxz = z+n;
								int idxxy = idxx + idxy*w;

								if ((idxx < 0) || (idxx > w-1) || (idxy < 0) || (idxy > h-1) || (idxz < 0) || (idxz > d-1)) continue;

								ppTemp[idxz][idxxy] = m_bytImageValue;
							}						
						}
					}
				}
			}
		}
	}

	for (int z = 0 ; z < d ; z++)
	{
		memcpy(m_ppcData[z], ppTemp[z], sizeof(W3BYTE)*wh);
	}
	SAFE_DELETE_VOLUME(ppTemp, d);
}


void CW3Mask::leaveonlyone()
{
	GxConnectedComponentLabeling gxccl;
	gxccl.Create(m_nWidth, m_nHeight, m_nDepth, m_ppcData);
	gxccl.Get3DConnectedComponent();
	gxccl.SortVolumeSize();	
	gxccl.GetVolData(1, m_ppcData);
	gxccl.Release();
}
