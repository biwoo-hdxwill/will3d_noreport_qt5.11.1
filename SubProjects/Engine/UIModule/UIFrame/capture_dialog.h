#pragma once

/*=========================================================================

File:			class CaptureDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-09
Last modify:	2016-05-09

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QCheckBox;
class QHBoxLayout;
class QComboBox;

class UIFRAME_EXPORT CaptureDialog : public CW3Dialog {
	Q_OBJECT

public:
	CaptureDialog(const QStringList& view_list, QWidget* parent = 0);
	~CaptureDialog();

public slots:
	virtual void reject() override;

private slots:
	void slotCapture();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();
	void GetValues();

private:
	QComboBox* select_area_to_capture_combo_ = nullptr;

	QCheckBox* include_dicom_info_check_ = nullptr;

	QStringList capture_list_{ "Full screen" };
};
