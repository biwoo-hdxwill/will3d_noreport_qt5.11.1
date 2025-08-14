#pragma once
/*=========================================================================
File:			w3_struct.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-06-07
Last modify:	2021-06-07

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "W3Enum.h"

namespace lightbox_resource
{
	typedef struct _LightboxParams
	{
		LightboxViewType view_type;
		int count_row;
		int count_col;
		float interval;
		float thickness;

		_LightboxParams() : count_row(0), interval(0.0f), thickness(0.0f),
			view_type(LightboxViewType::VIEW_TYPE_UNKNOWN) {}
		_LightboxParams(int cnt_row, int cnt_col, float itv, float thk, const LightboxViewType& vt) :
			count_row(cnt_row), count_col(cnt_col), interval(itv), thickness(thk), view_type(vt) {}
	} LightboxParams;

	struct MaximizeParams
	{
		int prev_row = 0;
		int prev_col = 0;
		int target_lightbox_id = -1;
	};

	typedef struct _PlaneParams
	{
		glm::vec3 up, right, back;
		float dist_from_vol_center;
		int width, height, available_depth;

		_PlaneParams()
			: up(0.f), right(0.f), back(0.f), dist_from_vol_center(0.f),
			width(0), height(0), available_depth(0)
		{

		}

		_PlaneParams(const glm::vec3& up, const glm::vec3& right, const glm::vec3& back,
			float dist, int w, int h, int depth)
			: up(up), right(right), back(back), dist_from_vol_center(dist),
			width(w), height(h), available_depth(depth)
		{

		}
	} PlaneParams;

	typedef struct _MPRParams
	{
		glm::vec3 first_cross_pt;
		glm::vec3 last_cross_pt;

		_MPRParams() :
			first_cross_pt(0.0f), last_cross_pt(0.0f) {}

		_MPRParams(const glm::vec3& first, const glm::vec3& last) :
			first_cross_pt(first), last_cross_pt(last) {}
	} MPRParams;
} // end of namespace lightbox_resource
