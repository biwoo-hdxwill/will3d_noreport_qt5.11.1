#include "view.h"

#include <QDebug>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include <QOpenGLWidget>

#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Logger.h"

#include "../UIViewController/view_render_param.h"
#include "scene.h"

namespace
{
	const int kQOpenGLsamples = 4;
	const int kSliderWheelStep = 84;
	const int kSliderKeyPressedStep = 1;
}  // namespace

using namespace std;
using namespace UIViewController;

unsigned int View::view_id_ = 0;

View::View(const common::ViewTypeID& view_type, QWidget* parent)
	: view_type_(view_type), QGraphicsView(parent)
{
	++view_id_;
	this->setFrameShape(QFrame::NoFrame);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setRenderHint(QPainter::Antialiasing, true);
	this->setMouseTracking(true);

	scene_ = new Scene(this);
	this->setScene(scene_);

	connect(scene_, &Scene::sigRotateMatrix, this, &View::slotRotateMatrix);
	connect(scene_, &Scene::sigSliderValueChanged, this,
		&View::slotChangedValueSlider);
	connect(scene_, &Scene::sigActiveFilteredItem, this,
		&View::slotActiveFilteredItem);
	connect(scene_, &Scene::sigActiveSharpenItem, this,
		&View::slotActiveSharpenItem);

	connect(scene_, &Scene::sigGetProfileData, this,
		&View::slotGetProfileData);
	connect(scene_, &Scene::sigGetROIData, this, &View::slotGetROIData);

	pt_scene_center_ = GetSceneCenterPos();
	view_render_param_.reset(new ViewRenderParam());
	SetInitScale(1.0f);

	NewQOpenGLWidget();
}

View::~View() {}

/**=================================================================================================
public functions
*===============================================================================================**/
#ifndef WILL3D_VIEWER
void View::exportProject(float* scene_scale, float* scene_to_gl,
	QPointF* trans_gl)
{
	*scene_scale = view_render_param_->scene_scale();
	*scene_to_gl = view_render_param_->map_scene_to_gl();
	*trans_gl = view_render_param_->gl_trans();
}

void View::importProject(const float& scene_scale, const float& scene_to_gl,
	const QPointF& trans_gl,
	const bool& is_counterpart_exists,
	const bool& is_update_resource)
{
	view_render_param_->set_scene_scale(scene_scale);
	view_render_param_->set_gl_trans(trans_gl);
	view_render_param_->set_map_scene_to_gl(scene_to_gl);
	scene().SetMeasureParams(*view_render_param_);

#if 0
	if (is_counterpart_exists)
		scene().SyncMeasureResourceCounterparts(is_update_resource);
	else
		scene().ImportMeasureResource(is_update_resource);
#else
	scene().ImportMeasureResource(is_update_resource);
#endif
}
#endif
void View::SetRenderModeQuality(bool is_high_quality)
{
	if (is_high_quality)
		view_render_param_->SetRenderModeQuality();
	else
		view_render_param_->SetRenderModeFast();

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::UPDATE);
	this->ProcessViewEvent();
}

void View::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on)
{
	switch (type)
	{
	case common::CommonToolTypeOnce::V_RESET:
		ResetView();
		break;
	case common::CommonToolTypeOnce::V_FIT:
		FitView();
		break;
	case common::CommonToolTypeOnce::V_INVERT:
		InvertView();
		break;
	case common::CommonToolTypeOnce::V_GRID:
		SetGridOnOff(on);
		break;
	case common::CommonToolTypeOnce::V_HIDE_TXT:
		HideText(on);
		break;
	case common::CommonToolTypeOnce::V_HIDE_UI:
		HideAllUI(on);
		break;
	case common::CommonToolTypeOnce::M_HIDE_M:
		HideMeasure(on);
		break;
	case common::CommonToolTypeOnce::M_DEL_ALL:
		DeleteAllMeasure();
		break;
	case common::CommonToolTypeOnce::M_DEL_INCOMPLETION:
		DeleteUnfinishedMeasure();
		break;
	default:
		break;
	}
}
void View::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	common_tool_type_ = type;
	if (!is_right_button_clicked_)
	{
		DeleteUnfinishedMeasure();
	}
	scene_->SetMeasureType(type);

	// TODO.... CommonMenus에서 connect되어서 뷰가 활성화 안되어있는데도 시그널이
	// 날라옴.
	// 해결이 필요함.
	if (!isVisible()) return;

	SetEvent();
}

void View::SceneUpdate() { scene().update(); }
void View::SetZoomScale(float scene_scale)
{
	if (!is_view_ready_)
	{
		return;
	}

	float pre_scale = view_render_param_->scene_scale();
	if (std::fabs(pre_scale - scene_scale) <
		std::numeric_limits<float>::epsilon())
		return;

	view_render_param_->set_scene_scale(scene_scale);
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::UPDATE);
	this->ProcessViewEvent();

	this->ScaleItems(view_render_param_->scene_center(),
		view_render_param_->scene_center(), scene_scale / pre_scale);
}

void View::SetPanTranslate(const QPointF& gl_translate)
{
#if 0
	if (!is_view_ready_)
	{
		return;
	}
#endif

	if (-1 == view_render_param_->scene_size_width() || -1 == view_render_param_->scene_size_height())
	{
		return;
	}

	QPointF pre_scene_trans =
		view_render_param_->MapGLToScene(view_render_param_->gl_trans());

	if ((pre_scene_trans - gl_translate).manhattanLength() <
		std::numeric_limits<double>::epsilon())
		return;

	view_render_param_->set_gl_trans(gl_translate);
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::UPDATE);
	this->ProcessViewEvent();

	QPointF cur_scene_trans = view_render_param_->MapGLToScene(gl_translate);
	this->TranslateItems(pre_scene_trans - cur_scene_trans);
}

void View::SetGridOnOff(bool visible) { scene_->SetGridOnOff(visible); }

void View::setVisible(bool state)
{
	QGraphicsView::setVisible(state);
	scene().SetVisible(state);

	if (!state && IsEnableGLContext())
	{
		DeleteQOpenGLWidget();
	}
	else
	{
		NewQOpenGLWidget();
	}
}

void View::ResetView()
{
	if (isVisible()) ProcessResetEvent();
}

void View::FitView()
{
	if (isVisible()) ProcessFitEvent();
}

void View::InvertView()
{
	if (isVisible()) ProcessInvertEvent();
}

void View::DeleteUnfinishedMeasure() { scene_->DeleteUnfinishedMeasure(); }

void View::DeleteAllMeasure() { scene_->DeleteAllMeasure(); }

void View::HideText(bool is_hide)
{
	// HideUI 로 통일
}

void View::HideAllUI(bool is_hide) { scene_->HideAllUI(is_hide); }

void View::HideMeasure(bool is_hide) { scene_->HideMeasure(is_hide); }

/**=================================================================================================
protected functions
*===============================================================================================**/

void View::drawBackground(QPainter* painter, const QRectF& rect)
{
	QGraphicsView::drawBackground(painter, rect);

	if (!is_view_ready_)
	{
		painter->beginNativePainting();
		InitializeController();
		painter->endNativePainting();

		SetGraphicsItems();

		is_view_ready_ = true;
		SyncViewStatus();
	}
}

void View::resizeEvent(QResizeEvent* pEvent)
{
	QGraphicsView::resizeEvent(pEvent);

	if (!is_first_resize_)
	{
		this->fitInView(0, 0, this->width(), this->height());
		is_first_resize_ = true;
	}
	else
	{
		pt_scene_center_pre_ = pt_scene_center_;
		this->centerOn((double)this->width() * 0.5, (double)this->height() * 0.5);
	}

	pt_scene_center_ = GetSceneCenterPos();
	view_render_param_->SetViewPort(this->width(), this->height());
	view_render_param_->set_scene_center(pt_scene_center_);
	view_render_param_->set_scene_size(GetSceneSize());
	view_render_param_->set_scene_offset(mapToScene(QPoint(2, 2)) -
		mapToScene(QPoint(0, 0)));

	scene_->resizeEvent(*(view_render_param_));

	is_update_controller_ = true;

	if (is_first_resize_ && is_view_ready_)
	{
		ProcessResizeEvent();
	}
}

void View::SetGraphicsItems()
{
	scene_->SetViewRulerItem(*(view_render_param_));
	scene_->SetGridItem(*(view_render_param_));
	scene_->SetMeasureParams(*(view_render_param_));
}

void View::mousePressEvent(
	QMouseEvent* event /*, bool default_mouseevent_available*/)
{
	QGraphicsView::mousePressEvent(event);

	pt_scene_pressed_ = mapToScene(event->pos());
	this->SetMousePosCurr(pt_scene_pressed_);
	this->SetMousePosPrev(pt_scene_pressed_);

	is_pressed_ = true;
	if (event->button() == Qt::RightButton)
	{
		is_measure_delete_event_ = true;
		is_right_button_clicked_ = true;
	}

	if (scene_->IsMeasureInteractionAvailable(common_tool_type_) &&
		IsEventLeftButton(event))
	{
		scene_->MeasureMousePress(pt_scene_pressed_);
	}

	this->SetEvent();

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(), event->button(),
		event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);

	const QPointF& pt_scene = mapToScene(event->pos());
	this->SetMousePosCurr(pt_scene);
	this->SetMousePosPrev(pt_scene);

	if (event->buttons() == Qt::NoButton)
		is_pressed_ = false;
	else
		is_pressed_ = true;

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		// temp..
		emit sigZoomDone();
	}

	is_current_measure_available_ = scene_->IsMeasureInteractionAvailable(common_tool_type_);
	is_current_measure_available_ |= common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW;

	// 이벤트가 없어도 scene을 한번 다시 그려서 최신 상태를 유지하도록 함
	if (view_render_param_->event_type() == EVIEW_EVENT_TYPE::NO_EVENT)
	{
		View::SetViewEvent(EVIEW_EVENT_TYPE::UPDATE);
		ProcessViewEvent();
	}

	this->SetEvent();

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
		event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (!IsMouseOnViewItems()) CW3Cursor::SetViewCursor(common_tool_type_);

	/*if ((is_current_measure_available_ && is_measure_delete_event_) ||
		common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW)*/
	if (is_current_measure_available_ || common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW)
	{
		scene_->MeasureMouseRelease(event->button(), pt_scene);
	}

	is_measure_delete_event_ = false;
	is_current_measure_available_ = false;

	is_right_button_clicked_ = false;

	// event->ignore();
}

void View::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);

	pt_scene_current_ = mapToScene(event->pos());

	view_render_param_->set_scene_mouse_curr(pt_scene_current_);
	view_render_param_->set_scene_mouse_prev(pt_scene_prev_);
	view_render_param_->set_view_mouse_curr(event->pos());

	pt_scene_prev_ = pt_scene_current_;

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		is_measure_delete_event_ = false;
	}

	if (event->buttons() == Qt::RightButton)
	{
		is_measure_delete_event_ = false;
	}

	if (scene_->MeasureMouseMove(event->buttons(), common_tool_type_,
		pt_scene_current_))
		return;

	this->SetEvent();
}

void View::mouseDoubleClickEvent(QMouseEvent* event)
{
	QGraphicsView::mouseDoubleClickEvent(event);
	//if (IsEventLeftButton(event))
	scene_->MeasureMouseDoubleClick(event->button(), pt_scene_pressed_);

	if (event->button() == Qt::RightButton)
	{
		is_measure_delete_event_ = true;
		is_right_button_clicked_ = true;
	}
}

void View::leaveEvent(QEvent* event)
{
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void View::enterEvent(QEvent* event)
{
	QGraphicsView::enterEvent(event);
	setFocus();
	CW3Cursor::SetViewCursor(common_tool_type_);
}

void View::wheelEvent(QWheelEvent* event)
{
	int wheel_step = (event->delta() / kSliderWheelStep);
	if (IsReadyController()) scene_->EditSliderValue(wheel_step);
	event->ignore();
}

void View::keyPressEvent(QKeyEvent* event)
{
	QWidget::keyPressEvent(event);

	if (event->key() == Qt::Key_Up)
	{
		scene_->EditSliderValue(kSliderKeyPressedStep);
	}
	else if (event->key() == Qt::Key_Down)
	{
		scene_->EditSliderValue(-kSliderKeyPressedStep);
	}
}

void View::keyReleaseEvent(QKeyEvent* event)
{
	QWidget::keyReleaseEvent(event);
}

bool View::IsEnableGLContext() const
{
	if (gl_widget_.get() && gl_widget_->context())
		return true;
	else
		return false;
}

bool View::IsSetTool() const
{
	if (common_tool_type_ == common::CommonToolTypeOnOff::NONE &&
		!IsMouseOnViewItems())
		return false;
	else
		return true;
}

bool View::IsMouseOnViewItems() const { return scene_->IsMouseOnViewItems(); }

bool View::IsEventLeftButton(QMouseEvent* event) const
{
	return (event->buttons() == Qt::LeftButton ||
		event->button() == Qt::LeftButton)
		? true
		: false;
}

bool View::IsEventRightButton(QMouseEvent* event) const
{
	return (event->buttons() == Qt::RightButton ||
		event->button() == Qt::RightButton)
		? true
		: false;
}

bool View::IsEventLRButton(QMouseEvent* event) const
{
	return (event->buttons() == (Qt::RightButton | Qt::LeftButton) ||
		event->button() == (Qt::RightButton | Qt::LeftButton))
		? true
		: false;
}

bool View::MakeCurrent()
{
	if (gl_widget_->isValid())
	{
		gl_widget_->makeCurrent();
		return true;
	}
	else
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"View::MakeCurrent: The opengl context is invalid.");
		return false;
	}
}

bool View::DoneCurrent()
{
	if (gl_widget_->isValid())
	{
		gl_widget_->doneCurrent();
		return true;
	}
	else
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"View::MakeCurrent: The opengl context is invalid.");
		return false;
	}
}

void View::SetRenderModeFast() { view_render_param_->SetRenderModeFast(); }

void View::SetRenderModeQuality()
{
	view_render_param_->SetRenderModeQuality();
}

void View::SetViewEvent(const UIViewController::EVIEW_EVENT_TYPE& event)
{
	view_render_param_->SetEventType(event);
}

void View::UpdateDoneContoller() { is_update_controller_ = false; }

uint View::GetDefaultFrameBufferObject() const
{
	return (uint)gl_widget_->defaultFramebufferObject();
}

void View::SetViewRenderParam(
	const std::shared_ptr<ViewRenderParam>& view_render_param)
{
	view_render_param_ = view_render_param;
	scene_->SetViewRulerItem(*(View::view_render_param()));
	scene_->SetGridItem(*(View::view_render_param()));
}

void View::ProcessZoomWheelEvent(QWheelEvent* event)
{
	const float kSingleStep = 120.0f;
	float delta = (float)(event->delta()) / kSingleStep;
	view_render_param_->set_mouse_wheel_delta(delta);

	float pre_scale = view_render_param_->scene_scale();

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::ZOOM_WHEEL);
	ProcessViewEvent();

	float cur_scale = view_render_param_->scene_scale();
	this->ScaleItems(view_render_param_->scene_center(),
		view_render_param_->scene_center(), cur_scale / pre_scale);

	emit sigProcessedZoomEvent(cur_scale);

	view_render_param_->set_mouse_wheel_delta(0.0f);

	// temp..
	emit sigZoomDone();
}

bool View::IsUpdateController() const
{
	return (is_update_controller_ && this->IsEnableGLContext());
}

/**=================================================================================================
private functions
*===============================================================================================**/

void View::NewQOpenGLWidget()
{
	if (gl_widget_.get() != nullptr) return;

	gl_widget_.reset(new QOpenGLWidget(this));

	QSurfaceFormat format;
	format.setSamples(kQOpenGLsamples);
	gl_widget_->setFormat(format);

	setViewport(gl_widget_.get());
}

void View::DeleteQOpenGLWidget()
{
	if (gl_widget_.get() == nullptr || gl_widget_->context() == nullptr) return;

	gl_widget_->makeCurrent();
	this->ClearGL();
	gl_widget_->doneCurrent();

	// setViewport(nullptr)을 하면 qt가 자동으로 gl_widget의 소멸자를 호출하므로
	// reset()으로 삭제하면 안된다.
	gl_widget_.release();

	setViewport(nullptr);

	is_update_controller_ = true;
}

void View::ProcessViewEvent()
{
	ActiveControllerViewEvent();

	if (IsReadyController())
	{
		scene_->update();
		ClearViewEvent();
	}
}

void View::ClearViewEvent()
{
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::NO_EVENT);
}

QPointF View::GetSceneCenterPos()
{
	QPointF left_top = mapToScene(QPoint(0, 0));
	QPointF right_bottom =
		mapToScene(QPoint(this->width() - 1, this->height() - 1));

	return QPointF((right_bottom.x() + left_top.x()) * 0.5f,
		(right_bottom.y() + left_top.y()) * 0.5f);
}

QSize View::GetSceneSize()
{
	QPointF left_top = mapToScene(QPoint(0, 0));
	QPointF right_bottom =
		mapToScene(QPoint(this->width() - 1, this->height() - 1));

	return QSize(abs(right_bottom.x() - left_top.x()),
		abs(right_bottom.y() - left_top.y()));
}

void View::TranslateItems(const QPointF& translate)
{
	QTransform transform;
	transform.translate(translate.x(), translate.y());
	TransformPositionItems(transform, true);

#if 1
	scene_->SetMeasureParamTrans(view_render_param_->MapGLToScene(view_render_param_->gl_trans()));
#else
	scene_->SetMeasureParamTrans(view_render_param_->gl_trans());
#endif
}

void View::ScaleItems(const QPointF& pre_view_center_in_scene,
	const QPointF& cur_view_center_in_scene, float scale)
{
	QTransform transform;
	transform.translate(cur_view_center_in_scene.x(),
		cur_view_center_in_scene.y());
	transform.scale(scale, scale);
	transform.translate(-pre_view_center_in_scene.x(),
		-pre_view_center_in_scene.y());

	TransformPositionItems(transform);

	scene_->SetViewRulerItem(*(view_render_param_));
	scene_->SetGridItem(*(view_render_param_));
	//scene_->SetMeasureParamScale(0.5f * view_render_param_->map_scene_to_gl());
	scene_->SetMeasureParamZoomFactor(view_render_param_->scene_scale());
}

void View::TransformPositionItems(const QTransform& transform, const bool translate)
{
	TransformItems(transform);
	scene_->TransformMeasure(transform, translate);

#if 1
	scene_->SetMeasureParamTrans(view_render_param_->MapGLToScene(view_render_param_->gl_trans()));
	scene_->SetMeasureParamScale(0.5f * view_render_param_->map_scene_to_gl());
	scene_->SetMeasureParamZoomFactor(view_render_param_->scene_scale());
#endif
}

void View::SetEvent()
{
	if (!isVisible()) return;

	switch (common_tool_type_)
	{
	case common::CommonToolTypeOnOff::V_PAN:
	case common::CommonToolTypeOnOff::V_PAN_LR:
		if (is_pressed_)
			ProcessPanEvent();
		else
			ClearViewEvent();
		break;
	case common::CommonToolTypeOnOff::V_ZOOM:
	case common::CommonToolTypeOnOff::V_ZOOM_R:
		if (is_pressed_)
			ProcessZoomEvent();
		else
			ClearViewEvent();
		break;
	case common::CommonToolTypeOnOff::V_LIGHT:
		if (is_pressed_)
			ProcessLightEvent();
		else
			ClearViewEvent();
		break;
	case common::CommonToolTypeOnOff::M_RULER:
	case common::CommonToolTypeOnOff::M_TAPELINE:
	case common::CommonToolTypeOnOff::M_TAPECURVE:
	case common::CommonToolTypeOnOff::M_ANGLE:
	case common::CommonToolTypeOnOff::M_PROFILE:
	case common::CommonToolTypeOnOff::M_AREALINE:
	case common::CommonToolTypeOnOff::M_ROI:
	case common::CommonToolTypeOnOff::M_RECTANGLE:
	case common::CommonToolTypeOnOff::M_CIRCLE:
	case common::CommonToolTypeOnOff::M_ARROW:
	case common::CommonToolTypeOnOff::M_LINE:
	case common::CommonToolTypeOnOff::M_FREEDRAW:
	case common::CommonToolTypeOnOff::M_NOTE:
	case common::CommonToolTypeOnOff::M_DEL:
		ProcessMeasureEvent();
		break;
	case common::CommonToolTypeOnOff::NONE:
		ProcessDefaultEvent();
		break;
	default:
		assert(false);
		break;
	}
}

void View::ProcessDefaultEvent()
{
	if (view_render_param_->event_type() != EVIEW_EVENT_TYPE::NO_EVENT)
	{
		ProcessViewEvent();
	}
}

void View::ProcessResizeEvent()
{
	float map_scene_to_gl_pre = view_render_param_->map_scene_to_gl();

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::UPDATE);
	this->ProcessViewEvent();

	float map_scene_to_gl_cur = view_render_param_->map_scene_to_gl();

	if (fabsf(map_scene_to_gl_pre) < 0.0001f && fabsf(map_scene_to_gl_cur) < 0.0001f)
	{
		return;
	}

	this->ScaleItems(pt_scene_center_pre_, pt_scene_center_, map_scene_to_gl_pre / map_scene_to_gl_cur);
}
void View::ProcessResetEvent()
{
	ProcessFitEvent();

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::RESET);
	ProcessViewEvent();
}

void View::ProcessPanEvent()
{
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::PAN);
	ProcessViewEvent();

	const QPointF& pos_curr = view_render_param_->scene_mouse_curr();
	const QPointF& pos_prev = view_render_param_->scene_mouse_prev();

	QPointF scene_trans = pos_curr - pos_prev;

	this->TranslateItems(scene_trans);

	emit sigProcessedPanEvent(view_render_param_->gl_trans());
}

void View::ProcessZoomEvent()
{
	float pre_scale = view_render_param_->scene_scale();

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::ZOOM);
	ProcessViewEvent();

	float cur_scale = view_render_param_->scene_scale();
	this->ScaleItems(view_render_param_->scene_center(),
		view_render_param_->scene_center(), cur_scale / pre_scale);

	emit sigProcessedZoomEvent(cur_scale);
}

void View::ProcessFitEvent()
{
	float pre_scale = view_render_param_->scene_scale();
	QPointF view_trans =
		view_render_param_->MapGLToScene(view_render_param_->gl_trans());

	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::FIT);
	ProcessViewEvent();

	float cur_scale = view_render_param_->scene_scale();

	this->TranslateItems(view_trans);
	this->ScaleItems(view_render_param_->scene_center(),
		view_render_param_->scene_center(), cur_scale / pre_scale);
}

void View::ProcessInvertEvent()
{
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::UPDATE);
	ProcessViewEvent();
}

void View::ProcessLightEvent()
{
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::LIGHT);
	ProcessViewEvent();
	emit sigProcessedLightEvent();
}

void View::ProcessMeasureEvent()
{
	view_render_param_->SetEventType(EVIEW_EVENT_TYPE::MEASUREMENT);
	ProcessViewEvent();
}

void View::SetMousePosCurr(const QPointF& pt_scene)
{
	pt_scene_current_ = pt_scene;
	view_render_param_->set_scene_mouse_curr(pt_scene);
}

void View::SetMousePosPrev(const QPointF& pt_scene)
{
	pt_scene_prev_ = pt_scene;
	view_render_param_->set_scene_mouse_prev(pt_scene);
}

void View::SetDisableMeasureEvent()
{
	is_current_measure_available_ = false;
}

void View::SyncViewStatus()
{
	if (!View::IsEnableGLContext()) return;

	DoneCurrent();
	float scene_scale = std::numeric_limits<float>::min();
	QPointF gl_trans(std::numeric_limits<double>::min(),
		std::numeric_limits<double>::min());
	QPointF comp_gl_trans = gl_trans;
	emit sigRequestSyncViewStatus(&scene_scale, &gl_trans);
	if (std::fabsf(scene_scale - std::numeric_limits<float>::min()) >
		std::numeric_limits<float>::epsilon())
	{
		SetZoomScale(scene_scale);
	}
	if ((gl_trans - comp_gl_trans).manhattanLength() >
		std::numeric_limits<double>::epsilon())
	{
		SetPanTranslate(gl_trans);
	}
	MakeCurrent();
}

void View::ChangedViewRulerItem(float old_view_len_in_scene,
	float changed_length)
{
	float new_view_width_in_scene =
		view_render_param_->MapActualToScene(changed_length);

	float view_scale = view_render_param_->scene_scale() *
		(old_view_len_in_scene / new_view_width_in_scene);
	view_render_param_->set_scene_scale(view_scale);

	ProcessZoomEvent();
}

void View::ApplyPreferences()
{
	scene_->ApplyPreferences();
}

const float& View::GetSceneScale() const
{
	return view_render_param_->scene_scale();
}

const QPointF& View::GetGLTranslate() const
{
	return view_render_param_->gl_trans();
}

const bool View::IsTextVisible() const { return scene_->IsTextVisible(); }

const bool View::IsUIVisible() const { return scene_->IsUIVisible(); }

bool View::IsMeasureInteractionAvailable() { return false; }

void View::SyncMeasureResourceSiblings(const bool& is_update_resource)
{
	scene_->SyncMeasureResourceSiblings(is_update_resource);
}

void View::SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform)
{
	scene_->SyncMeasureResourceCounterparts(is_update_resource, need_transform);
}

void View::SyncCreateMeasureUI(const unsigned int& measure_id)
{
	scene_->SyncCreateMeasureUI(measure_id);
}

void View::SyncDeleteMeasureUI(const unsigned int& measure_id)
{
	scene_->SyncDeleteMeasureUI(measure_id);
}

void View::SyncModifyMeasureUI(const unsigned int& measure_id)
{
	scene_->SyncModifyMeasureUI(measure_id);
}

void View::GetMeasureParams(const common::ViewTypeID& view_type,
	const unsigned int& measure_id,
	common::measure::VisibilityParams* measure_params)
{
	scene_->GetMeasureParams(view_type, measure_id, measure_params);
}

void View::SetInitScale(const float scale)
{
	view_render_param_->SetInitScale(scale);
}
