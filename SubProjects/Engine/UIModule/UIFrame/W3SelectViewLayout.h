#pragma once
/*=========================================================================

File:		pop up widget for change layout
Language:	C++11
Library:	Qt Library

=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"
#include "GeneratedFiles\ui_W3SelectViewLayout.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"

class UIFRAME_EXPORT CW3SelectViewLayout : public QWidget
{
	Q_OBJECT

public:
	CW3SelectViewLayout(QWidget *parent = 0);
	~CW3SelectViewLayout();

signals:
	void sigChangeLayout(EVIEW_LAYOUT_TYPE eType , W3INT row, W3INT col);

private slots:
	void slotMPRLayout(void);
	void slotSingletonLayout(void);
	void slotGridLayout(void);
	void slotSegmentationLayout(void);
	void slotLMainLayout(void);

	void slotRowChanged(int row);
	void slotColChanged(int col);
	void slotSelectViewIdx(int idx);

private:
	void setConnections(void);
	void setDisconnections(void);

private:
	 Ui::CW3SelectViewLayout ui;
	 W3INT	m_nRow;
	 W3INT	m_nCol;

	 W3INT	m_nSelectedViewIdx;
};

