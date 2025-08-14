#include "view_tmj_frontal.h"

#include <QDebug>
#include <QApplication>
#include <QGraphicsItem>
#include <QMouseEvent>
#include <iostream>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/color_will3d.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_tmj.h"
#endif
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/tmj_resource.h"

#include "../UIPrimitive/W3LineItem.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/annotation_freedraw.h"
#include "../UIPrimitive/guide_line_list_item.h"

#include "../UIViewController/view_controller_slice.h"

#include "scene.h"

using namespace UIViewController;

namespace
{
	const int kLineID = 0;
	const int kMaxCutHistoryIndex = 15;
}  // namespace

ViewTmjFrontal::ViewTmjFrontal(int frontal_id, const common::ViewTypeID& view_type, TMJDirectionType type, QWidget* parent)
	: frontal_id_(frontal_id)
	, direction_type_(type)
	, View(view_type, parent)
	, controller_slice_(new ViewControllerSlice)
	, reference_lateral_line_(new GuideLineListItem(GuideLineListItem::VERTICAL))
	, reference_axial_line_(new GuideLineListItem(GuideLineListItem::HORIZONTAL))
{
	controller_slice_->set_view_param(view_render_param());

	scene().InitViewItem(Viewitems::BORDER);
	scene().InitViewItem(Viewitems::RULER);
	scene().InitViewItem(Viewitems::HU_TEXT);
	scene().InitViewItem(Viewitems::DIRECTION_TEXT);
	scene().InitViewItem(Viewitems::SHARPEN_FILTER);
	scene().InitViewItem(Viewitems::GRID);

	scene().InitMeasure(view_type);
	connect(&scene(), &Scene::sigMeasureCreated,
		[=](const unsigned int& measure_id) {
		emit sigMeasureCreated(direction_type_, frontal_id_, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureDeleted,
		[=](const unsigned int& measure_id) {
		emit sigMeasureDeleted(direction_type_, frontal_id_, measure_id);
	});
	connect(&scene(), &Scene::sigMeasureModified,
		[=](const unsigned int& measure_id) {
		emit sigMeasureModified(direction_type_, frontal_id_, measure_id);
	});

	scene().SetBorderColor(ColorView::kTMJFrontal);
	scene().SetRulerColor(ColorView::kTMJFrontal);
	scene().SetDirectionTextItem(QString("M"), (type == TMJ_LEFT) ? true : false);

	QPen lateral_pen(ColorTmjItem::kLineHighlight, 2.0, Qt::SolidLine,
		Qt::FlatCap);
	lateral_pen.setCosmetic(true);

	reference_lateral_line_->setZValue(0);
	reference_lateral_line_->SetHighlight(true);
	reference_lateral_line_->set_pen(lateral_pen);
	scene().addItem(reference_lateral_line_.get());

	reference_axial_line_->setZValue(0);
	reference_axial_line_->SetHighlight(true);
	reference_axial_line_->set_pen(
		QPen(ColorTmjItem::kAxialLine, 2.0, Qt::SolidLine, Qt::FlatCap));
	scene().addItem(reference_axial_line_.get());

	ResetTMJCutUI();
}

ViewTmjFrontal::~ViewTmjFrontal() {
	if (View::IsEnableGLContext()) {
		View::MakeCurrent();
		controller_slice_->ClearGL();
		View::DoneCurrent();
	}
}

/**=================================================================================================
public functions
*===============================================================================================**/

void ViewTmjFrontal::SetEnabledSharpenUI(const bool& is_enabled) {
	scene().SetEnabledItem(Viewitems::SHARPEN_FILTER, is_enabled);
}
void ViewTmjFrontal::SetSharpen(SharpenLevel level) {
	scene().ChangeSharpenLevel(static_cast<int>(level));
	controller_slice_->SetSharpenLevel(level);
	scene().update();
}

void ViewTmjFrontal::SetLateralVisible(const bool& visible) {
	is_lateral_visible_ = visible;
	reference_lateral_line_->setVisible(is_lateral_visible_ && IsUIVisible());
}

void ViewTmjFrontal::SetLateralSelected(const int& lateral_id) {
	bool visible = is_lateral_visible_;
	reference_lateral_line_->SetVisibleSelectedLine(lateral_id, visible);
}

void ViewTmjFrontal::SetTMJCutMode(const bool& cut_on,
	const VRCutTool& cut_tool) {
	cut_tool_ = cut_tool;
	cut_event_type_ = (cut_on) ? CutEventType::DRAW : CutEventType::NONE;
	DeleteCutUI();

	if (cut_result_display_ != nullptr) cut_result_display_->setVisible(cut_on);
}

void ViewTmjFrontal::ResetTMJCutUI() {
	cut_result_display_.reset();
	cut_ui_history_.clear();
	cut_ui_history_.resize(kMaxCutHistoryIndex);
	cut_step_ = -1;
	cut_stack_index_ = -1;
	is_over_cut_stack_ = false;
}

void ViewTmjFrontal::UndoTMJCutUI() {
	if ((cut_step_ == 0 && !is_over_cut_stack_) || cut_step_ > 0) cut_step_--;

	DrawCutResultAreaUI();
}

void ViewTmjFrontal::RedoTMJCutUI() {
	if (cut_step_ < kMaxCutHistoryIndex && cut_step_ < cut_stack_index_)
		cut_step_++;

	DrawCutResultAreaUI();
}

void ViewTmjFrontal::UpdateThickness() {
	if (!isVisible()) return;

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return;

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral == nullptr) return;

	QPen lateral_pen(ColorTmjItem::kLineHighlight, 2.0, Qt::SolidLine,
		Qt::FlatCap);
	lateral_pen.setCosmetic(true);
	float scene_thickness =
		view_render_param()->MapVolToScene(res_lateral.param().thickness);
	if (scene_thickness >= 2.0) {
		lateral_pen.setWidthF(scene_thickness);
	}
	reference_lateral_line_->set_pen(lateral_pen);
	for (int i = 0; i < res_lateral.number().size(); i++) {
		reference_lateral_line_->UpdateLine(i);
	}
}

void ViewTmjFrontal::UpdateUI() {
	if (!IsValidFrontalResource()) {
		reference_lateral_line_->ClearLines();
		reference_axial_line_->ClearLines();
		return;
	}

	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	const auto& res_frontal = res_tmj.frontal(direction_type_);

	const double width = (double)res_frontal.param().width;
	const double height = (double)GetTMJresource().height();

	const glm::vec3& axial_pos_vol = res_tmj.axial_position();
	QPointF axial_pos_scene = controller_slice_->MapVolToScene(axial_pos_vol);

	QPointF line_left_top = controller_slice_->MapPlaneToScene(QPointF(0.0, 0.0));
	QPointF line_right_bottom =
		controller_slice_->MapPlaneToScene(QPointF(width, height));
	axial_pos_scene.setX((line_left_top.x() + line_right_bottom.x()) * 0.5);

	const auto& iter = reference_axial_line_->line_positions().find(kLineID);
	if (iter == reference_axial_line_->line_positions().end() ||
		iter->second != axial_pos_scene) {
		const float len_width =
			(float)abs(line_right_bottom.x() - line_left_top.x());

		reference_axial_line_->SetRangeScene(line_left_top.y(),
			line_right_bottom.y());
		reference_axial_line_->set_length(len_width);
		reference_axial_line_->SetLine(kLineID, axial_pos_scene,
			QVector2D(1.0f, 0.0f));
	}

	const float len_height =
		(float)abs(line_right_bottom.ry() - line_left_top.ry());

	reference_lateral_line_->SetRangeScene(line_left_top.x(),
		line_right_bottom.x());
	reference_lateral_line_->set_length(len_height);

	const auto& res_lateral = res_tmj.lateral(direction_type_);
	if (&res_lateral) {
		const std::vector<int>& lateral_number = res_lateral.number();
		for (int i = 0; i < lateral_number.size(); i++) {
			QPointF lateral_center_pos_in_scene = controller_slice_->MapPlaneToScene(
				QPointF(lateral_number[i], height * 0.5));

			reference_lateral_line_->SetLine(i, lateral_center_pos_in_scene,
				QVector2D(0.0f, 1.0f));
		}
		SetLateralSelected(res_lateral.selected_id());
	}
}

void ViewTmjFrontal::UpdateFrontal() {
	if (!IsValidFrontalResource()) {
		controller_slice_->ClearPlane();

		UpdateUI();
		scene().ViewEnableStatusChanged(false);
		return;
	}
	else {
		scene().ViewEnableStatusChanged(true);
	}

	SetFrontalPlane();
	RenderSlice();

	UpdateMeasurePlaneInfo();
	UpdateUI();

	scene().update();

#if DEVELOP_MODE
	common::Logger::instance()->PrintDebugMode(
		"ViewTmjFrontal::UpdatedCrossSection");
#endif
}

void ViewTmjFrontal::ClearLateralLine() {
	reference_lateral_line_->ClearLines();
}

void ViewTmjFrontal::RenderSlice() {
	if (!View::IsEnableGLContext()) return;

	MakeCurrent();
	controller_slice_->RenderingSlice();
	DoneCurrent();
}

void ViewTmjFrontal::GetMapVolToSceneFunc(
	std::function<void(const std::vector<glm::vec3>&, std::vector<QPointF>&)>&
	callback) {
	callback = std::bind(
		(void (ViewControllerSlice::*)(const std::vector<glm::vec3>&,
			std::vector<QPointF>&)) &
		ViewControllerSlice::MapVolToScene,
		controller_slice_.get(), std::placeholders::_1, std::placeholders::_2);
}

void ViewTmjFrontal::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type) {
	View::SetCommonToolOnOff(type);

	if (!isVisible()) return;

	if (type == common::CommonToolTypeOnOff::NONE) {
		reference_lateral_line_->SetHighlight(true);
		reference_axial_line_->SetHighlight(true);
	}
	else {
		reference_lateral_line_->SetHighlight(false);
		reference_axial_line_->SetHighlight(false);
	}
}

void ViewTmjFrontal::HideAllUI(bool is_hide) {
	View::HideAllUI(is_hide);

	reference_lateral_line_->setVisible(!is_hide && is_lateral_visible_);
	reference_axial_line_->setVisible(!is_hide);
}
#ifndef WILL3D_VIEWER
void ViewTmjFrontal::ExportProject(ProjectIOTMJ& out, float* scene_scale,
	float* scene_to_gl, QPointF* trans_gl) {
	View::exportProject(scene_scale, scene_to_gl, trans_gl);

	if (cut_result_display_ == nullptr) return;

	out.SaveCutPolygonPoints(direction_type_,
		cut_result_display_->polygon().toStdVector());
}

void ViewTmjFrontal::ImportProject(ProjectIOTMJ& in, const float& scene_scale,
	const float& scene_to_gl,
	const QPointF& trans_gl,
	const bool& is_update_resource) {
	View::importProject(scene_scale, scene_to_gl, trans_gl, false,
		is_update_resource);

	std::vector<QPointF> cut_poly_points;
	in.LoadCutPolygonPoints(direction_type_, cut_poly_points);

	if (cut_poly_points.empty()) return;
	ResetTMJCutUI();

	QPolygonF cut_area;
	for (const auto& pt : cut_poly_points) {
		cut_area.push_back(pt);
	}

	QColor colorPen(Qt::red);
	colorPen.setAlphaF(0.0f);
	QPen pen;
	pen.setWidthF(0.0f);
	pen.setColor(colorPen);

	QColor colorBrush(Qt::red);
	colorBrush.setAlphaF(0.5f);
	QBrush brush(colorBrush);
	brush.setStyle(Qt::DiagCrossPattern);

	cut_selected_area_for_draw_.reset(scene().addPolygon(cut_area, pen, brush));
	InsertCutHistory();
	DrawCutResultAreaUI();

	emit sigCut(direction_type_, cut_selected_area_for_draw_->polygon(),
		is_cut_inside_);
}
#endif
/**=================================================================================================
protected functions
*===============================================================================================**/

void ViewTmjFrontal::slotActiveSharpenItem(const int index) {
	SharpenLevel level = static_cast<SharpenLevel>(index);

	controller_slice_->SetSharpenLevel(level);
	emit sigSetSharpen(level);
}
void ViewTmjFrontal::TransformItems(const QTransform& transform) {
	if (cut_result_display_) {
		cut_result_display_->setPolygon(
			transform.map(cut_result_display_->polygon()));
	}
	if (cut_selected_area_for_draw_) {
		cut_selected_area_for_draw_->setPolygon(
			transform.map(cut_selected_area_for_draw_->polygon()));
	}
	if (cut_polygon_) {
		cut_polygon_->setPolygon(transform.map(cut_polygon_->polygon()));
	}
	for (auto& elem : cut_ui_history_) {
		elem = transform.map(elem);
	}

	reference_lateral_line_->TransformItems(transform);
	reference_axial_line_->TransformItems(transform);
}

void ViewTmjFrontal::leaveEvent(QEvent* event) {
	View::leaveEvent(event);
	scene().SetEnabledItem(Viewitems::HU_TEXT, false);
}

void ViewTmjFrontal::enterEvent(QEvent* event) {
	View::enterEvent(event);

	if (IsReadyController()) scene().SetEnabledItem(Viewitems::HU_TEXT, true);
}

void ViewTmjFrontal::wheelEvent(QWheelEvent* event) {
	if (!IsReadyController() || !IsValidFrontalResource()) return event->ignore();

	View::wheelEvent(event);

	const int kSingleStep = 120;
	const int wheel_step = event->delta() / kSingleStep;
	emit sigWheelEvent(wheel_step);
}

void ViewTmjFrontal::keyPressEvent(QKeyEvent* event) 
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
	else if (event->key() == Qt::Key_Escape) 
	{
		if (cut_event_type_ == CutEventType::NONE)
		{
			return;
		}

		DeleteCutUI();
		cut_event_type_ = CutEventType::DRAW;
		scene().update();
	}
}

void ViewTmjFrontal::keyReleaseEvent(QKeyEvent* event)
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

void ViewTmjFrontal::resizeEvent(QResizeEvent* pEvent) {
	View::resizeEvent(pEvent);

	QPointF scene_center = view_render_param()->scene_center();
	QSize scene_size = view_render_param()->scene_size();

	controller_slice_->SetProjection();
}

void ViewTmjFrontal::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!IsValidFrontalResource())
	{
		return QGraphicsView::mousePressEvent(event);
	}

	if (View::IsSetTool())
	{
		return View::mousePressEvent(event);
	}

	if (IsEventLeftButton(event))
	{
		switch (cut_event_type_)
		{
		case ViewTmjFrontal::CutEventType::DRAW:
			CreateCutUI(mapToScene(event->pos()));
			break;
		case ViewTmjFrontal::CutEventType::SELECT:
			SelectCutArea();
			break;
		default:
			break;
		}
	}
	else
	{
		if (cut_event_type_ == ViewTmjFrontal::CutEventType::DRAW)
		{
			return QGraphicsView::mousePressEvent(event);
		}
	}

	View::mousePressEvent(event);
}

void ViewTmjFrontal::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!IsValidFrontalResource())
	{
		return QGraphicsView::mouseMoveEvent(event);
	}

	View::mouseMoveEvent(event);
	QPointF scene_pos = pt_scene_current();
	RequestDICOMInfo(scene_pos);

	if (cut_polygon_ != nullptr)
	{
		if (cut_event_type_ == CutEventType::DRAW && View::IsSetTool())
		{
			DeleteCutUI();
		}
		else
		{
			switch (cut_tool_)
			{
			case VRCutTool::POLYGON:
			{
				Qt::MouseButtons buttons = event->buttons();
				QPolygonF poly = cut_polygon_->polygon();

				switch (cut_event_type_)
				{
				case CutEventType::DRAW:
					Draw3DCutUI(poly, scene_pos);
					break;
				case CutEventType::SELECT:
					DrawCutSelectAreaUI(poly, scene_pos);
					break;
				}
			}
			break;
			case VRCutTool::FREEDRAW:
			{
				switch (cut_event_type_)
				{
				case CutEventType::DRAW:
					if (cut_freedraw_) cut_freedraw_->processMouseMove(scene_pos);
					break;
				case CutEventType::SELECT:
					QPolygonF poly = cut_polygon_->polygon();
					DrawCutSelectAreaUI(poly, scene_pos);
					break;
				}
			}
			break;
			default:
				assert(false);
				break;
			}
		}
	}
	else if (IsEventLeftButton(event))
	{
		if (IsEventAxialLineHovered())
		{
			emit sigSetAxialSlice(controller_slice_->MapSceneToVol(scene_pos));
		}
		else if (IsEventLateralLineHovered())
		{
			emit sigSetLateralSlice(controller_slice_->MapSceneToVol(scene_pos));
		}
	}
}

void ViewTmjFrontal::mouseReleaseEvent(QMouseEvent* event)
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

	if (!IsValidFrontalResource())
	{
		return QGraphicsView::mouseReleaseEvent(event);
	}

	if (View::IsSetTool())
	{
		if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT &&
			IsEventLeftButton(event))
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
	}
	else if (cut_tool_ == VRCutTool::FREEDRAW && cut_event_type_ == CutEventType::DRAW && cut_freedraw_ && cut_polygon_)
	{
		if (!IsEventRightButton(event)) 
		{
			cut_freedraw_->processMouseReleased(mapToScene(event->pos()));
			QPolygonF freedraw_area = cut_freedraw_->GetPolygon();
			cut_polygon_->setPolygon(freedraw_area);
			cut_freedraw_.reset();

			EndDrawCutUI();
		}
		else 
		{
			DeleteCutUI();
		}
	}

	View::mouseReleaseEvent(event);
}

void ViewTmjFrontal::mouseDoubleClickEvent(QMouseEvent* event) {
	if (cut_event_type_ == CutEventType::DRAW && cut_polygon_ != nullptr) {
		EndDrawCutUI();

		QGraphicsView::mouseDoubleClickEvent(event);
	}
	else {
		View::mouseDoubleClickEvent(event);
	}
}

bool ViewTmjFrontal::IsInFrontalPlane(const int& x, const int& y) const {
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return false;

	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr) return false;

	const int width = static_cast<int>(res_frontal.param().width);
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
void ViewTmjFrontal::SetGraphicsItems() {
	View::SetGraphicsItems();
	bool is_update_needed = false;
	SyncMeasureResourceCounterparts(is_update_needed);
}

void ViewTmjFrontal::InitializeController() { controller_slice_->Initialize(); }

bool ViewTmjFrontal::IsReadyController() {
	return controller_slice_->IsReady();
}

void ViewTmjFrontal::ClearGL() { controller_slice_->ClearGL(); }

void ViewTmjFrontal::ActiveControllerViewEvent() {
	bool need_render = false;
	controller_slice_->ProcessViewEvent(&need_render);

	if (tool_type() == common::CommonToolTypeOnOff::V_LIGHT) need_render = true;

	if (need_render) {
		RenderSlice();
	}
}

void ViewTmjFrontal::drawBackground(QPainter* painter, const QRectF& rect) {
	View::drawBackground(painter, rect);

	if (!IsReadyController()) {
		if (controller_slice_->GetInvertWindow())
			painter->fillRect(rect, Qt::white);
		else
			painter->fillRect(rect, Qt::black);
		return;
	}

	if (IsUpdateController()) {
		UpdateUI();
		this->SetFrontalPlane();
		controller_slice_->RenderingSlice();
		UpdateDoneContoller();
	}

	painter->beginNativePainting();
	RenderScreen();
	painter->endNativePainting();
}

void ViewTmjFrontal::RenderScreen() {
	controller_slice_->RenderScreen(GetDefaultFrameBufferObject());
}

void ViewTmjFrontal::SetFrontalPlane() {
	if (!IsValidFrontalResource()) {
		controller_slice_->ClearPlane();
		return;
	}
	const auto& res_frontal = this->GetFrontalResource();
	const glm::vec3& up_vector = res_frontal.up_vector();
	const glm::vec3& back_vector = GetTMJresource().back_vector();
	const glm::vec3 right_vector =
		glm::normalize(glm::cross(back_vector, up_vector));
	const auto& param = res_frontal.param();

	glm::vec3 center_position = GetFrontalCenter();
	controller_slice_->SetPlane(center_position, right_vector * param.width,
		back_vector * GetTMJresource().height(),
		(int)param.thickness);

	scene().SetViewRulerItem(*(view_render_param()));
	scene().SetGridItem(*(view_render_param()));
	scene().SetMeasureParams(*(view_render_param()));
}

void ViewTmjFrontal::UpdateMeasurePlaneInfo() {
	if (!this->IsValidFrontalResource()) return;
	const auto& res_frontal = this->GetFrontalResource();

	glm::vec3 center_position = GetFrontalCenter();

	scene().UpdatePlaneInfo(center_position, res_frontal.up_vector(),
		GetTMJresource().back_vector());
}

void ViewTmjFrontal::RequestDICOMInfo(const QPointF& pt_scene) {
	if (!controller_slice_->IsReady()) return;

	QPointF pt_plane = controller_slice_->MapSceneToPlane(pt_scene);
	glm::vec4 vol_info;
	if (IsInFrontalPlane(pt_plane.x(), pt_plane.y())) {
		vol_info = controller_slice_->GetDicomInfoPoint(pt_scene);
	}
	else {
		vol_info = glm::vec4(-1, -1, -1, -1);
	}
	DisplayDICOMInfo(vol_info);
}

void ViewTmjFrontal::DisplayDICOMInfo(const glm::vec4& vol_info) {
	float window_width, window_level;
	controller_slice_->GetWindowParams(&window_width, &window_level);
	const int ww = static_cast<int>(window_width);
	const int wl =
		static_cast<int>(window_level + controller_slice_->GetIntercept());
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

void ViewTmjFrontal::DeleteCutUI() {
	if (cut_polygon_ != nullptr) {
		scene().removeItem(cut_polygon_.get());
		cut_polygon_.reset();
	}

	if (cut_freedraw_ != nullptr) {
		cut_freedraw_.reset();
	}

	DeleteCutSelectedArea();
}

void ViewTmjFrontal::DeleteCutSelectedArea() {
	if (cut_selected_area_for_draw_ != nullptr) {
		scene().removeItem(cut_selected_area_for_draw_.get());
		cut_selected_area_for_draw_.reset();
	}
}

void ViewTmjFrontal::CreateCutUI(const QPointF& curr_scene_pos) {
	QColor color(Qt::red);

	if (cut_polygon_ == nullptr) {
		// 다른 UI에 방해받지 않는지?

		QPen pen;
		pen.setColor(color);
		pen.setWidthF(2.0f);
		QBrush brush;
		brush.setColor(color);

		cut_polygon_.reset(new QGraphicsPolygonItem());
		cut_polygon_->setPen(pen);
		cut_polygon_->setBrush(brush);
		scene().addItem(cut_polygon_.get());
	}

	switch (cut_tool_) {
	case VRCutTool::POLYGON: {
		QPolygonF poly = cut_polygon_->polygon();
		if (poly.size() == 0) poly.append(curr_scene_pos);
		poly.append(curr_scene_pos);
		cut_polygon_->setPolygon(poly);
	} break;
	case VRCutTool::FREEDRAW: {
		bool done = true;
		cut_freedraw_.reset(new AnnotationFreedraw());
		cut_freedraw_->InputParam(&scene(), curr_scene_pos, glm::vec3(0.0f),
			done);
		cut_freedraw_->SetLineColor(color);
		break;
	}
	default:
		break;
	}
}

void ViewTmjFrontal::Draw3DCutUI(QPolygonF& poly,
	const QPointF& curr_scene_pos) {
	if (poly.size() > 1) {
		poly.last().setX(curr_scene_pos.x());
		poly.last().setY(curr_scene_pos.y());
		cut_polygon_->setPolygon(poly);
	}
}

void ViewTmjFrontal::DrawCutSelectAreaUI(QPolygonF& poly,
	const QPointF& curr_scene_pos) {
	if (poly.size() < 3) return;

	QPolygonF preSelectedPolygon;
	if (cut_selected_area_for_draw_ != nullptr)
		preSelectedPolygon = cut_selected_area_for_draw_->polygon();

	DeleteCutSelectedArea();

	QColor colorPen(Qt::red);
	colorPen.setAlphaF(0.0f);
	QPen pen;
	pen.setWidthF(0.0f);
	pen.setColor(colorPen);

	QColor colorBrush(Qt::red);
	colorBrush.setAlphaF(0.5f);
	QBrush brush(colorBrush);
	brush.setStyle(Qt::DiagCrossPattern);

	is_cut_inside_ = false;

	QPolygonF remain;
	remain.append(QPointF(-width(), -height()));
	remain.append(QPointF(width(), -height()));
	remain.append(QPointF(width(), height()));
	remain.append(QPointF(-width(), height()));

	QList<QPolygonF> listPoly = GetCutUISubpathPolygons();
	for (int i = 0; i < listPoly.size(); i++) {
		poly = listPoly.at(i);
		if (poly.containsPoint(curr_scene_pos, Qt::WindingFill)) {
			cut_selected_area_for_draw_.reset(scene().addPolygon(poly, pen, brush));

			is_cut_inside_ = true;
			break;
		}
		else {
			remain = remain.subtracted(poly);
		}
	}

	if (!is_cut_inside_)
		cut_selected_area_for_draw_.reset(scene().addPolygon(remain, pen, brush));

	if (preSelectedPolygon != cut_selected_area_for_draw_->polygon()) {
		scene().update();
	}
}

void ViewTmjFrontal::DrawCutResultAreaUI() {
	if (cut_step_ < 0) {
		cut_result_display_.reset();
		return;
	}

	QColor colorPen(Qt::red);
	colorPen.setAlphaF(0.0f);
	QPen pen;
	pen.setWidthF(0.0f);
	pen.setColor(colorPen);

	QColor colorBrush(Qt::red);
	colorBrush.setAlphaF(0.2f);
	QBrush brush(colorBrush);
	brush.setStyle(Qt::SolidPattern);

	QPolygonF cut_area = cut_ui_history_[cut_step_];
	cut_result_display_.reset(scene().addPolygon(cut_area, pen, brush));
}

void ViewTmjFrontal::EndDrawCutUI() {
	if (cut_polygon_->polygon().size() > 3) {
		cut_event_type_ = CutEventType::SELECT;
	}
	else {
		scene().removeItem(cut_polygon_.get());
		cut_polygon_.reset();

		cut_event_type_ = CutEventType::DRAW;
		scene().update();
	}
}

void ViewTmjFrontal::SelectCutArea() {
	if (cut_selected_area_for_draw_ == nullptr) return;

	InsertCutHistory();
	DrawCutResultAreaUI();

	emit sigCut(direction_type_, cut_polygon_->polygon(), is_cut_inside_);

	DeleteCutUI();
	cut_event_type_ = CutEventType::DRAW;
}

void ViewTmjFrontal::InsertCutHistory() {
	// create current status polygon
	QPolygonF curr_poly = cut_selected_area_for_draw_->polygon();
	if (cut_step_ > -1) curr_poly = curr_poly.united(cut_ui_history_[cut_step_]);

	// history stack is full. remove front polygon
	if (cut_step_ == kMaxCutHistoryIndex) {
		for (int idx = 0; idx < kMaxCutHistoryIndex - 1; ++idx) {
			cut_ui_history_[idx] = cut_ui_history_[idx + 1];
		}
		is_over_cut_stack_ = true;
	}
	else {
		cut_step_++;
	}

	cut_ui_history_[cut_step_] = curr_poly;
	cut_stack_index_ = cut_step_;
}

QList<QPolygonF> ViewTmjFrontal::GetCutUISubpathPolygons() {
	QPainterPath pp, winding_pp;
	pp.addPolygon(cut_polygon_->polygon());

	pp.setFillRule(Qt::WindingFill);
	winding_pp = pp.simplified();

	pp.setFillRule(Qt::OddEvenFill);
	pp = pp.simplified();

	QList<QPolygonF> polygons = pp.toSubpathPolygons();
	QPolygonF inside_polygon =
		winding_pp.toFillPolygon().subtracted(pp.toFillPolygon());
	if (!inside_polygon.isEmpty()) polygons.append(inside_polygon);

	return polygons;
}

bool ViewTmjFrontal::IsEventAxialLineHovered() const {
	return (reference_axial_line_->is_highlight() && !View::IsSetTool()) ? true
		: false;
}

bool ViewTmjFrontal::IsEventLateralLineHovered() const {
	return (reference_lateral_line_->is_highlight() && !View::IsSetTool())
		? true
		: false;
}

bool ViewTmjFrontal::IsValidFrontalResource() const {
	const auto& res_tmj = ResourceContainer::GetInstance()->GetTMJResource();
	if (&res_tmj == nullptr) return false;
	const auto& res_frontal = res_tmj.frontal(direction_type_);
	if (&res_frontal == nullptr) return false;
	return true;
}

const TMJresource& ViewTmjFrontal::GetTMJresource() const {
	return ResourceContainer::GetInstance()->GetTMJResource();
}

const TMJfrontalResource& ViewTmjFrontal::GetFrontalResource() const {
	return ResourceContainer::GetInstance()->GetTMJResource().frontal(
		direction_type_);
}

const glm::vec3 ViewTmjFrontal::GetFrontalCenter() const {
	const auto& res_frontal = this->GetFrontalResource();
	const glm::vec3& up_vector = res_frontal.up_vector();
	const auto& param = res_frontal.param();

	return res_frontal.center_position() +
		up_vector *
		GetTMJresource().lateral(direction_type_).param().interval *
		(float)frontal_id_;
}

/**=================================================================================================
private slots
*===============================================================================================**/
void ViewTmjFrontal::slotGetProfileData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) {
	controller_slice_->GetDicomHULine(start_pt_scene, end_pt_scene, data);
}

void ViewTmjFrontal::slotGetROIData(const QPointF& start_pt_scene,
	const QPointF& end_pt_scene,
	std::vector<short>& data) {
	controller_slice_->GetDicomHURect(start_pt_scene, end_pt_scene, data);
}
