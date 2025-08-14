#include "base_view_orientation.h"

#include <QDebug>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtx/transform2.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtx/transform2.hpp>
#endif

#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../UIViewController/view_render_param.h"
#include "../UIViewController/view_controller_orientation.h"
#include "../../Module/Renderer/volume_renderer.h"

#include "../UIPrimitive/pano_orientatoin_roi_item.h"

#include <Engine/UIModule/UIViewController/base_transform.h>

#include "scene.h"

using namespace UIViewController;

BaseViewOrientation::BaseViewOrientation(
	const common::ViewTypeID& view_type,
	const ReorientViewID& type,
	const bool enable_roi,
	QWidget* parent
)
	: View3D(view_type, parent), type_(type)
{
	controller_.reset(new ViewControllerOrientation(type_));
	switch (type)
	{
	case ReorientViewID::ORIENT_A:
		rotate_img_.reset(new QGraphicsPixmapItem(QPixmap(":/image/orientation/orientation-arrow.png")));
		break;
	case ReorientViewID::ORIENT_R:
		rotate_img_.reset(new QGraphicsPixmapItem(QPixmap(":/image/orientation/orientation-arrow2.png")));

		roi_item_.reset(new PanoOrientationROIItem);
		if (enable_roi)
		{
			scene().addItem(roi_item_.get());
		}
		else
		{
			roi_item_->setVisible(false);
		}
		connect(roi_item_.get(), SIGNAL(sigTranslateLine(int, float)), this, SLOT(slotTranslateROILine(int, float)));
		connect(roi_item_.get(), SIGNAL(sigMouseReleased()), this, SIGNAL(sigUpdateDoneROI()));
		break;
	case ReorientViewID::ORIENT_I:
		rotate_img_.reset(new QGraphicsPixmapItem(QPixmap(":/image/orientation/orientation-arrow.png")));
		break;
	default:
		break;
	}

	connect(&timer_, &QTimer::timeout, this, &BaseViewOrientation::slotUpdate);
	timer_.setSingleShot(true);
	timer_.setInterval(250);

	controller_->set_view_param(view_render_param());

	view_render_param()->set_scene_scale(2.0f);
	scene().InitViewItem(Viewitems::GRID);
	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().addItem(rotate_img_.get());

	SetGridOnOff(true);
}

BaseViewOrientation::~BaseViewOrientation()
{
	ClearGL();
}
/**=================================================================================================
public functions
*===============================================================================================**/
void BaseViewOrientation::ReadyROILines(float top, float slice, float bottom)
{
	roi_vol_.top = top;
	roi_vol_.slice = slice;
	roi_vol_.bottom = bottom;
	is_ready_roi_ = true;
}

void BaseViewOrientation::SetSliceInVol(float z_pos_vol)
{
	if (!is_ready_roi_ || !is_init_roi_lines_)
		return;

	const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (&vol == nullptr)
	{
		common::Logger::instance()->Print(common::LogType::DBG, "not ready volume..");
		return;
	}

	if (roi_vol_.slice == (int)z_pos_vol)
		return;

	roi_vol_.slice = (int)z_pos_vol;

	QPointF scene_center = view_render_param()->scene_center();
	QSize scene_size = view_render_param()->scene_size();

	if (scene_size == QSize())
		return;

	double slice_pos = (double)view_render_param()->MapVolToScene(roi_vol_.slice - vol.depth() / 2);

	roi_item_->SetROILine(PanoOrientationROIItem::EROI::EROI_MID,
		scene_center.x() - (scene_size.width() / 2), scene_center.x() + (scene_size.width() / 2),
		slice_pos + scene_center.y());
}

void BaseViewOrientation::SetOrientationAngle(int degree, bool sign)
{
	int prev_degree = GetAngleDegree();

	glm::mat4 rot_mat;
	if (sign)
		rot_mat = GetRotateMatrix(degree - prev_degree)*controller_->GetRotateMatrix();
	else
		rot_mat = GetRotateMatrix(prev_degree - degree)*controller_->GetRotateMatrix();

	controller_->ForceRotateMatrix(rot_mat);
	controller_->SetRotateAngle(degree + 180);

	if (isVisible())
	{
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
		this->RenderVolume();
		scene().update();
	}
	else
	{
		is_changed_navigator_ = true;
	}

	emit sigRotateMat(type_, rot_mat);

	timer_.start();
}

void BaseViewOrientation::SetRotateMatrix(const glm::mat4 & mat)
{
	controller_->ForceRotateMatrix(mat);

	if (isVisible())
	{
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
		View::SetRenderModeFast();
		this->RenderVolume();
		scene().update();
	}
	else
	{
		is_changed_navigator_ = true;
	}
}

void BaseViewOrientation::SetRenderQuality()
{
	if (!isVisible())
		return;

	View::SetRenderModeQuality();
	this->RenderVolume();
	scene().update();
}

float BaseViewOrientation::GetSliceInVol() const
{
	return roi_vol_.slice;
}

float BaseViewOrientation::GetTopInVol() const
{
	return roi_vol_.top;
}

float BaseViewOrientation::GetBottomInVol() const
{
	return roi_vol_.bottom;
}

BaseViewController3D* BaseViewOrientation::controller_3d()
{
	return (BaseViewController3D*)controller_.get();
}

void BaseViewOrientation::SetAngleDegree(int degree)
{
	controller_->SetRotateAngle(degree + 180);
}
int BaseViewOrientation::GetAngleDegree()
{
	int degree = abs(((int)controller_->GetRotateAngle()) % 360);
	if (degree > 360)
		degree -= 360;
	else if (degree < 0)
		degree += 360;

	return degree - 180;
}

/**=================================================================================================
protected slots
*===============================================================================================**/

void BaseViewOrientation::slotRotateMatrix(const glm::mat4 & mat)
{
	controller_->ForceRotateMatrix(mat);
	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
	scene().update();
}

/**=================================================================================================
private functions
*===============================================================================================**/
void BaseViewOrientation::InitROILines()
{
	const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (&vol == nullptr)
	{
		common::Logger::instance()->Print(common::LogType::DBG, "not ready volume..");
		return;
	}

	double scene_slice_loc[PanoOrientationROIItem::EROI::EROI_LINE_END] = {
		(double)view_render_param()->MapVolToScene(roi_vol_.top - vol.depth() / 2),
		(double)view_render_param()->MapVolToScene(roi_vol_.bottom - vol.depth() / 2),
		(double)view_render_param()->MapVolToScene(roi_vol_.slice - vol.depth() / 2) };

	QPointF scene_center = view_render_param()->scene_center();
	QSize scene_size = view_render_param()->scene_size();

	for (int i = 0; i < PanoOrientationROIItem::EROI::EROI_LINE_END; i++)
	{
		roi_item_->SetROILine((PanoOrientationROIItem::EROI)i,
			scene_center.x() - (scene_size.width() / 2), scene_center.x() + (scene_size.width() / 2),
			scene_slice_loc[i] + scene_center.y());
	}
	roi_item_->SetMinMax(
		-(double)view_render_param()->MapVolToScene(vol.depth() / 2) + scene_center.y(),
		(double)view_render_param()->MapVolToScene(vol.depth() / 2) + scene_center.y());
	is_init_roi_lines_ = true;
}

void BaseViewOrientation::drawBackground(QPainter * painter, const QRectF & rect)
{
	View::drawBackground(painter, rect);

	if (!controller_->IsReady())
		return;

	if (is_changed_navigator_)
	{
		View::DoneCurrent();
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
		is_changed_navigator_ = false;
		View::MakeCurrent();
	}

	painter->beginNativePainting();

	if (IsUpdateController())
	{
		controller_->RenderingVolume();
		UpdateDoneContoller();
	}

	controller_->RenderScreen(View::GetDefaultFrameBufferObject());

	painter->endNativePainting();
}

void BaseViewOrientation::resizeEvent(QResizeEvent * pEvent)
{
	View::resizeEvent(pEvent);

	if (type_ == ReorientViewID::ORIENT_R)
	{
		QPointF left_top = mapToScene(0, 0);
		QPointF pos_rot_img = left_top
			+ QPointF(this->width(), this->height() / 2)
			- QPointF(rotate_img_->pixmap().size().width(), rotate_img_->pixmap().size().height() / 2);
		rotate_img_->setPos(pos_rot_img.x(), pos_rot_img.y());
	}
	else
	{
		QPointF left_top = mapToScene(0, 0);
		QPointF pos_rot_img = left_top
			+ QPointF(this->width() / 2, this->height() / 8)
			- QPointF(rotate_img_->pixmap().size().width() / 2, 0.0);
		rotate_img_->setPos(pos_rot_img.x(), pos_rot_img.y());
	}

	controller_->SetProjection();
}

void BaseViewOrientation::mouseMoveEvent(QMouseEvent * event)
{
	if (is_pressed())
		View::SetRenderModeFast();

	if (IsEventRightButton(event))
	{
		View::SetViewEvent(EVIEW_EVENT_TYPE::ROTATE);
		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
		View::mouseMoveEvent(event);

		emit sigRotateMat(type_, controller_->GetRotateMatrix());
	}
	else
	{
		if (IsHoveredROIitem())
			return QGraphicsView::mouseMoveEvent(event);

		View::mouseMoveEvent(event);
	}
}

void BaseViewOrientation::mousePressEvent(QMouseEvent * event)
{
	View::mousePressEvent(event);
}
void BaseViewOrientation::mouseReleaseEvent(QMouseEvent * event)
{
	View::SetRenderModeQuality();
	View::mouseReleaseEvent(event);

	if (IsHoveredROIitem())
		return;

	emit sigRenderQuality(type_);
	emit sigUpdateDoneROI();
}

void BaseViewOrientation::keyPressEvent(QKeyEvent* event)
{
#if 0
	View3D::keyPressEvent(event);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
	RenderVolume();

	emit sigRotateMat(type_, controller_->GetRotateMatrix());

	emit sigRenderQuality(type_);
	emit sigUpdateDoneROI();

	SceneUpdate();
#else
	View::keyPressEvent(event);

	if (!View::IsEnableGLContext())
		return;

	float rotate_direction = 1.0f;

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up)
	{
		rotate_direction = -1.0f;
	}

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
#if 1
		int prev_degree = GetAngleDegree();

		glm::mat4 rot_mat = GetRotateMatrix(rotate_direction) * controller_->GetRotateMatrix();

		controller_->ForceRotateMatrix(rot_mat);
		controller_->SetRotateAngle(prev_degree + rotate_direction + 180);
#else
		switch (type_)
		{
		case ReorientViewID::ORIENT_A:
			controller()->transform().Rotate(rotate_direction, glm::vec3(0.0f, 1.0f, 0.0f));
			break;
		case ReorientViewID::ORIENT_R:
			controller()->transform().Rotate(rotate_direction, glm::vec3(-1.0f, 0.0f, 0.0f));
			break;
		case ReorientViewID::ORIENT_I:
			controller()->transform().Rotate(rotate_direction, glm::vec3(0.0f, 0.0f, -1.0f));
			break;
		default:
			break;
	}
#endif

		scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
		RenderVolume();

		emit sigRotateMat(type_, controller_->GetRotateMatrix());

		emit sigRenderQuality(type_);
		emit sigUpdateDoneROI();

		SceneUpdate();
	}
#endif
}

void BaseViewOrientation::InitializeController()
{
	controller_->Initialize();

	if (controller_->IsReady())
		controller_->RenderingVolume();

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(), controller_->GetViewMatrix());
}

bool BaseViewOrientation::IsReadyController()
{
	return controller_->IsReady();
}

void BaseViewOrientation::TransformItems(const QTransform & transform)
{
	View::TransformItems(transform);
}

void BaseViewOrientation::ClearGL()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		controller_->ClearGL();
		is_cleared_gl_ = true;
		View::DoneCurrent();
	}
}

void BaseViewOrientation::ActiveControllerViewEvent()
{
	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);
	if (need_render)
		this->RenderVolume();
}
void BaseViewOrientation::RenderVolume()
{
	if (!View::IsEnableGLContext())
		return;

	View::MakeCurrent();
	controller_->RenderingVolume();
	View::DoneCurrent();
}

void BaseViewOrientation::SetGraphicsItems()
{
	View::SetGraphicsItems();

	if (type_ != ReorientViewID::ORIENT_R)
		return;

	if (!is_init_roi_lines_)
	{
		if (!is_ready_roi_)
		{
			common::Logger::instance()->Print(common::LogType::DBG, "not ready roi..");
			//assert(false);
			return;
		}
		InitROILines();
	}
}

glm::mat4 BaseViewOrientation::GetRotateMatrix(int degree)
{
	switch (type_)
	{
	case ReorientViewID::ORIENT_A:
		return glm::rotate(glm::radians((float)degree), glm::vec3(0.0f, 1.0f, 0.0f));
	case ReorientViewID::ORIENT_R:
		return glm::rotate(glm::radians((float)degree), glm::vec3(-1.0f, 0.0f, 0.0f));
	case ReorientViewID::ORIENT_I:
		return glm::rotate(glm::radians((float)degree), glm::vec3(0.0f, 0.0f, -1.0f));
	default:
		assert(false);
		return glm::mat4();
	}
}

bool BaseViewOrientation::IsHoveredROIitem()
{
	if (roi_item_ && roi_item_->IsHoveredLines())
		return true;
	else
		return false;
}

/**=================================================================================================
private slots
*===============================================================================================**/

void BaseViewOrientation::slotTranslateROILine(int id, float trans)
{
	float trans_vol = view_render_param()->MapSceneToVol(trans);
	if (id == PanoOrientationROIItem::EROI::EROI_TOP)
	{
		roi_vol_.top += trans_vol;
		emit sigChangedROI();
	}
	else if (id == PanoOrientationROIItem::EROI::EROI_BOTTOM)
	{
		roi_vol_.bottom += trans_vol;
		emit sigChangedROI();
	}
	else if (id == PanoOrientationROIItem::EROI::EROI_MID)
	{
		roi_vol_.slice += trans_vol;
		emit sigChangedSlice();
	}
}

void BaseViewOrientation::slotUpdate()
{
	emit sigRenderQuality(type_);
	emit sigUpdateDoneROI();
}
