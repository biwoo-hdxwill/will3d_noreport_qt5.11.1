#include "W3EllipseItem_anno.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

CW3EllipseItem_anno::CW3EllipseItem_anno(const QPointF point) {
	m_pen = QPen(QColor(128, 255, 0), 2, Qt::SolidLine);
	m_hoverPen = QPen(QColor(128, 255, 0), 3, Qt::SolidLine);
	m_pen.setCosmetic(true);
	m_hoverPen.setCosmetic(true);

	m_brush = QBrush(Qt::transparent);

	this->setFlag(CW3EllipseItem_anno::ItemIsSelectable, true);
	this->setFlag(CW3EllipseItem_anno::ItemSendsGeometryChanges, true);
	this->setFlag(CW3EllipseItem_anno::ItemIsMovable, true);
	this->setPen(m_pen);
	this->setBrush(m_brush);
	this->setZValue(52.0f);
	this->setAcceptHoverEvents(true);

	m_bHighlight = false;
}

CW3EllipseItem_anno::~CW3EllipseItem_anno() {
}

void CW3EllipseItem_anno::setHighlightEffect(bool bFlag) {
	m_bHighlight = bFlag;

	if (bFlag) {
		this->setPen(m_hoverPen);
	} else {
		this->setPen(m_pen);
	}
}

void CW3EllipseItem_anno::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsEllipseItem::paint(pPainter, pOption, pWidget);
}

void CW3EllipseItem_anno::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverEnterEvent(event);

	if (!m_bHighlight)
		setHighlightEffect(true);

	emit sigOnHover(true);
}

void CW3EllipseItem_anno::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsEllipseItem::hoverLeaveEvent(event);

	if (m_bHighlight)
		setHighlightEffect(false);

	emit sigOnHover(false);
}

void CW3EllipseItem_anno::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsEllipseItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();
	if (event->buttons() == Qt::LeftButton)
		emit sigTranslateCircle(trans);
}

void CW3EllipseItem_anno::setPenColor(QColor color) {
	m_pen.setColor(color);
	m_hoverPen.setColor(color);
	this->setPen(m_pen);
}

void CW3EllipseItem_anno::setBrushColor(QColor color) {
	m_brush.setColor(color);
	this->setBrush(m_brush);
}
