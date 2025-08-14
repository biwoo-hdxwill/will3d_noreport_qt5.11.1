#pragma once

#include <vector>
#include "../../../Engine/Common/GLfunctions/W3GLTypes.h"

#include "../endoscopy_global.h"

typedef unsigned short AreaType;

#define CHECK_FOR_SEGMENTABLE(X)	(!((X)&LABEL_SEGMENTING || (X)&LABEL_PROTECTED))

// Segmemtation definition
#define LABEL_PROTECTED				0x80
#define LABEL_SEGMENTING			0x40
#define LABEL_NONE					0x00

/*
	example code :
	CAirwaySeg airwaySeg;
	bool bSuccess = airwaySeg.doRun(ppVolData3D,ppMaskData3D,nW,nH,nD,nDataMin,fPS,vecCP,vecPath);

	std::vector<tri_STL> meshSTL;
	if(bSuccess)
	{
		meshSTL = airwaySeg.getMeshSTL();
		emit sigRendering(meshSTL);
	}
*/

class ENDOSCOPY_EXPORT CAirwaySeg
{
public:
	CAirwaySeg(void);
	~CAirwaySeg(void);

	/*
	input parameter 
		unsigned short** ppVolData3D	: original volume 
		unsigned char** ppMaskData3D	: original mask 3D buffer
		int nW							: width of volume 
		int nH							: height of volume
		int nD							: depth of volume
		int nDataMin					: intensity minimum value of volume
		float fPixelSpacing				: pixel spacing of volume
		float fIntercet					: intercept of data
		std::vector<CVector_3D> vecCP	: location information of control points 
		std::vector<CVector_3D> vecPath	: location information of path voxel
		int nAirwayMax					: Prior information of airway maximum HU value (water = 0)
		bool bClosing					: true --> add morphological closing operation 
	return 
		bool							: true --> segmentation success, false --> try to re-draw curve
	*/
	bool doRun(unsigned short** ppVolData3D, unsigned char** ppMaskData3D, int nW, int nH, int nD, int nDataMin, float fPixelSpacing, float fIntercept,
		std::vector<glm::vec3> vecCP, std::vector<glm::vec3> vecPath, int nDownFactor = 1, int nAirwayMax = -800, bool bClosing = false);

	/*
	return 
		std::vector<tri_STL>			: airway STL information
		
		typedef struct _STL_TRI_SE{
		f3 normal, v1, v2, v3;
		unsigned short	cntAttributes;
		unsigned int	nColorVal;
		f3				fColor;
		}tri_STL;
	*/
	inline std::vector<tri_STL> getMeshSTL() {return m_meshSTL;}

private:
	/*
	input parameter 
		unsigned char* input			: input segmented image 
		unsigned int* output			: arbitary result
		int nW							: width of image
		int nH							: height of image
	return 
		int								: the number of connected components 
	*/
	int run2DCCL(unsigned char* input, unsigned int* output, int nW, int nH);
	void LabelComponent(unsigned short* STACK, unsigned char* input, unsigned int* output,int nW, int nH, int labelNo, unsigned short x, unsigned short y);
	/*
	input parameter 
		unsigned int* outputImage		: input image 
		AreaType* AreaImage				: output area image
		int nLabel						: CCL return, the number of components
		int nW							: width of image
		int nH							: height of image
		float fPS_SQ					: squrare of pixel spacing 
	return 
		void
	*/
	void CountArea(unsigned int* outputImage, AreaType* AreaImage,int nLabel, int nW, int nH, float fPS_SQ);
	/*
	input parameter 
		unsigned char** ppInput			: input volume 
		int nW							: width of volume
		int nH							: height of volume
		int nD							: depth of volume
		float fPixelSpacing				: pixel spacing of volume
	return 
		void
	*/
	void getGaussianVolume(unsigned char** ppInput, int nW, int nH, int nD, float fPixelSpacing);

private:
	std::vector<tri_STL> m_meshSTL;
};
