#include "transform_pano_orientation.h"

#include "../../Common/GLfunctions/WGLHeaders.h"

using namespace UIViewController;

TransformPanoOrientation::TransformPanoOrientation(const ReorientViewID& type)
	: type_(type) {}

void TransformPanoOrientation::SetViewMatrix() {
	switch (type_) {
	case ReorientViewID::ORIENT_A:
		set_view(glm::lookAt(glm::vec3(0.0f, -cam_fov(), 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		break;
	case ReorientViewID::ORIENT_R:
		set_view(glm::lookAt(glm::vec3(cam_fov(), 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		break;
	case ReorientViewID::ORIENT_I:
		set_view(glm::lookAt(glm::vec3(0.0f, 0.0f, cam_fov()), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0)));
		break;
	default:
		break;
	}
	UpdateMVP();
}
