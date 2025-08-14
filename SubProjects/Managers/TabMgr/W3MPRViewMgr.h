#pragma once
/*=========================================================================

File:			class CW3MPRViewMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-26
Last modify:	2015-11-26

=========================================================================*/
#include <memory>
#include <vector>

#include <qvector2d.h>
#include <qwidget.h>

#include "../../Engine/Common/Common/w3_struct.h"
#include "../../Engine/Common/Common/define_view.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"

class CW3ViewMPR;
class CW3View3DMPR;
class CW3MPREngine;
class CW3VREngine;
class CW3ResourceContainer;
class ViewLightbox;
class MPRTaskTool;
class ViewMPROrientation;
class PanoEngine;
#ifndef WILL3D_VIEWER
class ProjectIOMPR;
#endif

typedef struct _STL_TRI_SE tri_STL;

namespace common
{
	namespace measure
	{
		struct VisibilityParams;
	}  // end of namespace measure
}  // end of namespace common

#define kRegistrationParamCnt 6

class CW3MPRViewMgr : public QWidget
{
	Q_OBJECT

signals :
	void sigThicknessChanged(const MPRViewType&, float thickness);

	// Lightbox signals
	void sigLightboxTranslate(const int& lightbox_id, const int& slider_value);
	void sigLightboxTranslate(const float& displacement);
	void sigGetLightboxDICOMInfo(const int& lightbox_id,
		const QPointF& pt_lightbox_plane,
		glm::vec4& vol_info);
	void sigGetLightboxProfileData(const int& lightbox_id,
		const QPointF& start_pt_plane,
		const QPointF& end_pt_plane,
		std::vector<short>& data);
	void sigGetLightboxROIData(const int& lightbox_id,
		const QPointF& start_pt_plane,
		const QPointF& end_pt_plane,
		std::vector<short>& data);
	void sigLightboxMaximize(const int& lightbox_id);

	void sigMPROverlayOn();

	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();

	void sigTranslateImplant(const int& implant_id, const glm::vec3& delta_trans);
	void sigRotateImplant(const int& implant_id, const glm::vec3& rotate_axes, const float& delta_degree);

	void sigUpdateArch(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
	void sigSendMPRPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigRequestCreateDCMFilesToLightbox(bool nerve_visible, bool implant_visible);

#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public:
	CW3MPRViewMgr(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
		CW3ResourceContainer* Rcontainer, QWidget* parent = 0);

	~CW3MPRViewMgr(void);
	void set_task_tool(const std::weak_ptr<MPRTaskTool>& tool);

	void reset();
	void activate();
	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);
	void UpdateVRview(bool is_high_quality);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOMPR& out);
	void importProject(ProjectIOMPR& in);
	void RequestedCreateLightBoxDCMFiles(const bool nerve_visible, const bool implant_visible, const int filter, const int thickness);
	const int GetLightBoxFilterValue() const;
#endif

	inline QWidget* GetViewMPR(int i) { return (QWidget*)m_pViewMPR[i].get(); }
	inline QWidget* GetViewVR() { return (QWidget*)m_pView3DMPR.get(); }
	inline QWidget* GetViewZoom3D() { return (QWidget*)m_pViewZoom3D.get(); }
	std::vector<QWidget*> GetViewLightbox() const;
	QWidget* GetViewLightboxMaximize(const int& lightbox_id) const;

	void secondDisabled(bool, float*);
	void setTranslateMatSecondVolume(glm::mat4* mat);
	void setRotateMatSecondVolume(glm::mat4* mat);
	void SetAirway(std::vector<tri_STL>& tries);
	void pointModel(glm::mat4* mat);

	void resetZoomView();
	void setMaximizeVRView(bool bMaximize);
	void setVisibleMPRView(MPRViewType eType, bool bVisible);
	void setVisibleLightboxViews(bool visible);
	void setVisibleZoomView(bool bVisible);
	void setMIP(bool bMIP);
	void setOnlyTRDMode();
	void updateImageOnViewPlane(MPRViewType eType);

	void ChangeThicknessDirect(MPRViewType view_type, float real_thickness);
	void ChangeIntervalDirect(MPRViewType view_type, float real_interval);

	void TaskOblique(bool bToggled);
	void TaskSTLExport(bool clip_on);
	void Task3DCut(bool on, VRCutTool cut_tool);
	void TaskZoom3D(bool);
	void TaskDrawArch();

	// lightbox functions
	void CreateLightboxViews(const LightboxViewType& view_type);
	void DeleteLightboxViews();
	void GetLightboxPlaneParams(const MPRViewType& view_type,
		lightbox_resource::PlaneParams& plane_params);
	glm::vec3 GetLightboxPlaneCenter(const MPRViewType& view_type);
	void UpdateLightboxViews();
	void InitLightboxSlider(const int& row, const int& col,
		const std::vector<int>& slider_positions,
		const int& available_depth);
	void SyncLightboxSliderPositions(const std::vector<int>& slider_positions);
	void SyncLightboxMeasureResource();

	void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);
	void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);
	void ApplyPreferences();
	
	inline QWidget* GetViewOrientation(const ReorientViewID& id) const
	{
		return (QWidget*)(view_orientation_[id].get());
	}

	void SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine);
	void EmitSendMPRPlaneInfo(const MPRViewType mpr_view_type);
	const lightbox_resource::PlaneParams GetMPRPlaneParams(MPRViewType mpr_view_type);	

	void InitMPRItems(MPRViewType eType);

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
	MPRViewType GetMouseOverMPR();
#endif // WILL3D_EUROPE

public slots:
	void slotRotateMatrix(MPRViewType, glm::mat4*);
	void slotRotate(UILineType, MPRViewType, const glm::vec3&, float);
	void slotTranslate(MPRViewType, MPRViewType, float);
	void slotTranslateCross(MPRViewType, QVector2D&);
	void slotThickChange(MPRViewType, MPRViewType, float);
	void slotSetZoom3DMPR(const MPRViewType eViewType, const QPointF center,
		const float radius, const bool updateVR);
	void slotSelectedMPR(MPRViewType view_type, const bool update_3d_view = true);
	void slotUpdateMPRPhoto(void);

	void slotReorientReset();
	void slotGridOnOffOrientation(bool on);
	void slotMPRVisible(const VisibleID&, int);
	void slotMPRChangeFaceTransparency(int);
	void slotMPRClipEnable(int);
	void slotMPRClipRangeMove(int, int);
	void slotMPRClipRangeSet();
	void slotMPRClipPlaneChanged(const MPRClipID&);
	void slotFlipClipping(int state);

	void slot3DCutReset();
	void slot3DCutUndo();
	void slot3DCutRedo();

	void slotRotateOrientationFromTaskTool(const ReorientViewID& view_type, int angle);
	void slotAdjustImplantButtonToggled(bool checked);

	void slotUpdateMPROverlay();	

private:
	void connections();
	bool STLExport(const QString& path, int threshold);
	void ConnectLightbox();

	void GetViewsVisibilityParams(
		const common::ViewTypeID& view_type, const unsigned int& measure_id,
		common::measure::VisibilityParams* visibility_params);
	void MoveMPRViewToSelectedMeasure(
		const MPRViewType& view_type, const unsigned int& measure_id,
		const common::measure::VisibilityParams& visibility_param);
	void MoveLightboxViewToSelectedMeasure(
		const common::measure::VisibilityParams& visibility_param);

private slots:
	void slotReoriUpdateVR(glm::mat4* mat);
	void slotReoriUpdateMPR(glm::mat4* mat);
	void slotSecondUpdate(glm::mat4*);
	void slotSetZoom3DVR(const MPRViewType, const glm::vec3, const float);
	void slotTFUpdateCompleted();
	void slotWheel(MPRViewType view_type, float delta, const bool update_3d_view = true);
	void slotChangeSTLExportThreshold(double threshold);
	void slotSetMPRClipStatus();

	void slotMPRMeasureCreated(const unsigned int& measure_id);
	void slotMPRMeasureDeleted(const unsigned int& measure_id);

	// Lightbox slot functions
	void slotLightboxTranslate(const int& lightbox_id, const int& slider_value);
	void slotLightboxWindowing();
	void slotLightboxZoom(float scene_scale);
	void slotLightboxPan(const QPointF& trans);
	void slotLightboxViewSharpen(const SharpenLevel& sharpen_level);
	void slotLightboxMaximize(const int& lightbox_id);
	void slotSetLightboxViewStatus(float* scene_scale, QPointF* gl_translate);
	void slotLightboxMeasureCreated(const int& lightbox_id,
		const unsigned int& measure_id);
	void slotLightboxMeasureDeleted(const int& lightbox_id,
		const unsigned int& measure_id);
	void slotLightboxMeasureModified(const int& lightbox_id,
		const unsigned int& measure_id);

	void slotRotateOrientationView(const ReorientViewID& view_type, const glm::mat4& orien);
	void slotSetOrientationViewRenderQuality(const ReorientViewID& view_type);

	void slotSelectImplant(const int id);
	void slotTranslateImplant(const int& implant_id, const glm::vec3& delta_trans);
	void slotRotateImplant(const int& implant_id, const glm::vec3& rotate_axes, const float& delta_degree);
	void slotDoneAdjustImplant();

	void slotVolumeClicked(const glm::vec3 gl_pos);

private:
	struct MPRTansfromStatusForMeasure
	{
		glm::mat4 axial;
		glm::mat4 sagittal;
		glm::mat4 coronal;
		glm::vec3 rotate_center;
		float axial_angle_degree;
		float sagittal_angle_degree;
		float coronal_angle_degree;
	};

private:
	std::weak_ptr<MPRTaskTool> task_tool_;
	std::shared_ptr<CW3ViewMPR> m_pViewMPR[MPRViewType::MPR_END];
	std::shared_ptr<CW3View3DMPR> m_pView3DMPR;
	std::shared_ptr<CW3View3DMPR> m_pViewZoom3D;

	std::shared_ptr<ViewMPROrientation> view_orientation_[ReorientViewID::REORI_VIEW_END];

	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3VREngine* m_pgVRengine = nullptr;

	int lightbox_count_ = 0;
	std::vector<ViewLightbox*> view_lightbox_;
	bool is_draw_nerve_ = false;
	bool is_draw_implant_ = false;

	bool m_isInitialized_ = false;
	bool is_visible_canal_ = false;

	float m_RegiPointsSol[kRegistrationParamCnt];
	std::map<unsigned int, MPRTansfromStatusForMeasure> mpr_transform_measure_;

	MPRViewType current_mpr_overlay_plane_ = MPRViewType::AXIAL;

	std::shared_ptr<PanoEngine> pano_engine_;
};
