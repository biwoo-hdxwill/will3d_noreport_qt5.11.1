#pragma once
/*=========================================================================

File:			class CW3ActiveBlock
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-17
Last modify:	2015-12-17

=========================================================================*/
#include "vrengine_global.h"

//#define OCTREE_SIZE (1 + 8 + 8*8 + 8*8*8 + 8*8*8*8 + 8*8*8*8*8)
class CW3Image3D;

class VRENGINE_EXPORT CW3ActiveBlock {
public:
	CW3ActiveBlock(
		int nx, int ny, int nz, int blockSize,
		float stepX, float stepY, float stepZ,
		float stepXtex, float stepYtex, float stepZtex,
		float offsetXtex, float offsetYtex, float offsetZtex,
		int nxVol, int nyVol, int nzVol, CW3Image3D *vol);

	~CW3ActiveBlock(void);

	void setActiveBlock(int minValue, int maxValue);
	void SetVRCutActiveBlock(unsigned short** vr_cut_mask, int cur_vr_cut_history_step, int min_value, int max_value);
	void SetVRCutActiveBlockHistoryStep(int cur_vr_cut_history_step, int min_value, int max_value);

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

	//inline glm::vec2* getOctreeMaxMin() {return m_ocTreeMaxMin;}
	//inline int* getOctreeLevel() {return m_ocTreeLevel;}
private:
	void InitMinMax();
	//void createOctreeLv5();

private:
	float *m_verticesCoord = nullptr;
	float *m_texCoord = nullptr;
	CW3Image3D* volume_ = nullptr;
	unsigned short** vr_cut_mask_ = nullptr;
	int cur_vr_cut_history_step_ = 0;

	//glm::vec2	m_ocTreeMaxMin[ OCTREE_SIZE ];
	//int		m_ocTreeLevel[ OCTREE_SIZE ];

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

	int m_Nvertices;
	unsigned int m_nActiveBlock = 0;

	const int kNumActiveBlockFace = 36;
};

