#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			pano_ruler_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-27
Last modify: 	2018-02-27

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "uiprimitive_global.h"

#include <memory>
#include <QObject>
#include <QGraphicsItem>

class LineListItem;
class CW3LineItem;
class CW3TextItem;
class UIPRIMITIVE_EXPORT PanoRulerItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)
public:
	PanoRulerItem();
	~PanoRulerItem();

	PanoRulerItem(const PanoRulerItem&) = delete;
	PanoRulerItem& operator=(const PanoRulerItem&) = delete;

public:
	void SetRuler(int idx_min, int idx_max, int idx_arch_front,
		int idx_min_in_scene, int idx_max_in_scene, int idx_arch_front_in_scene,
		const std::vector<int>& medium_gradation,
		const std::vector<int>& small_gradation,
		const std::vector<double>& medium_gradation_in_scene,
		const std::vector<double>& small_gradation_in_scene,
		const QPointF& position);
	void DeleteRulerItem();

	void TransformItems(const QTransform& transform);

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return rect_; }
	inline bool initialized() const { return initialized_; }
	inline const void set_pixel_spacing(const float pixel_spacing) { pixel_spacing_ = pixel_spacing; }

private:
	void InitRulerItem();
	CW3TextItem* CreateTextItem();

private:
	enum RulerLineType
	{
		RLT_V,
		RLT_S,
		RLT_M,
		RLT_L,
		RLT_END
	};
	enum RulerTextType
	{
		RTT_FRONT,
		RTT_CENTER,
		RTT_BACK,
		RTT_END
	};

	std::unique_ptr<LineListItem> ruler_lines_[RLT_END];
	std::unique_ptr<CW3TextItem> slice_index_texts_[RTT_END];
	std::vector<CW3TextItem*> length_texts_;

	QRectF rect_;
	bool initialized_ = false;
	float pixel_spacing_ = 0.0f;
};
