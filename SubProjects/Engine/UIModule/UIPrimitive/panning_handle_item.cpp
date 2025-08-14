#include "pannig_handle_item.h"

#include "../../Common/Common/W3Cursor.h"

#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QApplication>

PanningHandleItem::PanningHandleItem(QGraphicsItem* parent) : QGraphicsPixmapItem(parent) {
	this->setFlag(QGraphicsPixmapItem::ItemIsSelectable, true);
	this->setFlag(QGraphicsPixmapItem::ItemIsMovable, true);
	this->setPixmap(QPixmap(":/cursor/size_all.png"));
	setAcceptHoverEvents(true);
	setZValue(0);
}

PanningHandleItem::~PanningHandleItem() {

}
void PanningHandleItem::paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) {
	QStyleOptionGraphicsItem* style = const_cast<QStyleOptionGraphicsItem*>(pOption);

	// Remove the HasFocus style state to prevent the dotted line from being drawn.
	style->state &= ~QStyle::State_HasFocus;
	style->state &= ~QStyle::State_Selected;

	QGraphicsPixmapItem::paint(pPainter, pOption, pWidget);
}
void PanningHandleItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsPixmapItem::hoverEnterEvent(event);

	pixmap_prev_cursor = QApplication::overrideCursor()->pixmap();
	QApplication::setOverrideCursor(CW3Cursor::ClosedHandCursor());
}

void PanningHandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsPixmapItem::hoverLeaveEvent(event);

	QApplication::setOverrideCursor(QCursor(pixmap_prev_cursor));
}

void PanningHandleItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsPixmapItem::mouseMoveEvent(event);

	QPointF trans = event->scenePos() - event->lastScenePos();

	if (event->buttons() == Qt::LeftButton) {
		emit sigTranslate(trans);
	}
}

void PanningHandleItem::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsPixmapItem::mousePressEvent(event);
}

void PanningHandleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsPixmapItem::mouseReleaseEvent(event);

	
	emit sigMouseReleased();
}

QPainterPath PanningHandleItem::shape() const {
	QPainterPath path;
	path.addRect(QRectF(0,0,50,50));

	return path;
}
