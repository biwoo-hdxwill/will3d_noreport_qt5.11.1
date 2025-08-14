#include "view_tmj_3d.h"

#include <QDebug>
#include <QMouseEvent>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Define.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/tmj_resource.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIViewController/view_render_param.h"
#include "../UIViewController/view_controller_tmj3d.h"

#include "scene.h"

using namespace UIViewController;

ViewTMJ3D::ViewTMJ3D(const TMJDirectionType& type, QWidget *parent)
	: View3D(common::ViewTypeID::IMPLANT_3D, parent), direction_type_(type) {
	controller_.reset(new ViewControllerTMJ3D(type));
	controller_->set_view_param(view_render_param());


	scene().InitViewItem(Viewitems::ALIGN_TEXTS);
	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::GRID);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitMeasure(view_type());

	scene().SetRulerColor(ColorView::k3D);
	scene().SetBorderColor(ColorView::k3D);

	scene().SetMeasureReconType(common::ReconTypeID::VR);
}

ViewTMJ3D::~ViewTMJ3D() {
	if (View::IsEnableGLContext()) {
		View::MakeCurrent();
		ClearGL();
		View::DoneCurrent();
	}
}
void ViewTMJ3D::UpdateCutting(const int& curr_step) {
	controller_->UpdateCutting(curr_step);
	
	UpdateVolume();
}
void ViewTMJ3D::UpdateVR(bool is_high_quality) {

	if (is_high_quality)
		view_render_param()->SetRenderModeQuality();
	else
		view_render_param()->SetRenderModeFast();

	UpdateVolume();
}
void ViewTMJ3D::ResetVolume() {
	if (View::IsEnableGLContext()) {
		View::MakeCurrent();
		ClearGL();
		View::DoneCurrent();
	}

	View::SetViewEvent(EVIEW_EVENT_TYPE::UPDATE);
	ActiveControllerViewEvent();

	scene().update();
}

void ViewTMJ3D::UpdateFrontal() {
	if (!View::IsEnableGLContext())
		return;

	View::MakeCurrent();
	controller_->ClearGL();
	controller_->InitVAOVBOROIVolume();
	View::DoneCurrent();

	View::SetViewEvent(EVIEW_EVENT_TYPE::UPDATE);
	ActiveControllerViewEvent();

	scene().SetViewRulerItem(*(view_render_param()));

	scene().update();
}

void ViewTMJ3D::SetCutting(const bool & cut_enable) {
	controller_->SetCutting(cut_enable);
	UpdateVolume();
}

void ViewTMJ3D::UpdateVolume() {
	this->RenderVolume();
	scene().update();
}

void ViewTMJ3D::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type) {
	if (type == common::CommonToolTypeOnOff::V_ZOOM_R) {
		return;
	}

	View::SetCommonToolOnOff(type);
}

void ViewTMJ3D::slotRotateMatrix(const glm::mat4& mat) {
	controller_->ForceRotateMatrix(mat);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());

	UpdateVolume();
}

void ViewTMJ3D::drawBackground(QPainter * painter, const QRectF & rect) {
	View::drawBackground(painter, rect);

	if (!controller_->IsReady()) {
		if (controller_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}
	painter->beginNativePainting();

	if (IsUpdateController()) {
		controller_->ClearGL();
		controller_->InitVAOVBOROIVolume();
		controller_->RenderingVolume();
		UpdateDoneContoller();
		scene().SetViewRulerItem(*(view_render_param()));
	}

	controller_->RenderScreen(View::GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

void ViewTMJ3D::resizeEvent(QResizeEvent* pEvent) 
{
	View::resizeEvent(pEvent);
	controller_->SetProjection();
}

void ViewTMJ3D::mouseMoveEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (is_pressed())
		View::SetRenderModeFast();

	DisplayDICOMInfo(mapToScene(event->pos()));

	View::mouseMoveEvent(event);

	if (IsEventRightButton(event) &&
		!View::IsSetTool()) {
		View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
	}
}

void ViewTMJ3D::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	View3D::mousePressEvent(event);
}

void ViewTMJ3D::mouseReleaseEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		if (event->button() == Qt::RightButton)
		{
			emit sigShowButtonListDialog(event->globalPos());
			return;
		}
	}
#endif // WILL3D_EUROPE

	View::SetRenderModeQuality();	
	View::mouseReleaseEvent(event);
}

void ViewTMJ3D::leaveEvent(QEvent * event) {
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void ViewTMJ3D::enterEvent(QEvent * event) {
	View::enterEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}
void ViewTMJ3D::wheelEvent(QWheelEvent * event) {
	View::wheelEvent(event);
	View::ProcessZoomWheelEvent(event);
}

void ViewTMJ3D::InitializeController() {
	controller_->Initialize();
	controller_->InitVAOVBOROIVolume();

	if (controller_->IsReady())
		controller_->RenderingVolume();

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

bool ViewTMJ3D::IsReadyController() {
	return controller_->IsReady();
}

void ViewTMJ3D::TransformItems(const QTransform & transform) {
	View::TransformItems(transform);
}

void ViewTMJ3D::ClearGL() {
	controller_->ClearGL();
}

void ViewTMJ3D::ActiveControllerViewEvent() {
	EVIEW_EVENT_TYPE event_type = view_render_param()->event_type();

	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);
	if (need_render) {
		this->RenderVolume();
	}
}

void ViewTMJ3D::RenderVolume() {
	if (!View::IsEnableGLContext())
		return;

	if (controller_->IsReady()) {
		View::MakeCurrent();
		controller_->RenderingVolume();
		View::DoneCurrent();
	}
}

void ViewTMJ3D::DisplayDICOMInfo(const QPointF & pt_scene) {
	glm::vec4 vol_info;
	if (View::IsEnableGLContext() && controller_->IsReady()) {
		View::MakeCurrent();
		glm::vec3 vol_pos = controller_->MapSceneToVol(pt_scene);
		vol_info = ResourceContainer::GetInstance()->GetMainVolume().GetVolumeInfo(vol_pos);
		View::DoneCurrent();
	} else {
		return;
	}

	float window_width, window_level;
	controller_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_->GetIntercept());
	if (vol_info.w != common::dicom::kInvalidHU) {
		scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
						   .arg(wl).arg(ww)
						   .arg(static_cast<int>(vol_info.x))
						   .arg(static_cast<int>(vol_info.y))
						   .arg(static_cast<int>(vol_info.z)).arg(vol_info.w));
	} else {
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)")
						   .arg(wl).arg(ww));
	}
}

BaseViewController3D* ViewTMJ3D::controller_3d()
{
	return (BaseViewController3D*)controller_.get();
}

void ViewTMJ3D::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	View3D::keyPressEvent(event);
}

void ViewTMJ3D::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	View3D::keyReleaseEvent(event);
}

void ViewTMJ3D::SetWorldAxisDirection()
{
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}
