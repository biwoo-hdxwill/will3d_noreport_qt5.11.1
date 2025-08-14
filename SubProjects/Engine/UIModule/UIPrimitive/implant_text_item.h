#pragma once
/**=================================================================================================

Project: 		UIPrimitive
File:			simple_text_item.h
Language:		C++11
Library:		Qt 5.8.0
author:			Seo Seok Man
First date:		2018-06-07
Last modify:	2018-06-07

Copyright (c) 2017 ~ 2018 All rights reserved by HDXWILL.

*===============================================================================================**/
#include "simple_text_item.h"
#include "uiprimitive_global.h"

class ImplantData;

class UIPRIMITIVE_EXPORT ImplantTextItem : public SimpleTextItem
{
	Q_OBJECT

public:
	ImplantTextItem(QGraphicsItem* parent = 0);
	ImplantTextItem(ImplantData* implant_data, QGraphicsItem* parent = 0);
	~ImplantTextItem();

	ImplantTextItem(const ImplantTextItem&) = delete;
	ImplantTextItem& operator=(const ImplantTextItem&) = delete;

	void ChangeImplantSpec(ImplantData* implant_data);

	void setPos(const QPointF& scene_pos);

	void SetSelected(bool selected);
	void SetSelected(bool selected, const QPointF& scene_pos);
	void SetCollided(bool collided);

	void TransformItems(const QTransform& transform);

private:
	void SetImplantSpecText(bool selected);

private:
	bool selected_ = false;
	QPointF scene_position_;

	int implant_id_ = 0;
	float implant_diameter_ = 0.0f;
	float implant_length_ = 0.0f;
};
