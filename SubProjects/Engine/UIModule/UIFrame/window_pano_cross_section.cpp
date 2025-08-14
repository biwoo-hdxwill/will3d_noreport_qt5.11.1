#include "window_pano_cross_section.h"

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"

/**=================================================================================================
class WindowPanoCrossSection
*===============================================================================================**/

WindowPanoCrossSection::WindowPanoCrossSection(QWidget* parent)
    : Window(parent) {
  Initialize();
}

WindowPanoCrossSection::~WindowPanoCrossSection(void) {}

void WindowPanoCrossSection::Initialize() {
  InitSpinboxes();
  std::vector<QAbstractSpinBox*> spin_boxes;
  spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(spin_box_.interval.get()));
  spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(spin_box_.rotate.get()));
  spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(spin_box_.thickness.get()));

  InitViewMenu(lang::LanguagePack::txt_cross_section());
  Window::AddSpinBox(spin_boxes);
  Window::AddGridButton();
}

void WindowPanoCrossSection::InitSpinboxes() {
  spin_box_.rotate.reset(new QDoubleSpinBox);
  spin_box_.rotate->setDecimals(0);
  spin_box_.rotate->setRange(-89, 89);
  spin_box_.rotate->setSingleStep(1.0);
  spin_box_.rotate->setValue(0);
  //spin_box_.rotate->setSuffix(QString::fromLocal8Bit("°"));
  spin_box_.rotate->setSuffix(QString(QChar(0x00B0)));
  spin_box_.rotate->setObjectName(lang::LanguagePack::txt_rotate());

  double default_interval =
      GlobalPreferences::GetInstance()
          ->preferences_.advanced.cross_section_view.default_interval;
  spin_box_.interval.reset(new QDoubleSpinBox);
  spin_box_.interval->setDecimals(2);
  spin_box_.interval->setValue(default_interval);
  spin_box_.interval->setSuffix(" mm");
  spin_box_.interval->setRange(0.1, 20.0);
  spin_box_.interval->setObjectName(lang::LanguagePack::txt_interval());

  ApplyIntervalIncrements();

  spin_box_.thickness.reset(new QDoubleSpinBox);
  spin_box_.thickness->setDecimals(2);
  spin_box_.thickness->setValue(0.0);
  spin_box_.thickness->setSuffix(" mm");
  spin_box_.thickness->setRange(0.0, 20.0);
  spin_box_.thickness->setObjectName(lang::LanguagePack::txt_thickness());

  ApplyThicknessIncrements();
}

void WindowPanoCrossSection::SetIntervalMinimumValue(const float& value) {
  spin_box_.interval->setMinimum(value);
}

void WindowPanoCrossSection::ApplyPreferences() {
  ApplyIntervalIncrements();
  ApplyThicknessIncrements();
}

void WindowPanoCrossSection::ApplyIntervalIncrements() {
  double interval_increments =
      GlobalPreferences::GetInstance()
          ->preferences_.advanced.cross_section_view.interval_increments;
  spin_box_.interval->setSingleStep(interval_increments);
}

void WindowPanoCrossSection::ApplyThicknessIncrements() {
  double thickness_increments =
      GlobalPreferences::GetInstance()
          ->preferences_.advanced.cross_section_view.thickness_increments;
  spin_box_.thickness->setSingleStep(thickness_increments);
}
