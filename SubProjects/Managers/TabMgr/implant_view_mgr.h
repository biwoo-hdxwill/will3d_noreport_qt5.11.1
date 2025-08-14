#pragma once
/**=================================================================================================

Project:		TabMgr
File:			implant_view_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-10
Last modify: 	2018-04-10

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "base_pano_view_mgr.h"

class ViewImplantArch;
class ViewImplantPano;
class ViewImplantSagittal;
class ViewImplant3D;
class ViewImplantCrossSection;
class ViewBoneDensity;
class ImplantTaskTool;
#ifndef WILL3D_VIEWER
class ProjectIOImplant;
#endif

class ImplantViewMgr : public BasePanoViewMgr {
  Q_OBJECT

 public:
  explicit ImplantViewMgr(QObject* parent = nullptr);
  ~ImplantViewMgr(void);

 signals:
  void sigImplantPlaced();
  void sigImplantSelectionChanged(int implant_id);
  void sigDeleteImplantFromView(int implant_id);
  void sigSyncBDViewStatus();
  void sigSyncArchType();
  void sigMaximizeSingleCrossSectionView(bool);

 public:
  // serialization
#ifndef WILL3D_VIEWER
  void exportProject(ProjectIOImplant& out);
  void importProject(ProjectIOImplant& in, const bool& is_counterpart_exists);
#endif

  // common menu bar와 연결되는 동작들

  inline QWidget* GetViewSagittalWidget() const {
	return (QWidget*)(view_sagittal_.get());
  }
  inline QWidget* GetView3DWidget() const { return (QWidget*)(view_3d_.get()); }
  inline QWidget* GetViewBoneDensity() const {
	return (QWidget*)(view_bd_.get());
  }
  void SetImplantMenu(const std::shared_ptr<ImplantTaskTool>& implant_menu);
  virtual void SetVisibleViews(bool visible) override;

  void SetImplantSelectedStatusFromTab(int implant_id, bool selected);
  void DeleteImplantFromTab(int implant_id);
  void DeleteAllImplantsFromTab();
  void ChangeImplantModelFromTab(int implant_id);
  void UpdateSceneAllViews();

  void ApplyPreferences();
  bool tmpIsRenderPano2D() const;

  virtual void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
										  const unsigned int& measure_id) override;
  void MoveSagittalViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param);

  virtual void ConnectPanoMenus() override;
  virtual void DisconnectPanoMenus() override;

  void Clip3DOnOff(const bool clip_on);
  void CheckCrossSectionMaximizeAlone();
  inline void set_single_cross_section_maximized(bool is_maximize) { single_cross_section_maximized_ = is_maximize; }

#ifdef WILL3D_EUROPE
  void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

 private slots:
  virtual void slotProcessedLightEvent() override;

  virtual void slotArchUpdateFinishFromPanoArch() override;
  virtual void slotTranslateZfromPanoArch(float z_pos_vol) override;
  virtual void slotArchShifteFromPanoArch(float shifted_value_in_vol) override;
  void slotGetPanoPosition(const glm::vec3& vol_pos);
  void slotSelectImplantFromArch(int implant_id);
  void slotTranslateImplantFromArch(int implant_id, const glm::vec3& vol_pos);
  void slotRotateImplantFromArch(int implant_id, const glm::vec3& axis,
								 float delta_degree);
  void slotUpdateAllImplantImagesFromArch(int implant_id,
										  const glm::vec3& vol_pos);
  void slotSetImplantPositionFromCross(const int implant_id, const glm::vec3& vol_pos);
  void slotTranslateImplantFromCross(int implant_id, const glm::vec3& vol_pos);
  void slotRotateImplantFromCross(int implant_id, const glm::vec3& axis,
								  float delta_degree);
  void slotUpdateAllImplantImagesFromCross(int implant_id,
										   const glm::vec3& vol_pos);
  void slotSelectImplantFromCross(int implant_id, int cross_section_id);

  virtual void slotSetAxialSliceFromPanoView(
	  const QPointF& pt_pano_plane) override;

  void slotRotateSagittal(float);
  void slotChangedSagittalRotateAngle(double);
  void slotSetAxialSliceFromSagittalView(const glm::vec3& pt_vol);
  void slotImplantHoveredFromSagittal(const QPointF& pt_sagittal_plane,
									  int* hovered_id) const;
  void slotSetImplantPlanePos(int implant_id, QPointF& pt_sagittal_plane);
  void slotSelectImplantFromSagittal(int implant_id);
  void slotTranslateImplantFromSagittal(int implant_id,
										const QPointF& pt_sagittal_plane);
  void slotRotateImplantFromSagittal(int implant_id, float delta_degree);
  void slotUpdateAllImplantImagesFromSagittal(int implant_id,
											  const QPointF& pt_sagittal_plane);

  void slotSagittalViewZoonDoneEvent();

  virtual void slotReconTypeChangedFromPano() override;
  void slotSelectImplantFromPano(int implant_id);
  void slotUpdateAllImplantImagesFromPano(int implant_id,
										  const QPointF& pt_pano_plane);
  void slotUpdateAllImplantImagesFromPano3D(int implant_id);
  void slotTranslateImplantFromPano(int implant_id,
									const QPointF& pt_pano_plane);
  void slotRotateImplantFromPano(int implant_id, float delta_degree);
  void slotTranslateImplantFromPano3D(const int& implant_id,
									  const glm::vec3& translate);
  void slotRotateImplantFromPano3D(const int& implant_id,
								   const glm::vec3& rotate_axes,
								   const float& rotate_degree);
  void slotPlacedImplantFromPano();
  void slotPlacedImplantFromCross();

  void slotSelectImplantFrom3D(int implant_id);
  void slotUpdateAllImplantImagesFrom3D(int implant_id);
  void slotRotatedView3D();
  void slotRotatedBoneDensityView();
  void slotTranslateImplantFrom3D(const int& implant_id,
								  const glm::vec3& translate);
  void slotRotateImplantFrom3D(const int& implant_id,
							   const glm::vec3& rotate_axes,
							   const float& rotate_degree);

  void slotImplantHoveredFromCS(const int& cross_section_id,
								const QPointF& pt_cs_plane,
								int& hovered_implant_id,
								QPointF& implant_pos_in_scene);

  void slotImplantBDSyncPopupStatus(bool popup);
  void slotMaximizeSingleCrossSectionView();

 private:
  virtual void InitializeViews() override;
  virtual void connections() override;
  virtual void ChangedArchType(const ArchTypeID& type) override;
  virtual void UpdateSurfaceObjs() override;

  virtual void VisibleNerve(bool is_visible) override;
  virtual void VisibleImplant(bool is_visible) override;

  void InitSagittalResource();
  void InitAxialLineInSagittalView();
  void InitSagittalParams();
  void UpdateSagittal(const glm::vec3& vol_pt);
  void SetSagittalLineInArchView(const glm::vec3& vol_pt,
								 const QPointF& pano_pos);
  void GetSagittalParamsFromTools(int* width, int* height, float* degree);

  void MoveAllViewPlanesByImplant(int implant_id);

  void MovePanoramaPlaneByImplant(const int& implant_id);
  void MoveArchPlaneByImplant(const int& implant_id);
  void MoveSagittalPlaneByImplant(const int& implant_id);
  void MoveCrossSectionByImplant(const int& implant_id, const int& cross_section_id = -1);

 private:
  std::shared_ptr<ViewBoneDensity> view_bd_ = nullptr;
  std::shared_ptr<ViewImplantSagittal> view_sagittal_ = nullptr;
  std::shared_ptr<ViewImplant3D> view_3d_ = nullptr;
  std::shared_ptr<ViewImplantArch> view_arch_;
  std::shared_ptr<ViewImplantPano> view_pano_;
  std::map<int, std::shared_ptr<ViewImplantCrossSection>> view_cross_section_;

  std::shared_ptr<ImplantTaskTool> task_tool_;

  bool single_cross_section_maximized_ = false;
};
