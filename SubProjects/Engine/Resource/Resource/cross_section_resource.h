#pragma once
/**=================================================================================================

Project: 			Resource
File:				pano_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-23
Last modify:		2017-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <memory>
#include <map>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "include/cross_section_data.h"
#include "W3Resource.h"
#include "resource_global.h"

class RESOURCE_EXPORT CrossSectionResource : public CW3Resource {
public:
	struct Params 
	{
		int count;
		int width;
		int height;
		float interval;
		float thickness;
		float degree;

		Params() : count(0), width(0), height(0), interval(0.0f), thickness(0.0f), degree(0.0f) {}
		Params(int cnt, int w, int h, float itv, float thk, float deg) :
			count(cnt), width(w), height(h), interval(itv), thickness(thk), degree(deg) {}
	};

	/** The pano curve. 반드시 등간격 1 voxel로 된 spline을 넘겨주어야 한다.*/
	CrossSectionResource(const CurveData& pano_curve,
						 const glm::vec3& pano_back_vector,
						 const int& pano_front_index,
						 const int& pano_height,
						 const float& arch_position_in_pano);

	CrossSectionResource(const CrossSectionResource&) = delete;
	CrossSectionResource& operator=(const CrossSectionResource&) = delete;

public:
	void SetPanoCurveData(const CurveData& pano_curve, const int& pano_front_index);
	void SetCrossSectionParams(const Params& params);
	void EditShiftedValue(float shifted_value);
	void ResetDeltaShiftedValue();
	void SetCenterPositionsInPanoPlane(const std::map<int, QPointF>& center_positions);
	void SetArchPositionsInPanoPlane(const std::map<int, QPointF>& arch_positions);

	inline const Params& params() const { return params_; }
	inline const CurveData& pano_curve() const { return pano_curve_; }
	inline const std::map<int, std::unique_ptr<CrossSectionData>>& data() const { return data_; }
	inline const glm::vec3& pano_back_vector() const { return pano_back_vector_; }
	inline const float& shifted_value() const { return shifted_value_; }

private:
	void UpdateCurveData();
	void GetAvailableShiftedValue(float* min, float* max) const;
	int GetBeginCrossIdxAtPanoCurve() const;
	int GetEndCrossIdxAtPanoCurve() const;
	int CalculateShiftedValueByInterval();

private:
	Params params_;

	std::map<int, std::unique_ptr<CrossSectionData>> data_;
	
	CurveData pano_curve_;
	glm::vec3 pano_back_vector_ = glm::vec3(0.0f);
	int pano_height_ = 0;
	float arch_position_in_pano_ = 0.0f;
	int pano_front_index_ = 0;

	float shifted_value_ = 0.0f;
};
