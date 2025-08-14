#pragma once

/**=================================================================================================

Project: 			Renderer
File:				image_renderer.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-23
Last modify:		2017-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <memory>
#include <QObject>

#include "../../Common/GLfunctions/W3GLTypes.h"

#include "image_param.h"
#include "renderer_global.h"

class GLObject;
class CW3Image3D;

class RENDERER_EXPORT ImageRenderer : public QObject {
	Q_OBJECT
public:
	ImageRenderer();
	ImageRenderer(const ImageRenderer&) = delete;
	ImageRenderer& operator=(const ImageRenderer&) = delete;


	typedef struct SHADER_PROGRAMS {
		uint render_image = 0;
		uint render_mask = 0;
		uint render_surface = 0;

	}ShaderPrograms;

public:

	/**=================================================================================================
	renderer settings ( from RendererManager )
	*===============================================================================================**/

	void InvertWindow(bool is_invert);
	void ResetDeltaWindowWL();
	void AdjustWindowWL(const float& delta_width, const float& delta_level);

	/**=================================================================================================
	renderer settings ( from RendererEngine )
	*===============================================================================================**/

	void SettingsImageParamSet(const CW3Image3D& vol);
	inline void set_programs(const ShaderPrograms& programs) { programs_ = programs; }

	/**=================================================================================================
	function calls ( from ViewController )[
	*===============================================================================================**/

	void DrawImage(GLObject* planeObj, const glm::mat4& mvp, const PackTexture& pack_image);
	void BlendImage(GLObject * planeObj, const glm::mat4 & mvp, const PackTexture& pack_mask);

	void PostProcessingSharpen(const PackTexture& pack_texture, SharpenLevel level);
	bool IsInitialize() const;

	float GetBasePixelSpacingMM() const;
	float GetIntercept() const;
	void GetWindowParams(float* window_width, float* window_level) const;
	const bool& GetInvertWindow() const;


	inline const ShaderPrograms& programs() const { return programs_; }
private:

	std::unique_ptr<ImageParam> image_params_;

	ShaderPrograms programs_;
};
