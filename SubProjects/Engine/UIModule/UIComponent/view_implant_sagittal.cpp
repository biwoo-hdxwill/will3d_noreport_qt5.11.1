#include "view_implant_sagittal.h"

#include <QMouseEvent>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/sagittal_resource.h"

#include "../UIPrimitive/W3EllipseItem.h"
#include "../UIPrimitive/guide_line_list_item.h"
#include "../UIPrimitive/implant_handle.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace 
{
	const int kLineID = 0;
}

ViewImplantSagittal::ViewImplantSagittal(QWidget* parent)
	: View(common::ViewTypeID::IMPLANT_SAGITTAL, parent),
	implant_handle_(new ImplantHandle()),
	implant_info_(new SimpleTextItem()),
	sagittal_position_ui_(new CW3EllipseItem()),
	reference_axial_line_(new GuideLineListItem(GuideLineListItem::HORIZONTAL)),
	hovered_implant_spec_(new SimpleTextItem()) 
{
	this->scene().addItem(implant_handle_.get());
	connect(implant_handle_.get(), SIGNAL(sigTranslate()), this, SLOT(slotTranslateImplant()));
	connect(implant_handle_.get(), SIGNAL(sigRotate(float)), this, SLOT(slotRotateImplant(float)));
	connect(implant_handle_.get(), SIGNAL(sigUpdate()), this, SLOT(slotUpdateImplantPos()));

	controller_slice_.reset(new ViewControllerSlice);
	controller_slice_->set_view_param(view_render_param());
	controller_slice_->SetVisibleImplant(true);
	controller_slice_->SetVisibleNerve(true);

	scene().InitViewItem(Viewitems::RULER);
	scene().SetRulerColor(ColorView::kSagittal);

	scene().InitViewItem(Viewitems::BORDER);
	scene().SetBorderColor(ColorView::kSagittal);

	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitMeasure(view_type());

	scene().InitViewItem(Viewitems::GRID);

	implant_info_->setTextColor(ColorView::k3D);
	scene().addItem(implant_info_.get());

	hovered_implant_spec_->setVisible(false);
	hovered_implant_spec_->setTextColor(ColorImplant::kImplantPlaced);
	scene().addItem(hovered_implant_spec_.get());

	sagittal_position_ui_->setPen(QPen(ColorView::kSagittal, 1, Qt::SolidLine));
	sagittal_position_ui_->setRect(-5, -5, 10, 10);
	scene().addItem(sagittal_position_ui_.get());

	reference_axial_line_->setZValue(0);
	reference_axial_line_->SetHighlight(true);
	reference_axial_line_->set_pen(
		QPen(ColorAxialItem::kLinePenColor, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_axial_line_.get());
}

ViewImplantSagittal::~ViewImplantSagittal() 
{
	if (View::IsEnableGLContext()) 
	{
		View::MakeCurrent();
		controller_slice_->ClearGL();
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewImplantSagittal::RenderSlice() 
{
	if (!View::IsEnableGLContext()) return;

	this->UpdateMeasurePlaneInfo();

	MakeCurrent();
	controller_slice_->RenderingSlice();
	DoneCurrent();
}

void ViewImplantSagittal::HideAllUI(bool is_hide) 
{
	View::HideAllUI(is_hide);
	reference_axial_line_->setVisible(!is_hide);
	sagittal_position_ui_->setVisible(!is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantSagittal::HideText(bool is_hide) 
{
	View::HideText(is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantSagittal::SetAxialLine(const QPointF& axial_position_in_sagittal_plane) 
{
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();

	if (&res_sagittal == nullptr ||
		axial_position_in_sagittal_plane == QPointF()) 
	{
		reference_axial_line_->ClearLines();
		return;
	}
	axial_position_in_sagittal_plane_ = axial_position_in_sagittal_plane;

	double width = (double)res_sagittal.params().width;
	double height = (double)res_sagittal.params().height;

	QPointF axial_position_in_scene = controller_slice_->MapPlaneToScene(
		QPointF(width * 0.5, axial_position_in_sagittal_plane.y()));

	auto iter = reference_axial_line_->line_positions().find(kLineID);
	if (iter != reference_axial_line_->line_positions().end() &&
		iter->second == axial_position_in_scene) 
	{
		return;
	}

	QPointF line_left_top = controller_slice_->MapPlaneToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom = controller_slice_->MapPlaneToScene(QPointF(width, height));

	float len_width = (float)abs(line_right_bottom.x() - line_left_top.x());

	reference_axial_line_->SetRangeScene(line_left_top.y(),
		line_right_bottom.y());
	reference_axial_line_->set_length(len_width);
	reference_axial_line_->SetLine(kLineID, axial_position_in_scene,
		QVector2D(1.0, 0.0));

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"ViewImplantSagittal::SetAxialLine");
#endif
}

void ViewImplantSagittal::SceneUpdate() 
{
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();
	if (&res_sagittal == nullptr) 
	{
		controller_slice_->ClearPlane();
		scene().ViewEnableStatusChanged(false);
		return;
	}
	else 
	{
		scene().ViewEnableStatusChanged(true);
	}

	SetSagittalPlane();
	RenderSlice();
	scene().update();

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"ViewImplantSagittal::SceneUpdate");
#endif
}

/*
불리는 조건
1. Implant selection
2. 임플란트가 이동했을 때 Handle 과 Spec 위치를 업데이트
*/
void ViewImplantSagittal::UpdateImplantHandleAndSpec() 
{
	if (!controller_slice_->IsReady()) return;

	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();

	int selected_id = res_implant.selected_implant_id();
	this->UpdateSelectedImplantHandleAndInfo(implant_datas, selected_id);
}

void ViewImplantSagittal::UpdateSelectedImplantHandleAndInfo(
	const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
	int selected_id)
{
	if (!controller_slice_->IsReady())
	{
		implant_handle_->Disable();
		return;
	}

	// selected 인 spec text 및 handle의 visible 상태와 position을 결정
	if (selected_id < 0 ||
		implant_datas.find(selected_id) == implant_datas.end())
	{
		implant_handle_->Disable();
	}
	else
	{
		QPointF pt_scene = GetImplantScenePosition(selected_id);
		implant_handle_->Enable(
			selected_id,
			IsUIVisible() && implant_datas.find(selected_id)->second->is_visible(),
			pt_scene);
	}

	UpdateImplantInfoText(selected_id);
}

void ViewImplantSagittal::DisableImplantHandleAndSpec() 
{
	implant_handle_->Disable();
	implant_info_->setVisible(false);
}

void ViewImplantSagittal::DeleteImplant(int implant_id) 
{
	if (implant_handle_->selected_id() == implant_id) 
	{
		implant_handle_->Disable();
	}
}

void ViewImplantSagittal::DeleteAllImplants() 
{ 
	implant_handle_->Disable(); 
}

glm::vec3 ViewImplantSagittal::GetUpVector() 
{
	return controller_slice_->GetUpVector();
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void ViewImplantSagittal::slotActiveSharpenItem(const int index) 
{
	controller_slice_->SetSharpenLevel(static_cast<SharpenLevel>(index));
}

void ViewImplantSagittal::TransformItems(const QTransform& transform) 
{
	reference_axial_line_->TransformItems(transform);
	implant_handle_->TransformItems(transform);
}

void ViewImplantSagittal::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) 
{
	View::SetCommonToolOnOff(type);

	if (type == common::CommonToolTypeOnOff::NONE) 
	{
		implant_handle_->set_flag_movable(true);
	}
	else 
	{
		implant_handle_->set_flag_movable(false);
	}
}

void ViewImplantSagittal::wheelEvent(QWheelEvent* event) 
{
	const float kSingleStep = 120.0f;
	emit sigRotateView((float)(event->delta()) / kSingleStep);
}

void ViewImplantSagittal::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	int key = event->key();
	input_key_map_[key] = true;

	Qt::KeyboardModifiers modifiers = event->modifiers();
	if (modifiers.testFlag(Qt::ControlModifier))
	{
		QWidget::keyPressEvent(event);

		TranslationDetailedControlImplant();
	}
	else if (modifiers.testFlag(Qt::AltModifier))
	{
		QWidget::keyPressEvent(event);

		RotationDetailedControlImplant();
	}
	else
	{
		const int kSingleStep = 1;
		if (event->key() == Qt::Key_Up)
		{
			emit sigRotateView((float)(kSingleStep));
		}
		else if (event->key() == Qt::Key_Down)
		{
			emit sigRotateView((float)(-kSingleStep));
		}
		else if (event->key() == Qt::Key_W)
		{
			controller_slice_->set_is_implant_wire(!controller_slice_->is_implant_wire());
			RenderSlice();
			scene().update();
		}
	}	
}

void ViewImplantSagittal::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
	}
#endif // WILL3D_EUROPE

	View::keyReleaseEvent(event);

	int implant_id = implant_handle_->selected_id();
	if (implant_id == -1)
	{
		InputKeyClear();
		return;
	}

	int key = event->key();
	input_key_map_[key] = false;

	Qt::KeyboardModifiers modifiers = event->modifiers();
	if (modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::AltModifier) || !is_update_implant_)
	{
		return;
	}

	if (!input_key_map_[Qt::Key_Up] && !input_key_map_[Qt::Key_Down] && !input_key_map_[Qt::Key_Left] && !input_key_map_[Qt::Key_Right])
	{
		int implant_id = implant_handle_->selected_id();
		QPointF pt_scene_imp_handle = implant_handle_->scenePos();

		emit sigUpdateImplantImages(implant_id, controller_slice()->MapSceneToPlane(pt_scene_imp_handle));

		InputKeyClear();

		is_update_implant_ = false;
	}
}

void ViewImplantSagittal::resizeEvent(QResizeEvent* pEvent) 
{
	View::resizeEvent(pEvent);

	implant_info_->setPos(25, 35);
	sagittal_position_ui_->setPos(50, height() * 0.5);
	controller_slice_->SetProjection();
}

void ViewImplantSagittal::mouseMoveEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	QPointF scene_pos = pt_scene_current();
	RequestDICOMInfo(scene_pos);
	View::mouseMoveEvent(event);

	if (IsEventAxialLineHovered() && IsEventLeftButton(event)) 
	{
		emit sigSetAxialSlice(controller_slice_->MapSceneToVol(scene_pos));
	}

	if (IsTextVisible())
	{
		int implant_id = GetImplantHoveredID(scene_pos);
		if (implant_id > 0 && implant_id != implant_handle_->selected_id()) 
		{
			QPointF implant_pos = GetImplantScenePosition(implant_id);
			hovered_implant_spec_->SetText(QString("#%1").arg(QString::number(implant_id)));
			hovered_implant_spec_->setPosCenter(implant_pos);
			hovered_implant_spec_->setVisible(true);
		}
		else 
		{
			hovered_implant_spec_->setVisible(false);
		}
	}

	if (!implant_handle_->IsActive()) 
	{
		int implant_id = GetImplantHoveredID(mapToScene(event->pos()));

		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		const auto& implant_datas = res_implant.data();

		if (implant_id > 0) 
		{
			this->UpdateSelectedImplantHandleAndInfo(implant_datas, implant_id);
		}
		else 
		{
			int selected_id = res_implant.selected_implant_id();
			UpdateSelectedImplantHandleAndInfo(implant_datas, selected_id);
		}
	}
}

void ViewImplantSagittal::mousePressEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	int implant_id = GetImplantHoveredID(mapToScene(event->pos()));
	if (implant_id < 0) return View::mousePressEvent(event);

	if (implant_id != ResourceContainer::GetInstance()->GetImplantResource().selected_implant_id())
		emit sigSelectImplant(implant_id);

	SceneUpdate();

	View::mousePressEvent(event);
}

void ViewImplantSagittal::mouseReleaseEvent(QMouseEvent* event) 
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

	if (event->button() != Qt::LeftButton || View::IsSetTool())
		return View::mouseReleaseEvent(event);

	if (implant_handle_->IsActive()) 
	{
		implant_handle_->EndEvent();
	}

	View::mouseReleaseEvent(event);

	hovered_implant_spec_->setVisible(false);
}

void ViewImplantSagittal::leaveEvent(QEvent* event)
{
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);

	if (implant_handle_)
	{
		implant_handle_->setVisible(false);
	}
}

void ViewImplantSagittal::enterEvent(QEvent* event)
{
	View::enterEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, true);

	ImplantHandleVisible();
}

const bool ViewImplantSagittal::IsInSagittalPlane(const int& x, const int& y) const 
{
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();

	if (&res_sagittal == nullptr) 
	{
		return false;
	}

	int width = static_cast<int>(res_sagittal.params().width);
	int height = static_cast<int>(res_sagittal.params().height);

	if (width > x && x >= 0 && height > y && y >= 0)
		return true;
	else
		return false;
}

/**=================================================================================================
private functions
*===============================================================================================**/
void ViewImplantSagittal::InitializeController() 
{
	controller_slice_->Initialize();
}

bool ViewImplantSagittal::IsReadyController() 
{
	return controller_slice_->IsReady();
}

void ViewImplantSagittal::ClearGL() { controller_slice_->ClearGL(); }

void ViewImplantSagittal::ActiveControllerViewEvent() 
{
	bool need_render = false;
	controller_slice_->ProcessViewEvent(&need_render);
	if (need_render) 
	{
		RenderSlice();
	}
}

void ViewImplantSagittal::drawBackground(QPainter* painter, const QRectF& rect) 
{
	View::drawBackground(painter, rect);

	if (!IsReadyController()) 
	{
		if (controller_slice_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	painter->beginNativePainting();
	if (IsUpdateController()) 
	{
		this->SetSagittalPlane();

		controller_slice_->RenderingSlice();

		SetAxialLine(axial_position_in_sagittal_plane_);
		UpdateDoneContoller();
	}

	RenderScreen();
	painter->endNativePainting();
}

void ViewImplantSagittal::RenderScreen() 
{
	controller_slice_->RenderScreen(GetDefaultFrameBufferObject());
}

void ViewImplantSagittal::SetSagittalPlane() 
{
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();
	if (&res_sagittal == nullptr) 
	{
		controller_slice_->ClearPlane();
		return;
	}

	const glm::vec3& up_vector = res_sagittal.up_vector();
	const glm::vec3 back_vector = glm::normalize(res_sagittal.back_vector());
	const glm::vec3 right_vector =
		glm::normalize(glm::cross(back_vector, up_vector));
	const auto& sagittal_params = res_sagittal.params();
	controller_slice_->SetPlane(res_sagittal.center_position(),
		right_vector * (float)sagittal_params.width,
		back_vector * (float)sagittal_params.height, 0);

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));
}

void ViewImplantSagittal::UpdateMeasurePlaneInfo() 
{
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();
	if (&res_sagittal == nullptr) 
	{
		return;
	}
	scene().UpdatePlaneInfo(res_sagittal.center_position(),
		res_sagittal.up_vector(), res_sagittal.back_vector());
}
void ViewImplantSagittal::UpdateImplantInfoText(int implant_id) 
{
	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	if (&implant_resource == nullptr || implant_id < 0) 
	{
		implant_info_->setVisible(false);
		return;
	}

	const auto implant_data = implant_resource.data().at(implant_id).get();
	implant_info_->SetText(QString("ID : %1\nDiameter : %2\nLength : %3")
		.arg(QString::number(implant_data->id()))
		.arg(QString::number(implant_data->diameter()))
		.arg(QString::number(implant_data->length())));
	implant_info_->setVisible(IsTextVisible() && implant_data->is_visible());
}

void ViewImplantSagittal::RequestDICOMInfo(const QPointF& pt_scene) 
{
	glm::vec4 vol_info = controller_slice_->GetDicomInfoPoint(pt_scene);
	DisplayDICOMInfo(vol_info);
}

void ViewImplantSagittal::DisplayDICOMInfo(glm::vec4& vol_info) 
{
	float window_width, window_level;
	controller_slice_->GetWindowParams(&window_width, &window_level);
	int ww = static_cast<int>(window_width);
	int wl = static_cast<int>(window_level + controller_slice_->GetIntercept());
	if (vol_info.w != common::dicom::kInvalidHU) {
		scene().SetHUValue(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
			.arg(wl)
			.arg(ww)
			.arg(vol_info.x)
			.arg(vol_info.y)
			.arg(vol_info.z)
			.arg(vol_info.w));
	}
	else 
	{
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)").arg(wl).arg(ww));
	}
}

bool ViewImplantSagittal::IsEventAxialLineHovered() const 
{
	return (reference_axial_line_->is_highlight() && !View::IsSetTool()) ? true
		: false;
}

QPointF ViewImplantSagittal::GetImplantScenePosition(const int& implant_id) 
{
#if 0
	QPointF plane_pos;
	emit sigGetImplantPlanePos(implant_id, plane_pos);
	return controller_slice_->MapPlaneToScene(plane_pos);
#else
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	auto iter = implant_datas.find(implant_id);
	QPointF pt_scene;
	if (iter != implant_datas.end())
	{
		pt_scene = controller_slice()->MapVolToScene(iter->second->position_in_vol());
	}
	return pt_scene;
#endif
}

int ViewImplantSagittal::GetImplantHoveredID(const QPointF& pt_scene) const 
{
	int hovered_id = -1;
	// implant handle이 호버이거나 선택된 상태이면 핸들의 아이디를 가져오고
	// 아니면 PanoEngine의 Picking 알고리즘 결과를 가져온다.
	if (implant_handle_->is_hovered() || implant_handle_->isSelected()) 
	{
		hovered_id = implant_handle_->selected_id();
	}
	else 
	{
		emit sigImplantHovered(controller_slice()->MapSceneToPlane(pt_scene), &hovered_id);
	}

	return hovered_id;
}

void ViewImplantSagittal::TranslationDetailedControlImplant()
{
	int implant_id = implant_handle_->selected_id();
	if (implant_id == -1)
	{
		return;
	}

	GlobalPreferences::Preferences *preferences = &GlobalPreferences::GetInstance()->preferences_;
	float increments = preferences->advanced.implant_view.translation_increments;

	QPointF trans = QPointF(0.f, 0.f);
	if (input_key_map_[Qt::Key_Up])
	{
		trans = QPointF(0.f, -increments);
	}
	else if (input_key_map_[Qt::Key_Down])
	{
		trans = QPointF(0.f, increments);
	}
	else if (input_key_map_[Qt::Key_Left])
	{
		trans = QPointF(-increments, 0.f);
	}
	else if (input_key_map_[Qt::Key_Right])
	{
		trans = QPointF(increments, 0.f);
	}
	else
	{
		return;
	}

	trans /= view_render_param()->base_pixel_spacing_mm();

	QPointF scene_trans = view_render_param()->MapGLToScene(trans);
	QPointF implant_set_pos = implant_handle_->scenePos() + scene_trans;

	implant_handle_->setPos(implant_set_pos);

	QPointF implant_scene_pos = implant_handle_->scenePos();

	emit sigTranslateImplant(implant_id, controller_slice()->MapSceneToPlane(implant_set_pos));

	SceneUpdate();

	is_update_implant_ = true;
}

void ViewImplantSagittal::RotationDetailedControlImplant()
{
	int implant_id = implant_handle_->selected_id();
	if (implant_id == -1)
	{
		return;
	}

	GlobalPreferences::Preferences *preferences = &GlobalPreferences::GetInstance()->preferences_;
	int degree = 0; preferences->advanced.implant_view.rotation_increments;

	if (input_key_map_[Qt::Key_Left])
	{
		degree = preferences->advanced.implant_view.rotation_increments;
	}
	else if (input_key_map_[Qt::Key_Right])
	{
		degree = -preferences->advanced.implant_view.rotation_increments;
	}
	else
	{
		return;
	}

	emit sigRotateImplant(implant_handle_->selected_id(), degree);

	SceneUpdate();

	is_update_implant_ = true;
}

void ViewImplantSagittal::InputKeyClear()
{
	std::map<int, bool>::iterator iter = input_key_map_.begin();
	std::map<int, bool>::iterator iter_end = input_key_map_.end();

	for (; iter != iter_end; ++iter)
	{
		if (iter->second)
		{
			input_key_map_.clear();
			break;
		}
	}
}

void ViewImplantSagittal::ImplantHandleVisible()
{
	if (implant_handle_)
	{
		const std::map<int, bool>& visibility_map = controller_slice()->GetImplantVisibility();
		if (!visibility_map.empty())
		{
			int selected_id = implant_handle_->selected_id();
			auto iter = visibility_map.find(selected_id);
			if (iter != visibility_map.end())
			{
				implant_handle_->setVisible(visibility_map.at(selected_id) && IsTextVisible());
			}
		}
	}
}

void ViewImplantSagittal::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) 
{
	controller_slice_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void ViewImplantSagittal::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) 
{
	controller_slice_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}

void ViewImplantSagittal::slotTranslateImplant() 
{
	QPointF implant_scene_pos = implant_handle_->scenePos();
	emit sigTranslateImplant(
		implant_handle_->selected_id(),
		controller_slice()->MapSceneToPlane(implant_scene_pos));
	SceneUpdate();
}

void ViewImplantSagittal::slotRotateImplant(float degree_angle) 
{
	emit sigRotateImplant(implant_handle_->selected_id(), degree_angle);
	SceneUpdate();
}

void ViewImplantSagittal::slotUpdateImplantPos() 
{
	int implant_id = implant_handle_->selected_id();
	emit sigUpdateImplantImages(
		implant_handle_->selected_id(),
		controller_slice()->MapSceneToPlane(implant_handle_->scenePos()));
}

void ViewImplantSagittal::ApplyPreferences()
{
	controller_slice_->ApplyPreferences();

	View::ApplyPreferences();
}
