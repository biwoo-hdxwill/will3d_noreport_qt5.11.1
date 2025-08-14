#include "event_handle_measure.h"

#include "W3Logger.h"
#include "event_sender_measure.h"

EventHandleMeasure::EventHandleMeasure()
{
	sender_.reset(new EventSenderMeasure);
}

EventHandleMeasure::~EventHandleMeasure() {}

void EventHandleMeasure::BlockSignals(bool block) const
{
	sender_->blockSignals(block);
}

void EventHandleMeasure::EmitSigMeasureCreate(const common::ViewTypeID& view_type,
	const common::measure::MeasureType& measure_type,
	const common::measure::VisibilityParams& vp,
	const float& pixel_pitch, const float& scale,
	std::weak_ptr<MeasureData>* measure_data) const
{
	emit sender_->sigMeasureCreate(view_type, measure_type, vp,
		pixel_pitch, scale, measure_data);
}

void EventHandleMeasure::EmitSigMeasureDelete(const common::ViewTypeID& view_type,
	const unsigned int& measure_id) const
{
	emit sender_->sigMeasureDelete(view_type, measure_id);
}

void EventHandleMeasure::EmitSigMeasureDeleteAll(const common::ViewTypeID & view_type) const
{
	emit sender_->sigMeasureDeleteAll(view_type);
}

void EventHandleMeasure::EmitSigMeasureSetNote(const common::ViewTypeID & view_type,
	const unsigned int & measure_id,
	const QString & note) const
{
	emit sender_->sigMeasureSetNote(view_type, measure_id, note);
}

void EventHandleMeasure::EmitSigMeasureModify(const common::ViewTypeID & view_type,
	const unsigned int & measure_id,
	const QString & value,
	const std::vector<QPointF>& points) const
{
	emit sender_->sigMeasureModify(view_type, measure_id, value, points);
}

void EventHandleMeasure::EmitSigMeasureGetMeasureList(const common::ViewTypeID & view_type,
	std::vector<std::weak_ptr<MeasureData>>* measure_list) const
{
	emit sender_->sigMeasureGetMeasureList(view_type, measure_list);
}

void EventHandleMeasure::EmitSigMeasureGetMeasureData(const common::ViewTypeID & view_type,
	const unsigned int & measure_id,
	std::weak_ptr<MeasureData>* w_measure_data) const
{
	emit sender_->sigMeasureGetMeasureData(view_type, measure_id, w_measure_data);
}

void EventHandleMeasure::EmitSigMeasureSetScale(const common::ViewTypeID & view_type, const float & scale) const
{
	emit sender_->sigMeasureSetScale(view_type, scale);
}

void EventHandleMeasure::EmitSigMeasureSetZoomFactor(const common::ViewTypeID & view_type, const float & zoom_factor) const
{
	emit sender_->sigMeasureSetZoomFactor(view_type, zoom_factor);
}

void EventHandleMeasure::EmitSigMeasureSetPixelPitch(const common::ViewTypeID & view_type, const float & pixel_pitch) const
{
	emit sender_->sigMeasureSetPixelPitch(view_type, pixel_pitch);
}

void EventHandleMeasure::EmitSigMeasureSetSceneCenter(const common::ViewTypeID & view_type, const QPointF & center) const
{
	emit sender_->sigMeasureSetSceneCenter(view_type, center);
}

void EventHandleMeasure::EmitSigMeasureSetSceneTrans(const common::ViewTypeID & view_type,
	const QPointF & trans) const
{
	emit sender_->sigMeasureSetSceneTrans(view_type, trans);
}

void EventHandleMeasure::EmitSigMeasureSetSceneTransform(const common::ViewTypeID & view_type,
	const QTransform & transform) const
{
	emit sender_->sigMeasureSetSceneTransform(view_type, transform);
}

void EventHandleMeasure::EmitSigMeasureGetCounterpartViewInfo(const common::ViewTypeID & view_type,
	common::measure::ViewInfo* view_info) const
{
	emit sender_->sigMeasureGetCounterpartViewInfo(view_type, view_info);
}

void EventHandleMeasure::EmitSigMeasureGetCurrentViewInfo(const common::ViewTypeID & view_type,
	common::measure::ViewInfo * view_info) const
{
	emit sender_->sigMeasureGetCurrentViewInfo(view_type, view_info);
}

void EventHandleMeasure::EmitSigMeasureSetCounterpartTab(const TabType tab_type) const
{
	emit sender_->sigMeasureSetCounterpartTab(tab_type);
}

void EventHandleMeasure::EmitSigMeasureSetCounterpartAsCurrentTab() const
{
	emit sender_->sigMeasureSetCounterpartAsCurrentTab();
}

void EventHandleMeasure::ConnectSigMeasureCreate(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureCreate(common::ViewTypeID,
		common::measure::MeasureType,
		common::measure::VisibilityParams,
		float, float,
		std::weak_ptr<MeasureData>*)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureDelete(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureDelete(common::ViewTypeID, unsigned int)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureDeleteAll(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureDeleteAll(common::ViewTypeID)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetNote(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetNote(common::ViewTypeID, unsigned int, QString)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureModify(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureModify(common::ViewTypeID, unsigned int,
		QString, std::vector<QPointF>)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureGetMeasureList(const MeasureResourceMgr * receiver,
	const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureGetMeasureList(common::ViewTypeID,
		std::vector<std::weak_ptr<MeasureData>>*)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureGetMeasureData(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureGetMeasureData(common::ViewTypeID, unsigned int,
		std::weak_ptr<MeasureData>*)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetScale(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetScale(common::ViewTypeID, float)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetZoomFactor(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetZoomFactor(common::ViewTypeID, float)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetPixelPitch(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetPixelPitch(common::ViewTypeID, float)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetSceneCenter(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetSceneCenter(common::ViewTypeID, QPointF)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetSceneTrans(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetSceneTrans(common::ViewTypeID, QPointF)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetSceneTransform(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetSceneTransform(common::ViewTypeID, QTransform)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureGetCounterpartViewInfo(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureGetCounterpartViewInfo(common::ViewTypeID, common::measure::ViewInfo*)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureGetCurrentViewInfo(const MeasureResourceMgr * receiver, const char * slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureGetCurrentViewInfo(common::ViewTypeID, common::measure::ViewInfo*)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetCounterpartTab(const MeasureResourceMgr* receiver, const char* slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetCounterpartTab(TabType)),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleMeasure::ConnectSigMeasureSetCounterpartAsCurrentTab(const MeasureResourceMgr* receiver, const char* slot) const
{
	connect(sender_.get(), SIGNAL(sigMeasureSetCounterpartAsCurrentTab()),
		(QObject*)receiver, slot, Qt::UniqueConnection);
}
