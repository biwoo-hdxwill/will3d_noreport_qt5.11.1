#include "W3FloatingTab.h"
#include "W3ViewLayout.h"
#include "W3GeneralMenu.h"
#include "W3FloatingMenuBar.h"
#include "../../Common/Common/W3Memory.h"

#include <qdesktopwidget.h>

CW3FloatingTab::CW3FloatingTab(QWidget *parent)
	: m_pParent(parent)
{
	ui.setupUi(this);

	QDesktopWidget widget;
	QRect mainScreenSize = widget.availableGeometry(widget.screen(0));
	QRect subScreenSize = widget.availableGeometry(widget.screen(1));
	this->setFixedWidth(subScreenSize.width() - 15);
	this->setFixedHeight(subScreenSize.height() - 35);

	this->move(mainScreenSize.width(),0);

	initLayout();
	connections();
}
void CW3FloatingTab::initLayout(void)
{
	m_viewLayoutFloating = new CW3ViewLayout(EVIEW_LAYOUT_TYPE::MPR, this);
	m_viewLayoutFloating->setVisible(true);
	m_generalMenuFloating = new CW3GeneralMenu(this);
	m_generalMenuFloating->setFixedHeight(GENERALMENU_FLOATING_SIZE_HIEGHT);
	m_menuBarFloating = new CW3FloatingMenuBar(this);

	QVBoxLayout *menuLayout =  new QVBoxLayout;
	menuLayout->setSpacing(LAYOUT_SPACING);
	menuLayout->addWidget(m_menuBarFloating);
	menuLayout->addWidget(m_generalMenuFloating);

	QHBoxLayout *mainLayout = new QHBoxLayout;
	mainLayout->setSpacing(LAYOUT_SPACING); 
	//mainLayout->addWidget(m_generalMenuFloating);
	mainLayout->addLayout(menuLayout);
	mainLayout->setStretch(0,1);
	mainLayout->addWidget(m_viewLayoutFloating);
	mainLayout->setStretch(1,4);

	QHBoxLayout* layout = (QHBoxLayout*)ui.m_layoutMain->layout();
	layout->addLayout(mainLayout);


}

CW3FloatingTab::~CW3FloatingTab()
{
	m_pParent = nullptr;
	SAFE_DELETE_OBJECT(m_generalMenuFloating);
	SAFE_DELETE_OBJECT(m_viewLayoutFloating);
	this->destroy();
}

void CW3FloatingTab::connections(void)
{
	
}

void CW3FloatingTab::connections_seg(void)
{

}

void CW3FloatingTab::disconnections_seg(void)
{
	
}

CW3ViewLayout* CW3FloatingTab::getViewLayout(void)
{
	return m_viewLayoutFloating; 
}

void CW3FloatingTab::slotSetDisplayList(std::vector<W3BOOL> bList)
{
	//TODO : volume header need

	//if( !m_volume ) return; // rejection.

	//const std::vector<std::pair<std::string, std::string>>& coreList = m_volume->getHeader()->getListCore();
	//std::vector<std::pair<std::string, std::string>> list;

	//for( W3INT i=0; i<bList.size(); i++ ) {
	//	if( bList.at(i) )	list.push_back(coreList.at(i));
	//}

	//m_pTabMgr->slotSetDisplayLabels(list)
}

void CW3FloatingTab::closeEvent(QCloseEvent* pEvent)
{
	emit sigClosed();
	QWidget::closeEvent(pEvent);
}
