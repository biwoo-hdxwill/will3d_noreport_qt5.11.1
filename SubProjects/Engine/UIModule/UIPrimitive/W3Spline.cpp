#include "W3Spline.h"

/*=========================================================================

File:			class CW3Spline
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-07-15
Modify Date:	2015-09-04
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <qgraphicsscene.h>
#include <qvector2d.h>

#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Memory.h"

CW3Spline::CW3Spline(int nID, QGraphicsItem* parent)
	: CW3Curve(nID, parent) {
	m_pShiftedCurvePath = nullptr;
	m_bVisibleShiftedPath = true;
	m_fSplineShifted = 0;
}

CW3Spline::~CW3Spline() {
	clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// public functions
/////////////////////////////////////////////////////////////////////////////////////////

void CW3Spline::setOpacity(float opacity) {
	for (auto &i : m_lEllipseCurve)
		i->setOpacity(opacity);

	if (m_pCurvePath != nullptr)
		m_pCurvePath->setOpacity(opacity);
}
qreal CW3Spline::opacity() {
	if (m_pCurvePath)
		return m_pCurvePath->opacity();
	else return 0.0;
}

void CW3Spline::forceAddPoint(const QPointF pointf) {
	if (m_lCurveData.size() == 0)
		return;

	CW3EllipseItem* pEllipse = new CW3EllipseItem(pointf, this);

	addEllipse(pEllipse);

	m_lEllipseCurve.push_back(pEllipse);
	m_lCurveData.push_back(pointf);

	updateSpline();

	emit sigUpdateSpline();
}

void CW3Spline::TransformItems(const QTransform & transform) {
	CW3Curve::TransformItems(transform);
	updateSpline();
}

void CW3Spline::setVisibleShiftedPath(const bool bFlag) {
	m_bVisibleShiftedPath = bFlag;

	if (m_lSplineData.size() > 0) {
		updateSpline();
	}
}

void CW3Spline::setPointPos(const QPointF pos, const int nIndex) {
	CW3Curve::setPointPos(pos, nIndex);
}
void CW3Spline::updateSpline() {
	if (m_lCurveData.size() < 2) {
		if (m_pShiftedCurvePath != nullptr)
			SAFE_DELETE_OBJECT(m_pShiftedCurvePath);

		if (m_pCurvePath != nullptr)
			SAFE_DELETE_OBJECT(m_pCurvePath);
		return;
	}

	if (m_pCurvePath == nullptr) {
		m_pCurvePath = new CW3PathItem(this);
		m_pCurvePath->setHighlighted(adjust_mode_);
		m_pCurvePath->setZValue(m_fZValue);
		m_pCurvePath->setPen(m_penPath);
	}

	m_lSplineData.clear();
	Common::generateCubicSpline(m_lCurveData, m_lSplineData);

	if (m_fSplineShifted == 0 || !m_bVisibleShiftedPath) {
		m_lShiftedSplineData.clear();

		if (m_pShiftedCurvePath != nullptr)
			SAFE_DELETE_OBJECT(m_pShiftedCurvePath);

		m_pCurvePath->drawingPath(m_lSplineData);
	} else {
		if (m_pShiftedCurvePath == nullptr) {
			m_pShiftedCurvePath = new CW3PathItem(this);
			QPen pen = QPen(QColor(200, 248, 200), 2, Qt::SolidLine);
			m_pShiftedCurvePath->setPen(pen);
			m_pShiftedCurvePath->setZValue(0);
		}

		m_lShiftedSplineData.clear();

		std::vector<QPointF> splinePoints = m_lSplineData;
		std::vector<QPointF> curvePoints = m_lCurveData;
		QPointF addTail = splinePoints.back() * 2 - splinePoints.at(splinePoints.size() - 2);

		splinePoints.push_back(addTail);

		for (int i = 0; i < splinePoints.size() - 1; i++) {
			QPointF ptTemp = splinePoints.at(i + 1) - splinePoints.at(i);
			QVector2D norDir = QVector2D(-ptTemp.y(), ptTemp.x());
			norDir.normalize();

			QVector2D vOffset = norDir * m_fSplineShifted;

			QPointF curveData = splinePoints.at(i);
			m_lShiftedSplineData.push_back(QPointF(curveData.x() + vOffset.x(),
												   curveData.y() + vOffset.y()));
		}

		splinePoints.pop_back();

		m_pCurvePath->drawingPath(m_lSplineData);
		m_pShiftedCurvePath->drawingPath(m_lShiftedSplineData);
	}
}

bool CW3Spline::drawShiftedPath(const float Dist) {
	if (m_fSplineShifted != Dist) {
		m_fSplineShifted = Dist;
		updateSpline();

		emit sigUpdateSpline();
		return true;
	}

	return false;
}

QPair<int, QPointF> CW3Spline::insertCloserPoint(const QPointF pointf) {
	float fDistMin = std::numeric_limits<float>::max();
	QPointF insertSplinePoint;
	for (int idx = 0; idx < m_lSplineData.size(); idx++) {
		QPointF splinePoint = m_lSplineData.at(idx);

		float fDist = sqrt((splinePoint.x() - pointf.x())*(splinePoint.x() - pointf.x())
						   + (splinePoint.y() - pointf.y())*(splinePoint.y() - pointf.y()));

		if (fDist < fDistMin) {
			fDistMin = fDist;
			insertSplinePoint = splinePoint;
		}
	}

	QPair<int, QPointF> pirInsertCurvePoint = getInsertPoint(m_lCurveData, pointf);

	if (pirInsertCurvePoint.first >= 0) {
		CW3EllipseItem* pEllipse = new CW3EllipseItem(insertSplinePoint, this);
		pEllipse->SetFlagHighlight(true);
		pEllipse->SetFlagMovable(true);
		this->addEllipse(pEllipse);

		auto insertCurveIdx = m_lCurveData.begin() + pirInsertCurvePoint.first;
		m_lCurveData.insert(insertCurveIdx, insertSplinePoint);

		auto insertEllIdx = m_lEllipseCurve.begin() + pirInsertCurvePoint.first;
		m_lEllipseCurve.insert(insertEllIdx, pEllipse);

		m_lSplineData.clear();
		Common::generateCubicSpline(m_lCurveData, m_lSplineData);
		m_pCurvePath->drawingPath(m_lSplineData);

		updateSpline();

		emit sigUpdateSpline();

		pirInsertCurvePoint.second = insertSplinePoint;
	} else {
		pirInsertCurvePoint.first = -1;
	}

	return pirInsertCurvePoint;
}

void CW3Spline::removePoint(const int nIndexPoint) {
	if (m_lCurveData.size() > 1) {
		auto iterRemoveData = m_lCurveData.begin() + nIndexPoint;
		m_lCurveData.erase(iterRemoveData);

		auto iterRemoveElip = m_lEllipseCurve.begin() + nIndexPoint;
		SAFE_DELETE_OBJECT(*iterRemoveElip);
		m_lEllipseCurve.erase(iterRemoveElip);

		updateSpline();

		emit sigUpdateSpline();
	} else {
		if (nIndexPoint == 0)
			this->clear();
	}
}

void CW3Spline::pop_back(bool isTherePath) {
	auto iterRemoveData = m_lCurveData.end() - 1;
	m_lCurveData.erase(iterRemoveData);

	auto iterRemoveElip = m_lEllipseCurve.end() - 1;
	SAFE_DELETE_OBJECT(*iterRemoveElip);
	m_lEllipseCurve.erase(iterRemoveElip);

	if (isTherePath) {
		updateSpline();

		emit sigUpdateSpline();
	}
}
void CW3Spline::drawingCurPath() {
	if (m_bStartEdit) {
		updateSpline();
	}
}
void CW3Spline::drawingCurPath(const QPointF pointf) {
	if (m_bStartEdit) {
		m_lCurveData.push_back(pointf);

		updateSpline();

		m_lCurveData.pop_back();
	}
}

void CW3Spline::clear() {
	CW3Curve::clear();

	m_lSplineData.clear();
	m_lShiftedSplineData.clear(); //skpark modify 20160623

	if (m_pShiftedCurvePath != nullptr) {
		SAFE_DELETE_OBJECT(m_pShiftedCurvePath);
	}

	m_fSplineShifted = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// private slots
/////////////////////////////////////////////////////////////////////////////////////////

bool CW3Spline::slotTranslateEllipse(const QPointF ptTrans) {
	if (CW3Curve::slotTranslateEllipse(ptTrans)) {
		updateSpline();

		emit sigUpdateSpline();
		return true;
	}

	return false;
}

void CW3Spline::slotTranslatePath(const QPointF trans) {
	CW3Curve::slotTranslatePath(trans);
	updateSpline();

	emit sigUpdateSpline();
}

void CW3Spline::drawCurve() {
	if (isValid()) {
		m_pCurvePath->drawingPath(m_lSplineData);
		if (m_pShiftedCurvePath) {
			m_pShiftedCurvePath->drawingPath(m_lShiftedSplineData);
		}
	}
}

void CW3Spline::SetAdjustMode(const bool enabled)
{
	adjust_mode_ = enabled;
	if (m_pCurvePath)
	{
		m_pCurvePath->setHighlighted(enabled);
	}
	for (auto& e : m_lEllipseCurve)
	{
		e->setVisible(adjust_mode_);
	}
}
