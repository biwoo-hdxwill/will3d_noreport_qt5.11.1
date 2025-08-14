#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			line_list_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-18
Last modify: 	2017-12-18

	Copyright (c) 2017 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include <memory>
#include <QObject>
#include <QGraphicsItem>
#include <QPen>

#include "uiprimitive_global.h"

class CW3LineItem;
class QVector2D;
class QPointF;
////////////////////////////////////////////////////////////////////////////////////
//LineListItem
////////////////////////////////////////////////////////////////////////////////////

class UIPRIMITIVE_EXPORT LineListItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	LineListItem(QGraphicsItem* parent = nullptr);
	~LineListItem();

	LineListItem(const LineListItem&) = delete;
	LineListItem& operator=(const LineListItem&) = delete;
public:
	
	virtual void SetHighlight(const bool& is_highlight);
	void SetMovable(const bool & flag_movable);
	void ClearLines();
	void AddLine(const QPointF& pos, const QVector2D& vector);
	void SetLine(int line_id, const QPointF& pos, const QVector2D& vector);
	void SetRotatePointAtLine(int line_id, const QPointF& pos);
	void UpdateLine(int line_id);
	
	void SetVisibleSpecificLine(const bool& is_visible, int line_id);
	void SetColorSpecificLine(const QColor& color, int line_id);
	virtual void TransformItems(const QTransform& transform);
	
	void GetLineVertex(int line_id, QPointF& p1, QPointF& p2);
	QPointF GetLineCenterPosition(int line_id);

	QPointF GetLineRotatePosition(int line_id);

	int GetHoveredLineID() const;
	void TransformLine(int line_id, const QTransform& transform);

	inline void set_pen(const QPen& pen) { pen_ = pen; }
	inline void set_length(const float& length) { length_ = length; }
	inline bool is_highlight() const { return is_highlight_; }
	inline bool is_hovered() const { return is_hovered_; }
	const std::map<int, std::unique_ptr<CW3LineItem>>& lines() { return lines_; }
	const std::map<int, QPointF>& line_positions() const { return line_positions_; }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { };


	virtual QRectF boundingRect() const;

signals:
	void sigTranslateLines(const QPointF& trans);
	void sigMouseReleased();

protected:
	inline bool flag_movable() const { return flag_movable_; }
	inline bool flag_highlight() const { return flag_highlight_; }

private slots:
	void slotHighLightEventLine(bool is_highlight);
	void slotHoveredEvent(bool is_hovered);
	virtual void slotTranslateLine(const QPointF & trans, int id);

private:
	int GetNewLineID() const;
	void InitLine(int line_id, const QPointF& pos, const QVector2D& vector);
	bool IsInvalidLineID(int line_id);

private:
	/**key = line ID, value = CW3LineItem.*/
	std::map<int, std::unique_ptr<CW3LineItem>> lines_;

	std::map<int, QPointF> line_positions_;
	std::map<int, QPointF> line_rotate_positions_;
	std::map<int, QVector2D> line_vectors_;
	float length_ = 0.0f;
	bool flag_highlight_ = false;
	bool flag_movable_ = false;
	bool is_highlight_ = false;
	bool is_hovered_ = false;
	QPen pen_;
};
