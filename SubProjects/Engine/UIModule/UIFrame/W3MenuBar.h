#pragma once
/*=========================================================================

File:		class CW3MenuBar
Language:	C++11
Library:	Qt 5.4, Standard C++ Library
Author:		Hong Jung
First Date:	2015-12-10
Last Date:	2016-04-14

=========================================================================*/

#include "uiframe_global.h"

#include <QWidget>
#include "GeneratedFiles\ui_W3MenuBar.h"
#include "../../Common/Common/W3Types.h"
#include "../../Common/Common/W3Enum.h"

#include "../UIMenus/W3CommonMenus.h"
#include "../UIMenus/W3MPRMenus.h"
#include "../UIMenus/W3PanoMenus.h"
#include "../UIMenus/W3ImplantMenus.h"
#include "../UIMenus/W3FaceMenus.h"
#include "../UIMenus/W3SIMenus.h"
#include "../UIMenus/W3EndoMenus.h"
#include "../UIMenus/W3TmjMenus.h"
#include "../UIMenus/W3OrthoMenus.h"

#define TAB_IDX_DISABLED		-1
#define TAB_IDX_FILE			0
#define TAB_IDX_MPR				1
#define TAB_IDX_PANORAMA		2
#define TAB_IDX_IMPLANT			3
#define TAB_IDX_FACESIM			4
#define TAB_IDX_SUPERIMPOSE		5
#define TAB_IDX_ENDO			6
#define TAB_IDX_3DCEPH			7
#define TAB_IDX_TMJ				8
#define TAB_IDX_ORTHODONTIC		9
#define TAB_IDX_REPORT			10


class UIFRAME_EXPORT CW3MenuBar : public QWidget
{
	Q_OBJECT

public:
	CW3MenuBar(QWidget *parent = 0);
	~CW3MenuBar();

	void setMaskTree(QWidget* pMaskTree);
	inline CW3CommonMenus* getCommonMenus()
	{
		return m_pCommonMenus;
	}
	inline CW3MPRMenus*	getMPRMenus()
	{
		return m_pMPRMenus;
	}
	inline CW3PanoMenus* getPanoMenus()
	{
		return m_pPanoMenus;
	}
	inline CW3ImplantMenus* getImplantMenus()
	{
		return m_pImplantMenus;
	}
	inline CW3FaceMenus* getFaceMenus()
	{
		return m_pFaceMenus;
	}
	inline CW3SIMenus*	getSIMenus()
	{
		return m_pSIMenus;
	}
	inline CW3EndoMenus* getEndoMenus()
	{
		return m_pEndoMenus;
	}
	inline CW3TmjMenus* getTmjMenus()
	{
		return m_pTmjMenus;
	}
	inline CW3OrthoMenus* getOrthoMenus()
	{
		return m_pOrthoMenus;
	}

signals:
	

	// APPLICATION HANDLING SIGNALS.	
	void sigLoadDicom(void);
	void sigLoadDicomFolder(void);
	void sigLoadRawfile(void);
	void sigMinimize(W3BOOL);
	void sigSetMenuSize(W3INT);

	//for layout change
	void sigChangeTab(ETAB_TYPE eTabType); 


	
	// by jdk 150714
	void sigActiveVOI(void);
	// jdk end

	// by jdk 150722
	void sigActiveZoom3D(void);
	// jdk end
	
public slots:
	//slot functions for DBM
	void slotLoadDICOM(void);
	void slotLoadDICOMFolder(void);
	void slotSetVolume(ETAB_TYPE);


	//slot functions for tab selection
	void slotMenuClicked(int tabIndex);

	// by jdk 150714
	void slotActiveVOI(void);
	// jdk end
	
	// by jdk 150722
	void slotActiveZoom3D(void);
	// jdk end

	//void slotGoToMPR();
	//void slotGoToFILE();	// by jdk 160706

	void slotGoTo(ETAB_TYPE tab);	// by jdk 160707

private:
	// private functions
	void connections(void);
	void disconnections(void);

private:
	Ui::CW3MenuBar ui;

	CW3CommonMenus	*m_pCommonMenus;
	CW3MPRMenus		*m_pMPRMenus;
	CW3PanoMenus	*m_pPanoMenus;
	CW3ImplantMenus	*m_pImplantMenus;
	CW3FaceMenus	*m_pFaceMenus;
	CW3SIMenus		*m_pSIMenus;
	CW3EndoMenus	*m_pEndoMenus;
	CW3TmjMenus		*m_pTmjMenus;
	CW3OrthoMenus	*m_pOrthoMenus;

	EVIEW_TYPE	m_eSelectedView;
	W3INT	m_nRow;
	W3INT	m_nCol;
	
	W3BOOL	m_bOTF;
	
	W3BOOL	m_bMPRMask;
	
	W3BOOL	m_bManualSmartPainter;
	W3BOOL	m_bTempGL;

	W3INT m_SelectedAnno;

	QDoubleValidator *m_pDoubleValidator;
};

