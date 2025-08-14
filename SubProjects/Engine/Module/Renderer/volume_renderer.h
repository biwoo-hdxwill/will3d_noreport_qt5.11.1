#pragma once

/**=================================================================================================

Project: 			Renderer
File:				volume_renderer.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-21
Last modify:		2017-07-21

 *===============================================================================================**/
#include <memory>
#include <mutex>
#include <functional>
#include <QObject>

#if defined(__APPLE__)
#include <glm/detail/type_mat.hpp>
#else
#include <GL/glm/detail/type_mat.hpp>
#endif

#include "../../Common/GLfunctions/W3GLTypes.h"

#include "vr_param.h"
#include "renderer_global.h"

class CW3Image3D;
class ActiveBlockResource;
class GLObject;

class RENDERER_EXPORT VolumeRenderer : public QObject {
	Q_OBJECT

public:
	VolumeRenderer();
	~VolumeRenderer();

	VolumeRenderer(const VolumeRenderer&) = delete;
	VolumeRenderer& operator=(const VolumeRenderer&) = delete;

	typedef struct SHADER_PROGRAMS {
		uint front_face_cube = 0;
		uint front_face_final = 0;
		uint back_face_cube = 0;
		uint ray_casting = 0;
		uint ray_firsthit = 0;
		uint render_surface = 0;
		uint render_bone_density = 0;
		uint texturing_surface = 0;
		uint pick_object = 0;
		uint render_screen = 0;

	}ShaderPrograms;

signals:
	void sigRenderQuality();

public:
	/**=================================================================================================
	renderer settings ( from RendererManager )
	*===============================================================================================**/

	void InvertWindow(bool is_invert);
	void ResetDeltaWindowWL();
	void AdjustWindowWL(const float& delta_width, const float& delta_level);
	void SetActiveBlock(const int& min_intensity, const int& max_intensity);

	/**=================================================================================================
	renderer settings ( from RendererEngine )
	*===============================================================================================**/

	void SetTexHandler(unsigned int handler, unsigned int texture_id);
	void SetRenderDownFactor(int down_factor);

	void InitVRparamSet(const CW3Image3D& vol);
	void SetVolCenterZtoZero();

	void SettingsTFhandler(unsigned int handler, unsigned int texture_id);
	void SettingsTFMaxAxisTexLength(int max_tf_axis_tex_length);

	void SetEnableShade(bool is_shade);
	void SetEnableMIP(bool is_mip);
	void SetEnableXray(bool is_xray);
	void Clear();

	inline void set_programs(const ShaderPrograms& programs) { programs_ = programs; }

	/**=================================================================================================
	function calls ( from ViewController )
	*===============================================================================================**/

	void UpdateActiveBlockVAO(uint* vao);

	void SetVolumeTexture();
	void SetVolumeTexture(const uint & program);
	void SetTFtexture();
	void SetStepSizeFast();
	void SetStepSize();

	void DrawFrontFace(uint vao_cube, int indices, uint front_tex_buffer, const glm::mat4 & mvp, const glm::mat4& vol_tex_transform = glm::mat4(1.0f));
	void DrawBackFace(uint vao_cube, int indices, uint back_tex_buffer, const glm::mat4 & mvp, const glm::mat4 & vol_tex_transform = glm::mat4(1.0f));

	void DrawFinalFrontFace(GLObject* plane_obj,
		const PackTexture& pack_front,
		const PackTexture& pack_back,
		const PACK_CLIPPING& clipping_pack);

	void DrawFirstHitRay(GLObject * plane_obj,
		uint tex_buffer,
		const PackTexture & pack_front,
		const PackTexture & pack_back,
		float threshold_alpha);

	void DrawRaycastingCut(GLObject * plane_obj,
						uint ray_tex_buffer,
						int cut_step,
						const glm::mat4& mvp,
						const glm::mat4& map_vol_to_mask,
						const PackTexture & pack_front,
						const PackTexture & pack_back,
						const PackTexture & pack_mask);

	void DrawRaycasting(GLObject* plane_obj,
						uint ray_tex_buffer,
						const glm::mat4 & mvp,
						const PackTexture& pack_front,
						const PackTexture& pack_back);

	void PostProcessingEnhanced(const PackTexture& pack_texture);

	void DrawTextureToTexture(GLObject* plane_obj,
		uint dst_tex_buffer,
		const PackTexture& src_pack_texture);

	void DrawTexture(GLObject* plane_obj,
		const PackTexture& pack_textrue);

	void DrawTransparencySurface(const std::function<void()>& func_draw_surface);

	void DrawBoneDensity(GLObject* surface_obj,
						 const glm::mat4& model,
						 const glm::mat4& view,
						 const glm::mat4& projection,
						 const glm::mat4 & vol_tex_transform);

	bool IsInitialize() const;

	const unsigned int GetActiveIndices() const;
	const glm::vec3& GetModelScale() const;
	const glm::vec3& GetVolTexScale() const;
	const glm::vec3 & GetVolCenter() const;
	float GetSpacingZ() const;
	const glm::mat4& GetVolTexBias() const;
	float GetBasePixelSpacingMM() const;
	float GetIntercept() const;
	const glm::vec3& GetTexelSize() const;
	const bool& GetInvertWindow() const;
	bool IsXRAY() const;
	void GetWindowForClearColor(float* width, float* level) const;
	void GetWindowParams(float* window_width, float* window_level) const;
	int GetRenderDownFactor() const;
	inline const ShaderPrograms& programs() const { return programs_; }

	void ApplyPreferences();

private:
	void SetRaycastingWindow();
	void ClearActiveVBO();

private:
	std::unique_ptr<VRParam> vr_params_;
	std::unique_ptr<ActiveBlockResource> active_block_resource_;

	ShaderPrograms programs_;

	uint active_vbo_[3] = { 0 };
};
