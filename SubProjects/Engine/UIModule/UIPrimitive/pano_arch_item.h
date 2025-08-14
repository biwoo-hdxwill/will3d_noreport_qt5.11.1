#pragma once

/**=================================================================================================

Project: 			UIPrimitive
File:				pano_arch_item.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-22
Last modify:		2017-08-22

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "uiprimitive_global.h"

#include <memory>
#include <map>

#include <QObject>
#include <QGraphicsItem>

#include "W3Spline.h"
class LineListItem;
class CW3TextItem;
class PanningHandleItem;

class UIPRIMITIVE_EXPORT PanoArchItem : public QObject, public QGraphicsItem {
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)

signals:
	void sigUpdated();
	void sigUpdatedFinish();
	void sigEndEdit();
	void sigChangedArchRange();

public:
	enum class DisplayMode {
		PANORAMA,
		IMPLANT
	};

	PanoArchItem();
	~PanoArchItem();

	PanoArchItem(const PanoArchItem&) = delete;
	PanoArchItem& operator=(const PanoArchItem&) = delete;

	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) { };
	virtual QRectF boundingRect() const { return adjust_spline_->boundingRect(); }

public:
  void SetHighlightCrossSection(int cross_id, bool is_highlight);
	void SetAdjustMode(bool is_adjust);
	void SetHighlight(const bool& is_highlight);
	void SetDisplayMode(DisplayMode display_mode);
	void DrawCrossSectionLine(const std::vector<QPointF>& points, const std::vector<int>& index, 
							  int length, float thickness);
	void DrawRuler(int idx_min, int idx_max, int idx_arch_front,
				   const std::vector<int>& medium_gradation,
				   const std::vector<int>& small_gradation);
	void SetShfitedPath(float current_spline_value);
	void SetPanoRange(float range_value);
	void SetThickness(float thickness_value);
	void TransformItem(const QTransform& transform);
	void Clear();
	void CancelLastPoint();

	bool EndEdit();

	void RemoveSelectedPoint();

	bool IsFinished() const { return adjust_spline_->is_finished(); }
	void SetSelectPointCurrentHover();
	void ReleaseSelectedPoint();

	inline bool AddPoint(const QPointF& point) { return adjust_spline_->addPoint(point); }
	inline void InsertCloserPoint(const QPointF point) { adjust_spline_->insertCloserPoint(point); }
	inline void DrawingCurrentPath(const QPointF& point) { adjust_spline_->drawingCurPath(point); }
	inline const std::vector<QPointF>& GetPoints() const { return adjust_spline_->getData(); }
	inline const std::vector<QPointF>& GetCtrlPoints() const { return adjust_spline_->getCurveData(); }
	inline bool IsStartEdit() const { return adjust_spline_->isStartEdit(); }
	inline bool IsHoveredLine() const { return is_hovered_spline_; }
	inline bool IsHoveredPoint() const { return is_hovered_point_spline_; }

	inline float current_spline_value() const { return current_spline_value_; }
	inline float range_value() const { return range_value_; }
	inline float thickness_value() const { return thickness_value_; }
	inline float zoom_scale() const { return zoom_scale_; }

private:
	void InitAdjustSplineItem();
	void InitCurrentSpline();
	void InitCurrentSplineThickness();
	void InitRulerItem();
	void InitRangePathItem();
	void InitCrossSectionItem();
	void InitRangeEllipseItem();

	void SetThicknessSplinePen();
	void SetPositionPanningHandle();

	void DeleteRangePathItem();
	void DeleteRangeEllipseItem();

	void DeleteCurrentSpline();
	void DeleteRulerItem();
	void DeleteCrossSectionItem();

	void DrawCurrentSpline();
	void DrawRangeItems();

	void GetCurrentSplinePoints(std::vector<QPointF>& current_spline) const;
	void GetUpperLowerPoints(std::vector<QPointF>& upper_points, std::vector<QPointF>& lower_points) const;

	void GetSelectedRangeEllipseDir(QVector2D& dir) const;

	QVector2D GetDirection(const QPointF & point0, const QPointF & point1) const;

private slots:
	void slotTranslateRange(const QPointF& pt_trans);
	void slotUpdateSpline();
	void slotTranslateAdjustSpline(const QPointF & pt_trans, const int id);
	void slotHoverArchPoint(const bool is_hovered, const int id, const int index);
	void slotHoverArchSpline(const bool is_hovered);
	void slotTranslatePanningHandle(const QPointF& pt_trans);

private:
	enum RangePath {
		UPPER_PATH = 0,
		LOWER_PATH,
		RANGE_PATH_END,
	};
	enum RangeEllipse {
		UPPER_FRONT = 0,
		UPPER_BACK,
		LOWER_FRONT,
		LOWER_BACK,
		RANGE_ELLIPSE_END
	};

	enum RulerLineType {
		RLT_S,
		RLT_M,
		RLT_L,
		RLT_END
	};

	enum RulerTextType {
		RTT_FRONT,
		RTT_CENTER,
		RTT_BACK,
		RTT_END
	};

	std::unique_ptr<CW3Spline> adjust_spline_ = nullptr;
	std::unique_ptr<CW3EllipseItem> adjust_range_ellipse_[RANGE_ELLIPSE_END];
	std::unique_ptr<CW3PathItem> adjust_range_path_[RANGE_PATH_END];
	std::unique_ptr<PanningHandleItem> adjust_panning_handle_;

	std::unique_ptr<CW3PathItem> curr_spline_ = nullptr;
	std::unique_ptr<CW3PathItem> curr_spline_thickness = nullptr;
	std::unique_ptr<LineListItem> cross_line_ = nullptr;
	std::map<int, std::unique_ptr<CW3TextItem>> cross_number_text_;

	std::unique_ptr<LineListItem> ruler_line_[RLT_END];
	std::unique_ptr<CW3TextItem> ruler_text_[RTT_END];

	float current_spline_value_ = 0.0f;
	float range_value_ = 0.0f;
	float thickness_value_ = 0.0f;
	float cross_section_value_ = 0.0f;

	bool is_mouse_pressed = false;

	bool is_hovered_point_spline_ = false;
	bool is_hovered_spline_ = false;

	bool is_highlight_ = false;

	int id_hovered_spline_ = -1;
	int id_idx_hovered_point_spline_ = -1;
	int id_selected_point_spline_ = -1;

	bool is_adjust_mode_ = true;

	int id_highlighted_cross_line_ = -1;
	float zoom_scale_ = 1.0f;

	DisplayMode display_mode_ = DisplayMode::PANORAMA;
};
