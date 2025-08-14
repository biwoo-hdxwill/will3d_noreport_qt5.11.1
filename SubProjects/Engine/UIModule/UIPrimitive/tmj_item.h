#pragma once

/**=================================================================================================

Project:		UIPrimitive
File:			tmj_item.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-16
Last modify: 	2018-11-16

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include <memory>

#include <QGraphicsItem>
#include <QObject>

#include "../../Common/Common/W3Enum.h"
#include "uiprimitive_global.h"

class CW3RectItem;
class LineListItem;
class CW3EllipseItem;
class PolygonItem;
class CW3ArcItem;
class CW3Curve;
class CW3LineItem;
class CW3TextItem;
class TmjArrowItem;

class UIPRIMITIVE_EXPORT TMJItem : public QObject, public QGraphicsItem
{
	Q_OBJECT
		Q_INTERFACES(QGraphicsItem)
public:
	TMJItem();
	~TMJItem();

	TMJItem(const TMJItem&) = delete;
	TMJItem& operator=(const TMJItem&) = delete;

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
		QWidget* widget)
	{
	};
	virtual QRectF boundingRect() const;

	struct InitParam
	{
		float scene_width = 0.0f;
		float scene_height = 0.0f;
		float left_roi_width = 0.0f;
		float left_roi_height = 0.0f;
		float right_roi_width = 0.0f;
		float right_roi_height = 0.0f;
		float degree_angle = 0.0f;
	};
signals:
	void sigUpdated(const TMJDirectionType& type);

public:
	void Initialize(const InitParam& params);
	void SetPositionAndDegree(const TMJDirectionType& type,
		const QPointF& center_pos,
		const QVector2D& up_vector);
	void SetFrontalLineVisible(
		const TMJDirectionType& type, const bool& is_visible);
	void SetLateralLineVisible(const TMJDirectionType& type,
		const bool& is_visible);
	void SetHighlightLateralLine(const TMJDirectionType& type, const int& index);
	void SetTMJRectSize(const TMJRectID& roi_id, float value);
	bool ShiftLateralLine(const TMJDirectionType& type, const float& value);
	bool ShiftFrontalLine(const TMJDirectionType& type, const float& value);

	bool GetROIcenter(const TMJDirectionType& type, QPointF* pt_centezr_in_scene);
	void GetROIRectSize(const TMJDirectionType& type, float* width,
		float* height) const;
	bool GetLateralPositionInfo(const TMJDirectionType& type,
		std::map<float, QPointF>* pt_center_in_scene,
		QPointF* up_vector_in_scene) const;
	const float& GetLateralInterval(const TMJDirectionType& type) const;
	bool GetFrontalPositionInfo(const TMJDirectionType& type,
		QPointF* pt_center_in_scene,
		QPointF* up_vector_in_scene) const;

	void AddPoint(const QPointF& pt, TMJDirectionType* tmj_type_created);
	void DrawPoint(const QPointF& pt);

	void SetDrawOn(const TMJDirectionType& direction_type, bool draw_on);
	void ClearTMJ(const TMJDirectionType& type);
	void TransformItems(const QTransform& transform);

	void SetLateralParam(const TMJLateralID& id, const float& value);
	void GetLateralParam(const TMJLateralID& id, float* value) const;
	void SetLateralLineCount(const TMJDirectionType& type, int count);
	void SetLateralSliceInterval(const TMJDirectionType& type, float interval);

	bool IsAvaliableAddPoint() const;
	bool IsStartEdit() const;

	void GetAvailableFrontalRange(const TMJDirectionType& type, float& min, float& max);
	void GetAvailableLateralRange(const TMJDirectionType& type, float& min, float& max);

private:
	void UpdateItems(const TMJDirectionType& type);
	void UpdateTMJRect(const TMJDirectionType& type);
	void UpdateFrontalLine(const TMJDirectionType& type);
	void UpdateLateralLine(const TMJDirectionType& type);
	void UpdateAngleItems(const TMJDirectionType& type);
	void UpdateResizeArrow(const TMJDirectionType& type);

	void CreateTMJwidget(const TMJDirectionType& type);
	void CreateAngleWidget(const TMJDirectionType& type);

	// CreateTMJwidget
	void CreateROIItem(const TMJDirectionType& type);
	void CreateFrontalLine(const TMJDirectionType& type);
	void CreateLateralLine(const TMJDirectionType& type);
	void CreateFrontalEllipse(const TMJDirectionType& type);
	void CreateResizeArrow(const TMJDirectionType& type);

	// CreateAngleWidget
	void CreateAngleArc(const TMJDirectionType& type);
	void CreateAngleArcLine(const TMJDirectionType& type);
	void CreateAngleHorizonLine(const TMJDirectionType& type);
	void CreateAngleText(const TMJDirectionType& type);

	void CreateCriterionLine();
	void CreateDrawLine();

	void DeleteTMJwidget(const TMJDirectionType& type);
	void DeleteAngleWidget(const TMJDirectionType& type);
	void DeleteDrawLine();

	void GetFrontalVector(const TMJDirectionType& type, QVector2D* vec) const;
	void GetLateralVector(const TMJDirectionType& type, QVector2D* vec) const;

	bool TranslateTmjResize(const QPointF& trans);

	bool TranslateTmjResizeVertical(const TMJDirectionType& type,
		const QPointF& trans);
	bool TranslateTmjResizeHorizon(const TMJDirectionType& type,
		const QPointF& trans);
	bool IsValidTMJ(const TMJDirectionType& type) const;

	bool IsShiftableFrontalLine(const TMJDirectionType& type, const float shift);
	bool IsShiftableLateralLine(const TMJDirectionType& type, const float shift);

	void ClampShiftedFrontalLine(const TMJDirectionType& type);
	void ClampShiftedLateralLine(const TMJDirectionType& type);

	TMJDirectionType GetDrawType() const;
	void UpdateHighlightLateralLine(const TMJDirectionType& type) const;

	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

private slots:
	void slotTranslateTmjEll(const QPointF& trans);
	void slotTranslateTmjLines(const QPointF& trans);
	void slotTranslateTmjROI(const QPointF& trans);
	void slotTranslateCriterion();
	void slotHoverTmjResizeVertical(bool is_hover);
	void slotHoverTmjResizeHorizon(bool is_hover);

private:
	struct TMJWidget
	{
		std::unique_ptr<PolygonItem> roi;
		std::unique_ptr<LineListItem> frontal_line;
		std::unique_ptr<LineListItem> lateral_line;
		std::unique_ptr<CW3EllipseItem> frontal_ellipse[2];
		std::unique_ptr<TmjArrowItem> frontal_resize_arrow[2];
		std::unique_ptr<TmjArrowItem> lateral_resize_arrow[2];
	};

	struct AngleWidget
	{
		std::unique_ptr<CW3ArcItem> arc;
		std::unique_ptr<CW3LineItem> arc_line;
		std::unique_ptr<CW3LineItem> h_line;
		std::unique_ptr<CW3TextItem> text;
	};

	struct ROIinfo
	{
		float width = 0.0f;
		float height = 0.0f;
		int lateral_line_count = 0;
		float lateral_line_interval = 0.0f;
		float lateral_line_thickness = 0.0f;
		float lateral_line_shifted = 0.0f;
		float frontal_line_shifted = 0.0f;
		std::vector<float> line_id;
		int lateral_highlight_index = 0;
		bool is_lateral_visible_ = true;
		bool is_frontal_visible_ = true;
		QPointF center;
	};

	std::unique_ptr<CW3Curve> criterion_line_;
	std::unique_ptr<CW3Curve> draw_line_;
	TMJWidget tmj_widget_[TMJDirectionType::TMJ_TYPE_END];
	AngleWidget angle_widget_[TMJDirectionType::TMJ_TYPE_END];
	ROIinfo roi_info_[TMJDirectionType::TMJ_TYPE_END];

	bool is_init = false;
	float angle_length_ = 0.0f;

	bool draw_on_[TMJDirectionType::TMJ_TYPE_END] = { false, false };
};
