#pragma once
/**=================================================================================================

Project:		TabMgr
File:			W3IMPLANTtab.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-10
Last modify: 	2018-04-10

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <qboxlayout.h>

#include "base_tab.h"

class CW3MPREngine;
class CW3ResourceContainer;
class CW3VREngine;
class ImplantTaskTool;

class PanoEngine;

class WindowImplant3D;
class WindowImplantSagittal;
class WindowPanoCrossSection;
class WindowPanoArch;
class WindowPano;
class ImplantPanel;
class ImplantViewMgr;
//20250210 LIN
//#ifndef WILL3D_VIEWER
class ProjectIOImplant;
//#endif

namespace implant_resource {
	typedef struct _ImplantInfo ImplantInfo;
}

class CW3IMPLANTtab : public BaseTab {
	Q_OBJECT

public:
	CW3IMPLANTtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
		CW3ResourceContainer *Rcontainer);

	virtual ~CW3IMPLANTtab(void);

private:
	enum class ImplantLayoutType {
		DEFAULT,
		VR_MAIN,
		ARCH_MAIN,
		PANORAMA_MAIN,
		CROSS_SECTION_MAIN
	};

public:
	void SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine);

	// serialization
//20250210 LIN import기능 viewer에 유지
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOImplant& out);
#endif
	void importProject(ProjectIOImplant& in, const bool& is_counterpart_exists);
//#endif

	virtual void SetVisibleWindows(bool isVisible) override;
	virtual void SetApplicationUIMode(const bool& is_maximize) override;

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;
	void SyncMeasureResource();
	virtual void ApplyPreferences() override;

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

signals:
	void sigSyncImplantAngle();
	void sigSyncBDViewStatus();
	void sigSyncCrossSectionParams(const float, const float, const float);
	void sigCrossSectionMainLayoutMaximize(bool);

public slots:
	void slotSyncCrossSectionParams(const float interval, const float degree, const float thickness);

private slots:
	void slotClip3DOnOff(bool);

	void slotSelectLayout(int row, int column);

	void slotPlacedImplant();
	void slotAddImplant(const implant_resource::ImplantInfo& implant_params);
	void slotChangeImplant(const implant_resource::ImplantInfo& implant_params);
	void slotSelectImplant(int implant_id, bool selected);
	void slotDeleteImplant(int implant_id);
	void slotDeleteAllImplants();
	void slotImplantSelectionChanged(int implant_id);
	void slotDeleteImplantFromView(int implant_id);
	void slotImplantMemoChanged(const QString& memo);

	void slotSyncArchType();

	void slotCancelAddImplant();
	void slotMaximizeSingleCrossSectionView(const bool maximized);

private:
	virtual void Initialize() override;
	virtual void SetLayout() override;
	void SetDefaultLayout();
	void SetVRMainLayout();
	void SetArchMainLayout();
	void SetPanoramaMainLayout();
	void SetCrossSectionMainLayout();
	void ChangeLayout(const ImplantLayoutType& layout_type, bool on);
	void SyncOtherWindowMaximizeStatus();
	void connections();

private:
	CW3VREngine * m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	ImplantPanel* implant_panel_ = nullptr;

	std::shared_ptr<ImplantViewMgr> implant_view_mgr_;
	std::shared_ptr<ImplantTaskTool> task_tool_;
	std::shared_ptr<PanoEngine> pano_engine_;

	std::unique_ptr<QVBoxLayout> main_layout_ = nullptr;

	std::unique_ptr<WindowImplant3D> window_3d_implant_ = nullptr;
	std::unique_ptr<WindowImplantSagittal> window_sagittal_ = nullptr;
	std::unique_ptr<WindowPanoCrossSection> window_cross_section_ = nullptr;
	std::unique_ptr<WindowPanoArch> window_arch_ = nullptr;
	std::unique_ptr<WindowPano> window_pano_ = nullptr;
	ImplantLayoutType curr_layout_type_ = ImplantLayoutType::DEFAULT;

	int top_layout_stretch_ = 0;
	int middle_layout_stretch_ = 0;
	int bottom_layout_stretch_ = 0;
	bool single_cross_maximized_ = false;

	QHBoxLayout* bottom_layout_ = nullptr;
};
