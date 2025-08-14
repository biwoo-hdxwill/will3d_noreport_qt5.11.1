#include "measure_length.h"

#include <QDebug>

#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/W3Math.h>
#include "W3PathItem_anno.h"

MeasureLength::MeasureLength() {}
MeasureLength::~MeasureLength() {}

bool MeasureLength::IsSelected() const {
	if (is_drawing_)
		return false;
	if (line_->isSelected() || is_selected_)
		return true;
	else
		return false;
}

void MeasureLength::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

void MeasureLength::processMouseMove(const QPointF& pt, const glm::vec3& ptVol) {
	if (line_->IsLineSelected())
	{
		data_.lock().get()->set_points(line_->getCurveData());
	}


	if (line_->getCurveData().size() == 1)
	{
		pixel_dist_ = GetPixelDistance(line_->getCurveData().at(0), pt);
	}
	else
	{
		pixel_dist_ = GetPixelDistance(line_->getCurveData().at(0), line_->getCurveData().at(1));
	}

	UpdateMeasure();
	line_->drawingCurPath(pt);
}

bool MeasureLength::InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) {
	bool processed = true;
	is_drawing_ = true;
	switch (node_count_) {
	case 0:
		line_.reset(new CW3PathItem_anno(pScene));
		line_->AddPoint(pt);
		line_->setLabel(QString("0 [mm]"));
		line_->drawingCurPath(pt);
		break;
	case 1:
		if (line_->getCurveData().at(0) == pt)
			processed = false;

		line_->AddPoint(pt);
		pixel_dist_ = GetPixelDistance(line_->getCurveData().at(0), line_->getCurveData().at(1));
		UpdateMeasure();
		line_->EndEdit();
		is_drawing_ = false;
		done = true;		
		break;
	}
	++node_count_;

	return processed;
}

void MeasureLength::setNodeDisplay(bool bDisplay) {
	line_->displayNode(bDisplay);
}

void MeasureLength::UpdateMeasure() {
	const auto& data_ptr = data_.lock().get();

	if (!data_ptr)
	{
		return;
	}

	data_.lock().get()->set_points(line_->getCurveData());

	float length = pixel_dist_ * data_ptr->pixel_pitch() * data_ptr->scale();
	float nearest = roundf(length * 100) / 100;
	line_->setLabel(QString("%1 [mm]").arg(QString::number(nearest)));
}

void MeasureLength::setVisible(bool bShow) 
{
	line_->setVisible(bShow);
}

std::vector<QPointF> MeasureLength::GetMeasurePoints() const {
	if (line_)
		return line_->getCurveData();
	else
		return std::vector<QPointF>();
}

bool MeasureLength::TransformItems(const QTransform& transform) 
{
	if (line_) 
	{
		const std::vector<QPointF> before = GetMeasurePoints();
		line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();		
		
		if (line_->getCurveData().size() == 2)
		{
			pixel_dist_ = GetPixelDistance(line_->getCurveData().at(0), line_->getCurveData().at(1));
			UpdateMeasure();
		}

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

void MeasureLength::ApplyPreferences() {
	if (line_) {
		line_->ApplyPreferences();
	}
}

QString MeasureLength::GetValue() {
	return line_->getLabel();
}

const float MeasureLength::GetPixelDistance(QPointF p1, QPointF p2)
{
	QPointF fDist = p1 - p2;
	fDist.setX(fabs(fDist.rx()));
	fDist.setY(fabs(fDist.ry()));
	return sqrt((float)(fDist.rx() * fDist.rx() + fDist.ry() * fDist.ry()));
}
