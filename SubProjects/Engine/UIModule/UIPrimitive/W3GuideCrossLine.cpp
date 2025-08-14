#include "W3GuideCrossLine.h"

#include "../../Common/Common/W3Memory.h"

#include <QGraphicsScene>
#include <QGraphicsLineItem>

CW3GuideCrossLine::CW3GuideCrossLine(QGraphicsScene* pScene) : m_pgScene(pScene)
{
	QPen pen(Qt::yellow);
	pen.setWidthF(1.0f);
	pen.setCosmetic(true);

	m_pHorizontal = pScene->addLine(pScene->sceneRect().x(), 0.0f, pScene->sceneRect().x() + pScene->sceneRect().width(), 0.0f, pen);
	m_pHorizontal->setVisible(false);

	m_pVertical = pScene->addLine(0.0f, pScene->sceneRect().y(), 0.0f, pScene->sceneRect().y() + pScene->sceneRect().height(), pen);
	m_pVertical->setVisible(false);
}

CW3GuideCrossLine::~CW3GuideCrossLine()
{
	m_pgScene->removeItem(m_pHorizontal);
	SAFE_DELETE_OBJECT(m_pHorizontal);

	m_pgScene->removeItem(m_pVertical);
	SAFE_DELETE_OBJECT(m_pVertical);
}

void CW3GuideCrossLine::setPos(QPointF pos)
{
	m_pHorizontal->setY(pos.y());
	m_pVertical->setX(pos.x());
}

void CW3GuideCrossLine::setVisible(bool visible)
{
	m_pHorizontal->setVisible(visible);
	m_pVertical->setVisible(visible);
}
