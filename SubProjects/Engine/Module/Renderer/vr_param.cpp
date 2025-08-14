#include "vr_param.h"

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Resource/Resource/W3Image3D.h"

using glm::mat4;
using glm::vec4;
using glm::vec3;

namespace {
	const float STEP_LOW_FACTOR = 4.0f;
}


void VRParam::SetVolume(const CW3Image3D& vol) {

	vec3 vol_size = vec3((float)vol.width(),
						 (float)vol.height(),
						 (float)vol.depth());

	vol_center_ = vol_size * 0.5f - vec3(0.5f);

	z_spacing_ = vol.sliceSpacing() / vol.pixelSpacing();

	model_scale_ = vec3(vol_size.x,
						vol_size.y,
						vol_size.z*z_spacing_);

	intensity_min_ = static_cast<float>(vol.getMin());
	intensity_max_ = static_cast<float>(vol.getMax());

	float xsize = (vol_size.x)*vol.pixelSpacing();
	float ysize = (vol_size.y)*vol.pixelSpacing();
	float zsize = (vol_size.z)*vol.sliceSpacing();

	base_pixel_size_mm_ = std::min(vol.pixelSpacing(), vol.sliceSpacing());
	max_axis_size_ = std::max(std::max(xsize, ysize), zsize);

	vol_tex_scale_ = vec3(xsize / max_axis_size_, ysize / max_axis_size_, zsize / max_axis_size_);
	vol_tex_scale_inv_ = vec3(1.0f) / vol_tex_scale_;

	SetRaycastingStepSize();

	texel_size_ = vec3(1.0f / vol_size.x, 1.0f / vol_size.y, 1.0f / vol_size.z);

	vol_tex_bias_ = mat4(vec4(-0.5f, 0.0f, 0.0f, 0.0f),
						 vec4(0.0f, 0.5f, 0.0f, 0.0f),
						 vec4(0.0f, 0.0f, 0.5f, 0.0f),
						 vec4(0.5f, 0.5f, 0.5f, 1.0f));

	mvp_for_final_ = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f) *
		glm::lookAt(vec3(0.0f, 0.0f, 1.0f),
					vec3(0.0f, 0.0f, 0.0f),
					vec3(0.0f, 1.0f, 0.0f));

	intercept_ = vol.intercept();

	wwl_ = Renderer::WindowWL(static_cast<float>(vol.windowWidth()), static_cast<float>(vol.windowCenter()));
	bone_threshold_ = (float)vol.getTissueBoneThreshold();

	is_set_volume_ = true;
}

void VRParam::SetRenderDownFactor(int down_factor) {
	if (!is_set_volume_) {
		std::cout << "VRParam::SetRenderDownFactor: It is not set volume." << std::endl;
		assert(false);
	}
	down_factor_ = down_factor;
	SetRaycastingStepSize();
}


void VRParam::SetVolTexHandler(unsigned int handler, unsigned int texture_id) {
	vol_tex_handler_ = handler;
	vol_tex_id_ = texture_id;
}

void VRParam::SetTFhandler(unsigned int handler, unsigned int texture_id) {
	tf_tex_handler_ = handler;
	tf_tex_id_ = texture_id;
}

void VRParam::SetTFMaxAxisTexLength(int maxTFAxisTexLength) {
	tf_max_axis_tex_length_ = maxTFAxisTexLength;
}

bool VRParam::IsInitialize() const {
	if (is_set_volume_ && vol_tex_handler_ != 0)
		return true;
	else
		return false;
}
void VRParam::SetIsMIP(bool is_mip) {

	if (is_mip) {
		is_mip_ = true;
		is_xray_ = false;
	} else {
		is_mip_ = false;
	}
}

void VRParam::SetIsXRAY(bool is_xray) {

	if (is_xray) {
		is_mip_ = false;
		is_xray_ = true;
	} else {
		is_xray_ = false;
	}

}

void VRParam::SetRaycastingStepSize()
{
	step_size_ = (base_pixel_size_mm_ * (float)down_factor_) / (max_axis_size_ * high_res_step_size_factor_);
	step_size_fast_ = step_size_ * low_res_step_size_factor_;
}

void VRParam::ApplyPreferences()
{
	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;
	switch (volume_rendering_quality)
	{
	case GlobalPreferences::Quality2::High:
		low_res_step_size_factor_ = 2.0f;
		break;
	case GlobalPreferences::Quality2::Low:
		low_res_step_size_factor_ = 8.0f;
		break;
	}

	SetRaycastingStepSize();
}
