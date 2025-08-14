#pragma once
/**=================================================================================================

Project: 			Resource
File:				pano_resource.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-04-17
Last modify:		2018-04-17

Copyright (c) 2017 ~ 2018 All rights reserved by HDXWILL.

*===============================================================================================**/
#include <qpoint.h>
#include "W3Resource.h"
#include "include/curve_data.h"
#include "resource_global.h"

class RESOURCE_EXPORT SagittalResource : public CW3Resource {
public:
	SagittalResource();
	virtual ~SagittalResource();

	SagittalResource(const SagittalResource&) = delete;
	SagittalResource& operator=(const SagittalResource&) = delete;

	struct Params {
		int width;
		int height;
		float degree;

		Params() : width(0), height(0), degree(0.0f) {}

		Params(int w, int h, float deg) :
			width(w), height(h), degree(deg) {}
	};

public:
	void SetSagittalParams(const Params& params);
	void SetSagittal(const glm::vec3& vol_position,
					 const CurveData& pano_curve,
					 const glm::vec3& pano_back_vector,
					 const QPointF& pano_position);

	inline const Params& params() const { return params_; }
	inline const glm::vec3& back_vector() const noexcept { return back_vector_; }
	inline const glm::vec3& right_vector() const noexcept { return right_vector_; }
	inline const glm::vec3& up_vector() const noexcept { return up_vector_; }
	inline const glm::vec3& center_position() const noexcept { return center_position_; }
	inline const bool& is_valid() const { return is_valid_; }


private:
	Params params_;
	glm::vec3 center_position_ = glm::vec3(0.0f);
	glm::vec3 right_vector_ = glm::vec3(0.0f);
	glm::vec3 up_vector_ = glm::vec3(0.0f);
	glm::vec3 back_vector_ = glm::vec3(0.0f);
	bool is_valid_ = false;
};

