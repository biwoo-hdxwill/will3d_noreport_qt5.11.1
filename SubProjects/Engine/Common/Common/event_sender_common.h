#pragma once

/**=================================================================================================

Project:		Common
File:			event_sender_common.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-06
Last modify: 	2018-11-06

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <QObject>

class QLayout;
class EventSenderCommon : public QObject {
	Q_OBJECT

public:
	EventSenderCommon();
	~EventSenderCommon();

	EventSenderCommon(const EventSenderCommon&) = delete;
	EventSenderCommon& operator=(const EventSenderCommon&) = delete;

signals:
	void sigSetTabSlotLayout(QLayout* layout);
	void sigSetMainVolume();
	void sigSetSecondVolume();
	void sigSetPanoVolume();
	void sigClearPanoVolume();
	void sigUpdateNerve();
	void sigUpdateImplant();
	void sigSetTFpreset(const QString& preset);
	void sigMoveTFpolygon(const double& value);
};
