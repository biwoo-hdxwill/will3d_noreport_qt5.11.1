#include "base_pano_tool.h"
#include <QBoxLayout>
#include <QSpinBox>
#include <QToolButton>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/implant_resource.h>
#include <Engine/Resource/Resource/nerve_resource.h>

#include "clipping_tool.h"
#include "tool_box.h"
#include "visibility_tool_box.h"

BasePanoTaskTool::BasePanoTaskTool(QObject* parent) : BaseTool(parent) {
  BasePanoTaskTool::CreateUI();
  BasePanoTaskTool::SetToolTips();
}

BasePanoTaskTool::~BasePanoTaskTool() {}

void BasePanoTaskTool::InitExternUIs(const CrossSectionUI& cs_ui,
                                     const PanoUI& pano_ui) {
  cs_.thickness = cs_ui.thickness;
  cs_.interval = cs_ui.interval;
  cs_.angle = cs_ui.angle;

  pano_.arch_range = pano_ui.arch_range;
  pano_.thickness = pano_ui.thickness;
}

void BasePanoTaskTool::ResetUI() { clip_tool_->ResetUI(); }

void BasePanoTaskTool::SetActivateClippingTools(bool activate) {
  clip_tool_->SetEnable(activate);
}

void BasePanoTaskTool::SetCrossSectionInterval(double interval) {
  cs_.interval->setValue(interval);
}

void BasePanoTaskTool::SetCrossSectionThickness(float thickness)
{
	cs_.thickness->setValue(thickness);
}

void BasePanoTaskTool::UpdateCrossSectionAngle(float delta_angle) {
  double angle = cs_.angle->value();
  cs_.angle->setValue(angle + delta_angle);
}

void BasePanoTaskTool::SetCrossSectionAngle(float angle)
{
	cs_.angle->setValue(angle);
}

float BasePanoTaskTool::GetCrossSectionAngle() { return cs_.angle->value(); }

float BasePanoTaskTool::GetCrossSectionInterval() {
  return cs_.interval->value();
}

float BasePanoTaskTool::GetCrossSectionThickness() {
  return cs_.thickness->value();
}

float BasePanoTaskTool::GetPanoRange() {
  return pano_.arch_range->value();
}

bool BasePanoTaskTool::IsVisibleImplantEnable() const {
  return visibility_tool_->IsEnable(VisibleID::IMPLANT);
}

bool BasePanoTaskTool::IsVisibleImplantChecked() const {
  return visibility_tool_->IsChecked(VisibleID::IMPLANT);
}

void BasePanoTaskTool::SetArchRange(float range_mm) {
  pano_.arch_range->setValue(range_mm);
}

void BasePanoTaskTool::SetArchThickness(float thickness_mm) {
  pano_.thickness->setValue(thickness_mm);
}

void BasePanoTaskTool::SetPreviousArchThickness(float previous_thickness_mm)
{
	previous_arch_thickness_mm_ = previous_thickness_mm;
}

float BasePanoTaskTool::GetArchRange() { return pano_.arch_range->value(); }

float BasePanoTaskTool::GetArchThickness() { return pano_.thickness->value(); }

float BasePanoTaskTool::GetPreviousArchThickness()
{
	return previous_arch_thickness_mm_;
}

void BasePanoTaskTool::SyncVisibilityResources() {
  visibility_tool_->SyncVisibilityResources();
  visibility_tool_->blockSignals(true);

  const auto& nerve_resource =
      ResourceContainer::GetInstance()->GetNerveResource();
  const auto& nerve_datas = nerve_resource.GetNerveDataInVol();
  bool visible_nerve_exist = false;
  for (const auto& nerve : nerve_datas) {
    if (nerve.second->is_visible()) {
      visible_nerve_exist = true;
      break;
    }
  }
  visibility_tool_->SetVisibleResource(VisibleID::NERVE, visible_nerve_exist);

  const auto& imp_resource =
      ResourceContainer::GetInstance()->GetImplantResource();
  bool visible_implant = false;
  if (imp_resource.IsSetImplant() && imp_resource.is_visible_all())
    visible_implant = true;
  visibility_tool_->SetVisibleResource(VisibleID::IMPLANT, visible_implant);
  visibility_tool_->blockSignals(false);
}

QWidget* BasePanoTaskTool::GetClipWidget() { return clip_tool_->GetWidget(); }
QWidget* BasePanoTaskTool::GetVisibilityWidget() {
  return visibility_tool_->GetWidget();
}

void BasePanoTaskTool::CreateUI() {
  CW3Theme* theme = CW3Theme::getInstance();
  QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
  QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
  int spacingM = theme->getToolVBarSizeInfo().spacingM;

  // create visibility tool ui
  visibility_tool_.reset(
      new VisibilityToolBox(true, true, false, false, false, this));
  // create clipping tool ui
  std::vector<QString> mode_names = {lang::LanguagePack::txt_left_right(),
                                     lang::LanguagePack::txt_top_bottom(),
                                     lang::LanguagePack::txt_buccal_lingual()};
  clip_tool_.reset(
      new ClippingTool(mode_names, ClippingTool::DirectionType::VERTICAL));
}

void BasePanoTaskTool::Connections() {
  connect(visibility_tool_.get(), &VisibilityToolBox::sigVisible, this,
          &BasePanoTaskTool::slotVisible);

  connect(clip_tool_.get(), SIGNAL(sigEnable(int)), this,
          SLOT(slotClipStatusChanged()));
  connect(clip_tool_.get(), SIGNAL(sigRangeMove(int, int)), this,
          SLOT(slotClipStatusChanged()));
  connect(clip_tool_.get(), SIGNAL(sigRangeSet()), this,
          SLOT(slotClipStatusChanged()));
  connect(clip_tool_.get(), SIGNAL(sigPlaneChanged(int)), this,
          SLOT(slotClipStatusChanged()));

  connect(cs_.thickness, SIGNAL(valueChanged(double)), this,
          SIGNAL(sigBasePanoCSParamsChanged()));
  connect(cs_.interval, SIGNAL(valueChanged(double)), this,
          SLOT(slotCSIntervalChanged(double)));
  connect(cs_.angle, SIGNAL(valueChanged(double)), this,
          SIGNAL(sigBasePanoCSParamsChanged()));

  connect(pano_.arch_range, SIGNAL(valueChanged(double)), this,
          SIGNAL(sigBasePanoArchRangeChanged(double)));
  connect(pano_.thickness, SIGNAL(valueChanged(double)), this,
          SIGNAL(sigBasePanoArchThicknessChanged(double)));
}

void BasePanoTaskTool::Disconnections()
{
	disconnect(visibility_tool_.get(), &VisibilityToolBox::sigVisible, this, &BasePanoTaskTool::slotVisible);

	disconnect(clip_tool_.get(), SIGNAL(sigEnable(int)), this, SLOT(slotClipStatusChanged()));
	disconnect(clip_tool_.get(), SIGNAL(sigRangeMove(int, int)), this, SLOT(slotClipStatusChanged()));
	disconnect(clip_tool_.get(), SIGNAL(sigRangeSet()), this, SLOT(slotClipStatusChanged()));
	disconnect(clip_tool_.get(), SIGNAL(sigPlaneChanged(int)), this, SLOT(slotClipStatusChanged()));

	disconnect(cs_.thickness, SIGNAL(valueChanged(double)), this, SIGNAL(sigBasePanoCSParamsChanged()));
	disconnect(cs_.interval, SIGNAL(valueChanged(double)), this, SLOT(slotCSIntervalChanged(double)));
	disconnect(cs_.angle, SIGNAL(valueChanged(double)), this, SIGNAL(sigBasePanoCSParamsChanged()));

	disconnect(pano_.arch_range, SIGNAL(valueChanged(double)), this, SIGNAL(sigBasePanoArchRangeChanged(double)));
	disconnect(pano_.thickness, SIGNAL(valueChanged(double)), this, SIGNAL(sigBasePanoArchThicknessChanged(double)));
}

void BasePanoTaskTool::SetToolTips() {}

void BasePanoTaskTool::slotVisible(const VisibleID& visible_id, int state) {
  bool is_visible;
  switch (state) {
    case Qt::CheckState::Checked:
      is_visible = true;
      break;
    case Qt::CheckState::Unchecked:
      is_visible = false;
      break;
  }

  emit sigBasePanoVisible(visible_id, is_visible);
}

void BasePanoTaskTool::slotCSIntervalChanged(double slider_value) {
  double interval;
  if (slider_value == cs_.interval->minimum() + cs_.interval->singleStep()) {
    interval = 1.0;
    cs_.interval->blockSignals(true);
    cs_.interval->setValue(interval);
    cs_.interval->blockSignals(false);
  } else {
    interval = slider_value;
  }

  emit sigBasePanoCSParamsChanged();
}

void BasePanoTaskTool::slotClipStatusChanged() {
  float upper = static_cast<float>(clip_tool_->GetUpperValue());
  float lower = static_cast<float>(clip_tool_->GetLowerValue());
  float cliping_lower_value = ((lower / 100.0f) - 0.5f) * 2.0f;
  float cliping_upper_value = -((upper / 100.0f) - 0.5f) * 2.0f;

  glm::vec3 cliping_norm;
  switch (clip_tool_->GetClipPlaneID()) {
    case ClipID::LR:
      cliping_norm = glm::vec3(1.0f, 0.0f, 0.0f);
      break;
    case ClipID::TB:
      cliping_norm = glm::vec3(0.0f, 1.0f, 0.0f);
      break;
    case ClipID::BL:
      cliping_norm = glm::vec3(0.0f, 0.0f, -1.0f);
      break;
    default:
      common::Logger::instance()->Print(
          common::LogType::ERR,
          "BasePanoTool::slotClipStatusChanged : invalid clip tool");
      cliping_norm = glm::vec3();
  }

  std::vector<glm::vec4> clipping_planes = {
      glm::vec4(-cliping_norm, cliping_lower_value),
      glm::vec4(cliping_norm, cliping_upper_value)};

  emit sigBasePanoClipParamsChanged(clipping_planes,
                                    clip_tool_->IsClipEnable());
}
