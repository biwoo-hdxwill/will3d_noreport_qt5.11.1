#pragma once

#include <memory>
#include <QTransform>
#include <QObject>
#include "W3Enum.h"
#include "define_measure.h"

class EventSenderMeasure : public QObject {
	Q_OBJECT

public:
	EventSenderMeasure();
	~EventSenderMeasure();

	EventSenderMeasure(const EventSenderMeasure&) = delete;
	EventSenderMeasure& operator=(const EventSenderMeasure&) = delete;

signals:
	void sigMeasureCreate(const common::ViewTypeID& view_type,
						  const common::measure::MeasureType& measure_type,
						  const common::measure::VisibilityParams& vp,
						  const float& pixel_pitch, const float& scale,
						  std::weak_ptr<MeasureData>* measure_data);
	void sigMeasureDelete(const common::ViewTypeID& view_type,
						  const unsigned int& measure_id);
	void sigMeasureDeleteAll(const common::ViewTypeID& view_type);
	void sigMeasureSetNote(const common::ViewTypeID& view_type,
						   const unsigned int& measure_id,
						   const QString & note);
	void sigMeasureModify(const common::ViewTypeID & view_type,
						  const unsigned int & measure_id,
						  const QString & value,
						  const std::vector<QPointF>& points);
	void sigMeasureGetMeasureList(const common::ViewTypeID & view_type,
								  std::vector<std::weak_ptr<MeasureData>>* measure_list);
	void sigMeasureGetMeasureData(const common::ViewTypeID& view_type,
								  const unsigned int& measure_id,
								  std::weak_ptr<MeasureData>* w_measure_data);
	void sigMeasureSetScale(const common::ViewTypeID& view_type, const float& scale);
	void sigMeasureSetZoomFactor(const common::ViewTypeID& view_type, const float& zoom_factor);
	void sigMeasureSetPixelPitch(const common::ViewTypeID& view_type, const float& pixel_pitch);
	void sigMeasureSetSceneTrans(const common::ViewTypeID& view_type, const QPointF& trans);
	void sigMeasureSetSceneTransform(const common::ViewTypeID& view_type, const QTransform& transform);
	void sigMeasureSetSceneCenter(const common::ViewTypeID & view_type, const QPointF & center);
	void sigMeasureGetCounterpartViewInfo(const common::ViewTypeID& view_type, common::measure::ViewInfo* view_info);
	void sigMeasureGetCurrentViewInfo(const common::ViewTypeID& view_type,
									  common::measure::ViewInfo* view_info);
	void sigMeasureSetCounterpartTab(const TabType tab_type);
	void sigMeasureSetCounterpartAsCurrentTab();
};
