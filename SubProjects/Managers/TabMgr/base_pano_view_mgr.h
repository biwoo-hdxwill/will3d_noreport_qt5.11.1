#pragma once
/**=================================================================================================

Project:		TabMgr
File:			base_pano_view_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-04
Last modify: 	2018-04-04

        Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "base_view_mgr.h"

#include "../../Engine/Common/Common/W3Enum.h"

class BaseViewPano;
class BaseViewPanoArch;
class BaseViewPanoCrossSection;
class PanoEngine;
class CW3BasePanoMenus;
class BasePanoTaskTool;

enum SharpenLevel;

class BasePanoViewMgr : public BaseViewMgr {
  Q_OBJECT
 public:
  explicit BasePanoViewMgr(QObject* parent = nullptr);
  virtual ~BasePanoViewMgr();

 public:
  virtual void SyncPanoResource();
  void SyncMeasureResource();

  void SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine);

  void SelectLayoutCrossSection(int row, int col);
  virtual void SetVisibleViews(bool bVisible);

  inline QWidget* GetViewPanoWidget() const {
    return (QWidget*)(base_view_pano_.get());
  }
  inline QWidget* GetViewPanoArchWidget() const {
    return (QWidget*)(base_view_pano_arch_.get());
  }
  std::vector<QWidget*> GetViewPanoCrossSectionWidget() const;

  void set_cross_section_count(const int& count) {
    cross_section_count_ = count;
  }

  virtual void ConnectPanoMenus();
  virtual void DisconnectPanoMenus();

  inline void set_init_arch_from_mpr(const bool value) { init_arch_from_mpr_ = value; }

signals:
  void sigSyncCrossSectionParams(const float, const float, const float);

 public slots:
  void slotAutoArch(const bool use_current_slice);
  void slotChangedArchTypeFromPanoArch(const ArchTypeID& type);

 protected slots:
  virtual void slotProcessedLightEvent();
  virtual void slotTranslateZfromPanoArch(float z_pos_vol);
  virtual void slotArchShifteFromPanoArch(float shifted_value_in_vol);
  void slotBasePanoVisible(const VisibleID& visible_id, bool is_visible);
  void slotChangedArchRangeFromPanoArch(float range_mm);
  void slotChangePanoArchRange(double value);
  void slotChangePanoThickness(double value);
  void slotRequestInitializeFromPanoArch();
  virtual void slotArchUpdateFinishFromPanoArch();
  virtual void slotSetAxialSliceFromPanoView(const QPointF& pt_pano_plane);
  virtual void slotReconTypeChangedFromPano();
  virtual void slotCrossSectionToolsChanged();

  void slotImplantHoveredFromPano(const QPointF& pt_pano_plane,
                                  int* hovered_id) const;
 protected:
  virtual void InitializeViews() = 0;
  virtual void connections();
  virtual void ChangedArchType(const ArchTypeID& type) {}

  void SetAxialZposition(const glm::vec3& vol);
  void InitPanoResource();
  virtual void InitCrossSectionResource();
  void UpdateAxialLine();
  void MoveAxialLineToPanoViewCenter();
  void MoveAxialLineToCrossSectionViewCenter();
  virtual void InitPanoramaROI();
  void InitAxialLinePanoView();
  void InitArchLinePanoView();
  void InitAxialLineCrossView();

  void GeneratePanorama();

  void ReconAllResources();
  void ReconPanoResource();
  void ReconNerveResource();
  virtual void ReconImplantResource();
  virtual void UpdateSurfaceObjs();

  void UpdateCrossSection(bool is_set_view_pano = true);

  virtual void VisibleNerve(bool is_visible);
  virtual void VisibleImplant(bool is_visible);

  bool IsPanoReconMPR() const;
  bool IsSetPanoArch() const;
  virtual void ApplyPreferences() override;

  void DeleteMeasuresInPanorama();

  void MoveArchViewToSelectedMeasure(
      const common::measure::VisibilityParams& visibility_param);
  void MovePanoViewToSelectedMeasure(
      const common::measure::VisibilityParams& visibility_param);
  void MoveCrossSectionViewToSelectedMeasure(
      const common::measure::VisibilityParams& visibility_param);

  void set_base_view_pano(const std::shared_ptr<BaseViewPano>& base_view_pano);
  void set_base_view_pano_arch(
      const std::shared_ptr<BaseViewPanoArch>& base_view_pano_arch);
  void set_base_view_pano_cross_section(
      int id, const std::shared_ptr<BaseViewPanoCrossSection>&
                  base_view_cross_sections);

  void set_task_tool_base(const std::shared_ptr<BasePanoTaskTool>& task_tool);

  inline PanoEngine* pano_engine() const { return pano_engine_.get(); }

  inline int cross_section_count() const { return cross_section_count_; }

 private slots:
  void slotCrossSectionRotate(float delta_angle);
  void slotCrossTranslateFromPanoView(const float& shifted);
  void slotCrossTranslateFromCrossView(int delta);
  void slotCrossSectionLineReleased();
  void slotCrossViewProcessedZoomEvent(float scale);
  void slotCrossViewProcessedPanEvent(const QPointF& translate);
  void slotCrossViewHovered(bool is_hovered);
  void slotCrossViewSharpen(SharpenLevel level);
  void slotCrossMeasureCreated(const int& cross_section_id,
                               const unsigned int& measure_id);
  void slotCrossMeasureDeleted(const int& cross_section_id,
                               const unsigned int& measure_id);
  void slotCrossMeasureModified(const int& cross_section_id,
                                const unsigned int& measure_id);
  void slotCrossMeasureUpdateNeeded(const int& cross_section_id,
                                    bool& is_need_update);

  void slotSetAxialSliceFromCrossView(int cross_id, const glm::vec3& pt_vol);
  void slotArchUpdateFromPanoArch();

  void slotTranslateZfromPano(float z_delta);
  void slotReconPanoResource();

  void slotBasePanoClipParamsChanged(const std::vector<glm::vec4>&, bool);

  void slotDisplayDICOMInfoFromPanoView(const QPointF& pt_pano_plane,
                                        glm::vec4& vol_info);

  void slotSetProfileDataFromPanoView(const QPointF& start_plane_pos,
                                      const QPointF& end_plane_pos,
                                      std::vector<short>& data);

  void slotSetROIDataFromPanoView(const QPointF& start_plane_pos,
                                  const QPointF& end_plane_pos,
                                  std::vector<short>& data);
  
  void slotSetCrossSectionViewStatus(float* scene_scale,
                                     QPointF* gl_translate);
  void slotCrossViewZoomDoneEvent();
  void slotPanoViewZoomDoneEvent();

  void slotToggleImplantRenderingType();

 private:
  void SetViewPanoSliceRange();
  void SetCrossSectionParams();

  void GetCrossSectionParamsFromTools(float* interval, int* width, int* height,
                                      float* thickness, float* degree);
  void GetPanoPoints(std::vector<glm::vec3>& points) const;
  void GetPanoCtrlPoints(std::vector<glm::vec3>& ctrl_points) const;
  void GetPanoTopPoints(const std::vector<glm::vec3>& vol_arch_points,
                        std::vector<glm::vec3>& points) const;
  void GetPanoBackVector(glm::vec3& back_vector) const;
  void GetPanoRange(float& depth, float& range) const;
  void GetPanoShifted(float& shifted) const;
  void GetPanoThickness(float& thickness) const;

protected:
	  std::shared_ptr<BasePanoTaskTool> task_tool_base_;

	  bool init_arch_from_mpr_ = false;

 private:
  std::shared_ptr<BaseViewPano> base_view_pano_ = nullptr;
  std::shared_ptr<BaseViewPanoArch> base_view_pano_arch_ = nullptr;
  std::map<int, std::shared_ptr<BaseViewPanoCrossSection>>
      base_view_cross_sections_;

  std::shared_ptr<CW3BasePanoMenus> base_pano_menus_;

  int cross_section_count_ = 0;
  std::shared_ptr<PanoEngine> pano_engine_;
};
