#pragma once

/*=========================================================================

File:			class STLExportDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-17
Last modify:	2016-05-17

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QCheckBox;
class QHBoxLayout;
class QButtonGroup;

class UIFRAME_EXPORT STLExportDialog : public CW3Dialog {
	Q_OBJECT

public:
	STLExportDialog(QWidget* parent = 0);
	~STLExportDialog();

	QString GetSavePath();

private slots:
	void slotExport();
	void slotBrowse();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();
	void GetValues();

private:
	QButtonGroup* quality_radio_group_ = nullptr;

	QCheckBox* smooth_on_check_ = nullptr;

	QLineEdit* path_input_ = nullptr;
};
