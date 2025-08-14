#pragma once
/*=========================================================================

File:			class CW3ENDOtab
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-07

=========================================================================*/
#include <memory>
#include <qboxlayout.h>

#include "../../Engine/Common/Common/W3Enum.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"
#include "base_tab.h"

class CW3VREngine;
class CW3MPREngine;
class CW3JobMgr;
class CW3ResourceContainer;
class CW3ENDOViewMgr;
class WindowPlane;
class EndoTaskTool;
#ifndef WILL3D_VIEWER
class ProjectIOEndo;
#endif

class CW3ENDOtab : public BaseTab 
{
	Q_OBJECT

public:
	CW3ENDOtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
			   CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer);

	virtual ~CW3ENDOtab(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOEndo& out);
	void importProject(ProjectIOEndo& in);
#endif

	void UpdateVRview(bool is_high_quality);

	virtual void SetVisibleWindows(bool isVisible) override;
	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;
	virtual void DeleteMeasureUI(const common::ViewTypeID & view_type,
								 const unsigned int & measure_id) override;
	void ApplyPreferences();

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

signals:
	void sigSetAirway(std::vector<tri_STL>&);
	void sigAirwayEnable(bool);
	void sigSetAirwaySize(double);

#ifdef WILL3D_EUROPE
private slots:
	void slotMaximizeOnOff(bool maximize);
#endif // WILL3D_EUROPE

private:
	virtual void SetLayout() override;
	virtual void Initialize() override;
	void connections();

private:
	CW3VREngine * m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3JobMgr*	m_pgJobMgr = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3ENDOViewMgr*	m_pENDOViewMgr = nullptr;

	QVBoxLayout*	main_layout_ = nullptr;

	std::shared_ptr<EndoTaskTool> task_tool_;
	std::unique_ptr<WindowPlane> window_slice_ = nullptr;
	std::unique_ptr<WindowPlane> window_sagittal_ = nullptr;
	std::unique_ptr<WindowPlane> window_vr_ = nullptr;
	std::unique_ptr<WindowPlane> window_modify_ = nullptr;
};
