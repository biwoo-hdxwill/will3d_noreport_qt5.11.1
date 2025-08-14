#include "profile_line_item.h"

#include <QColor>
#include <QGraphicsScene>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"

#include "W3PathItem.h"

ProfileLineItem::ProfileLineItem(QGraphicsScene* scene, common::measure::PathType type)
	: CW3PathItem_anno(scene, type, false)
{
	length_line_ = new CW3PathItem();
	length_line_->setZValue(line_->zValue() + 1.0f);
	length_line_->setMovable(true);
	scene_->addItem(length_line_);

	SetFullLineColor(QColor(Qt::black));
	SetFullLineWidth(1.0f);

	ApplyLineColor();
}

ProfileLineItem::~ProfileLineItem()
{
	if (length_line_)
	{
		scene_->removeItem(length_line_);
		SAFE_DELETE_OBJECT(length_line_);
	}
}

void ProfileLineItem::AddPoint(const QPointF& point)
{
	CW3PathItem_anno::AddPoint(point);
}

bool ProfileLineItem::EndEdit()
{
	length_line_->setHighlighted(true);

	return CW3PathItem_anno::EndEdit();
}

void ProfileLineItem::setVisible(bool bFlag)
{
	CW3PathItem_anno::setVisible(bFlag);

	if (length_line_)
	{
		length_line_->setVisible(bFlag);
	}		
}

bool ProfileLineItem::IsLineSelected() const
{
	return length_line_->isSelected() || CW3PathItem_anno::IsLineSelected();
}

void ProfileLineItem::DrawingPath()
{
	CW3PathItem_anno::DrawingPath();

	QPainterPath full_path = line_->path().toReversed();
	QPointF length_start_point = full_path.pointAtPercent(start_pos_percent_);
	QPointF length_end_point = full_path.pointAtPercent(end_pos_percent_);
	QPainterPath length_path(length_start_point);
	length_path.lineTo(length_end_point);
	length_line_->setPath(length_path);
}

void ProfileLineItem::GraphicItemsConnection()
{
	CW3PathItem_anno::GraphicItemsConnection();

	connect(length_line_, SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));
	connect(length_line_, SIGNAL(sigHoverPath(bool)), this, SLOT(slotHighlightCurve(bool)));
	connect(length_line_, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslatePath(QPointF)));
}

void ProfileLineItem::SetFullLineColor(const QColor& color)
{
	SetLineColor(color);
}

void ProfileLineItem::SetFullLineWidth(const float width)
{
	SetLineWidth(width);
}

void ProfileLineItem::slotTranslatePath(const QPointF& translate)
{
	length_line_->setPos(QPointF(0.0f, 0.0f));

	CW3PathItem_anno::slotTranslatePath(translate);
}

void ProfileLineItem::slotChangeLengthStartPos(const float percent_to_start)
{
	start_pos_percent_ = percent_to_start;
	DrawingPath();
}

void ProfileLineItem::slotChangeLengthEndPos(const float percent_to_start)
{
	end_pos_percent_ = percent_to_start;
	DrawingPath();
}

void ProfileLineItem::ApplyLineColor()
{
	CW3PathItem_anno::ApplyLineColor();

	QColor line_color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	QPen pen = length_line_->pen();
	pen.setColor(line_color);
	length_line_->setPen(pen);
}
