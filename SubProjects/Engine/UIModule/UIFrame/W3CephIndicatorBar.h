#pragma once

/*=========================================================================

File:			class CW3CephIndicatorBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-05-24
Last modify:	2016-05-24

=========================================================================*/
#include <QWidget>
#include <qframe.h>

#include "uiframe_global.h"

class QSignalMapper;
class QPainter;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class CW3CephIndicatorTabStyle;
class QTabWidget;
class QComboBox;
class CW3CephDM;

class UIFRAME_EXPORT CW3CephIndicatorList : public QWidget {
	Q_OBJECT
public:
	CW3CephIndicatorList(QString caption, QWidget* parent = 0);
	~CW3CephIndicatorList();

public:
	void addRow(const QStringList& strColums);

	void connectMinimize();
	void disconnectMinimize();

	void connectSwitch();
	void disconnectSwitch();

	void setMinimizeText(const QString& str);
	void setMaximizeText(const QString& str);

	void setSwitchTextOn(const QString& str);
	void setSwitchTextOff(const QString& str);

	void changeAnalysisMode();

	QStringList getContentStrList();

	void setSwitch(bool isSwitchOn);

	void syncStatus();
signals:
	void sigChangeContentSwitch(const QString& caption, const QString& firstField, bool isEnable);
private:
	void setContentSwitch(bool isEnable);
private slots:
	void slotMinimize();
	void slotSwitch();
	void slotClickedContent(int);

private:
	QToolButton* m_pCaption;
	QToolButton* m_pSwitch;

	QSignalMapper* m_sigMapper;
	QList<QToolButton*> m_contents;

	std::map<int, int> m_columnWidth;

	QHBoxLayout* m_pCaptionLayout;
	QVBoxLayout* m_pContentLayout;
	QVBoxLayout* m_pMainLayout;

	QString m_strCaption;
	QString m_strMinimize;
	QString m_strMaximize;
	QString m_strSwitchOn;
	QString m_strSwitchOff;

	bool m_flagMinimize;
	bool m_flagSwitch;
};

class UIFRAME_EXPORT CW3CephAnalysisGraph : public QWidget {
	Q_OBJECT

public:
	CW3CephAnalysisGraph(QWidget* parent = 0);
	~CW3CephAnalysisGraph();

public:
	void addAnalysisList(CW3CephIndicatorList* analysisList);
	void clear();

	virtual QSize sizeHint() const override { return this->rect().size(); };
	//virtual QSize minimumSizeHint() const;
protected:
	virtual void paintEvent(QPaintEvent *event) override;

private:
	void drawScalemark(QPainter* painter);
	void drawValueLines(QPainter* painter);
private:

	struct RECORD {
		float value;
		float norm;
		float sd;
		bool isUsed;
		bool isCaption;
	};

	QList<RECORD> m_records;
	QPolygonF* m_polyValues;
};

class UIFRAME_EXPORT CW3CephIndicatorBar : public QFrame {
	Q_OBJECT

public:
	enum TAB {
		TAB_LANDMARK,
		TAB_MEASUREMENT,
		TAB_REFERENCE,
		TAB_ANALYSIS,
		TAB_END
	};
public:
	CW3CephIndicatorBar(CW3CephDM* DataManager, QWidget* parent = 0);
	~CW3CephIndicatorBar();

public:
	TAB getCurrentTab();
	void setCurrentTab(TAB tab);
	QComboBox* getComboBoxSelectAnalysis() {
		return m_pSelectAnalysis;
	}

	void TracingTasksClear();

signals:
	void sigLandmarkChangeContentSwitch(const QString& caption, const QString& firstField, bool isEnable);
	void sigMeasurementChangeContentSwitch(const QString& caption, const QString& firstField, bool isEnable);
	void sigReferenceChangeContentSwitch(const QString& caption, const QString& firstField, bool isEnable);

public slots:
	void slotSyncCephView();

private slots:
	void slotSelectedAnalysis(const QString& analysis);
	void slotChangedRaceBox(const QString& race);
	void slotTurnOnEnableCephData();
	void slotUpdateLandmarkPositions();

private:
	void releaseIndicatorList();
	void releaseLandmark();
	void releaseMeasurement();
	void releaseAnalysis();
	void releaseReference();

	void updateMeasureAndAnalysis(const QString& analysis);
	void updateLandmarkAndReference();

private:
	QTabWidget* m_pTabWidget;
	QList<QWidget*> m_pTabs;
	QList<QVBoxLayout*> m_pTabLayouts;
	QList<CW3CephIndicatorTabStyle*> m_pTabStyle;
	std::map<QString, CW3CephIndicatorList*> m_landmarkList;
	std::map<QString, CW3CephIndicatorList*> m_measurementList;
	std::map<QString, CW3CephIndicatorList*> m_referenceList;
	std::map<QString, CW3CephIndicatorList*> m_analysisList;

	QComboBox* m_pSelectAnalysis;
	QComboBox* m_pRaceComboBox;
	CW3CephAnalysisGraph* m_pAnalysisGraph;

	CW3CephDM* m_pgDataManager;
};
