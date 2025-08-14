#include "annotation_arrow.h"

#include <QDebug>

#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/W3Math.h>
#include "W3ArrowItem_anno.h"

bool AnnotationArrow::TransformItems(const QTransform & transform) {
	if (ui_arrow_) {
		const std::vector<QPointF> before = GetMeasurePoints();
		ui_arrow_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

AnnotationArrow::AnnotationArrow() {
}

AnnotationArrow::~AnnotationArrow() {
}

bool AnnotationArrow::IsSelected() const {
	if (is_drawing_)
		return false;
	if (is_selected_ || ui_arrow_->isSelected())
		return true;
	else
		return false;
}

void AnnotationArrow::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

void AnnotationArrow::processMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (ui_arrow_->isSelected())
	{
		data_.lock().get()->set_points(ui_arrow_->getCurveData());
		//return;
	}

	ui_arrow_->drawingCurPath(pt);
}

void AnnotationArrow::setNodeDisplay(bool bDisplay) {
	if (ui_arrow_)
		ui_arrow_->setNodeDisplay(bDisplay);
}

bool AnnotationArrow::InputParam(QGraphicsScene* pScene, const QPointF& pt,
	const glm::vec3& ptVol, bool& done) {
	bool bProcessed = true;
	is_drawing_ = true;
	switch (node_count_) {
	case 0:
		ui_arrow_.reset(new CW3ArrowItem_anno(pScene));
		ui_arrow_->addPoint(pt);
		ui_arrow_->drawingCurPath(pt);
		break;
	case 1:
		if (ui_arrow_->getCurveData().at(0) == pt)
			bProcessed = false;
		ui_arrow_->addPoint(pt);
		ui_arrow_->endEdit();
		is_drawing_ = false;
		done = true;
		break;
	}
	++node_count_;
	return bProcessed;
}

void AnnotationArrow::setVisible(bool bShow) {
	ui_arrow_->setVisible(bShow);
}

std::vector<QPointF> AnnotationArrow::GetMeasurePoints() const {
	if (ui_arrow_)
		return ui_arrow_->getCurveData();

	return std::vector<QPointF>();
}

void AnnotationArrow::ApplyPreferences() {
	if (ui_arrow_)
		ui_arrow_->ApplyPreferences();
}
