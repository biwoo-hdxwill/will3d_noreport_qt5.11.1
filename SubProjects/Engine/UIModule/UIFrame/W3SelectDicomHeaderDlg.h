#pragma once
/*=========================================================================

File:		class CW3SelectDicomHeader
Language:	C++11
Library:	Qt 5.2.1, Standard C++ Library

=========================================================================*/

#include "uiframe_global.h"
#include "GeneratedFiles\ui_W3SelectDicomHeader.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"

#include <qwidget.h>
class UIFRAME_EXPORT CW3SelectDicomHeaderDlg : public QDialog
{
	Q_OBJECT

public:
	CW3SelectDicomHeaderDlg(QWidget *parent = 0);
	~CW3SelectDicomHeaderDlg(void);

signals:
	// signal for changed list. (for display)
	void sigSetDisplayList(std::vector<W3BOOL> bList);

public slots:
	virtual void accept(void) override;
	void show(void);

private:
	// private member fields.
	Ui::DicomHeaderSelect ui;

	std::vector<W3BOOL> m_bList;
};

