#include "image_param.h"

#include "../../Resource/Resource/W3Image3D.h"

void ImageParam::SetVolume(const CW3Image3D& vol) {
	base_pixel_size_mm_ = vol.pixelSpacing();
	intercept_ = vol.intercept();
	intensity_min_ = static_cast<float>(vol.getMin());
	intensity_max_ = static_cast<float>(vol.getMax());

	wwl_ = Renderer::WindowWL(static_cast<float>(vol.windowWidth()), static_cast<float>(vol.windowCenter()));
	is_set_volume_ = true;
}

bool ImageParam::IsInitialize() const {
	if (is_set_volume_)
		return true;
	else
		return false;
}
