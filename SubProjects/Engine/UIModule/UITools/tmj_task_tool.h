#pragma once
/**=================================================================================================

Project: 			UITools
File:				tmj_task_tool.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-03
Last modify:		2018-10-03

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include <Engine/Common/Common/W3Enum.h>
#include "base_tool.h"
#include "orientation_tool.h"
#include "uitools_global.h"

class QButtonGroup;
class QCheckBox;
class QDoubleSpinBox;
class QRadioButton;
class QToolButton;
class QFrame;
class QWidget;
class QVBoxLayout;

class ToolBox;
class TextEdit;
class ClippingTool;
#ifndef WILL3D_VIEWER
class ProjectIOTMJ;
#endif

class UITOOLS_EXPORT TMJTaskTool : public BaseTool {
  Q_OBJECT
 public:
  explicit TMJTaskTool(QObject* parent = nullptr);
  virtual ~TMJTaskTool();

  TMJTaskTool(const TMJTaskTool&) = delete;
  TMJTaskTool& operator=(const TMJTaskTool&) = delete;

 public:
  enum Mode { MODE_2D = 0, MODE_3D, MODE_END };

  enum ClipID { AP = 0, LM, TB, CLIP_END };

  struct LateralUI {
    QDoubleSpinBox* left_interval = nullptr;
    QDoubleSpinBox* left_thickness = nullptr;
    QDoubleSpinBox* right_interval = nullptr;
    QDoubleSpinBox* right_thickness = nullptr;
  };

  struct FrontalUI {
    QDoubleSpinBox* left_width = nullptr;
    QDoubleSpinBox* left_height = nullptr;

    QDoubleSpinBox* right_width = nullptr;
    QDoubleSpinBox* right_height = nullptr;
  };

  struct CutControlUI {
    QToolButton* reset = nullptr;
    QToolButton* undo = nullptr;
    QToolButton* redo = nullptr;
  };
  
 signals:
  // 3D 모드일 때 ToolMgr에서 OTF Tool Widget을 task_contents_layout_ 에
  // 추가하기 위해 호출하는 signal
  void sigTMJGetTaskLayout(QVBoxLayout* layout);
  void sigTMJReorient();
  void sigTMJReorientReset();
  void sigTMJOrientRotate(const ReorientViewID&, int);

  void sigTMJLaoutChanged(const TmjLayoutType& layout_type);
  void sigTMJDrawRect(const TMJDirectionType& direction_type, bool draw_on);
  void sigTMJDeleteRect(const TMJDirectionType& direction_type);

  void sigTMJRectChanged(const TMJRectID& roi_id, double value);
  void sigTMJLateralParamChanged(const TMJLateralID& id, double value);

  void sigTMJCutEnable(const bool& cut_on, const VRCutTool& cut_tool);
  void sigTMJCutReset(const TMJDirectionType& direction_type);
  void sigTMJCutUndo(const TMJDirectionType& direction_type);
  void sigTMJCutRedo(const TMJDirectionType& direction_type);

  void sigTMJClipParamsChanged(const ClipID& clip_id,
                               const std::vector<float>& values,
                               bool clip_enable);

 public:
#ifndef WILL3D_VIEWER
  void ExportProject(ProjectIOTMJ& out);
  void ImportProject(ProjectIOTMJ& in);
#endif

  virtual void ResetUI() override;
  virtual void Connections() override;

  void UpdateTMJRect(const TMJRectID& id, const double& value);
  void UpdateLateralParam(const TMJLateralID& id, const double& value);
  void InitExternUIs(const OrientationTool::OrientUI& orient_ui,
                     const LateralUI& lateral_ui, const FrontalUI& frontal_ui,
                     const CutControlUI& cut_right,
                     const CutControlUI& cut_left);
  void ResetOrientDegreesUI();
  int GetOrientDegree(const ReorientViewID& view_type) const;
  void SetOrientDegrees(const int& degree_a, const int& degree_r,
                        const int& degree_i);
  void SyncOrientDegreeUIOnly(const ReorientViewID& view_type,
                              const int& degree_view);

  void DrawRectDone(const TMJDirectionType& type);

  float GetLateralParam(const TMJLateralID& id);
  float GetTMJRectparam(const TMJRectID& id);
  QWidget* GetOrientationWidget();
  QWidget* GetModeSelectionWidget();
  QWidget* GetTaskWidget();

  QWidget* GetTMJRectWidget();
  QWidget* GetTMJ3DTaskWidget();
  QWidget* GetTMJ2DTaskWidget();
  QWidget* GetMemoWidget();
  QVBoxLayout* GetTMJTaskLayout();
  inline const TMJTaskTool::Mode& mode() const noexcept { return mode_; }

  void SetFrontalWidthLowerBound(const TMJDirectionType& direction_type,
                                 const int& slice_count);
  void SyncFrontalWidth(const TMJDirectionType& direction_type,
                        const float& width);
  void SyncLateralWidth(const TMJDirectionType& direction_type,
                        const float& height);

  const bool IsCutEnabled();

 private:
  enum Task2D {
    TMJ_LAYOUT,
    LATERAL_MAIN_LAYOUT,
    FRONTAL_MAIN_LAYOUT,
    TASK_2D_END
  };

  enum CutType { CUT_FREEDRAW, CUT_POLYGON, CUT_END };

 private slots:
  void slotCutEnable(int state);
  void slotCutToolChanged();
  void slotSelectLayout();
  void slotSelectModeUI(const Mode& mode, bool checked);
  void slotRectDraw(const TMJDirectionType& type, const bool& checked);
  void slotRectDelete(const TMJDirectionType& type);
  void slotClipStatusChanged();

 private:
  virtual void CreateUI() override;
  virtual void SetToolTips() override;
  void SelectModeUI(const Mode& mode, const bool& checked);
  void SetEnableCutUI(const bool& is_enable);
  TmjLayoutType GetLayoutType() const;
  void LateralIntervalChanged(const TMJLateralID& lateral_id,
                              double slider_value);
  void CounterpartDrawModeOff(const TMJDirectionType& direction);

 private:
  Mode mode_ = Mode::MODE_2D;

  std::unique_ptr<QFrame> task_contents_;
  std::unique_ptr<QVBoxLayout> task_contents_layout_;

  std::unique_ptr<ToolBox> tmp_tool_box_;

  std::unique_ptr<QFrame> mode_selection_tool_box_;
  std::unique_ptr<ToolBox> memo_tool_box_;
  std::unique_ptr<ToolBox> tmj_rect_tool_box_;
  std::unique_ptr<ToolBox> task_2d_tool_box_;
  std::unique_ptr<ToolBox> task_3d_tool_box_;

  std::unique_ptr<QButtonGroup> cut_group_;

  std::unique_ptr<ClippingTool> clip_tool_;
  std::unique_ptr<OrientationTool> orient_tool_;
  std::unique_ptr<QToolButton> mode_selection_[Mode::MODE_END];
  std::unique_ptr<QToolButton> tmj_rect_draw_[TMJDirectionType::TMJ_TYPE_END];
  std::unique_ptr<QToolButton> tmj_rect_delete_[TMJDirectionType::TMJ_TYPE_END];
  std::unique_ptr<QRadioButton> task_2d_[Task2D::TASK_2D_END];
  std::unique_ptr<TextEdit> tmj_memo_;

  std::unique_ptr<QCheckBox> cut_activate_;
  std::unique_ptr<QRadioButton> cut_tool_[CutType::CUT_END];

  //  외부(Tab)에서 생성되어 포인터만 세팅되는 UI들
  QDoubleSpinBox* tmj_rect_[TMJRectID::TMJ_RECT_END];
  QDoubleSpinBox* lateral_[TMJLateralID::TMJ_LATERAL_END];  
  CutControlUI cut_control_[TMJDirectionType::TMJ_TYPE_END];
};
