#pragma once

/**=================================================================================================

Project: 			Renderer
File:				slice_renderer.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-01
Last modify:		2017-08-01

 *===============================================================================================**/
#include <memory>
#include <mutex>
#include <QObject>
#if defined(__APPLE__)
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/mat4x4.hpp>
#endif
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "slice_param.h"
#include "renderer_global.h"

class CW3Image3D;
class GLObject;

class RENDERER_EXPORT SliceRenderer : public QObject {
	Q_OBJECT
public:
	SliceRenderer();
	SliceRenderer(const SliceRenderer&) = delete;
	SliceRenderer& operator=(const SliceRenderer&) = delete;


	typedef struct SHADER_PROGRAMS {
		uint render_slice = 0;
		uint render_screen = 0;
		uint render_slice_nerve = 0;
		uint render_slice_implant = 0;
		uint render_slice_implant_wire = 0;
		uint pick_object = 0;
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

	void SetTexHandler(unsigned int handler, unsigned int texture_id);
	void InitSliceParamSet(const CW3Image3D& vol);

	inline void set_programs(const ShaderPrograms& programs) { programs_ = programs; }

	/**=================================================================================================
	function calls ( from ViewController )
	*===============================================================================================**/

	void SetVolumeTexture();
	void SetSliceWindow(float level, float width);
	void DrawSlice(GLObject * slice_obj, uint tex_buffer,
		const glm::mat4& mvp, const glm::mat4& vol_tex_transform_mat,
		const glm::vec3& up_vector, int thickness, bool cull_face = true, bool get_data = false);

	void DrawScreen(GLObject * planeObj, const PackTexture& pack_screen);

	void PostProcessingSharpen(const PackTexture& pack_texture, const SharpenLevel& level);

	bool IsInitialize() const;
	
	void GetSliceRange(const glm::vec3& slice_up_vector, float& gl_min, float& gl_max) const;
	
	float GetLocationChin() const;
	float GetLocationNose() const;

	const glm::vec3& GetModelScale() const;
	const glm::vec3& GetVolCenter() const;
	const glm::vec3& GetVolTexScale() const;
	float GetBasePixelSpacingMM() const;
	float GetIntercept() const;
	 
	float GetSpacingZ() const;
	const std::vector<glm::vec3>& GetVolVertex() const;
	const bool& GetInvertWindow() const;

	inline const ShaderPrograms& programs() const { return programs_; }

	void GetWindowParams(float* window_width, float* window_level) const;

private:
	float GetMaximumLengthVectorInVol(const glm::vec3& vec) const;

private:
	std::unique_ptr<SliceParam> slice_params_;
	ShaderPrograms programs_;
};
