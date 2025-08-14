#include "view_render_param.h"

#include <QDebug>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

using namespace UIViewController;
ViewRenderParam::ViewRenderParam(const ViewRenderParam& param) {
	*this = param;
}
ViewRenderParam& ViewRenderParam::operator=(const ViewRenderParam& param) {

	this->view_size_ = param.view_size();
	this->scene_size_ = param.scene_size();
	this->scene_center_ = param.scene_center();
	this->gl_trans_ = param.gl_trans();
	this->scene_mouse_curr_ = param.scene_mouse_curr();
	this->scene_mouse_prev_ = param.scene_mouse_prev();
	this->scene_offset_ = param.scene_offset();
	this->scene_scale_ = param.scene_scale();
	this->is_render_fast_ = param.is_render_fast();
	this->map_scene_to_gl_ = param.map_scene_to_gl();
	this->base_pixel_spacing_mm_ = param.base_pixel_spacing_mm();
	this->event_type_ = param.event_type();
	this->is_set_viewport_ = param.is_set_viewport();
	return *this;
}
/**=================================================================================================
Initializer
*===============================================================================================**/

void ViewRenderParam::SetViewPort(int width, int height)
{
#if 0
	int width_ = width;
	int height_ = height;
	int remainder_w = width % 2;
	int remainder_h = height % 2;

	if (remainder_w == 1)
	{
		width_ += 1;
	}

	if (remainder_h == 1)
	{
		height_ += 1;
	}
	view_size_ = QSize(width_, height_);
#else
	view_size_ = QSize(width, height);
#endif
	is_set_viewport_ = true;
}

void ViewRenderParam::FitView() {
	scene_scale_ = init_scale_;
	gl_trans_ = QPointF(0.0f, 0.0f);
}

void ViewRenderParam::SetInitScale(const float scale)
{
	init_scale_ = scene_scale_ = scale;
}

/**=================================================================================================
Sets
*===============================================================================================**/

inline void ViewRenderParam::SetEventType(const EVIEW_EVENT_TYPE & type) {
	if (event_type_ == EVIEW_EVENT_TYPE::NO_EVENT ||
		type == EVIEW_EVENT_TYPE::NO_EVENT)
		event_type_ = type;
}

/**=================================================================================================
Mapping Functions
*===============================================================================================**/

QPointF ViewRenderParam::MapSceneToActual(const QPointF& pt_scene) const {
	return QPointF(MapSceneToActual(pt_scene.x()), MapSceneToActual(pt_scene.y()));
}

float ViewRenderParam::MapSceneToActual(const float& scene) const {
	return GLhelper::ScaleGLtoVol(scene * map_scene_to_gl_ * base_pixel_spacing_mm_);
}

QPointF ViewRenderParam::MapActualToScene(const QPointF& ptActual) const {
	return QPointF(MapActualToScene(ptActual.x()), MapActualToScene(ptActual.y()));
}

float ViewRenderParam::MapActualToScene(const float& actual) const {
	return GLhelper::ScaleVolToGL(actual / map_scene_to_gl_ / base_pixel_spacing_mm_);
}

float ViewRenderParam::MapSceneToGL(const float& pt_scene) const {
	return pt_scene * map_scene_to_gl_;
}

QPointF ViewRenderParam::MapSceneToGL(const QPointF& pt_scene) const {
	return QPointF(MapSceneToGL(pt_scene.x()), MapSceneToGL(pt_scene.y()));
}

float ViewRenderParam::MapGLToScene(const float& gl) const {
	return gl / map_scene_to_gl_;
}

QPointF ViewRenderParam::MapGLToScene(const QPointF& ptGL) const {
	return QPointF(MapGLToScene(ptGL.x()), MapGLToScene(ptGL.y()));
}

float ViewRenderParam::MapSceneToVol(const float& scene) const {
	return GLhelper::ScaleGLtoVol(scene*map_scene_to_gl_);
}

float ViewRenderParam::MapVolToScene(const float& vol) const {
	return GLhelper::ScaleVolToGL(vol/map_scene_to_gl_);
}

/**=================================================================================================
Gets
*===============================================================================================**/

/* render mode 에 맞는 view size return */
const QSize ViewRenderParam::GetRenderViewSize() const {
	float low_res_frame_buffer_resize_factor = 1.0f;
	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;
	switch (volume_rendering_quality)
	{
	case GlobalPreferences::Quality2::High:
		low_res_frame_buffer_resize_factor = 0.5f;
		break;
	case GlobalPreferences::Quality2::Low:
		low_res_frame_buffer_resize_factor = 0.25f;
		break;
	}

	if (is_render_fast_)
		return QSize(view_size_.width() * low_res_frame_buffer_resize_factor, view_size_.height() * low_res_frame_buffer_resize_factor);
	else
		return view_size_;

}
