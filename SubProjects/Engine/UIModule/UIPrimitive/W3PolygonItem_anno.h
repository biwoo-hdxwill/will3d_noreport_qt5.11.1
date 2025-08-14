#pragma once
/*=========================================================================

File:			class CW3PolygonItem_anno
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong Kim, JUNG DAE GUN
First Date:		2015-09-10
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <qobject.h>
#include <qpoint.h>

#include <Engine/Common/Common/define_measure.h>
#include "uiprimitive_global.h"

class QGraphicsScene;
class CW3RectItem_anno;
class CW3EllipseItem_anno;
class CW3EllipseItem;
class CW3PathItem;
class CW3LabelItem_anno;

class UIPRIMITIVE_EXPORT CW3PolygonItem_anno : public QObject
{
	Q_OBJECT
public:
	explicit CW3PolygonItem_anno(QGraphicsScene* pScene,
								 common::measure::Shape eShapeType = common::measure::Shape::RECT);
	~CW3PolygonItem_anno(void);

public:
	int getSelectedNode() const;
	bool isLineSelected() const;
	bool isLabelSelected() const;
	
	void AddPoint(const QPointF& pointf);
	void drawingCurPath(const QPointF& pointf);
	bool endEdit();
	void setLabel(const QString& str);
	QString getLabel();
	void displayNode(bool bVisible);
	void setVisible(bool bShow);

	void transformItems(const QTransform& transform);

	bool isSelected() const;

	QRectF getRect();

	std::vector<QPointF> getPoints();
	std::vector<QPointF> GetTwoPoints();

	bool isInRect(const QPointF& pt) const;

	void ApplyPreferences();

private:
	void DrawLabelUI();
	void drawingGuide();
	void drawingLabel();
	void graphicItemsConnection();

	void ApplyNodeColor();
	void ApplyLineColor();
	void ApplyTextColor();
	void ApplyTextSize();

private slots:
	void slotTranslateEllipse(const QPointF&);
	void slotTranslateRect(const QPointF&);
	void slotTranslateLabel(const QPointF&);
	void slotTranslateGuide(const QPointF&);
	void slotHighlightRect(bool);

private:
	bool m_bShowNodes = true;
	int m_nGuideSelectedIdx = 1;
	common::measure::Shape m_eShapeType;
	QPointF start_pt_;
	bool m_bStartEdit = false;
	bool m_bUseLabel = false;
	QGraphicsScene *m_pgScene;
	QPointF m_ptLabelPosOffset = QPointF(30.0f, -30.0f);

	std::vector<CW3EllipseItem *>	node_list_;
	CW3RectItem_anno*				ui_rect_ = nullptr;
	CW3EllipseItem_anno*			ui_circle_ = nullptr;
	CW3PathItem*					ui_label_guide_ = nullptr;
	CW3LabelItem_anno*				label_ = nullptr;
};
