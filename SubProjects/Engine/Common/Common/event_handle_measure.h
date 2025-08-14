#pragma once
#include <memory>
#include <mutex>

#include <QObject>
#include "W3Enum.h"
#include "define_measure.h"
#include "common_global.h"

class EventSenderMeasure;
class MeasureResourceMgr;

class COMMON_EXPORT EventHandleMeasure : public QObject {
	Q_OBJECT

public:
	EventHandleMeasure();
	~EventHandleMeasure();

	EventHandleMeasure(const EventHandleMeasure&) = delete;
	EventHandleMeasure& operator=(const EventHandleMeasure&) = delete;

public:
	void BlockSignals(bool block) const;

	void EmitSigMeasureCreate(const common::ViewTypeID& view_type,
							  const common::measure::MeasureType& measure_type,
							  const common::measure::VisibilityParams& vp,
							  const float& pixel_pitch, const float& scale,
							  std::weak_ptr<MeasureData>* measure_data) const;
	void EmitSigMeasureDelete(const common::ViewTypeID& view_type,
							  const unsigned int& measure_id) const;
	void EmitSigMeasureDeleteAll(const common::ViewTypeID& view_type) const;
	void EmitSigMeasureSetNote(const common::ViewTypeID& view_type,
							   const unsigned int& measure_id,
							   const QString& note) const;
	void EmitSigMeasureModify(const common::ViewTypeID & view_type,
							  const unsigned int & measure_id,
							  const QString & value,
							  const std::vector<QPointF>& points) const;
	void EmitSigMeasureGetMeasureList(const common::ViewTypeID & view_type,
									  std::vector<std::weak_ptr<MeasureData>>* measure_list) const;
	void EmitSigMeasureGetMeasureData(const common::ViewTypeID& view_type,
									  const unsigned int& measure_id,
									  std::weak_ptr<MeasureData>* w_measure_data) const;
	void EmitSigMeasureSetScale(const common::ViewTypeID & view_type, const float& scale) const;
	void EmitSigMeasureSetZoomFactor(const common::ViewTypeID & view_type, const float& scale) const;
	void EmitSigMeasureSetPixelPitch(const common::ViewTypeID & view_type, const float& pixel_pitch) const;
	void EmitSigMeasureSetSceneCenter(const common::ViewTypeID & view_type, const QPointF & center) const;
	void EmitSigMeasureSetSceneTrans(const common::ViewTypeID& view_type, const QPointF& trans) const;
	void EmitSigMeasureSetSceneTransform(const common::ViewTypeID& view_type, const QTransform& transform) const;
	void EmitSigMeasureGetCounterpartViewInfo(const common::ViewTypeID& view_type, common::measure::ViewInfo* view_info) const;
	void EmitSigMeasureGetCurrentViewInfo(const common::ViewTypeID& view_type, common::measure::ViewInfo* view_info) const;
	void EmitSigMeasureSetCounterpartTab(const TabType tab_type) const;
	void EmitSigMeasureSetCounterpartAsCurrentTab() const;

	void ConnectSigMeasureCreate(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureDelete(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureDeleteAll(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetNote(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureModify(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureGetMeasureList(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureGetMeasureData(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetScale(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetZoomFactor(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetPixelPitch(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetSceneCenter(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetSceneTrans(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetSceneTransform(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureGetCounterpartViewInfo(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureGetCurrentViewInfo(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetCounterpartTab(const MeasureResourceMgr* receiver, const char* slot) const;
	void ConnectSigMeasureSetCounterpartAsCurrentTab(const MeasureResourceMgr* receiver, const char* slot) const;

private:
	std::unique_ptr<EventSenderMeasure> sender_;
};
