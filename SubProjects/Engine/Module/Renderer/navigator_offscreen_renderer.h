#pragma once

/**=================================================================================================

Project:		Renderer
File:			navigator_offscreen_renderer.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-03-22
Last modify: 	2018-03-22

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/mat4x4.hpp>
#endif

#include "base_offscreen_renderer.h"

class CW3VBOSTL;
class CW3SurfaceLineItem;

class RENDERER_EXPORT NavigatorOffscreenRenderer : public BaseOffscreenRenderer {

public:
	NavigatorOffscreenRenderer();
	~NavigatorOffscreenRenderer();

public:
	void Render();

	void SetSize(int width, int height);

	void SetWorldAxisDirection(const glm::mat4& rot_mat, const glm::mat4& view_mat);

	inline const glm::mat4 rotate() { return rotate_; }

private:
	enum Axis {
		AXIS_X = 0,
		AXIS_Y,
		AXIS_Z,
		AXIS_END
	};
	void SetUniformMatrix(const uint& prog);
	void SetUniformLight(const uint& prog);
	void SetUniformColor(const uint& prog);
	void SetAxisColor(const uint & prog, const Axis& type);
private:

	uint shader_prog_ = 0;
	glm::mat4 rotate_;

	std::unique_ptr<CW3VBOSTL> stl_head_;

	std::unique_ptr<CW3SurfaceLineItem> major_axes_[AXIS_END];
	
	struct Size {
		int w = 100;
		int h = 100;

		Size() {}
		Size(int width, int height) : w(width), h(height) { };
	};
	Size size_;

	glm::mat4 view_;
	glm::mat4 projection_;
};
