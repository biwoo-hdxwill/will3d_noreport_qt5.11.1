#pragma once

/*=========================================================================

File:			class CW3ToolHBar
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-04-05
Last modify:	2016-04-05

=========================================================================*/
#include <QWidget>
#include "uiframe_global.h"

class QHBoxLayout;
class CW3ImageHeader;
class DicomInfoBox;

class UIFRAME_EXPORT CW3ToolHBar : public QWidget {
	Q_OBJECT
public:
	CW3ToolHBar(QWidget* parent = 0);
	~CW3ToolHBar();

	void SetDicomInfo(CW3ImageHeader* header);
	
private:
	QHBoxLayout* main_layout_ = nullptr;
	DicomInfoBox* dicom_info_ = nullptr;
};
