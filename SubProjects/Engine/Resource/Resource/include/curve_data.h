#pragma once
/**=================================================================================================

Project:		Resource
File:			curve_data.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-10-31
Last modify: 	2017-10-31

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "../resource_global.h"

class RESOURCE_EXPORT CurveData {
public:
	CurveData() { };
	~CurveData() { };
	CurveData(const CurveData&);
	CurveData& operator=(const CurveData&);

	inline void AddPoint(const glm::vec3& point) { points_.push_back(point); }
	inline void AddUpVector(const glm::vec3& upVector) { up_vectors_.push_back(upVector); }

	glm::vec3 GetInterpolatedPoint(float index) const;
	glm::vec3 GetInterpolatedUpvector(float index) const;
	int GetCurveLength() const { return (int)points_.size(); }
	const std::vector<glm::vec3>& points() const { return points_; }
	const std::vector<glm::vec3>& up_vectors() const { return up_vectors_; }
private:
	std::vector<glm::vec3> points_;
	std::vector<glm::vec3> up_vectors_;
};
