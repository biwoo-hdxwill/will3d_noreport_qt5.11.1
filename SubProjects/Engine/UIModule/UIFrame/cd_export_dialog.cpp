#include "cd_export_dialog.h"

#include <windows.h>
#include <stdio.h>

#include <QDebug>
#include <QToolButton>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <QComboBox>
#include <QStorageInfo>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"

namespace
{
	const int kDlgWidth = 600;
	const int kDlgHeight = 180;
	const QString kDefulatPathString("Select Path");
} // end of namespace

CDExportDialog::CDExportDialog(QWidget* parent)
	: CW3Dialog(tr("CD/USB Export"), parent), cancel_(new QToolButton),
	select_(new QToolButton), export_(new QToolButton),
	method_cd_(new QRadioButton("CD")), method_usb_(new QRadioButton("USB")),
	include_viewer_(new QCheckBox(lang::LanguagePack::txt_include_viewer())),
	dicom_compressed_(new QCheckBox(lang::LanguagePack::txt_dicom_compressed())),
	usb_path_edit_(new QLineEdit),
	cd_drives_combo_(new QComboBox())
{
	InitUI();
	SetConnections();
}

CDExportDialog::~CDExportDialog() {}

bool CDExportDialog::IsIncludeViewer() const
{
	return include_viewer_->isChecked();
}

bool CDExportDialog::IsDicomCompressed() const
{
	return dicom_compressed_->isChecked();
}

DCMExportMethod CDExportDialog::ExportMethod() const
{
	return method_cd_->isChecked() ? DCMExportMethod::CD : DCMExportMethod::USB;
}

QString CDExportDialog::Path() const
{
	switch (method_)
	{
	case DCMExportMethod::CD:
		return cd_drives_combo_->currentText();
	case DCMExportMethod::USB:
		return usb_path_edit_->text();
	default:
		return QString();
	}
}

void CDExportDialog::closeEvent(QCloseEvent * e)
{
	QDialog::closeEvent(e);
	slotCancel();
}

void CDExportDialog::slotCancel()
{
	reject();
}

void CDExportDialog::slotExport()
{
	bool result = false;
	switch (method_)
	{
	case DCMExportMethod::CD:
		result = QDir(cd_drives_combo_->currentText()).exists();
		break;
	case DCMExportMethod::USB:
		result = QDir(usb_path_edit_->text()).exists();
		break;
	}

	if (!result)
	{
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_57(), CW3MessageBox::Critical);
		msgBox.exec();
		usb_path_edit_->setText(kDefulatPathString);
		return;
	}
	accept();
}

void CDExportDialog::slotSelectPath()
{
	QString title = ExportMethod() == DCMExportMethod::CD ? QString("Select CD/DVD Drive") : QString("Select USB Drive");
	QString dir = QFileDialog::getExistingDirectory(this, title, "../../", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

#if 0
	if (dir.isEmpty())
	{
		usb_path_edit_->setText(kDefulatPathString);
	}
	else
#endif
	{
		usb_path_edit_->setText(dir);
	}
}

void CDExportDialog::InitUI()
{
	cancel_->setText(lang::LanguagePack::txt_cancel());
	export_->setText(lang::LanguagePack::txt_export());
	select_->setText(lang::LanguagePack::txt_select());
	usb_path_edit_->setText(kDefulatPathString);
	usb_path_edit_->setReadOnly(true);
	method_usb_->setChecked(true);
	include_viewer_->setChecked(true);
	cd_drives_combo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	setFixedSize(kDlgWidth, kDlgHeight);

	QHBoxLayout* select_method_layout = new QHBoxLayout;
	select_method_layout->setAlignment(Qt::AlignLeft);
	select_method_layout->setSpacing(10);
	select_method_layout->setContentsMargins(0, 0, 0, 0);
	select_method_layout->addWidget(new QLabel(lang::LanguagePack::txt_export_to() + ":"));
	select_method_layout->addWidget(method_cd_);
	select_method_layout->addWidget(method_usb_);

	QHBoxLayout* option_layout = new QHBoxLayout;
	option_layout->setAlignment(Qt::AlignLeft);
	option_layout->setSpacing(10);
	option_layout->setContentsMargins(0, 0, 0, 0);
	option_layout->addWidget(include_viewer_);
	option_layout->addWidget(dicom_compressed_);

	QHBoxLayout* changable_widget_layout = new QHBoxLayout;
	changable_widget_layout->setSpacing(10);
	changable_widget_layout->setContentsMargins(0, 0, 0, 0);
	changable_widget_layout->addWidget(cd_drives_combo_);
	changable_widget_layout->addWidget(usb_path_edit_);
	changable_widget_layout->addWidget(select_);

	QHBoxLayout* button_layout = new QHBoxLayout;
	button_layout->setAlignment(Qt::AlignCenter);
	button_layout->setSpacing(10);
	button_layout->setContentsMargins(0, 0, 0, 0);
	button_layout->addWidget(export_);
	button_layout->addWidget(cancel_);

	QFrame *h_line = new QFrame();
	h_line->setFrameShape(QFrame::HLine);
	h_line->setObjectName("line");
	h_line->setLineWidth(0);
	h_line->setMidLineWidth(0);
	h_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	m_contentLayout->setAlignment(Qt::AlignCenter);
	m_contentLayout->setContentsMargins(15, 6, 15, 9);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addLayout(select_method_layout);
	m_contentLayout->addWidget(h_line);
	m_contentLayout->addLayout(option_layout);
	m_contentLayout->addLayout(changable_widget_layout);
	m_contentLayout->addLayout(button_layout);

	slotUSBSelected();
	InitCDDrivesList();
}

void CDExportDialog::InitCDDrivesList()
{
	std::string cd_drive = "";

	DWORD drives = GetLogicalDrives();
	DWORD size = MAX_PATH;
	char logical_drives[MAX_PATH] = { 0 };
	DWORD dwResult = GetLogicalDriveStringsA(size, logical_drives);

	if (dwResult > 0 && dwResult <= MAX_PATH)
	{
		char* current_drive = logical_drives;

		while (*current_drive)
		{
			const UINT type = GetDriveTypeA(current_drive);

			if (type == 5)
			{
				cd_drive = current_drive;
				cd_drive = cd_drive.substr(0, 2);

				qDebug() << "cd_drive :" << current_drive << cd_drive.c_str();
				
				QString cur_drive = QString(current_drive);
				cur_drive.replace("\\", "/");

				cd_drives_combo_->addItem(cur_drive);
				//break;
			}

			// Get the next drive
			current_drive += strlen(current_drive) + 1;
		}
	}
}

void CDExportDialog::slotCDSelected()
{
	method_ = DCMExportMethod::CD;

	cd_drives_combo_->setVisible(true);
	usb_path_edit_->setVisible(false);
	select_->setVisible(false);
}

void CDExportDialog::slotUSBSelected()
{
	method_ = DCMExportMethod::USB;

	cd_drives_combo_->setVisible(false);
	usb_path_edit_->setVisible(true);
	select_->setVisible(true);
}

void CDExportDialog::SetConnections()
{
	connect(select_, SIGNAL(clicked()), this, SLOT(slotSelectPath()));
	connect(export_, SIGNAL(clicked()), this, SLOT(slotExport()));
	connect(cancel_, SIGNAL(clicked()), this, SLOT(slotCancel()));

	connect(method_usb_, SIGNAL(clicked()), this, SLOT(slotUSBSelected()));
	connect(method_cd_, SIGNAL(clicked()), this, SLOT(slotCDSelected()));
}
