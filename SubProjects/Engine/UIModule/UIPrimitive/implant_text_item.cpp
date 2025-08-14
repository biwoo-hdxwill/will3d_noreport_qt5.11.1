#include "implant_text_item.h"
/**=================================================================================================
Copyright (c) 2017 ~ 2018 All rights reserved by HDXWILL.
*===============================================================================================**/

#include "../../Common/Common/color_will3d.h"
#include "../../Resource/Resource/implant_resource.h"

ImplantTextItem::ImplantTextItem(QGraphicsItem* parent) : SimpleTextItem(parent)
{
	setZValue(0.0f);

	SetImplantSpecText(selected_);
}
ImplantTextItem::ImplantTextItem(ImplantData* implant_data, QGraphicsItem* parent) :
	SimpleTextItem(parent)
{
	setZValue(0.0f);

	implant_id_ = implant_data->id();
	implant_diameter_ = implant_data->diameter();
	implant_length_ = implant_data->length();

	SetImplantSpecText(selected_);
}

ImplantTextItem::~ImplantTextItem() {}

void ImplantTextItem::ChangeImplantSpec(ImplantData* implant_data)
{
	implant_id_ = implant_data->id();
	implant_diameter_ = implant_data->diameter();
	implant_length_ = implant_data->length();

	SetImplantSpecText(selected_);
}

void ImplantTextItem::setPos(const QPointF& scene_pos)
{
	scene_position_ = scene_pos;
	setPosCenter(scene_pos);
}

void ImplantTextItem::SetSelected(bool selected)
{
	SetSelected(selected, scene_position_);
}

void ImplantTextItem::SetSelected(bool selected, const QPointF& scene_pos)
{
	if (selected_ != selected)
	{
		SetImplantSpecText(selected);
	}

	if (scene_pos != pos())
	{
		setPos(scene_pos);
	}
}

void ImplantTextItem::SetCollided(bool collided)
{
	if (collided)
	{
		setTextColor(ColorImplant::kImplantCollided);
	}
	else
	{
		setTextColor(ColorImplant::kImplantPlaced);
	}
}

void ImplantTextItem::TransformItems(const QTransform & transform)
{
	setPos(transform.map(scene_position_));
}

void ImplantTextItem::SetImplantSpecText(bool selected)
{
	if (selected)
	{
		setTextColor(ColorImplant::kImplantSelected);
		SetText(QString("#%1\nD %2\nL %3")
			.arg(QString::number(implant_id_))
			.arg(QString::number(implant_diameter_))
			.arg(QString::number(implant_length_)));
	}
	else
	{
		setTextColor(ColorImplant::kImplantPlaced);
		SetText(QString("#%1").arg(QString::number(implant_id_)));
	}
	selected_ = selected;
}
