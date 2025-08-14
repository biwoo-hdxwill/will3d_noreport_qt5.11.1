/*=========================================================================

File:			class CW3MPRMTMJ
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			PARK, SANGKEUN
First Date:		2015-07-22


Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.k

=========================================================================*/

#include "W3ReOrientationDlg.h"
#include "../../Module/W3MPRMod/W3MPRMod.h"


#include <qdebug.h>

CW3ReOrientationDlg::CW3ReOrientationDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);	

	this->setFixedWidth(DLG_WIDTH);
	this->setFixedHeight(DLG_HEIGHT);

	for(W3INT i=0; i <EMPR_VIEW_TYPE::mpr_end ; i++)
		m_pWindowMPR[i] = nullptr;

	//m_pWindowVR = nullptr;
	m_pMPRModule = nullptr;

	m_pBtnOk		= nullptr;
	m_pBtnCancel	= nullptr;	
	m_pBtnReset		= nullptr;
	m_pBtnRotation	= nullptr;
	m_pBtnZoom		= nullptr;
	m_pBtnPan		= nullptr;
	
	createObject();
	initLayout();
	connection();

	m_pBtnOk->setFixedSize(BTN_WIDTH, BTN_HEIGHT);
	m_pBtnCancel->setFixedSize(BTN_WIDTH, BTN_HEIGHT);
	m_pBtnReset->setFixedSize(BTN_WIDTH,BTN_HEIGHT);
	m_pBtnRotation->setFixedSize(BTN_WIDTH,BTN_HEIGHT);
	m_pBtnZoom->setFixedSize(BTN_WIDTH,BTN_HEIGHT);
	m_pBtnPan->setFixedSize(BTN_WIDTH,BTN_HEIGHT);	

}

void CW3ReOrientationDlg::setVolume(CW3Image3D *volume)
{
	if(volume == nullptr) return;
	
	m_pMPRModule->setVolume(volume);
	m_pMPRModule->initModule();	
}
CW3ReOrientationDlg::~CW3ReOrientationDlg()
{
	SAFE_DELETE_OBJECT(m_pMPRModule);
	//SAFE_DELETE_OBJECT(m_pWindowVR);	

	for(W3INT i=0; i < EMPR_VIEW_TYPE::mpr_end; i++)
		SAFE_DELETE_OBJECT(m_pWindowMPR[i]);

}

void CW3ReOrientationDlg::connection(void)
{
	for(W3INT i=0; i <EMPR_VIEW_TYPE::mpr_end; i++)
	{
		connect(m_pWindowMPR[i],SIGNAL(sigRotate(const EMPR_VIEW_TYPE, const W3FLOAT)),this, SLOT(slotRotate(const EMPR_VIEW_TYPE, const W3FLOAT)));
		connect(m_pWindowMPR[i],SIGNAL(sigWheelEvent(const EMPR_VIEW_TYPE, const W3INT)),this, SLOT(slotWheelEvent(const EMPR_VIEW_TYPE, const W3INT)));
	}

	connect(m_pBtnReset,SIGNAL(clicked()),this,SLOT(slotResetBtn()));
	connect(m_pBtnRotation,SIGNAL(clicked()),this,SLOT(slotRotationBtn()));
	connect(m_pBtnZoom,SIGNAL(clicked()),this,SLOT(slotZoomBtn()));
	connect(m_pBtnPan,SIGNAL(clicked()),this,SLOT(slotPanBtn()));

	connect(m_pBtnCancel,SIGNAL(clicked()),this,SLOT(slotCancelBtn()));
	connect(m_pBtnOk,SIGNAL(clicked()),this,SLOT(slotOkBtn()));

	connect(m_pCheckboxGrid, SIGNAL(stateChanged(int)), this, SLOT(slotCheckboxGrid(int)));
}

void CW3ReOrientationDlg::createObject(void)
{	
	for(W3INT i=0; i <EMPR_VIEW_TYPE::mpr_end ; i++)
		m_pWindowMPR[i] = new CW3WindowReOrientationDlg(i, (EVIEW_TYPE)i, ECONTAINER_TYPE::MAIN, this);

	m_pMPRModule	= new CW3MPRMod;
	//m_pWindowVR		= new CW3WindowVR(ECONTAINER_TYPE::MAIN, this);

	m_pBtnReset		= new QPushButton(tr("Reset"));	
	m_pBtnRotation	= new QPushButton(tr("Rotation"));	
	m_pBtnRotation->setCheckable(true);
	m_pBtnRotation->setChecked(true);

	m_pBtnZoom		= new QPushButton(tr("Zoom"));	
	m_pBtnZoom->setCheckable(true);
	m_pBtnPan		= new QPushButton(tr("Pan"));
	m_pBtnPan->setCheckable(true);
	m_pBtnOk		= new QPushButton(tr("OK"));	
	m_pBtnCancel	= new QPushButton(tr("Cancel"));
	
	m_pCheckboxGrid = new QCheckBox(tr("Show Grids"));	
}

void CW3ReOrientationDlg::initLayout(void)
{	
	QHBoxLayout *pWindowLayout = new QHBoxLayout;
	pWindowLayout->addWidget(m_pWindowMPR[EMPR_VIEW_TYPE::axial]);
	pWindowLayout->addWidget(m_pWindowMPR[EMPR_VIEW_TYPE::sagittal]);
	//pWindowLayout->addWidget(m_pWindowVR);
	pWindowLayout->addWidget(m_pWindowMPR[EMPR_VIEW_TYPE::coronal]);
	
	QHBoxLayout *pButtonLayout_L = new QHBoxLayout;
	pButtonLayout_L->addWidget(m_pBtnReset);
	pButtonLayout_L->addWidget(m_pBtnRotation);
	pButtonLayout_L->addWidget(m_pBtnZoom);
	pButtonLayout_L->addWidget(m_pBtnPan);
	pButtonLayout_L->setAlignment(Qt::AlignLeft);
	
	QHBoxLayout *pButtonLayout_R = new QHBoxLayout;
	pButtonLayout_R->addWidget(m_pCheckboxGrid);
	pButtonLayout_R->addWidget(m_pBtnOk);
	pButtonLayout_R->addWidget(m_pBtnCancel);
	pButtonLayout_R->setAlignment(Qt::AlignRight);


	QHBoxLayout *pButtonLayout = new QHBoxLayout;
	pButtonLayout->addLayout(pButtonLayout_L);
	pButtonLayout->addLayout(pButtonLayout_R);


	QVBoxLayout* pMainLayout = new QVBoxLayout;
	pMainLayout->addLayout(pWindowLayout);
	pMainLayout->addLayout(pButtonLayout);

	m_pWindowMPR[EMPR_VIEW_TYPE::axial]->setVisible(true);
	m_pWindowMPR[EMPR_VIEW_TYPE::sagittal]->setVisible(true);
	//m_pWindowVR->setVisible(true);
	m_pWindowMPR[EMPR_VIEW_TYPE::coronal]->setVisible(true);

	setLayout(pMainLayout);
}

void CW3ReOrientationDlg::initDisplay()
{
	CW3Image2D* img = nullptr;
	CW3Image3D* pVol = nullptr;

	if(checkObject() == true)
	{		
		pVol = m_pMPRModule->getVolume();
		for(W3INT i=0; i < EMPR_VIEW_TYPE::mpr_end; i++)
		{		
			img = m_pMPRModule->returnImage((EMPR_VIEW_TYPE)i);
			m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setImage(img);
			m_pWindowMPR[(EMPR_VIEW_TYPE)i]->initializeViewFromData(img->width(), img->height(), 0,
												pVol->windowWidth(), pVol->windowCenter(),
												pVol->intercept(), pVol->slope(), 
												m_pMPRModule->getIntensityMin(), m_pMPRModule->getIntensityMax(), 
												pVol->getHeader()->getListCore());	

			m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setMPRMoudleVectors(m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)i), m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)i), m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)i));
		
			m_pWindowMPR[(EMPR_VIEW_TYPE)i]->moveItems();
			m_pWindowMPR[(EMPR_VIEW_TYPE)i]->display();

			//skpark test 20150727
			//qDebug() <<"Plane: " <<i <<"\n";
			//qDebug() <<"Back vector:" <<m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)i) <<"\n";
			//qDebug() <<"Right vector:" <<m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)i) <<"\n";
			//qDebug() <<"Up vector:" <<m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)i) <<"\n";

			//DICOM 정보를 Default로 사용하지 않을 경우 사용
			//m_pWindowMPR[(EMPR_VIEW_TYPE)i]->showOverlay(true);
		}
	}
}

W3BOOL CW3ReOrientationDlg::checkObject()
{
	if(m_pMPRModule == nullptr)
		return false;

	for(W3INT i = 0; i <EMPR_VIEW_TYPE::mpr_end; i++)
	{
		if(m_pWindowMPR[(EMPR_VIEW_TYPE)i] == nullptr)
			return false;
	}

	return true;
}

void CW3ReOrientationDlg::slotRotate(const EMPR_VIEW_TYPE eViewType, const W3FLOAT fAngle)
{
	//CW3Vector3D vNorm, vPt;
	
	CW3Point2DF vD(0.0f,0.0f);

	vD = m_pMPRModule->rotateView(eViewType, eViewType, fAngle*180/M_PI);
	m_pWindowMPR[(EMPR_VIEW_TYPE)eViewType]->moveCenterAsRotate();
	QPointF viewCenterPt = m_pWindowMPR[(EMPR_VIEW_TYPE)eViewType]->getPixmapCenterPt();
	m_pWindowMPR[(EMPR_VIEW_TYPE)eViewType]->setCenterPt(viewCenterPt + vD);
	m_pWindowMPR[(EMPR_VIEW_TYPE)eViewType]->setMPRMoudleVectors(m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)eViewType), m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)eViewType), m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)eViewType));
	

	for(W3INT kView = 0; kView < EMPR_VIEW_TYPE::mpr_end; kView++)
    {
        EMPR_VIEW_TYPE viewType = (EMPR_VIEW_TYPE)kView;	

		if(eViewType != viewType)
		{
			vD = m_pMPRModule->rotateView(eViewType, viewType, fAngle*180/M_PI);
			m_pWindowMPR[(EMPR_VIEW_TYPE)kView]->moveCenterAsRotate();
			QPointF viewCenterPt = m_pWindowMPR[(EMPR_VIEW_TYPE)kView]->getPixmapCenterPt();
			m_pWindowMPR[(EMPR_VIEW_TYPE)kView]->setCenterPt(viewCenterPt + vD);

			//skpark 20150727
			m_pWindowMPR[(EMPR_VIEW_TYPE)kView]->setMPRMoudleVectors(m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)viewType), m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)viewType), m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)viewType));
			
		}

		// VR rendering 사용시 사용
		//vNorm = m_pMPRMod->getPlaneNormal((EMPR_VIEW_TYPE)kView);
		//vPt = m_pMPRMod->getPlaneCenter((EMPR_VIEW_TYPE)kView);
		//emit sigChangeMPRDisplay((EMPR_PLANE)kView, vPt, vNorm);	

		////skpark test 20150727
		//qDebug() <<"Plane: " <<viewType <<"\n";
		//qDebug() <<"Back vector:" <<m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)viewType) <<"\n";
		//qDebug() <<"Right vector:" <<m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)viewType) <<"\n";
		//qDebug() <<"Up vector:" <<m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)viewType) <<"\n";
		////end


    }

	displayMPR();
}

void CW3ReOrientationDlg::slotWheelEvent(const EMPR_VIEW_TYPE eViewType, const W3INT delta)
{
	W3FLOAT fStepSize = ( delta > 0 ? std::abs(1) : -std::abs(1) );
	W3FLOAT fDelta = delta;
	if( delta > 100 || delta < -100 ) fDelta = delta/100;
	W3FLOAT fittedDist = m_pMPRModule->translateView(eViewType, fStepSize*std::abs(fDelta));
	std::cout << "fDelta : " << fDelta << std::endl;
	std::cout << "fittedDist : " << fittedDist << std::endl;
	if(fittedDist == 0)
		return;

	displayMPR();
}

void CW3ReOrientationDlg::displayMPR()
{		
	CW3Image2D* img;
	W3BYTE* mask;
	for(W3INT i = 0 ; i < EMPR_VIEW_TYPE::mpr_end; i++)
	{
		img = m_pMPRModule->returnImage((EMPR_VIEW_TYPE)i);
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setImage(img);
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->moveItems();		
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->display();
	}	
}

void CW3ReOrientationDlg::slotOkBtn()
{
	CW3Vector3D vX = m_pMPRModule->getPlaneRight(EMPR_VIEW_TYPE::axial);
	CW3Vector3D vY = m_pMPRModule->getPlaneBack(EMPR_VIEW_TYPE::axial);
	CW3Vector3D vZ = m_pMPRModule->getPlaneNormal(EMPR_VIEW_TYPE::axial);	

	emit sigReOrientationVector(vX, vY,vZ);
	
	//this->reset();
	//this->setVisible(false);
}

void CW3ReOrientationDlg::slotCancelBtn()
{
	this->reset();
	this->setVisible(false);
}

void CW3ReOrientationDlg::slotResetBtn()
{
	this->reset();	
	this->initDisplay();
}

void CW3ReOrientationDlg::slotRotationBtn()
{
	m_pBtnZoom->setChecked(false);
	m_pBtnPan->setChecked(false);

	if(!m_pBtnRotation->isChecked())
	{
		m_pBtnRotation->setChecked(true);
	}

	W3BOOL bChecked = m_pBtnRotation->isChecked();

	for(W3INT i = 0 ; i < EMPR_VIEW_TYPE::mpr_end; i++)
	{
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setRotationMode(bChecked);
	}

}

void CW3ReOrientationDlg::slotZoomBtn()
{
	m_pBtnRotation->setChecked(false);
	m_pBtnPan->setChecked(false);

	if(!m_pBtnZoom->isChecked())
	{
		m_pBtnZoom->setChecked(true);
	}

	W3BOOL bChecked = m_pBtnZoom->isChecked();

	for(W3INT i = 0 ; i < EMPR_VIEW_TYPE::mpr_end; i++)
	{
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setZoomMode(bChecked);
	}
}

void CW3ReOrientationDlg::slotPanBtn()
{
	m_pBtnRotation->setChecked(false);
	m_pBtnZoom->setChecked(false);

	if(!m_pBtnPan->isChecked())
	{
		m_pBtnPan->setChecked(true);
	}

	W3BOOL bChecked = m_pBtnPan->isChecked();
	
	for(W3INT i = 0 ; i < EMPR_VIEW_TYPE::mpr_end; i++)
	{
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setPanMode(bChecked);
	}
}

void CW3ReOrientationDlg::slotCheckboxGrid(int state)//skpark 20150804
{
	for(W3INT i = 0 ; i < EMPR_VIEW_TYPE::mpr_end; i++)
	{
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setGridMode(state);
	}
	displayMPR();
}

void CW3ReOrientationDlg::reset()
{
	m_pMPRModule->initModule();		
	
	for(W3INT i = 0; i < EMPR_VIEW_TYPE::mpr_end; i++)
    {    
		m_pWindowMPR[(EMPR_VIEW_TYPE)i]->setMPRMoudleVectors(m_pMPRModule->getPlaneNormal((EMPR_VIEW_TYPE)i), m_pMPRModule->getPlaneBack((EMPR_VIEW_TYPE)i), m_pMPRModule->getPlaneRight((EMPR_VIEW_TYPE)i));
	}

	displayMPR();

}
