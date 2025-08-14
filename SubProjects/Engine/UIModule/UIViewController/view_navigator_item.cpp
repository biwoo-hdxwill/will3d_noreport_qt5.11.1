#include "view_navigator_item.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include "../../Module/Will3DEngine/renderer_manager.h"

ViewNavigatorItem::ViewNavigatorItem(QObject* parent) 
	: QObject(parent) {
	//img_.reset(new QImage(":/image/tracing/guide/NoImage.png"));
}

ViewNavigatorItem::~ViewNavigatorItem() {
}

void ViewNavigatorItem::SetWorldAxisDirection(const glm::mat4& rot_mat, const glm::mat4& view_mat) {

	auto& renderer = RendererManager::GetInstance().renderer_navigator();
	
	renderer.SetSize(size_.width(), size_.height());
	renderer.SetWorldAxisDirection(rot_mat, view_mat);

	renderer.Render();

	img_ = renderer.GrabFrameBufferQPixmap(size_.width(), size_.height());
}

void ViewNavigatorItem::SetSize(int width, int height) {
	size_ = QSize(width, height);

	rect_ = QRectF(-(double)width / 2.0, -(double)height/ 2.0, 
		(double)width, (double)height);
}

void ViewNavigatorItem::paint(QPainter * pPainter, const QStyleOptionGraphicsItem * pOption,
							  QWidget * pWidget) {
	pPainter->setBackgroundMode(Qt::TransparentMode);
	pPainter->fillRect(rect_, QColor(0,0,0,0));
	pPainter->drawPixmap(rect_, img_, QRectF(0, 0, size_.width(), size_.height()));
}

void ViewNavigatorItem::hoverEnterEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsItem::hoverEnterEvent(event);
}

void ViewNavigatorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent * event) {
	QGraphicsItem::hoverLeaveEvent(event);
}

void ViewNavigatorItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event) {
	QGraphicsItem::mouseMoveEvent(event);
}

const glm::mat4& ViewNavigatorItem::GetRotateMatrix()
{
	return RendererManager::GetInstance().renderer_navigator().rotate();
}
