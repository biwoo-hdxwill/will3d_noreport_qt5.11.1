#include "measure_degree.h"

#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/W3Math.h>
#include <Engine/Common/Common/define_measure.h>

#include "W3PathItem_anno.h"

MeasureDegree::MeasureDegree() {
}

MeasureDegree::~MeasureDegree() {
}

void MeasureDegree::setSelected(bool bSelected) {
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

bool MeasureDegree::IsSelected() const {
	if (is_drawing_)
		return false;
	if (m_pLine->isSelected() || is_selected_)
		return true;
	else
		return false;
}

void MeasureDegree::processMouseMove(const QPointF& pt, const glm::vec3& ptVol) 
{
	if (m_pLine->IsLineSelected())
	{
		data_.lock().get()->set_points(m_pLine->getCurveData());
	}

	m_pLine->drawingCurPath(pt);
	if (node_count_ >= 2) {
		const std::vector<QPointF>& ptArr = m_pLine->getCurveData();
		float angle = 0.0f;
		if (ptArr.size() == 2)
			angle = W3::getAngle(ptArr.at(0), ptArr.at(1), pt);
		else
			angle = W3::getAngle(ptArr.at(0), ptArr.at(1), ptArr.at(2));
		m_pLine->setFixLabelPos(1);
		SetAngleText(angle);
	}
}

bool MeasureDegree::InputParam(QGraphicsScene* pScene, const QPointF& pt,
							   const glm::vec3& ptVol, bool& done) {
	bool bProcessed = true;
	is_drawing_ = true;
	switch (node_count_) {
	case 0:
		m_pLine.reset(new CW3PathItem_anno(pScene));
		m_pLine->AddPoint(pt);
		m_pLine->drawingCurPath(pt);
		m_pLine->setGuideType(common::measure::GuideType::ARC);
		break;
	case 1:
		if (m_pLine->getCurveData().at(0) == pt)
			bProcessed = false;
		m_pLine->AddPoint(pt);
		m_pLine->drawingCurPath(pt);
		break;
	case 2:
		if (m_pLine->getCurveData().at(0) == pt || m_pLine->getCurveData().at(1) == pt)
			bProcessed = false;
		m_pLine->AddPoint(pt);

		float angle = W3::getAngle(m_pLine->getCurveData().at(0),
								   m_pLine->getCurveData().at(1),
								   m_pLine->getCurveData().at(2));
		SetAngleText(angle);
		m_pLine->EndEdit();
		is_drawing_ = false;
		done = true;
		break;
	}
	++node_count_;
	return bProcessed;
}

void MeasureDegree::setNodeDisplay(bool bDisplay) {
	m_pLine->displayNode(bDisplay);
}

void MeasureDegree::setVisible(bool bShow) {
	m_pLine->setVisible(bShow);
}

bool MeasureDegree::TransformItems(const QTransform & transform) {
	if (m_pLine) {
		const std::vector<QPointF> before = GetMeasurePoints();
		m_pLine->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

std::vector<QPointF> MeasureDegree::GetMeasurePoints() const {
	if (m_pLine)
		return m_pLine->getCurveData();
	else
		return std::vector<QPointF>();
}

void MeasureDegree::SetAngleText(const float degree) {
	float nearest = roundf(degree * 10) / 10;
	m_pLine->setLabel(QString("%1 [deg]").arg(QString::number(nearest)));
}

void MeasureDegree::ApplyPreferences() {
	if (m_pLine)
		m_pLine->ApplyPreferences();
}

QString MeasureDegree::GetValue() {
	return m_pLine->getLabel();
}
