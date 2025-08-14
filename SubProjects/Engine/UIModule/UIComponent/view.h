#pragma once

/**=================================================================================================

Project: 			UIComponent
File:				Cview.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

 *===============================================================================================**/
#include <memory>

#include <QGraphicsView>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <gl/glm/glm.hpp>
#endif

#include "../../Common/Common/define_view.h"

#include "uicomponent_global.h"
class ViewRenderParam;
class Scene;

namespace UIViewController
{
	enum EVIEW_EVENT_TYPE;
}  // end of namespace UIViewController

namespace common
{
	namespace measure
	{
		enum class SyncType;
		struct VisibilityParams;
	}  // end of namespace measure
}  // end of namespace common

class UICOMPONENT_EXPORT View : public QGraphicsView
{
	Q_OBJECT
public:
	View(const common::ViewTypeID& view_type, QWidget* parent);
	virtual ~View();

	View(const View&) = delete;
	View& operator=(const View&) = delete;

signals:
	void sigProcessedZoomEvent(float scene_scale);
	void sigProcessedPanEvent(const QPointF& gl_translate);
	void sigProcessedLightEvent();

	void sigZoomDone();  // temp...
	void sigRequestSyncViewStatus(float* scene_scale, QPointF* gl_translate);

#ifdef WILL3D_EUROPE
	void sigSyncControlButton(bool);
	void sigShowButtonListDialog(const QPoint& window_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public:
	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(float* scene_scale, float* scene_to_gl, QPointF* trans_gl);
	void importProject(const float& scene_scale, const float& scene_to_gl,
		const QPointF& trans_gl,
		const bool& is_counterpart_exists = false,
		const bool& is_update_resource = true);
#endif
	void SetRenderModeQuality(bool is_high_quality);

	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	void SceneUpdate();
	void SetZoomScale(float scene_scale);
	void SetPanTranslate(const QPointF& gl_translate);
	void SetGridOnOff(bool visible);

	virtual void setVisible(bool state) override;

	virtual void ApplyPreferences();
	const bool IsTextVisible() const;
	const bool IsUIVisible() const;

	bool IsMeasureInteractionAvailable();
	void SyncMeasureResourceSiblings(const bool& is_update_resource);
	void SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform = true);
	void SyncCreateMeasureUI(const unsigned int& measure_id);
	void SyncDeleteMeasureUI(const unsigned int& measure_id);
	void SyncModifyMeasureUI(const unsigned int& measure_id);
	void GetMeasureParams(const common::ViewTypeID& view_type,
		const unsigned int& measure_id,
		common::measure::VisibilityParams* measure_params);

	inline const bool& is_view_ready() const noexcept { return is_view_ready_; }
	inline const common::ViewTypeID& view_type() const noexcept { return view_type_; }
	inline const unsigned int& view_id() const noexcept { return view_id_; }

	const float& GetSceneScale() const;
	const QPointF& GetGLTranslate() const;

	void SetInitScale(const float scale);

#ifdef WILL3D_EUROPE
	inline void SetSyncControlButton(bool is_on) { is_control_key_on_ = is_on; }
#endif // WILL3D_EUROPE

protected slots:
	virtual void slotRotateMatrix(const glm::mat4& mat) {};
	virtual void slotChangedValueSlider(int) {};
	virtual void slotActiveFilteredItem(const QString& text) {};
	virtual void slotActiveSharpenItem(const int index) {};
	virtual void slotGetProfileData(const QPointF& start_pt_scene,
		const QPointF& end_pt_scene,
		std::vector<short>& data)
	{
	}
	virtual void slotGetROIData(const QPointF& start_pt_scene,
		const QPointF& end_pt_scene,
		std::vector<short>& data)
	{
	}

protected:
	virtual void InitializeController() = 0;
	virtual bool IsReadyController() = 0;
	virtual void ClearGL() = 0;
	virtual void ActiveControllerViewEvent() = 0;

	virtual void drawBackground(QPainter* painter, const QRectF& rect) = 0;
	virtual void resizeEvent(QResizeEvent* pEvent) = 0;

	virtual void TransformItems(const QTransform& transform) {};
	virtual void SetGraphicsItems();

	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	bool IsEnableGLContext() const;
	bool IsSetTool() const;
	bool IsMouseOnViewItems() const;
	common::CommonToolTypeOnOff tool_type() const noexcept
	{
		return common_tool_type_;
	}
	bool IsEventLeftButton(QMouseEvent* event) const;
	bool IsEventRightButton(QMouseEvent* event) const;
	bool IsEventLRButton(QMouseEvent* event) const;

	bool MakeCurrent();
	bool DoneCurrent();
	void SetRenderModeFast();
	void SetRenderModeQuality();

	void SetViewEvent(const UIViewController::EVIEW_EVENT_TYPE& event);
	void SetEvent();
	virtual void ResetView();
	void FitView();
	void InvertView();
	virtual void DeleteUnfinishedMeasure();
	virtual void DeleteAllMeasure();
	virtual void HideText(bool is_hide);
	virtual void HideAllUI(bool is_hide);
	virtual void HideMeasure(bool is_hide);

	void SetViewRenderParam(
		const std::shared_ptr<ViewRenderParam>& view_render_param);

	void ProcessZoomWheelEvent(QWheelEvent* event);

	inline void set_pt_scene_current(const QPointF& pt)
	{
		pt_scene_current_ = pt;
	}
	inline void set_pt_scene_prev(const QPointF& pt) { pt_scene_prev_ = pt; }
	void UpdateDoneContoller();

	inline Scene& scene() const { return *scene_; }
	inline uint GetDefaultFrameBufferObject() const;

	inline const std::shared_ptr<ViewRenderParam>& view_render_param() const
	{
		return view_render_param_;
	}
	inline const QPointF& pt_scene_center() const { return pt_scene_center_; }
	inline const QPointF& pt_scene_center_pre() const
	{
		return pt_scene_center_pre_;
	}
	inline const QPointF& pt_scene_pressed() const { return pt_scene_pressed_; }
	inline const QPointF& pt_scene_current() const { return pt_scene_current_; }
	inline const QPointF& pt_scene_prev() const { return pt_scene_prev_; }
	inline const bool& is_pressed() const { return is_pressed_; }
	bool IsUpdateController() const;
	void SetDisableMeasureEvent();
	QPointF GetSceneCenterPos();

private:
	void NewQOpenGLWidget();
	void DeleteQOpenGLWidget();
	void ProcessViewEvent();
	void ClearViewEvent();

	QSize GetSceneSize();

	void TranslateItems(const QPointF& translate);
	void ScaleItems(const QPointF& pre_view_center_in_scene,
		const QPointF& cur_view_center_in_scene, float scale);
	void TransformPositionItems(const QTransform& transform, const bool translate = false);

	void ProcessResizeEvent();
	void ProcessResetEvent();
	void ProcessPanEvent();
	void ProcessZoomEvent();
	void ProcessFitEvent();
	void ProcessInvertEvent();
	void ProcessLightEvent();
	void ProcessMeasureEvent();
	void ProcessDefaultEvent();

	void SetMousePosCurr(const QPointF& pt_scene);
	void SetMousePosPrev(const QPointF& pt_scene);

	void SyncViewStatus();

	void ChangedViewRulerItem(float old_view_len_in_scene, float changed_length);

protected:
	Scene* scene_ = nullptr;
	bool is_measure_delete_event_ = false;
	bool is_current_measure_available_ = false;
	bool is_right_button_clicked_ = false;

#ifdef WILL3D_EUROPE
	bool is_control_key_on_ = false;
#endif // WILL3D_EUROPE

private:
	std::unique_ptr<QOpenGLWidget> gl_widget_;
	bool is_pressed_ = false;

	QPointF pt_scene_center_;
	QPointF pt_scene_center_pre_;

	QPointF pt_scene_pressed_;
	QPointF pt_scene_current_;
	QPointF pt_scene_prev_;

	std::shared_ptr<ViewRenderParam> view_render_param_;

	common::CommonToolTypeOnOff common_tool_type_ =
		common::CommonToolTypeOnOff::NONE;
	common::ViewTypeID view_type_;

	bool is_view_ready_ = false;
	bool is_first_resize_ = false;
	bool is_update_controller_ = false;

	// View 클래스 생성 시 View의 ID를 할당하는 함수.
	static unsigned int view_id_;
};
