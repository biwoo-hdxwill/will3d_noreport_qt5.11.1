#pragma once

#include "VpGlobalDefine.h"
#include "../imageprocessing_global.h"

class IMAGEPROCESSING_EXPORT CVpThreshold
{
public:
	enum eThreshType
	{
		LESSTHAN,
		BETWEEN,
		GREATERTHAN,
		AUTO,
		AUTOPERSLICE,
		AUTOGREATERTHAN,
		ITERATIVEAUTO,
		ITERATIVEAUTOSLICE,
	};

private:
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;
	voltype* m_pData;
	voltype** m_ppData;
	usvoltype* m_pusData;
	usvoltype** m_ppusData;

	int GetOtsuThre(int w, int h, int d, voltype **src_image, unsigned char ** pMask = NULL);
	int GetOtsuThre(int w, int h, int d, usvoltype **src_image, unsigned char ** pMask = NULL);

public:
	CVpThreshold();
	virtual ~CVpThreshold();

	inline	void Init(int w, int h, voltype* pData) { m_nWidth = w; m_nHeight = h; m_pData = pData; }
	inline	void Init(int w, int h, usvoltype* pData)	{ m_nWidth = w; m_nHeight = h; m_pusData = pData; }
	inline	void Init(int w, int h, int d, voltype** ppData) { m_nWidth = w; m_nHeight = h; m_nDepth = d; m_ppData = ppData; }
	inline	void Init(int w, int h, int d, usvoltype** ppData) { m_nWidth = w; m_nHeight = h; m_nDepth = d; m_ppusData = ppData; }

	int Otsu(unsigned char** ppMask = NULL);
	int OtsuEx(unsigned char** ppOutThresh, unsigned char** ppVOIMask = NULL);

	bool Do(unsigned char** ppBin, voltype threshmin, voltype threshmax, eThreshType threshtype = GREATERTHAN, unsigned char** ppMask = NULL);
	bool Do(unsigned char** ppBin, usvoltype threshmin, usvoltype threshmax, eThreshType threshtype = GREATERTHAN, unsigned char** ppMask = NULL);
};
