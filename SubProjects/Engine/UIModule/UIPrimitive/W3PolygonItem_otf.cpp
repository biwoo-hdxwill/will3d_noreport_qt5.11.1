#include "W3PolygonItem_otf.h"
#include <qpen.h>
CW3PolygonItem_otf::CW3PolygonItem_otf(void)
{
	// set UI color components.
	QPen temppen(Qt::white);	
	temppen.setCosmetic(true);
	QBrush brush(Qt::white);
	QColor color(Qt::white);	
	color.setAlphaF(0.0);
	brush.setColor(color);

	this->setAcceptHoverEvents(true);
	this->setZValue(1.0f);
	this->setBrush(brush);

	// set polygon color.
	QPen pen = this->pen();
	pen.setColor(Qt::blue);
	pen.setCosmetic(true);
	this->setPen(pen);

	// by jdk 150617
	m_bIsSelected = false;
	// jdk end

	QObject::installEventFilter(this);
}

CW3PolygonItem_otf::~CW3PolygonItem_otf(void)
{
}

void CW3PolygonItem_otf::activate(void)
{
	QBrush brush = this->brush();
	QColor color(Qt::blue);
	color.setAlphaF(0.1);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::white);
	pen.setCosmetic(true);
	this->setPen(pen);
}

void CW3PolygonItem_otf::select(void)
{
	QBrush brush = this->brush();
	QColor color(Qt::blue);
	color.setAlphaF(0.3);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::white);
	pen.setCosmetic(true);
	this->setPen(pen);

	this->m_bIsSelected = true;
}

void CW3PolygonItem_otf::deactivate(void)
{
	// by jdk 150617
	//if(this->m_bIsSelected == true)
		//return;
	// jdk end

	QBrush brush = this->brush();
	QColor color(Qt::white);
	color.setAlphaF(0.1);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::blue);
	pen.setWidth(1);
	pen.setCosmetic(true);
	this->setPen(pen);

	//this->m_bIsSelected = false;

	// by jdk 150617
	//this->m_bIsSelected = false;
	// jdk end
}

//void CW3PolygonItem_otf::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
//{
//	QGraphicsPolygonItem::hoverEnterEvent(event);
//	QBrush brush = this->brush();
//	QColor color(Qt::green);
//	color.setAlphaF(0.3);
//	brush.setColor(color);
//	this->setBrush(brush);
//	QPen pen = this->pen();
//	pen.setColor(Qt::yellow);
//	this->setPen(pen);
//
//	std::cout<<"[HOVER]"<<event->lastScenePos().x()<<", "<<event->lastScenePos().y()<<std::endl;
//}
//
//bool CW3PolygonItem_otf::eventFilter(QObject *watched, QEvent *event)
//{
//	std::cout<<"EVENT FILTER() call !"<<std::endl;
//	if(event->type() == QEvent::GraphicsSceneMouseMove)
//	{
//		
//	QBrush brush = this->brush();
//	QColor color(Qt::green);
//	color.setAlphaF(0.3);
//	brush.setColor(color);
//	this->setBrush(brush);
//	QPen pen = this->pen();
//	pen.setColor(Qt::yellow);
//	this->setPen(pen);
//	}
//
//	return true;
//}

void CW3PolygonItem_otf::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	/*
	QGraphicsPolygonItem::hoverLeaveEvent(event);
	QBrush brush = this->brush();
	QColor color(Qt::red);
	color.setAlphaF(0.1);
	brush.setColor(color);
	this->setBrush(brush);

	QPen pen = this->pen();
	pen.setColor(Qt::red);
	pen.setWidth(1);
	this->setPen(pen);
	*/
	deactivate();
}

//void CW3PolygonItem_otf::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//	QGraphicsPolygonItem::mousePressEvent(event);
//	emit sigPressed();
//}
