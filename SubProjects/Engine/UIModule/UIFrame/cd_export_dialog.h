#pragma once
/*=========================================================================

File:			class CW3ImplantDBDlg
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Seo Seok Man
First date:		2018-04-13
Last modify:	2018-04-13

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"
#include "../../Common/Common/W3Enum.h"
#include "uiframe_global.h"

class QCheckBox;
class QToolButton;
class QRadioButton;
class QLineEdit;
class QComboBox;

class UIFRAME_EXPORT CDExportDialog : public CW3Dialog
{
	Q_OBJECT

public:
	explicit CDExportDialog(QWidget* parent = nullptr);
	virtual ~CDExportDialog();

public:
	bool IsIncludeViewer() const;
	bool IsDicomCompressed() const;
	DCMExportMethod ExportMethod() const;
	QString Path() const;

protected:
	virtual void closeEvent(QCloseEvent* e) override;

private slots:
	void slotSelectPath();
	void slotCancel();
	void slotExport();
	void slotCDSelected();
	void slotUSBSelected();

private:
	void InitUI();
	void InitCDDrivesList();
	void SetConnections();

private:
	DCMExportMethod method_ = DCMExportMethod::USB;

	QToolButton* select_ = nullptr;
	QToolButton* export_ = nullptr;
	QToolButton* cancel_ = nullptr;

	QRadioButton* method_cd_ = nullptr;
	QRadioButton* method_usb_ = nullptr;

	QCheckBox* include_viewer_ = nullptr;
	QCheckBox* dicom_compressed_ = nullptr;

	QLineEdit* usb_path_edit_ = nullptr;
	QComboBox* cd_drives_combo_ = nullptr;
};
