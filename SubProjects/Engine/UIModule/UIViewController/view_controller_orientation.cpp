#include "view_controller_orientation.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Module/Will3DEngine/renderer_manager.h"

#include "view_render_param.h"

using namespace UIViewController;
using namespace Will3DEngine;

ViewControllerOrientation::ViewControllerOrientation(ReorientViewID type)
	: type_(type) {
	transform_.reset(new TransformPanoOrientation(type_));
}

ViewControllerOrientation::~ViewControllerOrientation() {}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewControllerOrientation::ClearGL() {
	BaseViewController3D::ClearGL();
}

void ViewControllerOrientation::SetRotateAngle(int angle) {
	transform_->set_angle((float)angle);
}
float ViewControllerOrientation::GetRotateAngle() const {
	return transform_->angle();
}

/**=================================================================================================
private functions: level 1
*===============================================================================================**/
VolumeRenderer& ViewControllerOrientation::Renderer() const {
	return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}

/**=================================================================================================
private functions: level 2
*===============================================================================================**/

BaseTransform& ViewControllerOrientation::transform() const {
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

glm::vec3 ViewControllerOrientation::GetArcBallVector(const QPointF& pt_gl) {
	vec3 arcball_vector;

	float pow;
	switch (type_) {
	case ReorientViewID::ORIENT_A:
		arcball_vector = vec3(pt_gl.x(), 0.0f, 0.0f);
		pow = arcball_vector.x*arcball_vector.x;

		if (pow < 1.0f) {
			arcball_vector.z = sqrt(1.0f - pow);
		} else {
			arcball_vector = glm::normalize(arcball_vector);
		}
		break;

	case ReorientViewID::ORIENT_R:
		arcball_vector = vec3(0.0f, pt_gl.y(), 0.0f);
		pow = arcball_vector.y*arcball_vector.y;

		if (pow < 1.0f) {
			arcball_vector.z = sqrt(1.0f - pow);
		} else {
			arcball_vector = glm::normalize(arcball_vector);
		}
		break;

	case ReorientViewID::ORIENT_I:
		arcball_vector = vec3(pt_gl.x(), 0.0f, 0.0f);
		pow = arcball_vector.x*arcball_vector.x;

		if (pow < 1.0f) {
			arcball_vector.y = sqrt(1.0f - pow);
		} else {
			arcball_vector = glm::normalize(arcball_vector);
		}
		break;

	default:
		break;
	}

	return arcball_vector;
}

void ViewControllerOrientation::ApplyPreferences()
{
	BaseViewController3D::ApplyPreferences();
}

void ViewControllerOrientation::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
