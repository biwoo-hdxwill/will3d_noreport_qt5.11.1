#include "annotation_handler.h"
/*=========================================================================
Copyright (c) 2015~2018 All rights reserved by HDXWILL.

File: W3AnnotationHandler.cpp
Desc: In header.
=========================================================================*/

#include <Engine\Common\Common\W3Memory.h>
#include <Engine\Common\Common\W3Logger.h>
#include <Engine\Resource\Resource\W3Image2D.h>

#include "W3BaseAnnotation.h"
#include "W3LengthAnnotation.h"
#include "W3DegreeAnnotation.h"
#include "W3ProfileAnnotation.h"
#include "W3ArrowAnnotation.h"
#include "W3TapeAnnotation.h"
#include "W3PolygonAnnotation.h"
#include "W3MemoAnnotation.h"
#include "W3FreeDrawAnnotation.h"
#include "W3CaptureAnnotation.h"

using namespace anno;

AnnotationHandler::~AnnotationHandler() {
	for (int i = 0; i < anno_list_.size(); i++)
		SAFE_DELETE_OBJECT(anno_list_.at(i));
	anno_list_.clear();
}

void AnnotationHandler::Update(const glm::vec3& vp_center, const glm::vec3& vp_up,
							   const glm::vec3& vp_back) {
	UpdateProjection();

	vp_center_ = vp_center;
	vp_up_ = vp_up * 100.0f;
	vp_back_ = vp_back * 100.0f;
	CheckVisibility();
}

void AnnotationHandler::UpdateProjection() {
	emit sigProtterUpdate();
}

void AnnotationHandler::exportProject(QDataStream& out) {}
void AnnotationHandler::importProject(QDataStream& in, const int& version) {}

// 이후 동작들에서 수행 될 이벤트 타입을 결정한다.
void AnnotationHandler::ProcessMousePressed(const QPointF& pt, const glm::vec3& ptVol) {
	selected_anno_ = nullptr;
	SelectEventType();

	switch (event_type_) {
	case EventType::DRAW:
		ActionDrawAnno(pt, ptVol);
		return;
	case EventType::DEL:
		ActionDeleteAnno(pt, ptVol);
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

void AnnotationHandler::ProcessMouseMove(const QPointF& pt, const glm::vec3& ptVol) {
	if (event_type_ == EventType::DRAW) {
		anno_list_.back()->processMouseMove(pt);
	} else if (event_type_ == EventType::SELECT) {
		selected_anno_->processMouseMove(pt);
	} else {
		return;
	}
}

// Pressed 에서 검사된 이벤트 타입에 따른 동작 실행
void AnnotationHandler::ProcessMouseReleased(Qt::MouseButton button,
											 const QPointF& pt, const glm::vec3& ptVol) {
	if (curr_anno_type_ == AnnotationType::FREEDRAW &&
		button == Qt::RightButton) {
		DeleteSelectedAnno();
		return;
	}

	if (curr_anno_type_ == AnnotationType::FREEDRAW && 
		event_type_ == EventType::DRAW) {
		anno_list_.back()->processMouseReleased(pt);
		if (!anno_list_.back()->is_drawing())
			EndAnno();
		else
			DeleteLastAnno();
	}
}

void AnnotationHandler::ProcessMouseDoubleClick(const QPointF& pt, const glm::vec3& ptVol) {
	if (anno_list_.empty())
		return;

	CW3BaseAnnotation* anno = anno_list_.back();
	if (anno->is_drawing()) {
		anno->processMouseDoubleClicked(pt);
		EndAnno();
		event_type_ = EventType::END;
	} else {
		if(selected_anno_)
			selected_anno_->processMouseDoubleClicked(pt);
	}
}

void AnnotationHandler::DeleteAllAnno() {
	if (anno_list_.empty())
		return;

	selected_anno_ = nullptr;

	for (int i = 0; i < anno_list_.size(); i++)
		SAFE_DELETE_OBJECT(anno_list_.at(i));
	anno_list_.clear();
}

void AnnotationHandler::ClearUnfinishedItem() {
	if (anno_list_.empty())
		return;

	if (anno_list_.back()->is_drawing())
		DeleteLastAnno();
}

void AnnotationHandler::SetVisible(bool visible) {
	annotation_visible_ = visible;
	CheckVisibility();
}

void AnnotationHandler::SetViewRenderMode(const ViewRenderMode & mode) {
	view_render_mode_ = mode;
}

void AnnotationHandler::SetCenter(const QPointF& point) {
	curr_view_center_ = point;
	for (auto x : anno_list_)
		x->setViewCenterInScene(point);
}

void AnnotationHandler::SetScale(const float scale, const float scale_vol) {
	scale_ = scale / scale_vol;
	for (auto x : anno_list_) {
		x->SetScale(scale_);
	}
}

void AnnotationHandler::TransformItems(const QTransform & transform) {
	for (auto x : anno_list_)
		x->TransformItems(transform);
}

void AnnotationHandler::RemoveItems() {
	for (auto x : anno_list_) {
		x->RemoveItemAll();

		if (x->type() == anno::AnnotationType::PROFILE)
			((CW3ProfileAnnotation*)x)->hideDialog();
	}
}

void AnnotationHandler::AddItems() {
	for (auto x : anno_list_)
		x->AddItemAll();
}

void AnnotationHandler::SetPixelSpacing(float pixel_spacing) {
	spacing_ = pixel_spacing;
	for (auto x : anno_list_) {
		x->setPixelPitch(spacing_);
	}
}

void AnnotationHandler::SetAnnoType(ETOOL_TYPE toolType) {
	using anno::AnnotationType;
	switch (toolType) {
	case ETOOL_TYPE::MEASUREMENT_RULER:
		curr_anno_type_ = AnnotationType::LENGTH_LINE;
		break;
	case ETOOL_TYPE::MEASUREMENT_TAPELINE:
		curr_anno_type_ = AnnotationType::LENGTH_TAPELINE;
		break;
	case ETOOL_TYPE::MEASUREMENT_TAPECURVE:
		curr_anno_type_ = AnnotationType::LENGTH_CURVE;
		break;
	case ETOOL_TYPE::MEASUREMENT_ANGLE:
		curr_anno_type_ = AnnotationType::ANGLE_THREEPOINT;
		break;
	case ETOOL_TYPE::MEASUREMENT_PROFILE:
		curr_anno_type_ = AnnotationType::PROFILE;
		break;
	case ETOOL_TYPE::MEASUREMENT_AREALINE:
		curr_anno_type_ = AnnotationType::AREA_LINE;
		break;
	case ETOOL_TYPE::MEASUREMENT_ROIRECT:
		curr_anno_type_ = AnnotationType::ROI_RECT;
		break;
	case ETOOL_TYPE::MEASUREMENT_ROICIRCLE:
		curr_anno_type_ = AnnotationType::ROI_CIRCLE;
		break;
	case ETOOL_TYPE::MEASUREMENT_ROIPOLYGON:
		curr_anno_type_ = AnnotationType::ROI_POLYGON;
		break;
	case ETOOL_TYPE::MEASUREMENT_RECTANGLE:
		curr_anno_type_ = AnnotationType::STAMP_RECT;
		break;
	case ETOOL_TYPE::MEASUREMENT_CIRCLE:
		curr_anno_type_ = AnnotationType::STAMP_CIRCLE;
		break;
	case ETOOL_TYPE::MEASUREMENT_ARROW:
		curr_anno_type_ = AnnotationType::STAMP_ARROW;
		break;
	case ETOOL_TYPE::MEASUREMENT_FREEDRAW:
		curr_anno_type_ = AnnotationType::FREEDRAW;
		break;
	case ETOOL_TYPE::MEASUREMENT_MEMO:
		curr_anno_type_ = AnnotationType::MEMO;
		break;
	case ETOOL_TYPE::MEASUREMENT_DELETE:
		curr_anno_type_ = AnnotationType::DELETE_ONE;
		break;
	case ETOOL_TYPE::MEASUREMENT_DELETEALL:
		curr_anno_type_ = AnnotationType::DELETE_ALL;
		break;
	default:
		curr_anno_type_ = AnnotationType::NONE;
	}
}

const bool AnnotationHandler::IsDrawing() const {
	if (anno_list_.empty())
		return false;
	if (anno_list_.back()->is_drawing())
		return true;
	return false;
}
const bool AnnotationHandler::IsSelected() const {
	for (const auto& x : anno_list_) {
		if (x->IsSelected())
			return true;
	}
	return false;
}

void AnnotationHandler::SetProfileData(int anno_index, const std::vector<short>& data,
									   const QPointF & start_pt, const QPointF & end_pt) {
	CW3ProfileAnnotation* anno = dynamic_cast<CW3ProfileAnnotation*>(anno_list_.at(anno_index));
	anno->SetPlotterData(data, start_pt, end_pt);
}

// free draw 를 위한 함수. freedraw 는 다른 툴들과 다르게 셀렉션 모드가 따로 존재
void AnnotationHandler::slotSyncSelectedStatus(bool) {
	QObject* sender = QObject::sender();
	if (curr_anno_type_ == anno::AnnotationType::DELETE_ONE) {
		if (sender) {
			selected_anno_ = dynamic_cast<CW3BaseAnnotation*>(sender);
			DeleteSelectedAnno();
		}
		return;
	}

	for (const auto& x : anno_list_) {
		if (sender == dynamic_cast<QObject*>(x)){
			selected_anno_ = x;
			continue;
		}
		x->setSelected(false);
	}
}

void AnnotationHandler::slotGetPlotterData(const QPointF& start_pt,
										   const QPointF& end_pt) {
	QObject* sender = QObject::sender();
	for (int anno_idx = 0; anno_idx < anno_list_.size(); ++anno_idx) {
		if (sender == dynamic_cast<QObject*>(anno_list_.at(anno_idx))) {
			emit sigGetProfileData(anno_idx, start_pt, end_pt);
			break;
		}
	}
}

void AnnotationHandler::ActionDrawAnno(const QPointF & pt, const glm::vec3& ptVol) {
	if (IsDrawing()) {
		CW3BaseAnnotation* anno = anno_list_.back();
		if (anno->type() == curr_anno_type_) {
			bool done = false;
			if (!anno->InputParam(scene_, pt, ptVol, done))
				DeleteLastAnno();

			if (done)
				EndAnno();
		} else {
			DeleteLastAnno();
			NewAnno(pt, ptVol);
		}
	} else {
		NewAnno(pt, ptVol);
	}
}

void AnnotationHandler::ActionDeleteAnno(const QPointF & pt, const glm::vec3& ptVol) {
	ClearUnfinishedItem();

	for (auto iter = anno_list_.begin(); iter != anno_list_.end(); ++iter) {
		if ((*iter)->IsSelected()) {
			DeleteLastAnno();
			break;
		}
	}

	DeleteSelectedAnno();
	EndAnno();
}

void AnnotationHandler::ActionHighlightAnno() {
	for (auto &i : anno_list_) {
		if (i->is_drawing()) {
			continue;
		} else if (i->IsSelected()) {
			i->setNodeDisplay(true);
		} else {
			i->setNodeDisplay(false);
		}
	}
	slotSyncSelectedStatus(false);
}

void AnnotationHandler::AddAnno(CW3BaseAnnotation * obj,
								   const QPointF & pt, const glm::vec3 & ptVol) {
	bool done = false;
	if (!obj->InputParam(scene_, pt, ptVol, done)) {
		SAFE_DELETE_OBJECT(obj);
		return;
	}
	obj->SetViewParams(vp_center_, vp_up_, vp_back_);
	anno_list_.push_back(obj);

	if (done)
		EndAnno();
}

void AnnotationHandler::NewAnno(const QPointF & pt, const glm::vec3 & ptVol) {
	switch (curr_anno_type_) {
	case AnnotationType::LENGTH_LINE:
		BeginLengthMeasureInsert(pt, ptVol);
		break;
	case AnnotationType::ANGLE_THREEPOINT:
		BeginDegreeMeasureInsert(pt, ptVol);
		break;
	case AnnotationType::PROFILE:
		BeginProfileInsert(pt, ptVol);
		break;
	case AnnotationType::LENGTH_TAPELINE:
		BeginTapeLineInsert(pt, ptVol);
		break;
	case AnnotationType::LENGTH_CURVE:
		BeginTapeCurveInsert(pt, ptVol);
		break;
	case AnnotationType::ROI_RECT:
		BeginROIRect(pt, ptVol);
		break;
	case AnnotationType::AREA_LINE:
		BeginAreaLineInsert(pt, ptVol);
		break;
	case AnnotationType::ROI_CIRCLE:
		BeginROICircle(pt, ptVol);
		break;
	case AnnotationType::ROI_POLYGON:
		BeginROIPolygon(pt, ptVol);
		break;
	case AnnotationType::STAMP_ARROW:
		BeginArrowInsert(pt, ptVol);
		break;
	case AnnotationType::STAMP_CIRCLE:
		BeginCircle(pt, ptVol);
		break;
	case AnnotationType::STAMP_RECT:
		BeginRectangle(pt, ptVol);
		break;
	case AnnotationType::FREEDRAW:
		BeginFreeDraw(pt, ptVol);
		break;
	case AnnotationType::MEMO:
		BeginMemoInsert(pt, ptVol);
		break;

	default:
		break;
	}
}

void AnnotationHandler::EndAnno() {
	// 하나만 그리고 그만 그리고 싶을 때 쓰기위해 만들어둠
	//emit sigAnnotationDone(curr_anno_type_);
	//curr_anno_type_ = AnnotationType::NONE;
	
	ActionHighlightAnno();
}

void AnnotationHandler::DeleteLastAnno() {
	if (anno_list_.back() == selected_anno_)
		selected_anno_ = nullptr;

	SAFE_DELETE_OBJECT(anno_list_.back());
	anno_list_.pop_back();
}

void AnnotationHandler::DeleteSelectedAnno() {
	if (!selected_anno_)
		return;

	for (auto iter = anno_list_.begin(); iter != anno_list_.end(); ++iter) {
		if (*iter == selected_anno_) {
			anno_list_.erase(iter);
			SAFE_DELETE_OBJECT(selected_anno_);
			selected_anno_ = nullptr;
			break;
		}
	}
}

void AnnotationHandler::CheckVisibility() {
	if (anno_list_.empty())
		return;

	for (auto& item : anno_list_) {
		bool item_visible = item->VisibilityCheck(vp_center_, vp_up_, vp_back_);
		item->setVisible(item_visible & annotation_visible_);
	}
}

void AnnotationHandler::SelectEventType() {
	event_type_ = EventType::NONE;
	if (!anno_list_.empty()) {
		if (anno_list_.back()->is_drawing()) {
			event_type_ = EventType::DRAW;
			return;
		}

		for (auto& item : anno_list_) {
			if (item->IsSelected()) {
				item->setNodeDisplay(true);
				selected_anno_ = item;
				event_type_ = EventType::SELECT;
			} else {
				item->setNodeDisplay(false);
			}
		}

		if (event_type_ == EventType::SELECT) {
			if (curr_anno_type_ == AnnotationType::DELETE_ONE) {
				event_type_ = EventType::DEL;
			}
			return;
		}
	}

	if (curr_anno_type_ == AnnotationType::NONE) {
		event_type_ = EventType::NONE;
	} else {
		event_type_ = EventType::DRAW;
	}
}

void AnnotationHandler::BeginLengthMeasureInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3LengthAnnotation *obj = new CW3LengthAnnotation(scale_, spacing_,
													   curr_view_center_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginTapeLineInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3TapeAnnotation *obj = new CW3TapeAnnotation(LineType::LINE, MeasureType::LENGTH,
												   scale_, spacing_,
												   curr_view_center_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginTapeCurveInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3TapeAnnotation *obj = new CW3TapeAnnotation(LineType::CURVE, MeasureType::LENGTH,
												   scale_, spacing_,
												   curr_view_center_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginDegreeMeasureInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3DegreeAnnotation *obj = new CW3DegreeAnnotation();
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginAreaLineInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3TapeAnnotation *obj = new CW3TapeAnnotation(LineType::LINE, MeasureType::AREA,
												   scale_, spacing_,
												   curr_view_center_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginROIRect(const QPointF& pt, const glm::vec3& ptVol) {
	CW3PolygonAnnotation *obj = new CW3PolygonAnnotation(Shape::RECT, MeasureType::ROI,
														 scale_, spacing_,
														 curr_view_center_, image_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginROICircle(const QPointF& pt, const glm::vec3& ptVol) {
	CW3PolygonAnnotation *obj = new CW3PolygonAnnotation(Shape::CIRCLE, MeasureType::ROI,
														 scale_, spacing_,
														 curr_view_center_, image_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginROIPolygon(const QPointF& pt, const glm::vec3& ptVol) {
	CW3PolygonAnnotation *obj = new CW3PolygonAnnotation(Shape::POLYGON, MeasureType::ROI,
														 scale_, spacing_,
														 curr_view_center_, image_);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginProfileInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3ProfileAnnotation *obj = new CW3ProfileAnnotation(scale_, spacing_,
														 curr_view_center_,
														 profile_index_++);
	connect(obj, SIGNAL(sigPlotterWasClosed(CW3ProfileAnnotation*)),
			this, SLOT(slotPlotterWasClosed(CW3ProfileAnnotation*)));
	connect(obj, SIGNAL(sigGetPlotterData(QPointF, QPointF)),
			this, SLOT(slotGetPlotterData(QPointF, QPointF)));

	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginRectangle(const QPointF& pt, const glm::vec3& ptVol) {
	CW3PolygonAnnotation *obj = new CW3PolygonAnnotation(Shape::RECT, MeasureType::JUSTDRAW);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginCircle(const QPointF& pt, const glm::vec3& ptVol) {
	CW3PolygonAnnotation *obj = new CW3PolygonAnnotation(Shape::CIRCLE, MeasureType::JUSTDRAW);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginFreeDraw(const QPointF& pt, const glm::vec3& ptVol) {
	CW3FreeDrawAnnotation *obj = new CW3FreeDrawAnnotation();
	connect(obj, SIGNAL(sigSelected(bool)), this, SLOT(slotSyncSelectedStatus(bool)));
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginArrowInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3ArrowAnnotation *obj = new CW3ArrowAnnotation();
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginMemoInsert(const QPointF& pt, const glm::vec3& ptVol) {
	CW3MemoAnnotation *obj = new CW3MemoAnnotation(memo_index_++);
	AddAnno(obj, pt, ptVol);
}

void AnnotationHandler::BeginCapture(const QPointF& pt) {
	CW3CaptureAnnotation *obj = new CW3CaptureAnnotation();
	anno_list_.push_back(obj);
}

void AnnotationHandler::slotPlotterWasClosed(CW3ProfileAnnotation *pPlotter) {
	std::vector<CW3BaseAnnotation *>::iterator iter;
	for (iter = anno_list_.begin(); iter != anno_list_.end();) {
		if ((*iter) == pPlotter) {
			SAFE_DELETE_OBJECT(*iter);
			iter = anno_list_.erase(iter);
		} else {
			++iter;
		}
	}
}
