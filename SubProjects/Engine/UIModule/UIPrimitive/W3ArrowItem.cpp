#include "W3ArrowItem.h"

#include <QGraphicsScene>
#include <qmath.h>

#include "../../Common/Common/W3Memory.h"

ArrowItem::ArrowItem(QGraphicsScene* pScene, QGraphicsItem* parent) : m_pgScene(pScene)

{
	QPen pen(Qt::yellow);
	pen.setWidthF(4.0f);
	QBrush brush(Qt::yellow);

	m_pLine = pScene->addLine(0.0f, 0.0f, 0.0f, 0.0f, pen);
	m_pLine->setVisible(false);
	m_pLine->setZValue(1.0f);

	for (int i = 0; i < 2; i++)
	{
		head[i] = pScene->addPolygon(QPolygonF(), pen, brush);
		head[i]->setVisible(false);
		head[i]->setZValue(2.0f);
	}
}

ArrowItem::~ArrowItem()
{
	m_pgScene->removeItem(m_pLine);
	SAFE_DELETE_OBJECT(m_pLine);
	
	for (int i = 0; i < 2; i++)
	{
		m_pgScene->removeItem(head[i]);
		SAFE_DELETE_OBJECT(head[i]);
	}
}

void ArrowItem::setArrowHead()
{
	double angle = ::acos(m_pLine->line().dx() / m_pLine->line().length());
	if (m_pLine->line().dy() >= 0)
		angle = (M_PI * 2) - angle;

	float arrowSize = 20.0f;
	QPointF arrowP1 = m_pLine->line().p1() + QPointF(sin(angle + M_PI / 3) * arrowSize,
		cos(angle + M_PI / 3) * arrowSize);
	QPointF arrowP2 = m_pLine->line().p1() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
		cos(angle + M_PI - M_PI / 3) * arrowSize);

	QPolygonF arrowHead;
	arrowHead << m_pLine->line().p1() << arrowP1 << arrowP2;
	head[0]->setPolygon(arrowHead);

	arrowP1 = m_pLine->line().p2() + QPointF(sin(angle - M_PI / 3) * arrowSize,
		cos(angle - M_PI / 3) * arrowSize);
	arrowP2 = m_pLine->line().p2() + QPointF(sin(angle - M_PI + M_PI / 3) * arrowSize,
		cos(angle - M_PI + M_PI / 3) * arrowSize);

	arrowHead.clear();
	arrowHead << m_pLine->line().p2() << arrowP1 << arrowP2;
	head[1]->setPolygon(arrowHead);
}

void ArrowItem::setPos(QPointF pos)
{

}

void ArrowItem::setStart(QPointF pos)
{
	QLineF line = m_pLine->line();
	line.setP1(pos);
	m_pLine->setLine(line);

	setArrowHead();
}

void ArrowItem::setEnd(QPointF pos)
{
	QLineF line = m_pLine->line();
	line.setP2(pos);
	m_pLine->setLine(line);

	setArrowHead();
}

void ArrowItem::setVisible(bool visible)
{
	m_pLine->setVisible(visible);
	for (int i = 0; i < 2; i++)
		head[i]->setVisible(visible);
}

void ArrowItem::addItemAll()	// by jdk 160722
{
	if (m_pLine)
		m_pgScene->addItem(m_pLine);

	if (head[0])
		m_pgScene->addItem(head[0]);

	if (head[1])
		m_pgScene->addItem(head[1]);
}

void ArrowItem::removeItemAll()	// by jdk 160722
{
	if (m_pLine)
		m_pgScene->removeItem(m_pLine);

	if (head[0])
		m_pgScene->removeItem(head[0]);

	if (head[1])
		m_pgScene->removeItem(head[1]);
}
