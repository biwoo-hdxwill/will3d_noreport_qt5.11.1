#pragma once

#include "VpGlobalDefine.h"
#include "../imageprocessing_global.h"

class IMAGEPROCESSING_EXPORT CVpProcBase {
public:
	CVpProcBase();
	virtual ~CVpProcBase();

	virtual	void Init2D(int w, int h, unsigned char* pData);
	virtual	void Init3D(int w, int h, int d, unsigned char** ppData);

	virtual void Init2D(int w, int h, voltype* pData);
	virtual	void Init3D(int w, int h, int d, voltype** ppData);

	inline bool IsEmpty() {
		if ((m_pbytData == NULL) && (m_ppbytData == NULL) && (m_pvolData == NULL) && (m_ppvolData == NULL))
			return true;

		return false;
	}

protected:
	int m_nWidth;
	int m_nHeight;
	int m_nDepth;
	int m_nWH;
	union {
		unsigned char*	m_pbytData;
		unsigned char**	m_ppbytData;
		voltype*		m_pvolData;
		voltype**		m_ppvolData;
	};
};
