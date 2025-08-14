#pragma once
/*=========================================================================

File:			class CW3Render3DParam
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-21
Last date:		2016-04-21

=========================================================================*/
#include "vrengine_global.h"

class VRENGINE_EXPORT CW3VRengineTextures {
public:
	CW3VRengineTextures();
	~CW3VRengineTextures();

	inline const float windowing_min() const noexcept { return windowing_min_; }
	inline const float windowing_norm() const noexcept { return windowing_norm_; }
	inline void set_windowing_min(float min) noexcept { windowing_min_ = min; }
	inline void set_windowing_norm(float norm) noexcept { windowing_norm_ = norm; }

public:
	unsigned int m_texHandler[3] = { 0,0,0 };

private:
	float windowing_min_ = 0.0f;
	float windowing_norm_ = 0.0f;
};
