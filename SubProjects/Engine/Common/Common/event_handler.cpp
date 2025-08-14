#include "event_handler.h"

#include "W3Logger.h"
#include "event_handle_measure.h"
#include "event_handle_common.h"

EventHandler* EventHandler::instance_ = nullptr;
std::once_flag EventHandler::onceFlag_;

void EventHandler::SetInstance() {
	std::call_once(EventHandler::onceFlag_,[=]() {instance_ = new EventHandler;});
}

EventHandler * EventHandler::GetInstance() {
	if (instance_ == nullptr) {
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR, "EventHandler::GetInstance: The instance does not exist");
	}
	return instance_;
}

const EventHandleMeasure& EventHandler::GetMeasureEventHandle() {
	return *(handle_measure_.get());
}

const EventHandleCommon& EventHandler::GetCommonEventHandle() {
	return *(handle_common_.get());
}

EventHandler::EventHandler() {
	handle_measure_.reset(new EventHandleMeasure);
	handle_common_.reset(new EventHandleCommon);
}

EventHandler::~EventHandler() {
}
