#include "view_pano.h"

#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../UIViewController/view_controller_image.h"
#include "../UIViewController/view_controller_pano3d.h"
#include "../UIPrimitive/pano_nerve_item.h"
#include "../UIPrimitive/guide_line_list_item.h"
#include "../UIPrimitive/implant_text_item.h"

#include "scene.h"

using namespace UIViewController;

namespace {
	const int kMousePressInterval = 300;
}

ViewPano::ViewPano(QWidget* parent)
	: BaseViewPano(parent) {

	this->SetControllerImage(new ViewControllerImage());
	this->SetController3D(new ViewControllerPano3D());

	nerve_.reset(new PanoNerveItem);
	nerve_->setZValue(10);
	connect(nerve_.get(), SIGNAL(sigTranslatedNerveEllipse(int, int, QPointF)), this, SLOT(slotTranslatedNerveEllipse(int, int, QPointF)));
	connect(nerve_.get(), SIGNAL(sigPressedNerveEllipse(int, int, bool)), this, SIGNAL(sigModifyNerveEllipse(int, int, bool)));
	connect(nerve_.get(), SIGNAL(sigAddNerveEllipse(int, QPointF)), this, SLOT(slotAddedNerveEllipse(int, QPointF)));
	connect(nerve_.get(), SIGNAL(sigCancelLastNerveEllipse(int, int)), this, SIGNAL(sigCancelLastNerveEllipse(int, int)));
	connect(nerve_.get(), SIGNAL(sigClearNerve(int)), this, SIGNAL(sigClearedNerve(int)));
	connect(nerve_.get(), SIGNAL(sigRemoveNerveEllipse(int, int)), this, SIGNAL(sigRemovedNerveEllipse(int, int)));
	connect(nerve_.get(), SIGNAL(sigInserteNerveEllipse(int, int, QPointF)), this, SLOT(slotInsertedNerveEllipse(int, int, QPointF)));

	scene().addItem(nerve_.get());

	mouse_pressed_timer_.reset(new QTimer);
	connect(mouse_pressed_timer_.get(), SIGNAL(timeout()), this, SLOT(slotMousePressedTimeOut()));

	InitializeNerveMenus();
}

ViewPano::~ViewPano() {}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewPano::ReleaseSelectedNerve() {
	nerve_->ReleaseSelectedNerve();
}
void ViewPano::SetEditNerveMode(const bool& edit) {
	is_edit_nerve_mode_ = edit;
	nerve_->SetHighlight(edit);
	//nerve_->SetVisibleAll(edit);
	if (!edit) {
		nerve_->CancelCurrentNerve();
	}
}

void ViewPano::SetNerveHover(int nerve_id, bool is_hover) {
	nerve_->SetHover(nerve_id, is_hover);
}
void ViewPano::SetVisibleNerve(int nerve_id, bool is_visible) {
	nerve_->SetVisible(nerve_id, is_visible);
}
void ViewPano::SetVisibleNerveAll(bool is_visible) {
	nerve_->SetVisibleAll(is_visible);
}

void ViewPano::AddNerveEllipseFromCrossSection(const QPointF& pt_pano_plane) {
	if (!is_edit_nerve_mode_)
		return;

	AddNervePoint(controller_image()->MapImageToScene(pt_pano_plane));
}

void ViewPano::EndEditNerveFromCromssSection() {
	if (!is_edit_nerve_mode_)
		return;

	EndEditNerve();
}
void ViewPano::ClearUnfinishedNerve() {
	nerve_->CancelCurrentNerve();
}
void ViewPano::ClearNerve(int nerve_id) {
	nerve_->Clear(nerve_id);
}

void ViewPano::ClearAllNerve() {
	nerve_->ClearAll();
}

void ViewPano::PressedKeyESC() {
	if (IsEventCancelLastNervePoint())
		this->CancelLastNervePoint();
	if (!IsEventEditNerve() && is_edit_nerve_mode_)
		ReleaseSelectedNerve();
}

void ViewPano::UpdateNerveCtrlPoints(const std::map<int, std::vector<QPointF>>& ctrl_points_in_pano_plane) {
	std::map<int, std::vector<QPointF>> ctrl_points_in_scene;

	for (const auto& elem : ctrl_points_in_pano_plane)
		controller_image()->MapImageToScene(elem.second, ctrl_points_in_scene[elem.first]);

	nerve_->SetControlPoints(ctrl_points_in_scene);
}

void ViewPano::GetNerveCtrlPointsInPanoPlane(int id, std::vector<QPointF>& dst_ctrl_points_in_pano_plane) {
	const std::vector<QPointF>& ctrl_points_in_scene = nerve_->GetControlPoints(id);
	controller_image()->MapSceneToImage(ctrl_points_in_scene, dst_ctrl_points_in_pano_plane);
}

int ViewPano::GetCurrentNerveID() const {
	return nerve_->GetAvailableNerveID();
}
/*
불리는 조건
1. UpdatedPano함수
2. Recon Type 바뀌었을 때
*/
void ViewPano::UpdateImplantHandleAndSpec() {
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.empty())
	{
		DeleteAllImplants();
		return;
	}

	int selected_id = res_implant.selected_implant_id();

	// selected 가 아닌 Spec Text들의 visible 상태와 position을 결정
	BaseViewPano::UpdateNotSelectedImplantSpec(implant_datas, selected_id, res_implant.add_implant_id());

	// selected 인 spec text 상태와 position을 결정
	BaseViewPano::UpdateSelectedImplantSpec(implant_datas, selected_id);
}

void ViewPano::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	BaseViewPano::SetCommonToolOnOff(type);

	if (is_edit_nerve_mode_) {
		if (type == common::CommonToolTypeOnOff::NONE)
			nerve_->SetHighlight(true);
		else
			nerve_->SetHighlight(false);
	}
}

/**=================================================================================================
protected functions
*===============================================================================================**/

void ViewPano::slotActiveFilteredItem(const QString& text) {
	BaseViewPano::slotActiveFilteredItem(text);

	if (text == filtered_texts(RECON_MPR)) {
		nerve_->setVisible(true);

	}
	else if (text == filtered_texts(RECON_XRAY)) {
		nerve_->setVisible(false);
	}
	else if (text == filtered_texts(RECON_3D)) {
		nerve_->setVisible(false);
	}
}

/**=================================================================================================
private functions
*===============================================================================================**/

void ViewPano::InitializeNerveMenus() {
	menu_nerve_ell_.reset(new QMenu());
	menu_nerve_spl_.reset(new QMenu());

	menu_act_delete_nerve_.reset(new  QAction("Delete this nerve"));
	menu_act_remove_pnt_nerve_.reset(new  QAction("Remove the control point"));
	menu_act_insert_pnt_nerve_.reset(new QAction("Insert the control point"));

	menu_nerve_spl_->addAction(menu_act_insert_pnt_nerve_.get());
	menu_nerve_spl_->addAction(menu_act_delete_nerve_.get());

	menu_nerve_ell_->addAction(menu_act_remove_pnt_nerve_.get());
	menu_nerve_ell_->addAction(menu_act_delete_nerve_.get());

	connect(menu_act_delete_nerve_.get(), SIGNAL(triggered()), this, SLOT(slotDeleteNerveFromQAction()));
	connect(menu_act_remove_pnt_nerve_.get(), SIGNAL(triggered()), this, SLOT(slotRemovePointNerveFromQAction()));
	connect(menu_act_insert_pnt_nerve_.get(), SIGNAL(triggered()), this, SLOT(slotInsertPointNerveFromQAction()));
}
void ViewPano::TransformItems(const QTransform & transform) {
	BaseViewPano::TransformItems(transform);

	if (ctrl_type() == CTRL_IMAGE) {
		nerve_->TransformItem(transform);
	}
}

void ViewPano::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (IsEventRightButton(event) && !IsEventCrossSectionHovered() && !IsEventEditNerve())
	{
		if (IsEventNerveHoveredPoint())
		{
			nerve_->SaveCurrentHoveredPointIndex();
			menu_nerve_ell_->popup(mapToGlobal(event->pos()) + QPoint(5, 5));
			QGraphicsView::mousePressEvent(event);
			return;
		}
		else if (IsEventNerveHoveredLine())
		{
			menu_nerve_spl_->popup(mapToGlobal(event->pos()) + QPoint(5, 5));
			QGraphicsView::mousePressEvent(event);
			return;
		}
	}

	BaseViewPano::mousePressEvent(event);

	is_mouse_preseed_time_out_ = false;
	mouse_pressed_timer_->start(kMousePressInterval);
}

void ViewPano::mouseReleaseEvent(QMouseEvent* event)
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

	if (IsEventLeftButton(event))
	{
		if (IsEventAddNerve())
		{
			QPointF pt_scene = mapToScene(event->pos());
			if (controller_image()->IsCursorInImage(pt_scene))
			{
				AddNervePoint(pt_scene);
			}
			else
			{
				CW3MessageBox msg_box(QString("Will3D"), QString("Invalid position."), CW3MessageBox::Information);
				msg_box.exec();
			}
		}

		if (!IsEventNerveHovered() && !IsEventCrossSectionHovered() && !IsEventAxialLineHovered())
			nerve_->ReleaseSelectedNerve();
	}

	BaseViewPano::mouseReleaseEvent(event);
}

void ViewPano::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (IsEventLeftButton(event) && IsEventEditNerve())
	{
		EndEditNerve();
	}
	BaseViewPano::mouseDoubleClickEvent(event);
}

void ViewPano::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	BaseViewPano::mouseMoveEvent(event);
}

void ViewPano::keyPressEvent(QKeyEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (event->key() == Qt::Key_Escape) 
	{
		PressedKeyESC();
	}

	BaseViewPano::keyPressEvent(event);
}

void ViewPano::keyReleaseEvent(QKeyEvent* event)
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
}

bool ViewPano::IsEventAddNerve() const {
	if (recon_type() == RECON_MPR &&
		controller_image()->IsReady() &&
		is_edit_nerve_mode_ &&
		!is_pressed_double_click() &&
		!View::IsSetTool()) {

		if (!is_mouse_preseed_time_out_)
			return true;

		if (!cross_line()->is_highlight() &&
			!axial_line()->is_highlight())
			return true;
	}

	return false;
}

bool ViewPano::IsEventCancelLastNervePoint() const {
	if (recon_type() == RECON_MPR &&
		controller_image()->IsReady() &&
		is_edit_nerve_mode_ &&
		!View::IsSetTool()) {
		return true;
	}

	return false;

}

bool ViewPano::IsEventEditNerve() const {
	return (recon_type() == RECON_MPR &&
		nerve_->IsEdit() &&
		!View::IsSetTool()) ? true : false;
}

bool ViewPano::IsEventNerveHovered() const {
	return (recon_type() == RECON_MPR &&
		(nerve_->IsHoveredLine() || nerve_->IsHoveredPoint()) &&
		!View::IsSetTool()) ? true : false;
}
bool ViewPano::IsEventNerveHoveredLine() const {
	return (recon_type() == RECON_MPR &&
		nerve_->IsHoveredLine() &&
		!View::IsSetTool()) ? true : false;
}

bool ViewPano::IsEventNerveHoveredPoint() const {
	return (recon_type() == RECON_MPR &&
		nerve_->IsHoveredPoint() &&
		!View::IsSetTool()) ? true : false;
}

void ViewPano::AddNervePoint(const QPointF& pt_scene) {
	nerve_->AddPoint(pt_scene);
}


void ViewPano::EndEditNerve() {
	bool is_success_end_edit = nerve_->EndEdit();

	if (is_success_end_edit)
		emit sigEndEditedNerve(nerve_->curr_nerve_id());
}

void ViewPano::CancelLastNervePoint() {
	int current_id = nerve_->curr_nerve_id();
	int removed_index;
	nerve_->CancelLastPoint(current_id);
}
/**=================================================================================================
private slots
*===============================================================================================**/

void ViewPano::slotDeleteNerveFromQAction() {
	int current_id = nerve_->curr_nerve_id();
	nerve_->ReleaseSelectedNerve();
	nerve_->Clear(current_id);
}
void ViewPano::slotRemovePointNerveFromQAction() {
	int current_id = nerve_->curr_nerve_id();
	nerve_->ReleaseSelectedNerve();
	nerve_->RemoveSelectedPoint(current_id);
}
void ViewPano::slotInsertPointNerveFromQAction() {
	int current_id = nerve_->curr_nerve_id();
	QPointF pt_scene = pt_scene_current();
	nerve_->ReleaseSelectedNerve();
	nerve_->InsertCloserPoint(current_id, pt_scene);
}

void ViewPano::slotAddedNerveEllipse(int nerve_id, const QPointF& pt_scene) {
	if (!controller_image()->IsCursorInImage(pt_scene))
		return;

	QPointF pt_in_pano_plane = controller_image()->MapSceneToImage(pt_scene);
	emit sigAddedNerveEllipse(nerve_id, pt_in_pano_plane);
}

void ViewPano::slotInsertedNerveEllipse(int nerve_id, int insert_index, const QPointF & pt_scene) {
	if (!controller_image()->IsCursorInImage(pt_scene))
		return;

	QPointF pt_in_pano_plane = controller_image()->MapSceneToImage(pt_scene);
	emit sigInsertedNerveEllipse(nerve_id, insert_index, pt_in_pano_plane);
}

void ViewPano::slotTranslatedNerveEllipse(int nerve_id, int nerve_selected_index, const QPointF& pt_scene) {
	if (!controller_image()->IsCursorInImage(pt_scene))
		return;

	QPointF pt_pano_plane = controller_image()->MapSceneToImage(pt_scene);
	emit sigTranslatedNerveEllipse(nerve_id, nerve_selected_index, pt_pano_plane);
}

void ViewPano::slotMousePressedTimeOut() {
	is_mouse_preseed_time_out_ = true;
	mouse_pressed_timer_->stop();
}
