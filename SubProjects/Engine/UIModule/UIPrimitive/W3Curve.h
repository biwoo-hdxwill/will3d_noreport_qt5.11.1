#pragma once

/*=========================================================================

File:			class CW3Curve
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-07-09
Modify Date:	2017-08-07
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include "W3EllipseItem.h"
#include "W3PathItem.h"

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3Curve : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

	typedef struct _CURVE_HIGHTLIGHTED_INFO
	{
		int nID = 0;
		bool bHighlight = false;
		QPointF ptSceneMouse;
	} curveHighInfo_t;

public:
	CW3Curve(int nID = 0, QGraphicsItem* parent = nullptr);
	~CW3Curve();
public:
	void	startEdit();
	bool	addPoint(const QPointF pointf);
	void	editPoint(int idx, const QPointF& point);

	void TransformItems(const QTransform& transform);

	void	sortPointsOrderX();
	void	setEllipsePosAll();
	bool	isSelectPoints();
	inline const std::vector<QPointF>& getCurveData() { return m_lCurveData; }
	inline const int		getSelectedIndex() { return m_nCurveSelectedIdx; }
	inline const bool		isStartEdit() { return m_bStartEdit; }
	inline const bool		is_finished() const { return is_finished_; }
	inline const bool		IsHoverItem() const { return (is_hovered_curve || is_hovered_ellipse); }

	void	setZValue(const float fZValue);

	virtual bool	endEdit();
	virtual void	setVisible(const bool bFlag);
	virtual void	insertPoint(const int nIndex, const QPointF pointf);
	virtual QPair<int, QPointF> 	insertCloserPoint(const QPointF pointf);
	virtual void	drawingCurPath(const QPointF pointf);
	virtual void	removePoint(const int nIndexPoint);
	virtual void	clear();
	virtual void	setPointPos(const QPointF pos, const int nIndex);

	void setPenPath(const QPen& pen);
	void setPenEllipse(const QPen& pen);
	void setBrushEllipse(const QBrush& brush);
	
	int getIndexUnderMouseElip(void) const;
	void translatePath(const QPointF trans);

	inline void setHighligtedPath(const bool bIsHighlight) { if(m_pCurvePath) m_pCurvePath->setHighlighted(bIsHighlight); }
	inline void setHighligtedEllipse(const bool bIsHighlight) { for (const auto& i : m_lEllipseCurve) { i->SetFlagHighlight(bIsHighlight); i->SetFlagMovable(bIsHighlight); } }
	inline void setHighligtedEllipse(const int nIndex, const bool bIsHighlight) { m_lEllipseCurve.at(nIndex)->SetFlagHighlight(bIsHighlight); m_lEllipseCurve.at(nIndex)->SetFlagMovable(bIsHighlight); }
	inline void setHighlightEffectEllipse(const int nIndex, const bool bIsHighlight) { m_lEllipseCurve.at(nIndex)->SetHighlight(bIsHighlight); }
	inline void setHighlightFlagEllipse(const int nIndex, const bool bIsHighlight) { m_lEllipseCurve.at(nIndex)->SetFlagHighlight(bIsHighlight); }
	inline void setVisibleEllipse(const bool bFlag) { for (const auto& i : m_lEllipseCurve) i->setVisible(bFlag); }
	inline void setVisiblePath(const bool bFlag) { if (m_pCurvePath) m_pCurvePath->setVisible(bFlag); }

	inline int getID(void) const { return m_nID; }
	inline int getCurveSelectedID(void) const { return m_nCurveSelectedIdx; }
	inline int getEllipseCount() const { return m_lEllipseCurve.size(); }
	inline bool isValid(void) const { return (m_lCurveData.size() > 1) ? true : false; }
	void drawCurve();


	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return m_rect; }

private:
	void	connectEllipise(CW3EllipseItem* pEllipseItem);
	void	connectCurvePath();
	void	translateEllipse();
	void	SetBoundingRect();
	void	DrawPath();


protected:
	QPair<int, QPointF> getInsertPoint(std::vector<QPointF> targetPoints, QPointF inputPoint);
	void	addEllipse(CW3EllipseItem* pEllipseItem);

signals:
	void sigTranslateEllipse(const QPointF&, const int nID);
	void sigTranslatePath(const QPointF&, const int nID);
	void sigHighlightCurve(const bool bFlag, const int nID);
	void sigHighlightEllipse(const bool bFlag, const int nID, const int nIndex);
	void sigTranslatedEllipse(const QPointF& pos, const int nID);
	void sigMouseReleased();
	void sigMousePressed();

protected slots:
	virtual void	slotHighlightCurve(const bool);
	virtual void	slotHoverEllipse(const bool);
	virtual bool	slotTranslateEllipse(const QPointF&);
	virtual void	slotTranslatedEllipse(const QPointF&);
	virtual void	slotTranslatePath(const QPointF&);

protected:

	CW3PathItem*					m_pCurvePath;
	std::vector<QPointF>			m_lCurveData;
	QList<CW3EllipseItem*>			m_lEllipseCurve;

	int								m_nCurveSelectedIdx;
	bool							m_bStartEdit;
	float							m_fZValue;

	QPen							m_penPath;
	QPen							m_penEllipse;
	QBrush							m_brushEllipse;

	int								m_nID;
	QRectF							m_rect;

	bool							is_finished_ = false;
	bool							is_hovered_curve = false;
	bool							is_hovered_ellipse = false;
};
