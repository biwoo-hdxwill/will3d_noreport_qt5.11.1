#include "annotation_freedraw.h"

#include <Engine/Resource/Resource/include/measure_data.h>
#include <Engine/Common/Common/define_measure.h>
#include <Engine/Common/Common/W3Math.h>
#include "pathitem_freedraw.h"

AnnotationFreedraw::AnnotationFreedraw()
{
}

AnnotationFreedraw::~AnnotationFreedraw()
{
}

void AnnotationFreedraw::setSelected(bool bSelected)
{
	ui_line_->setSelected(bSelected);
}

bool AnnotationFreedraw::IsSelected() const
{
	if (ui_line_->IsSelected())
		return true;

	return false;
}

void AnnotationFreedraw::processMouseMove(const QPointF& pt, const glm::vec3& ptVol)
{
	if (is_drawing_)
		ui_line_->addPoint(pt);

	if (ui_line_->IsSelected())
	{
		data_.lock().get()->set_points(ui_line_->getData());
	}
}

void AnnotationFreedraw::processMouseReleased(const QPointF & pt, const glm::vec3 & ptVol)
{
	if (ui_line_->endEdit())
	{
		is_drawing_ = false;
	}
}

bool AnnotationFreedraw::InputParam(QGraphicsScene* pScene, const QPointF& pt,
	const glm::vec3& ptVol, bool& done)
{
	is_drawing_ = true;

	if (node_count_ == 0)
	{
		ui_line_.reset(new PathitemFreedraw(pScene));
		connect(ui_line_.get(), SIGNAL(sigSelected(bool)), this, SIGNAL(sigSelected(bool)));
		ui_line_->addPoint(pt);
	}

	ui_line_->drawingCurPath(pt);

	++node_count_;
	return true;
}

void AnnotationFreedraw::setNodeDisplay(bool bDisplay)
{
	//m_pLine->displayNode(bDisplay);
}

void AnnotationFreedraw::setVisible(bool bShow)
{
	ui_line_->setVisible(bShow);
}

bool AnnotationFreedraw::TransformItems(const QTransform & transform)
{
	if (ui_line_)
	{
		const std::vector<QPointF> before = GetMeasurePoints();
		ui_line_->transformItems(transform);
		const std::vector<QPointF>& after = GetMeasurePoints();

		return W3::IsEqualPoint(before.at(0), after.at(0));
	}
	return false;
}

std::vector<QPointF> AnnotationFreedraw::GetMeasurePoints() const
{
	if (ui_line_)
		return ui_line_->getData();
	return std::vector<QPointF>();
}

QPolygonF AnnotationFreedraw::GetPolygon()
{
	if (ui_line_)
	{
		return ui_line_->GetPolygon();
	}
	else
		return QPolygonF();
}

void AnnotationFreedraw::ApplyPreferences()
{
	if (ui_line_)
		ui_line_->ApplyPreferences();
}

void AnnotationFreedraw::SetLineColor(const QColor& color)
{
	if (ui_line_)
		ui_line_->SetLineColor(color);
}

void AnnotationFreedraw::SetLineWidth(const float width)
{
	if (ui_line_)
	{
		ui_line_->SetLineWidth(width);
	}
}
