#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			pano_orientatoin_roi_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-22
Last modify: 	2018-02-22

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "uiprimitive_global.h"

#include <memory>
#include <QObject>
#include <QGraphicsItem>


class CW3LineItem;

class UIPRIMITIVE_EXPORT PanoOrientationROIItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	PanoOrientationROIItem();
	~PanoOrientationROIItem();

	PanoOrientationROIItem(const PanoOrientationROIItem&) = delete;
	PanoOrientationROIItem& operator=(const PanoOrientationROIItem&) = delete;

	enum EROI {
		EROI_TOP = 0,
		EROI_BOTTOM,
		EROI_MID,
		EROI_LINE_END
	};

signals:
	void sigTranslateLine(int id, float trans);
	void sigMouseReleased();

public:
	void SetROILine(EROI type, double scene_x1, double scene_x2, double scene_y);
	void SetMinMax(double min, double max);
	bool IsHoveredLines();
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { };
	virtual QRectF boundingRect() const { return rect_; }

private:
	void CreateLines();

	void TransformLine(int line_id, const QTransform& transform);
private slots:
	void slotTranslateLineItem(const QPointF& trans, int id);
private:
	std::unique_ptr<CW3LineItem> lines_[EROI_LINE_END];

	struct ROI {
		double min = 0.0;
		double max = 0.0;
	};
	ROI roi_;

	QRectF rect_;

};
