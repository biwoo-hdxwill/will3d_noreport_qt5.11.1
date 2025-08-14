#include "view_controller_pano3d.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../UIGLObjects/gl_nerve_widget.h"
#include "../../UIGLObjects/gl_implant_widget.h"

#include "surface_items.h"

using namespace Will3DEngine;
using namespace UIViewController;
using namespace UIGLObjects;

ViewControllerPano3D::ViewControllerPano3D(){
	transform_.reset(new TransformPanoVR());

	surface_items()->InitItem(SurfaceItems::NERVE, ObjCoordSysType::TYPE_PANORAMA);
	surface_items()->InitItem(SurfaceItems::IMPLANT, ObjCoordSysType::TYPE_PANORAMA);
}

ViewControllerPano3D::~ViewControllerPano3D() {
}

void ViewControllerPano3D::SetProjection() {
	glm::vec3 world_scale = Renderer().GetModelScale();
	BaseViewController3D::SetProjectionFitIn(world_scale.x*2.0f, world_scale.y*2.0f);
}

VolumeRenderer& ViewControllerPano3D::Renderer() const {
	return RendererManager::GetInstance().renderer_vol(VOL_PANORAMA);
}

BaseTransform& ViewControllerPano3D::transform() const {
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}

glm::vec3 ViewControllerPano3D::GetArcBallVector(const QPointF& pt_gl) {
	vec3 arcball_vector = vec3(-pt_gl.x(), pt_gl.y(), 0.0f);

	float sqrt_yz = arcball_vector.x*arcball_vector.x + arcball_vector.y*arcball_vector.y;

	if (sqrt_yz < 1.0f) {
		arcball_vector.z = sqrt(1.0f - sqrt_yz);
	} else {
		arcball_vector = glm::normalize(arcball_vector);
	}

	return arcball_vector;
}

void ViewControllerPano3D::ApplyPreferences()
{
	BaseViewController3D::ApplyPreferences();
}

void ViewControllerPano3D::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}
