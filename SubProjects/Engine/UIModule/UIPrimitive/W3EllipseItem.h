#pragma once

/*=========================================================================

File:			class CW3EllipseItem
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-06-17
Modify Date:	2016-05-31
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QObject>
#include <QPen>
#include <QGraphicsEllipseItem>
#include <qpoint.h>

#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT CW3EllipseItem : public QObject, public QGraphicsEllipseItem {
	Q_OBJECT

signals:
	void sigTranslateEllipse(const QPointF& ptTrans);
	void sigTranslatedEllipse(const QPointF& ptPos);
	// interaction type이 Rotate일 때, 회전각(degree)을 넘긴다.
	void sigRotateEllipse(float degree_angle);
	void sigRotateWithHandle(const QPointF& prev_pt, const QPointF& curr_pt);
	void sigHoverEllipse(const bool bHovered);
	void sigMouseReleased();
	void sigMousePressed();

public:
	explicit CW3EllipseItem(QGraphicsItem* parent = 0);
	CW3EllipseItem(const QPointF& point, QGraphicsItem *parent = 0);
	CW3EllipseItem(const QPointF& point, const QRectF &rect,
				   const QRectF& hover_rect, QGraphicsItem *parent = 0);

	~CW3EllipseItem(void) {}

	enum class InteractionType {
		TRANSLATE,
		ROTATE,
		ROTATE_HANDLE
	};

public:
	void SetFlagHighlight(const bool & edited);
	void SetFlagMovable(const bool & movable);
	void SetHighlight(bool is_highlight);
	void setPenColor(const QColor& color);
	void setBrushColor(const QColor& color);
	void setVisible(bool is_visible);
	void SetToRotateHandle();

	inline bool isHovered() const noexcept { return is_hovered_; }
	virtual void setPen(const QPen& pen);
	void setHoveredPen(const QPen& pen);
  void setPens(const QPen& normal, const QPen& hovered);
	virtual QPainterPath shape() const;

	inline const QRectF& GetHoveredRect() const noexcept { return rect_hover_; }
	inline const QRectF& GetNormalRect() const noexcept { return rect_normal_; }
	inline const bool& is_highlight() const noexcept { return is_highlight_; }

protected:
	virtual void paint(QPainter *pPainter, const QStyleOptionGraphicsItem *pOption, QWidget *pWidget) override;
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
	void Initialize();

private:
	QPen	pen_normal_;
	QPen	pen_hover_;
	QRectF	rect_normal_;
	QRectF	rect_hover_;

	bool flag_highlight_ = false;
	bool flag_movable_ = false;
	bool is_hovered_ = false;
	bool is_highlight_ = false;
	bool m_bLeftClicked = false;
	InteractionType interaction_type_ = InteractionType::TRANSLATE;
};
