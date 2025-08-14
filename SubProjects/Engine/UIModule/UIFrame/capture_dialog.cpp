#include "capture_dialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/global_preferences.h"

CaptureDialog::CaptureDialog(const QStringList& view_list, QWidget* parent)
    : CW3Dialog(lang::LanguagePack::txt_capture(), parent) {
  capture_list_.append(view_list);
  SetLayout();
}

CaptureDialog::~CaptureDialog() {}

void CaptureDialog::SetLayout() {
  m_contentLayout->setContentsMargins(20, 10, 20, 10);
  m_contentLayout->setSpacing(10);
  m_contentLayout->addLayout(CreateContentsLayout());
  m_contentLayout->addLayout(CreateButtonLayout());
}

QVBoxLayout* CaptureDialog::CreateContentsLayout() {
  QLabel* select_area_to_capture_label = new QLabel();
  select_area_to_capture_label->setText(
      lang::LanguagePack::txt_select_area_to_capture() + " :");

  select_area_to_capture_combo_ = new QComboBox();
  select_area_to_capture_combo_->setStyleSheet(
      CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());
  select_area_to_capture_combo_->setSizePolicy(QSizePolicy::Preferred,
                                               QSizePolicy::Fixed);
  select_area_to_capture_combo_->setFixedHeight(
      CW3Theme::getInstance()->size_button().height());

  select_area_to_capture_combo_->addItems(capture_list_);
  select_area_to_capture_combo_->setFixedWidth(200);

  QHBoxLayout* select_area_to_capture_layout = new QHBoxLayout();
  select_area_to_capture_layout->setSpacing(10);
  select_area_to_capture_layout->addWidget(select_area_to_capture_label);
  select_area_to_capture_layout->addWidget(select_area_to_capture_combo_);

  include_dicom_info_check_ = new QCheckBox();
  include_dicom_info_check_->setText(
      lang::LanguagePack::txt_include_dicom_info());

  GetValues();

  QVBoxLayout* layout = new QVBoxLayout();
  layout->addLayout(select_area_to_capture_layout);
  layout->addWidget(include_dicom_info_check_);

  return layout;
}

QHBoxLayout* CaptureDialog::CreateButtonLayout() {
  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing(10);
  layout->setAlignment(Qt::AlignCenter);

  QToolButton* capture_button = new QToolButton();
  QToolButton* cancel_button = new QToolButton();

  connect(capture_button, SIGNAL(clicked()), this, SLOT(slotCapture()));
  connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

  capture_button->setText(lang::LanguagePack::txt_capture());
  cancel_button->setText(lang::LanguagePack::txt_cancel());

  layout->addWidget(capture_button);
  layout->addWidget(cancel_button);

  positive_button_ = capture_button;

  return layout;
}

void CaptureDialog::GetValues() {
  include_dicom_info_check_->setChecked(
      GlobalPreferences::GetInstance()
          ->preferences_.capture.include_dicom_info);
}

void CaptureDialog::reject() { done(-1); }

void CaptureDialog::slotCapture() {
  GlobalPreferences::GetInstance()->preferences_.capture.include_dicom_info =
      include_dicom_info_check_->isChecked();
  GlobalPreferences::GetInstance()->SaveCapture();

  done(select_area_to_capture_combo_->currentIndex());
}
