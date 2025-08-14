#include "view_tmj_lateral.h"

#include <QDebug>
#include <QMouseEvent>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/tmj_resource.h"

#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/guide_line_list_item.h"

#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace {
	const int kLineID = 0;
}

ViewTmjLateral::ViewTmjLateral(int lateral_id,
	const common::ViewTypeID& view_type,
	TMJDirectionType type, QWidget* parent)
	: lateral_id_(lateral_id), direction_type_(type), View(view_type, parent) {
	controller_slice_.reset(new ViewControllerSlice);
	controller_slice_->set_view_param(view_render_param());

	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	if (lateral_id_ == 0)
	{
		scene().InitViewItem(Viewitems::DIRECTION_TEXT);
		if (type == TMJ_LEFT)
			scene().SetDirectionTextItem(QString("A"), true);
		else
			scene().SetDirectionTextItem(QString("A"), false);
	}
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitViewItem(Viewitems::GRID);

	scene().InitMeasure(view_type);
	connect(&scene(), &Scene::sigMeasureCreated,
		[=](const unsigned int& measure_id) {
		emit sigMeasureCreated(direction_type_, lateral_id_, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureDeleted,
		[=](const unsigned int& measure_id) {
		emit sigMeasureDeleted(direction_type_, lateral_id_, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureModified,
		[=](const unsigned int& measure_id) {
		emit sigMeasureModified(direction_type_, lateral_id_, measure_id);
	});

	scene().SetBorderColor(ColorView::kTMJLateral);
	scene().SetRulerColor(ColorView::kTMJLateral);

	text_id_.reset(new CW3TextItem(false));
	text_id_->setTextColor(ColorView::kTMJLateral);
	scene().addItem(text_id_.get());

	reference_frontal_line_.reset(
		new GuideLineListItem(GuideLineListItem::VERTICAL));
	reference_frontal_line_->setZValue(0);
	reference_frontal_line_->SetHighlight(true);
	reference_frontal_line_->set_pen(
		QPen(ColorTmjItem::kFrontalLineNormal, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_frontal_line_.get());

	reference_axial_line_.reset(
		new GuideLineListItem(GuideLineListItem::HORIZONTAL));
	reference_axial_line_->setZValue(0);
	reference_axial_line_->SetHighlight(true);
	reference_axial_line_->set_pen(
		QPen(ColorTmjItem::kAxialLine, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_axial_line_.get());

	SetSelectedStatus(false);
}

ViewTmjLateral::~ViewTmjLateral() {
	if (View::IsEnableGLContext()) {
		View::MakeCurrent();
		controller_slice_->ClearGL();
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/
void ViewTmjLateral::SetEnabledSharpenUI(const bool& is_enabled) {
	scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, is_enabled);
}
void ViewTmjLateral::SetSharpen(const SharpenLevel& level) {
	scene().ChangeSharpenLevel(static_cast<int>(level));
	controller_slice_->SetSharpenLevel(level);
	RenderSlice();
	scene().update();
}

void ViewTmjLateral::SetSelectedStatus(const bool& selected) {
	is_selected_ = selected;
	SetTMJUISelectedStatus();
}

void ViewTmjLateral::UpdateThickness() {
	if (!isVisible()) return;

	QPen frontal_line(ColorTmjItem::kFrontalLineNormal, 2.0, Qt::SolidLine,
		Qt::FlatCap);
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr) return;

	float scene_thickness =
		view_render_param()->MapActualToScene(res_lateral.param().thickness);
	if (scene_thickness >= 2.0) {
		frontal_line.setWidthF(scene_thickness);
	}
	reference_frontal_line_->set_pen(frontal_line);
	reference_frontal_line_->UpdateLine(kLineID);
}

void ViewTmjLateral::UpdateUI() {
	if (!IsValidLateralResource()) {
		reference_axial_line_->ClearLines();
		reference_frontal_line_->ClearLines();
		text_id_->setVisible(false);
		return;
	}
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	const auto& res_lateral = res_tmj.lateral(direction_type_);

	if (!res_lateral.IsValidCenterPosition(lateral_id_)) {
		reference_axial_line_->ClearLines();
		return;
	}

	int lateral_num;
	res_lateral.GetNumber(lateral_id_, &lateral_num);
	QString id_text = text_id_->toPlainText();
	if (!is_selected_) {
		if (id_text.toInt() != lateral_num) {
			text_id_->setPlainText(QString("%1").arg(lateral_num));
			text_id_->setVisible(IsTextVisible());
		}
		return;
	}

	if (id_text.toInt() != lateral_num) {
		text_id_->setPlainText(QString("%1").arg(lateral_num));
		text_id_->setVisible(IsTextVisible());
	}

	const double width = (double)res_lateral.param().width;
	const double height = (double)GetTMJresource().height();

	const glm::vec3 axial_pos_vol = res_tmj.axial_position();
	QPointF axial_pos_scene = controller_slice_->MapVolToScene(axial_pos_vol);

	QPointF line_left_top = controller_slice_->MapPlaneToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom =
		controller_slice_->MapPlaneToScene(QPointF(width, height));
	axial_pos_scene.setX((line_left_top.x() + line_right_bottom.x()) * 0.5);

	auto iter = reference_axial_line_->line_positions().find(kLineID);
	if (iter == reference_axial_line_->line_positions().end() ||
		iter->second != axial_pos_scene) {
		float len_width = (float)abs(line_right_bottom.x() - line_left_top.x());

		reference_axial_line_->SetRangeScene(line_left_top.y(),
			line_right_bottom.y());
		reference_axial_line_->set_length(len_width);
		reference_axial_line_->SetLine(kLineID, axial_pos_scene,
			QVector2D(1.0, 0.0));
	}

	float len_height = (float)abs(line_right_bottom.ry() - line_left_top.ry());
	reference_frontal_line_->SetRangeScene(line_left_top.x(),
		line_right_bottom.x());
	reference_frontal_line_->set_length(len_height);

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	QPointF frontal_center_pos_in_scene =
		controller_slice_->MapVolToScene(res_frontal.center_position());
	reference_frontal_line_->SetLine(kLineID, frontal_center_pos_in_scene,
		QVector2D(0, 1));
}

void ViewTmjLateral::UpdateLateral() {
	if (!IsValidLateralResource()) {
		controller_slice_->ClearPlane();

		UpdateUI();
		scene().ViewEnableStatusChanged(false);
		return;
	}
	else {
		scene().ViewEnableStatusChanged(true);
	}

	const auto& res_lateral =
		ResourceContainer::GetInstance()->GetTMJResource().lateral(
			direction_type_);

	SetLateralPlane();
	RenderSlice();

	if (!res_lateral.IsValidCenterPosition(lateral_id_)) return;

	UpdateMeasurePlaneInfo();
	UpdateUI();

	scene().update();

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"ViewTmjLateral::UpdatedCrossSection");
#endif
}
void ViewTmjLateral::RenderSlice() {
	if (!View::IsEnableGLContext()) return;

	MakeCurrent();
	controller_slice_->RenderingSlice();
	DoneCurrent();
}
void ViewTmjLateral::HideAllUI(bool is_hide) {
	View::HideAllUI(is_hide);
	text_id_->setVisible(!is_hide);
	SetTMJUISelectedStatus();
}

void ViewTmjLateral::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type) {
	View::SetCommonToolOnOff(type);

	if (!isVisible()) return;

	if (type == common::CommonToolTypeOnOff::NONE) {
		reference_frontal_line_->SetHighlight(true);
		reference_axial_line_->SetHighlight(true);
	}
	else {
		reference_frontal_line_->SetHighlight(false);
		reference_axial_line_->SetHighlight(false);
	}
}

/**=================================================================================================
protected functions
*===============================================================================================**/
void ViewTmjLateral::slotActiveSharpenItem(const int index) {
	SharpenLevel level = static_cast<SharpenLevel>(index);

	controller_slice_->SetSharpenLevel(level);
	emit sigSetSharpen(level);
}
void ViewTmjLateral::TransformItems(const QTransform& transform) {
	reference_frontal_line_->TransformItems(transform);
	reference_axial_line_->TransformItems(transform);
}

void ViewTmjLateral::leaveEvent(QEvent* event) {
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void ViewTmjLateral::enterEvent(QEvent* event) {
	View::enterEvent(event);

	if (!IsReadyController()) return;

	emit sigLateralViewSelect(direction_type_, lateral_id_);
	scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}

void ViewTmjLateral::wheelEvent(QWheelEvent* event) {
	if (!IsReadyController() || !IsValidLateralResource()) return event->ignore();

	View::wheelEvent(event);

	const int kSingleStep = 120;
	const int wheel_step = event->delta() / kSingleStep;
	emit sigWheelEvent(wheel_step);
}

void ViewTmjLateral::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (!IsReadyController())
	{
		View::keyPressEvent(event);
		return;
	}

	const int kSingleStep = 1;
	if (event->key() == Qt::Key_Up)
	{
		emit sigWheelEvent(kSingleStep);
	}
	else if (event->key() == Qt::Key_Down)
	{
		emit sigWheelEvent(-kSingleStep);
	}
}

void ViewTmjLateral::keyReleaseEvent(QKeyEvent* event)
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

void ViewTmjLateral::resizeEvent(QResizeEvent* pEvent) {
	View::resizeEvent(pEvent);

	QPointF scene_center = view_render_param()->scene_center();
	QSize scene_size = view_render_param()->scene_size();
	text_id_->setPos(scene_center.x() + (double)scene_size.width() / 2.0 - 40.0,
		scene_center.y() + (double)scene_size.height() / 2.0 - 40.0);

	controller_slice_->SetProjection();
}

void ViewTmjLateral::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!IsValidLateralResource())
	{
		return QGraphicsView::mousePressEvent(event);
	}

	View::mousePressEvent(event);
}

void ViewTmjLateral::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!IsValidLateralResource())
	{
		return QGraphicsView::mouseMoveEvent(event);
	}

	View::mouseMoveEvent(event);
	QPointF scene_pos = pt_scene_current();

	RequestDICOMInfo(scene_pos);

	if (IsEventLeftButton(event))
	{
		if (IsEventAxialLineHovered())
		{
			emit sigSetAxialSlice(controller_slice_->MapSceneToVol(scene_pos));
		}
		else if (IsEventFrontalLineHovered())
		{
			emit sigSetFrontalSlice(controller_slice_->MapSceneToVol(scene_pos));
		}
	}
}

void ViewTmjLateral::mouseReleaseEvent(QMouseEvent* event)
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

	if (!IsValidLateralResource())
	{
		return QGraphicsView::mouseReleaseEvent(event);
	}

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT && IsEventLeftButton(event))
	{
		emit sigWindowingDone();
	}
	else if (tool_type() == common::CommonToolTypeOnOff::V_ZOOM || tool_type() == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		emit sigZoomDone(view_render_param()->scene_scale());
	}
	else if (tool_type() == common::CommonToolTypeOnOff::V_PAN || tool_type() == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		emit sigPanDone(view_render_param()->gl_trans());
	}

	View::mouseReleaseEvent(event);
}

bool ViewTmjLateral::IsInLateralPlane(const int& x, const int& y) const {
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return false;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr) return false;

	const int width = static_cast<int>(res_lateral.param().width);
	const int height = static_cast<int>(GetTMJresource().height());

	if (x >= 0 && x < width && y >= 0 && y < height)
		return true;
	else
		return false;
}

/**=================================================================================================
private functions
*===============================================================================================**/
// Counterpart tab 이 없는 상태에서 불림
void ViewTmjLateral::SetGraphicsItems() {
	View::SetGraphicsItems();
	bool is_update_needed = false;
	emit sigMeasureResourceUpdateNeeded(lateral_id_, is_update_needed);
	SyncMeasureResourceCounterparts(is_update_needed);
}

void ViewTmjLateral::InitializeController() { controller_slice_->Initialize(); }

bool ViewTmjLateral::IsReadyController() {
	return controller_slice_->IsReady();
}

void ViewTmjLateral::ClearGL() { controller_slice_->ClearGL(); }

void ViewTmjLateral::ActiveControllerViewEvent() {
	bool need_render = false;
	controller_slice_->ProcessViewEvent(&need_render);

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT) need_render = true;

	if (need_render) {
		RenderSlice();
	}
}

void ViewTmjLateral::drawBackground(QPainter* painter, const QRectF& rect) {
	View::drawBackground(painter, rect);

	if (!IsReadyController()) {
		if (controller_slice_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	if (IsUpdateController()) {
		this->SetLateralPlane();
		controller_slice_->RenderingSlice();
		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	RenderScreen();
	painter->endNativePainting();
}

void ViewTmjLateral::RenderScreen() {
	controller_slice_->RenderScreen(GetDefaultFrameBufferObject());
}

void ViewTmjLateral::SetLateralPlane() {
	if (!IsValidLateralResource()) {
		controller_slice_->ClearPlane();
		return;
	}
	glm::vec3 center_position;
	const auto& res_lateral = this->GetLateralResource();
	bool res = res_lateral.GetCenterPosition(lateral_id_, &center_position);
	if (!res) {
		controller_slice_->ClearPlane();
		return;
	}

	const glm::vec3& up_vector = res_lateral.up_vector();
	const glm::vec3& back_vector = GetTMJresource().back_vector();
	const glm::vec3 right_vector =
		glm::normalize(glm::cross(back_vector, up_vector));
	const auto& lateral_param = res_lateral.param();

	controller_slice_->SetPlane(
		center_position, right_vector * lateral_param.width,
		back_vector * GetTMJresource().height(), (int)lateral_param.thickness);

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));
}

void ViewTmjLateral::UpdateMeasurePlaneInfo() {
	if (!this->IsValidLateralResource()) return;
	const auto& res_lateral = this->GetLateralResource();
	glm::vec3 center_position;
	if (!res_lateral.GetCenterPosition(lateral_id_, &center_position)) return;

	scene().UpdatePlaneInfo(center_position, res_lateral.up_vector(),
		GetTMJresource().back_vector());
}

void ViewTmjLateral::RequestDICOMInfo(const QPointF& pt_scene) {
	if (!controller_slice_->IsReady()) return;

	QPointF pt_plane = controller_slice_->MapSceneToPlane(pt_scene);
	glm::vec4 vol_info;
	if (IsInLateralPlane(pt_plane.x(), pt_plane.y())) {
		vol_info = controller_slice_->GetDicomInfoPoint(pt_scene);
	}
	else {
		vol_info = glm::vec4(-1, -1, -1, -1);
	}
	DisplayDICOMInfo(vol_info);
}

void ViewTmjLateral::DisplayDICOMInfo(const glm::vec4& vol_info) {
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
	else {
		scene().SetHUValue(QString("WL %1\nWW %2\n(-, -, -)").arg(wl).arg(ww));
	}
}

void ViewTmjLateral::SetTMJUISelectedStatus() {
	const bool is_ui_visible = IsUIVisible();
	reference_frontal_line_->setVisible(is_ui_visible && is_selected_);
	reference_axial_line_->setVisible(is_ui_visible && is_selected_);

	if (is_selected_) {
		scene().SetBorderColor(ColorView::kTMJLateralSelected);
		scene().SetRulerColor(ColorView::kTMJLateralSelected);
		text_id_->setTextColor(ColorView::kTMJLateralSelected);
		UpdateUI();
	}
	else {
		scene().SetBorderColor(ColorView::kTMJLateral);
		scene().SetRulerColor(ColorView::kTMJLateral);
		text_id_->setTextColor(ColorView::kTMJLateral);
	}
}

bool ViewTmjLateral::IsEventAxialLineHovered() const {
	return (reference_axial_line_->is_highlight() && !View::IsSetTool()) ? true
		: false;
}

bool ViewTmjLateral::IsEventFrontalLineHovered() const {
	return (reference_frontal_line_->is_highlight() && !View::IsSetTool())
		? true
		: false;
}

bool ViewTmjLateral::IsValidLateralResource() const {
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return false;
	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr) return false;
	return true;
}

const TMJresource& ViewTmjLateral::GetTMJresource() const {
	return ResourceContainer::GetInstance()->GetTMJResource();
}
const TMJlateralResource& ViewTmjLateral::GetLateralResource() const {
	return ResourceContainer::GetInstance()->GetTMJResource().lateral(
		direction_type_);
}

/**=================================================================================================
private slots
*===============================================================================================**/
void ViewTmjLateral::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) {
	controller_slice_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void ViewTmjLateral::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) {
	controller_slice_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}
