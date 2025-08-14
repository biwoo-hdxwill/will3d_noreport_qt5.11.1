#pragma once
/*=========================================================================

File:			class CW3MPRtab
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-12-02

=========================================================================*/
#include <memory>

#include <qboxlayout.h>

#include "../../Engine/Common/Common/w3_struct.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"
#include "base_tab.h"

class QKeyEvent;

class CW3JobMgr;
class CW3MPREngine;
class MPREngine;
class CW3ResourceContainer;
class CW3MPRViewMgr;
class CW3VREngine;
class MPRTaskTool;
class WindowMPR;
class WindowMPR3D;
class WindowLightbox;
class OrientationDlg;
class PanoEngine;
#ifndef WILL3D_VIEWER
class ProjectIOMPR;
#endif

class CW3MPRtab : public BaseTab
{
	Q_OBJECT

public:
	CW3MPRtab(CW3VREngine* VREngine, CW3MPREngine* MPRengine, CW3JobMgr* JobMgr, CW3ResourceContainer* Rcontainer);

	virtual ~CW3MPRtab(void);

	void activate(const float& pixel_spacing, const float& slice_spacing);
	virtual void SetVisibleWindows(bool isVisible) override;

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOMPR& out);
	void importProject(ProjectIOMPR& in);
#endif

	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void DeleteMeasureUI(const common::ViewTypeID& view_type, const unsigned int& measure_id) override;
	virtual void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type, const unsigned int& measure_id) override;

	void UpdateVRview(bool is_high_quality);

	void setOnlyTRDMode();
	void setMIP(bool bOn);

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;

	void ApplyPreferences();
	void SyncStatusMPRmenus();

	void SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine);
	void EmitSendMPRPlaneInfo(const MPRViewType mpr_view_type);
	const lightbox_resource::PlaneParams GetMPRPlaneParams(MPRViewType mpr_view_type);
#ifndef WILL3D_VIEWER
	void RequestedGetLightBoxViewInfo(int& filter, int& thickness) const;
#endif
#ifdef WILL3D_EUROPE
	void TaskSTLExportDialogOn();
	void LightboxOn();
#endif // WILL3D_EUROPE

private:
	enum class LayoutMode { DEFAULT = 0, MAXIMIZE, ONLY_TRD, LIGHTBOX };

	enum class MaximizedViewType
	{
		NONE,
		AXIAL,
		SAGITTAL,
		CORONAL,
		VR,
	};

signals:
	void sigMPRLightboxOn();

	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();

	void sigUpdateArch(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
	void sigSendMPRPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth);	
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public slots:
	void slotSecondDisabled(bool, float*);
	void slotSetTranslateMatSecondVolume(glm::mat4*);
	void slotSetRotateMatSecondVolume(glm::mat4*);
	void slotSetAirway(std::vector<tri_STL>&);
	void slotAirwayEnable(bool);
	void slotSetAirwaySize(double);
	void slotFaceEnabled();
	void slotPointModel(glm::mat4*);
#ifndef WILL3D_VIEWER
	void slotRequestedCreateLightBoxDCMFiles(const bool nerve_visible, const bool implant_visible, const int filter, const int thickness);
#endif // !WILL3D_VIEWER

private slots:
	void slotMaximizeOnOff(bool max);
	void slotChangeThickness(double);
	void slotChangeInterval(double interval);
	void slotSyncThicknessChanged(const MPRViewType&, float);

	void slotAutoReorient();
	void slotAdjustOrientationClicked();
	void slotMPRTask(const MPRTaskID&, bool);

	// slot functions for lightbox
	void slotLightboxTransltate(const int& lightbox_id, const int& slider_value);
	void slotLightboxTransltate(const float& displacement);
	void slotGetLightboxDICOMInfo(const int& lightbox_id, const QPointF& pt_lightbox_plane, glm::vec4& vol_info);
	void slotGetLightboxProfileData(const int& lightbox_id, const QPointF& start_pt_plane, const QPointF& end_pt_plane,	std::vector<short>& data);
	void slotGetLightboxROIData(const int& lightbox_id, const QPointF& start_pt_plane, const QPointF& end_pt_plane, std::vector<short>& data);
	void slotLightboxMaximize(const int& lightbox_id);
	void slotSetLightboxLayout(const int& row, const int& col);
	void slotSetLightboxOff();
	void slotChangeLightboxInterval(double real_interval);
	void slotChangeLightboxThickness(double real_thickness);
	// end of slot functions for lightbox

	void slotMPROverlayOn();
	void slotKeyPressEventFromView(QKeyEvent* event);	

private:
	virtual void Initialize() override;
	virtual void SetLayout() override;

	void SetLayoutTRD();
	void SetLayoutMaximize();
	void SetLayoutDefault();
	void SetLayoutLightbox();

	void SetVisibleWindowsTRD(bool visible);
	void SetVisibleWindowsDefaultAndMaximize(bool visible);
	void SetVisibleWindowsLightbox(bool visible);

	void connections();
	CW3MPRtab::MaximizedViewType GetMaximizedViewType();

	void TaskZoom3D(bool on);
	void TaskCut3D(bool on);
	void TaskOblique(bool on);
	void TaskSTLExport();
	void TaskDrawArch();

	void SetLightboxOn(const MPRViewType& view_type, int row, int col,
		float interval, float thickness);
	void SyncLightboxSliderPositions();
	void ConnectLightbox();
	void DisconnectLightbox();

#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

private:
	bool is_visible_ = false;

	CW3VREngine* m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3JobMgr* m_pgJobMgr = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3MPRViewMgr* m_pMPRViewMgr = nullptr;

	std::shared_ptr<MPRTaskTool> task_tool_;
	std::unique_ptr<MPREngine> mpr_engine_;

	std::unique_ptr<WindowMPR> window_axial_ = nullptr;
	std::unique_ptr<WindowMPR> window_sagittal_ = nullptr;
	std::unique_ptr<WindowMPR> window_coronal_ = nullptr;
	std::unique_ptr<WindowMPR3D> window_vr_ = nullptr;
	std::unique_ptr<WindowMPR3D> window_vr_zoom_3d_ = nullptr;
	WindowLightbox* window_lightbox_ = nullptr;

	std::unique_ptr<OrientationDlg> orientation_dlg_ = nullptr;

	QHBoxLayout* view_layout_ = nullptr;
	LayoutMode layout_mode_ = LayoutMode::DEFAULT;
	MaximizedViewType maximized_view_type_ = MaximizedViewType::NONE;

	bool hide_mpr_views_on_maximized_vr_layout_ = false;

	std::shared_ptr<PanoEngine> pano_engine_;

	QRect axial_rect_;
	QRect sagittal_rect_;
	QRect coronal_rect_;
	QRect vr_rect_;
	QRect vr_zoom_3d_rect_;
};
