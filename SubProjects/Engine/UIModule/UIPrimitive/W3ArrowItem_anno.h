#pragma once
/*=========================================================================

File:			class CW3ArrowItem_anno
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QObject>
#include <qpoint.h>

#include "uiprimitive_global.h"

class QGraphicsScene;
class CW3PathItem;
class CW3EllipseItem;

class UIPRIMITIVE_EXPORT CW3ArrowItem_anno : public QObject {
	Q_OBJECT
public:
	CW3ArrowItem_anno(QGraphicsScene* pScene);
	~CW3ArrowItem_anno(void);

public:
	void setNodeDisplay(bool bDisplay);
	bool isNodeSelected();
	bool isLineSelected() const;

	void addPoint(const QPointF& pointf);
	void drawingCurPath(const QPointF& pointf);
	void updatePath();
	void clear();
	void setVisible(bool bFlag);
	bool endEdit();

	void transformItems(const QTransform& transform);

	inline const std::vector<QPointF>& getCurveData() const { return m_listNodePt; }
	inline bool getIsStartEdit() const noexcept { return m_bStartEdit; }

	inline bool isSelected() {
		return (isNodeSelected() || isLineSelected()) ? true : false;
	}

	void ApplyPreferences();

signals:
	void sigUpdateCurve(const std::vector<QPointF> points);

private:
	void drawingPath(CW3PathItem& pPathItem, std::vector<QPointF> vecPointList) const;
	void graphicItemsConnection();
	void updatePoints(const QPointF& trans);

	void ApplyNodeColor();
	void ApplyLineColor();
	
private slots:
	//void slotHighlightCurve(bool);
	void slotTranslateEllipse(const QPointF&);
	void slotTranslatePath(const QPointF&);

private:
	QGraphicsScene * m_pgScene;

	CW3PathItem* m_pLine = nullptr;
	QList<CW3EllipseItem *> m_listpNode;
	std::vector<QPointF> m_listNodePt;

	bool m_bStartEdit = false;
	int	m_nCurveSelectedIdx = -1;
};
