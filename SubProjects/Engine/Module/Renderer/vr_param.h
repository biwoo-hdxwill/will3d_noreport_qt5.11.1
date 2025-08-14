#pragma once

/**=================================================================================================

Project: 			Renderer
File:				vr_param.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-21
Last modify:		2017-07-21

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/mat4x4.hpp>
#endif

#include "defines.h"
class CW3Image3D;

class VRParam {
public:
	VRParam() = default;
	VRParam(const VRParam&) = delete;
	VRParam& operator=(const VRParam&) = delete;

public:
	void SetVolume(const CW3Image3D& vol);
	void SetRenderDownFactor(int down_factor);
	void SetVolTexHandler(unsigned int handler, unsigned int texture_id);

	void SetTFhandler(unsigned int handler, unsigned int texture_id);
	void SetTFMaxAxisTexLength(int maxTFAxisTexLength);
	void SetIsMIP(bool is_mip);
	void SetIsXRAY(bool is_xray);

	inline void set_invert_window(bool is_invert) { invert_window_ = is_invert; }
	inline void set_is_shade(bool is_shade) { is_shade_ = is_shade; }
	inline void set_wwl(const Renderer::WindowWL& wwl) { wwl_ = wwl; }
	inline void set_vol_center(const glm::vec3 vol_center) { vol_center_ = vol_center; }

	inline const glm::mat4& vol_tex_bias() const { return vol_tex_bias_; }
	inline const glm::mat4& mvp_for_final() const { return mvp_for_final_; }
	inline int tf_max_axis_tex_length() const { return tf_max_axis_tex_length_; }
	inline unsigned int vol_tex_handler() const { return vol_tex_handler_; }
	inline unsigned int vol_tex_id() const { return vol_tex_id_; }
	inline int vol_tex_num() const { return (int)((vol_tex_id_ & 0x00FF) - 0x00C0); }
	inline unsigned int tf_tex_handler() const { return tf_tex_handler_; }
	inline unsigned int tf_tex_id() const { return tf_tex_id_; }
	inline int tf_tex_num() const { return (int)((tf_tex_id_ & 0x00FF) - 0x00C0); }
	inline const glm::vec3& vol_center() const { return vol_center_; }
	inline const glm::vec3& model_scale() const { return model_scale_; }
	inline const glm::vec3& vol_tex_scale() const { return vol_tex_scale_; }
	inline const glm::vec3& vol_tex_scale_inv() const { return vol_tex_scale_inv_; }
	inline const glm::vec3& texel_size() const { return texel_size_; }
	inline float intensity_max() const { return intensity_max_; }
	inline float step_size() const { return step_size_; }
	inline float step_size_fast() const { return step_size_fast_; }
	inline float base_pixel_size_mm() const { return base_pixel_size_mm_; }
	inline float z_spacing() const { return z_spacing_; }

	inline const bool& invert_window() const { return invert_window_; }
	inline bool is_shade() const { return is_shade_; }
	inline bool is_mip() const { return is_mip_; }
	inline bool is_xray() const { return is_xray_; }
	inline const Renderer::WindowWL& wwl() const { return wwl_; }
	inline float intercept() const { return intercept_; }
	inline float bone_threshold() const { return bone_threshold_; }
	inline int down_factor() const { return down_factor_; }

	bool IsInitialize() const;
	
	void ApplyPreferences();

private:
	void SetRaycastingStepSize();

private:
	unsigned int vol_tex_handler_ = 0;
	unsigned int vol_tex_id_ = 0;

	unsigned int tf_tex_handler_ = 0;
	unsigned int tf_tex_id_ = 0;

	int tf_max_axis_tex_length_ = 0;

	glm::vec3 vol_center_;
	glm::mat4 vol_tex_bias_;
	glm::mat4 mvp_for_final_;
	glm::vec3 vol_tex_scale_;
	glm::vec3 vol_tex_scale_inv_;

	glm::vec3 model_scale_;
	glm::vec3 texel_size_;

	float base_pixel_size_mm_ = 0.0f;

	float intensity_min_ = 0.0f;
	float intensity_max_ = 0.0f;

	float step_size_ = 0.0f;
	float step_size_fast_ = 0.0f;
	float max_axis_size_ = 0.0f;

	float z_spacing_ = 0.0f;
	float intercept_ = 0.0f;

	float bone_threshold_ = 0.0f;

	bool invert_window_ = false;
	int down_factor_ = 1;
	Renderer::WindowWL wwl_;

	bool is_set_volume_ = false;
	bool is_shade_ = false;
	bool is_mip_ = false;
	bool is_xray_ = false;


	float high_res_step_size_factor_ = 6.0f;
	float low_res_step_size_factor_ = 8.0f;
};
