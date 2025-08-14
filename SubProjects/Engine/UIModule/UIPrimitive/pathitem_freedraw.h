#pragma once
/*=========================================================================

File:			class PathitemFreedraw
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			Seo Seok Man
First Date:		2018-02-27
Modify Date:	2018-02-27
Version:		1.0

Copyright (c) 2015~2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <vector>

#include <QObject>
#include <QPen>
#include <qpoint.h>
#include <QGraphicsPathItem>
#include "uiprimitive_global.h"

class QGraphicsScene;

class UIPath : public QObject, public QGraphicsPathItem
{
	Q_OBJECT
public:
	UIPath(QGraphicsItem* pParent = 0);
	~UIPath();

signals:
	void sigTranslatePath(const QPointF& ptTrans);
	void sigSelected(bool);

public:
	void setOpacityColor(bool bEnable, uchar alpha = 255);
	void drawingPath(const std::vector<QPointF>& points);

	void SetSelected(bool selected);
	void SetUISelected(bool selected);
	void transformItem(const QTransform& transform);

	QPolygonF GetPolygon();

	void setPen(const QPen& pen);

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
	void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget);
	virtual QPainterPath shape() const override;

private:
	QPainterPath * m_path = nullptr;
	QPen m_penLine;

	bool m_bHovered = false;
	bool selected_ = false;
};

class UIPRIMITIVE_EXPORT PathitemFreedraw : public QObject
{
	Q_OBJECT

public:
	explicit PathitemFreedraw(QGraphicsScene* pScene);
	~PathitemFreedraw();

signals:
	void sigTranslated();
	void sigSelected(bool);

public:
	void setSelected(bool);
	void setVisible(const bool bFlag);
	const bool IsSelected() const;
	void addPoint(const QPointF& pt);
	void drawingCurPath(const QPointF& pointf);
	bool endEdit();

	void removeItemAll();
	void addItemAll();
	void transformItems(const QTransform& transform);

	QPolygonF GetPolygon();
	inline const std::vector<QPointF>& getData() const { return node_pt_list_; }

	void ApplyPreferences();
	void SetLineColor(const QColor& color);
	void SetLineWidth(const float width);

private:
	void drawingPath();
	void graphicItemsConnection();
	void updatePoints(const QPointF& trans);

	void ApplyLineColor();
	void ApplyLineWidth();

private slots:
	void slotTranslatePath(const QPointF&);

private:
	QGraphicsScene * m_pgScene;
	UIPath *line_ = nullptr;
	bool edit_start_ = false;
	std::vector<QPointF> node_pt_list_;
};
