#include "event_handle_common.h"

#include "W3Logger.h"
#include "event_sender_common.h"

EventHandleCommon::EventHandleCommon() {
	sender_.reset(new EventSenderCommon);
}

EventHandleCommon::~EventHandleCommon() {
}

void EventHandleCommon::EmitSigSetTabSlotLayout(QLayout * layout) const {
	emit sender_->sigSetTabSlotLayout(layout);
}

void EventHandleCommon::EmitSigSetMainVolume() const {
	emit sender_->sigSetMainVolume();
}

void EventHandleCommon::EmitSigSetSecondVolume() const {
	emit sender_->sigSetSecondVolume();
}

void EventHandleCommon::EmitSigSetPanoVolume() const {
	emit sender_->sigSetPanoVolume();
}

void EventHandleCommon::EmitSigClearPanoVolume() const {
	emit sender_->sigClearPanoVolume();
}

void EventHandleCommon::EmitSigSetTFpreset(const QString& preset) const {
	emit sender_->sigSetTFpreset(preset);
}

void EventHandleCommon::EmitSigMoveTFpolygon(const double& value) const {
	emit sender_->sigMoveTFpolygon(value);
}

void EventHandleCommon::EmitSigUpdateNerve() const {
	emit sender_->sigUpdateNerve();
}

void EventHandleCommon::EmitSigUpdateImplant() const {
	emit sender_->sigUpdateImplant();
}

void EventHandleCommon::ConnectSigSetTabSlotLayout(TabContentsWidget * receiver, const char * slot)  const {
	connect(sender_.get(), SIGNAL(sigSetTabSlotLayout(QLayout*)), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigSetMainVolume(const CWill3D *receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigSetMainVolume()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigSetSecondVolume(const CWill3D * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigSetSecondVolume()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigSetPanoVolume(const CWill3D * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigSetPanoVolume()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigClearPanoVolume(const CWill3D * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigClearPanoVolume()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigSetTFpreset(const CWill3D * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigSetTFpreset(QString)), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigMoveTFpolygon(const CWill3D * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigMoveTFpolygon(double)), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigUpdateNerve(const GLNerveWidget * receiver,
										 const char * slot) const {
	connect(sender_.get(), SIGNAL(sigUpdateNerve()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigUpdateImplant(const GLImplantWidget * receiver,
										   const char * slot) const {
	connect(sender_.get(), SIGNAL(sigUpdateImplant()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigUpdateNerve(const ImplantCollisionRenderer * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigUpdateNerve()), (QObject*)receiver, slot, Qt::UniqueConnection);
}

void EventHandleCommon::ConnectSigUpdateImplant(const ImplantCollisionRenderer * receiver, const char * slot) const {
	connect(sender_.get(), SIGNAL(sigUpdateImplant()), (QObject*)receiver, slot, Qt::UniqueConnection);
}
