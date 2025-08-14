#include "W3ViewMenuBarPath.h"

CW3ViewMenuBarPath::CW3ViewMenuBarPath(EVIEW_TYPE eViewType, QWidget *parent)
	: CW3ViewMenuBar(eViewType, parent)
{
	m_pCbxViewType->addItem("CPR");
	m_pCbxViewType->addItem("Path");
	this->displayViewType();

	ui.m_btnReset->setVisible(false);
	ui.m_btnOverlay->setVisible(false);
	ui.m_btnCapture->setVisible(false);
	ui.m_btnFit->setVisible(true);

	setConnections();
}

CW3ViewMenuBarPath::~CW3ViewMenuBarPath()
{
	this->setDisconnections();
}

void CW3ViewMenuBarPath::setConnections(void)
{
	
}
void CW3ViewMenuBarPath::setDisconnections(void)
{
	
}

void CW3ViewMenuBarPath::slotSwapWindow(int idx)
{
	EVIEW_TYPE eSwap;
	switch (idx){
	case 0:	eSwap = EVIEW_TYPE::CPR;	break;
	case 1:	eSwap = EVIEW_TYPE::PATH;	break;
	}

	if(m_eViewType == eSwap)
		return;

	emit sigSwapWindow(m_eViewType, eSwap);
	displayViewType();
}

void CW3ViewMenuBarPath::setViewType(EVIEW_TYPE eType)
{
	m_eViewType = eType;
	displayViewType();
}

void CW3ViewMenuBarPath::displayViewType(void)
{
	switch (m_eViewType){
	case EVIEW_TYPE::CPR:		m_pCbxViewType->setCurrentIndex(0);	break;
	case EVIEW_TYPE::PATH:		m_pCbxViewType->setCurrentIndex(1);	break;
	}
}
