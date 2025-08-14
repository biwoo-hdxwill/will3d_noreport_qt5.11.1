#include "transform_pano_vr.h"

#include "../../Common/GLfunctions/WGLHeaders.h"

TransformPanoVR::TransformPanoVR() {
	//초기화 안된상태에서 viewmaxtrix가 필요했음.(BoneDensityView에서 쓰임)
	const float kInitialCamFOV = 500.0f;
	set_view(glm::lookAt(glm::vec3(0.0f, 0.0f, kInitialCamFOV), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
}

void TransformPanoVR::SetViewMatrix()
{
	set_view(glm::lookAt(glm::vec3(0.0f, 0.0f, cam_fov()), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	UpdateMVP();
}
