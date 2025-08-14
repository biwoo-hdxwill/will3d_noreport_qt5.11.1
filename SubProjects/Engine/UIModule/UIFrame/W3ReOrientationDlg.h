#pragma once

/*=========================================================================

File:			class CW3MPRMTMJ
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			PARK, SANGKEUN
First Date:		2015-07-22


Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.k

=========================================================================*/
#include "uiframe_global.h"
#include <QTWidgets>
#include "GeneratedFiles\ui_W3ReOrientationDlg.h"

#include "W3WindowReOrientationDlg.h"
//#include "W3WindowVR.h"

#define BTN_HEIGHT		22
#define BTN_WIDTH		70

#define DLG_HEIGHT		512
#define DLG_WIDTH		1024

class CW3MPRMod;
class CW3Image3D;

class UIFRAME_EXPORT CW3ReOrientationDlg : public QDialog
{
	Q_OBJECT

public:
	CW3ReOrientationDlg(QWidget *parent = 0);
	~CW3ReOrientationDlg();

	void setVolume(CW3Image3D *volume);
	void initDisplay(void);
	void displayMPR(void);
	
signals:
	void sigReOrientationVector(CW3Vector3D vX, CW3Vector3D vY, CW3Vector3D vZ);

public slots:
	void slotRotate(const EMPR_VIEW_TYPE eViewType, const W3FLOAT fAngle);
	void slotWheelEvent(const EMPR_VIEW_TYPE eViewType, const W3INT delta);
	
	void slotOkBtn(void);
	void slotCancelBtn(void);	
	void slotCheckboxGrid(int);

	void slotResetBtn(void);
	void slotRotationBtn(void);
	void slotZoomBtn(void);
	void slotPanBtn(void);

private:
	Ui::CW3ReOrientationDlg ui;

	void createObject(void);
	void initLayout(void);
	W3BOOL checkObject(void);
	void connection(void);
	void reset(void);

	
	CW3WindowReOrientationDlg * m_pWindowMPR[EMPR_VIEW_TYPE::mpr_end];
	//CW3WindowVR  * m_pWindowVR;
	CW3MPRMod *m_pMPRModule;
	QPushButton * m_pBtnOk;
	QPushButton * m_pBtnCancel;
	QPushButton * m_pBtnReset;
	QPushButton * m_pBtnRotation;
	QPushButton * m_pBtnZoom;
	QPushButton * m_pBtnPan;
	QCheckBox *m_pCheckboxGrid;		
	
};


