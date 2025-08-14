#pragma once

#include <memory>
#include <mutex>
#include <QObject>

#include "common_global.h"
class EventHandleMeasure;
class EventHandleCommon;

class COMMON_EXPORT EventHandler : public QObject{
	Q_OBJECT
public:
	static void SetInstance();
	static EventHandler* GetInstance();

	EventHandler(const EventHandler&) = delete;
	EventHandler& operator=(const EventHandler&) = delete;
private:
	EventHandler();
	~EventHandler();

public:
	const EventHandleMeasure& GetMeasureEventHandle();
	const EventHandleCommon& GetCommonEventHandle();

private:
	static EventHandler* instance_;
	static std::once_flag onceFlag_;

private:
	std::unique_ptr<EventHandleMeasure> handle_measure_;
	std::unique_ptr<EventHandleCommon> handle_common_;
};
