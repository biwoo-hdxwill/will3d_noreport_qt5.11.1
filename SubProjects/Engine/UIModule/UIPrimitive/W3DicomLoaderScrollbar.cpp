#include "W3DicomLoaderScrollbar.h"

#include <math.h>

#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Theme.h"

CW3DicomLoaderScrollbar::CW3DicomLoaderScrollbar(QWidget* parent /*= nullptr*/)
	: QGraphicsView(parent)
{
	m_nSelectedCtrl = -1;
	m_fStart = m_fMiddle = m_fEnd = 0;
	m_fFirst = m_fLast = 0;

	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setStyleSheet("background: transparent; border: 0px");
	this->setContentsMargins(1, 1, 1, 1);

	QGraphicsScene *pScene = new QGraphicsScene(this);
	this->setScene(pScene);
	this->setBackgroundBrush(Qt::transparent);
	this->setMouseTracking(true);
	QRectF rt = rect();

	m_fPos[0] = m_fPos[1] = m_fPos[2] = 0;
	m_fNodeWidth = 14;
	bar_height_ = 18;

	m_nPreWidth = 0;
}

CW3DicomLoaderScrollbar::~CW3DicomLoaderScrollbar(void) {
}

void CW3DicomLoaderScrollbar::drawForeground(QPainter *painter, const QRectF &rect) {
	QColor border_color = QColor(ColorGeneral::kBorder);
	QColor bar_color = QColor(ColorSelectVolumeRangeBar::kBackground);
	QColor selected_bar_color = QColor(ColorSelectVolumeRangeBar::kSelectedBackground);
	QColor handle_color = QColor(ColorSelectVolumeRangeBar::kHandle);

	float bar_padding = 1.0f;

	float node_top = rect.y();

	float bar_x = rect.x() + (m_fNodeWidth * 0.5f);
	float bar_width = rect.width() - m_fNodeWidth;
	float bar_y = node_top + bar_padding;
	bar_rect_ = QRectF(bar_x, bar_y, bar_width, bar_height_);

	float node_bottom = bar_rect_.bottom() + bar_padding;
	float node_vertex = node_bottom + (m_fNodeWidth * 0.5f);

	painter->save();
	if (fabs(m_fLast - m_fFirst) <= 1) {
		painter->setPen(QPen(border_color, 1.0f, Qt::SolidLine));
		painter->setBrush(QBrush(selected_bar_color));
		painter->drawRect(bar_rect_);

		painter->setPen(QPen(border_color, 1.0f, Qt::SolidLine));
		painter->setBrush(QBrush(handle_color));

		QPointF poly[5];
		poly[0] = QPointF(rect.left(), node_bottom);
		poly[1] = QPointF(rect.left(), node_top);
		poly[2] = QPointF(rect.left() + m_fNodeWidth, node_top);
		poly[3] = QPointF(rect.left() + m_fNodeWidth, node_bottom);
		poly[4] = QPointF(rect.left() + (m_fNodeWidth * 0.5), node_vertex);
		//poly[5] = QPointF(-rt.width() / 2.0f, bar_height_ + 2.0f);

		painter->drawPolygon(poly, 5);

		poly[0] = QPointF(-m_fNodeWidth * 0.5f, node_bottom);
		poly[1] = QPointF(-m_fNodeWidth * 0.5f, node_top);
		poly[2] = QPointF(m_fNodeWidth * 0.5f, node_top);
		poly[3] = QPointF(m_fNodeWidth * 0.5f, node_bottom);
		poly[4] = QPointF(0, node_vertex);
		//poly[5] = QPointF(-m_fNodeWidth*0.5, bar_height_ + 2.0f);

		painter->drawPolygon(poly, 5);

		poly[0] = QPointF(rect.right() - 1 - m_fNodeWidth, node_bottom);
		poly[1] = QPointF(rect.right() - 1 - m_fNodeWidth, node_top);
		poly[2] = QPointF(rect.right() - 1, node_top);
		poly[3] = QPointF(rect.right() - 1, node_bottom);
		poly[4] = QPointF(rect.right() - 1 - m_fNodeWidth * 0.5, node_vertex);
		//poly[5] = QPointF(rt.width() / 2.0f - m_fNodeWidth, bar_height_ + 2.0f);

		painter->drawPolygon(poly, 5);
	} else {
		m_fPos[LEFT] = (m_fStart / (m_fLast - m_fFirst) * 2.0f - 1.0f) * (bar_width * 0.5f);
		m_fPos[CENTER] = (m_fMiddle / (m_fLast - m_fFirst) * 2.0f - 1.0f) * (bar_width * 0.5f);
		m_fPos[RIGHT] = (m_fEnd / (m_fLast - m_fFirst) * 2.0f - 1.0f) * (bar_width * 0.5f);

		painter->setPen(QPen(border_color, 1.0f, Qt::SolidLine));
		painter->setBrush(QBrush(bar_color));
		painter->drawRect(bar_rect_);

		QRectF SelectedRt(m_fPos[LEFT], bar_y, m_fPos[RIGHT] - m_fPos[LEFT], bar_height_);
		painter->setBrush(QBrush(selected_bar_color));
		painter->drawRect(SelectedRt);

		painter->setBrush(QBrush(handle_color));
		painter->setPen(QPen(border_color, 1.0f, Qt::SolidLine));
		//painter->drawRect(QRectF(m_fPos[LEFT] - m_fNodeWidth*1.5, -bar_height_ * 0.5f, m_fNodeWidth, bar_height_));

		QPointF poly[5];
		poly[0] = QPointF(m_fPos[LEFT] - m_fNodeWidth * 0.5, node_bottom);
		poly[1] = QPointF(m_fPos[LEFT] - m_fNodeWidth * 0.5, node_top);
		poly[2] = QPointF(m_fPos[LEFT] + m_fNodeWidth * 0.5, node_top);
		poly[3] = QPointF(m_fPos[LEFT] + m_fNodeWidth * 0.5, node_bottom);
		poly[4] = QPointF(m_fPos[LEFT], node_vertex);
		//poly[5] = QPointF(m_fPos[LEFT] - m_fNodeWidth*1.5, bar_height_ + 2.0f);

		left_node_rect_ = QRectF(poly[1].x(), poly[1].y(), m_fNodeWidth, node_vertex - node_top);

		painter->drawPolygon(poly, 5);

		if (fabs(m_fLast - m_fFirst) > 2) {
			poly[0] = QPointF(m_fPos[CENTER] - m_fNodeWidth * 0.5, node_bottom);
			poly[1] = QPointF(m_fPos[CENTER] - m_fNodeWidth * 0.5, node_top);
			poly[2] = QPointF(m_fPos[CENTER] + m_fNodeWidth * 0.5, node_top);
			poly[3] = QPointF(m_fPos[CENTER] + m_fNodeWidth * 0.5, node_bottom);
			poly[4] = QPointF(m_fPos[CENTER], node_vertex);
			//poly[5] = QPointF(m_fPos[CENTER] - m_fNodeWidth*0.5, bar_height_ + 2.0f);

			center_node_rect_ = QRectF(poly[1].x(), poly[1].y(), m_fNodeWidth, node_vertex - node_top);

			painter->drawPolygon(poly, 5);
			//painter->drawRect(QRectF(m_fPos[CENTER] - m_fNodeWidth*0.5, -bar_height_ * 0.5f, m_fNodeWidth, bar_height_));
		}

		poly[0] = QPointF(m_fPos[RIGHT] - m_fNodeWidth * 0.5, node_bottom);
		poly[1] = QPointF(m_fPos[RIGHT] - m_fNodeWidth * 0.5, node_top);
		poly[2] = QPointF(m_fPos[RIGHT] + m_fNodeWidth * 0.5, node_top);
		poly[3] = QPointF(m_fPos[RIGHT] + m_fNodeWidth * 0.5, node_bottom);
		poly[4] = QPointF(m_fPos[RIGHT], node_vertex);
		//poly[5] = QPointF(m_fPos[RIGHT] + m_fNodeWidth*0.5, bar_height_ + 2.0f);

		right_node_rect_ = QRectF(poly[1].x(), poly[1].y(), m_fNodeWidth, node_vertex - node_top);

		painter->drawPolygon(poly, 5);
		//painter->drawRect(QRectF(m_fPos[RIGHT] + m_fNodeWidth*0.5, -bar_height_ * 0.5f, m_fNodeWidth, bar_height_));

		QFont newFont = QApplication::font();
		newFont.setPixelSize(CW3Theme::getInstance()->fontsize_regular());
		newFont.setWeight(QFont::Bold);
		painter->setFont(newFont);
		painter->setPen(QPen(QColor(Qt::white), 1.0f, Qt::SolidLine));

		QString strStart = QString("%1").arg((int)m_fStart);
		QString strMiddle = QString("%1").arg((int)m_fMiddle);
		QString strEnd = QString("%1").arg((int)m_fEnd);

		QFontMetrics me(painter->font());

		int font_y = node_vertex + me.height() * 0.5f + 5;

		painter->drawText(m_fPos[LEFT] - m_fNodeWidth * 0.5f, font_y, strStart);
		painter->drawText(m_fPos[CENTER] - me.width(strMiddle) * 0.5f, font_y, strMiddle);
		painter->drawText(m_fPos[RIGHT] - ((me.width(strEnd) + m_fNodeWidth) * 0.5f), font_y, strEnd);
	}
	painter->restore();
}

void CW3DicomLoaderScrollbar::resizeEvent(QResizeEvent *event) {
	QGraphicsView::resizeEvent(event);

	int curWidth = this->width();
	if (m_nPreWidth > 0) {
		float scale = (float)curWidth / (float)m_nPreWidth;
		m_fPos[LEFT] *= scale;
		m_fPos[CENTER] *= scale;
		m_fPos[RIGHT] *= scale;
	}
	m_nPreWidth = curWidth;
}

void CW3DicomLoaderScrollbar::mousePressEvent(QMouseEvent *event) {
	if (fabs(m_fLast - m_fFirst) <= 1)	
		return;

	QPointF pos = mapToScene(event->pos());

	if (left_node_rect_.contains(pos))
		m_nSelectedCtrl = LEFT;
	else if (center_node_rect_.contains(pos) && fabs(m_fLast - m_fFirst) > 2)
		m_nSelectedCtrl = CENTER;
	else if (right_node_rect_.contains(pos))
		m_nSelectedCtrl = RIGHT;

	m_ptClickedPos = mapToScene(event->pos());
}

void CW3DicomLoaderScrollbar::mouseMoveEvent(QMouseEvent *event) {
	if (fabs(m_fLast - m_fFirst) <= 1)	return;

	if (event->buttons() == Qt::LeftButton && m_nSelectedCtrl != -1) {
		QPointF pos = mapToScene(event->pos());
		float dTotalCount = m_fLast - m_fFirst;

		switch (m_nSelectedCtrl) {
		case LEFT:
			m_fPos[LEFT] = pos.x();

			if (m_fPos[LEFT] < bar_rect_.left())
				m_fPos[LEFT] = bar_rect_.left();
			else if (m_fPos[LEFT] > m_fPos[CENTER] - 1.0f)
				m_fPos[LEFT] = m_fPos[CENTER] - 1.0f;

			m_fPos[CENTER] = m_fPos[LEFT] + (m_fPos[RIGHT] - m_fPos[LEFT]) * 0.5f;

			m_fMiddle = (m_fPos[CENTER] + bar_rect_.width() * 0.5f) / bar_rect_.width() * dTotalCount;
			if ((int)m_fMiddle <= (int)m_fStart || (m_fMiddle - m_fStart) < 1.0f)
				m_fMiddle = (int)m_fStart + 1;
			else if ((int)m_fMiddle >= (int)m_fEnd || (m_fEnd - m_fMiddle) < 1.0f)
				m_fMiddle = (int)m_fEnd - 1;

			emit sigScrollTranslated(CENTER, m_fMiddle);

			m_fStart = (m_fPos[LEFT] + bar_rect_.width() * 0.5f) / bar_rect_.width() * dTotalCount;
			if ((int)m_fStart >= (int)m_fMiddle || (m_fMiddle - m_fStart) < 1.0f)
				m_fStart = (int)m_fMiddle - 1;

			emit sigScrollTranslated(LEFT, m_fStart);

			break;

		case CENTER:
			m_fPos[CENTER] = pos.x();

			if (m_fPos[CENTER] < m_fPos[LEFT] + 1.0f)
				m_fPos[CENTER] = m_fPos[LEFT] + 1.0f;
			else if (m_fPos[CENTER] > m_fPos[RIGHT] - 1.0f)
				m_fPos[CENTER] = m_fPos[RIGHT] - 1.0f;

			m_fMiddle = (m_fPos[CENTER] + bar_rect_.width() * 0.5f) / bar_rect_.width() * dTotalCount;
			if ((int)m_fMiddle <= (int)m_fStart || (m_fMiddle - m_fStart) < 1.0f)
				m_fMiddle = (int)m_fStart + 1;
			else if ((int)m_fMiddle >= (int)m_fEnd || (m_fEnd - m_fMiddle) < 1.0f)
				m_fMiddle = (int)m_fEnd - 1;

			emit sigScrollTranslated(CENTER, m_fMiddle);

			break;

		case RIGHT:
			m_fPos[RIGHT] = pos.x();

			if (m_fPos[RIGHT] > bar_rect_.right())
				m_fPos[RIGHT] = bar_rect_.right();
			else if (m_fPos[RIGHT] < m_fPos[CENTER] + 1.0f)
				m_fPos[RIGHT] = m_fPos[CENTER] + 1.0f;

			m_fPos[CENTER] = m_fPos[LEFT] + (m_fPos[RIGHT] - m_fPos[LEFT]) * 0.5f;

			m_fMiddle = (m_fPos[CENTER] + bar_rect_.width() * 0.5f) / bar_rect_.width() * dTotalCount;
			if ((int)m_fMiddle <= (int)m_fStart || (m_fMiddle - m_fStart) < 1.0f)
				m_fMiddle = (int)m_fStart + 1;
			else if ((int)m_fMiddle >= (int)m_fEnd || (m_fEnd - m_fMiddle) < 1.0f)
				m_fMiddle = (int)m_fEnd - 1;

			emit sigScrollTranslated(CENTER, m_fMiddle);

			m_fEnd = (m_fPos[RIGHT] + bar_rect_.width() * 0.5f) / bar_rect_.width() * dTotalCount;
			if ((int)m_fEnd <= (int)m_fMiddle || (m_fEnd - m_fMiddle) < 1.0f)
				m_fEnd = (int)m_fMiddle + 1;

			emit sigScrollTranslated(RIGHT, m_fEnd);

			break;
		}
		this->update();

		m_ptClickedPos = mapToScene(event->pos());
	}
}

void CW3DicomLoaderScrollbar::mouseReleaseEvent(QMouseEvent *event) {
	if (fabs(m_fLast - m_fFirst) <= 1)	
		return;

	m_nSelectedCtrl = -1;
}

void CW3DicomLoaderScrollbar::mouseDoubleClickEvent(QMouseEvent *event) {

}

void CW3DicomLoaderScrollbar::Reset() {
	if (fabs(m_fLast - m_fFirst) <= 1)
		return;

	m_fStart = m_fFirst;
	m_fEnd = m_fLast;

	QRectF rt = rect();
	//m_fPos[LEFT] = m_fNodeWidth * 1.5f - rt.width() * 0.5f;
	//m_fPos[CENTER] = -(m_fNodeWidth * 0.5f);
	//m_fPos[RIGHT] = rt.width() * 0.5f - m_fNodeWidth * 1.5f;
	m_fStart = m_fFirst;
	m_fMiddle = (m_fLast - m_fFirst) * 0.5f + m_fFirst;
	m_fEnd = m_fLast;

	this->update();
}

void CW3DicomLoaderScrollbar::setRange(float first, float last) {
	m_fFirst = m_fStart = first;
	m_fMiddle = (last - first) * 0.5f + first;
	m_fLast = m_fEnd = last;

	if (last - first <= 1.0f) {
		this->update();
		return;
	}

	//QRectF rt = rect();
	//m_fPos[LEFT] = m_fNodeWidth * 1.5f - rt.width() * 0.5f;
	//m_fPos[RIGHT] = rt.width() * 0.5f - m_fNodeWidth * 1.5f;
	////m_fPos[CENTER] = -(m_fNodeWidth * 0.5f);
	//m_fPos[CENTER] = m_fPos[LEFT] + (m_fPos[RIGHT] - m_fPos[LEFT]) * 0.5f;

	this->update();

	emit sigScrollTranslated(LEFT, m_fStart);
	emit sigScrollTranslated(CENTER, m_fMiddle);
	emit sigScrollTranslated(RIGHT, m_fEnd);
}

void CW3DicomLoaderScrollbar::setValidRange(float first, float last, float start, float end) {
	m_fFirst = first;
	m_fLast = last;

	m_fStart = start;
	m_fMiddle = (end - start) * 0.5f + start;
	m_fEnd = end;

	if (end - start <= 1.0f) {
		this->update();
		return;
	}

	this->update();

	emit sigScrollTranslated(LEFT, m_fStart);
	emit sigScrollTranslated(CENTER, m_fMiddle);
	emit sigScrollTranslated(RIGHT, m_fEnd);
}

void CW3DicomLoaderScrollbar::setStart(float dIndex) {
	if (fabs(m_fLast - m_fFirst) <= 1)
		return;

	if (dIndex > m_fMiddle || dIndex < 0)
		return;
	m_fStart = dIndex;

	QRectF rt = rect();
	float dTotalCount = m_fLast - m_fFirst;
	m_fPos[LEFT] = m_fStart * (rt.width() - m_fNodeWidth * 3.0f) / dTotalCount - (rt.width() * 0.5f - m_fNodeWidth * 1.5f);

	this->update();
	emit sigScrollTranslated(LEFT, m_fStart);
}

void CW3DicomLoaderScrollbar::setMiddle(float dIndex) {
	if (fabs(m_fLast - m_fFirst) <= 1)
		return;

	if (dIndex > m_fEnd || dIndex < m_fStart)
		return;
	m_fMiddle = dIndex;

	QRectF rt = rect();
	float dTotalCount = m_fLast - m_fFirst;
	m_fPos[CENTER] = m_fMiddle * (rt.width() - m_fNodeWidth * 3.0f) / dTotalCount - (rt.width() * 0.5f - m_fNodeWidth * 1.5f);

	this->update();
	emit sigScrollTranslated(CENTER, m_fMiddle);
}

void CW3DicomLoaderScrollbar::setEnd(float dIndex) {
	if (fabs(m_fLast - m_fFirst) <= 1)
		return;

	if (dIndex > m_fLast || dIndex < m_fMiddle)
		return;
	m_fEnd = dIndex;

	QRectF rt = rect();
	float dTotalCount = m_fLast - m_fFirst;
	m_fPos[RIGHT] = m_fEnd * (rt.width() - m_fNodeWidth * 3.0f) / dTotalCount - (rt.width() * 0.5f - m_fNodeWidth * 1.5f);

	this->update();
	emit sigScrollTranslated(RIGHT, m_fEnd);
}
