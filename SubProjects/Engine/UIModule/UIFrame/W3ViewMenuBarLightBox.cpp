#include "W3ViewMenuBarLightBox.h"


/*public:*/
CW3ViewMenuBarLightBox::CW3ViewMenuBarLightBox(EMPR_VIEW_TYPE eViewType, QWidget *parent/* = 0*/)
	: CW3ViewMenuBar((EVIEW_TYPE)eViewType, parent)
{
	m_pCbxViewType->addItem("axial");
	m_pCbxViewType->addItem("sagittal");
	m_pCbxViewType->addItem("coronal");
	this->displayViewType();
}

CW3ViewMenuBarLightBox::~CW3ViewMenuBarLightBox()
{
}

/*public slots:*/
/*virtual */void CW3ViewMenuBarLightBox::slotSwapWindow(int idx)
{	
	if(m_eViewType == (EVIEW_TYPE)idx)
		return;

	emit sigSwapWindow(m_eViewType, (EVIEW_TYPE)idx);
	displayViewType();
}


void CW3ViewMenuBarLightBox::setViewType(EVIEW_TYPE eType)
{
	m_eViewType = eType;
	displayViewType();
}

/*virtual */void CW3ViewMenuBarLightBox::displayViewType(void)
{
	m_pCbxViewType->setCurrentIndex((W3INT)m_eViewType);
}
