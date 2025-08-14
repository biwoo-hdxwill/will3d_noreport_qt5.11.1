#pragma once

/**=================================================================================================

Project:		Renderer
File:			implant_slice_renderer.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-05-04
Last modify: 	2018-05-04

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/mat4x4.hpp>
#endif

#include "base_offscreen_renderer.h"

class GLImplantWidget;

class RENDERER_EXPORT ImplantSliceRenderer : public BaseOffscreenRenderer {
public:
	struct Arguments {
		glm::vec3 slice_up_vector;
		glm::vec3 slice_back_vector;
		glm::vec3 center_position_in_vol_gl;
		int img_width = 0;
		int img_height = 0;
		float scale = 1.0f;
	};
	ImplantSliceRenderer();
	~ImplantSliceRenderer();

	typedef struct SHADER_PROGRAMS
	{
		uint render_slice_implant = 0;
		uint render_slice_implant_wire = 0;
		uint pick_object = 0;
	} ShaderPrograms;

public:
	void RenderInVol(const Arguments& args, resource::Image2D*& image);
	void RenderInPano(const Arguments& args, resource::Image2D*& image);

	void PickInVol(const Arguments & args, const QPointF& pt_in_image, int * pick_implant_id);
	void PickInPano(const Arguments & args, const QPointF& pt_in_image, int * pick_implant_id);

	void SetSize(int width, int height);

	glm::mat4 GetProjectionMatrix(const Arguments& args) const;
	glm::mat4 GetViewMatrix(const Arguments& args) const;

	inline void set_programs(const ShaderPrograms& programs) { programs_ = programs; }
	inline const ShaderPrograms& programs() const { return programs_; }

	inline const bool is_wire() { return is_wire_; }
	inline void set_is_wire(bool wire) { is_wire_ = wire; }
	
	void ApplyPreferences();

private:
	void Render(GLImplantWidget* implant, const Arguments& args, resource::Image2D* image_2d);
	int Pick(GLImplantWidget* implant, const Arguments & args, const QPointF& pt_in_image);
	void GetViewportSize(const int& image_width, const int& image_height, const float& scale,
						 int* viewport_width, int* viewport_height) const;
	glm::mat4 GetProjectionMatrix(int viewport_width,
								  int viewport_height) const;
	void GrabFrameImage2D(int viewport_width, int viewport_height, resource::Image2D* image_2d);

private:
	ShaderPrograms programs_;

	struct Size {
		int w = 100;
		int h = 100;

		Size() {}
		Size(int width, int height) : w(width), h(height) { };
	};

	Size size_;

	std::unique_ptr<GLImplantWidget> implant_in_vol;
	std::unique_ptr<GLImplantWidget> implant_in_pano;

	bool is_wire_ = false;
};
