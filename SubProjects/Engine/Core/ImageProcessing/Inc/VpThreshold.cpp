#include "VpThreshold.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVpThreshold::CVpThreshold()
	: m_nWidth(0), m_nHeight(0), m_nDepth(0), 
	m_pData(NULL), m_ppData(NULL), m_pusData(NULL), m_ppusData(NULL)
{
}

CVpThreshold::~CVpThreshold()
{
}

int CVpThreshold::GetOtsuThre(int w, int h, int d, voltype **src_image, unsigned char ** pMask)
{
	// 1. calc min and max
	voltype volmin = SHRT_MAX;
	voltype volmax = SHRT_MIN;
	int i, j;
	int wh = w*h;

	for (j = 0; j < d; j++)
	{
		for (i = 0; i < wh; i++)
		{
			if (src_image[j][i] < volmin) volmin = src_image[j][i];
			if (src_image[j][i] > volmax) volmax = src_image[j][i];
		}
	}

	int L = (volmax - volmin) + 1;
	double *hist = new double[L];
	double *chist = new double[L];
	double *cxhist = new double[L];
	memset(hist, 0, sizeof(double)*L);
	memset(chist, 0, sizeof(double)*L);
	memset(cxhist, 0, sizeof(double)*L);
	int nSize = 0;

	// make histogram
	if (pMask)
	{
		for (j = 0; j < d; j++)
		{
			for (i = 0; i < wh; i++)
			{
				if (pMask[j][i] != 0)
				{
					++hist[src_image[j][i] - volmin];
					++nSize;
				}
			}
		}
	}
	else
	{
		for (j = 0; j < d; j++)
		{
			for (i = 0; i < wh; i++)
			{
				++hist[src_image[j][i] - volmin];
				++nSize;
			}
		}
	}

	// normalize
	for (i = 0; i < L; i++)
	{
		hist[i] /= nSize;
	}

	// make 0- and 1-st cumulative histogram
	chist[0] = hist[0];
	cxhist[0] = 0;
	for (i = 1; i < L; i++)
	{
		chist[i] = chist[i - 1] + hist[i];		// 0-th
		cxhist[i] = cxhist[i - 1] + (i*hist[i]);	// 1-th
	}

	double cost_max = .0;
	int thresh = 0;
	double m = cxhist[L - 1];		// total mean

	for (i = 0; i < L; i++)
	{
		if (chist[i] == 0) continue;

		double q1 = chist[i];
		double q2 = 1 - q1;

		if (q2 == 0.) break;

		double m1 = cxhist[i] / q1;
		double m2 = (m - cxhist[i]) / q2;
		double cost = q1*q2*(m1 - m2)*(m1 - m2);
		if (cost_max < cost)
		{
			cost_max = cost;
			thresh = i;
		}
	}

	delete[] hist;
	delete[] chist;
	delete[] cxhist;

	return (thresh + volmin);
}

int CVpThreshold::GetOtsuThre(int w, int h, int d, usvoltype **src_image, unsigned char ** pMask)
{
	// 1. calc min and max
	usvoltype volmin = USHRT_MAX;
	usvoltype volmax = 0;
	int i, j;
	int wh = w*h;

	for (j = 0; j < d; j++)
	{
		for (i = 0; i < wh; i++)
		{
			if (src_image[j][i] < volmin) volmin = src_image[j][i];
			if (src_image[j][i] > volmax) volmax = src_image[j][i];
		}
	}

	int L = (volmax - volmin) + 1;
	double *hist = new double[L];
	double *chist = new double[L];
	double *cxhist = new double[L];
	memset(hist, 0, sizeof(double)*L);
	memset(chist, 0, sizeof(double)*L);
	memset(cxhist, 0, sizeof(double)*L);
	int nSize = 0;

	// make histogram
	if (pMask)
	{
		for (j = 0; j < d; j++)
		{
			for (i = 0; i < wh; i++)
			{
				if (pMask[j][i] != 0)
				{
					++hist[src_image[j][i] - volmin];
					++nSize;
				}
			}
		}
	}
	else
	{
		for (j = 0; j < d; j++)
		{
			for (i = 0; i < wh; i++)
			{
				++hist[src_image[j][i] - volmin];
				++nSize;
			}
		}
	}

	// normalize
	for (i = 0; i < L; i++)
	{
		hist[i] /= nSize;
	}

	// make 0- and 1-st cumulative histogram
	chist[0] = hist[0];
	cxhist[0] = 0;
	for (i = 1; i < L; i++)
	{
		chist[i] = chist[i - 1] + hist[i];		// 0-th
		cxhist[i] = cxhist[i - 1] + (i*hist[i]);	// 1-th
	}

	double cost_max = .0;
	int thresh = 0;
	double m = cxhist[L - 1];		// total mean

	for (i = 0; i < L; i++)
	{
		if (chist[i] == 0) continue;

		double q1 = chist[i];
		double q2 = 1 - q1;

		if (q2 == 0.) break;

		double m1 = cxhist[i] / q1;
		double m2 = (m - cxhist[i]) / q2;
		double cost = q1*q2*(m1 - m2)*(m1 - m2);
		if (cost_max < cost)
		{
			cost_max = cost;
			thresh = i;
		}
	}

	delete[] hist;
	delete[] chist;
	delete[] cxhist;

	return (thresh + volmin);
}

int CVpThreshold::Otsu(unsigned char** ppMask)
{
	return GetOtsuThre(m_nWidth, m_nHeight, m_nDepth, m_ppusData, ppMask);
}

int CVpThreshold::OtsuEx(unsigned char** ppOutThresh, unsigned char** ppVOIMask)
{
	int nThre = GetOtsuThre(m_nWidth, m_nHeight, m_nDepth, m_ppusData, ppVOIMask);
	for (int z = 0; z < m_nDepth; z++)
	{
		for (int xy = 0; xy < m_nWidth*m_nHeight; xy++)
		{
			if (ppVOIMask && !ppVOIMask[z][xy])
				continue;

			if (m_ppusData[z][xy] >= nThre)
				ppOutThresh[z][xy] = 255;
		}
	}

	return nThre;
}

bool CVpThreshold::Do(unsigned char** ppBin, voltype threshmin, voltype threshmax, eThreshType threshtype, unsigned char** ppMask)
{
	if (m_ppData == NULL) return false;

	int d = m_nDepth;
	int wh = m_nWidth*m_nHeight;

	// 16.06.24 - initialize
	for (int z = 0; z < d; z++)
	{
		memset(ppBin[z], 0, sizeof(unsigned char)*wh);
	}

	switch (threshtype)
	{
	case LESSTHAN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if ((m_ppData[z][xy] <= threshmin) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (m_ppData[z][xy] <= threshmin)
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	case BETWEEN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (CompareEq<voltype>(m_ppData[z][xy], threshmin, threshmax) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (CompareEq<voltype>(m_ppData[z][xy], threshmin, threshmax))
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	case GREATERTHAN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if ((m_ppData[z][xy] >= threshmax) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (m_ppData[z][xy] >= threshmax)
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	default:
		break;
	}

	return true;
}

bool CVpThreshold::Do(unsigned char** ppBin, usvoltype threshmin, usvoltype threshmax, eThreshType threshtype, unsigned char** ppMask)
{
	if (m_ppusData == NULL) return	false;

	int d = m_nDepth;
	int wh = m_nWidth*m_nHeight;

	// 16.06.24 - initialize
	for (int z = 0; z < d; z++)
	{
		memset(ppBin[z], 0, sizeof(unsigned char)*wh);
	}

	switch (threshtype)
	{
	case LESSTHAN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if ((m_ppusData[z][xy] <= threshmin) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (m_ppusData[z][xy] <= threshmin)
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	case BETWEEN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (CompareEq<usvoltype>(m_ppusData[z][xy], threshmin, threshmax) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (CompareEq<usvoltype>(m_ppusData[z][xy], threshmin, threshmax))
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	case GREATERTHAN:
		if (ppMask)
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if ((m_ppusData[z][xy] >= threshmax) && (ppMask[z][xy] != 0))
						ppBin[z][xy] = 255;
				}
			}
		}
		else
		{
			for (int z = 0; z < d; z++)
			{
				for (int xy = 0; xy < wh; xy++)
				{
					if (m_ppusData[z][xy] >= threshmax)
						ppBin[z][xy] = 255;
				}
			}
		}
		break;

	default:
		break;
	}

	return true;
}
