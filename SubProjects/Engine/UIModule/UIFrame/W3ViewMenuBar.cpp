#include "W3ViewMenuBar.h"
#include <iostream>

CW3ViewMenuBar::CW3ViewMenuBar(EVIEW_TYPE eViewType, QWidget *parent)
	: QWidget(parent), m_eViewType(eViewType)
{
	ui.setupUi(this);
	ui.m_cbxViewType->setFixedSize(CBX_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnPanning->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnZooming->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnMaximize->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnReset->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnOverlay->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnCapture->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	ui.m_btnExpand->setFixedSize(BTN_SIZE_WIDTH, BTN_SIZE_HEIGHT);
	
	ui.m_btnReset->setIcon(QIcon("./Images/Button/reset.png"));
	ui.m_btnZooming->setIcon(QIcon("./Images/Button/zooming.png"));
	ui.m_btnPanning->setIcon(QIcon("./Images/Button/panning.png"));
	ui.m_btnMaximize->setIcon(QIcon("./Images/Button/maximize.png"));
	ui.m_btnOverlay->setIcon(QIcon("./Images/Button/textOverlay.png"));
	ui.m_btnCapture->setIcon(QIcon("./Images/Button/capture.png"));
	ui.m_btnExpand->setIcon(QIcon("./Images/Button/expand.png"));
	ui.m_btnFit->setIcon(QIcon("./Images/Button/fit.png"));
	ui.m_btnFit->setVisible(false);

	this->setFixedHeight(VIEWMENU_SIZE_HEIGHT);

	QPalette pal(palette());
	pal.setColor(QPalette::Background, Qt::gray);
	this->setPalette(pal);

	m_bPanning = false;
	m_bZooming = false;
	m_bMaximize = false;
	m_bOverlay = false;
	m_bExpand = false;

	m_pLayout = ui.m_layoutBtnArea;
	m_pCbxViewType = ui.m_cbxViewType;

	setConnections();
}

CW3ViewMenuBar::~CW3ViewMenuBar()
{
	setDisconnections();
}

void CW3ViewMenuBar::setConnections(void)
{
	connect(ui.m_cbxViewType, SIGNAL(activated(int)), this, SLOT(slotSwapWindow(int)));
	connect(ui.m_btnPanning, SIGNAL(clicked()), this, SLOT(slotSetPanning()));
	connect(ui.m_btnZooming, SIGNAL(clicked()), this, SLOT(slotSetZooming()));
	connect(ui.m_btnMaximize, SIGNAL(clicked()), this, SLOT(slotSetMaximize()));
	connect(ui.m_btnReset, SIGNAL(clicked()), this, SIGNAL(sigReset()));
	connect(ui.m_btnOverlay, SIGNAL(clicked()), this, SLOT(slotOverlay()));
	connect(ui.m_btnExpand, SIGNAL(clicked()), this, SLOT(slotExpand()));
	connect(ui.m_btnCapture, SIGNAL(clicked()), this, SIGNAL(sigCapture()));
	connect(ui.m_btnFit, SIGNAL(clicked()), this, SIGNAL(sigFit()));
}
void CW3ViewMenuBar::setDisconnections(void)
{
	disconnect(ui.m_cbxViewType, SIGNAL(activated(int)), this, SLOT(slotSwapWindow(int)));
	disconnect(ui.m_btnPanning, SIGNAL(clicked()), this, SLOT(slotSetPanning()));
	disconnect(ui.m_btnZooming, SIGNAL(clicked()), this, SLOT(slotSetZooming()));
	disconnect(ui.m_btnMaximize, SIGNAL(clicked()), this, SLOT(slotSetMaximize()));
	disconnect(ui.m_btnReset, SIGNAL(clicked()), this, SIGNAL(sigReset()));
	disconnect(ui.m_btnOverlay, SIGNAL(clicked()), this, SLOT(slotOverlay()));
	disconnect(ui.m_btnExpand, SIGNAL(clicked()), this, SLOT(slotExpand()));
	disconnect(ui.m_btnCapture, SIGNAL(clicked()), this, SIGNAL(sigCapture()));
	disconnect(ui.m_btnFit, SIGNAL(clicked()), this, SIGNAL(sigFit()));
}

void CW3ViewMenuBar::displayViewType(void)
{

}
void CW3ViewMenuBar::setViewType(EVIEW_TYPE)
{

}
void CW3ViewMenuBar::setExpandButton(W3BOOL bExpand)
{
	m_bExpand = bExpand;
	ui.m_btnExpand->setFlat(bExpand);
}
void CW3ViewMenuBar::setBtnMaximize(W3BOOL bFlag)
{
	m_bMaximize = bFlag;
	ui.m_btnMaximize->setFlat(bFlag);
}
void CW3ViewMenuBar::setVisibleExpandButton(W3BOOL bExpand)
{
	ui.m_btnExpand->setVisible(bExpand);
}

void CW3ViewMenuBar::slotSwapWindow(int)
{

}

void CW3ViewMenuBar::slotSetPanning(void)
{
	m_bPanning ^= true;
	ui.m_btnPanning->setFlat(m_bPanning);

	if(m_bZooming)
	{
		m_bZooming ^= true;
		ui.m_btnZooming->setFlat(m_bZooming);
		emit sigSetZooming(m_bZooming);
	}

	emit sigSetPanning(m_bPanning);
}
void CW3ViewMenuBar::slotSetZooming(void)
{
	m_bZooming ^= true;
	ui.m_btnZooming->setFlat(m_bZooming);

	if(m_bPanning)
	{
		m_bPanning ^= true;
		ui.m_btnPanning->setFlat(m_bPanning);
		emit sigSetPanning(m_bPanning);
	}

	emit sigSetZooming(m_bZooming);
}
void CW3ViewMenuBar::slotSetMaximize(void)
{
	m_bMaximize ^= true;
	ui.m_btnMaximize->setFlat(m_bMaximize);
	emit sigSetMaximize(m_eViewType);
}
void CW3ViewMenuBar::slotOverlay(void)
{
	m_bOverlay ^= true;
	ui.m_btnOverlay->setFlat(m_bOverlay);
	emit sigSetOverlay(m_bOverlay);
}
void CW3ViewMenuBar::slotExpand(void)
{
	m_bExpand ^= true;
	ui.m_btnExpand->setFlat(m_bExpand);
	emit sigExpand(m_bExpand);
}
//sync slot functions
void CW3ViewMenuBar::slotSyncPanning(W3BOOL bFlag)
{
	m_bPanning = bFlag;
	ui.m_btnPanning->setFlat(m_bPanning);
}
void CW3ViewMenuBar::slotSyncZooming(W3BOOL bFlag)
{
	m_bZooming = bFlag;
	ui.m_btnZooming->setFlat(m_bZooming);
}
void CW3ViewMenuBar::slotSyncOverlay(W3BOOL bFlag)
{
	m_bOverlay = bFlag;
	ui.m_btnOverlay->setFlat(m_bOverlay);
}

