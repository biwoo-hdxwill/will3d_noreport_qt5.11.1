#include "plotter_module.h"

#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionFocusRect>
#include <qevent.h>
#include <QLine>
#include <QApplication>

#include <Engine/Common/Common/color_will3d.h>
#include <Engine/Common/Common/W3Cursor.h>

namespace
{
	QPen kPenAxis(Qt::black);
	QPen kPenMinMax("#FF6D3844");
	QPen kPenGrid("#FF404451");
	QPen kPenText("#FFFFFFFF");
	QPen kPenHighLight("#FFFF0000");
	QPen kPenGraph("#FF307CBC");
	QPen kPenLength("#ff00ffff");

	const int kFixedWidth = 730;
	const int kFixedHeight = 320;
	const int kMarginPlotterLeft = 50;
	const int kMarginPlotterRight = 20;
	const int kMarginPlotterTop = 15;
	const int kMarginPlotterBottom = 18;
	const int kMinTicks = 6;
	const int kLengthLinePickMargin = 2;

	QColor kProfileBGActive("#FF4D5160");
	QColor kProfileBGInactive("#FF3D4152");
} // end of namespace

PlotterModule::PlotterModule(QWidget *parent)
	: QWidget(parent)
{
	setMouseTracking(true);

	m_fChartMinY = std::numeric_limits<float>::max();
	m_fChartMaxY = std::numeric_limits<float>::min();
	span_x_ = std::numeric_limits<float>::min();
}

PlotterModule::~PlotterModule(void)
{
}

void PlotterModule::initialize(
	const std::vector<short>& data,
	short min,
	short max,
	float pixel_pitch,
	float length
)
{
	profile_ = data;

	length_ = length;
	pixel_pitch_ = pixel_pitch;
	ticks_x_ = ticks_y_ = 6;

	min_hu_ = min;
	max_hu_ = max;

	m_fChartMinY = min;
	m_fChartMaxY = max;

	no_value_ = (m_fChartMinY >= m_fChartMaxY) ? true : false;
	curr_pos_ = QPoint(-1, -1);

	span_x_ = profile_.size() * pixel_pitch_;

	span_x_ = length_;
	adjust();
	InitProfilePos();

	SetLengthLines();

	setFixedSize(kFixedWidth, kFixedHeight);
}

void PlotterModule::InitProfilePos()
{
	profile_pos_.clear();

	const float span_y = m_fChartMaxY - m_fChartMinY;
	const int rect_x = draw_rect_.width() - 1;
	const int rect_y = draw_rect_.height() - 1;

	profile_pos_.reserve(profile_.size());
	const float real_x = static_cast<float>(rect_x) / span_x_;
	const float real_y = static_cast<float>(rect_y) / span_y;
	for (int i = 0; i < profile_.size(); ++i)
	{
		if (profile_[i] < min_hu_)
		{
			continue;
		}

		const float dx = (span_x_ / static_cast<float>(profile_.size() - 1)) * i;
		const float hu = m_fChartMaxY - profile_[i];
		const float x = draw_rect_.left() + dx * real_x;
		const float y = draw_rect_.top() + hu * real_y;

		profile_pos_.push_back(QPointF(x, y));
	}
}

void PlotterModule::captureImage()
{
	QPixmap pixmap = parentWidget()->grab();
	QString save_path = QFileDialog::getSaveFileName(this, "Save image", "", "Image files (*.png)");
	pixmap.save(save_path, "png");
}

void PlotterModule::DrawCurrentValue(const QPointF& pos)
{
	const int rect_x = draw_rect_.width() - 1;
	const int rect_y = draw_rect_.height() - 1;

#if 1
	float dx = (pos.x() - draw_rect_.left()) / (static_cast<float>(rect_x) / span_x_);
	float index = dx / (span_x_ / static_cast<float>(profile_.size() - 1));
#else
	float index = static_cast<float>(pos.x() - draw_rect_.left()) * span_x_ / (pixel_pitch_ * static_cast<float>(rect_x));
#endif
	pressed_profile_index_ = static_cast<int>(roundf(index));

	if (pressed_profile_index_ < 0 || 
		pressed_profile_index_ >= profile_.size())
	{
		emit sigDisplayInfo(-1, 0);
		return;
	}

	const float span_y = m_fChartMaxY - m_fChartMinY;

	const float real_x = static_cast<float>(rect_x) / span_x_;
	const float real_y = static_cast<float>(rect_y) / span_y;

	dx = (span_x_ / static_cast<float>(profile_.size() - 1)) * pressed_profile_index_;
	const float x = draw_rect_.left() + dx * real_x;
	curr_pos_.setX(static_cast<int>(x));

	const float dy = m_fChartMaxY - profile_[pressed_profile_index_];
	const float y = draw_rect_.top() + dy * real_y;
	curr_pos_.setY(static_cast<int>(y));

	if (no_value_)
	{
		emit sigDisplayInfo(-1, profile_[pressed_profile_index_]);
	}
	else
	{
		emit sigDisplayInfo(pressed_profile_index_, profile_[pressed_profile_index_]);
	}
}

void PlotterModule::SetLengthStartLine(const float start)
{
	SetLengthLines(start, length_end_line_.center().x());

	float max_position = static_cast<float>(draw_rect_.right());
	float min_position = static_cast<float>(draw_rect_.left());
	start_pos_percent_ = (length_start_line_.center().x() - min_position) / (max_position - min_position);
}

void PlotterModule::SetLengthEndLine(const float end)
{
	SetLengthLines(length_start_line_.center().x(), end);

	float max_position = static_cast<float>(draw_rect_.right());
	float min_position = static_cast<float>(draw_rect_.left());
	end_pos_percent_ = (length_end_line_.center().x() - min_position) / (max_position - min_position);
}

void PlotterModule::SetLengthLines(const float start, const float end)
{
	if (profile_pos_.size() < 1)
	{
		return;
	}

	float start_x = start;
	float end_x = end;

	int start_min = std::max(static_cast<int>(profile_pos_.at(0).x()), draw_rect_.left());
	int end_max = std::min(static_cast<int>(profile_pos_.at(profile_pos_.size() - 1).x()), draw_rect_.right());

	if (start_x < start_min)
	{
		start_x = start_min;
		if (end_x < start_x + kLengthLinePickMargin)
		{
			end_x = start_x + kLengthLinePickMargin;
		}
	}
	else if (end_x > end_max)
	{
		end_x = end_max;
		if (start_x > end_x - kLengthLinePickMargin)
		{
			start_x = end_x - kLengthLinePickMargin;
		}
	}
	else if (start_x > end_x - kLengthLinePickMargin)
	{
		return;
	}

	QPointF start_p1(start_x, draw_rect_.topLeft().y());
	QPointF start_p2(start_x, draw_rect_.bottomLeft().y());
	QPointF end_p1(end_x, draw_rect_.topRight().y());
	QPointF end_p2(end_x, draw_rect_.bottomRight().y());

	length_start_line_.setPoints(start_p1, start_p2);
	length_end_line_.setPoints(end_p1, end_p2);
	length_range_line_.setPoints(length_start_line_.center(), length_end_line_.center());

	emit sigChangeLengthStartPos(std::min(std::max(start_pos_percent_, 0.0f), 1.0f));
	emit sigChangeLengthEndPos(std::max(std::min(end_pos_percent_, 1.0f), 0.0f));
}

void PlotterModule::SetLengthLines()
{
	if (profile_pos_.size() < 1)
	{
		return;
	}

	float max_position = static_cast<float>(draw_rect_.right());
	float min_position = static_cast<float>(draw_rect_.left());
	float total_length = max_position - min_position + 1.0f;
	float start_pos = min_position + (total_length * start_pos_percent_);
	float end_pos = min_position + (total_length * end_pos_percent_);

	SetLengthLines(start_pos, end_pos);
}

void PlotterModule::mousePressEvent(QMouseEvent* event)
{
	QWidget::mousePressEvent(event);

	if (event->button() == Qt::LeftButton)
	{
		DrawCurrentValue(event->pos());
	}
}

void PlotterModule::mouseMoveEvent(QMouseEvent* event)
{
	QWidget::mouseMoveEvent(event);

	QPoint pos = event->pos();
	if (event->buttons() == Qt::NoButton)
	{
		QPointF start_p1 = length_start_line_.p1();
		QPointF start_p2 = length_start_line_.p2();
		QPointF end_p1 = length_end_line_.p1();
		QPointF end_p2 = length_end_line_.p2();

		double distance_to_start_line =
			fabs((start_p2.x() - start_p1.x()) * (start_p1.y() - pos.y()) - (start_p1.x() - pos.x()) * (start_p2.y() - start_p1.y())) /
			sqrtf(pow(start_p2.x() - start_p1.x(), 2) + pow(start_p2.y() - start_p1.y(), 2));
		double distance_to_end_line =
			fabs((end_p2.x() - end_p1.x()) * (end_p1.y() - pos.y()) - (end_p1.x() - pos.x()) * (end_p2.y() - end_p1.y())) /
			sqrtf(pow(end_p2.x() - end_p1.x(), 2) + pow(end_p2.y() - end_p1.y(), 2));

		if (distance_to_start_line <= kLengthLinePickMargin)
		{
			QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
			selected_item_ = Item::LENGTH_START_LINE;
		}
		else if (distance_to_end_line <= kLengthLinePickMargin)
		{
			QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
			selected_item_ = Item::LENGTH_END_LINE;
		}
		else
		{
			QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
			selected_item_ = Item::NONE;
		}
	}
	else if (event->buttons() == Qt::LeftButton)
	{
		switch (selected_item_)
		{
		case PlotterModule::NONE:
			DrawCurrentValue(pos);
			break;
		case PlotterModule::LENGTH_START_LINE:
			SetLengthStartLine(pos.x());
			break;
		case PlotterModule::LENGTH_END_LINE:
			SetLengthEndLine(pos.x());
			break;
		default:
			break;
		}
	}
}

void PlotterModule::mouseReleaseEvent(QMouseEvent* event)
{
	QWidget::mouseReleaseEvent(event);

	if (event->button() == Qt::LeftButton)
	{
		DrawCurrentValue(event->pos());
	}
}

void PlotterModule::paintEvent(QPaintEvent *event)
{
	QStylePainter painter(this);
	painter.initFrom(this);
	drawChart(&painter);
	if (hasFocus())
	{
		QStyleOptionFocusRect option;
		option.initFrom(this);
		option.backgroundColor = palette().dark().color();
		painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
	}
	update();
}

void PlotterModule::drawChart(QPainter *painter)
{
	if (!draw_rect_.isValid() ||
		profile_pos_.size() < 2)
	{
		return;
	}

	const float span_y = m_fChartMaxY - m_fChartMinY;
	const int rect_y = draw_rect_.height() - 1;
	if (!no_value_)
	{
		// active area's backround color
		painter->fillRect(draw_rect_, kProfileBGActive);

		// inactive area's background color (top)
		for (int i = 0; i < profile_.size(); ++i)
		{
			if (profile_[i] == max_hu_)
			{
				const float hu = m_fChartMaxY - profile_[i];
				const float y = draw_rect_.top() + hu * rect_y / span_y;
				painter->fillRect(QRect(kMarginPlotterLeft, kMarginPlotterTop,
					draw_rect_.width(), y - kMarginPlotterTop),
					kProfileBGInactive);
				break;
			}
		}

		// inactive area's background color (bottom)
		for (int i = 0; i < profile_.size(); ++i)
		{
			if (profile_[i] == min_hu_)
			{
				const float hu = m_fChartMaxY - profile_[i];
				const float y = draw_rect_.top() + hu * rect_y / span_y;
				painter->fillRect(QRect(kMarginPlotterLeft, y,
					draw_rect_.width(), kMarginPlotterTop + draw_rect_.height() - y),
					kProfileBGInactive);
				break;
			}
		}
	}

	const int rect_x = draw_rect_.width() - 1;
	for (int i = 0; i <= ticks_x_; ++i)
	{
		const int x = draw_rect_.left() + i * rect_x / ticks_x_;
		painter->setPen(i == 0 ? kPenAxis : kPenGrid);
		painter->drawLine(x, draw_rect_.top(), x, draw_rect_.bottom());
		painter->drawLine(x, draw_rect_.bottom(), x, draw_rect_.bottom() + 5);
		const float label = i * span_x_ / static_cast<float>(ticks_x_);
		painter->setPen(kPenText);
		painter->drawText(x - 50, draw_rect_.bottom() + 5,
			100, 20,
			Qt::AlignHCenter | Qt::AlignTop,
			QString::number(label, 'f', 2));
	}

	for (int i = 0; i <= ticks_y_; ++i)
	{
		const int y = draw_rect_.bottom() - i * rect_y / ticks_y_;
		painter->setPen(i == 0 ? kPenAxis : kPenGrid);
		painter->drawLine(draw_rect_.left(), y, draw_rect_.right(), y);
		painter->drawLine(draw_rect_.left() - 5, y, draw_rect_.left(), y);
		const int label = no_value_ ? 0 : static_cast<int>(m_fChartMinY + i * span_y / ticks_y_);
		painter->setPen(kPenText);
		painter->drawText(draw_rect_.left() - kMarginPlotterLeft, y - 10,
			kMarginPlotterLeft - 5, 20,
			Qt::AlignRight | Qt::AlignVCenter,
			QString::number(label));
	}

	//painter->setClipRect(draw_rect_.adjusted(1, 1, -1, -1));

	if (no_value_)
	{
		return;
	}

	bool bMaxDisplayed = false, bMinDisplayed = false;
	for (int i = 0; i < profile_pos_.size(); ++i)
	{
		if (profile_[i] < min_hu_)
		{
			continue;
		}

		const float y = profile_pos_.at(i).y();
		if (bMinDisplayed == false && profile_[i] == min_hu_)
		{
			bMinDisplayed = true;
			painter->setPen(kPenMinMax);
			painter->drawLine(draw_rect_.left(), y, draw_rect_.right(), y);
		}

		if (bMaxDisplayed == false && profile_[i] == max_hu_)
		{
			bMaxDisplayed = true;
			painter->setPen(kPenMinMax);
			painter->drawLine(draw_rect_.left(), y, draw_rect_.right(), y);
		}
	}

	QPolygonF polyline(profile_pos_.size());
	for (int i = 0; i < profile_pos_.size(); i++)
	{
		polyline[i] = profile_pos_.at(i);
	}

	painter->setPen(kPenGraph);
	painter->drawPolyline(polyline);

	// length lines
	painter->setPen(kPenLength);
	painter->drawLine(length_start_line_);
	painter->drawLine(length_end_line_);
	painter->drawLine(length_range_line_);

	float pos_total_length = profile_pos_.at(profile_pos_.size() - 1).x() - profile_pos_.at(0).x() + 1.0f;
	float pos_length = length_end_line_.center().x() - length_start_line_.center().x() + 1.0f;
	float length = span_x_ / draw_rect_.width() * pos_length;
	float margin = fabs((static_cast<float>(draw_rect_.width()) - pos_total_length) * 0.5f);

	QRectF length_label_rect(length_range_line_.p1(), QSizeF(pos_length, draw_rect_.height() / 2));
	length_label_rect.adjust(-margin, 3, margin, 0);
	QRectF length_label_bounding_rect;
	QFlags<Qt::AlignmentFlag> length_label_align = Qt::AlignHCenter | Qt::AlignTop;
	QString length_label = QString::number(length, 'f', 2) + " mm";

#if 1
	painter->setPen(Qt::transparent);
	painter->drawText(length_label_rect, length_label_align, length_label, &length_label_bounding_rect);

	painter->setPen(QPen(kProfileBGInactive));
	painter->setBrush(QBrush(kProfileBGInactive));
	painter->drawRect(length_label_bounding_rect.adjusted(-2, -2, 2, 2));
#endif

	painter->setPen(kPenLength);
	painter->drawText(length_label_bounding_rect, length_label_align, length_label);
	//

	// axis label
	painter->setPen(kPenText);
	painter->drawText(draw_rect_.adjusted(0, 0, -2, -2), Qt::AlignRight | Qt::AlignBottom, "mm");
	painter->drawText(draw_rect_.adjusted(2, 2, 0, 0), Qt::AlignLeft | Qt::AlignTop, "HU");
	//

	if (pressed_profile_index_ < 0 || 
		pressed_profile_index_ >= profile_.size())
	{
		return;
	}

	painter->setPen(kPenText);
	painter->drawLine(curr_pos_.x(), draw_rect_.top(), curr_pos_.x(), draw_rect_.bottom());
	painter->drawLine(draw_rect_.left(), curr_pos_.y(), draw_rect_.right(), curr_pos_.y());

	painter->setPen(kPenHighLight);
	painter->drawLine(curr_pos_.x(), curr_pos_.y() - 5, curr_pos_.x(), curr_pos_.y() + 5);
	painter->drawLine(curr_pos_.x() - 5, curr_pos_.y(), curr_pos_.x() + 5, curr_pos_.y());
}

void PlotterModule::adjust()
{
	float min_x = 0.0f;
	//adjustAxis(min_x, span_x_, ticks_x_);
	adjustAxis(m_fChartMinY, m_fChartMaxY, ticks_y_);

	draw_rect_ = QRect(kMarginPlotterLeft, kMarginPlotterTop,
		kFixedWidth - kMarginPlotterLeft - kMarginPlotterRight,
		kFixedHeight - 2 * kMarginPlotterTop - kMarginPlotterBottom);
}

void PlotterModule::adjustAxis(float &min, float &max, int &numThicks)
{
	const float grossStep = (max - min) / kMinTicks;
	float step = std::pow(10.0f, std::floor(std::log10(grossStep)));

	if (5 * step < grossStep)
		step *= 5;
	else if (2 * step < grossStep)
		step *= 2;

	numThicks = int(std::ceil(max / step) - std::floor(min / step));
	if (numThicks < kMinTicks)
		numThicks = kMinTicks;

	min = std::floor(min / step) * step;
	max = std::ceil(max / step) * step;
}
