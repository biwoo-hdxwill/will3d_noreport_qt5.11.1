#include "W3GeneralMenu.h"

#include <iostream>

CW3GeneralMenu::CW3GeneralMenu(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.m_btnChangeLayout->setIcon(QIcon("./Images/Button/layout.png"));
	ui.m_btnInfoDisplay->setIcon(QIcon("./Images/Button/infoDisplay.png"));
	ui.m_btnCapture->setIcon(QIcon("./Images/Button/capture.png"));
	ui.m_btnResetMPR->setIcon(QIcon("./Images/Button/reset.png"));

	this->setFixedHeight(GENERALMENU_SIZE_HEIGHT);
	this->setFixedWidth(MENUBAR_SIZE_LEFT);

	m_bChangeLayout = false;
	m_bCapture = false;
	m_bMinimized = false;
	m_bResourceView = false;

	m_dlg = new CW3SelectDicomHeaderDlg(this);
	m_dlg->setVisible(false);

	m_dlgSelectLayout = nullptr;
	m_dlgSelectLayout = new CW3SelectViewLayout(parent);
	
	setConnections();
}

CW3GeneralMenu::~CW3GeneralMenu()
{

}

void CW3GeneralMenu::setConnections(void)
{
	connect(ui.m_btnResetMPR, SIGNAL(clicked()), this, SIGNAL(sigResetMPR(void)));
	connect(ui.m_btnChangeLayout, SIGNAL(clicked()), this, SLOT(slotChangeLayout(void)));
	connect(ui.m_btnInfoDisplay, SIGNAL(clicked()), this, SLOT(slotInfoDisplay(void)));
	connect(ui.m_btnCapture, SIGNAL(clicked()), this, SLOT(slotCapture(void)));
	connect(ui.m_btnResourceView, SIGNAL(clicked()), this, SLOT(slotResourceView(void)));

	connect(m_dlg, SIGNAL(sigSetDisplayList(std::vector<W3BOOL>)), this->parent(), SLOT(slotSetDisplayList(std::vector<W3BOOL>)));
	connect(m_dlgSelectLayout, SIGNAL(sigChangeLayout(EVIEW_LAYOUT_TYPE, W3INT, W3INT)),
		this, SIGNAL(sigChangeLayout(EVIEW_LAYOUT_TYPE, W3INT, W3INT)));
}

void CW3GeneralMenu::slotChangeLayout(void)
{
	m_bChangeLayout ^= true;
	QRect rect = ui.m_btnChangeLayout->geometry();
	QPoint pt(rect.x(), rect.y());
	pt.setX( pt.x() + rect.width());
	pt.setY( pt.y() - 40 );
	m_dlgSelectLayout->move( mapToGlobal(pt) );
	m_dlgSelectLayout->setVisible(m_bChangeLayout);
}
void CW3GeneralMenu::slotInfoDisplay(void)
{
	QRect rect = ui.m_btnInfoDisplay->geometry();
	QPoint pt(rect.x(), rect.y());
	pt.setX( pt.x() + rect.width() + 7 );
	pt.setY( pt.y() - m_dlg->geometry().height() - 5 );
	m_dlg->move( mapToGlobal(pt) );
	m_dlg->show();
}
void CW3GeneralMenu::slotCapture(void)
{

}
void CW3GeneralMenu::slotResourceView(void)
{
	m_bResourceView ^= true;
	ui.m_btnResourceView->setFlat(m_bResourceView);

	QRect rect = ui.m_btnChangeLayout->geometry();
	QPoint pt(rect.x(), rect.y() - 105);
	pt.setX( pt.x() + rect.width() + 2 );
	pt = mapToGlobal(pt);
	emit sigResourceView(pt, m_bResourceView);
}
void CW3GeneralMenu::slotSetMenuSize(W3INT size)
{
	if(size == 0)
		this->setVisible(false);
	else
	{
		this->setVisible(true);
		this->setFixedWidth(size);
	}
}
