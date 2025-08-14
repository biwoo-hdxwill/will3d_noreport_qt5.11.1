#pragma once

/**=================================================================================================

Project: 			Renderer
File:				image_param.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-25
Last modify:		2017-08-25

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/mat4x4.hpp>
#endif

#include "defines.h"
class CW3Image3D;

class ImageParam {
public:
	ImageParam() = default;
	ImageParam(const ImageParam&) = delete;
	ImageParam& operator=(const ImageParam) = delete;

public:
	void SetVolume(const CW3Image3D& vol);

	inline void set_invert_window(const bool& is_invert) { invert_window_ = is_invert; }
	inline void set_wwl(const Renderer::WindowWL& wwl) { wwl_ = wwl; }

	inline const bool& invert_window() const { return invert_window_; }
	inline float base_pixel_size_mm() const { return base_pixel_size_mm_; }
	inline float intensity_min() const { return intensity_min_; }
	inline float intensity_max() const { return intensity_max_; }
	inline const Renderer::WindowWL& wwl() const { return wwl_; }
	inline const float& intercept() const { return intercept_; }
	bool IsInitialize() const;
private:

	float intensity_max_ = 0.0f;
	float intensity_min_ = 0.0f;

	bool invert_window_ = false;

	bool is_set_volume_ = false;

	float base_pixel_size_mm_ = 0.0f;
	float intercept_ = 0.0f;

	Renderer::WindowWL wwl_;
};
