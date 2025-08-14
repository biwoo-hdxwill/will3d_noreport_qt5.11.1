#include "measure_line.h"

#include <QDebug>

#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/W3Math.h>
#include "W3PathItem_anno.h"

MeasureLine::MeasureLine() {}
MeasureLine::~MeasureLine() {}

bool MeasureLine::IsSelected() const
{
	if (is_drawing_)
	{
		return false;
	}

	if (line_->isSelected() || is_selected_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void MeasureLine::setSelected(bool bSelected)
{
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

void MeasureLine::processMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (line_->IsLineSelected())
	{
		data_.lock().get()->set_points(line_->getCurveData());
		//return;
	}

	line_->drawingCurPath(pt);
}

bool MeasureLine::InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done)
{
	bool processed = true;
	is_drawing_ = true;
	switch (node_count_)
	{
	case 0:
		line_.reset(new CW3PathItem_anno(pScene));
		line_->AddPoint(pt);
		line_->drawingCurPath(pt);
		break;
	case 1:
		if (line_->getCurveData().at(0) == pt)
		{
			processed = false;
		}

		line_->AddPoint(pt);
		line_->EndEdit();
		is_drawing_ = false;
		done = true;
		break;
	}
	++node_count_;

	return processed;
}

void MeasureLine::setNodeDisplay(bool bDisplay)
{
	line_->displayNode(bDisplay);
}

void MeasureLine::setVisible(bool bShow)
{
	line_->setVisible(bShow);
}

std::vector<QPointF> MeasureLine::GetMeasurePoints() const
{
	if (line_)
	{
		return line_->getCurveData();
	}
	else
	{
		return std::vector<QPointF>();
	}
}

bool MeasureLine::TransformItems(const QTransform & transform)
{
	if (line_)
	{
		const std::vector<QPointF> before = GetMeasurePoints();
		line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

void MeasureLine::ApplyPreferences()
{
	if (line_)
	{
		line_->ApplyPreferences();
	}
}
