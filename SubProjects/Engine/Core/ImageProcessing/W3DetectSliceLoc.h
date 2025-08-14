#pragma once
/*=========================================================================

File:			class CW3DetectSliceLoc
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2017-03-29
Last modify:	2017-05-04

=========================================================================*/
#include <vector>

#include "../../Common/Common/W3Types.h"
#include "imageprocessing_global.h"

class CW3Image3D;

namespace cv {
class Mat;
}
class IMAGEPROCESSING_EXPORT CW3DetectSliceLoc {
public:
	static void run(const CW3Image3D* vol, int thdTissue, int thdBone, SliceLoc* sliceLoc);
	static int FindChinSliceLocation(CW3Image3D* volume);

private:
	static void FindSliceLocation(const CW3Image3D* vol, int tissue_threshold, int bone_threshold, SliceLoc* slice_location, int method);

	static int findChinSlice(std::vector<std::vector<uchar>>& volBoneMask, int nWidth, float pixelSpacing, float slice_spacing, int nScale);
	static int findNoseSlice(cv::Mat& projTissueMask);
};
