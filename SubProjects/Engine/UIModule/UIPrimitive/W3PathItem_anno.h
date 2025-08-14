#pragma once
/*=========================================================================

File:			class CW3PathItem_anno
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			YouSong kim, JUNG DAE GUN
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <vector>
#include <qobject.h>
#include <qpoint.h>

#include <Engine/Common/Common/define_measure.h>
#include "uiprimitive_global.h"

class CW3EllipseItem;
class CW3PathItem;
class CW3LabelItem_anno;
class QGraphicsScene;

class UIPRIMITIVE_EXPORT CW3PathItem_anno : public QObject {
	Q_OBJECT
public:
	CW3PathItem_anno(
		QGraphicsScene* pScene,
		common::measure::PathType eLineType = common::measure::PathType::LINE,
		bool bClosedPoly = false
	);
	~CW3PathItem_anno(void);

public:
	bool isNodeSelected();
	virtual bool IsLineSelected() const;
	int getSelectedLabel() const;
	bool isSelected();
	virtual void AddPoint(const QPointF& pointf);
	void drawingCurPath(const QPointF& pointf);
	void addCurNode(const QPointF& pt);
	void deleteCurNode();
	void updatePath();
	void clear();
	bool IsVisible();
	virtual void setVisible(bool bFlag);
	void displayNode(bool bVisible);
	void setLabel(const QString& str);
	void setMultiLabel(std::vector<QString> *strArr);
	virtual bool EndEdit();
	QString getLabel();
	QString getLabel(int dIndex);
	QPolygonF getSimplipiedPolygon(const QPointF& pt = QPointF(-1.0f, -1.0f));
	void deleteLastPoint();
	void shiftCurve(const QPointF &newSceneCenter,
					const QPointF &oldSceneCenter, float scale);
	void shiftCurve(const QPointF &shift);

	void transformItems(const QTransform& transform);

	inline const std::vector<QPointF>& getCurveData() const noexcept { return m_listNodePt; }
	inline const std::vector<QPointF>& getSplineData() const noexcept { return m_listSplinePt; }
	inline const bool getIsStartEdit() const noexcept { return m_bStartEdit; }

	inline void setGuideType(common::measure::GuideType type) noexcept { m_eGuideType = type; }

	inline void setFixLabelPos(int dFix) noexcept {
		m_dFixLabelPos = dFix;
		m_nGuideSelectedIdx = 0;
	}

	void ApplyPreferences();

	void SetLineWidth(const float width);
	void SetLineColor(const QColor& color);

signals:
	void sigTranslated();
	void sigMouseReleased();
	void sigLabelClicked();

protected slots:
	virtual void slotTranslatePath(const QPointF&);

private slots:
	void slotHighlightCurve(bool);
	void slotTranslateEllipse(const QPointF&);
	void slotTranslateGuide(const QPointF&);
	void slotTranslateLabel(QPointF);

protected:
	virtual void DrawingPath();
	virtual void GraphicItemsConnection();
	virtual void ApplyLineColor();

private:
	void drawingGuide();
	void drawingLabel();
	void updatePoints(const QPointF trans);

	void ApplyNodeColor();
	void ApplyTextColor();
	void ApplyTextSize();

protected:
	CW3PathItem* line_ = nullptr;
	QGraphicsScene* scene_ = nullptr;

private:
	int m_dFixLabelPos = -1;
	bool m_bClosedPolygon;
	common::measure::PathType	m_eLineType;
	bool m_bStartEdit = false;
	int m_nCurveSelectedIdx = -1;
	int m_nGuideSelectedIdx = 0;
	bool m_bUseLabel = false;
	common::measure::GuideType	m_eGuideType;
	bool m_bShowNodes = true;
	bool fix_line_color_ = false;

	std::vector<QPointF> m_listNodePt;
	std::vector<QPointF> m_listSplinePt;
	std::vector<QPointF> m_listLabelPosOffset;

	std::vector<CW3EllipseItem *> m_listpNode;
	std::vector<CW3PathItem *> m_listpLabelGuideLine;
	std::vector<CW3LabelItem_anno *> m_listpLabel;
};
