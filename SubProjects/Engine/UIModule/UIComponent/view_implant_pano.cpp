#include "view_implant_pano.h"

#include <QDebug>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/pano_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../UIPrimitive/implant_handle.h"
#include "../UIPrimitive/implant_text_item.h"
#include "../UIViewController/view_controller_image.h"
#include "../UIViewController/view_controler_implant3d_pano.h"

#include "scene.h"

ViewImplantPano::ViewImplantPano(QWidget* parent)
	: BaseViewPano(parent), implant_handle_(new ImplantHandle) {
	this->SetControllerImage(new ViewControllerImage());
	controller_implant_3d_.reset(new ViewControllerImplant3Dpano);
	this->SetController3D(controller_implant_3d_);

	this->scene().addItem(implant_handle_.get());

	connect(implant_handle_.get(), SIGNAL(sigTranslate()), this, SLOT(slotTranslateImplant()));
	connect(implant_handle_.get(), SIGNAL(sigRotate(float)), this, SLOT(slotRotateImplant(float)));
	connect(implant_handle_.get(), SIGNAL(sigUpdate()), this, SLOT(slotUpdateImplantPos()));

	InitializeImplantMenu();
}

ViewImplantPano::~ViewImplantPano() {}


/**=================================================================================================
public functions
*===============================================================================================**/

/*
	불리는 조건
	1. Implant selection
	2. 임플란트가 이동했을 때 Handle 과 Spec 위치를 업데이트
*/

void ViewImplantPano::UpdateImplantHandleAndSpec() {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.size() == 0)
		return;

	int selected_id = res_implant.selected_implant_id();

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	BaseViewPano::UpdateNotSelectedImplantSpec(implant_datas, selected_id, res_implant.add_implant_id());

	// selected 인 spec text 및 handle의 visible 상태와 position을 결정
	BaseViewPano::UpdateSelectedImplantSpec(implant_datas, selected_id);
	this->UpdateSelectedImplantHandle(implant_datas, selected_id);
}

void ViewImplantPano::ChangeSelectedImplantSpec() {
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

void ViewImplantPano::DeleteImplant(int implant_id) {
	if (implant_handle_->selected_id() == implant_id)
		implant_handle_->Disable();

	BaseViewPano::DeleteImplant(implant_id);
}

void ViewImplantPano::DeleteAllImplants() {
	implant_handle_->Disable();

	BaseViewPano::DeleteAllImplants();
}

glm::mat4 ViewImplantPano::GetCameraMatrix() const {
	return controller_3d()->GetCameraMatrix();
}
bool ViewImplantPano::tmpIsRender2D() {
	if (recon_type() == ReconType::RECON_MPR) { return true; }
	else return false;
}

void ViewImplantPano::HideAllUI(bool is_hide) {
	BaseViewPano::HideAllUI(is_hide);

	if (implant_handle_->selected_id() > 0)
		implant_handle_->setVisible(!is_hide);

	/*bool text_visible = IsTextVisible();
	for (auto& spec : implant_specs())
		spec.second->setVisible(text_visible);*/
}

void ViewImplantPano::HideText(bool is_hide) {
	BaseViewPano::HideText(is_hide);

	/*bool text_visible = IsTextVisible();
	for (auto& spec : implant_specs())
		spec.second->setVisible(text_visible);*/
}

/**=================================================================================================
private functions
*===============================================================================================**/

void ViewImplantPano::InitializeImplantMenu() {
	menu_implant_.reset(new QMenu());
	act_delete_implant_.reset(new  QAction("Delete this implant"));
	menu_implant_->addAction(act_delete_implant_.get());
	connect(act_delete_implant_.get(), SIGNAL(triggered()), this, SLOT(slotDeleteImplant()));
}

void ViewImplantPano::UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
	int selected_id) {
	if (selected_id > 0) {
		auto iter = implant_datas.find(selected_id);
		if (iter != implant_datas.end()) {
			QPointF pt_scene_handle;
			switch (recon_type()) {
			case ReconType::RECON_MPR: {
				glm::vec3 pt_pano_plane = iter->second->position_in_pano_plane();
				pt_scene_handle = controller_image()->MapImageToScene(QPointF(pt_pano_plane.x, pt_pano_plane.y));
				implant_handle_->Enable(selected_id, IsUIVisible() && iter->second->is_visible(), pt_scene_handle);
				break; }
			case ReconType::RECON_3D: {
				glm::vec3 pt_pano_plane = iter->second->position_in_pano_plane();
				pt_scene_handle = controller_image()->MapImageToScene(QPointF(pt_pano_plane.x, pt_pano_plane.y));
				implant_handle_->Enable(selected_id, false, pt_scene_handle);
				break; }
			case ReconType::RECON_XRAY: {
				glm::vec3 pt_pano = iter->second->position_in_pano();
				pt_scene_handle = controller_image()->MapImageToScene(QPointF(pt_pano.x, pt_pano.y));
				implant_handle_->Enable(selected_id, IsUIVisible() && iter->second->is_visible(), pt_scene_handle);
				break; }
			default:
				assert(false);
				break;
			}
		}
		else {
			common::Logger::instance()->Print(common::LogType::ERR, "ViewImplantPano::UpdateSelectedImplantHandle");
			assert(false);
		}
	}
	else {
		implant_handle_->Disable();
	}
}

void ViewImplantPano::TransformItems(const QTransform & transform) {
	BaseViewPano::TransformItems(transform);
	implant_handle_->TransformItems(transform);
}

void ViewImplantPano::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	BaseViewPano::SetCommonToolOnOff(type);

	if (type == common::CommonToolTypeOnOff::NONE) {
		implant_handle_->set_flag_movable(true);
	}
	else {
		implant_handle_->set_flag_movable(false);
	}
}

//void ViewImplantPano::keyPressEvent(QKeyEvent* event)
//{
//	BaseViewPano::keyPressEvent(event);
//
//	if (event->key() == Qt::Key_Escape)
//	{
//		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
//		int add_implant_id = res_implant.add_implant_id();
//		if (add_implant_id < 1)
//		{
//			return;
//		}
//
//		emit sigDeleteImplant(add_implant_id);
//		UpdatedPano();
//	}
//}

void ViewImplantPano::mouseMoveEvent(QMouseEvent * event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPano::mouseMoveEvent(event);

	if (recon_type() == RECON_MPR)
	{
		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		int add_implant_id = res_implant.add_implant_id();

		if (add_implant_id > 0)
		{
			QPointF pt_scene = pt_scene_current();

			emit sigTranslateImplant(add_implant_id, controller_image()->MapSceneToImage(pt_scene));

			UpdatedPano();
		}
		else
		{
			if (!implant_handle_->IsActive() && res_implant.IsSetImplant())
			{
				int implant_id = GetImplantHoveredID(mapToScene(event->pos()));

				const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
				const auto& implant_datas = res_implant.data();

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
	else if (recon_type() == RECON_3D)
	{
		if (!IsEventRightButton(event) && IsEventImplantSelected())
		{
			PickAndMoveImplant();
		}
	}
}

void ViewImplantPano::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (View::IsSetTool())
	{
		return BaseViewPano::mousePressEvent(event);
	}

	int implant_id = GetImplantHoveredID(mapToScene(event->pos()));
	if (implant_id > 0)
	{
		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		if (event->button() == Qt::LeftButton)
		{
			bool is_changed_selected_id = implant_id != res_implant.selected_implant_id();
			if (is_changed_selected_id)
			{
				emit sigSelectImplant(implant_id);
			}
		}
		else if (event->button() == Qt::RightButton)
		{
			int add_implant_id = res_implant.add_implant_id();
			if (add_implant_id > 0)
			{
				return;
			}

			menu_implant_->popup(mapToGlobal(event->pos()) + QPoint(5, 5));
			QGraphicsView::mousePressEvent(event);
			return;
		}
	}

	BaseViewPano::mousePressEvent(event);
}

void ViewImplantPano::mouseReleaseEvent(QMouseEvent * event)
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
		return BaseViewPano::mouseReleaseEvent(event);
	}

	if (implant_handle_->IsActive())
	{
		implant_handle_->EndEvent();
	}

	if (recon_type() == RECON_MPR)
	{
		if (event->button() == Qt::LeftButton)
		{
			int add_implant_id = ResourceContainer::GetInstance()->GetImplantResource().add_implant_id();
			if (add_implant_id > 0)
			{
				emit sigPlacedImplant();
				this->UpdatedPano();

				if (implant_handle_ && implant_handle_->selected_id() != -1)
				{
					implant_handle_->setVisible(IsTextVisible() && true);
				}
			}
		}
	}
	else if (recon_type() == RECON_XRAY)
	{
		//TODO: implant를 select하는 동작에서 sigImplantHovered가 pano_plane좌표계에서 
		// 이루어지기 때문에 pano좌표계에서 하도록 변경해야한다. 

	}
	else if (recon_type() == RECON_3D)
	{
		if (event->button() == Qt::LeftButton)
		{
			this->SelectImplantIfRecon3D();

			if (controller_implant_3d_->IsPickImplant())
			{
				emit sigUpdateImplantImagesIn3D(controller_implant_3d_->GetPickImplantID());
			}
		}
	}

	BaseViewPano::mouseReleaseEvent(event);
}

void ViewImplantPano::keyPressEvent(QKeyEvent* event)
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
		BaseViewPano::keyPressEvent(event);

		if (event->key() == Qt::Key_Escape)
		{
			const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
			int add_implant_id = res_implant.add_implant_id();
			if (add_implant_id < 1)
			{
				return;
			}

			emit sigDeleteImplant(add_implant_id);
			UpdatedPano();
		}
	}
}

void ViewImplantPano::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPano::keyReleaseEvent(event);

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

		emit sigUpdateImplantImages(implant_id, controller_image()->MapSceneToImage(pt_scene_imp_handle));

		InputKeyClear();

		is_update_implant_ = false;
	}
}

void ViewImplantPano::leaveEvent(QEvent* event)
{
	BaseViewPano::leaveEvent(event);

	if (implant_handle_ && implant_handle_->selected_id() != -1)
	{
		implant_handle_->setVisible(false);
	}
}

void ViewImplantPano::enterEvent(QEvent* event)
{
	BaseViewPano::enterEvent(event);

	if (implant_handle_ && implant_handle_->selected_id() != -1)
	{
		implant_handle_->setVisible(IsTextVisible() && true);
	}
}

void ViewImplantPano::PickAndMoveImplant() {
	bool is_need_render_vol = false;

	if (View::MakeCurrent()) {
		if (is_pressed()) {
			glm::vec3 delta_translate;
			glm::vec3 rotate_axes;
			float delta_degree = 0.0f;
			int implant_id = -1;

			controller_implant_3d_->MoveImplant(&implant_id, &delta_translate, &rotate_axes, &delta_degree);

			if (delta_translate != glm::vec3()) {
				emit sigTranslateImplantIn3D(implant_id, delta_translate);
				is_need_render_vol = true;

				BaseViewPano::UpdateSelectedImplantSpec(ResourceContainer::GetInstance()->GetImplantResource().data(),
					implant_id);
			}
			if (delta_degree != 0.0f) {
				emit sigRotateImplantIn3D(implant_id, rotate_axes, delta_degree);
				is_need_render_vol = true;

				BaseViewPano::UpdateSelectedImplantSpec(ResourceContainer::GetInstance()->GetImplantResource().data(),
					implant_id);
			}
		}
		else {
			controller_implant_3d_->PickAxesItem(&is_need_render_vol);
		}
	}View::DoneCurrent();

	if (is_need_render_vol) {
		RenderPanoVolume();
		scene().update();
	}
}

void ViewImplantPano::RenderPanoVolume() {
	BaseViewPano::RenderPanoVolume();

	if (!View::IsEnableGLContext()) return;

	View::MakeCurrent();
	controller_implant_3d_->RenderForPickAxes();
	View::DoneCurrent();

}

bool ViewImplantPano::IsEventImplantSelected() const {
	return (controller_implant_3d_->is_selected_implant() && !View::IsSetTool()) ? true : false;
}

void ViewImplantPano::SelectImplantIfRecon3D() {
	if (recon_type() != RECON_3D) {
		assert(false);
		return;
	}

	bool is_update_scene = false;
	View::MakeCurrent(); {
		int selected_id = -1;
		controller_implant_3d_->SelectImplant(&selected_id);

		if (selected_id > 0) {
			emit sigSelectImplant(selected_id);
		}
	}View::DoneCurrent();

	if (is_update_scene) {
		RenderPanoVolume();
		scene().update();
	}
}

int ViewImplantPano::GetImplantHoveredID(const QPointF& pt_scene) const {
	int hovered_id = -1;

	if (ctrl_type() != RECON_MPR)
		return hovered_id;

	// implant handle이 호버이거나 선택된 상태이면 핸들의 아이디를 가져오고
	// 아니면 PanoEngine의 Picking 알고리즘 결과를 가져온다.
	if (implant_handle_->is_hovered() || implant_handle_->isSelected()) {
		hovered_id = implant_handle_->selected_id();
	}
	else {
		emit sigImplantHovered(controller_image()->MapSceneToImage(pt_scene), &hovered_id);
	}

	return hovered_id;
}

void ViewImplantPano::TranslationDetailedControlImplant()
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

	emit sigTranslateImplant(implant_id, controller_image()->MapSceneToImage(implant_set_pos));

	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));

	UpdatedPano();

	is_update_implant_ = true;
}

void ViewImplantPano::RotationDetailedControlImplant()
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

	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));

	UpdatedPano();

	is_update_implant_ = true;
}

void ViewImplantPano::InputKeyClear()
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

/**=================================================================================================
private slots
*===============================================================================================**/
void ViewImplantPano::slotTranslateImplant() {
	int implant_id = implant_handle_->selected_id();
	QPointF implant_scene_pos = implant_handle_->scenePos();
	emit sigTranslateImplant(implant_id,
		controller_image()->MapSceneToImage(implant_scene_pos));
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	UpdatedPano();
}

void ViewImplantPano::slotRotateImplant(float degree_angle) {
	emit sigRotateImplant(implant_handle_->selected_id(), degree_angle);
	int implant_id = implant_handle_->selected_id();
	implant_specs()[implant_id]->setPos(GetImplantSpecPosition(implant_id, true));
	UpdatedPano();
}

//마우스가 handle에서 벗어난 위치에서 release되었을 때 implant를 업데이트하기 위한 함수.
void ViewImplantPano::slotUpdateImplantPos() {
	int implant_id = implant_handle_->selected_id();
	QPointF pt_scene_imp_handle = implant_handle_->scenePos();
	emit sigUpdateImplantImages(implant_id,
		controller_image()->MapSceneToImage(pt_scene_imp_handle));
	UpdatedPano();
}

void ViewImplantPano::slotDeleteImplant() {
	int implant_id = implant_handle_->selected_id();
	if (implant_id < 0)
		return;

	emit sigDeleteImplant(implant_id);
	UpdatedPano();
}
