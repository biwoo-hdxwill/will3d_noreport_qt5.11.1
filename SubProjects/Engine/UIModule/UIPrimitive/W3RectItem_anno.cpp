#include "W3RectItem_anno.h"

#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include "../../Common/Common/define_ui.h"

CW3RectItem_anno::CW3RectItem_anno(const QPointF point) {
	m_pen = QPen(QColor(128, 255, 0), common::ui_define::kToolLineWidth, Qt::SolidLine);
	m_hoverPen = QPen(QColor(128, 255, 0), common::ui_define::kToolLineWidth + 1, Qt::SolidLine);
	m_pen.setCosmetic(true);
	m_hoverPen.setCosmetic(true);

	m_brush = QBrush(Qt::transparent);

	this->setFlag(CW3RectItem_anno::ItemIsSelectable, true);
	this->setFlag(CW3RectItem_anno::ItemSendsGeometryChanges, true);
	this->setFlag(CW3RectItem_anno::ItemIsMovable, true);
	this->setPen(m_pen);
	this->setBrush(m_brush);
	//this->setZValue(5);
	this->setAcceptHoverEvents(true);

	m_bHighlight = false;
}

CW3RectItem_anno::~CW3RectItem_anno() {
}

void CW3RectItem_anno::setHighlightEffect(bool bFlag) {
	m_bHighlight = bFlag;
	if (bFlag) {
		this->setPen(m_hoverPen);
	} else {
		this->setPen(m_pen);
	}
}

void CW3RectItem_anno::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsRectItem::paint(pPainter, pOption, pWidget);
}

void CW3RectItem_anno::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsRectItem::hoverEnterEvent(event);
	setHighlightEffect(true);
	emit sigHoverRect(true);
}

void CW3RectItem_anno::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
	QGraphicsRectItem::hoverLeaveEvent(event);
	setHighlightEffect(false);
	emit sigHoverRect(false);
}

void CW3RectItem_anno::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();
	if (event->buttons() == Qt::LeftButton)
		emit sigTranslateRect(trans);
}

void CW3RectItem_anno::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mousePressEvent(event);

	this->setSelected(true);
}

void CW3RectItem_anno::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
	QGraphicsRectItem::mouseReleaseEvent(event);

	this->setSelected(false);
}
void CW3RectItem_anno::setPenColor(QColor color) {
	m_pen.setColor(color);
	m_hoverPen.setColor(color);
	this->setPen(m_pen);
}

void CW3RectItem_anno::setBrushColor(QColor color) {
	m_brush.setColor(color);
	this->setBrush(m_brush);
}
