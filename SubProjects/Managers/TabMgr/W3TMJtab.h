#pragma once
/*=========================================================================

File:			class CW3TMJtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-02

=========================================================================*/
#include <memory>
#include <qboxlayout.h>

#include "base_tab.h"
#include "../../Engine/Common/Common/W3Enum.h"

class CW3TMJViewMgr;
class TMJTaskTool;
class WindowPlane;
class WindowTmjFrontal;
class WindowTmjLateral;
class TMJViewMgr;
class OrientationDlg;
class TMJengine;
#ifndef WILL3D_VIEWER
class ProjectIOTMJ;
#endif

class CW3TMJtab : public BaseTab
{
	Q_OBJECT
public:
	CW3TMJtab();

	virtual ~CW3TMJtab(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOTMJ& out);
	void importProject(ProjectIOTMJ& in);
#endif

	void SetTMjengine(const std::shared_ptr<TMJengine>& tmj_engine);

	virtual void SetVisibleWindows(bool isVisible) override;

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;

	void ApplyPreferences();

	bool ShiftContinuousView(QWidget* source);

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

signals:
	void sigContinuousCapture(QWidget* source);

private:
	void connections();
	virtual void SetLayout() override;
	virtual void Initialize() override;

	void SetDefault2Dlayout();
	void SetLateralMain2Dlayout();
	void SetFrontalMain2Dlayout();
	void SetDefault3Dlayout();
	void SetTMJlayout(const TmjLayoutType& layout_type);
	void SetVisibleCutUI(const bool visible);

private slots:
	void slotTMJGetTaskLayout(QVBoxLayout* layout);
	void slotOrienAdjust();
	void slotTMJLaoutChanged(const TmjLayoutType& layout_type);
	void slotTMJRectDelete(const TMJDirectionType & type);
	void slotSelectLayout(int row, int col);
	void slotResetOrientation();
	void slotGridOnOffOrientation(bool on);
	void slotTMJCutEnable(const bool& cut_on, const VRCutTool& cut_tool);

private:
	TmjLayoutType tmj_layout_type_ = TmjLayoutType::DEFAULT_2D;

	std::shared_ptr<TMJengine> tmj_engine_ = nullptr;
	std::shared_ptr<TMJViewMgr> tmj_view_mgr_ = nullptr;
	std::shared_ptr<TMJTaskTool> task_tool_ = nullptr;

	std::unique_ptr<WindowPlane> window_axial_ = nullptr;
	std::unique_ptr<WindowPlane> window_right_3d_ = nullptr;
	std::unique_ptr<WindowPlane> window_left_3d_ = nullptr;
	std::unique_ptr<WindowTmjFrontal> window_left_frontal_ = nullptr;
	std::unique_ptr<WindowTmjFrontal> window_right_frontal_ = nullptr;
	std::unique_ptr<WindowTmjLateral> window_left_lateral_ = nullptr;
	std::unique_ptr<WindowTmjLateral> window_right_lateral_ = nullptr;

	std::unique_ptr<OrientationDlg> orientation_dlg_ = nullptr;

	std::unique_ptr<QVBoxLayout> main_layout_ = nullptr;
	std::unique_ptr<QHBoxLayout> bottom_layout_ = nullptr;
	std::unique_ptr<QHBoxLayout> top_layout_ = nullptr;
};
