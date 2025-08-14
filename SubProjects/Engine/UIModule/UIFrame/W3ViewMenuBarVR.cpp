#include "W3ViewMenuBarVR.h"

CW3ViewMenuBarVR::CW3ViewMenuBarVR(EVIEW_TYPE eViewType, QWidget *parent)
	: CW3ViewMenuBar(eViewType, parent)
{
	m_pCbxViewType->addItem("axial");
	m_pCbxViewType->addItem("sagittal");
	m_pCbxViewType->addItem("coronal");
	m_pCbxViewType->addItem("VR");
	this->displayViewType();

	setConnections();
}

CW3ViewMenuBarVR::~CW3ViewMenuBarVR()
{
	setDisconnections();
}

void CW3ViewMenuBarVR::setConnections(void)
{
}
void CW3ViewMenuBarVR::setDisconnections(void)
{
}

void CW3ViewMenuBarVR::slotSwapWindow(int idx)
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

void CW3ViewMenuBarVR::setViewType(EVIEW_TYPE eType)
{
	m_eViewType = eType;
	displayViewType();
}

void CW3ViewMenuBarVR::displayViewType(void)
{
	switch (m_eViewType){
	case EVIEW_TYPE::MPR_AXIAL:		m_pCbxViewType->setCurrentIndex(0);	break;
	case EVIEW_TYPE::MPR_SAGITTAL:	m_pCbxViewType->setCurrentIndex(1);	break;
	case EVIEW_TYPE::MPR_CORONAL:	m_pCbxViewType->setCurrentIndex(2);	break;
	case EVIEW_TYPE::VR_MPR:			m_pCbxViewType->setCurrentIndex(3);	break;
	}
}
