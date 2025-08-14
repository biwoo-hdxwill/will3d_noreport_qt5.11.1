#pragma once
/**=================================================================================================

Project:		UIPrimitive
File:			rotate_line_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-04-18
Last modify: 	2018-04-18

Copyright (c) 2018 HDXWILL. All rights reserved.

*===============================================================================================**/
#include <qpoint.h>
#include <qvector2d.h>

#include "W3LineItem.h"

#include "uiprimitive_global.h"
class UIPRIMITIVE_EXPORT RotateLineItem : public CW3LineItem {
	Q_OBJECT

public:
	RotateLineItem();
	virtual ~RotateLineItem();

	RotateLineItem(const RotateLineItem&) = delete;
	RotateLineItem& operator=(const RotateLineItem&) = delete;

signals:
	void sigRotateLine(float angle);

public:
	void SetHighlight(const bool& is_highlight);
	QPointF TransformItems(const QTransform& transform);
	void SetLine(const QPointF& pos, const QVector2D& vector);
	void UpdateLine();

	bool IsActive() const noexcept { return active_; }
	void EndEvent() noexcept { active_ = false; }

	QPointF p1() const;
	QPointF p2() const;
	
	QVector2D LineDirection() const noexcept { return line_vector_; }
	inline bool IsHighlight() const noexcept { return flag_highlight(); }
	inline void set_length(const float& length) { length_ = length; }

public slots:
	void slotRotateWithHandle(const QPointF& prev_pt, const QPointF& curr_pt);

private:
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

	void RotateLine(const QPointF& mouse_curr);

private:
	bool active_ = false;
	QPointF mouse_prev_;
	QPointF center_position_;
	QVector2D line_vector_ = QVector2D(1.0f, 0.0f);
	float length_ = 0.0f;
};
