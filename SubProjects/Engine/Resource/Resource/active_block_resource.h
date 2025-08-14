#pragma once

/**=================================================================================================

Project:		Resource
File:			active_block_resource.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-19
Last modify: 	2018-09-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "resource_global.h"

class CW3Image3D;

class RESOURCE_EXPORT ActiveBlockResource {
public:
	ActiveBlockResource(const CW3Image3D& vol, const int& block_size);

	~ActiveBlockResource(void);

	void setActiveBlock(int minValue, int maxValue);

	inline float* getVertexC() { return m_verticesCoord; }
	inline float* getTexC() { return m_texCoord; }
	inline unsigned int* getActiveIndex() { return m_ActiveIndex; }
	inline unsigned char* getStepMap() { return m_pStep; }

	inline int getNvertices() { return m_Nvertices; }

	inline unsigned int getNactiveBlock() { return m_nActiveBlock; }
	inline unsigned int GetNumActiveBlockFace() { return m_nActiveBlock*kNumActiveBlockFace; }

	inline int getWidth() { return m_width - 1; }
	inline int getHeight() { return m_height - 1; }
	inline int getDepth() { return m_depth - 1; }

private:
	void InitMinMax(const CW3Image3D& vol);

private:

	float *m_verticesCoord = nullptr;
	float *m_texCoord = nullptr;

	struct W3MinMax {
		unsigned short min = 0;
		unsigned short max = 0;
	};
	W3MinMax *m_minmax = nullptr;

	unsigned int *m_ActiveIndex = nullptr;
	unsigned char *m_pStep = nullptr;

	int m_width;
	int m_height;
	int m_depth;
	int m_blockSize;

	int nx_vol_;
	int ny_vol_;
	int nz_vol_;
	int m_Nvertices;
	unsigned int m_nActiveBlock = 0;

	const int kNumActiveBlockFace = 36;
};

