#pragma once
/**=================================================================================================

Project: 			UITools
File:				base_pano_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-20
Last modify:		2018-09-20

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <Engine/Common/Common/W3Enum.h>

#include "base_tool.h"
#include "uitools_global.h"

class QDoubleSpinBox;
class ClippingTool;
class VisibilityToolBox;

class UITOOLS_EXPORT BasePanoTaskTool : public BaseTool {
  Q_OBJECT

 public:
  explicit BasePanoTaskTool(QObject* parent = nullptr);
  virtual ~BasePanoTaskTool();

  BasePanoTaskTool(const BasePanoTaskTool&) = delete;
  BasePanoTaskTool& operator=(const BasePanoTaskTool&) = delete;

 public:
  struct CrossSectionUI {
    QDoubleSpinBox* thickness = nullptr;
    QDoubleSpinBox* interval = nullptr;
    QDoubleSpinBox* angle = nullptr;
  };

  struct PanoUI {
    QDoubleSpinBox* arch_range = nullptr;
    QDoubleSpinBox* thickness = nullptr;
  };

 signals:
  void sigBasePanoVisible(const VisibleID&, bool);
  void sigBasePanoClipParamsChanged(const std::vector<glm::vec4>&, bool);
  void sigBasePanoCSParamsChanged();
  void sigBasePanoArchRangeChanged(double value);
  void sigBasePanoArchThicknessChanged(double value);

 public:
  void InitExternUIs(const CrossSectionUI& cs_ui, const PanoUI& pano_ui);
  virtual void ResetUI() override;

  void SetActivateClippingTools(bool activate);
  void SetCrossSectionInterval(double interval);
  void SetCrossSectionThickness(float thickness);
  void UpdateCrossSectionAngle(float delta_angle);
  void SetCrossSectionAngle(float angle);
  float GetCrossSectionAngle();
  float GetCrossSectionInterval();
  float GetCrossSectionThickness();
  float GetPanoRange();

  bool IsVisibleImplantEnable() const;
  bool IsVisibleImplantChecked() const;

  void SetArchRange(float range_mm);
  void SetArchThickness(float thickness_mm);
  void SetPreviousArchThickness(float previous_thickness_mm);
  float GetArchRange();
  float GetArchThickness();
  float GetPreviousArchThickness();

  void SyncVisibilityResources();

  QWidget* GetClipWidget();
  QWidget* GetVisibilityWidget();

  virtual void Connections() override;
  void Disconnections();

 private:
  virtual void CreateUI() override;
  virtual void SetToolTips() override;

 private slots:
  void slotClipStatusChanged();
  void slotVisible(const VisibleID& visible_id, int state);
  void slotCSIntervalChanged(double slider_value);

 private:
  enum ClipID { LR = 0, TB, BL, CLIP_END };

 private:
  std::unique_ptr<ClippingTool> clip_tool_;
  std::unique_ptr<VisibilityToolBox> visibility_tool_;

  CrossSectionUI cs_;  // 외부(Tab)에서 생성되어 포인터만 세팅됨
  PanoUI pano_;        // 외부(Tab)에서 생성되어 포인터만 세팅됨

  float previous_arch_thickness_mm_ = 0.0f;
};
