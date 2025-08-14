#pragma once
/**=================================================================================================

Project:		TabMgr
File:			W3PANOtab.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-10
Last modify: 	2018-04-10

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <qboxlayout.h>
#include <memory>

#include "../../Engine/Common/Common/W3Enum.h"
#include <Engine/Common/GLfunctions/W3GLTypes.h>

#include "base_tab.h"

class WindowPanoCrossSection;
class WindowPanoArch;
class WindowPano;
class OrientationDlg;
class PanoEngine;
class PanoViewMgr;
class PanoTaskTool;
//20250123 LIN 주석처리
//#ifndef WILL3D_VIEWER
class ProjectIOPanorama;
class ProjectIOPanoEngine;
//#endif

class CW3PANOtab : public BaseTab {
	Q_OBJECT
public:
	CW3PANOtab();

	virtual ~CW3PANOtab(void);
	
private:
	enum class PanoLayoutType {
		DEFAULT,
		PANORAMA_MAIN,
		CROSS_SECTION_MAIN
	};

public:
	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOPanorama& out);
	void importProject(ProjectIOPanorama& in_pano, const bool& is_counterpart_exists);
#endif
//20250123 LIN import 기능 viewer에 적용
	void importProjectPanoTool(ProjectIOPanoEngine& in_pano_engine);
//===
	void SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine);

	virtual void SetVisibleWindows(bool isVisible) override;
	inline PanoViewMgr* getPANOViewMgr() { return pano_view_mgr_.get(); }

	void setMIP(bool isMIP);

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;

	void SyncMeasureResource();
	virtual void ApplyPreferences() override;

	void SetInitArchFromMPR(const bool value);
	void SetCurrentArchType(const ArchTypeID& arch_type);
	void UpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
	void ClearArch(ArchTypeID arch_type);

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

signals:
	void sigAutoArch();
	void sigSyncCrossSectionParams(const float, const float, const float);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);
	
public slots:
	void slotSyncCrossSectionParams(const float interval, const float degree, const float thickness);
#ifndef WILL3D_VIEWER
	void slotRequestedGetPanoCrossSectionViewInfo(int& filter_level, int& thickness);
	void slotRequestedCreateCrossSectionDCMFiles(bool nerve_visible, bool implant_visible, int filter, int thickness);
#endif // !WILL3D_VIEWER	

private slots:
	void slotMaximizeOnOff(bool max);
	void slotSelectLayout(int row, int column);
	void slotOrienAdjust();

	void slotResetOrientation();
	void slotGridOnOffOrientation(bool);

	void slotPanoTask(const PanoTaskID&);

	void slotNerveDrawOn();

	void slotTaskResetAutoArchSliceNumber();

	void slotMaximizeSingleCrossSectionView(const bool maximized);

private:
	virtual void Initialize() override;
	virtual void SetLayout() override;
	void connections();
	PanoLayoutType GetMaximizedViewType();

private:
	std::shared_ptr<PanoViewMgr> pano_view_mgr_ = nullptr;
	std::shared_ptr<PanoEngine> pano_engine_;
	std::shared_ptr<PanoTaskTool> task_tool_;

	std::unique_ptr<WindowPanoCrossSection> window_pano_cross_section_ = nullptr;
	std::unique_ptr<WindowPanoArch> window_pano_arch_ = nullptr;
	std::unique_ptr<WindowPano> window_pano_ = nullptr;
	std::unique_ptr<OrientationDlg> orientation_dlg_ = nullptr;

	QVBoxLayout*	m_pTopLayout = nullptr;
	QHBoxLayout*	m_pBottomLayout = nullptr;
	QVBoxLayout*	m_pLayout = nullptr;

	PanoLayoutType curr_layout_type_ = PanoLayoutType::DEFAULT;

	bool init_arch_from_mpr_ = false;

	int top_layout_stretch_ = 0;
	int bottom_layout_stretch_ = 0;
	bool single_cross_maximized_ = false;
};

