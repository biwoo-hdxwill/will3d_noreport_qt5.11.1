#pragma once

/*=========================================================================

File:			class CW3CephViewMgr
Language:		C++11
Library:              Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-11
Last modify:	2016-01-11

=========================================================================*/
#include <memory>
#include <QWidget>

#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_view.h>

class CW3CephDM;
class CW3CephIndicatorBar;
class CephSurgeryTool;
class CW3CephTracingBar;
class CW3MPREngine;
class CW3ResourceContainer;
class CW3View3DCeph;
class CW3VREngine;
class CW3VTOSTO;
class CephTaskTool;
#ifndef WILL3D_VIEWER
class ProjectIOCeph;
#endif

class CW3CephViewMgr : public QWidget 
{
	Q_OBJECT

public:
	CW3CephViewMgr(CW3VREngine *VREngine, CW3MPREngine *MPRengine, CW3VTOSTO* vtosto, CW3ResourceContainer *Rcontainer, QWidget *parent = 0);
	virtual ~CW3CephViewMgr();
	void set_task_tool(const std::shared_ptr<CephTaskTool>& tool);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOCeph& out);
	void importProject(ProjectIOCeph& in, bool& is_data_turn_on);
#endif

	inline QWidget* getView3DCeph() { return (QWidget*)(m_pView3DCeph); }
	inline QWidget* getTracingBar() { return (QWidget*)m_pTracingBar; }
	inline QWidget* getIndicatorBar() { return (QWidget*)m_pIndicatorBar; }
	inline QWidget* getSurgeryBar() { return (QWidget*)surgery_bar_.get(); }

	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	void setVisibleViews(bool bVisible);
	void setVolume(bool& isCephDataTurnOn);
	void setCephNoEventMode(bool isEnable);
	void setMIP(bool isMIP);
	void disableSurgery();

	void UpdateVRview(bool is_high_quality);

	void reset();

	bool IsTracingFinished() const;

	void TaskTracing();
	void TaskSelectAnalysis();
	void TaskSurgery();
	void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);

	void ApplyPreferences();
#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

signals:
	void sigSetIndicatorLayoutMode();
	void sigSetNoCephLayoutMode();
	void sigEnableSurgeryBarLayout(const bool isEnable);

#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

private slots:
	void slotChangeSelectAnalysis(const QString&);

	void slotSurgeryModeEnable(const bool);

	// from tracing bar
	void slotTracingTaskClear();
	void slotTracingTaskFinished();
	void slotTracingTaskCancel();

	// from view
	void slotViewClipLower(int value);
	void slotViewClipUpper(int value);
	void slotViewClipParamsSet(const bool, const bool, const MPRClipID, const int, const int);

	// from tab
	void slotCephClipParamsChanged();
	void slotCephClipSet();
	void slotCephVisible(const VisibleID& id, int status);
	void slotCephChangeFaceTransparency(int value);

private:
	void connections(void);

private:
	std::shared_ptr<CephTaskTool> task_tool_;
	std::unique_ptr<CephSurgeryTool> surgery_bar_;
	CW3View3DCeph*		m_pView3DCeph = nullptr;
	CW3VREngine*		m_pgVRengine = nullptr;

	CW3CephTracingBar*		m_pTracingBar = nullptr;
	CW3CephIndicatorBar*	m_pIndicatorBar = nullptr;
	CW3CephDM*				m_pDataManager = nullptr;
};
