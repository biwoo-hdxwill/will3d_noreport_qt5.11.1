#include "W3Curve.h"
/*=========================================================================

File:			class CW3Curve
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-07-09
Modify Date:	2015-09-05
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include "qgraphicsscene.h"
#include "../../Common/Common/W3Memory.h"

CW3Curve::CW3Curve(int nID, QGraphicsItem* parent)
	: m_nID(nID),
	QGraphicsItem(parent) {
	m_pCurvePath = nullptr;
	m_bStartEdit = false;
	m_nCurveSelectedIdx = -1;

	m_penPath = QPen(QColor(128, 255, 0), 2, Qt::SolidLine);
	m_penPath.setCosmetic(true);
	m_penEllipse = QPen(QColor(0, 255, 0), 2, Qt::SolidLine);
	m_penEllipse.setCosmetic(true);
	m_brushEllipse = QBrush(QColor(255, 0, 0));

	this->setZValue(0);
	m_fZValue = 10.f;
}

CW3Curve::~CW3Curve() {
	clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////////

void CW3Curve::startEdit() {
	if (m_lCurveData.size() == 0) {
		m_bStartEdit = true;
		m_pCurvePath = new CW3PathItem(this);
		m_pCurvePath->setZValue(m_fZValue);
		m_pCurvePath->setPen(m_penPath);
	}
}

bool CW3Curve::addPoint(const QPointF pointf) {
	if (!m_bStartEdit)
		startEdit();

	if (m_bStartEdit) {
		CW3EllipseItem* pEllipse = new CW3EllipseItem(pointf, this);
		pEllipse->setZValue(m_fZValue);
		addEllipse(pEllipse);
		m_lEllipseCurve.push_back(pEllipse);
		m_lCurveData.push_back(pointf);

		return true;
	}

	return false;
}

void CW3Curve::editPoint(int idx, const QPointF & point) {
	if ((int)m_lCurveData.size() - 1 < idx ||
		idx < 0) {
		return;
	}

	m_lEllipseCurve[idx]->setPos(point);
	m_lCurveData[idx] = point;
}

void CW3Curve::TransformItems(const QTransform& transform) {
	for (int i = 0; i < m_lCurveData.size(); i++) {
		QPointF pos = m_lCurveData[i];
		m_lCurveData[i] = transform.map(pos);
	}
	for (int i = 0; i < m_lEllipseCurve.size(); i++) {
		QPointF pos = m_lEllipseCurve[i]->pos();
		m_lEllipseCurve[i]->setPos(transform.map(pos));
	}

	DrawPath();
}

void CW3Curve::setPointPos(const QPointF pos, const int nIndex) {
	if (nIndex >= 0 && nIndex < m_lEllipseCurve.size() && nIndex < m_lCurveData.size()) {
		m_lEllipseCurve.at(nIndex)->setPos(pos);
		m_lCurveData.at(nIndex) = pos;
		DrawPath();
	}
}

void CW3Curve::setVisible(const bool bFlag) {
	for (auto &i : m_lEllipseCurve)
		i->setVisible(bFlag);

	if (m_pCurvePath != nullptr)
		m_pCurvePath->setVisible(bFlag);
}

bool CW3Curve::endEdit() {
	m_bStartEdit = false;

	int ptCnt = m_lCurveData.size();

	if (ptCnt > 1) {
		if (m_pCurvePath != nullptr)
			m_pCurvePath->setHighlighted(true);

		for (auto &i : m_lEllipseCurve) {
			i->SetFlagHighlight(true);
			i->SetFlagMovable(true);
			i->setSelected(false);
		}

		connectCurvePath();

		is_finished_ = true;
	} else
		is_finished_ = false;

	SetBoundingRect();

	return is_finished_;
}
void CW3Curve::sortPointsOrderX() {
	int curveDir = m_lCurveData.back().x() - m_lCurveData.front().x();

	if (curveDir < 0) {
		std::reverse(std::begin(m_lCurveData), std::end(m_lCurveData));
		std::reverse(std::begin(m_lEllipseCurve), std::end(m_lEllipseCurve));
	}

	m_lEllipseCurve.front()->setBrushColor(QColor(125, 0, 255));
}

void CW3Curve::insertPoint(const int nIndex, const QPointF pointf) {
	if (nIndex >= 0 && nIndex < m_lCurveData.size()) {
		CW3EllipseItem* pEllipse = new CW3EllipseItem(pointf, this);
		addEllipse(pEllipse);

		auto iterInsertElip = m_lEllipseCurve.begin() + nIndex;
		m_lEllipseCurve.insert(iterInsertElip, pEllipse);

		auto iterInsertData = m_lCurveData.begin() + nIndex;
		m_lCurveData.insert(iterInsertData, pointf);

		DrawPath();
	}
}

QPair<int, QPointF> CW3Curve::insertCloserPoint(const QPointF pointf) {
	QPair<int, QPointF> pirInsertPoint = getInsertPoint(m_lCurveData, pointf);

	if (pirInsertPoint.first >= 0) {
		CW3EllipseItem* pEllipse = new CW3EllipseItem(pirInsertPoint.second, this);
		addEllipse(pEllipse);

		auto iterInsertElip = m_lEllipseCurve.begin() + pirInsertPoint.first;
		m_lEllipseCurve.insert(iterInsertElip, pEllipse);

		auto iterInsertData = m_lCurveData.begin() + pirInsertPoint.first;
		m_lCurveData.insert(iterInsertData, pirInsertPoint.second);

		DrawPath();
	} else
		pirInsertPoint.first = -1;

	return pirInsertPoint;
}

void CW3Curve::removePoint(const int nIndexPoint) {
	if (m_lCurveData.size() > 2) {
		auto iterRemoveData = m_lCurveData.begin() + nIndexPoint;
		m_lCurveData.erase(iterRemoveData);

		auto iterRemoveElip = m_lEllipseCurve.begin() + nIndexPoint;
		SAFE_DELETE_OBJECT(*iterRemoveElip);
		m_lEllipseCurve.erase(iterRemoveElip);

		DrawPath();
	}
}

void CW3Curve::drawingCurPath(const QPointF pointf) {
	if (m_bStartEdit) {
		m_lCurveData.push_back(pointf);

		DrawPath();

		m_lCurveData.pop_back();
	}
}

void CW3Curve::drawCurve() {
	if (isValid()) {
		DrawPath();
	}
}
void CW3Curve::clear() {
	while (m_lEllipseCurve.size()) {
		auto iter = m_lEllipseCurve.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_lEllipseCurve.erase(iter);
	}

	m_lCurveData.clear();

	if (m_pCurvePath != nullptr) {
		SAFE_DELETE_OBJECT(m_pCurvePath);
	}

	m_bStartEdit = false;
	is_finished_ = false;
}

void CW3Curve::setEllipsePosAll() {
	for (int i = 0; i < m_lCurveData.size(); i++) {
		m_lEllipseCurve.at(i)->setPos(m_lCurveData.at(i));
	}
}

bool CW3Curve::isSelectPoints() {
	bool bHasSelectedItem = false;
	int nSize;

	nSize = m_lEllipseCurve.size();
	for (int i = 0; i < nSize; i++) {
		bool bSelected = m_lEllipseCurve[i]->isSelected();

		if (bSelected) {
			m_nCurveSelectedIdx = i;
			bHasSelectedItem = true;
		}
	}

	return bHasSelectedItem;
}

void CW3Curve::setPenPath(const QPen& pen) {
	m_penPath = pen;

	if (m_pCurvePath != nullptr)
		m_pCurvePath->setPen(m_penPath);
}

void CW3Curve::setPenEllipse(const QPen& pen) {
	m_penEllipse = pen;

	for (auto i : m_lEllipseCurve)
		i->setPen(m_penEllipse);
}

void CW3Curve::setBrushEllipse(const QBrush& brush) {
	m_brushEllipse = brush;

	for (auto i : m_lEllipseCurve)
		i->setBrush(m_brushEllipse);
}

int CW3Curve::getIndexUnderMouseElip(void) const {
	for (int i = 0; i < m_lEllipseCurve.size(); i++) {
		if (m_lEllipseCurve.at(i)->isUnderMouse())
			return i;
	}

	return -1;
}

void CW3Curve::translatePath(const QPointF trans) {
	for (int i = 0; i < m_lCurveData.size(); i++) {
		QPointF transPos = m_lCurveData.at(i) + trans;
		m_lCurveData.at(i) = transPos;
		m_lEllipseCurve.at(i)->setPos(transPos);
	}

	DrawPath();
}

//////////////////////////////////////////////////////////////////////////
// private functions
//////////////////////////////////////////////////////////////////////////

void CW3Curve::addEllipse(CW3EllipseItem* pEllipseItem) {
	pEllipseItem->setZValue(m_fZValue + 2);
	pEllipseItem->setBrush(m_brushEllipse);
	pEllipseItem->setPen(m_penEllipse);

	this->connectEllipise(pEllipseItem);
}

void CW3Curve::connectEllipise(CW3EllipseItem* pEllipseItem) {
	connect(pEllipseItem, SIGNAL(sigTranslateEllipse(QPointF)), this, SLOT(slotTranslateEllipse(QPointF)));
	connect(pEllipseItem, SIGNAL(sigTranslatedEllipse(QPointF)), this, SLOT(slotTranslatedEllipse(QPointF)));
	connect(pEllipseItem, SIGNAL(sigHoverEllipse(bool)), this, SLOT(slotHoverEllipse(bool)));
	connect(pEllipseItem, SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));
	connect(pEllipseItem, SIGNAL(sigMousePressed()), this, SIGNAL(sigMousePressed()));
}
void CW3Curve::connectCurvePath() {
	connect(m_pCurvePath, SIGNAL(sigHoverPath(bool)), this, SLOT(slotHighlightCurve(bool)));
	connect(m_pCurvePath, SIGNAL(sigTranslatePath(QPointF)), this, SLOT(slotTranslatePath(QPointF)));
	connect(m_pCurvePath, SIGNAL(sigMouseReleased()), this, SIGNAL(sigMouseReleased()));
}

void CW3Curve::translateEllipse() {
	m_lCurveData.at(m_nCurveSelectedIdx) = m_lEllipseCurve.at(m_nCurveSelectedIdx)->pos();

	DrawPath();
}

void CW3Curve::SetBoundingRect() {
	double min_x;
	double min_y;
	double max_x;
	double max_y;

	min_x = min_y = std::numeric_limits<double>::max();
	max_x = max_y = std::numeric_limits<double>::min();

	for (const auto& elem : m_lCurveData) {
		min_x = std::min(min_x, elem.x());
		min_y = std::min(min_y, elem.y());
		max_x = std::max(max_x, elem.x());
		max_y = std::max(max_y, elem.y());
	}

	m_rect = QRectF(min_x, min_y, max_x - min_x, max_y - min_y);
}

void CW3Curve::DrawPath() {
	if(m_pCurvePath)
		m_pCurvePath->drawingPath(m_lCurveData);
	SetBoundingRect();
}

//////////////////////////////////////////////////////////////////////////
// protected functions
//////////////////////////////////////////////////////////////////////////

QPair<int, QPointF> CW3Curve::getInsertPoint(std::vector<QPointF> targetPoints, QPointF inputPoint) {
	float fDistMin = std::numeric_limits<float>::max();

	int insertIdx = -1;
	QPointF insertCloserPt = QPointF(-1.f, -1.f);

	if (targetPoints.size() > 1) {
		for (int idx = 0; idx < targetPoints.size() - 1; idx++) {
			QPointF p1 = targetPoints.at(idx);
			QPointF p2 = targetPoints.at(idx + 1);

			float a = p2.y() - p1.y();
			float b = p1.x() - p2.x();
			float c = p2.x()*p1.y() - p1.x()*p2.y();

			float ai = -b;
			float bi = a;
			float ci = -ai * inputPoint.x() - bi * inputPoint.y();

			float k = ai * b - bi * a;
			float x, y;

			if (k == 0.0f) {
				x = inputPoint.x();
				y = inputPoint.y();
			} else {
				x = (bi*c - b * ci) / k;
				y = (a*ci - ai * c) / k;
			}

			if (p2.x() < p1.x()) {
				float temp = p1.x();
				p1.setX(p2.x());
				p2.setX(temp);
			}
			if (p2.y() < p1.y()) {
				float temp = p1.y();
				p1.setY(p2.y());
				p2.setY(temp);
			}

			QPoint np1(p1.x(), p1.y());
			QPoint np2(p2.x(), p2.y());
			if (np1.x() <= (int)x && np2.x() >= (int)x && np1.y() <= (int)y && np2.y() >= (int)y) {
				float dist = abs(a*inputPoint.x() + b * inputPoint.y() + c) / sqrt(a*a + b * b);

				if (dist < fDistMin) {
					fDistMin = dist;
					insertIdx = idx + 1;
					insertCloserPt = QPointF(x, y);
				}
			}
		}
	}
	return qMakePair(insertIdx, insertCloserPt);
}

//////////////////////////////////////////////////////////////////////////
// protected slots
//////////////////////////////////////////////////////////////////////////

void CW3Curve::slotHighlightCurve(const bool bIsHighlight) {
	is_hovered_curve = bIsHighlight;

	emit sigHighlightCurve(bIsHighlight, getID());
}

void CW3Curve::slotHoverEllipse(const bool bHovered) {
	is_hovered_ellipse = bHovered;
	
	for (int i = 0; i < m_lEllipseCurve.size(); i++) {
		if (QObject::sender() == dynamic_cast<QObject*>(m_lEllipseCurve[i]))
			emit sigHighlightEllipse(bHovered, getID(), i);
	}
}

bool CW3Curve::slotTranslateEllipse(const QPointF& ptTrans) {
	if (isSelectPoints() && m_lEllipseCurve.at(m_nCurveSelectedIdx)->is_highlight()) {
		translateEllipse();

		emit sigTranslateEllipse(ptTrans, getID());

		return true;
	} else
		return false;
}

void CW3Curve::slotTranslatedEllipse(const QPointF& pos) {
	emit sigTranslatedEllipse(pos, m_nID);
}

void CW3Curve::slotTranslatePath(const QPointF& ptTrans) {
	if (m_pCurvePath->isHighlight()) {
		translatePath(ptTrans);

		emit sigTranslatePath(ptTrans, getID());
	}
}

void CW3Curve::setZValue(const float fZValue) {
	m_fZValue = fZValue;

	if (m_pCurvePath)
		m_pCurvePath->setZValue(m_fZValue);

	for (int i = 0; i < m_lEllipseCurve.size(); i++)
		m_lEllipseCurve.at(i)->setZValue(m_fZValue + 2);
}
