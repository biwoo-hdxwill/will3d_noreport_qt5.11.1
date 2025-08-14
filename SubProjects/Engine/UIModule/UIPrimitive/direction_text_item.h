#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			direction_text_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-06-19
Last modify: 	2018-06-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <memory>

#include "W3TextItem.h"
#include "uiprimitive_global.h"


class UIPRIMITIVE_EXPORT DirectionTextItem : public CW3TextItem{
public:
	DirectionTextItem();
	~DirectionTextItem();

public:
	void setVisible(bool visible);
	void SetText(const QString& text, bool is_align_left);
	void SetSceneSize(const double& scene_width, const double& scene_height);
	void UpdatePosition();

	inline bool is_align_left() const { return  is_align_left_; }

private:

	bool is_align_left_ = false;

	double scene_width_ = 0.0;
	double scene_height_ = 0.0;
};
