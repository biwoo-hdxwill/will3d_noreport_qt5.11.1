#pragma once
/*=========================================================================

File:			class ImplantHandle
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			TaeHoon Yoo, Hong Jung, Seokman Seo
First Date:		2016-05-30
Last Date:		2018-04-26
Version:		2.0

Copyright (c) 2016 ~ 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <QGraphicsItem>
#include <QPen>

#include "uiprimitive_global.h"

class CW3TextItem;
class CW3EllipseItem;
class QRectF;
class QPainterPath;

//십자모양을 그려주는 아이템
class UIPRIMITIVE_EXPORT ImplantCrossHandle : public QObject, public QGraphicsItem 
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	explicit ImplantCrossHandle(QGraphicsItem* parent = 0);

signals:
	void sigTranslateCross(const QPointF& trans_pos);
	void sigCrossHandleHovered(bool hovered);
	void sigMouseReleased();

public:
	inline const bool hovered() const noexcept { return hovered_; }

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual QRectF boundingRect() const { return rect_; }

protected:
	virtual QPainterPath shape() const override 
	{
		QPainterPath path;
		path.addEllipse(rect_.left(), rect_.top(), rect_.width(), rect_.height());
		return path;
	}

	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
	void setHighlightEffect(const bool bFlag);

private:
	QRectF rect_;
	QPen pen_;

	bool hovered_ = false;
};

// Arrow Class
class ArrowItem : public QGraphicsLineItem {
public:
	ArrowItem(const QColor &color,
			  const QPointF& start_pos,
			  QGraphicsItem *parent = 0);

	//화살표의 시작위치와 끝위치를 저장함.
	void SetPosition(float start_x, float start_y);

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
			   QWidget *widget = 0) Q_DECL_OVERRIDE;

private:
	QPointF pt_start_; // 화살표의 시작 위치
	QPointF pt_end_; // 화살표의 끝 위치
	QColor color_; // 화살표의 색상
	QPolygonF arrow_head_; // 화상표의 머리부분의 점좌표
};


////////////////////////////////////////////////
// Implant handle class
////////////////////////////////////////////////
class UIPRIMITIVE_EXPORT ImplantHandle : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

public:
	explicit ImplantHandle(QGraphicsItem* parent = 0);
	~ImplantHandle();

signals:
	void sigTranslate();
	void sigRotate(float degree_angle);
	void sigHovered(bool hovered);
	void sigUpdate();

public:
	void Enable(int selected_id, bool visible, const QPointF& pos = QPointF(0.0, 0.0));
	void Disable();
	void TransformItems(const QTransform& transform);
	
	inline int selected_id() const noexcept { return selected_id_; }
	inline void set_flag_movable(bool movable) noexcept { flag_movable_ = movable; }
	inline bool is_hovered() const { return is_hovered_; }
	bool isHovered();

	const bool IsActive() const;
	void EndEvent();

	void setArrowDirection(int h_dir, int v_dir);
	void setArrowRotation(float angle);
	void setArrowInit();

	inline const QPointF& prev_pos() { return prev_pos_; }

	virtual QRectF boundingRect() const override { return rect_; }

protected:
	virtual void paint(QPainter *painter,
					   const QStyleOptionGraphicsItem *option,
					   QWidget *widget) override {}
	
private slots:
	void slotCrossHovered(bool hovered);
	void slotTranslateHandle(const QPointF& trans_pt);
	void slotRotateHandle(float degree_angle);
	void slotRotateHandleDone();
	void slotTranslateDone();

private:
	void setArrowVisible(bool b);

private:
	int selected_id_ = -1;
	bool flag_movable_ = true;
	bool processing_ = false;
	QRectF rect_;
	ImplantCrossHandle* trans_handle_ = nullptr;
	CW3EllipseItem* rotate_handle_ = nullptr;

	ArrowItem *h_arrow_ = nullptr;
	ArrowItem *v_arrow_ = nullptr;
	bool is_hovered_ = false;

	enum class EventType 
	{
		NONE,
		TRANSLATE,
		ROTATE
	};

	EventType event_type_ = EventType::NONE;

	QPointF prev_pos_;
	float prev_degree_ = 0.0f;
};
