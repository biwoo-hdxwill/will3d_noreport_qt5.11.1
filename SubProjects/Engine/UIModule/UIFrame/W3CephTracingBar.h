#pragma once

/*=========================================================================

File:			class CW3CephTracingBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-06-01
Last modify:	2016-06-01

=========================================================================*/
#include <QFrame>

#include "../../Common/Common/W3Enum.h"
#include "uiframe_global.h"

class QVBoxLayout;
class QLabel;
class QToolButton;
class QScrollArea;

class UIFRAME_EXPORT CW3CephTracingBar : public QFrame {
	Q_OBJECT

public:
	CW3CephTracingBar(QWidget* parent = 0);
	~CW3CephTracingBar();

public:
	void checkAll();

	//tracing을 시작할 때 현재 tracing bar 상태를 체크하고 반영한다.
	void awake();
signals:
	void sigActiveTracingTask(const QString&);
	void sigClearTracingTasks();
	void sigFinishedTracingTasks();
	void sigSetCoordSystem(const QStringList&);
	void sigCancelTracingTask();
	void sigSetupTracingTask();

public slots:
	void slotNextTracingTask(const QString&);
	void slotSetGuideImage(const QString& landmark);
private:
	enum TRACING_ICON_STYLE {
		ICON_ACTIVE,
		ICON_BOTTOM,
		ICON_ISOL,
		ICON_MID,
		ICON_TOP,
		ICON_UNCHECK
	};

	enum BTN_COMMAND {
		COMMAND_START,
		COMMAND_RESET,
		COMMAND_CANCEL,
		COMMAND_SETUP,
		COMMAND_END
	};

	/* Ceph tracing landmark status */
	enum TRACING_STATUS {
		TRACING_UNCHECK,
		TRACING_CHECK,
		TRACING_ACTIVE,
	};

private:
	void setTracingStatus(const QString& tracing, TRACING_STATUS status, TRACING_ICON_STYLE style);
	QPixmap loadImage(const QString& path);
	void connections();
	void setTracingListScrollValue(const int& curTaskPos);

private slots:
	void slotClickedTracing(const QString&);
	void slotClickedStart();
	void slotClickedReset();
	void slotClickedCancel();
	void slotClickedSetup();

private:
	QVBoxLayout* m_pMainLayout;
	QScrollArea* m_pTracingListArea;
	QLabel* m_pCaptureImg;

	QStringList m_tracingLists;
	std::map<QString, QToolButton*> m_btnTracingLists;
	std::map<QString, TRACING_STATUS> m_tracingStatus;

	QList<QToolButton*> m_btnCommand;

	bool m_isSetCoordSys = false;
	bool m_isTracingDone = false;
};
