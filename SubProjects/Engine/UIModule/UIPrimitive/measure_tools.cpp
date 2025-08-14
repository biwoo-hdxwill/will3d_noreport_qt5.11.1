#include "measure_tools.h"
/*=========================================================================
Copyright (c) 2015~2018 All rights reserved by HDXWILL.

File: W3AnnotationHandler.cpp
Desc: In header.
=========================================================================*/

#include <QDebug>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/event_handle_measure.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/will3d_id_parser.h>

#include <Engine/Resource/Resource/include/measure_data.h>

#include "annotation_arrow.h"
#include "annotation_freedraw.h"
#include "annotation_note.h"
#include "measure_base.h"
#include "measure_degree.h"
#include "measure_length.h"
#include "measure_polygon.h"
#include "measure_profile.h"
#include "measure_tape.h"
#include "measure_line.h"

#define TEST 0

using namespace common;
using namespace common::measure;

namespace
{
	const glm::vec3 tmp_vol_pos(0.0f, 0.0f, 0.0f);
	const QTransform kITransform;
}  // end of namespace

MeasureTools::MeasureTools(const common::ViewTypeID& view_type,
	QGraphicsScene* pScene)
	: view_id_(view_type), scene_(pScene)
{
}

MeasureTools::MeasureTools(const common::ViewTypeID& view_type, int view_sub_id, QGraphicsScene* scene)
	: view_id_(view_type), view_sub_id_(view_sub_id), scene_(scene)
{
}

MeasureTools::~MeasureTools() {}

void MeasureTools::Update(const glm::vec3& vp_center, const glm::vec3& vp_up,
	const glm::vec3& vp_back)
{
	UpdateProjection();

	view_visibility_params_.center = vp_center;
	view_visibility_params_.up = vp_up;
	view_visibility_params_.back = vp_back;
	CheckVisibility();
}

void MeasureTools::UpdateProjection() { emit sigProtterUpdate(); }

void MeasureTools::Initialize(const float& pixel_spacing,
	const float& scene_to_gl)
{
	SetPixelSpacing(pixel_spacing);
	SetScale(0.5f * scene_to_gl);
	SetZoomFactor(1.0f);
}

void MeasureTools::SetScale(const float scale)
{
	if (scale <= 0.0f)
	{
		return;
	}

	curr_view_info_.scale = scale;
#if TEST
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetScale(
		static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.scale);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetScale(view_id_, curr_view_info_.scale);
#endif

	for (int i = 0; i < measure_ui_list_.size(); ++i)
	{
		measure_ui_list_.at(i)->UpdateMeasure();
	}
}

void MeasureTools::SetZoomFactor(const float zoom_factor)
{
	if (zoom_factor <= 0.0f)
	{
		return;
	}
		
	curr_view_info_.zoom_factor = zoom_factor;
#if TEST
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetZoomFactor(
		static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.zoom_factor);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetZoomFactor(view_id_, curr_view_info_.zoom_factor);
#endif
}

bool MeasureTools::IsMeasureInteractionAvailable(
	const common::CommonToolTypeOnOff& anno_type)
{
	switch (anno_type)
	{
	case common::CommonToolTypeOnOff::V_PAN:
	case common::CommonToolTypeOnOff::V_LIGHT:
	case common::CommonToolTypeOnOff::V_ZOOM:
	case common::CommonToolTypeOnOff::V_ZOOM_R:
	case common::CommonToolTypeOnOff::V_PAN_LR:
		return false;
	default:
		return true;
	}
}

// 이후 동작들에서 수행 될 이벤트 타입을 결정한다.
void MeasureTools::ProcessMousePressed(const QPointF& pt, const glm::vec3& ptVol)
{
	selected_measure_.reset();
	SelectEventType();

	if (view_render_mode_ == common::ReconTypeID::VR &&
		(curr_measure_type_ != MeasureType::DRAW_FREEDRAW &&
			curr_measure_type_ != MeasureType::DRAW_ARROW &&
			curr_measure_type_ != MeasureType::DRAW_CIRCLE &&
			curr_measure_type_ != MeasureType::DRAW_RECT &&
			curr_measure_type_ != MeasureType::DELETE_ONE))
	{
		return;
	}

	switch (event_type_)
	{
	case EventType::DRAW:
		ActionDrawMeasure(pt, ptVol);
		return;
	case EventType::DEL:
		ActionDeleteMeasure(pt, ptVol);
		event_type_ = EventType::NONE;
		break;
	case EventType::SELECT:
	case EventType::END:
		return;
	default:
		ClearUnfinishedItem();
		event_type_ = EventType::NONE;
		break;
	}
}

void MeasureTools::ProcessMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (view_render_mode_ == common::ReconTypeID::VR &&
		(curr_measure_type_ != MeasureType::DRAW_FREEDRAW &&
			curr_measure_type_ != MeasureType::DRAW_ARROW &&
			curr_measure_type_ != MeasureType::DRAW_CIRCLE &&
			curr_measure_type_ != MeasureType::DRAW_RECT))
	{
		return;
	}

	bool is_signal_blocked;
	switch (event_type_)
	{
	case MeasureTools::EventType::DRAW:
		is_signal_blocked = signalsBlocked();
		blockSignals(false);
		measure_ui_list_.back()->processMouseMove(pt);
		blockSignals(is_signal_blocked);
		break;
	case MeasureTools::EventType::SELECT:
		event_type_ = EventType::MODIFY;
	case MeasureTools::EventType::MODIFY:
		selected_measure_.lock().get()->processMouseMove(pt);

		break;
	default:
		break;
	}
}

// Pressed 에서 검사된 이벤트 타입에 따른 동작 실행
void MeasureTools::ProcessMouseReleased(Qt::MouseButton button,
	const QPointF& pt,
	const glm::vec3& ptVol)
{
	if (view_render_mode_ == common::ReconTypeID::VR &&
		(curr_measure_type_ != MeasureType::DRAW_FREEDRAW &&
			curr_measure_type_ != MeasureType::DRAW_ARROW &&
			curr_measure_type_ != MeasureType::DRAW_CIRCLE &&
			curr_measure_type_ != MeasureType::DRAW_RECT))
	{
		return;
	}

	if (button == Qt::RightButton)
	{
#if 0
		DeleteSelectedMeasure();
#endif
		return;
	}

	if (curr_measure_type_ == MeasureType::DRAW_FREEDRAW &&
		event_type_ == EventType::DRAW)
	{
		if (measure_ui_list_.empty())
			return;

		std::shared_ptr<MeasureBase> measure_ptr = measure_ui_list_.back();
		measure_ptr->processMouseReleased(pt);
		if (!measure_ptr->is_drawing())
		{
			EndMeasure();
			return;
		}
		else
		{
			DeleteLastMeasure();
			return;
		}
	}

	if (selected_measure_.lock().get())
	{
		if (event_type_ == EventType::MODIFY)
		{
			MeasureBase* measure_ptr = selected_measure_.lock().get();
			EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureModify(
				view_id_, measure_ptr->id(), measure_ptr->GetValue(),
				measure_ptr->GetMeasurePoints());

			event_type_ = EventType::SELECT;

			if (IsSiblingType())
			{
				emit sigMeasureModified(measure_ptr->id());
			}
		}
	}
}

void MeasureTools::ProcessMouseDoubleClick(Qt::MouseButton button, const QPointF& pt, const glm::vec3& ptVol)
{
	if (view_render_mode_ == common::ReconTypeID::VR &&
		button != Qt::RightButton)
	{
		return;
	}

	if (button == Qt::RightButton)
	{
		DeleteSelectedMeasure();
		return;
	}

	if (measure_ui_list_.empty()) return;

	MeasureBase* measure = measure_ui_list_.back().get();

	if (measure->type() == measure::MeasureType::LENGTH_LINE ||
		measure->type() == measure::MeasureType::DRAW_LINE)
	{
		return;
	}

	// 미완성인 annotation 에 대한 예외처리
	if (measure->type() == measure::MeasureType::ROI_RECT)
	{
		ClearUnfinishedItem();
		return;
	}
	else if (measure->type() == measure::MeasureType::AREA_LINE)
	{
		std::vector<QPointF> points = measure->GetMeasurePoints();
		if (points.size() < 3)
		{
			return;
		}
	}
	else if (measure->type() == measure::MeasureType::LENGTH_TAPELINE ||
		measure->type() == measure::MeasureType::LENGTH_CURVE)
	{
		std::vector<QPointF> points = measure->GetMeasurePoints();
		if (points.size() < 2)
		{
			return;
		}
	}

	if (measure->is_drawing())
	{
		measure->processMouseDoubleClicked(pt);
		EndMeasure();
		event_type_ = EventType::END;
	}
	else
	{
		if (selected_measure_.lock().get())
			selected_measure_.lock().get()->processMouseDoubleClicked(pt);
	}
}

void MeasureTools::DeleteAllMeasure()
{
	if (measure_ui_list_.empty()) return;

	selected_measure_.reset();
	measure_ui_list_.clear();

	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureDeleteAll(
		view_id_);
}

void MeasureTools::ClearUnfinishedItem()
{
	if (measure_ui_list_.empty()) return;

	if (measure_ui_list_.back()->is_drawing()) DeleteLastMeasure();
}

void MeasureTools::SetVisible(bool is_visible_from_toolbar,
	bool is_visible_from_view,
	const common::ReconTypeID& mode)
{
	visible_from_toolbar_ = is_visible_from_toolbar;
	visible_from_view_ = is_visible_from_view;
	view_render_mode_ = mode;
	CheckVisibility();
}

void MeasureTools::SetVisible(bool is_visible_from_toolbar,
	bool is_visible_from_view)
{
	visible_from_toolbar_ = is_visible_from_toolbar;
	visible_from_view_ = is_visible_from_view;
	CheckVisibility();
}

void MeasureTools::SetVisibleFromToolbar(bool visible)
{
	visible_from_toolbar_ = visible;
	CheckVisibility();
}

void MeasureTools::SetVisibleFromView(bool visible)
{
	visible_from_view_ = visible;
	CheckVisibility();
}

void MeasureTools::SetViewRenderMode(const common::ReconTypeID& mode)
{
	view_render_mode_ = mode;
	CheckVisibility();
}

void MeasureTools::SetCenter(const QPointF& point)
{
	curr_view_info_.scene_center = point;
#if TEST
	EventHandler::GetInstance()
		->GetMeasureEventHandle()
		.EmitSigMeasureSetSceneCenter(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.scene_center);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetSceneCenter(view_id_, curr_view_info_.scene_center);
#endif
}

void MeasureTools::SetSceneTrans(const QPointF& trans)
{
	curr_view_info_.translate = trans;
#if TEST
	EventHandler::GetInstance()
		->GetMeasureEventHandle()
		.EmitSigMeasureSetSceneTrans(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.translate);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetSceneTrans(view_id_, curr_view_info_.translate);
#endif
}

void MeasureTools::SetPixelSpacing(float pixel_spacing)
{
	if (pixel_spacing <= 0.0f) return;

	curr_view_info_.spacing = pixel_spacing;
#if TEST
	EventHandler::GetInstance()
		->GetMeasureEventHandle()
		.EmitSigMeasureSetPixelPitch(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.spacing);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetPixelPitch(view_id_, curr_view_info_.spacing);
#endif
}

void MeasureTools::TransformItems(const QTransform& transform, const bool translate)
{
	if (kITransform == transform) return;

	if (translate)
	{
		curr_view_info_.transform *= transform;
	}
#if TEST
	EventHandler::GetInstance()
		->GetMeasureEventHandle()
		.EmitSigMeasureSetSceneTransform(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.transform);
#else
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureSetSceneTransform(view_id_, curr_view_info_.transform);
#endif

	for (auto& x : measure_ui_list_)
	{
		if (x->TransformItems(transform))
		{  // return true if transformed
			x->UpdateMeasure();
			EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureModify(
				view_id_, x->id(), x->GetValue(), x->GetMeasurePoints());
		}
	}
}

void MeasureTools::SetMeasureType(const common::CommonToolTypeOnOff& toolType)
{
	using measure::MeasureType;
	/*if (view_render_mode_ == common::ReconTypeID::VR) {
	  curr_measure_type_ = MeasureType::NONE;
	  return;
	}*/

	switch (toolType)
	{
	case common::CommonToolTypeOnOff::M_RULER:
		curr_measure_type_ = MeasureType::LENGTH_LINE;
		break;
	case common::CommonToolTypeOnOff::M_TAPELINE:
		curr_measure_type_ = MeasureType::LENGTH_TAPELINE;
		break;
	case common::CommonToolTypeOnOff::M_TAPECURVE:
		curr_measure_type_ = MeasureType::LENGTH_CURVE;
		break;
	case common::CommonToolTypeOnOff::M_ANGLE:
		curr_measure_type_ = MeasureType::ANGLE_THREEPOINT;
		break;
	case common::CommonToolTypeOnOff::M_PROFILE:
		curr_measure_type_ = MeasureType::PROFILE;
		break;
	case common::CommonToolTypeOnOff::M_AREALINE:
		curr_measure_type_ = MeasureType::AREA_LINE;
		break;
	case common::CommonToolTypeOnOff::M_ROI:
		curr_measure_type_ = MeasureType::ROI_RECT;
		break;
	case common::CommonToolTypeOnOff::M_RECTANGLE:
		curr_measure_type_ = MeasureType::DRAW_RECT;
		break;
	case common::CommonToolTypeOnOff::M_CIRCLE:
		curr_measure_type_ = MeasureType::DRAW_CIRCLE;
		break;
	case common::CommonToolTypeOnOff::M_ARROW:
		curr_measure_type_ = MeasureType::DRAW_ARROW;
		break;
	case common::CommonToolTypeOnOff::M_LINE:
		curr_measure_type_ = MeasureType::DRAW_LINE;
		break;
	case common::CommonToolTypeOnOff::M_FREEDRAW:
		curr_measure_type_ = MeasureType::DRAW_FREEDRAW;
		break;
	case common::CommonToolTypeOnOff::M_NOTE:
		curr_measure_type_ = MeasureType::NOTE;
		break;
	case common::CommonToolTypeOnOff::M_DEL:
		curr_measure_type_ = MeasureType::DELETE_ONE;
		break;
	default:
		curr_measure_type_ = MeasureType::NONE;
	}
}

const bool MeasureTools::IsDrawing() const
{
	if (measure_ui_list_.empty()) return false;
	if (measure_ui_list_.back()->is_drawing()) return true;
	return false;
}
const bool MeasureTools::IsSelected() const
{
	for (const auto& x : measure_ui_list_)
	{
		if (x->IsSelected()) return true;
	}
	return false;
}

// free draw 를 위한 함수.
// freedraw 는 다른 툴들과 다르게 셀렉션 모드가 따로 존재
void MeasureTools::slotSyncSelectedStatus(bool selected)
{
	QObject* sender = QObject::sender();
	if (!sender)
	{
		return;
	}

	if (!selected)
	{
		selected_measure_.reset();
		return;
	}

	if (curr_measure_type_ == measure::MeasureType::DELETE_ONE)
	{
		for (const auto& x : measure_ui_list_)
		{
			if (sender == dynamic_cast<QObject*>(x.get()))
			{
				selected_measure_ = x;
				DeleteSelectedMeasure();
				return;
			}
		}
	}
	else
	{
		for (const auto& x : measure_ui_list_)
		{
			if (sender == dynamic_cast<QObject*>(x.get()))
			{
				selected_measure_ = x;
				continue;
			}
#if 0
			x->setSelected(false);
#endif
			}
		}
	}

void MeasureTools::ActionDrawMeasure(const QPointF& pt, const glm::vec3& ptVol)
{
	if (curr_measure_type_ == MeasureType::NONE) return;

	if (IsDrawing())
	{
		std::weak_ptr<MeasureBase> anno = measure_ui_list_.back();
		if (anno.lock().get()->type() == curr_measure_type_)
		{
			bool done = false;
			if (!anno.lock().get()->InputParam(scene_, pt, ptVol, done))
			{
				DeleteLastMeasure();
			}

			if (done)
			{
				EndMeasure();
			}
		}
		else
		{
			DeleteLastMeasure();
			NewMeasure(pt, ptVol);
		}
	}
	else
	{
		NewMeasure(pt, ptVol);
	}
}

void MeasureTools::ActionDeleteMeasure(const QPointF& pt,
	const glm::vec3& ptVol)
{
	ClearUnfinishedItem();

	if (selected_measure_.lock().get())
	{
		for (auto& iter = measure_ui_list_.begin(); iter != measure_ui_list_.end();
			++iter)
		{
			if ((*iter).get() == selected_measure_.lock().get())
			{
				if (IsSiblingType() || IsMPRType()) emit sigMeasureDeleted((*iter).get()->id());

				EventHandler::GetInstance()
					->GetMeasureEventHandle()
					.EmitSigMeasureDelete(view_id_, (*iter).get()->id());

				measure_ui_list_.erase(iter);
				selected_measure_.reset();
				break;
			}
		}
	}
}

void MeasureTools::ActionHighlightMeasure()
{
	for (auto& i : measure_ui_list_)
	{
		if (i->is_drawing())
		{
			continue;
		}
		else if (i->IsSelected())
		{
			selected_measure_ = i;
			i->setNodeDisplay(true);
		}
		else
		{
			i->setNodeDisplay(false);
		}
	}
	slotSyncSelectedStatus(false);
}

void MeasureTools::AddMeasureToMeasureList(MeasureBase* obj, const QPointF& pt,
	const glm::vec3& ptVol)
{
	bool done = false;
	if (!obj->InputParam(scene_, pt, ptVol, done))
	{
		SAFE_DELETE_OBJECT(obj);
		return;
	}
	measure_ui_list_.push_back(std::shared_ptr<MeasureBase>(obj));

	if (done) EndMeasure();
}

void MeasureTools::NewMeasure(const QPointF& pt, const glm::vec3& ptVol)
{
	std::weak_ptr<MeasureData> measure_data;
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureCreate(
		view_id_, curr_measure_type_, view_visibility_params_,
		curr_view_info_.spacing, curr_view_info_.scale, &measure_data);
	
	if (measure_data.expired())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "MeasureTools::NewMeasure, measure data create failed");
		return;
	}

	MeasureBase* obj = nullptr;
	CreateMeasureUI(curr_measure_type_, obj);
	obj->SetMeasureData(measure_data);
	AddMeasureToMeasureList(obj, pt, ptVol);
}

void MeasureTools::CreateMeasureUI(
	const common::measure::MeasureType& measure_type, MeasureBase*& obj)
{
	switch (measure_type)
	{
	case MeasureType::LENGTH_LINE:
		obj = new MeasureLength();
		break;
	case MeasureType::ANGLE_THREEPOINT:
		obj = new MeasureDegree();
		break;
	case MeasureType::PROFILE:
		obj = new MeasureProfile();
		connect(dynamic_cast<MeasureProfile*>(obj),
			&MeasureProfile::sigGetPlotterData, this,
			&MeasureTools::sigGetProfileData);
		break;
	case MeasureType::LENGTH_TAPELINE:
		obj = new MeasureTape(PathType::LINE, DrawMode::LENGTH);
		break;
	case MeasureType::LENGTH_CURVE:
		obj = new MeasureTape(PathType::CURVE, DrawMode::LENGTH);
		break;
	case MeasureType::ROI_RECT:
		obj = new MeasurePolygon(Shape::RECT, DrawMode::ROI);
		connect(dynamic_cast<MeasurePolygon*>(obj),
			&MeasurePolygon::sigGetROIData, this,
			&MeasureTools::sigGetROIData);
		break;
	case MeasureType::AREA_LINE:
		obj = new MeasureTape(PathType::LINE, DrawMode::AREA);
		break;
	case MeasureType::DRAW_ARROW:
		obj = new AnnotationArrow();
		break;
	case MeasureType::DRAW_CIRCLE:
		obj = new MeasurePolygon(Shape::CIRCLE, DrawMode::JUSTDRAW);
		break;
	case MeasureType::DRAW_RECT:
		obj = new MeasurePolygon(Shape::RECT, DrawMode::JUSTDRAW);
		break;
	case MeasureType::DRAW_LINE:
		obj = new MeasureLine();
		break;
	case MeasureType::DRAW_FREEDRAW:
		obj = new AnnotationFreedraw();
		connect(dynamic_cast<AnnotationFreedraw*>(obj),
			&AnnotationFreedraw::sigSelected, this,
			&MeasureTools::slotSyncSelectedStatus);
		break;
	case MeasureType::NOTE:
		obj = new AnnotationNote();
		connect(dynamic_cast<AnnotationNote*>(obj), &AnnotationNote::sigSetNote,
			[=](const QString& txt)
		{
			EventHandler::GetInstance()
				->GetMeasureEventHandle()
				.EmitSigMeasureSetNote(view_id_, obj->id(), txt);
		});
		break;

	default:
		QString err_msg("MeasureTools::CreateMeasureUI: wrong measure type");
		common::Logger::instance()->Print(common::LogType::ERR,
			err_msg.toStdString());
		assert(false);
		return;
	}
}

/**********************************************************************************************
Restore measure from measure data.

@param	measure_data	Information describing the measure.
@param	points			호출되는 상황에 따라 points가 transform 될
경우가 있으므로 measure_data 의 points 대신 이 매개변수를 사용해야 한다.
 **********************************************************************************************/
void MeasureTools::RestoreMeasureFromMeasureData(
	const std::weak_ptr<MeasureData>& measure_data,
	const std::vector<QPointF>& points)
{
	selected_measure_.reset();
	measure::MeasureType prev_measure_type = curr_measure_type_;
	curr_measure_type_ = measure_data.lock().get()->type();
	if (curr_measure_type_ == measure::MeasureType::NOTE)
	{
		const EventHandleMeasure& handle_measure =
			EventHandler::GetInstance()->GetMeasureEventHandle();
		AnnotationNote* obj = new AnnotationNote();
		connect(obj, &AnnotationNote::sigSetNote, [&](const QString& txt)
		{
			handle_measure.EmitSigMeasureSetNote(view_id_, obj->id(), txt);
		});

		obj->SetMeasureData(measure_data);
		obj->InputParamWithResource(scene_, points.at(0), tmp_vol_pos);
		measure_ui_list_.push_back(std::unique_ptr<MeasureBase>(obj));
	}
	else
	{
		MeasureBase* obj = nullptr;
		for (int idx = 0; idx < points.size(); ++idx)
		{
			const auto& pt = points[idx];
			if (idx == 0)
			{
				event_type_ = EventType::DRAW;
				CreateMeasureUI(curr_measure_type_, obj);
				obj->SetMeasureData(measure_data);
				AddMeasureToMeasureList(obj, pt, tmp_vol_pos);
			}
			else if (idx == points.size() - 1)
			{
				ProcessMouseMove(pt, tmp_vol_pos);
				if (curr_measure_type_ == measure::MeasureType::DRAW_FREEDRAW)
				{
					ProcessMouseReleased(Qt::MouseButton::LeftButton, pt, tmp_vol_pos);
				}
				else
				{
					ProcessMousePressed(pt, tmp_vol_pos);
					if (curr_measure_type_ == measure::MeasureType::LENGTH_TAPELINE ||
						curr_measure_type_ == measure::MeasureType::LENGTH_CURVE ||
						curr_measure_type_ == measure::MeasureType::AREA_LINE)
						ProcessMouseDoubleClick(Qt::MouseButton::LeftButton, pt, tmp_vol_pos);
				}
			}
			else
			{
				ProcessMouseMove(pt, tmp_vol_pos);
				ProcessMousePressed(pt, tmp_vol_pos);
			}
		}
	}

	curr_measure_type_ = prev_measure_type;
}

void MeasureTools::EndMeasure()
{
	if (measure_ui_list_.empty())
	{
		return;
	}

	auto& measure_ptr = measure_ui_list_.back();
	EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(false);
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureModify(
		view_id_, measure_ptr->id(), measure_ptr->GetValue(),
		measure_ptr->GetMeasurePoints());
	
	if (IsSiblingType() || IsMPRType())
	{
		emit sigMeasureCreated(measure_ptr->id());
	}

	ActionHighlightMeasure();
}

void MeasureTools::DeleteLastMeasure()
{
	if (measure_ui_list_.back().get() == selected_measure_.lock().get())
		selected_measure_.reset();

	const std::shared_ptr<MeasureBase>& last_item = measure_ui_list_.back();
	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureDelete(
		view_id_, last_item->id());

	measure_ui_list_.pop_back();
}

void MeasureTools::DeleteSelectedMeasure()
{
	if (selected_measure_.lock().get() == nullptr
		/*|| selected_anno_->type() != measure::MeasureID::FREEDRAW*/)
		return;

	for (auto iter = measure_ui_list_.begin(); iter != measure_ui_list_.end();
		++iter)
	{
		if ((*iter).get() == selected_measure_.lock().get())
		{
			unsigned int id = (*iter).get()->id();
			if (IsSiblingType() || IsMPRType())
			{
				blockSignals(false);
				emit sigMeasureDeleted(id);
			}

			EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureDelete(
				view_id_, id);

			measure_ui_list_.erase(iter);
			selected_measure_.reset();
			break;
		}
	}
}

void MeasureTools::CheckVisibility()
{
	if (measure_ui_list_.empty())
	{
		return;
	}
	
#if 0
	//Cross_section에서 도구가 유지되게끔
	if (view_id_ == common::ViewTypeID::CROSS_SECTION)
	{
		return;
	}
#endif

	bool is_render_mode_2D = view_render_mode_ == common::ReconTypeID::VR ? false : true;
	bool entire_visibility = visible_from_toolbar_ && visible_from_view_;
	for (auto& item : measure_ui_list_)
	{
		if (entire_visibility && is_render_mode_2D)
		{
			item->setVisible(item->VisibilityCheck(view_visibility_params_));
		}
		else
		{
			item->setVisible(entire_visibility);
		}
	}
}

void MeasureTools::SelectEventType()
{
	event_type_ = EventType::NONE;

	if (!measure_ui_list_.empty())
	{
		if (measure_ui_list_.back()->is_drawing())
		{
			event_type_ = EventType::DRAW;
			return;
		}

		for (auto& item : measure_ui_list_)
		{
			if (item->IsSelected())
			{
				item->setNodeDisplay(true);
				selected_measure_ = item;
				event_type_ = EventType::SELECT;
			}
			else
			{
				item->setNodeDisplay(false);
			}
		}

		if (event_type_ == EventType::SELECT)
		{
			if (curr_measure_type_ == MeasureType::DELETE_ONE)
			{
				event_type_ = EventType::DEL;
			}
			return;
		}
	}

	if (curr_measure_type_ == MeasureType::DELETE_ONE)
	{
		event_type_ = EventType::DEL;
	}
	else
	{
		event_type_ = (curr_measure_type_ == MeasureType::NONE) ? EventType::NONE
			: EventType::DRAW;
	}
}

void MeasureTools::UpdateMeasure()
{
	for (auto& x : measure_ui_list_) x->UpdateMeasure();
}

const bool MeasureTools::IsSiblingType() const
{
	return (view_id_ == common::ViewTypeID::CROSS_SECTION ||
		view_id_ == common::ViewTypeID::LIGHTBOX ||
		view_id_ == common::ViewTypeID::TMJ_FRONTAL_LEFT ||
		view_id_ == common::ViewTypeID::TMJ_FRONTAL_RIGHT ||
		view_id_ == common::ViewTypeID::TMJ_LATERAL_LEFT ||
		view_id_ == common::ViewTypeID::TMJ_LATERAL_RIGHT)
		? true
		: false;
}

const bool MeasureTools::IsMPRType() const
{
	return (view_id_ == common::ViewTypeID::MPR_AXIAL ||
		view_id_ == common::ViewTypeID::MPR_SAGITTAL ||
		view_id_ == common::ViewTypeID::MPR_CORONAL)
		? true
		: false;
}

void MeasureTools::ApplyPreferences()
{
	for (auto& x : measure_ui_list_) x->ApplyPreferences();
}

void MeasureTools::ImportMeasureResource(const bool& is_update_resource)
{
	const EventHandleMeasure& handle_measure =
		EventHandler::GetInstance()->GetMeasureEventHandle();
	common::measure::ViewInfo counter_view_info;
#if TEST
	handle_measure.EmitSigMeasureGetCounterpartViewInfo(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_),
		&counter_view_info);
#else
	handle_measure.EmitSigMeasureGetCounterpartViewInfo(view_id_,
		&counter_view_info);
#endif

	if (counter_view_info.scale <= 0.0f || counter_view_info.spacing <= 0.0f)
		counter_view_info = curr_view_info_;

	SyncMeasureInternal(counter_view_info, is_update_resource);

	curr_measure_type_ = common::measure::MeasureType::NONE;
}

void MeasureTools::SyncMeasureResourceSiblings(const bool& is_update_resource)
{
	SyncMeasureResource(SyncType::SIBLINGS, is_update_resource);
}
void MeasureTools::SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform)
{
#if 0
	return;
#endif
	SyncMeasureResource(SyncType::COUNTERPART, is_update_resource, need_transform);
}

void MeasureTools::SyncMeasureResource(const SyncType& sync_type, const bool& is_update_resource, const bool need_transform)
{
	if (curr_view_info_.spacing <= 0.0f || curr_view_info_.scale <= 0.0f)
		return;  // not initialized yet.

	const EventHandleMeasure& handle_measure = EventHandler::GetInstance()->GetMeasureEventHandle();
	common::measure::ViewInfo counter_view_info;
	if (sync_type == SyncType::COUNTERPART)
	{
#if TEST
		handle_measure.EmitSigMeasureGetCounterpartViewInfo(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_),
			&counter_view_info);
#else
		handle_measure.EmitSigMeasureGetCounterpartViewInfo(view_id_, &counter_view_info);
#endif
		if (counter_view_info.scale <= 0.0f || counter_view_info.spacing <= 0.0f)
			return;  // counterpart view is not ready
	}
	else if (sync_type == SyncType::SIBLINGS)
	{
		counter_view_info = curr_view_info_;
	}

	std::vector<std::weak_ptr<MeasureData>> measure_list;
	handle_measure.EmitSigMeasureGetMeasureList(view_id_, &measure_list);
	if (measure_list.empty())
	{
		return;
	} 

	if (sync_type == SyncType::COUNTERPART && need_transform)
	{
		TransformPointsForCounterparts();
	}
	SyncMeasureInternal(counter_view_info, is_update_resource);
}

void MeasureTools::SyncMeasureInternal(const common::measure::ViewInfo& counter_view_info, const bool& is_update_resource)
{
	const EventHandleMeasure& handle_measure = EventHandler::GetInstance()->GetMeasureEventHandle();

	selected_measure_.reset();
	measure_ui_list_.clear();

	std::vector<std::weak_ptr<MeasureData>> measure_list;
	handle_measure.EmitSigMeasureGetMeasureList(view_id_, &measure_list);
	if (measure_list.empty())
	{
		return;
	}

#if TEST
	handle_measure.EmitSigMeasureSetScale(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), counter_view_info.scale);
#else
	handle_measure.EmitSigMeasureSetScale(view_id_, counter_view_info.scale);
#endif

	double scale = (double)(counter_view_info.scale / curr_view_info_.scale);

	//QTransform transform;
#if 0
  //transform.translate(counter_view_info.translate.x(), counter_view_info.translate.y());
#else
  //transform.translate(curr_view_info_.scene_center.x(), curr_view_info_.scene_center.y());
  //transform.scale(scale, scale);
  //transform.translate(-counter_view_info.scene_center.x(), -counter_view_info.scene_center.y());
#endif

	this->blockSignals(true);
	EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(true);
	for (int idx = 0; idx < measure_list.size(); ++idx)
	{
		const std::weak_ptr<MeasureData>& measure_data = measure_list[idx];
		std::vector<QPointF> points = measure_data.lock().get()->points();

		RestoreMeasureFromMeasureData(measure_data, points);
	}
	measure::MeasureType prev_measure_type = curr_measure_type_;
	curr_measure_type_ = measure::MeasureType::NONE;

	//return;

#if 0
	QTransform transform_to_current;
#if 1
	transform_to_current.translate(curr_view_info_.scene_center.x(), curr_view_info_.scene_center.y());
	transform_to_current.scale(scale, scale);
	transform_to_current.translate(-counter_view_info.scene_center.x(), -counter_view_info.scene_center.y());
#else
	transform_to_current.translate(-curr_view_info_.translate.rx(), -curr_view_info_.translate.ry());
	transform_to_current.translate(curr_view_info_.scene_center.rx(), curr_view_info_.scene_center.ry());
	double transf_scale = (double)(counter_view_info.scale / curr_view_info_.scale);
	transform_to_current.scale(transf_scale, transf_scale);
	transform_to_current.translate(-counter_view_info.scene_center.x(), -counter_view_info.scene_center.y());
#endif

	this->blockSignals(false);
	if (is_update_resource)
	{
		EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(false);
		this->TransformItems(transform_to_current);
	}
	else
	{
		this->TransformItems(transform_to_current);
		EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(false);
}
#endif

	this->blockSignals(false);
#if TEST
	handle_measure.EmitSigMeasureSetScale(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_), curr_view_info_.scale);
#else
	handle_measure.EmitSigMeasureSetScale(view_id_, curr_view_info_.scale);
#endif
	CheckVisibility();
	curr_measure_type_ = prev_measure_type;

	for (int i = 0; i < measure_ui_list_.size(); ++i)
	{
		measure_ui_list_.at(i)->UpdateMeasure();
	}
}

void MeasureTools::TransformPointsForCounterparts()
{
	const EventHandleMeasure& handle_measure = EventHandler::GetInstance()->GetMeasureEventHandle();

	common::measure::ViewInfo counter_view_info;

#if TEST
	handle_measure.EmitSigMeasureGetCounterpartViewInfo(static_cast<common::ViewTypeID>(static_cast<int>(view_id_) + view_sub_id_),
		&counter_view_info);
#else
	handle_measure.EmitSigMeasureGetCounterpartViewInfo(view_id_, &counter_view_info);
#endif

	if (counter_view_info.scale <= 0.0f || counter_view_info.spacing <= 0.0f)
	{
		return;
	}

	std::vector<std::weak_ptr<MeasureData>> measure_list;
	handle_measure.EmitSigMeasureGetMeasureList(view_id_, &measure_list);
	if (measure_list.empty())
	{
		return;
	}

	double scale = (double)(counter_view_info.scale / curr_view_info_.scale);

	QTransform transform;
#if 0
	transform.translate(counter_view_info.translate.x(), counter_view_info.translate.y());
#else
	float curr_view_map_scene_to_gl = curr_view_info_.scale * 2.0f;
	float counter_view_map_scene_to_gl = counter_view_info.scale * 2.0f;

	qDebug() << "";
	qDebug() << "curr_view_info_.scene_center :" << curr_view_info_.scene_center;
	qDebug() << "curr_view_info_.scale :" << curr_view_info_.scale;
	qDebug() << "curr_view_info_.translate :" << curr_view_info_.translate;
	qDebug() << "curr_view_info_.zoom_factor :" << curr_view_info_.zoom_factor;

	qDebug() << "counter_view_info.scene_center :" << counter_view_info.scene_center;
	qDebug() << "counter_view_info.scale :" << counter_view_info.scale;
	qDebug() << "counter_view_info.translate :" << counter_view_info.translate;
	qDebug() << "counter_view_info.zoom_factor :" << counter_view_info.zoom_factor;
	qDebug() << "";
#if 0
	float zoom_ratio = counter_view_info.zoom_factor / curr_view_info_.zoom_factor;

	//transform.translate(-curr_view_info_.translate.x() * curr_view_info_.zoom_factor, -curr_view_info_.translate.y() * curr_view_info_.zoom_factor);
	//transform.translate(counter_view_info.translate.x() * counter_view_info.zoom_factor, counter_view_info.translate.y() * counter_view_info.zoom_factor);

	//transform.translate(-curr_view_info_.translate.x() * curr_view_info_.zoom_factor, -curr_view_info_.translate.y() * curr_view_info_.zoom_factor);

	transform.translate(-curr_view_info_.translate.x(), -curr_view_info_.translate.y());
	transform.translate(curr_view_info_.scene_center.x(), curr_view_info_.scene_center.y());
	transform.scale(scale, scale);
	transform.translate(-counter_view_info.scene_center.x(), -counter_view_info.scene_center.y());
	transform.translate(counter_view_info.translate.x(), counter_view_info.translate.y());

	//transform.translate(counter_view_info.translate.x() * counter_view_info.zoom_factor, counter_view_info.translate.y() * counter_view_info.zoom_factor);
#endif
#endif

	for (int idx = 0; idx < measure_list.size(); ++idx)
	{
		const std::weak_ptr<MeasureData>& measure_data = measure_list[idx];
		std::vector<QPointF> points = measure_data.lock().get()->points();
		for (auto& pt : points)
		{
#if 0
			pt = transform.map(pt);
#elif 1
			pt += counter_view_info.translate;
			pt -= counter_view_info.scene_center;
			pt *= scale;
			pt += curr_view_info_.scene_center;
			pt -= curr_view_info_.translate;
#else

			pt = counter_view_info.transform.inverted().map(pt);

			//pt *= scale;

			pt -= counter_view_info.scene_center;
			pt += curr_view_info_.scene_center;

			pt = curr_view_info_.transform.map(pt);
#endif
	}
		measure_data.lock().get()->set_points(points);
}
}

void MeasureTools::SyncCreateMeasureUI(const unsigned int& measure_id)
{
	if (curr_view_info_.spacing <= 0.0f || curr_view_info_.scale <= 0.0f)
	{
		return;  // not initialized yet.
	}

	this->blockSignals(true);
	{
		std::weak_ptr<MeasureData> measure_data;
		const EventHandleMeasure& handle_measure = EventHandler::GetInstance()->GetMeasureEventHandle();		
		handle_measure.EmitSigMeasureGetMeasureData(view_id_, measure_id, &measure_data);

		EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(true);
		{
			RestoreMeasureFromMeasureData(measure_data, measure_data.lock().get()->points());
		}
		EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(false);
	}
	this->blockSignals(false);

	CheckVisibility();
}

void MeasureTools::SyncDeleteMeasureUI(const unsigned int& id)
{
	if (curr_view_info_.spacing <= 0.0f || curr_view_info_.scale <= 0.0f)
		return;  // not initialized yet.

	this->blockSignals(true);
	EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(true);

	for (auto iter = measure_ui_list_.begin(); iter != measure_ui_list_.end(); ++iter)
	{
		if ((*iter).get()->id() == id)
		{
			measure_ui_list_.erase(iter);
			selected_measure_.reset();
			break;
		}
	}

	EventHandler::GetInstance()->GetMeasureEventHandle().BlockSignals(false);
	this->blockSignals(false);
}

void MeasureTools::SyncModifyMeasureUI(const unsigned int& id)
{
	SyncDeleteMeasureUI(id);
	SyncCreateMeasureUI(id);
}

void MeasureTools::GetMeasureParams(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	common::measure::VisibilityParams* measure_params)
{
	const EventHandleMeasure& handle_measure =
		EventHandler::GetInstance()->GetMeasureEventHandle();
	std::weak_ptr<MeasureData> measure_data;
	handle_measure.EmitSigMeasureGetMeasureData(view_id_, measure_id,
		&measure_data);

	const auto& vp = measure_data.lock()->visibility_params();
	measure_params->center = vp.center;
	measure_params->up = vp.up;
	measure_params->back = vp.back;
}
