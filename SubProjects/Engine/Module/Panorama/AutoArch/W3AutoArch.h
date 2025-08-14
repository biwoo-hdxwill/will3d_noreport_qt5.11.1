#pragma once

/*=========================================================================

File:			class CW3AutoArch
Language:		C++11
Library:		-
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last date:		2016-04-14

=========================================================================*/
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <QPointF>

class CW3Image3D;
namespace cv
{
	class Mat;
}

class CW3AutoArch
{
public:
	CW3AutoArch();
	~CW3AutoArch();

public:
	static void runMaxillaArch(const CW3Image3D * vol, std::vector<glm::vec3>& points, const int target_slice_number = -1);
	static void runMandibleArch(const CW3Image3D* vol, std::vector<glm::vec3>& points, const int target_slice_number = -1);
	//static void runFreeFOVArch(const CW3Image3D* vol, std::vector<glm::vec3>& points);
	//static float getTeethSliceNum(const float* depthMapTeeth, const int width, const int height, const float teethInterval);

private:
	typedef struct _pairPoint
	{
		QPointF p1;
		QPointF p2;

		_pairPoint() {}
		_pairPoint(const QPointF& arg1, const QPointF& arg2) :
			p1(arg1), p2(arg2)
		{
		}

		void operator=(const _pairPoint& pp)
		{
			p1 = pp.p1;
			p2 = pp.p2;
		}
	}PairPoint;
	static void findMandible(const CW3Image3D* vol, int* mandible_slice_idx, cv::Mat* mandible_mask, const bool use_fixed_target_slice = false);
	static void setLinePoints(const ushort * line_mask, const int width, const int height, const float pixelSpacing, std::vector<QPointF>& points, float margin_mm, int sample_iter = 2);
	static void setArchPoints(const ushort* archMask, const int width, const int height, const float sliceSpacing, std::vector<QPointF>& points, float margin_mm, int sample_iter = 4);
	static bool isTwistedInsertPoint(const std::vector<QPointF>& points, int insert_idx, QPointF insert_point);
	static void correctPoints(ushort** volData, const int startZ, const int endZ, const int width, const int height, const int boneThreshold, const float sliceSpacing, std::vector<QPointF>& points);
	static void samplingPoints(const float divideLen, std::vector<QPointF>& points);
};
