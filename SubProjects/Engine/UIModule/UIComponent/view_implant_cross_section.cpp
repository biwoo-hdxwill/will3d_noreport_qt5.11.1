#include "view_implant_cross_section.h"

#include <QDebug>
#include <QMouseEvent>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/sagittal_resource.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../UIPrimitive/implant_handle.h"
#include "../UIPrimitive/guide_line_list_item.h"
#include "../UIPrimitive/implant_text_item.h"
#include "../UIPrimitive/W3EllipseItem.h"
#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace {
	const int kLineID = 0;
}

ViewImplantCrossSection::ViewImplantCrossSection(int cross_section_id, QWidget* parent) :
	BaseViewPanoCrossSection(cross_section_id, parent),
	implant_handle_(new ImplantHandle) {

	this->scene().addItem(implant_handle_.get());
	connect(implant_handle_.get(), SIGNAL(sigTranslate()), this, SLOT(slotTranslateImplant()));
	connect(implant_handle_.get(), SIGNAL(sigRotate(float)), this, SLOT(slotRotateImplant(float)));
	connect(implant_handle_.get(), SIGNAL(sigUpdate()), this, SLOT(slotUpdateImplantPos()));
}
ViewImplantCrossSection::~ViewImplantCrossSection() {
}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewImplantCrossSection::HideAllUI(bool is_hide) {
	BaseViewPanoCrossSection::HideAllUI(is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantCrossSection::HideText(bool is_hide) {
	BaseViewPanoCrossSection::HideText(is_hide);

	UpdateImplantHandleAndSpec();
}

void ViewImplantCrossSection::ChangeSelectedImplantSpec() {
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

void ViewImplantCrossSection::UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id) {
	if (selected_id > 0) {
		auto iter = implant_datas.find(selected_id);
		if (iter != implant_datas.end()) {
			const std::map<int, bool>& visibility_map = controller_slice()->GetImplantVisibility();
			bool visibility = true;
			if (visibility_map.find(selected_id) != visibility_map.end())
			{
				visibility = visibility_map.at(selected_id);
			}

			QPointF pt_scene = controller_slice()->MapVolToScene(iter->second->position_in_vol());
			implant_handle_->Enable(selected_id, visibility && IsUIVisible() && iter->second->is_visible(), pt_scene);
		}
		else {
			common::Logger::instance()->Print(common::LogType::ERR, "ViewImplantArch::UpdateImplantHandleAndSpec");
			assert(false);
		}
	}
	else {
		implant_handle_->Disable();
	}
}

void ViewImplantCrossSection::DeleteImplant(int implant_id) {
	if (implant_handle_->selected_id() == implant_id) {
		implant_handle_->Disable();
	}
	DeleteImplantSpec(implant_id);

	RenderSlice();
}

void ViewImplantCrossSection::DeleteAllImplants() {
	implant_handle_->Disable();
	DeleteImplantSpecs();

	RenderSlice();
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void ViewImplantCrossSection::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	BaseViewPanoCrossSection::SetCommonToolOnOff(type);

	if (type == common::CommonToolTypeOnOff::NONE) {
		implant_handle_->set_flag_movable(true);
	}
	else {
		implant_handle_->set_flag_movable(false);
	}
}


void ViewImplantCrossSection::mouseMoveEvent(QMouseEvent * event) {
	BaseViewPanoCrossSection::mouseMoveEvent(event);

	if (IsSetTool())
		return;

#if 1
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int add_implant_id = res_implant.add_implant_id();

	if (add_implant_id > 0)
	{
		emit sigSetImplantPosition(add_implant_id, controller_slice()->MapSceneToVol(mapToScene(event->pos())));

		RenderSlice();
		scene().update();
	}
	else
	{
		if (!implant_handle_->IsActive() && res_implant.IsSetImplant())
		{
			const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
			const auto& implant_datas = res_implant.data();

			int selected_implant_id = res_implant.selected_implant_id();
#if 0
			int hovered_implant_id = GetImplantHoveredID();

			if (hovered_implant_id > 0 && hovered_implant_id == selected_implant_id)
			{
				UpdateSelectedImplantHandle(implant_datas, hovered_implant_id);
			}
			else
			{
				UpdateSelectedImplantHandle(implant_datas, selected_implant_id);
			}
#else
			UpdateSelectedImplantHandle(implant_datas, selected_implant_id);
#endif
		}
	}
#else
	if (!implant_handle_->IsActive()) {
		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();

		if (res_implant.is_visible_all()) {
			const auto& implant_datas = res_implant.data();
			int implant_id = GetImplantHoveredID();

			if (implant_id > 0) {
				UpdateSelectedImplantHandle(implant_datas, implant_id);
			}
			else {
				int selected_id = res_implant.selected_implant_id();
				UpdateSelectedImplantHandle(implant_datas, selected_id);
			}
		}
	}
#endif
}

void ViewImplantCrossSection::mousePressEvent(QMouseEvent * event) {
	if (View::IsSetTool())
		return BaseViewPanoCrossSection::mousePressEvent(event);

	if (event->button() == Qt::LeftButton) {
		int implant_id = GetImplantHoveredID();
		if (implant_id > 0) {
			//emit sigSelectImplant(implant_id, cross_section_id());
		}
	}

	BaseViewPanoCrossSection::mousePressEvent(event);
}

void ViewImplantCrossSection::mouseReleaseEvent(QMouseEvent * event) 
{
	if (event->button() != Qt::LeftButton || BaseViewPanoCrossSection::IsSetTool())
	{
		return BaseViewPanoCrossSection::mouseReleaseEvent(event);
	}

	if (implant_handle_->IsActive()) 
	{
		implant_handle_->EndEvent();
	}

	if (event->button() == Qt::LeftButton)
	{
		int add_implant_id = ResourceContainer::GetInstance()->GetImplantResource().add_implant_id();
		if (add_implant_id > 0)
		{
			emit sigPlacedImplant();

			RenderSlice();
			UpdateImplantHandleAndSpec();

			ImplantHandleVisible();

			scene().update();		
		}
		else
		{
			int implant_id = GetImplantHoveredID();
			if (implant_id > 0)
			{
				//emit sigSelectImplant(implant_id, cross_section_id());				
			}
		}
	}

	BaseViewPanoCrossSection::mouseReleaseEvent(event);
}
/**=================================================================================================
private functions
*===============================================================================================**/

void ViewImplantCrossSection::TransformItems(const QTransform & transform) {
	BaseViewPanoCrossSection::TransformItems(transform);
	implant_handle_->TransformItems(transform);
}

int ViewImplantCrossSection::GetImplantHoveredID() {
	int hovered_id = -1;
	if (View::MakeCurrent()) {
		controller_slice()->RenderAndPickImplant(&hovered_id);
		View::DoneCurrent();
	}

	return hovered_id;
}
QPointF ViewImplantCrossSection::GetImplantScenePosition(const int & implant_id) {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	glm::vec3 pt_vol = res_implant.data().at(implant_id)->position_in_vol();
	return controller_slice()->MapVolToScene(pt_vol);
}

void ViewImplantCrossSection::TranslationDetailedControlImplant()
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

	QPointF prev_scene_pos = mapToScene(implant_handle_->scenePos().x(), implant_handle_->scenePos().y());

	QPointF scene_trans = view_render_param()->MapGLToScene(trans);
	QPointF implant_set_pos = implant_handle_->scenePos() + scene_trans;

	implant_handle_->setPos(implant_set_pos);

	QPointF curr_scene_pos = mapToScene(implant_set_pos.x(), implant_set_pos.y());

	glm::vec3 translate = controller_slice()->MapSceneToVol(curr_scene_pos) - controller_slice()->MapSceneToVol(prev_scene_pos);

	emit sigTranslateImplant(implant_handle_->selected_id(), translate);

	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));

	RenderSlice();

	is_update_implant_ = true;
}

void ViewImplantCrossSection::RotationDetailedControlImplant()
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

	emit sigRotateImplant(implant_handle_->selected_id(), controller_slice()->GetUpVector(), degree);

	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	RenderSlice();

	is_update_implant_ = true;
}

void ViewImplantCrossSection::InputKeyClear()
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

void ViewImplantCrossSection::ImplantHandleVisible()
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

void ViewImplantCrossSection::slotTranslateImplant() {
	int implant_id = implant_handle_->selected_id();
#if 0
	emit sigTranslateImplant(implant_handle_->selected_id(),
		controller_slice()->MapSceneToVol(implant_handle_->scenePos()));
#else
	// 1. [이동량과 up vector] or [3d 좌표상의 이동량] 를 넘겨서
	// 2. implant 가 현재 cross section plane 으로 이동하지 않고
	// 3. implant 중심을 지나는 cross section plane 과 평행한 평면 위에서 움직이도록 함
	QPointF curr_scene_pos = mapToScene(implant_handle_->pos().x(), implant_handle_->pos().y());
	QPointF prev_scene_pos = mapToScene(implant_handle_->prev_pos().x(), implant_handle_->prev_pos().y());
	glm::vec3 translate = controller_slice()->MapSceneToVol(curr_scene_pos) - controller_slice()->MapSceneToVol(prev_scene_pos);

	emit sigTranslateImplant(implant_handle_->selected_id(), translate);
#endif
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	RenderSlice();
}

void ViewImplantCrossSection::slotRotateImplant(float degree_angle) {
	emit sigRotateImplant(implant_handle_->selected_id(),
		controller_slice()->GetUpVector(), degree_angle);
	int implant_id = implant_handle_->selected_id();
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	this->RenderSlice();
}

void ViewImplantCrossSection::slotUpdateImplantPos() {
#if 0
	emit sigUpdateImplantImages(implant_handle_->selected_id(),
		controller_slice()->MapSceneToVol(implant_handle_->scenePos()));
#else
	// 1. [이동량과 up vector] or [3d 좌표상의 이동량] 를 넘겨서
	// 2. implant 가 현재 cross section plane 으로 이동하지 않고
	// 3. implant 중심을 지나는 cross section plane 과 평행한 평면 위에서 움직이도록 함
	QPointF curr_scene_pos = mapToScene(implant_handle_->pos().x(), implant_handle_->pos().y());
	QPointF prev_scene_pos = mapToScene(implant_handle_->prev_pos().x(), implant_handle_->prev_pos().y());
	glm::vec3 translate = controller_slice()->MapSceneToVol(curr_scene_pos) - controller_slice()->MapSceneToVol(prev_scene_pos);

	emit sigUpdateImplantImages(implant_handle_->selected_id(), translate);
#endif
	this->RenderSlice();
	scene().update();
}

void ViewImplantCrossSection::enterEvent(QEvent* event)
{
	BaseViewPanoCrossSection::enterEvent(event);

	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	int add_implant_id = res_implant.add_implant_id();

	auto iter = implant_datas.find(add_implant_id);
#if 0
	if (iter == implant_datas.end())
	{
		return;
	}

	iter->second->set_is_visible(true);

	UpdateCrossSection();
#else
	if (iter != implant_datas.end())
	{
		iter->second->set_is_visible(true);
		UpdateCrossSection();
	}
	else
	{
		ImplantHandleVisible();
	}		
#endif	
}

void ViewImplantCrossSection::leaveEvent(QEvent* event)
{
	BaseViewPanoCrossSection::leaveEvent(event);

	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	int add_implant_id = res_implant.add_implant_id();

	auto iter = implant_datas.find(add_implant_id);
#if 0
	if (iter == implant_datas.end())
	{
		return;
	}

	iter->second->set_is_visible(false);

	UpdateCrossSection();

	iter->second->set_is_visible(true);
#else
	if (iter != implant_datas.end())
	{
		iter->second->set_is_visible(false);
		UpdateCrossSection();
		iter->second->set_is_visible(true);
	}
	else
	{
		if (implant_handle_)
		{
			implant_handle_->setVisible(false);
		}
	}
#endif	
}

void ViewImplantCrossSection::wheelEvent(QWheelEvent* event)
{
	BaseViewPanoCrossSection::wheelEvent(event);

	ImplantHandleVisible();
}

void ViewImplantCrossSection::keyPressEvent(QKeyEvent* event)
{
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
		BaseViewPanoCrossSection::keyPressEvent(event);

		if (event->key() == Qt::Key_Escape)
		{
			const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
			int add_implant_id = res_implant.add_implant_id();
			if (add_implant_id < 1)
			{
				return;
			}

			emit sigDeleteImplant(add_implant_id);
			UpdateCrossSection();
		}
	}
}

void ViewImplantCrossSection::keyReleaseEvent(QKeyEvent* event)
{
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

		emit sigUpdateImplantImages(implant_id, glm::vec3(0.f));

		InputKeyClear();

		is_update_implant_ = false;
	}
}
