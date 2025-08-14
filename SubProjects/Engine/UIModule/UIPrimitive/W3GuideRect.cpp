#include "W3GuideRect.h"

#include <QPen>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Memory.h"

CW3GuideRect::CW3GuideRect() {
	this->setAcceptedMouseButtons(Qt::LeftButton);
	this->setAcceptHoverEvents(true);

	QColor color(ColorGeneral::kAccent);
	QPen pen(color);
	pen.setCosmetic(true);

	QPen penCross = pen;
	penCross.setWidthF(1.0f);
	penCross.setStyle(Qt::SolidLine);

	QPen penEdge = pen;
	penEdge.setWidthF(2.0f);
	penEdge.setStyle(Qt::SolidLine);

	for (int i = 0; i < 2; i++) {
		m_lpCenterCross[i] = new QGraphicsLineItem(this);
		m_lpCenterCross[i]->setVisible(true);
		m_lpCenterCross[i]->setPen(penCross);
		//m_lpCenterCross[i]->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);

		m_lpTopLeftEdge[i] = new QGraphicsLineItem(this);
		m_lpTopLeftEdge[i]->setVisible(true);
		m_lpTopLeftEdge[i]->setPen(penEdge);
		//m_lpTopLeftEdge[i]->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);

		m_lpTopRightEdge[i] = new QGraphicsLineItem(this);
		m_lpTopRightEdge[i]->setVisible(true);
		m_lpTopRightEdge[i]->setPen(penEdge);
		//m_lpTopRightEdge[i]->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);

		m_lpBottomLeftEdge[i] = new QGraphicsLineItem(this);
		m_lpBottomLeftEdge[i]->setVisible(true);
		m_lpBottomLeftEdge[i]->setPen(penEdge);
		//m_lpBottomLeftEdge[i]->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);

		m_lpBottomRightEdge[i] = new QGraphicsLineItem(this);
		m_lpBottomRightEdge[i]->setVisible(true);
		m_lpBottomRightEdge[i]->setPen(penEdge);
		//m_lpBottomRightEdge[i]->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);
	}

	QPen penDash = pen;
	penDash.setWidthF(1.0f);
	penDash.setStyle(Qt::DashLine);

	m_pRect = new QGraphicsRectItem(this);
	m_pRect->setVisible(true);
	m_pRect->setPen(penDash);
	//m_pRect->setFlag(QGraphicsLineItem::ItemIgnoresTransformations);

	m_nSolidLineLength = 0;

	//m_eSelectedElement = EGUIDERECT_ELEMENT::NONE;
}

CW3GuideRect::~CW3GuideRect() {
	for (int i = 0; i < 2; i++) {
		SAFE_DELETE_OBJECT(m_lpCenterCross[i]);
		SAFE_DELETE_OBJECT(m_lpTopLeftEdge[i]);
		SAFE_DELETE_OBJECT(m_lpTopRightEdge[i]);
		SAFE_DELETE_OBJECT(m_lpBottomLeftEdge[i]);
		SAFE_DELETE_OBJECT(m_lpBottomRightEdge[i]);
	}

	SAFE_DELETE_OBJECT(m_pRect);
}

void CW3GuideRect::setRect(const QRectF rect, const bool change_dimension) {
	QPointF center(rect.x() + (rect.width() * 0.5f), rect.y() + (0.5f * rect.height()));
	/*float logestAxisLength = rect.width();
	if (rect.width() < rect.height())
		logestAxisLength = rect.height();
	int centerLineLength = logestAxisLength * 0.025f;*/

	if (change_dimension)
		m_nSolidLineLength = rect.width() * 0.025f;

	m_lpCenterCross[0]->setLine(center.x() - m_nSolidLineLength, center.y(), center.x() + m_nSolidLineLength, center.y());
	m_lpCenterCross[1]->setLine(center.x(), center.y() - m_nSolidLineLength, center.x(), center.y() + m_nSolidLineLength);

	m_lpTopLeftEdge[0]->setLine(rect.x(), rect.y(), rect.x() + m_nSolidLineLength, rect.y());
	m_lpTopLeftEdge[1]->setLine(rect.x(), rect.y(), rect.x(), rect.y() + m_nSolidLineLength);

	m_lpTopRightEdge[0]->setLine(rect.x() + rect.width() - m_nSolidLineLength, rect.y(), rect.x() + rect.width(), rect.y());
	m_lpTopRightEdge[1]->setLine(rect.x() + rect.width(), rect.y(), rect.x() + rect.width(), rect.y() + m_nSolidLineLength);

	m_lpBottomLeftEdge[0]->setLine(rect.x(), rect.y() + rect.height(), rect.x() + m_nSolidLineLength, rect.y() + rect.height());
	m_lpBottomLeftEdge[1]->setLine(rect.x(), rect.y() + rect.height() - m_nSolidLineLength, rect.x(), rect.y() + rect.height());

	m_lpBottomRightEdge[0]->setLine(rect.x() + rect.width() - m_nSolidLineLength, rect.y() + rect.height(), rect.x() + rect.width(), rect.y() + rect.height());
	m_lpBottomRightEdge[1]->setLine(rect.x() + rect.width(), rect.y() + rect.height() - m_nSolidLineLength, rect.x() + rect.width(), rect.y() + rect.height());

	m_pRect->setRect(rect);

	m_rect = rect;
}

//void CW3GuideRect::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
//{
//	QGraphicsItem::mouseMoveEvent(event);
//
//	QPointF ptCur = event->pos();
//	QPointF ptScene = event->scenePos();
//	m_eSelectedElement = getElement(ptCur);
//}
//void CW3GuideRect::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
//{
//	QGraphicsItem::hoverMoveEvent(event);
//
//	QPointF ptCur = event->pos();
//}

EGUIDERECT_ELEMENT CW3GuideRect::getElement(const QPointF &point) {
	EGUIDERECT_ELEMENT element = NONE;

	float longFactor = 0.8f;
	float shortFactor = 0.2f;

	float width = 0.0f;
	float height = 0.0f;

	width = m_rect.width() * shortFactor;
	height = m_rect.height() * shortFactor;
	QRectF topLeft(m_rect.x() - (width * 0.5f), m_rect.y() - (height * 0.5f), width, height);
	QRectF topRight((m_rect.x() + m_rect.width()) - (width * 0.5f), m_rect.y() - (height * 0.5f), width, height);

	QRectF bottomLeft(m_rect.x() - (width * 0.5f), (m_rect.y() + m_rect.height()) - (height * 0.5f), width, height);
	QRectF bottomRight((m_rect.x() + m_rect.width()) - (width * 0.5f), (m_rect.y() + m_rect.height()) - (height * 0.5f), width, height);

	width = m_rect.width() * shortFactor;
	height = m_rect.height() * longFactor;
	QRectF left(m_rect.x() - (width * 0.5f), (m_rect.y() + (m_rect.height() * 0.5f)) - (height * 0.5f), width, height);
	QRectF right((m_rect.x() + m_rect.width()) - (width * 0.5f), (m_rect.y() + (m_rect.height() * 0.5f)) - (height * 0.5f), width, height);

	width = m_rect.width() * longFactor;
	height = m_rect.height() * shortFactor;
	QRectF top((m_rect.x() + (m_rect.width() * 0.5f)) - (width * 0.5f), m_rect.y() - (height * 0.5f), width, height);
	QRectF bottom((m_rect.x() + (m_rect.width() * 0.5f)) - (width * 0.5f), (m_rect.y() + m_rect.height()) - (height * 0.5f), width, height);

	width = m_rect.width() * shortFactor;
	height = m_rect.height() * shortFactor;
	QRectF center((m_rect.x() + (m_rect.width() * 0.5f)) - (width * 0.5f), (m_rect.y() + (m_rect.height() * 0.5f)) - (height * 0.5f), width, height);

	if (topLeft.contains(point))
		element = EGUIDERECT_ELEMENT::TOP_LEFT;
	else if (topRight.contains(point))
		element = EGUIDERECT_ELEMENT::TOP_RIGHT;
	else if (bottomLeft.contains(point))
		element = EGUIDERECT_ELEMENT::BOTTOM_LEFT;
	else if (bottomRight.contains(point))
		element = EGUIDERECT_ELEMENT::BOTTOM_RIGHT;
	else if (left.contains(point))
		element = EGUIDERECT_ELEMENT::LEFT;
	else if (right.contains(point))
		element = EGUIDERECT_ELEMENT::RIGHT;
	else if (top.contains(point))
		element = EGUIDERECT_ELEMENT::TOP;
	else if (bottom.contains(point))
		element = EGUIDERECT_ELEMENT::BOTTOM;
	else if (center.contains(point))
		element = EGUIDERECT_ELEMENT::CENTER;

	return element;
}
