#include "slice_param.h"

#include "../../Resource/Resource/W3Image3D.h"

using glm::mat4;
using glm::vec4;
using glm::vec3;

using namespace NSliceParam;


void SliceParam::SetVolume(const CW3Image3D& vol) {

	glm::vec3 vol_size = vec3(vol.width(),
							  vol.height(),
							  vol.depth());

	//vol_center_ = vol_size*0.5f - vec3(0.5f);
	//vol_center_ = vol_size * 0.5f;
	
	vol_center_ = glm::vec3((vol_size.x - 1) * 0.5f, (vol_size.y - 1) * 0.5f, (vol_size.z - 1) * 0.5f/* * m_fSpacingZ[id] * 0.5f*/);

	z_spacing_ = vol.sliceSpacing() / vol.pixelSpacing();

	float xsize = (vol_size.x)*vol.pixelSpacing();
	float ysize = (vol_size.y)*vol.pixelSpacing();
	float zsize = (vol_size.z)*vol.sliceSpacing();

	model_scale_ = glm::vec3(vol_size.x,
		vol_size.y,
		vol_size.z*z_spacing_);

	base_pixel_size_mm_ = std::min(vol.pixelSpacing(), vol.sliceSpacing());
	intercept_ = vol.intercept();
	float max_axis_size = std::max(std::max(xsize, ysize), zsize);

	vol_tex_scale_ = vec3(xsize / max_axis_size, ysize / max_axis_size, zsize / max_axis_size);

	intensity_min_ = static_cast<float>(vol.getMin());
	intensity_max_ = static_cast<float>(vol.getMax());
	wwl_ = Renderer::WindowWL(static_cast<float>(vol.windowWidth()), static_cast<float>(vol.windowCenter()));

	vol_tex_bias_ = mat4(vec4(-0.5f, 0.0f, 0.0f, 0.0f),
						 vec4(0.0f, 0.5f, 0.0f, 0.0f),
						 vec4(0.0f, 0.0f, 0.5f, 0.0f),
						 vec4(0.5f, 0.5f, 0.5f, 1.0f));

	mvp_for_final_ = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f) *
		lookAt(vec3(0.0f, 0.0f, 1.0f),
			   vec3(0.0f, 0.0f, 0.0f),
			   vec3(0.0f, 1.0f, 0.0f));
	
	texel_size_ = vec3(1.0f / vol_size.x, 1.0f / vol_size.y, 1.0f / vol_size.z);

	vol_vertex_ = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(vol_size.x, 0.0f, 0.0f),
		glm::vec3(0.0f, vol_size.y, 0.0f),
		glm::vec3(vol_size.x, vol_size.y, 0.0f),
		glm::vec3(0.0f, 0.0f, (vol_size.z)*z_spacing_),
		glm::vec3(vol_size.x, 0.0f, (vol_size.z)*z_spacing_),
		glm::vec3(0.0f, vol_size.y, (vol_size.z)*z_spacing_),
		glm::vec3(vol_size.x, vol_size.y, (vol_size.z)*z_spacing_),
	};

	SliceLoc loc = vol.getSliceLoc();
	location_slice_in_vol_.maxilla = loc.maxilla;
	location_slice_in_vol_.teeth = loc.teeth;
	location_slice_in_vol_.nose = loc.nose;
	location_slice_in_vol_.chin = loc.chin;

	is_set_volume_ = true;

}

void SliceParam::SetVolTexHandler(unsigned int handler, unsigned int texture_id) {
	vol_tex_handler_ = handler;
	vol_tex_id_ = texture_id;
}

bool SliceParam::IsInitialize() const {
	if (is_set_volume_ && vol_tex_handler_ != 0)
		return true;
	else
		return false;
}
