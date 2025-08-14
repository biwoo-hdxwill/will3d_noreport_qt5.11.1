#pragma once
/*=========================================================================

File:			class CW3CEPHtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-12
Last date:		2016-01-12

=========================================================================*/
#include <memory>

#include "../../Engine/Common/Common/W3Enum.h"

#include "base_tab.h"

class CW3MPREngine;
class CW3VREngine;
class CephTaskTool;
class CW3ResourceContainer;
class CW3VTOSTO;
class CW3CephViewMgr;
class WindowPlane;
class QLayout;
#ifndef WILL3D_VIEWER
class ProjectIOCeph;
#endif

class CW3CEPHtab : public BaseTab 
{
	Q_OBJECT
public:
	CW3CEPHtab(CW3VREngine* VREngine, CW3MPREngine *MPRengine, CW3VTOSTO* vtosto, CW3ResourceContainer *Rcontainer);
	virtual ~CW3CEPHtab(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOCeph& out);
	void importProject(ProjectIOCeph& in);
#endif

	void UpdateVRview(bool is_high_quality);

	virtual QLayout* GetTabLayout() override;
	virtual void SetVisibleWindows(bool isVisible) override;

	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id) override;

	void ReleaseSurgeryViewFromLayout();
	virtual void SetLayout() override;
	void disableSurgery();
	void setMIP(bool);
	void ApplyPreferences();

	QWidget* getView3DCeph();
	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;
#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

private:
	virtual void Initialize() override;
	void setVisibleViews(bool isVisible);
	void connections();

	void TaskCoordSysSelect();
	void TaskTracing();
	void TaskSelectAnalysis();
	void TaskSurgery();
	void TaskShowSkin();

	bool setCephLayout(QWidget* v1, QWidget* v2, QWidget* v3);
	bool setCephLayout(QWidget* v1, QWidget* v2);

signals:
	void sigChangeTab(TabType tabType);
	void sigCephNoEventMode();

public slots:
	void slotEnableCephNoEventMode();

private slots:
	void SetTracingLayoutMode();
	void slotSetIndicatorLayoutMode();
	void slotSetNoCephLayoutMode();
	void slotEnableSurgeryBar(const bool);
	void slotCephTask(const CephTaskID&);

private:
	CW3VREngine * m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3VTOSTO* m_pgVTOSTO;

	std::unique_ptr<WindowPlane> window_ = nullptr;
	CW3CephViewMgr*		m_pCephViewMgr = nullptr;
	std::shared_ptr<CephTaskTool> task_tool_;

	bool m_bCephLayoutFlag = false;
	bool m_bCephSurgeryLayoutFlag = false;
};
