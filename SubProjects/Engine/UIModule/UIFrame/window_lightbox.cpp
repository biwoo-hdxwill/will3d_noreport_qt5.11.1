#include "window_lightbox.h"
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/language_pack.h>

WindowLightbox::WindowLightbox(const MPRViewType& view_type,
                               const float& interval, const float& thickness,
                               QWidget* parent)
    : Window(parent),
      thickness_(new QDoubleSpinBox),
      interval_(new QDoubleSpinBox) {
  QString view_title;
  switch (view_type) {
    case MPRViewType::AXIAL:
      view_title = lang::LanguagePack::txt_axial();
      break;
    case MPRViewType::SAGITTAL:
      view_title = lang::LanguagePack::txt_sagittal();
      break;
    case MPRViewType::CORONAL:
      view_title = lang::LanguagePack::txt_coronal();
      break;
    default:
      break;
  }
  InitViewMenu(view_title);
  Initialize();
  interval_->setValue(interval);
  thickness_->setValue(thickness);
  Connections();
}

WindowLightbox::~WindowLightbox() {}

void WindowLightbox::slotIntervalChanged(double slider_value) {
  double interval;
  if (slider_value == interval_->minimum() + interval_->singleStep()) {
    interval = 1.0;
    interval_->blockSignals(true);
    interval_->setValue(interval);
    interval_->blockSignals(false);
  } else {
    interval = slider_value;
  }

  emit sigChangeInterval(interval);
}

void WindowLightbox::Initialize() {
  auto func_gen_spin = [&](QDoubleSpinBox* spin_box) {
    spin_box->setDecimals(2);
    spin_box->setValue(0.0);
    spin_box->setSuffix(" mm");
    spin_box->setSingleStep(1.0);
    spin_box->setRange(0.0, common::kMPRThicknessMax);
  };

  std::vector<QAbstractSpinBox*> spin_boxes;
  spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(interval_.get()));
  spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(thickness_.get()));

  func_gen_spin(interval_.get());
  interval_->setObjectName(lang::LanguagePack::txt_interval());

  func_gen_spin(thickness_.get());
  thickness_->setObjectName(lang::LanguagePack::txt_thickness());

  Window::AddSpinBox(spin_boxes);
  Window::AddGridButton();
  Window::AddCloseButton();

  Window::SetMaximumRowColCount(4, 5);
  Window::HideMaximizeButton();
}

void WindowLightbox::slotSelectLayout(int row, int column) {
  if (QObject::sender()) {
    Window::slotSelectLayout(row, column);
    emit sigChangeLightboxLayout(row, column);
  } else {
    Window::slotSelectLayout(row, column);
  }
}

void WindowLightbox::InitInterval(const float& interval) {
  interval_->setValue(interval);
  interval_->setMinimum(interval);
}

const int WindowLightbox::GetThicknessValue() const
{
	return thickness_->value();
}

void WindowLightbox::Connections() {
  connect(interval_.get(), SIGNAL(valueChanged(double)), this,
          SLOT(slotIntervalChanged(double)));
  connect(thickness_.get(), SIGNAL(valueChanged(double)), this,
          SIGNAL(sigChangeThickness(double)));
}
