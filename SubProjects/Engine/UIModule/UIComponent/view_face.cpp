#include "view_face.h"

#include <QMenu>
#include <QAction>
#include <QMouseEvent>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/color_will3d.h"

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/face_photo_resource.h>

#include "../UIViewController/view_render_param.h"
#include "../UIViewController/view_controller_face3d.h"

#include "scene.h"

using namespace UIViewController;

ViewFace::ViewFace(QWidget* parent)
	: View3D(common::ViewTypeID::FACE_BEFORE, parent)
{
	controller_.reset(new ViewControllerFace3D);
	controller_->set_view_param(view_render_param());

	scene().InitViewItem(Viewitems::ALIGN_TEXTS);
	scene().InitViewItem(Viewitems::NAVIGATION);

	scene().InitViewItem(Viewitems::RULER);
	scene().SetRulerColor(ColorView::k3D);

	scene().InitViewItem(Viewitems::BORDER);
	scene().SetBorderColor(ColorView::k3D);
	scene().InitMeasure(view_type());

	menu_.reset(new QMenu());
	action_save_3d_face_to_ply_.reset(new QAction("Export 3d face to PLY file."));
	action_save_3d_face_to_obj_.reset(new QAction("Export 3d face to OBJ file."));
	menu_->addAction(action_save_3d_face_to_ply_.get());
	menu_->addAction(action_save_3d_face_to_obj_.get());
	connect(action_save_3d_face_to_ply_.get(), SIGNAL(triggered()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(action_save_3d_face_to_obj_.get(), SIGNAL(triggered()), this, SIGNAL(sigSave3DFaceToOBJFile()));

	view_render_param()->set_scene_scale(sqrt(2.0f));
	scene().SetMeasureReconType(common::ReconTypeID::VR);
	setVisible(false);
}

ViewFace::~ViewFace()
{
	ClearGL();
}
/**=================================================================================================
public functions
*===============================================================================================**/

void ViewFace::UpdateVRview(bool is_high_quality)
{
	if (is_high_quality)
		view_render_param()->SetRenderModeQuality();
	else
		view_render_param()->SetRenderModeFast();

	RenderVolume();
	scene().update();
}

void ViewFace::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type)
{
	if (type == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		return;
	}

	View::SetCommonToolOnOff(type);
}

void ViewFace::SetTransparencyFacePhoto(float alpha)
{
	controller_->SetTransparencySurfaceFace(alpha);
	RenderVolume();
	scene().update();
}
void ViewFace::SetVisibleFacePhoto(bool isVisible)
{
	controller_->SetVisibleSurfaceFace(isVisible);
	RenderVolume();
	scene().update();
}
void ViewFace::ForceRotateMatrix(const glm::mat4 & mat)
{
	controller_->ForceRotateMatrix(mat);
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
	View::SetRenderModeFast();
	RenderVolume();
	scene().update();
}

void ViewFace::LoadFace3D()
{
	controller_->LoadFace3D();
}

/**=================================================================================================
protected slots
*===============================================================================================**/

void ViewFace::slotRotateMatrix(const glm::mat4 & mat)
{
	controller_->ForceRotateMatrix(mat);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());

	emit sigRotateMat(controller_->GetRotateMatrix());

	RenderVolume();
	scene().update();
}

/**=================================================================================================
private functions
*===============================================================================================**/

void ViewFace::drawBackground(QPainter * painter, const QRectF & rect)
{
	View::drawBackground(painter, rect);

	if (!controller_->IsReady())
		return;

	if (IsUpdateController())
	{
		controller_->RenderingVolume();
		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	controller_->RenderScreen(View::GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

void ViewFace::resizeEvent(QResizeEvent * pEvent)
{
	View::resizeEvent(pEvent);
	controller_->SetProjection();

	RenderVolume();
}

void ViewFace::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	View::mousePressEvent(event);

	if (IsEventRightButton(event) && ResourceContainer::GetInstance()->GetFacePhotoResource().IsSetFace())
	{
		show_export_3d_face_menu_ = true;
	}
}

void ViewFace::mouseMoveEvent(QMouseEvent * event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	show_export_3d_face_menu_ = false;

	if (is_pressed())
		View::SetRenderModeFast();

	View::mouseMoveEvent(event);

	if (IsEventRightButton(event)/* &&!View::IsSetTool()*/)
	{
		View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());

		emit sigRotateMat(controller_->GetRotateMatrix());
	}
}

void ViewFace::mouseReleaseEvent(QMouseEvent * event)
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
	emit sigRenderQuality();

	if (show_export_3d_face_menu_)
	{
		menu_->popup(mapToGlobal(event->pos()));
	}
}

void ViewFace::wheelEvent(QWheelEvent * event)
{
	View::wheelEvent(event);
	View::ProcessZoomWheelEvent(event);
}

void ViewFace::InitializeController()
{
	controller_->Initialize();

	if (controller_->IsReady())
		controller_->RenderingVolume();

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

bool ViewFace::IsReadyController()
{
	return controller_->IsReady();
}

void ViewFace::TransformItems(const QTransform & transform) {}

void ViewFace::ClearGL()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		controller_->ClearGL();
		View::DoneCurrent();
	}
}

void ViewFace::ActiveControllerViewEvent()
{
	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);

	if (need_render)
	{
		this->RenderVolume();
	}
}
void ViewFace::RenderVolume()
{
	if (!View::IsEnableGLContext())
		return;

	View::MakeCurrent();
	controller_->RenderingVolume();
	View::DoneCurrent();
}

BaseViewController3D* ViewFace::controller_3d()
{
	return (BaseViewController3D*)controller_.get();
}

void ViewFace::keyPressEvent(QKeyEvent* event)
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

	emit sigRotateMat(controller_->GetRotateMatrix());
}

void ViewFace::keyReleaseEvent(QKeyEvent* event)
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
