#include "curve_data.h"

#if defined(__APPLE__)
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtx/transform2.hpp>
#endif

CurveData::CurveData(const CurveData & curve_data) {
	*this = curve_data;
}

CurveData& CurveData::operator=(const CurveData & curve_data) {
	
	const auto& points = curve_data.points();
	const auto& up_vectors = curve_data.up_vectors();

	points_.assign(points.begin(), points.end());
	up_vectors_.assign(up_vectors.begin(), up_vectors.end());
	return *this;
}

glm::vec3 CurveData::GetInterpolatedPoint(float index) const { //interpolated return
	int iidx = (int)(index);

	if (iidx < 0 || iidx > points_.size() - 2)
		return glm::vec3();

	float offset = index - iidx;
	return points_[iidx] + (points_[iidx + 1] - points_[iidx])*offset;
}

glm::vec3 CurveData::GetInterpolatedUpvector(float index) const {
	int iidx = (int)(index);

	if (iidx < 0 || iidx > up_vectors_.size() - 2)
		return glm::vec3();

	float offset = index - iidx;
	return glm::normalize(up_vectors_[iidx] + (up_vectors_[iidx + 1] - up_vectors_[iidx])*offset);
}
