#include "../../Common/Common/W3Memory.h"

#include "W3MenuBar.h"
#include <qpalette.h>
#include <iostream>

#include <QRegExp>	// by jdk 160512
#include <QValidator>	// by jdk 160512

CW3MenuBar::CW3MenuBar(QWidget *parent)
	: QWidget(parent),
	m_nCol(0), m_nRow(0), m_eSelectedView(EVIEW_TYPE::VR_MPR)
{
	ui.setupUi(this);

	QSizePolicy SizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	SizePolicy.setHorizontalStretch(0);
	SizePolicy.setVerticalStretch(0);
	setSizePolicy(SizePolicy);

	this->setFixedWidth(MENUBAR_SIZE_LEFT); //skpark 20151104 Demo del
	 

	m_pCommonMenus = nullptr;
	m_pMPRMenus = nullptr;
	m_pPanoMenus = nullptr;
	m_pImplantMenus = nullptr;
	m_pFaceMenus = nullptr;
	m_pSIMenus = nullptr;
	m_pEndoMenus = nullptr;
	m_pTmjMenus = nullptr;
	m_pOrthoMenus = nullptr;

	m_pCommonMenus = new CW3CommonMenus();
	m_pCommonMenus->m_btnViewFit = ui.m_btnViewFit;
	m_pCommonMenus->m_btnViewPanning = ui.m_btnViewPanning;
	m_pCommonMenus->m_btnViewReset = ui.m_btnViewResetMPR;
	m_pCommonMenus->m_btnViewZooming = ui.m_btnViewZooming;
	
	// by jdk 160524
	m_pCommonMenus->m_btnMeasureRuler = ui.m_btnMeasureRuler;
	m_pCommonMenus->m_btnMeasureTapeLine = ui.m_btnMeasureTapeLine;
	m_pCommonMenus->m_btnMeasureTapeCurve = ui.m_btnMeasureTapeCurve;
	m_pCommonMenus->m_btnMeasureAngle = ui.m_btnMeasureAngle;
	m_pCommonMenus->m_btnMeasureProfile = ui.m_btnMeasureProfile;
	m_pCommonMenus->m_btnMeasureArea = ui.m_btnMeasureArea;
	m_pCommonMenus->m_btnMeasureROI = ui.m_btnMeasureROI;
	m_pCommonMenus->m_btnMeasureDelete = ui.m_btnMeasureDelete;
	m_pCommonMenus->m_btnMeasureDeleteAll = ui.m_btnMeasureDeleteAll;

	m_pCommonMenus->m_btnDrawRectangle = ui.m_btnDrawRectangle;
	m_pCommonMenus->m_btnDrawCircle = ui.m_btnDrawCircle;
	m_pCommonMenus->m_btnDrawArrow = ui.m_btnDrawArrow;
	m_pCommonMenus->m_btnDrawFreeDraw = ui.m_btnDrawFreeDraw;
	m_pCommonMenus->m_btnDrawNote = ui.m_btnDrawNote;
	m_pCommonMenus->m_btnDrawDelete = ui.m_btnDrawDelete;
	m_pCommonMenus->m_btnDrawDeleteAll = ui.m_btnDrawDeleteAll;
	// jdk end

	m_pCommonMenus->connections();

	m_pMPRMenus = new CW3MPRMenus();
	m_pMPRMenus->m_btnAutoReOri = ui.m_btnReOrionoffMPR;
	m_pMPRMenus->m_btnOTFonoff = ui.m_btnOTFonoffMPR;
	m_pMPRMenus->m_btnVRCut = ui.m_btnVRCut;	// by jdk 160609
	m_pMPRMenus->m_checkBoxNerve = ui.m_checkBoxNerveMPR;
	m_pMPRMenus->m_checkBoxImplant = ui.m_checkBoxImplantMPR;
	m_pMPRMenus->m_checkBoxFace = ui.m_checkBoxFaceMPR;
	m_pMPRMenus->m_checkBoxSecond = ui.m_checkBoxSecondMPR;
	m_pMPRMenus->m_sliderPhotoTransparency = ui.m_sliderPhotoTransparencyMPR;

	m_pPanoMenus = new CW3PanoMenus();
	m_pPanoMenus->m_btnAutoArch = ui.m_btnAutoArch;
	m_pPanoMenus->m_btnOTFonoff = ui.m_btnOTFonoffPAN;
	m_pPanoMenus->m_btnApplyOri = ui.m_btnApplyOri;
	m_pPanoMenus->m_checkBoxNerve = ui.m_checkBoxNervePANO;
	m_pPanoMenus->m_checkBoxImplant = ui.m_checkBoxImplantPANO;

	m_pImplantMenus = new CW3ImplantMenus();
	m_pImplantMenus->m_btnOTFonoff = ui.m_btnOTFonoffIMPLANT;
	m_pImplantMenus->m_checkBoxNerve = ui.m_checkBoxNerveIMPLANT;
	m_pImplantMenus->m_checkBoxImplant = ui.m_checkBoxImplantIMPLANT;

	m_pFaceMenus = new CW3FaceMenus();
	m_pFaceMenus->m_btnPhotoOpen = ui.m_btnPhotoOpen;
	m_pFaceMenus->m_btnRegiPoints = ui.m_btnRegiPoints;
	m_pFaceMenus->m_btnSURFACINGonoffFACE = ui.m_btnSURFACINGonoffFACE;
	m_pFaceMenus->m_btnOTFonoff = ui.m_btnOTFonoffFACE;
	m_pFaceMenus->m_checkBoxFace = ui.m_checkBoxFaceFACE;
	m_pFaceMenus->m_sliderPhotoTransparency = ui.m_sliderPhotoTransparencyFACE;

	m_pSIMenus = new CW3SIMenus();
	m_pSIMenus->m_btnDicomLoad = ui.m_btnDicomOpenFolderSI;
	m_pSIMenus->m_btnOTFonoff = ui.m_btnOTFonoffSI;
	m_pSIMenus->m_rbtnMainVol = ui.m_rbtnMainVolSI;
	m_pSIMenus->m_rbtnSecondVol = ui.m_rbtnSecVolSI;
	m_pSIMenus->m_rbtnBothVol = ui.m_rbtnBothVolSI;

	m_pTmjMenus = new CW3TmjMenus();
	m_pTmjMenus->m_btnOTFonoff = ui.m_btnOTFonoffTMJ;
	m_pTmjMenus->m_checkBoxNerve = ui.m_checkBoxNerveTMJ;
	m_pTmjMenus->m_checkBoxImplant = ui.m_checkBoxImplantTMJ;

	m_pTmjMenus->m_editROIWidthLeft = ui.m_editROIWidthLeft;
	m_pTmjMenus->m_editROIHeightLeft = ui.m_editROIHeightLeft;
	m_pTmjMenus->m_editROIWidthRight = ui.m_editROIWidthRight;
	m_pTmjMenus->m_editROIHeightRight = ui.m_editROIHeightRight;
	m_pTmjMenus->m_editVSliceGapLeft = ui.m_editVSliceGapLeft;
	m_pTmjMenus->m_editVSliceGapRight = ui.m_editVSliceGapRight;
	m_pTmjMenus->m_editVSliceThicknessLeft = ui.m_editVSliceThicknessLeft;
	m_pTmjMenus->m_editVSliceThicknessRight = ui.m_editVSliceThicknessRight;

	/*QRegExp rx("^[0-9]*S");
	QValidator *validator = new QRegExpValidator(rx, m_pTmjMenus->m_editROIWidthLeft);*/
	W3DOUBLE bottom = 0.0;
	W3DOUBLE top = 100.0;
	W3INT dec = 2;

	m_pDoubleValidator = new QDoubleValidator(bottom, top, dec);
	m_pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);

	m_pTmjMenus->m_editROIWidthLeft->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editROIHeightLeft->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editROIWidthRight->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editROIHeightRight->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editVSliceGapLeft->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editVSliceGapRight->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editVSliceThicknessLeft->setValidator(m_pDoubleValidator);
	m_pTmjMenus->m_editVSliceThicknessRight->setValidator(m_pDoubleValidator);

	m_pTmjMenus->m_editVSliceThicknessLeft->setText("0");
	m_pTmjMenus->m_editVSliceThicknessRight->setText("0");

	m_pOrthoMenus = new CW3OrthoMenus();
	m_pOrthoMenus->m_btnOTFonoff = ui.m_btnOTFonoffORTHO;
	m_pOrthoMenus->m_checkBoxNerve = ui.m_checkBoxNerveORTHO;
	m_pOrthoMenus->m_checkBoxImplant = ui.m_checkBoxImplantORTHO;

	//////// Endo Menus
	m_pEndoMenus = new CW3EndoMenus();
	m_pEndoMenus->m_btnOTFonoff = ui.m_btnOTFonoffENDO;
	m_pEndoMenus->m_btnFREEonoffENDO = ui.m_btnFREEonoffENDO;

	m_pEndoMenus->m_btnPlayENDO = ui.m_btnPlayENDO;
	m_pEndoMenus->m_btnStopENDO = ui.m_btnStopENDO;
	m_pEndoMenus->m_btnStartENDO = ui.m_btnStartENDO;
	m_pEndoMenus->m_btnEndENDO = ui.m_btnEndENDO;
	m_pEndoMenus->m_btnPrevENDO = ui.m_btnPrevENDO;
	m_pEndoMenus->m_btnNextENDO = ui.m_btnNextENDO;

	m_pEndoMenus->m_sldIntervalENDO = ui.m_sldIntervalENDO;
	m_pEndoMenus->m_sldSpeedENDO = ui.m_sldSpeedENDO;

	m_pEndoMenus->m_rbtnCameraForwardENDO = ui.m_radioCameraForwardENDO;
	m_pEndoMenus->m_rbtnCameraBackwardENDO = ui.m_radioCameraBackwardENDO;
	m_pEndoMenus->m_rbtnCameraFixedENDO = ui.m_radioCameraFixedENDO;

	m_pEndoMenus->m_btnPath1ENDO = ui.m_btnPath1ENDO;
	m_pEndoMenus->m_btnPath2ENDO = ui.m_btnPath2ENDO;
	m_pEndoMenus->m_btnPath3ENDO = ui.m_btnPath3ENDO;
	m_pEndoMenus->m_btnPath4ENDO = ui.m_btnPath4ENDO;
	m_pEndoMenus->m_btnPath5ENDO = ui.m_btnPath5ENDO;

	m_pEndoMenus->m_btnPath1DelENDO = ui.m_btnPath1DelENDO;
	m_pEndoMenus->m_btnPath2DelENDO = ui.m_btnPath2DelENDO;
	m_pEndoMenus->m_btnPath3DelENDO = ui.m_btnPath3DelENDO;
	m_pEndoMenus->m_btnPath4DelENDO = ui.m_btnPath4DelENDO;
	m_pEndoMenus->m_btnPath5DelENDO = ui.m_btnPath5DelENDO;

	// Initialize states.
	m_bOTF = false;
	m_bMPRMask = false;
	m_bManualSmartPainter = false;
	
	connections();

	//thyoo	
	ui.tabWidget->setCurrentIndex(TAB_IDX_FILE);
}

CW3MenuBar::~CW3MenuBar()
{
	SAFE_DELETE_OBJECT(m_pCommonMenus);
	SAFE_DELETE_OBJECT(m_pMPRMenus);
	SAFE_DELETE_OBJECT(m_pPanoMenus);
	SAFE_DELETE_OBJECT(m_pImplantMenus);
	SAFE_DELETE_OBJECT(m_pFaceMenus);
	SAFE_DELETE_OBJECT(m_pEndoMenus);
	SAFE_DELETE_OBJECT(m_pTmjMenus);
	SAFE_DELETE_OBJECT(m_pOrthoMenus);

	SAFE_DELETE_OBJECT(m_pDoubleValidator);

	disconnections();
}

void CW3MenuBar::setMaskTree(QWidget* pMaskTree)
{
	pMaskTree->setVisible(true);
}

void CW3MenuBar::connections(void)
{
	//connect(ui.m_btnOTFonoffMPR, SIGNAL(clicked()), this, SIGNAL(sigOTFonoffMPR()));
	//connect(ui.m_btnOTFonoffPAN, SIGNAL(clicked()), this, SIGNAL(sigOTFonoffPAN()));
	//connect(ui.m_btnOTFonoffFACE, SIGNAL(clicked()), this, SIGNAL(sigOTFonoffFACE()));
	//connect(ui.m_btnReOrionoffMPR, SIGNAL(clicked()), this, SIGNAL(sigReOrientation()));
	
	// Panorama
	//connect(ui.m_btnAutoArch, SIGNAL(clicked()), this, SIGNAL(sigAutoArch()));

	// Face Simluation
	//connect(ui.m_btnSURFACINGonoffFACE, SIGNAL(clicked()), this, SIGNAL(sigGenSURFACE()));
	//connect(ui.m_btnPhotoOpen, SIGNAL(clicked()), this, SIGNAL(sigPhotoOpen()));
	//connect(ui.m_btnRegiPoints, SIGNAL(clicked()), this, SIGNAL(sigRegiPoints()));

	//sig & slot connections for DBM
	//connect(ui.m_btnDicomOpen, SIGNAL(clicked()), this, SLOT(slotLoadDICOM(void)));
	//connect(ui.m_btnDicomOpenFolder, SIGNAL(clicked()), this, SLOT(slotLoadDICOMFolder(void)));
	

	//sig & slot connections for MPR tab
	//connect(ui.m_btnViewResetMPR, SIGNAL(clicked()), this, SIGNAL(sigResetMPR(void)));
	//connect(ui.m_btnViewFit, SIGNAL(clicked()), this, SIGNAL(sigFitMPR(void)));
	//connect(ui.m_btnViewPanning, SIGNAL(clicked()), this, SLOT(slotSetPanning(void)));
	//connect(ui.m_btnViewZooming, SIGNAL(clicked()), this, SLOT(slotSetZooming(void)));

	//for minimize menu bar
	connect(ui.tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(slotMenuClicked(int)));

	
	// by jdk 150714
	connect(ui.m_btnVOI, SIGNAL(clicked()), this, SLOT(slotActiveVOI(void)));
	// jdk end

	// by jdk 150722
	connect(ui.m_btnZoom3D, SIGNAL(clicked()), this, SLOT(slotActiveZoom3D(void)));
	// jdk end
}

void CW3MenuBar::disconnections(void)
{
	//sig & slot connections for DBM
	//disconnect(ui.m_btnDicomOpen, SIGNAL(clicked()), this, SLOT(slotLoadDICOM(void)));
	//disconnect(ui.m_btnDicomOpenFolder, SIGNAL(clicked()), this, SLOT(slotLoadDICOMFolder(void)));
	
	//for minimize menu bar
	disconnect(ui.tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(slotMenuClicked(int)));
}

void CW3MenuBar::slotLoadDICOM(void)
{
	//ui.m_btnDicomOpen->setDefault(false);
	//emit sigLoadDicom();
}

void CW3MenuBar::slotLoadDICOMFolder(void)
{
	//ui.m_btnDicomOpenFolder->setDefault(false);
	//emit sigLoadDicomFolder();
}

// by jdk 150714
void CW3MenuBar::slotActiveVOI(void)
{
	ui.m_btnVOI->setDefault(false);
	emit sigActiveVOI();
}
// jdk end

// by jdk 150722
void CW3MenuBar::slotActiveZoom3D(void)
{
	ui.m_btnZoom3D->setDefault(false);
	emit sigActiveZoom3D();
}
// jdk end

void CW3MenuBar::slotSetVolume(ETAB_TYPE eType)
{
	//switch(eType)
	//{
	//case ETAB_TYPE::TAB_MPR:
	//	ui.tabWidget->setCurrentIndex(TAB_IDX_MPR);
	//	break;
	//default:
	//	break;
	//}
}

//void CW3MenuBar::slotGoToMPR()
//{
//	ui.tabWidget->setCurrentIndex(TAB_IDX_MPR);
//	//slotMenuClicked(TAB_IDX_MPR);
//}
//
//// by jdk 160706
//void CW3MenuBar::slotGoToFILE()
//{
//	ui.tabWidget->setCurrentIndex(TAB_IDX_FILE);
//	//slotMenuClicked(TAB_IDX_MPR);
//}

// by jdk 160707
void CW3MenuBar::slotGoTo(ETAB_TYPE tab)
{
	ui.tabWidget->setCurrentIndex(tab);
}

void CW3MenuBar::slotMenuClicked(int tabIndex)
{
	if(tabIndex == TAB_IDX_DISABLED)
		return;

	if (tabIndex == TAB_IDX_FILE){
		emit sigChangeTab(ETAB_TYPE::TAB_FILE);
	}
	else if(tabIndex == TAB_IDX_MPR){
		emit sigChangeTab(ETAB_TYPE::TAB_MPR);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
		//this->setFixedWidth(MENUBAR_SIZE_LEFT); //skpark 20151104 Demo del
	}
	else if (tabIndex == TAB_IDX_PANORAMA) 
	{
		emit sigChangeTab(ETAB_TYPE::TAB_PANORAMA);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
		//this->setFixedWidth(MENUBAR_SIZE_LEFT); //skpark 20151104 Demo del
	}
	else if (tabIndex == TAB_IDX_IMPLANT)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_IMPLANT);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
		//this->setFixedWidth(MENUBAR_SIZE_LEFT); //skpark 20151104 Demo del
	}
	else if (tabIndex == TAB_IDX_FACESIM)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_FACESIM);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
	}
	else if (tabIndex == TAB_IDX_SUPERIMPOSE)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_SI);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
	}
	else if (tabIndex == TAB_IDX_ENDO)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_ENDO);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
	}
	else if (tabIndex == TAB_IDX_3DCEPH)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_3DCEPH);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
	}
	else if (tabIndex == TAB_IDX_TMJ)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_TMJ);
	}
	else if (tabIndex == TAB_IDX_ORTHODONTIC)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_ORTHODONTIC);
	}
	else if (tabIndex == TAB_IDX_REPORT)
	{
		emit sigChangeTab(ETAB_TYPE::TAB_REPORT);
		//emit sigSetMenuSize(MENUBAR_SIZE_LEFT);
	}
}
