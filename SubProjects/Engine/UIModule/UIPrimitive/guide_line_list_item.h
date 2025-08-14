#pragma once
/**=================================================================================================

Project:		UIPrimitive
File:			guide_line_list_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-01-11
Last modify: 	2018-01-11

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "line_list_item.h"

class UIPRIMITIVE_EXPORT GuideLineListItem : public LineListItem {
	Q_OBJECT
public:
	enum ELINE_TYPE {
		HORIZONTAL,
		VERTICAL
	};

	GuideLineListItem(ELINE_TYPE line_type, QGraphicsItem* parent = nullptr);
	~GuideLineListItem();

public:
	void SetRangeScene(const double& min, const double& max);
	virtual void SetHighlight(const bool& is_highlight) override;
	virtual void TransformItems(const QTransform& transform) override;
	void SetVisibleSelectedLine(const int& line_id, const bool& visible);

private:
	void TranslateVerticalLine(const QPointF& trans, const int& id);
	void TranslateHorizontalLine(const QPointF& trans, const int& id);

private slots:
	virtual void slotTranslateLine(const QPointF& trans, int id) override;

private:
	double min_scene_ = 0.0;
	double max_scene_ = 0.0;
	ELINE_TYPE type_;
};
