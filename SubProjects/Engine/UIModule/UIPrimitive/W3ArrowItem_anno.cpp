#include "W3ArrowItem_anno.h"

#include <QGraphicsScene>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"

#include "W3EllipseItem.h"
#include "W3PathItem.h"

CW3ArrowItem_anno::CW3ArrowItem_anno(QGraphicsScene* pScene)
	: m_pgScene(pScene) {}

CW3ArrowItem_anno::~CW3ArrowItem_anno() {
	for (auto &i : m_listpNode)
		SAFE_DELETE_OBJECT(i);

	SAFE_DELETE_OBJECT(m_pLine)
}

void CW3ArrowItem_anno::addPoint(const QPointF& pointf) {
	if (m_listNodePt.size() == 0) {
		m_bStartEdit = true;
		m_pLine = new CW3PathItem();
		m_pLine->setMovable(true);
		m_pgScene->addItem(m_pLine);

		ApplyLineColor();
	}

	if (m_bStartEdit) {
		CW3EllipseItem *pEllipse = new CW3EllipseItem(pointf);
		pEllipse->setPen(QPen(Qt::green, 2, Qt::SolidLine));
		pEllipse->setBrush(Qt::red);
		m_pgScene->addItem(pEllipse);
		m_listpNode.push_back(pEllipse);
		m_listNodePt.push_back(pointf);

		//drawingPath(*m_pLine, m_listNodePt);
		ApplyNodeColor();
		updatePath();
	}
}

void CW3ArrowItem_anno::drawingCurPath(const QPointF& pointf) {
	if (m_bStartEdit) {
		m_listNodePt.push_back(pointf);
		updatePath();
		m_listNodePt.pop_back();
	}
}

void CW3ArrowItem_anno::clear() {
	for (auto &i : m_listpNode) {
		m_pgScene->removeItem(i);
		SAFE_DELETE_OBJECT(i);
	}

	m_listpNode.clear();
	m_listNodePt.clear();

	if (m_pLine) {
		m_pgScene->removeItem(m_pLine);
		SAFE_DELETE_OBJECT(m_pLine);
	}

	m_bStartEdit = false;
}

void CW3ArrowItem_anno::setVisible(bool bFlag) {
	if (m_pLine)
		m_pLine->setVisible(bFlag);

	for (auto &i : m_listpNode)
		i->setVisible(bFlag);
}

bool CW3ArrowItem_anno::endEdit() {
	m_bStartEdit = false;

	if (m_pLine != nullptr)
		m_pLine->setHighlighted(true);

	for (auto &i : m_listpNode) {
		i->SetFlagHighlight(true);
		i->SetFlagMovable(true);
	}

	if (m_listNodePt.size() > 1) {
		graphicItemsConnection();
		return true;
	} else {
		return false;
	}
}

bool CW3ArrowItem_anno::isNodeSelected() {
	for (int i = 0; i < m_listpNode.size(); i++) {
		if (m_listpNode[i]->isSelected()) {
			m_nCurveSelectedIdx = i;
			return true;
		}
	}
	return false;
}

bool CW3ArrowItem_anno::isLineSelected() const {
	return m_pLine->isSelected();
}

void CW3ArrowItem_anno::drawingPath(CW3PathItem& pPathItem, std::vector<QPointF> vecPointList) const {
	if (vecPointList.size() != 2)
		return;

	QPointF st = vecPointList[0];
	QPointF mt = (vecPointList[1] - vecPointList[0]) / 2 + vecPointList[0];
	QPointF et = vecPointList[1];

	const float lenX = mt.rx() - st.rx();
	const float lenY = mt.ry() - st.ry();
	std::vector<QPointF> vPT = {
		st,
		QPointF(mt.rx() + lenY / 2.0f, mt.ry() - lenX / 2.0f),
		QPointF(mt.rx() + lenY / 4.0f, mt.ry() - lenX / 4.0f),
		QPointF(et.rx() + lenY / 4.0f, et.ry() - lenX / 4.0f),
		QPointF(et.rx() - lenY / 4.0f, et.ry() + lenX / 4.0f),
		QPointF(mt.rx() - lenY / 4.0f, mt.ry() + lenX / 4.0f),
		QPointF(mt.rx() - lenY / 2.0f, mt.ry() + lenX / 2.0f),
		st
	};

	pPathItem.drawingPath(vPT);
	//QPainterPath p(st);
	//for (int i = 0; i < 6; i++)
	//	p.lineTo(TempPoint[i].x(), TempPoint[i].y());
	//p.lineTo(st);
	//
	//pPathItem.setPath(p);
}

void CW3ArrowItem_anno::graphicItemsConnection() {
	if (m_pLine) {
		//connect(m_pLine, SIGNAL(sigHighlightPath(bool)), this, SLOT(slotHighlightCurve(bool)));
		connect(m_pLine, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslatePath(QPointF)));
	}

	for (auto &i : m_listpNode)
		connect(i, SIGNAL(sigTranslateEllipse(QPointF)), this, SLOT(slotTranslateEllipse(QPointF)));
}

//void CW3ArrowItem_anno::slotHighlightCurve(bool highlighted)
//{
//	if (highlighted)
//	{
//		for (auto &i : m_listpNode)
//			i->setHighlightEffect(true);
//	}
//	else
//	{
//		for (auto &i : m_listpNode)
//			i->setHighlightEffect(false);
//	}
//}

void CW3ArrowItem_anno::updatePath() 
{
	if (m_nCurveSelectedIdx >= 0 && m_nCurveSelectedIdx < m_listpNode.size())
		m_listNodePt.at(m_nCurveSelectedIdx) = m_listpNode.at(m_nCurveSelectedIdx)->pos();
	m_pLine->setPos(QPointF(0, 0));
	drawingPath(*m_pLine, m_listNodePt);
}

void CW3ArrowItem_anno::updatePoints(const QPointF& trans) {
	for (int i = 0; i < m_listNodePt.size(); i++) {
		QPointF transPos = m_listNodePt.at(i) + trans;
		m_listNodePt.at(i) = transPos;
		m_listpNode.at(i)->setPos(transPos);
	}
}

void CW3ArrowItem_anno::slotTranslateEllipse(const QPointF& pos) {
	if (!isNodeSelected())
		return;

	updatePath();
}

void CW3ArrowItem_anno::slotTranslatePath(const QPointF& ptTrans) {
	if (!isNodeSelected() && !isLineSelected())
		return;

	updatePoints(ptTrans);
}

void CW3ArrowItem_anno::setNodeDisplay(bool bDisplay) {
	for (auto &i : m_listpNode)
		i->setVisible(bDisplay);
}

void CW3ArrowItem_anno::transformItems(const QTransform & transform) {
	for (int i = 0; i < m_listNodePt.size(); i++) {
		m_listNodePt[i] = transform.map(m_listNodePt[i]);
	}

	for (int i = 0; i < m_listpNode.size(); i++) {
		m_listpNode[i]->setPos(transform.map(m_listpNode.at(i)->pos()));
	}

	updatePath();
}

void CW3ArrowItem_anno::ApplyPreferences() {
	ApplyNodeColor();
	ApplyLineColor();
}

void CW3ArrowItem_anno::ApplyNodeColor() {
	QColor color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	for (int i = 0; i < m_listpNode.size(); i++)
		m_listpNode[i]->setPenColor(color);
}

void CW3ArrowItem_anno::ApplyLineColor() {
	QColor color = GlobalPreferences::GetInstance()->preferences_.objects.measure.line_color;
	QPen pen = m_pLine->pen();
	pen.setColor(color);
	m_pLine->setPen(pen);
}
