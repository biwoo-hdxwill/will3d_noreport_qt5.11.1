#pragma once

/**=================================================================================================

Project: 			Renderer
File:				slice_param.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-01
Last modify:		2017-08-01

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

namespace NSliceParam {
	struct SliceLocations {
		int maxilla = 0;
		int teeth = 0;
		int chin = 0;
		int nose = 0;
	};
};

class SliceParam {
public:
	SliceParam() = default;
	SliceParam(const SliceParam&) = delete;
	SliceParam& operator=(const SliceParam) = delete;

public:
	void SetVolume(const CW3Image3D& vol);
	void SetVolTexHandler(unsigned int handler, unsigned int texture_id);

	inline void set_invert_window(const bool& is_invert) { invert_window_ = is_invert; }
	inline void set_wwl(const Renderer::WindowWL& wwl) { wwl_ = wwl; }

	inline const bool& invert_window() const { return invert_window_; }
	inline unsigned int vol_tex_handler() const { return vol_tex_handler_; }
	inline unsigned int vol_tex_id() const { return vol_tex_id_; }
	inline int vol_tex_num() const { return (int)((vol_tex_id_ & 0x00FF) - 0x00C0); }	
	inline const glm::mat4& mvp_for_final() const { return mvp_for_final_; }
	inline const glm::mat4& vol_tex_bias() const { return vol_tex_bias_; }
	inline const glm::vec3& model_scale() const { return model_scale_; }
	inline const glm::vec3& vol_center() const { return vol_center_; }
	inline const glm::vec3& vol_tex_scale() const { return vol_tex_scale_; }
	inline const glm::vec3& texel_size() const { return texel_size_; }
	inline float base_pixel_size_mm() const { return base_pixel_size_mm_; }
	inline float intensity_min() const { return intensity_min_; }
	inline float intensity_max() const { return intensity_max_; }
	inline const Renderer::WindowWL& wwl() const { return wwl_; }
	inline const float& intercept() const { return intercept_; }
	inline float z_spacing() const { return z_spacing_; }
	inline const std::vector<glm::vec3>& vol_vertex() const { return vol_vertex_; }
	inline const NSliceParam::SliceLocations& location_slice_in_vol() const { return location_slice_in_vol_; }
	bool IsInitialize() const;

private:
	unsigned int vol_tex_handler_ = 0;
	unsigned int vol_tex_id_ = 0;

	float intensity_max_ = 0.0f;
	float intensity_min_ = 0.0f;

	bool invert_window_ = false;

	Renderer::WindowWL wwl_;

	float z_spacing_ = 0.0f;

	bool is_set_volume_ = false;

	glm::vec3 vol_center_;
	glm::mat4 vol_tex_bias_;
	glm::mat4 mvp_for_final_;
	glm::vec3 vol_tex_scale_;

	glm::vec3 model_scale_;
	glm::vec3 texel_size_;
	std::vector<glm::vec3> vol_vertex_;

	float base_pixel_size_mm_ = 0.0f;
	float intercept_ = 0.0f;
	NSliceParam::SliceLocations location_slice_in_vol_;
};
