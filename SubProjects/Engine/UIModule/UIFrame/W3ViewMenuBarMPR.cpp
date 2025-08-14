#include "W3ViewMenuBarMPR.h"


CW3ViewMenuBarMPR::CW3ViewMenuBarMPR(EVIEW_TYPE eViewType, QWidget *parent)
	: CW3ViewMenuBar(eViewType, parent)
{
	m_bLineOn = true;
	m_btnLineOnOff = new QPushButton;
	m_pLayout->addWidget(m_btnLineOnOff);
	m_btnLineOnOff->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	m_btnLineOnOff->setIcon(QIcon("./Images/Button/lineOn.png"));

	if(	m_eViewType == EVIEW_TYPE::MPR_AXIAL || 
		m_eViewType == EVIEW_TYPE::MPR_SAGITTAL || 
		m_eViewType == EVIEW_TYPE::MPR_CORONAL )
		m_btnLineOnOff->setVisible(true);
	else
		m_btnLineOnOff->setVisible(false);

	m_pCbxViewType->addItem("axial");
	m_pCbxViewType->addItem("sagittal");
	m_pCbxViewType->addItem("coronal");
	m_pCbxViewType->addItem("VR");
	this->displayViewType();

	setConnections();
}

CW3ViewMenuBarMPR::~CW3ViewMenuBarMPR()
{
	this->setDisconnections();
}

void CW3ViewMenuBarMPR::setConnections(void)
{
	connect(m_btnLineOnOff, SIGNAL(clicked()), this, SLOT(slotLineOn()));
}
void CW3ViewMenuBarMPR::setDisconnections(void)
{
	disconnect(m_btnLineOnOff, SIGNAL(clicked()), this, SLOT(slotLineOn()));
}

void CW3ViewMenuBarMPR::slotSwapWindow(int idx)
{
	EVIEW_TYPE eSwap;
	switch (idx){
	case 0:	eSwap = EVIEW_TYPE::MPR_AXIAL;		break;
	case 1:	eSwap = EVIEW_TYPE::MPR_SAGITTAL;	break;
	case 2:	eSwap = EVIEW_TYPE::MPR_CORONAL;	break;
	case 3:	eSwap = EVIEW_TYPE::VR_MPR;				break;
	}

	if(m_eViewType == eSwap)
		return;

	emit sigSwapWindow(m_eViewType, eSwap);
	displayViewType();
}
void CW3ViewMenuBarMPR::slotLineOn(void)
{
	m_bLineOn ^= true;
	m_btnLineOnOff->setFlat(!m_bLineOn);

	emit sigLineOn(m_bLineOn);
}

void CW3ViewMenuBarMPR::slotSyncLineOn(W3BOOL bFlag)
{
	m_bLineOn = bFlag;
	m_btnLineOnOff->setFlat(!m_bLineOn);
}
void CW3ViewMenuBarMPR::setViewType(EVIEW_TYPE eType)
{
	m_eViewType = eType;
	displayViewType();
}
void CW3ViewMenuBarMPR::displayViewType(void)
{
	switch (m_eViewType){
	case EVIEW_TYPE::MPR_AXIAL:		m_pCbxViewType->setCurrentIndex(0);	break;
	case EVIEW_TYPE::MPR_SAGITTAL:	m_pCbxViewType->setCurrentIndex(1);	break;
	case EVIEW_TYPE::MPR_CORONAL:	m_pCbxViewType->setCurrentIndex(2);	break;
	case EVIEW_TYPE::VR_MPR:			m_pCbxViewType->setCurrentIndex(3);	break;
	}
}
