#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				view_render_param.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

 *===============================================================================================**/
#include <vector>
#include <QSize>
#include <QPointF>

#include "uiviewcontroller_global.h"
#include "uiviewcontroller_types.h"

class UIVIEWCONTROLLER_EXPORT ViewRenderParam {
public:
	ViewRenderParam() = default;
	ViewRenderParam(const ViewRenderParam& param);
	ViewRenderParam& operator=(const ViewRenderParam& param);

public:

	/**=================================================================================================
	Initializer
	*===============================================================================================**/

	void SetViewPort(int width, int height);
	void FitView();
	void SetInitScale(const float scale);

	/**=================================================================================================
	Sets
	 *===============================================================================================**/

	inline void SetEventType(const UIViewController::EVIEW_EVENT_TYPE& type);
	inline void SetRenderModeFast() { is_render_fast_ = true; }
	inline void SetRenderModeQuality() { is_render_fast_ = false; }

	inline void set_scene_size(const QSize& size) { scene_size_ = size; }
	inline void set_scene_center(const QPointF& pos) { scene_center_ = pos; }
	inline void set_gl_trans(const QPointF& translate) { gl_trans_ = translate; }
	inline void set_scene_scale(const float& scale) { scene_scale_ = scale; }
	inline void set_scene_mouse_curr(const QPointF& pos) { scene_mouse_curr_ = pos; }
	inline void set_scene_mouse_prev(const QPointF& pos) { scene_mouse_prev_ = pos; }
	inline void set_scene_offset(const QPointF& offset) { scene_offset_ = offset; }
	inline void set_view_mouse_curr(const QPointF& pos) { view_mouse_curr_ = pos; }
	inline void set_map_scene_to_gl(const float& scale) { map_scene_to_gl_ = scale; }
	inline void set_base_pixel_spacing_mm(const float& pixelSpacing) { base_pixel_spacing_mm_ = pixelSpacing; }
	inline void set_mouse_wheel_delta(const float& delta) { mouse_wheel_delta_ = delta; }
	inline void set_is_set_viewport(const bool is_set_viewport) { is_set_viewport_ = is_set_viewport; }

	//temp
	inline void set_is_valid_map_scene_to_gl(const bool& is_valid) { is_valid_map_scene_to_gl_ = is_valid; }
	inline bool is_valid_map_scene_to_gl() const { return is_valid_map_scene_to_gl_; }
	/**=================================================================================================
	Mapping Functions.
	scene좌표계와 renderer좌표계를 매핑하는 함수. GLhelper의 함수와
	ViewRenderParam의 멤버변수 map_scene_to_gl_, base_pixel_spacing_mm_를 이용하여 래핑했다.	
	*===============================================================================================**/

	QPointF MapSceneToActual(const QPointF& pt_scene) const;
	float MapSceneToActual(const float& scene) const;

	QPointF MapActualToScene(const QPointF& ptActual) const;
	float MapActualToScene(const float& actual) const;

	float MapSceneToGL(const float& scene) const;
	QPointF MapSceneToGL(const QPointF& pt_scene) const;

	float MapGLToScene(const float& gl) const;
	QPointF MapGLToScene(const QPointF& ptGL) const;

	float MapSceneToVol(const float& scene) const;
	float MapVolToScene(const float& vol) const;

	/**=================================================================================================
	Gets
	*===============================================================================================**/

	const QSize GetRenderViewSize() const;

	inline const QSize& view_size() const { return view_size_; }
	inline const QSize& scene_size() const { return scene_size_; }
	inline const QPointF& scene_center() const { return scene_center_; }

	inline int view_size_width() const { return view_size_.width(); }
	inline int view_size_height() const { return view_size_.height(); }
	inline int scene_size_width() const { return scene_size_.width(); }
	inline int scene_size_height() const { return scene_size_.height(); }

	inline const QPointF& gl_trans() const { return gl_trans_; }
	inline float gl_trans_x() const { return gl_trans_.x(); }
	inline float gl_trans_y() const { return gl_trans_.y(); }

	inline const float& scene_scale() const noexcept { return scene_scale_; }
	inline const float& init_scale() const noexcept { return init_scale_; }
	inline const float& map_scene_to_gl() const noexcept { return map_scene_to_gl_; }

	inline const bool& is_set_viewport() const noexcept { return is_set_viewport_; }
	inline const bool& is_render_fast() const noexcept { return is_render_fast_; }

	inline const QPointF& scene_mouse_curr() const noexcept { return scene_mouse_curr_; }
	inline const QPointF& scene_mouse_prev() const noexcept { return scene_mouse_prev_; }
	inline const QPointF& scene_offset() const noexcept { return scene_offset_; }
	inline const QPointF& view_mouse_curr() const noexcept { return view_mouse_curr_; }

	inline const float& base_pixel_spacing_mm() const noexcept { return base_pixel_spacing_mm_; }
	inline const float& mouse_wheel_delta() const noexcept { return mouse_wheel_delta_; }

	inline const UIViewController::EVIEW_EVENT_TYPE event_type() const noexcept { return event_type_; }

private:
	/* values for initialize */
	float init_scale_ = 1.f;

	/* members */
	QSize view_size_;
	QSize scene_size_;
	QPointF scene_center_;
	QPointF gl_trans_;
	QPointF scene_mouse_curr_;
	QPointF scene_mouse_prev_;
	QPointF scene_offset_;
	QPointF view_mouse_curr_;

	float scene_scale_ = init_scale_;
	float map_scene_to_gl_ = 0.0f;
	float base_pixel_spacing_mm_ = 0.0f;

	bool is_set_viewport_ = false;
	bool is_render_fast_ = false;

	float mouse_wheel_delta_ = 0;

	UIViewController::EVIEW_EVENT_TYPE event_type_ = UIViewController::NO_EVENT;

	//temp
	bool is_valid_map_scene_to_gl_ = true;
};
