#include "window_tmj_lateral.h"

#include <QTextCodec>
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"

/**=================================================================================================
class WindowTmjLateral
*===============================================================================================**/

WindowTmjLateral::WindowTmjLateral(const TMJDirectionType& type,
                                   QWidget* parent)
    : type_(type), Window(parent) {
  Initialize();
}

WindowTmjLateral::~WindowTmjLateral(void) {}

void WindowTmjLateral::Initialize() {
  InitSpinboxes();
  std::vector<QAbstractSpinBox*> spin_boxes;
  spin_boxes.push_back(
      dynamic_cast<QAbstractSpinBox*>(spin_box_.interval.get()));
  spin_boxes.push_back(
      dynamic_cast<QAbstractSpinBox*>(spin_box_.thickness.get()));

  QString dir_text = (type_ == TMJ_LEFT) ? lang::LanguagePack::txt_left_lateral()
                                         : lang::LanguagePack::txt_right_lateral();

  InitViewMenu(dir_text);

  Window::AddSpinBox(spin_boxes);
  Window::AddGridButton();

  Window::SetMaximumRowColCount(3, 3);
  Window::HideMaximizeButton();
}

void WindowTmjLateral::InitSpinboxes() {
  QSettings settings("Will3D.ini", QSettings::IniFormat);
  settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

  double interval =
      settings.value("TMJ/lateral_interval", 1.0).toString().toDouble();
  double thickness =
      settings.value("TMJ/lateral_thickness", 0.0).toString().toDouble();

  double interval_maximum =
      settings.value("TMJ/lateral_interval_maximum", 100.0).toString().toDouble();
  double thickness_maximum =
      settings.value("TMJ/lateral_thickness_maximum", 200.0).toString().toDouble();

  double interval_stride =
      settings.value("TMJ/lateral_interval_stride", 1.0).toString().toDouble();
  double thickness_stride =
      settings.value("TMJ/lateral_thickness_stride", 1.0).toString().toDouble();

  spin_box_.interval.reset(new QDoubleSpinBox);
  spin_box_.interval->setDecimals(2);
  spin_box_.interval->setSuffix(" mm");
  spin_box_.interval->setRange(0.1, interval_maximum);
  spin_box_.interval->setValue(interval);
  spin_box_.interval->setObjectName(lang::LanguagePack::txt_interval());
  spin_box_.interval->setSingleStep(interval_stride);

  spin_box_.thickness.reset(new QDoubleSpinBox);
  spin_box_.thickness->setDecimals(2);
  spin_box_.thickness->setValue(0.0);
  spin_box_.thickness->setSuffix(" mm");
  spin_box_.thickness->setRange(0.0, thickness_maximum);
  spin_box_.thickness->setValue(thickness);
  spin_box_.thickness->setObjectName(lang::LanguagePack::txt_thickness());
  spin_box_.thickness->setSingleStep(thickness_stride);
}

void WindowTmjLateral::SetIntervalMinimumValue(const float& value) {
  spin_box_.interval->setMinimum(value);
}

void WindowTmjLateral::ApplyPreferences() {
  // double interval_increments =
  // GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.interval_increments;
  // spin_box_.interval->setSingleStep(interval_increments);
  //
  // double thickness_increments =
  // GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.thickness_increments;
  // spin_box_.thickness->setSingleStep(thickness_increments);
}
