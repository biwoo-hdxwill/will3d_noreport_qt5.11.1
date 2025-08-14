#pragma once

/**=================================================================================================

Project: 			UIPrimitive
File:				pano_nerve_item.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-09-14
Last modify:		2017-09-14

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "uiprimitive_global.h"

#include <memory>
#include <QObject>
#include <QGraphicsItem>

#include "W3Spline.h"

class UIPRIMITIVE_EXPORT PanoNerveItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

signals:
	void sigTranslatedNerveEllipse(int nerve_id, int nerve_selected_ell_index, const QPointF& pt_scene);
	void sigPressedNerveEllipse(int nerve_id, int nerve_selectd_ell_index, bool is_pressed);
	void sigAddNerveEllipse(int nerve_id, const QPointF& pt_scene);
	void sigCancelLastNerveEllipse(int nerve_id, int remove_index);
	void sigClearNerve(int nerve_id);
	void sigRemoveNerveEllipse(int nerve_id, int remove_index);
	void sigInserteNerveEllipse(int nerve_id, int insert_index, const QPointF& pt_scene);

public:
	PanoNerveItem();
	~PanoNerveItem();

	PanoNerveItem(const PanoNerveItem&) = delete;
	PanoNerveItem& operator=(const PanoNerveItem&) = delete;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return rect_; }

public:
	void SetHover(int id, bool is_hover);
	void SetVisible(int id, bool is_visible);
	void SetVisibleAll(bool is_visible);
	void ReleaseSelectedNerve();
	void SetHighlight(const bool& is_highlight);
	void TransformItem(const QTransform& transform);
	void AddPoint(const QPointF& point);

	/**=================================================================================================
	Sets control points.
	@param	control_points	key는 nerve의 ID, value는 nerve의 control points이다.
	 *===============================================================================================**/
	void SetControlPoints(const std::map<int, std::vector<QPointF>>& control_points);

	const std::vector<QPointF>& GetControlPoints(int id) const;
	void GetNerveIDs(std::vector<int>* ids) const;

	void DrawCurrentNerve();
	void DrawCurrentNerve(const QPointF & point);
	void CancelCurrentNerve();
	void Clear(int id);
	void ClearAll();
	void CancelLastPoint(int id);
	void RemoveSelectedPoint(int id);
	void InsertCloserPoint(int id, const QPointF& pt_scene);
	bool EndEdit();
	bool IsEdit() const;
	bool IsHovered() const;
	bool IsEventAddNerve() const;

	int GetAvailableNerveID();

	void SaveCurrentHoveredPointIndex();
	
	inline bool IsHoveredLine() const { return is_hovered_nerve_; }
	inline bool IsHoveredPoint() const { return is_hovered_point_nerve_; }

	inline int curr_nerve_id() const { return id_curr_nerve_; }
	inline int id_hovered_nerve() const { return id_hovered_nerve_; }

private slots:
	void slotTranslatedNerveEllipse(const QPointF& pos);
	void slotHoveredNerveSpline(const bool is_hovered);
	void slotHoveredNervePoint(const bool is_hovered, const int id, const int index);
	void slotMousePressedNerve();

private:
	struct StatusFocusEllipse {
		int nerve_index = -1;
		int ellipse_inex = -1;
		bool focus = false;
	};

	void SetFocusEllipse(const StatusFocusEllipse& status_focus_ell);
	void SetHoverEvent(const bool is_hovered);
	int GetEventSenderNerveID() const;
	void InsertNerve();
	void CreateNerve(int id);
	CW3Spline& CurrentNerve() const;
	bool IsMaximumNerve() const;

private:
	/**key = nerve ID, value = nerve nerve.*/
	std::map<int, std::unique_ptr<CW3Spline>> nerves_;

	QRectF rect_;

	bool is_hovered_point_nerve_ = false;
	bool is_hovered_nerve_ = false;
	bool is_hightlight_ = false;

	int id_curr_nerve_; //현재 addpoint로 그려지고 있는 nerve id.
	int id_selected_nerve_; //컨트롤 포인트 클릭 시 선택된 nerve id.
	int id_hovered_nerve_; //현재 호버된 nerve id.

	int idx_saved_hovered_pt_; //SaveCurrentHoveredPointIndex() 함수로 인해 저장된 호버 포인트 인덱스.
	int idx_hovered_pt_; //현재 호버된 nerve의 ellipse index.

	StatusFocusEllipse status_focus_ell_;
};
