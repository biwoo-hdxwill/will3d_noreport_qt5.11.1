#include "view_implant_arch.h"
#include <qmath.h>
#include <QMouseEvent>

#include <Engine/Common/Common/global_preferences.h> 
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Resource/Resource/sagittal_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../UIPrimitive/pano_arch_item.h"
#include "../UIPrimitive/rotate_line_item.h"
#include "../UIPrimitive/implant_handle.h"
#include "../UIPrimitive/implant_text_item.h"
#include "../UIPrimitive/W3EllipseItem.h"
#include "../UIViewController/view_controller_slice.h"
#include "scene.h"

namespace {
const QVector2D kRotLineDirection = QVector2D(1.0f, 0.0f);
const QRectF kSagittalPositionSize(-5, -5, 10, 10);
}

ViewImplantArch::ViewImplantArch(QWidget* parent) :
	BaseViewPanoArch(parent), sagittal_line_(new RotateLineItem),
	implant_handle_(new ImplantHandle) {
	this->scene().addItem(implant_handle_.get());
	
	for(int i = 0 ; i < ArchTypeID::ARCH_TYPE_END; i++)
		arch(i)->SetDisplayMode(PanoArchItem::DisplayMode::IMPLANT);
	scene().addItem(sagittal_line_.get());

	sagittal_position_ui_.reset(new CW3EllipseItem(QPointF(), kSagittalPositionSize, kSagittalPositionSize));
	sagittal_position_ui_->setPen(QPen(ColorView::kSagittal, 1, Qt::SolidLine));
	sagittal_position_ui_->setHoveredPen(QPen(ColorView::kSagittal, 2, Qt::SolidLine));
	sagittal_position_ui_->SetFlagHighlight(true);
	sagittal_position_ui_->SetHighlight(false);
	sagittal_position_ui_->SetToRotateHandle();
	
	scene().addItem(sagittal_position_ui_.get());

	SetConnections();
}

ViewImplantArch::~ViewImplantArch() {}

void ViewImplantArch::SetReconPlane(const glm::vec3& center_pos,
									const glm::vec3& right_vector,
									const glm::vec3& back_vector) {
	controller()->SetPlane(center_pos,
						   right_vector,
						   back_vector, 1.0f);

	BaseViewPanoArch::RenderSlice();
	this->scene().update();
}
/*
불리는 조건
1. Implant selection
2. 임플란트가 이동했을 때 Handle 과 Spec 위치를 업데이트
*/
void ViewImplantArch::UpdateImplantHandleAndSpec() {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();

	int selected_id = res_implant.selected_implant_id();

	if (implant_datas.size() == 0)
		return;

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	for (const auto& elem : implant_datas) {
		if (implant_specs().find(elem.first) == implant_specs().end()) {
			CreateImplantSpec(elem.second.get());
		}

		bool always_show_implant_id = GlobalPreferences::GetInstance()->preferences_.objects.implant.always_show_implant_id;
		if (always_show_implant_id)
		{
			implant_specs()[elem.first]->setVisible(IsTextVisible() && elem.second->is_visible());
		}
		else
		{
			implant_specs()[elem.first]->setVisible(false);
		}

		if (selected_id == elem.first)
			continue;

		QPointF pt_scene = controller()->MapVolToScene(elem.second->position_in_vol());
		implant_specs()[elem.first]->SetSelected(false, pt_scene);
	}

	// selected 인 spec text 및 handle의 visible 상태와 position을 결정
	if (selected_id > 0) {
		auto iter = implant_datas.find(selected_id);
		if (iter != implant_datas.end()) {

			QPointF pt_scene = controller()->MapVolToScene(iter->second->axis_point_in_vol());

			pt_scene.setY(pt_scene.y());

			if (implant_specs().find(selected_id) == implant_specs().end()) {
				CreateImplantSpec(iter->second.get());
			}

			implant_specs()[selected_id]->setPos(pt_scene);
			const bool visible = IsTextVisible() && iter->second->is_visible();
			implant_specs()[selected_id]->SetSelected(visible);
			implant_specs()[selected_id]->setVisible(visible);
		}
	}
	UpdateSelectedImplantHandle(implant_datas, selected_id);

}

void ViewImplantArch::UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id)
{
#if 0
	if (selected_id > 0)
	{
		auto iter = implant_datas.find(selected_id);
		if (iter == implant_datas.end())
		{
			return;
		}

		QPointF pt_scene = controller()->MapVolToScene(iter->second->position_in_vol());
		implant_handle_->Enable(selected_id, IsUIVisible() && iter->second->is_visible(), pt_scene);
	}
	else
	{
		implant_handle_->Disable();
	}
#else
	implant_handle_->Disable(); // implant 회전 UI 변경을 위해 arch 에서는 implant 조작 불가능하게 변경
#endif
}

void ViewImplantArch::ChangeSelectedImplantSpec() {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.size() == 0)
		return;

	int selected_id = res_implant.selected_implant_id();
	if (selected_id < 0)
		return;

	auto iter = implant_datas.find(selected_id);
	if (iter != implant_datas.end()) {
		implant_specs()[selected_id]->ChangeImplantSpec(iter->second.get());
	}
}

void ViewImplantArch::DeleteImplant(int implant_id) {
	if (implant_handle_->selected_id() == implant_id)
		implant_handle_->Disable();

	auto iter = implant_specs().find(implant_id);
	if (iter != implant_specs().end()) {
		scene().removeItem(iter->second.get());
		implant_specs().erase(iter);
	}

	RenderSlice();
}

void ViewImplantArch::DeleteAllImplants() {
	implant_handle_->Disable();

	for (auto& spec : implant_specs())
		scene().removeItem(spec.second.get());
	implant_specs().clear();

	RenderSlice();
}

void ViewImplantArch::SetSagittalLineFromVolPos(const glm::vec3& vol_pos,
												const glm::vec3& vol_prev,
												const glm::vec3& vol_next) {
	sagittal_line_center_ = controller()->MapVolToScene(vol_pos);
	const QPointF dest(controller()->MapVolToScene(vol_next));
	const QPointF base(controller()->MapVolToScene(vol_prev));
	QVector2D arch_tangential_(dest - base);
	arch_tangential_.normalize();
	arch_line_angle_radian_ = asin(arch_tangential_.x() * kRotLineDirection.y()
								   - arch_tangential_.y() * kRotLineDirection.x());
	RotateSagittalLine();
}

void ViewImplantArch::RotateSagittalLine() {
	const auto& res_sagittal = ResourceContainer::GetInstance()->GetSagittalResource();
	QPen pen_sagittal_line(ColorView::kSagittal, 2.0, Qt::SolidLine, Qt::FlatCap);
	sagittal_line_->setPen(pen_sagittal_line);

	float radian = (res_sagittal.params().degree * M_PI) / 180.0f - arch_line_angle_radian_;
	sagittal_line_->set_length(view_render_param()->MapActualToScene(kSzSagittalImage));
	sagittal_line_->SetLine(sagittal_line_center_, QVector2D(cos(radian), sin(radian)));
	sagittal_position_ui_->setPos(sagittal_line_->p2());
	
#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode("ViewImplantArch::RotateSagittalLine");
#endif 
}

glm::vec3 ViewImplantArch::GetCurrentSagittalCenterPos() {
	return controller()->MapSceneToVol(sagittal_line_center_);
}
glm::mat4 ViewImplantArch::GetCameraMatrix() const{
	return controller()->GetCameraMatrix();
}

void ViewImplantArch::HideAllUI(bool is_hide) {
	BaseViewPanoArch::HideAllUI(is_hide);
	sagittal_line_->setVisible(!is_hide);
  sagittal_position_ui_->setVisible(!is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantArch::HideText(bool is_hide) {
	BaseViewPanoArch::HideText(is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantArch::slotTranslateImplant() {
	int implant_id = implant_handle_->selected_id();
	emit sigTranslateImplant(implant_handle_->selected_id(),
							 controller()->MapSceneToVol(implant_handle_->scenePos()));
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	this->RenderSlice();
}

void ViewImplantArch::slotRotateImplant(float degree_angle) {
	emit sigRotateImplant(implant_handle_->selected_id(),
						  controller()->GetUpVector(), degree_angle);
	int implant_id = implant_handle_->selected_id();
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	this->RenderSlice();
}

void ViewImplantArch::slotUpdateImplantPos() {
	emit sigUpdateImplantImages(implant_handle_->selected_id(),
					controller()->MapSceneToVol(implant_handle_->scenePos()));
	this->UpdateSlice();
}

void ViewImplantArch::slotSyncSagittalLineStatusMovable(bool hovered) {
	sagittal_line_->SetHighlightEffect(hovered);
}

void ViewImplantArch::slotSyncSagittalPositionUIStatusMovable(bool hovered) {
	sagittal_position_ui_->SetHighlight(hovered);
}

void ViewImplantArch::mousePressEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (View::IsSetTool() || sagittal_line_->IsActive())
		return BaseViewPanoArch::mousePressEvent(event);

	if (event->button() == Qt::LeftButton) {
		int implant_id = GetImplantHoveredID();
		if (implant_id > 0) {
			emit sigSelectImplant(implant_id);
		}
	}

	BaseViewPanoArch::mousePressEvent(event);
}

void ViewImplantArch::mouseReleaseEvent(QMouseEvent *event)
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

	if (View::IsSetTool() || IsMouseOnViewItems())
		return BaseViewPanoArch::mouseReleaseEvent(event);

	// sagittal line rotate event
	if (sagittal_line_->IsActive())
	{
		sagittal_line_->EndEvent();
	}
	else if (implant_handle_->IsActive())
	{
		implant_handle_->EndEvent();
	}
	else if (!sagittal_line_->isSelected() &&
		!sagittal_position_ui_->isSelected() &&
		!implant_handle_->isHovered())
	{
		// sagittal line position set event
		if (event->button() == Qt::LeftButton && GetImplantHoveredID() < 0 && !scene().IsMeasureSelected())
		{
			emit sigRequestPanoPosition(controller()->MapSceneToVol(pt_scene_current()));
		}
	}

	BaseViewPanoArch::mouseReleaseEvent(event);
}

void ViewImplantArch::mouseMoveEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPanoArch::mouseMoveEvent(event);

	if (IsSetTool())
		return;

	if (!implant_handle_->IsActive()) 
	{
		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		if (res_implant.data().empty())
		{
			return;
		}
		
		if (res_implant.is_visible_all()) 
		{
			const auto& implant_datas = res_implant.data();
			int implant_id = GetImplantHoveredID();

			if (implant_id > 0) 
			{
				UpdateSelectedImplantHandle(implant_datas, implant_id);
			}
			else 
			{
				int selected_id = res_implant.selected_implant_id();
				UpdateSelectedImplantHandle(implant_datas, selected_id);
			}
		}
	}
}

void ViewImplantArch::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPanoArch::keyPressEvent(event);
}

void ViewImplantArch::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE
	BaseViewPanoArch::keyReleaseEvent(event);
}


void ViewImplantArch::TransformItems(const QTransform & transform) {
	BaseViewPanoArch::TransformItems(transform);
	sagittal_line_center_ = sagittal_line_->TransformItems(transform);
	implant_handle_->TransformItems(transform);
	sagittal_position_ui_->setPos(transform.map(sagittal_position_ui_->pos()));
}

void ViewImplantArch::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	BaseViewPanoArch::SetCommonToolOnOff(type);
	if (type == common::CommonToolTypeOnOff::NONE) {
		implant_handle_->set_flag_movable(true);
		sagittal_line_->SetHighlight(true);
	} else {
		implant_handle_->set_flag_movable(false);
		sagittal_line_->SetHighlight(false);
	}
}
void ViewImplantArch::SetConnections() {
	connect(implant_handle_.get(), SIGNAL(sigTranslate()), this, SLOT(slotTranslateImplant()));
	connect(implant_handle_.get(), SIGNAL(sigRotate(float)), this, SLOT(slotRotateImplant(float)));
	connect(implant_handle_.get(), SIGNAL(sigUpdate()), this, SLOT(slotUpdateImplantPos()));

	connect(sagittal_line_.get(), SIGNAL(sigRotateLine(float)), this, SIGNAL(sigRotateSagittal(float)));
	connect(sagittal_line_.get(), &RotateLineItem::sigHighLightEvent,
			this, &ViewImplantArch::slotSyncSagittalPositionUIStatusMovable);
	connect(sagittal_position_ui_.get(), &CW3EllipseItem::sigHoverEllipse,
			this, &ViewImplantArch::slotSyncSagittalLineStatusMovable);
	connect(sagittal_position_ui_.get(), &CW3EllipseItem::sigRotateWithHandle,
			sagittal_line_.get(), &RotateLineItem::slotRotateWithHandle);
}

int ViewImplantArch::GetImplantHoveredID() {
	int hovered_id = -1;
	if (View::MakeCurrent()) {
		controller()->RenderAndPickImplant(&hovered_id);
		View::DoneCurrent();
	}

	return hovered_id;
}
/*	무조건 만든다. 밖에서 std::map 중복 체크 하고 들어와야 한다. */
void ViewImplantArch::CreateImplantSpec(ImplantData* implant_data) {
	if (!implant_data)
	{
		return;
	}

	int implant_id = implant_data->id();

	implant_specs()[implant_id].reset(new ImplantTextItem(implant_data));
	//implant_specs()[implant_id]->setZValue(10.0);
	scene().addItem(implant_specs()[implant_id].get());
}
QPointF ViewImplantArch::GetImplantScenePosition(const int & implant_id) const {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	glm::vec3 pt_vol = res_implant.data().at(implant_id)->position_in_vol();
	return controller()->MapVolToScene(pt_vol);
}

void ViewImplantArch::ApplyPreferences()
{
	BaseViewPanoArch::ApplyPreferences();
	UpdateImplantHandleAndSpec();
}
