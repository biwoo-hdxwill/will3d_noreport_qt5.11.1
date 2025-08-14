#pragma once

/*=========================================================================

File:			class CW3OTFPresetDlg
Language:		C++11
Library:        Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-07-21
Last modify:	2016-07-21

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"
#include "uiframe_global.h"

class CW3ViewOTFPreset;
class QToolButton;

class UIFRAME_EXPORT CW3OTFPresetDlg : public CW3Dialog {
	Q_OBJECT

public:
	CW3OTFPresetDlg(QWidget *parent = 0);
	~CW3OTFPresetDlg();

signals:
	void sigSavePreset(const QString& strPath);

private slots:
	void slotSetFavorite(int id, int check);
	void slotWrite(int id);
	void slotOverwrite(int id);

	void slotClickedOK();
	void slotClickedCancel();

private:
	QToolButton *m_btnOK;
	QToolButton *m_btnCancel;

	QList<CW3ViewOTFPreset *> m_lpPresetView;

	QStringList m_listPresetFile;
	QStringList m_listFavorite;
};
