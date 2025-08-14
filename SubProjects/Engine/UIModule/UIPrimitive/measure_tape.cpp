#include "measure_tape.h"

#include <qrect.h>
#include <qpolygon.h>
#include <qpainterpath.h>

#include <Engine/Common/Common/W3Math.h>
#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/global_preferences.h>

#include "W3PathItem_anno.h"

using namespace common;

MeasureTape::MeasureTape(common::measure::PathType path_type, common::measure::DrawMode draw_mode)
	: draw_mode_(draw_mode)
	, path_type_(path_type)
{

}

MeasureTape::~MeasureTape()
{

}

void MeasureTape::setSelected(bool bSelected)
{
	is_selected_ = bSelected;
	setNodeDisplay(is_selected_);
}

bool MeasureTape::IsSelected() const
{
	if (is_drawing_)
		return false;
	if (ui_line_->isSelected() || is_selected_)
		return true;
	else
		return false;
}

void MeasureTape::processMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (ui_line_->IsLineSelected())
	{
		data_.lock().get()->set_points(ui_line_->getCurveData());
	}

	processing(pt);
	if (is_drawing_)
		ui_line_->drawingCurPath(pt);
}

void MeasureTape::processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol)
{
	ui_line_->EndEdit();
	is_drawing_ = false;
}

bool MeasureTape::InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done)
{
	is_drawing_ = true;

	if (node_count_ == 0)
	{
		ui_line_.reset(new CW3PathItem_anno(pScene, path_type_,
			draw_mode_ == measure::DrawMode::LENGTH ? false : true));
	}
	else
	{
		std::vector<QPointF> points = ui_line_->getCurveData();
		if (points.back() == pt)
		{
			return true;
		}
	}

	if (draw_mode_ == measure::DrawMode::AREA)
		ui_line_->setFixLabelPos(0);

	ui_line_->AddPoint(pt);
	processing(pt);
	ui_line_->drawingCurPath(pt);

	++node_count_;
	return true;
}

void MeasureTape::processing(const QPointF& pt)
{
	if (draw_mode_ == measure::DrawMode::LENGTH)
	{
		length_list_.clear();
		float Length = 0.0f;

		if (path_type_ == measure::PathType::LINE)
		{
			std::vector<QPointF> pNodeArr = ui_line_->getCurveData();
			if (pNodeArr.size() == 1)
			{
				Length = W3::PixelDist(pNodeArr.at(0), pt);
				if (Length != 0.0f)
					length_list_.push_back(Length);
			}
			else
			{
				for (int i = 1; i < pNodeArr.size(); ++i)
				{
					Length = W3::PixelDist(pNodeArr.at(i), pNodeArr.at(i - 1));
					if (Length != 0.0f)
						length_list_.push_back(Length);
				}

				if (is_drawing_)
				{
					Length = W3::PixelDist(pNodeArr.at(pNodeArr.size() - 1), pt);
					if (Length != 0.0f)
						length_list_.push_back(Length);
				}
			}
		}
		else if (path_type_ == measure::PathType::CURVE)
		{
			std::vector<QPointF> pNodeArr = ui_line_->getCurveData();
			std::vector<QPointF> pCurveArr = ui_line_->getSplineData();

			if (pNodeArr.size() == 1)
			{
				Length = W3::PixelDist(pNodeArr.at(0), pt);
				if (Length != 0.0f)
					length_list_.push_back(Length);
			}
			else
			{
				QRectF Region;
				int dFoundCnt = 0;
				float l = 0.f;
				for (int i = 1; i < pCurveArr.size(); i++)
				{
					Length += W3::PixelDist(pCurveArr.at(i - 1), pCurveArr.at(i));
#if 0
					if (dFoundCnt < pNodeArr.size())
					{
						//영역을 정해논 이유가 뭔지 모르겠음.
						Region = QRectF(pNodeArr.at(dFoundCnt).rx() - 2.5f,
							pNodeArr.at(dFoundCnt).ry() - 2.5f, 5.0f, 5.0f);
						if (Region.contains(pCurveArr.at(i - 1))) {

							if (dFoundCnt != 0)
							{
								if (Length > 1.0f)
									length_list_.push_back(Length);
								Length = 0.0f;
							}
							++dFoundCnt;
						}
					}
				}
#else
				}

				length_list_.push_back(Length);
#endif
			}
		}
	}
	else if (draw_mode_ == measure::DrawMode::AREA)
	{
		QPolygonF pf;
		if (is_drawing_)
			pf = ui_line_->getSimplipiedPolygon(pt);
		else
			pf = ui_line_->getSimplipiedPolygon();

		if (pf.size() < 2)
			return;
	}

	UpdateMeasure();
}

void MeasureTape::UpdateLength()
{
	std::vector<QPointF> node = ui_line_->getCurveData();
	int size = node.size();
	if (size < 2)
	{
		return;
	}

	if (draw_mode_ == measure::DrawMode::LENGTH)
	{
		length_list_.clear();
		float length = 0.0f;

		if (path_type_ == measure::PathType::LINE)
		{
			for (int i = 1; i < node.size(); ++i)
			{
				length = W3::PixelDist(node.at(i), node.at(i - 1));
				if (length != 0.0f)
				{
					length_list_.push_back(length);
				}
			}
		}
		else if (path_type_ == measure::PathType::CURVE)
		{
			std::vector<QPointF> spline_node = ui_line_->getSplineData();
			QRectF region;
			int found_cnt = 0;
			for (int i = 1; i < spline_node.size(); ++i)
			{
				length += W3::PixelDist(spline_node.at(i - 1), spline_node.at(i));
#if 0
				if (found_cnt < node.size())
				{
					region = QRectF(node.at(found_cnt).rx() - 2.5f, node.at(found_cnt).ry() - 2.5f, 5.0f, 5.0f);
					if (region.contains(spline_node.at(i - 1)))
					{
						if (found_cnt != 0)
						{
							if (length > 1.0f)
							{
								length_list_.push_back(length);
							}
							length = 0.0f;
						}
						++found_cnt;
					}
				}
			}
#else
			}

			length_list_.push_back(length);
#endif
		}

		UpdateMeasure();
	}
}

void MeasureTape::setNodeDisplay(bool bDisplay)
{
	ui_line_->displayNode(bDisplay);
}

void MeasureTape::UpdateMeasure()
{
	if (!ui_line_)
		return;

	const float& pixel_pitch = data_.lock().get()->pixel_pitch();
	const float& scale = data_.lock().get()->scale();
	const float unit_scale = pixel_pitch * scale;

	if (draw_mode_ == measure::DrawMode::LENGTH)
	{
		bool is_multi_label = GlobalPreferences::GetInstance()->preferences_.objects.measure.tape_line_multi_label;
		if (!is_multi_label || path_type_ == common::measure::PathType::CURVE)
		{
			float length = 0.f;
			for (int i = 0; i < length_list_.size(); ++i)
			{
				length += length_list_.at(i) * unit_scale;
			}
			float nearest = roundf(length * 100.f) / 100.f;
			ui_line_->setLabel(QString("%1 [mm]").arg(QString::number(nearest)));
		}
		else if(is_multi_label && path_type_ == common::measure::PathType::LINE)
		{
			std::vector<QString> length_list;
			for (int i = 0; i < length_list_.size(); ++i)
			{
				float length = roundf((length_list_.at(i) * unit_scale) * 100.f) / 100.f;

				length_list.push_back(QString("%1 [mm]").arg(QString::number(length)));
			}
			ui_line_->setMultiLabel(&length_list);
		}
	}
	else if (draw_mode_ == measure::DrawMode::AREA)
	{
		QPolygonF pf = ui_line_->getSimplipiedPolygon();
		float fSum = 0.0f;
		for (int i = 0; i < pf.size() - 1; ++i) 
		{
			fSum += (pf.at(i).x() * unit_scale + pf.at(i + 1).x() * unit_scale)
				* (pf.at(i).y() * unit_scale - pf.at(i + 1).y() * unit_scale);
		}
		fSum = fabs(fSum)*0.5f;
		float nearest = roundf(fSum * 100) / 100;
		ui_line_->setLabel(QString("%1 [mm2]").arg(QString::number(nearest)));
	}
}

void MeasureTape::setVisible(bool bShow) 
{
	ui_line_->setVisible(bShow);
}

std::vector<QPointF> MeasureTape::GetMeasurePoints() const 
{
	if (ui_line_)
		return ui_line_->getCurveData();
	else
		return std::vector<QPointF>();
}

bool MeasureTape::TransformItems(const QTransform& transform)
{
	if (ui_line_)
	{
		const std::vector<QPointF> before = GetMeasurePoints();
		ui_line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		UpdateLength();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

void MeasureTape::ApplyPreferences() 
{
	if (ui_line_)
		ui_line_->ApplyPreferences();
}

QString MeasureTape::GetValue() 
{
	return ui_line_->getLabel();
}
