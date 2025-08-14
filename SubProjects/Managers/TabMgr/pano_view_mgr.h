#pragma once
/**=================================================================================================

Project:		TabMgr
File:			W3PANOViewMgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-05
Last modify: 	2018-04-05

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "base_pano_view_mgr.h"

class ViewPanoOrientation;
class ViewPanoArch;
class ViewPano;
class ViewPanoCrossSection;
class PanoTaskTool;
#ifndef WILL3D_VIEWER
class ProjectIOPanorama;
#endif

class PanoViewMgr : public BasePanoViewMgr {
	Q_OBJECT

public:
	explicit PanoViewMgr(QObject* parent = nullptr);
	~PanoViewMgr();

signals:
	void sigNerveDrawModeOn();
	void sigMaximizeSingleCrossSectionView(const bool);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public:
	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOPanorama& out);
	void importProject(ProjectIOPanorama& in_pano, const bool& is_counterpart_exists);
#endif

	void SetPanoMenu(const std::shared_ptr<PanoTaskTool>& task_menu);
	virtual void SyncPanoResource() override;

	void ResetOrientationAndResetArch(const bool& is_redraw_auto_arch);
	void GridOnOffOrientation(const bool& on);
	void TaskManualArch();

	inline QWidget* GetViewPanoOrient(const ReorientViewID& id) const {
		return (QWidget*)(view_pano_orient_[id].get());
	}

	void ApplyPreferences();
	void ResetOrientationParams();

	virtual void MoveViewsToSelectedMeasure(
		const common::ViewTypeID& view_type,
		const unsigned int& measure_id) override;

	virtual void ConnectPanoMenus() override;
	virtual void DisconnectPanoMenus() override;

	void SetCurrentArchType(const ArchTypeID& arch_type);
	void UpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
	void ClearArch(ArchTypeID arch_type);

	void CheckCrossSectionMaximizeAlone();
#ifndef WILL3D_VIEWER
	const int GetCrossSectionFilterLevel();
	void RequestedCreateCrossSectionDCMFiles(bool nerve_visible, bool implant_visible, int filter, int thickness);
#endif

	inline void set_single_cross_section_maximized(bool is_maximize) { single_cross_section_maximized_ = is_maximize; }

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

private slots:
	virtual void slotTranslateZfromPanoArch(float z_pos_vol) override;
	virtual void slotArchShifteFromPanoArch(float shifted_in_vol) override;
	virtual void slotArchUpdateFinishFromPanoArch() override;
	virtual void slotCrossSectionToolsChanged() override;

	void slotArchTypeChanged(const ArchTypeID& arch_type);
	void slotChangeEditArchMode();

	void slotArchDeleteFromPanoArch();

	void slotNerveAddFromPanoView(int nerve_id, const QPointF& pt_in_pano_plane);
	void slotNerveAddFromCrossView(int nerve_id, const QPointF& pt_in_cross_plane);
	void slotNerveFinishFromCrossView();
	void slotPressedKeyESCfromCrossView();
	void slotNerveFinishFromPanoView(int nerve_id);
	void slotNerveTranslateFromPanoView(int nerve_id, int nerve_selected_index, const QPointF& pt_in_pano_plane);
	void slotNerveTranslateFromCrossView(int cross_id, const QPointF& pt_in_cross_plane);
	void slotNerveClear(int nerve_id);
	void slotNerveCancelLastPoint(int nerve_id, int nerve_removed_index);
	void slotNerveRemovePoint(int nerve_id, int nerve_removed_index);
	void slotNerveInsertPoint(int nerve_id, int nerve_inserted_index, const QPointF& pt_in_pano_plane);
	void slotNerveModifyPoint(int nerve_id, int nerve_selected_index, bool is_modify);
	void slotNerveChangedValuesFromTool(const bool& nerve_visible, const int& nerve_id, const float& diameter, const QColor& color);
	void slotNerveDrawModeFromTool(bool is_draw_mode);

	void slotNerveHoveredFromTool(int nerve_id, bool is_hovered);
	void slotDeleteNerveFromTool(int nerve_id);

	void slotOrientationViewRotate(const ReorientViewID& view_type, const glm::mat4& orien);
	void slotOrientationFromTaskTool(const ReorientViewID& view_type, int angle);
	void slotOrientationViewRenderQuality(const ReorientViewID& view_type);
	void slotChangeROIFromOrienView();
	void slotUpdateDoneROIFromOrienView();
	void slotChangeSliceFromOrienView();

	void slotMaximizeSingleCrossSectionView();

private:
	virtual void InitializeViews() override;
	virtual void connections() override;

	virtual void InitCrossSectionResource() override;
	virtual void InitPanoramaROI() override;
	virtual void ChangedArchType(const ArchTypeID& type) override;
	virtual void UpdateSurfaceObjs() override;

	virtual void VisibleNerve(bool is_visible) override;

	void UpdatePanorama();
	void UpdatePanoViewNerveCtrlPoints();

private:
	class SaveOrienDegrees {
	public:
		int value_r = 0;
		int value_i = 0;
		int value_a = 0;

		const SaveOrienDegrees& operator=(const SaveOrienDegrees& params) {
			this->value_r = params.value_r;
			this->value_i = params.value_i;
			this->value_a = params.value_a;
			return *this;
		}
	};
	std::shared_ptr<ViewPanoArch> view_pano_arch_ = nullptr;
	std::shared_ptr<ViewPano> view_pano_ = nullptr;
	std::map<int, std::shared_ptr<ViewPanoCrossSection>> view_cross_section_;

	std::shared_ptr<PanoTaskTool> task_tool_;

	std::shared_ptr<ViewPanoOrientation> view_pano_orient_[ReorientViewID::REORI_VIEW_END];

	SaveOrienDegrees save_orien_degrees_;  // 이전 arch_type 의 각도

	bool single_cross_section_maximized_ = false;
};
