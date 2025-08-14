// smseo : 안쓰이는 코드라 막아둠
#if 0
#include "GxConnectedComponentLabeling.h"
#include "../../../Common/Common/W3Memory.h"

GxConnectedComponentLabeling::GxConnectedComponentLabeling(void)
{
	m_nLabelCnt = 0;
	m_stLabelTable.nTableCnt = 0;
	m_stLabelTable.pTable = NULL;

	m_nWidth = 0;
	m_nHeight = 0;
	m_nDepth = 0;
	m_volMap = NULL;
}

GxConnectedComponentLabeling::~GxConnectedComponentLabeling(void)
{
	if( m_stLabelTable.pTable ) {
		delete [] m_stLabelTable.pTable;
		m_stLabelTable.pTable = NULL;
	}
	m_stLabelTable.nTableCnt = 0;
}


int GxConnectedComponentLabeling::Get3DConnectedComponent()
{
	if (!m_volMap) return -1;

	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	m_stLabelTable.nTableCnt = 327670; //sizeof(int)*nVX*nVY*nVZ / 2;
	if( m_stLabelTable.pTable )	delete [] m_stLabelTable.pTable;
	m_stLabelTable.pTable = new int [m_stLabelTable.nTableCnt];
	for( int i=0; i<m_stLabelTable.nTableCnt; i++)
		m_stLabelTable.pTable[i]=i;

	m_nLabelCnt = 1;

	for( int z=0; z<nVZ; z++)
	{
		for( int y=0; y<nVY; y++)
		{
			for(int x=0; x<nVX; x++)
			{
				int idxxy = x+y*nVX;
				if( m_volMap[z][idxxy] == BKValue)	continue;
				SetLabel( x, y, z );
			}
		}
	}
	for( int i=0; i<m_stLabelTable.nTableCnt; i++)
	{
		m_stLabelTable.pTable[i] = FindFinalLabel(i);
	}
	int nMaxLabel = MapFillingWithSortedLabel();

	return nMaxLabel;
}


int GxConnectedComponentLabeling::Get3DConnectedComponent(int idx)
{
	if (!m_volMap) return -1;

	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	m_stLabelTable.nTableCnt = 327670; //sizeof(int)*nVX*nVY*nVZ / 2;
	if( m_stLabelTable.pTable )	delete [] m_stLabelTable.pTable;
	m_stLabelTable.pTable = new int [m_stLabelTable.nTableCnt];
	for( int i=0; i<m_stLabelTable.nTableCnt; i++)
		m_stLabelTable.pTable[i]=i;

	m_nLabelCnt = 1;

	for( int z=0; z<nVZ; z++)
	{
		for( int y=0; y<nVY; y++)
		{
			for(int x=0; x<nVX; x++)
			{
				int idxxy = x+y*nVX;
				if( m_volMap[z][idxxy] != idx)	continue;
				SetLabel( x, y, z );
			}
		}
	}
	for( int i=0; i<m_stLabelTable.nTableCnt; i++)
	{
		m_stLabelTable.pTable[i] = FindFinalLabel(i);
	}
	int nMaxLabel = MapFillingWithSortedLabel();

	return nMaxLabel;
}


void GxConnectedComponentLabeling::SetLabel( int x, int y, int z )
{
	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	int idxxy = x + y*nVX;
	int idxxy_x_1 = x-1 + y*nVX;
	int idxxy_y_1 = x + (y-1)*nVX;

	bool bCompX=false, bCompY=false, bCompZ=false;

	if( x!=0)
		if( m_volMap[z][idxxy_x_1]!=BKValue)
			bCompX = true;
	if( y!=0)
		if( m_volMap[z][idxxy_y_1]!=BKValue)
			bCompY = true;
	if( z!=0)
		if( m_volMap[z-1][idxxy]!=BKValue)
			bCompZ = true;

	if( bCompX && bCompY && bCompZ)
	{		
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMid = MidLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMidValue = FindFinalLabel(nMid);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_stLabelTable.pTable[nMidValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && bCompY && !bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && !bCompY && bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( !bCompX && bCompY && bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && !bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z][idxxy_x_1];
	}
	if( !bCompX && bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z][idxxy_y_1];
	}
	if( !bCompX && !bCompY && bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z-1][idxxy];
	}
	if( !bCompX && !bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_nLabelCnt++;
	//	ASSERT( m_nLabelCnt < m_stLabelTable.nTableCnt );
	}

	return;
}


void GxConnectedComponentLabeling::SetLabel(int idx, int x, int y, int z )
{
	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	int idxxy = x + y*nVX;
	int idxxy_x_1 = x-1 + y*nVX;
	int idxxy_y_1 = x + (y-1)*nVX;

	bool bCompX=false, bCompY=false, bCompZ=false;

	if( x!=0)
		if( m_volMap[z][idxxy_x_1] == idx)
			bCompX = true;
	if( y!=0)
		if( m_volMap[z][idxxy_y_1] == idx)
			bCompY = true;
	if( z!=0)
		if( m_volMap[z-1][idxxy] == idx)
			bCompZ = true;

	if( bCompX && bCompY && bCompZ)
	{		
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMid = MidLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMidValue = FindFinalLabel(nMid);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_stLabelTable.pTable[nMidValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && bCompY && !bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z][idxxy_y_1]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && !bCompY && bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_x_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_x_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( !bCompX && bCompY && bCompZ)
	{
		int nMin = MinLabel( m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);
		int nMax = MaxLabel( m_volMap[z][idxxy_y_1], m_volMap[z-1][idxxy]);

		int nMinValue = FindFinalLabel(nMin);
		int nMaxValue = FindFinalLabel(nMax);

		m_stLabelTable.pTable[nMaxValue] = nMinValue;
		m_volMap[z][idxxy] = nMinValue;
	}
	if( bCompX && !bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z][idxxy_x_1];
	}
	if( !bCompX && bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z][idxxy_y_1];
	}
	if( !bCompX && !bCompY && bCompZ)
	{
		m_volMap[z][idxxy] = m_volMap[z-1][idxxy];
	}
	if( !bCompX && !bCompY && !bCompZ)
	{
		m_volMap[z][idxxy] = m_nLabelCnt++;
	//	ASSERT( m_nLabelCnt < m_stLabelTable.nTableCnt );
	}

	return;
}


int GxConnectedComponentLabeling::FindFinalLabel( int nNowLabel)
{
	int nRetLable = nNowLabel;
	while (m_stLabelTable.pTable[nRetLable] != nRetLable)
	{
		nRetLable = m_stLabelTable.pTable[nRetLable];
	}
	return nRetLable;
}

int GxConnectedComponentLabeling::MapFillingWithSortedLabel()
{
	if( m_nLabelCnt==NULL )
		return -1;

	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	int * pSortTable = new int [m_nLabelCnt];
//	ZeroMemory( pSortTable , m_nLabelCnt*sizeof(int));
	memset(pSortTable, 0, sizeof(int)*m_nLabelCnt);

	int nSortedCnt = 0;
	for( int z=0, i=0; z<nVZ; z++)
	{
		for( int y=0; y<nVY; y++)
		{
			for(int x=0; x<nVX; x++, i++)
			{
				int idxxy = x + y*nVX;

				int nUnsortedLabel = m_stLabelTable.pTable[ m_volMap[z][idxxy] ];
				m_volMap[z][idxxy] = nUnsortedLabel;

				if( nUnsortedLabel!=0  &&  pSortTable[nUnsortedLabel]==0)
					pSortTable[nUnsortedLabel] = ++nSortedCnt;
			}
		}
	}

	for( int z=0; z<nVZ; z++)
	{
		for( int y=0; y<nVY; y++)
		{
			for(int x=0; x<nVX; x++)
			{
				int idxxy = x + y*nVX;
				m_volMap[z][idxxy] = pSortTable[m_volMap[z][idxxy]];
			}
		}
	}

	delete [] pSortTable;

	m_nLabelCnt = nSortedCnt;
	return nSortedCnt;
}

bool GxConnectedComponentLabeling::SortVolumeSize()
{
	if( m_nLabelCnt < 1 )	return false;

	int nVX = m_nWidth;
	int nVY = m_nHeight;
	int nVZ = m_nDepth;

	struct stLabel { int nLabel; int nCnt; };
	stLabel * pLabelCnt = new stLabel [m_nLabelCnt+1];
	for( int i=0; i<=m_nLabelCnt; i++ ) {
		pLabelCnt[i].nCnt = 0;
		pLabelCnt[i].nLabel = i;
	}

	for( int z=0; z<nVZ; z++)
	{
		for( int y=0; y<nVY; y++)
		{
			for(int x=0; x<nVX; x++)
			{
				int idxxy = x + y*nVX;
			//	ASSERT(m_volMap[z][idxxy] <= m_nLabelCnt);
				pLabelCnt[m_volMap[z][idxxy]].nCnt++;
			}
		}
	}
	pLabelCnt[0].nCnt = INT_MAX;

	{
		/* bubble sort */

		int indx;
		int indx2;
		stLabel temp;
		stLabel temp2;
		int flipped;

		indx = 1; 
		do
		{
			flipped = 0;
			for (indx2 = m_nLabelCnt; indx2 >= indx; --indx2)
			{
				temp = pLabelCnt[indx2];
				temp2 = pLabelCnt[indx2 - 1];

				if ( temp2.nCnt < temp.nCnt )
				{
					pLabelCnt[indx2 - 1] = temp;
					pLabelCnt[indx2] = temp2;
					flipped = 1;
				}
			}
		} while ((++indx <= m_nLabelCnt) && flipped);
	}

	int * pLabelNew = new int [m_nLabelCnt+1];
//	ZeroMemory( pLabelNew, sizeof(int)*(m_nLabelCnt+1));
	memset(pLabelNew, 0, sizeof(int)*(m_nLabelCnt+1));
	for( int i=1; i<=m_nLabelCnt; i++ ) {
		pLabelNew[ pLabelCnt[i].nLabel ] = i;
	}

	for( int z=0; z<nVZ; z++) {
		for( int y=0; y<nVY; y++) {
			for(int x=0; x<nVX; x++) {
				int idxxy = x + y*nVX;
				m_volMap[z][idxxy] = pLabelNew[m_volMap[z][idxxy]];
			}
		}
	}

	delete [] pLabelNew;
	delete [] pLabelCnt;

	return true;
}


bool GxConnectedComponentLabeling::Create(int w, int h, int d, unsigned char** ppBinData)
{
	m_volMap = SAFE_ALLOC_VOLUME(GxVolumetype, d, w*h);
	if (!m_volMap) return false;

	m_nWidth = w;
	m_nHeight = h;
	m_nDepth = d;

	int wh = w*h;
	for (int z = 0 ; z < d ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			m_volMap[z][xy] = (GxVolumetype)ppBinData[z][xy];
		}
	}
	return true;
}

int GxConnectedComponentLabeling::GetVolData(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;
	int volumesize = 0;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] != 0)	
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else
			{
				ppOutBinData[z][xy] = 0;
			}
		}
	}

	return volumesize;
}


int GxConnectedComponentLabeling::GetVolData(int sizerank, unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;
	int volumesize = 0;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == sizerank)	
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else
			{
				ppOutBinData[z][xy] = 0;
			}
		}
	}
	return volumesize;
}


int GxConnectedComponentLabeling::GetVolData(int sizerank, unsigned char** ppOutBinData, SCCLPnt3D* pCenterPnt)
{
	int wh = m_nWidth*m_nHeight;
	int volumesize = 0;

	SCCLPnt3D center3d;	center3d.x = 0;	center3d.y = 0;	center3d.z = 0;
	
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int y = 0 ; y < m_nHeight ; y++)
		{
			for (int x = 0 ; x < m_nWidth ; x++)
			{
				int idxxy = x + y*m_nWidth;
				if (m_volMap[z][idxxy] == sizerank)	
				{
					ppOutBinData[z][idxxy] = 255;
					volumesize++;

					center3d.x += x;
					center3d.y += y;
					center3d.z += z;
				}
				else
				{
					ppOutBinData[z][idxxy] = 0;
				}
			}
		}
	}
	if (volumesize == 0) return 0;

	pCenterPnt->x = center3d.x / volumesize;
	pCenterPnt->y = center3d.y / volumesize;
	pCenterPnt->z = center3d.z / volumesize;

	return volumesize;
}


int	GxConnectedComponentLabeling::GetVolData(int* pSizeRank, int nCnt, unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;
	int volumesize = 0;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			bool bCheck = false;
			for (int l = 0 ; l < nCnt ; l++)
			{
				if (m_volMap[z][xy] == pSizeRank[l])
				{
					bCheck = true;
					break;
				}
			}
			if (bCheck)
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else
			{
				ppOutBinData[z][xy] = 0;
			}
		}
	}

	return volumesize;
}


int GxConnectedComponentLabeling::GetMaxVolData(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;
	int volumesize = 0;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == 1)	
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else
			{
				ppOutBinData[z][xy] = 0;
			}
		}
	}

	return volumesize;
}


int GxConnectedComponentLabeling::GetUpVolData(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;	
	int minz1 = INT_MAX;
	int minz2 = INT_MAX;
	int volumesize = 0;

	// 1st scan : find the center of volumes
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == 1)	
			{
				if (minz1 > z) minz1 = z;				
			}
			else if (m_volMap[z][xy] == 2)	
			{
				if (minz2 > z) minz2 = z;				
			}
		}
	}

	// 2nd scan : save upper result
	int uplevel = (minz1<minz2)? 1:2;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == uplevel)	
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else			
			{
				ppOutBinData[z][xy] = 0;			
			}
		}
	}

	return volumesize;
}


int GxConnectedComponentLabeling::GetDnVolData(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;	
	int minz1 = INT_MAX;
	int minz2 = INT_MAX;
	int volumesize = 0;

	// 1st scan : find the center of volumes
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == 1)	
			{
				if (minz1 > z) minz1 = z;				
			}
			else if (m_volMap[z][xy] == 2)	
			{
				if (minz2 > z) minz2 = z;				
			}
		}
	}

	// 2nd scan : save downer volume
	int dnlevel = (minz1>minz2)? 1:2;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == dnlevel)
			{
				ppOutBinData[z][xy] = 255;
				volumesize++;
			}
			else
			{
				ppOutBinData[z][xy] = 0;			
			}
		}
	}

	return volumesize;
}


void GxConnectedComponentLabeling::GetUpVolDataWithMeanZ(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;	
	int zsum1 = 0;
	int zsum2 = 0;
	int zcnt1 = 0;
	int zcnt2 = 0;

	// 1st scan : find the center of volumes	
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == 1)	
			{
				zsum1 += z;
				zcnt1++;
			}
			else if (m_volMap[z][xy] == 2)	
			{
				zsum2 += z;
				zcnt2++;
			}
		}
	}
	zsum1 = (int)(zsum1/(zcnt1+0.1f));
	zsum2 = (int)(zsum2/(zcnt2+0.1f));

	// 2nd scan : save downer volume
	int uplevel = (zsum1<zsum2)? 1:2;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == uplevel)		ppOutBinData[z][xy] = 255;
			else								ppOutBinData[z][xy] = 0;			
		}
	}
}


void GxConnectedComponentLabeling::GetDnVolDataWithMeanZ(unsigned char** ppOutBinData)
{
	int wh = m_nWidth*m_nHeight;	
	int zsum1 = 0;
	int zsum2 = 0;
	int zcnt1 = 0;
	int zcnt2 = 0;

	// 1st scan : find the center of volumes	
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == 1)	
			{
				zsum1 += z;
				zcnt1++;
			}
			else if (m_volMap[z][xy] == 2)	
			{
				zsum2 += z;
				zcnt2++;
			}
		}
	}
	zsum1 = (int)(zsum1/(zcnt1+0.1f));
	zsum2 = (int)(zsum2/(zcnt2+0.1f));

	// 2nd scan : save downer volume
	int dnlevel = (zsum1>zsum2)? 1:2;
	for (int z = 0 ; z < m_nDepth ; z++)
	{
		for (int xy = 0 ; xy < wh ; xy++)
		{
			if (m_volMap[z][xy] == dnlevel)		ppOutBinData[z][xy] = 255;
			else								ppOutBinData[z][xy] = 0;			
		}
	}
}

void GxConnectedComponentLabeling::Release()
{
	SAFE_DELETE_VOLUME(m_volMap, m_nDepth);

	m_nWidth = 0;
	m_nHeight = 0;
	m_nDepth = 0;

	m_nLabelCnt = 0;
	m_stLabelTable.nTableCnt = 0;

	SAFE_DELETE_ARRAY(m_stLabelTable.pTable);
}

#endif
