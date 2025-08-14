#include "measure_polygon.h"

#include <qrect.h>
#include <qpolygon.h>

#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/W3Math.h>
#include <Engine/Resource/Resource/include/measure_data.h>

#include "W3PolygonItem_anno.h"
#include "W3PathItem_anno.h"

using namespace common;

MeasurePolygon::MeasurePolygon(measure::Shape eShapeType,
							   measure::DrawMode eMeasureType) :
	measure_type_(eMeasureType), shape_type_(eShapeType) {}

MeasurePolygon::~MeasurePolygon() {}

void MeasurePolygon::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

bool MeasurePolygon::IsSelected() const {
	if (is_drawing_)
		return false;

	bool bIsSelected = false;
	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE)
		bIsSelected = polygon_->isSelected();
	else if (shape_type_ == measure::Shape::POLYGON)
		bIsSelected = line_->isSelected();

	return (bIsSelected || is_selected_) ? true : false;
}

void MeasurePolygon::processMouseMove(const QPointF& pt, const glm::vec3& ptVol) {
	if (shape_type_ == measure::Shape::POLYGON)
		if (line_->getSelectedLabel() != -1)
			return;

	processing(pt);

	if (shape_type_ == measure::Shape::POLYGON)
	{
		if (line_->IsLineSelected())
		{
			data_.lock().get()->set_points(line_->getCurveData());
		}
	} 
	else
	{
		if (polygon_->isSelected())
		{
			data_.lock().get()->set_points(polygon_->GetTwoPoints());
		}
	}
}

void MeasurePolygon::processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol) {
	if (shape_type_ == measure::Shape::POLYGON) {
		is_drawing_ = false;
		line_->EndEdit();
		processing(pt);
	}
}

bool MeasurePolygon::InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) {
	is_drawing_ = true;

	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE) {
		if (node_count_ == 0) {
			polygon_.reset(new CW3PolygonItem_anno(pScene, shape_type_));
			polygon_->AddPoint(pt);
			polygon_->drawingCurPath(pt);
		} else {
			std::vector<QPointF> points = polygon_->getPoints();
			if (points.at(0) == pt) {
				return false;
			}

			polygon_->AddPoint(pt);
			polygon_->endEdit();
			done = true;
			is_drawing_ = false;
		}
	} else if (shape_type_ == measure::Shape::POLYGON) {
		if (node_count_ == 0)
			line_.reset(new CW3PathItem_anno(pScene, measure::PathType::LINE, true));
		line_->setFixLabelPos(0);
		line_->AddPoint(pt);

		processing(pt);
	}

	++node_count_;
	return true;
}

void MeasurePolygon::processing(const QPointF& pt_scene) {
	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE) {
		polygon_->drawingCurPath(pt_scene);
	}

	if (measure_type_ != measure::DrawMode::ROI)
		return;

	polygon_->drawingCurPath(pt_scene);

	UpdateMeasure();
}

void MeasurePolygon::setNodeDisplay(bool bDisplay) {
	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE)
		polygon_->displayNode(bDisplay);
	else if (shape_type_ == measure::Shape::POLYGON)
		line_->displayNode(bDisplay);
}

void MeasurePolygon::UpdateMeasure() {
	if (measure_type_ != measure::DrawMode::ROI)
		return;

	QRectF bb = polygon_->getRect();

	QPointF pt_1 = bb.topLeft();
	QPointF pt_2 = bb.bottomRight();
	QPointF start_pos_scene(std::min(pt_1.x(), pt_2.x()), std::min(pt_1.y(), pt_2.y()));
	QPointF end_pos_scene(std::max(pt_1.x(), pt_2.x()), std::max(pt_1.y(), pt_2.y()));

	std::vector<short> data;
	emit sigGetROIData(start_pos_scene, end_pos_scene, data);

	if (data.empty())
		return;

	short min = std::numeric_limits<short>::max();
	short max = std::numeric_limits<short>::min();
	int sum = 0;
	int valid_data_cnt = 0;
	for (const auto& value : data) {
		if (value == common::dicom::kInvalidHU)
			continue;
		sum += value;
		if (value > max)	max = value;
		if (value < min)	min = value;

		++valid_data_cnt;
	}
	float avg = static_cast<float>(sum) / valid_data_cnt;

	float std_var_sum = 0.0f;
	for (const auto& value : data) {
		if (value == common::dicom::kInvalidHU)
			continue;

		std_var_sum += (value - avg) * (value - avg);
	}
	float std_var = sqrt(std_var_sum / valid_data_cnt);

	const auto& data_ptr = data_.lock().get();
	QPointF size = (end_pos_scene - start_pos_scene)*data_ptr->scale()*data_ptr->pixel_pitch();
	QString strLabel = QString("[ROI] %1 x %2 [mm] \nMin : %3\nMax : %4\nAvg : %5\nStd : %6")
		.arg(QString::number(roundf(fabsf(size.x()) * 100) / 100))
		.arg(QString::number(roundf(fabsf(size.y()) * 100) / 100))
		.arg(QString::number(min))
		.arg(QString::number(max))
		.arg(QString::number(roundf(avg * 100) / 100))
		.arg(QString::number(roundf(std_var * 100) / 100));

	polygon_->setLabel(strLabel);
}

void MeasurePolygon::setVisible(bool bShow) {
	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE)
		polygon_->setVisible(bShow);
	else if (shape_type_ == measure::Shape::POLYGON)
		line_->setVisible(bShow);
}

bool MeasurePolygon::TransformItems(const QTransform & transform) {
	const std::vector<QPointF> before = GetMeasurePoints();
	if (shape_type_ == measure::Shape::RECT || shape_type_ == measure::Shape::CIRCLE) {
		polygon_->transformItems(transform);
	} else {
		line_->transformItems(transform);
	}
	const std::vector<QPointF>& after = GetMeasurePoints();
	return W3::IsEqualPoint(before.at(0), after.at(0));
}

std::vector<QPointF> MeasurePolygon::GetMeasurePoints() const {
	if (measure_type_ == measure::DrawMode::ROI ||
		shape_type_ == measure::Shape::RECT ||
		shape_type_ == measure::Shape::CIRCLE) {
		QRectF bb = polygon_->getRect();

		QPointF pt_1 = bb.topLeft();
		QPointF pt_2 = bb.bottomRight();
		QPointF start_pos_scene(std::min(pt_1.x(), pt_2.x()), std::min(pt_1.y(), pt_2.y()));
		QPointF end_pos_scene(std::max(pt_1.x(), pt_2.x()), std::max(pt_1.y(), pt_2.y()));
		std::vector<QPointF> roi_data = { start_pos_scene , end_pos_scene };
		return roi_data;
	}

	return std::vector<QPointF>();
}

void MeasurePolygon::ApplyPreferences() {
	if (line_)
		line_->ApplyPreferences();

	if (polygon_)
		polygon_->ApplyPreferences();
}

QString MeasurePolygon::GetValue() {
	if (measure_type_ != measure::DrawMode::ROI)
		return QString();

	return polygon_->getLabel();
}
