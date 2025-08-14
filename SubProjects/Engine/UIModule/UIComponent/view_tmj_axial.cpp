#include "view_tmj_axial.h"

#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLWidget>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/tmj_resource.h"
#include "../UIViewController/view_controller_slice.h"
#include "../UIViewController/view_render_param.h"

#include "../UIPrimitive/tmj_item.h"

#include "scene.h"

using namespace UIViewController;

ViewTMJaxial::ViewTMJaxial(QWidget* parent)
	: View(common::ViewTypeID::TMJ_ARCH, parent)
{
	controller_.reset(new ViewControllerSlice);
	controller_->set_view_param(View::view_render_param());

	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::SLIDER);
	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::GRID);
	scene().InitViewItem(Viewitems::NAVIGATION);
	scene().InitViewItem(Viewitems::DIRECTION_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().SetRulerColor(ColorView::kAxial);
	scene().SetBorderColor(ColorView::kAxial);
	scene().SetSliderTextColor(ColorView::kAxial);
	scene().SetDirectionTextItem(QString("R"), true);

	scene().InitMeasure(view_type());

	tmj_item_.reset(new TMJItem);
	tmj_item_->setZValue(5.0);
	connect(tmj_item_.get(), &TMJItem::sigUpdated, this,
		&ViewTMJaxial::sigUpdateTMJ);

	scene().addItem(tmj_item_.get());
}

ViewTMJaxial::~ViewTMJaxial()
{
	if (View::IsEnableGLContext())
	{
		View::MakeCurrent();
		ClearGL();
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewTMJaxial::SetHighlightLateralLine(const TMJDirectionType& type,
	const int& index)
{
	tmj_item_->SetHighlightLateralLine(type, index);
}

void ViewTMJaxial::SetVisibleTMJitemLateralLine(
	const TMJDirectionType& direction_type, const bool& is_visible)
{
	tmj_item_->SetLateralLineVisible(direction_type, is_visible);
}

void ViewTMJaxial::SetVisibleTMJitemFrontalLine(
	const TMJDirectionType& direction_type, const bool& is_visible)
{
	tmj_item_->SetFrontalLineVisible(direction_type, is_visible);
}

void ViewTMJaxial::SetLateralParam(const TMJLateralID& id, const float& value)
{
	tmj_item_->SetLateralParam(id, view_render_param()->MapVolToScene(value));
}

void ViewTMJaxial::GetLateralParam(const TMJLateralID& id, float* value) const
{
	float value_in_scene;
	tmj_item_->GetLateralParam(id, &value_in_scene);
	*value = view_render_param()->MapSceneToVol(value_in_scene);
}

bool ViewTMJaxial::TranslateLateralFromWheelEvent(
	const TMJDirectionType& direction_type, const int& wheel_step)
{
	TMJLateralID id = direction_type == TMJDirectionType::TMJ_LEFT ? TMJLateralID::LEFT_INTERVAL : TMJLateralID::RIGHT_INTERVAL;

	float interval;
	tmj_item_->GetLateralParam(id, &interval);
	return tmj_item_->ShiftLateralLine(direction_type, interval * static_cast<float>(wheel_step));
}

bool ViewTMJaxial::TranslateFrontalFromWheelEvent(
	const TMJDirectionType& direction_type, const int& wheel_step)
{
	TMJLateralID id = direction_type == TMJDirectionType::TMJ_LEFT ? TMJLateralID::LEFT_INTERVAL : TMJLateralID::RIGHT_INTERVAL;

	float interval;
	tmj_item_->GetLateralParam(id, &interval);
	return tmj_item_->ShiftFrontalLine(direction_type, interval * static_cast<float>(wheel_step));
}

void ViewTMJaxial::TranslateLateral(const TMJDirectionType& direction_type,
	const float& delta_vol)
{
	float delta_scene = view_render_param()->MapVolToScene(delta_vol);
	tmj_item_->ShiftLateralLine(direction_type, delta_scene);
}

void ViewTMJaxial::TranslateFrontal(const TMJDirectionType& direction_type,
	const float& delta_vol)
{
	float delta_scene = view_render_param()->MapVolToScene(delta_vol);
	tmj_item_->ShiftFrontalLine(direction_type, delta_scene);
}

void ViewTMJaxial::SetTMJlateralCount(const TMJDirectionType& type,
	const int& count)
{
	tmj_item_->SetLateralLineCount(type, count);
}

void ViewTMJaxial::GetTMJSizeInfo(const TMJDirectionType& type, float* width,
	float* height)
{
	float scene_width, scene_height;
	tmj_item_->GetROIRectSize(type, &scene_width, &scene_height);
	*width = view_render_param()->MapSceneToVol(scene_width);
	*height = view_render_param()->MapSceneToVol(scene_height);
}

bool ViewTMJaxial::GetTMJRectCenter(const TMJDirectionType& type,
	glm::vec3* pt_center_in_vol)
{
	QPointF pt_center_in_scene;
	bool res = tmj_item_->GetROIcenter(type, &pt_center_in_scene);

	if (!res) return false;

	*pt_center_in_vol = controller_->MapSceneToVol(pt_center_in_scene);
	return true;
}

bool ViewTMJaxial::GetLateralPositionInfo(
	const TMJDirectionType& type, std::map<int, glm::vec3>* pt_center_in_vol,
	glm::vec3* up_vector_in_vol) const
{
	std::map<float, QPointF> pt_center_in_scene;
	QPointF up_vector_in_scene;
	bool res = tmj_item_->GetLateralPositionInfo(type, &pt_center_in_scene,
		&up_vector_in_scene);

	pt_center_in_vol->clear();
	for (const auto& elem : pt_center_in_scene)
	{
		const glm::vec3& pt_vol = controller_->MapSceneToVol(elem.second);
		pt_center_in_vol->emplace(
			(int)(view_render_param()->MapSceneToVol(elem.first) + 0.005f) + 1,
			pt_vol);
	}

	if (!res) return false;

	const glm::vec3 p1 = controller_->MapSceneToVol(
		pt_center_in_scene.begin()->second + up_vector_in_scene);
	const glm::vec3 p2 = controller_->MapSceneToVol(
		pt_center_in_scene.begin()->second - up_vector_in_scene);
	*up_vector_in_vol =
		(type == TMJ_LEFT) ? glm::normalize(p1 - p2) : glm::normalize(p2 - p1);
	return true;
}
bool ViewTMJaxial::GetFrontalPositionInfo(const TMJDirectionType& type,
	glm::vec3* pt_center_in_vol,
	glm::vec3* up_vector_in_vol) const
{
	QPointF pt_center_in_scene, up_vector_in_scene;
	bool res = tmj_item_->GetFrontalPositionInfo(type, &pt_center_in_scene,
		&up_vector_in_scene);

	*pt_center_in_vol = controller_->MapSceneToVol(pt_center_in_scene);

	glm::vec3 p1 =
		controller_->MapSceneToVol(pt_center_in_scene + up_vector_in_scene);
	glm::vec3 p2 =
		controller_->MapSceneToVol(pt_center_in_scene - up_vector_in_scene);
	*up_vector_in_vol = glm::normalize(p1 - p2);

	return res;
}
void ViewTMJaxial::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	tmj_item_->setEnabled(type == common::CommonToolTypeOnOff::NONE ? true : false);
	View::SetCommonToolOnOff(type);
}

void ViewTMJaxial::HideAllUI(bool is_hide)
{
	View::HideAllUI(is_hide);
	tmj_item_->setVisible(!is_hide);
}

void ViewTMJaxial::InitTMJitems()
{
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();

	float width[TMJDirectionType::TMJ_TYPE_END];
	float height[TMJDirectionType::TMJ_TYPE_END];
	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		TMJDirectionType d_type = static_cast<TMJDirectionType>(i);
		const TMJfrontalResource& tmj_frontal = res_tmj.frontal(d_type);
		const TMJlateralResource& tmj_lateral = res_tmj.lateral(d_type);

		tmj_item_->SetLateralLineCount(d_type, tmj_lateral.param().count);
		tmj_item_->SetLateralSliceInterval(
			d_type,
			view_render_param()->MapVolToScene(tmj_lateral.param().interval));
		width[i] = view_render_param()->MapVolToScene(tmj_frontal.param().width);
		height[i] = view_render_param()->MapVolToScene(tmj_lateral.param().width);
	}
	TMJItem::InitParam tmj_param;
	tmj_param.scene_width = (float)view_render_param()->scene_size_width();
	tmj_param.scene_height = (float)view_render_param()->scene_size_height();
	tmj_param.left_roi_height = height[TMJ_LEFT];
	tmj_param.left_roi_width = width[TMJ_LEFT];
	tmj_param.right_roi_height = height[TMJ_RIGHT];
	tmj_param.right_roi_width = width[TMJ_RIGHT];
	tmj_param.degree_angle = 45.0f;
	tmj_item_->blockSignals(true);
	tmj_item_->Initialize(tmj_param);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		TMJDirectionType d_type = static_cast<TMJDirectionType>(i);
		const auto& proj_info = res_tmj.project_info(d_type);
		if (proj_info.is_imported)
		{
			this->SetTMJPositionAndDegree(d_type, proj_info.rect_center,
				proj_info.lateral_up_vector);
		}
		emit sigUpdateTMJ(d_type);
	}
	tmj_item_->blockSignals(false);

	scene().update();
}

void ViewTMJaxial::InitSliceRange(const float& min, const float& max,
	const float& value)
{
	scene().SetSliderRange(static_cast<int>(min), static_cast<int>(max));
	scene().SetSliderValue(static_cast<int>(value));
}

void ViewTMJaxial::UpdateSlice()
{
	this->RenderSlice();
	scene().update();
}

void ViewTMJaxial::DeleteROIRectUI(const TMJDirectionType& direction_type)
{
	tmj_item_->ClearTMJ(direction_type);
}

void ViewTMJaxial::ForceRotateMatrix(const glm::mat4& mat)
{
	controller_->ForceRotateMatrix(mat);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
		controller_->GetViewMatrix());
	RenderSlice();
	scene().update();
}

// pano arch에 signal 이 발생하여 모든 뷰들의 axial line들을 일괄 이동한다.
void ViewTMJaxial::SetSliceInVol(const float& z_pos_vol)
{
	scene().SetSliderValue(static_cast<int>(z_pos_vol));
}

void ViewTMJaxial::SetSliceRange(const float& min, const float& max)
{
	scene().SetSliderRange(static_cast<int>(min), static_cast<int>(max));
}
glm::vec3 ViewTMJaxial::GetUpVector() const
{
	return controller_->GetUpVector();
}
glm::vec3 ViewTMJaxial::GetCenterPosition() const
{
	return controller_->MapSceneToVol(view_render_param()->scene_center());
}
int ViewTMJaxial::GetSliceInVol() const { return scene().GetSliderValue(); }

void ViewTMJaxial::GetSliceRange(int* min, int* max)
{
	scene().GetSliceRange(min, max);
}

/**=================================================================================================
protected functions
*===============================================================================================**/

void ViewTMJaxial::TransformItems(const QTransform& transform)
{
	tmj_item_->TransformItems(transform);
}

void ViewTMJaxial::resizeEvent(QResizeEvent* pEvent)
{
	View::resizeEvent(pEvent);

	controller_->SetProjection();
}

void ViewTMJaxial::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	View::mousePressEvent(event);
}

void ViewTMJaxial::mouseReleaseEvent(QMouseEvent* event)
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

	if (View::IsSetTool())
	{
		if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT &&
			IsEventLeftButton(event))
		{
			emit sigWindowingDone();
		}

		View::mouseReleaseEvent(event);
		return;
	}

	if (IsEventLeftButton(event) && tmj_item_->IsAvaliableAddPoint())
	{
		TMJDirectionType tmj_type_created;
		tmj_item_->AddPoint(mapToScene(event->pos()), &tmj_type_created);
		if (tmj_type_created != TMJ_TYPE_UNKNOWN)
			emit sigROIRectCreated(tmj_type_created);
	}

	View::mouseReleaseEvent(event);
}

void ViewTMJaxial::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	View::mouseMoveEvent(event);
	RequestDICOMInfo(mapToScene(event->pos()));
	if (View::IsSetTool())
		return;

	if (tmj_item_->IsStartEdit())
	{
		tmj_item_->DrawPoint(mapToScene(event->pos()));
	}
}

void ViewTMJaxial::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE
	
	View::keyPressEvent(event);
}

void ViewTMJaxial::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	View::keyReleaseEvent(event);
}

void ViewTMJaxial::leaveEvent(QEvent* event)
{
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void ViewTMJaxial::enterEvent(QEvent* event)
{
	View::enterEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}

void ViewTMJaxial::RenderSlice()
{
	if (!View::IsEnableGLContext()) return;

	MakeCurrent();
	controller_->RenderingSlice();
	DoneCurrent();

	this->UpdateMeasurePlaneInfo();
}

void ViewTMJaxial::UpdateMeasurePlaneInfo()
{
	scene().UpdatePlaneInfo(
		controller_->MapSceneToVol(
			view_render_param()->scene_center() -
			view_render_param()->MapGLToScene(view_render_param()->gl_trans())),
		controller_->GetUpVector(), controller_->GetRightVector());
}

/**=================================================================================================
puiblic slots
*===============================================================================================**/
void ViewTMJaxial::slotROIRectDraw(const TMJDirectionType& direction_type,
	bool draw_on)
{
	tmj_item_->SetDrawOn(direction_type, draw_on);
}

void ViewTMJaxial::slotTMJRectSizeChanged(const TMJRectID& roi_id,
	double value)
{
	float scene_thickness = view_render_param()->MapActualToScene(value);
	tmj_item_->SetTMJRectSize(roi_id, scene_thickness);
}

/**=================================================================================================
private slots
*===============================================================================================**/
void ViewTMJaxial::slotActiveSharpenItem(const int index)
{
	controller_->SetSharpenLevel(static_cast<SharpenLevel>(index));

	this->RenderSlice();
}
void ViewTMJaxial::slotChangedValueSlider(int value)
{
	if (!controller_->IsReady()) return;

	const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (&vol == nullptr) return;

	float vol_width = (float)vol.width();
	float vol_height = (float)vol.height();
	float vol_depth = (float)vol.depth();

	controller_->SetPlane(
		glm::vec3(vol_width * 0.5f, vol_height * 0.5f, (float)value),
		vol_width * UIViewController::kRightVector,
		vol_height * UIViewController::kBackVector, 0);
	RenderSlice();
	scene().update();

	emit sigTranslateZ(value);
}

void ViewTMJaxial::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	controller_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void ViewTMJaxial::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data)
{
	controller_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}

/**=================================================================================================
private functions
*===============================================================================================**/

void ViewTMJaxial::drawBackground(QPainter* painter, const QRectF& rect)
{
	View::drawBackground(painter, rect);

	if (!controller_->IsReady())
	{
		if (controller_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	painter->beginNativePainting();
	if (IsUpdateController())
	{
		controller_->RenderingSlice();
		UpdateDoneContoller();
	}

	controller_->RenderScreen(View::GetDefaultFrameBufferObject());
	painter->endNativePainting();
}

void ViewTMJaxial::SetGraphicsItems()
{
	View::SetGraphicsItems();
	emit sigRequestInitialize();
	SyncMeasureResourceCounterparts(true);
}

void ViewTMJaxial::InitializeController()
{
	controller_->Initialize();

	const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (&vol == nullptr) return;

	float vol_width = (float)vol.width();
	float vol_height = (float)vol.height();
	float vol_depth = (float)vol.depth();

	controller_->SetPlane(glm::vec3(vol_width * 0.5f, vol_height * 0.5f,
		(float)scene().GetSliderValue()),
		vol_width * UIViewController::kRightVector,
		vol_height * UIViewController::kBackVector, 0);

	scene().SetWorldAxisDirection(controller_->GetRotateMatrix(),
		controller_->GetViewMatrix());
}

bool ViewTMJaxial::IsReadyController() { return controller_->IsReady(); }

void ViewTMJaxial::ClearGL() { controller_->ClearGL(); }

void ViewTMJaxial::ActiveControllerViewEvent()
{
	bool need_render = false;
	controller_->ProcessViewEvent(&need_render);

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT) need_render = true;

	if (need_render)
	{
		RenderSlice();
	}
}

void ViewTMJaxial::RequestDICOMInfo(const QPointF& pt_scene)
{
	glm::vec4 vol_info = controller_->GetDicomInfoPoint(pt_scene);
	DisplayDICOMInfo(vol_info);
}

void ViewTMJaxial::DisplayDICOMInfo(const glm::vec4& vol_info)
{
	float window_width, window_level;
	controller_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_->GetIntercept());
	if (vol_info.w != common::dicom::kInvalidHU)
	{
		scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
			.arg(wl)
			.arg(ww)
			.arg(static_cast<int>(vol_info.x))
			.arg(static_cast<int>(vol_info.y))
			.arg(static_cast<int>(vol_info.z))
			.arg(vol_info.w));
	}
	else
	{
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)").arg(wl).arg(ww));
	}
}

void ViewTMJaxial::SetTMJPositionAndDegree(
	const TMJDirectionType& type, const glm::vec3& rect_center_position,
	const glm::vec3& up_vector)
{
	const QPointF pt_scene_center_pos =
		controller_->MapVolToScene(rect_center_position);

	const QPointF p1 =
		controller_->MapVolToScene(rect_center_position + up_vector);
	const QPointF p2 =
		controller_->MapVolToScene(rect_center_position - up_vector);
	QVector2D vec_scene_up = (type == TMJ_LEFT) ? QVector2D(p1 - p2).normalized()
		: QVector2D(p2 - p1).normalized();

	tmj_item_->SetPositionAndDegree(type, pt_scene_center_pos, vec_scene_up);
}

void ViewTMJaxial::SetFrontalLineIndex(const TMJDirectionType& direction, int index)
{
	float available_min = 0.0f;
	float available_max = 0.0f;
	tmj_item_->GetAvailableFrontalRange(direction, available_min, available_max);

	float shifted = index + available_min;
	shifted = std::min(std::max(shifted, available_min), available_max);

	TranslateFrontalFromWheelEvent(direction, static_cast<int>(shifted));
}

void ViewTMJaxial::SetLateralLineIndex(const TMJDirectionType& direction, int index)
{
	float available_min = 0.0f;
	float available_max = 0.0f;
	tmj_item_->GetAvailableLateralRange(direction, available_min, available_max);

	float shifted = index + available_min;
	shifted = std::min(std::max(shifted, available_min), available_max);

	TranslateLateralFromWheelEvent(direction, static_cast<int>(shifted));
}
