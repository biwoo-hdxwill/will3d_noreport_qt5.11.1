#pragma once

/*=========================================================================

File:			class CW3CephCoordSysDlg
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-06-28
Last modify:	2016-06-28

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"
#include "uiframe_global.h"

class QToolButton;

class UIFRAME_EXPORT CW3CephCoordSysDlg : public CW3Dialog {
	Q_OBJECT

public:
	CW3CephCoordSysDlg(QWidget * parent = 0);
	virtual ~CW3CephCoordSysDlg();

	CW3CephCoordSysDlg(const CW3CephCoordSysDlg&) = delete;
	CW3CephCoordSysDlg& operator=(const CW3CephCoordSysDlg&) = delete;

private slots:
	void slotClickedOK();

private:
	QToolButton* m_btnOK;
	QToolButton* m_btnCancel;
};
