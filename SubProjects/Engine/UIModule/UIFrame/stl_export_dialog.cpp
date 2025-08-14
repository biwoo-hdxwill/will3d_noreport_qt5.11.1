#include "stl_export_dialog.h"

#include <QDebug>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QFileDialog>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"

STLExportDialog::STLExportDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_stl_setting(), parent) {
	SetLayout();
}

STLExportDialog::~STLExportDialog() {
}

void STLExportDialog::SetLayout() {
	m_contentLayout->setContentsMargins(20, 10, 20, 10);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addLayout(CreateContentsLayout());
	m_contentLayout->addLayout(CreateButtonLayout());
}

QVBoxLayout* STLExportDialog::CreateContentsLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* quality_layout = new QHBoxLayout();
	QHBoxLayout* path_layout = new QHBoxLayout();
	QLabel* quality_label = new QLabel();
	QLabel* path_label = new QLabel();
	QToolButton* browse_button = new QToolButton();
	smooth_on_check_ = new QCheckBox();
	path_input_ = new QLineEdit();
	quality_radio_group_ = new QButtonGroup(this);
	QRadioButton* quality_high_radio = new QRadioButton();
	QRadioButton* quality_medium_radio = new QRadioButton();
	QRadioButton* quality_low_radio = new QRadioButton();

	quality_label->setText(lang::LanguagePack::txt_quality() + " :");
	quality_high_radio->setText(lang::LanguagePack::txt_high());
	quality_medium_radio->setText(lang::LanguagePack::txt_medium());
	quality_low_radio->setText(lang::LanguagePack::txt_low());
	path_label->setText(lang::LanguagePack::txt_path() + " :");
	smooth_on_check_->setText(lang::LanguagePack::txt_smooth_on());
	browse_button->setText(lang::LanguagePack::txt_browse());

	connect(browse_button, SIGNAL(clicked()), this, SLOT(slotBrowse()));

	quality_radio_group_->addButton(quality_high_radio, 0);
	quality_radio_group_->addButton(quality_medium_radio, 1);
	quality_radio_group_->addButton(quality_low_radio, 2);

	path_input_->setFixedWidth(300);

	quality_layout->setSpacing(10);
	quality_layout->setAlignment(Qt::AlignLeft);

	quality_layout->addWidget(quality_label);
	quality_layout->addWidget(quality_high_radio);
	quality_layout->addWidget(quality_medium_radio);
	quality_layout->addWidget(quality_low_radio);

	path_layout->setSpacing(10);

	path_layout->addWidget(path_label);
	path_layout->addWidget(path_input_);
	path_layout->addWidget(browse_button);

	GetValues();

	layout->addLayout(quality_layout);
	layout->addWidget(CreateHorizontalLine());
	layout->addWidget(smooth_on_check_);
	layout->addLayout(path_layout);

	return layout;
}

QHBoxLayout* STLExportDialog::CreateButtonLayout() {
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(10);
	layout->setAlignment(Qt::AlignCenter);

	QToolButton* export_button = new QToolButton();
	QToolButton* cancel_button = new QToolButton();

	connect(export_button, SIGNAL(clicked()), this, SLOT(slotExport()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	export_button->setText(lang::LanguagePack::txt_export());
	cancel_button->setText(lang::LanguagePack::txt_cancel());

	layout->addWidget(export_button);
	layout->addWidget(cancel_button);

	return layout;
}

void STLExportDialog::GetValues() {
	QAbstractButton* button = quality_radio_group_->button(static_cast<int>(GlobalPreferences::GetInstance()->preferences_.stl_export.quality));
	if (button)
		button->setChecked(true);

	smooth_on_check_->setChecked(GlobalPreferences::GetInstance()->preferences_.stl_export.smooth_on);
}

QString STLExportDialog::GetSavePath() {
	return path_input_->text();
}

// slots
void STLExportDialog::slotExport() {
	if (path_input_->text().isEmpty()) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_58(), CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	GlobalPreferences::GetInstance()->preferences_.stl_export.quality = static_cast<GlobalPreferences::Quality3>(quality_radio_group_->checkedId());
	GlobalPreferences::GetInstance()->preferences_.stl_export.smooth_on = smooth_on_check_->isChecked();
	GlobalPreferences::GetInstance()->SaveSTLExport();

	accept();
}

void STLExportDialog::slotBrowse() {
	QString dir_path = GlobalPreferences::GetInstance()->preferences_.general.files.stl_export_path;
	QDir dir(dir_path);
	if (!dir.exists())
		QDir().mkdir(dir.absolutePath());

	QString path;
#if defined(__APPLE__)
	path = QFileDialog::getSaveFileName(this, lang::LanguagePack::msg_59(), dir_path,
		"stl (*.stl)",
		nullptr,
		QFileDialog::Option::DontUseNativeDialog);
#else
	path = QFileDialog::getSaveFileName(this, lang::LanguagePack::msg_59(), dir_path, "STL files (*.stl)");
#endif

	path_input_->setText(path);
}
// slots
