#pragma once

/**=================================================================================================

Project:
File:
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-20
Last modify: 	2018-09-20

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/


#include "resource_global.h"
#include <vector>
#include <memory>
typedef struct TF_BGRA {
	float b = 0.0f;
	float g = 0.0f;
	float r = 0.0f;
	float a = 0.0f;

	void mulRGB(float x) {
		this->b *= x;
		this->g *= x;
		this->r *= x;
	}

	TF_BGRA operator+(TF_BGRA& rhs) {
		TF_BGRA tmp;
		tmp.b = this->b + rhs.b;
		tmp.g = this->g + rhs.g;
		tmp.r = this->r + rhs.r;
		tmp.a = this->a + rhs.a;
		return tmp;
	}

	TF_BGRA operator-(TF_BGRA& rhs) {
		TF_BGRA tmp;
		tmp.b = this->b - rhs.b;
		tmp.g = this->g - rhs.g;
		tmp.r = this->r - rhs.r;
		tmp.a = this->a - rhs.a;
		return tmp;
	}

	TF_BGRA operator*(float mul) {
		TF_BGRA tmp;
		tmp.b = this->b*mul;
		tmp.g = this->g*mul;
		tmp.r = this->r*mul;
		tmp.a = this->a*mul;
		return tmp;
	}

	TF_BGRA operator/(float div) {
		TF_BGRA tmp;
		tmp.b = this->b / div;
		tmp.g = this->g / div;
		tmp.r = this->r / div;
		tmp.a = this->a / div;
		return tmp;
	}
}TF_BGRA;

/*
	* class CW3TF
	 - Represent Transfer Function.
	 - Includes basic informations about TF.
*/
class RESOURCE_EXPORT CW3TF
{
public:
	explicit CW3TF(const std::weak_ptr<std::vector<TF_BGRA>>& tf_colors_, int offset);
	~CW3TF(void);

	CW3TF(const CW3TF&) = delete;
	const CW3TF& operator=(const CW3TF&) = delete;
public:
	inline unsigned int	size() const noexcept { return ref_tf_colors_.lock()->size(); }
	inline int	offset() const { return offset_; }
	inline float* getTF() const { return &(ref_tf_colors_.lock().get()->front().b); }

	inline int min_value() const noexcept { return min_value_; }
	inline int max_value() const noexcept { return max_value_; }

	void FindMinMax();

private:
	 std::weak_ptr<std::vector<TF_BGRA>> ref_tf_colors_;

	int		offset_;
	int		min_value_ = 0;
	int		max_value_ = 0;
};

