#pragma once
/*=========================================================================

File:			class CW3Spline
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-07-15
Modify Date:	2017-08-07
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/

#include "W3Curve.h"

class UIPRIMITIVE_EXPORT CW3Spline : public CW3Curve
{
	Q_OBJECT

public:
	CW3Spline(int nID = 0, QGraphicsItem* parent = nullptr);
	~CW3Spline();

signals:
	void sigUpdateSpline();
public:
	void setOpacity(float opacity);
	virtual qreal opacity();
	void forceAddPoint(const QPointF pointf);
	void TransformItems(const QTransform& transform);
	bool drawShiftedPath(const float nDist);
	void updateSpline();

	inline float getShiftedDist() { return m_fSplineShifted; }

	inline void setVisibleShiftedPath(const bool bFlag);

	inline bool isShifted(void) const { return (m_fSplineShifted == 0) ? false : true; }

	//TODO. getData함수가 m_lsplineData를 받게하고 getShiftedData함수를 만들어서 m_lShiftedSplineData를 받게하자.
	//view를 리펙토링 하기 전에 CW3Spline, CW3Curve, CW3Ellipse, CW3Path와 같은
	//primitive item들의 리펙토링이 먼저 수행되어야 함.

	const std::vector<QPointF>& getSplineData() const { return m_lSplineData; } 
	const std::vector<QPointF>& getShiftedSplineData() const { return m_lShiftedSplineData; }
	const std::vector<QPointF>& getData() const { return (m_fSplineShifted == 0) ? m_lSplineData : m_lShiftedSplineData; }
	inline QPointF getData(int i) const { return (m_fSplineShifted == 0) ? m_lSplineData.at(i) : m_lShiftedSplineData.at(i); }

	virtual QPair<int, QPointF> insertCloserPoint(const QPointF pointf) override;

	void drawingCurPath();
	virtual void drawingCurPath(const QPointF pointf) override;

	virtual void removePoint(const int nIndexPoint) override;
	void	pop_back(bool isTherePath);
	virtual void clear() override;
	virtual void setPointPos(const QPointF pos, const int nIndex) override;

	void drawCurve();

	void SetAdjustMode(const bool enabled);

private slots:
	virtual bool slotTranslateEllipse(const QPointF);
	virtual void slotTranslatePath(const QPointF);


private:
	std::vector<QPointF> m_lSplineData;
	std::vector<QPointF> m_lShiftedSplineData;
	
	CW3PathItem*			 m_pShiftedCurvePath;
	float					 m_fSplineShifted;

	bool					 m_bVisibleShiftedPath;

	bool adjust_mode_ = true;
};
