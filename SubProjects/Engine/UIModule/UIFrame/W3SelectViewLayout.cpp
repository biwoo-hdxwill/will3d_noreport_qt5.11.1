#include "W3SelectViewLayout.h"

CW3SelectViewLayout::CW3SelectViewLayout(QWidget *parent)
	: QWidget(parent), m_nCol(0), m_nRow(0)
{
	ui.setupUi(this);
	ui.m_spbSelectionCol->setRange(0, 4);
	ui.m_spbSelectionRow->setRange(0, 4);
	ui.m_btnMPRLayout->setIcon(QIcon("./Images/Button/mprLayout.png"));
	ui.m_btnLeftmostLayout->setIcon(QIcon("./Images/Button/lLayout.png"));
	ui.m_btnSingletonLayout->setIcon(QIcon("./Images/Button/singletonLayout.png"));
	ui.m_btnSegLayout->setIcon(QIcon("./Images/Button/segLayout.png"));
	ui.m_btnSelectionLayout->setIcon(QIcon("./Images/Button/gridLayout.png"));

	setWindowFlags(Qt::FramelessWindowHint);
	QPalette Pal(palette());
	// set black background
	Pal.setColor(QPalette::Background, Qt::darkGray);
	this->setAutoFillBackground(true);
	this->setPalette(Pal);
	this->setVisible(false);

	setConnections();
}

CW3SelectViewLayout::~CW3SelectViewLayout()
{
	setDisconnections();
}

void CW3SelectViewLayout::setConnections(void)
{
	connect(ui.m_btnMPRLayout, SIGNAL(clicked()), this, SLOT(slotMPRLayout()));
	connect(ui.m_btnLeftmostLayout, SIGNAL(clicked()), this, SLOT(slotLMainLayout()));
	connect(ui.m_btnSelectionLayout, SIGNAL(clicked()), this, SLOT(slotGridLayout()));
	connect(ui.m_btnSingletonLayout, SIGNAL(clicked()), this, SLOT(slotSingletonLayout()));
	connect(ui.m_btnSegLayout, SIGNAL(clicked()), this, SLOT(slotSegmentationLayout()));

	connect(ui.m_spbSelectionRow, SIGNAL(valueChanged(int)), this, SLOT(slotRowChanged(int)));
	connect(ui.m_spbSelectionCol, SIGNAL(valueChanged(int)), this, SLOT(slotColChanged(int)));
}

void CW3SelectViewLayout::setDisconnections(void)
{
	disconnect(ui.m_btnMPRLayout, SIGNAL(clicked()), this, SLOT(slotMPRLayout()));
	disconnect(ui.m_btnLeftmostLayout, SIGNAL(clicked()), this, SLOT(slotLMainLayout()));
	disconnect(ui.m_btnSelectionLayout, SIGNAL(clicked()), this, SLOT(slotGridLayout()));
	disconnect(ui.m_btnSingletonLayout, SIGNAL(clicked()), this, SLOT(slotSingletonLayout()));
	disconnect(ui.m_btnSegLayout, SIGNAL(clicked()), this, SLOT(slotSegmentationLayout()));

	disconnect(ui.m_spbSelectionRow, SIGNAL(valueChanged(int)), this, SLOT(slotRowChanged(int)));
	disconnect(ui.m_spbSelectionCol, SIGNAL(valueChanged(int)), this, SLOT(slotColChanged(int)));
}

//////////////////////////////////////////////////////////////////////////
//	private slot functions
//////////////////////////////////////////////////////////////////////////
void CW3SelectViewLayout::slotMPRLayout(void)
{
	emit sigChangeLayout(EVIEW_LAYOUT_TYPE::MPR, 4, 4);
	this->close();
}
void CW3SelectViewLayout::slotSingletonLayout(void)
{
	emit sigChangeLayout(EVIEW_LAYOUT_TYPE::SINGLE, 4, 4);
	this->close();
}
void CW3SelectViewLayout::slotGridLayout(void)
{
	emit sigChangeLayout(EVIEW_LAYOUT_TYPE::GRID, m_nRow, m_nCol);
	this->close();
}
void CW3SelectViewLayout::slotSegmentationLayout(void)
{
	emit sigChangeLayout(EVIEW_LAYOUT_TYPE::DOUBLE, 4, 4);
	this->close();
}
void CW3SelectViewLayout::slotLMainLayout(void)
{
	emit sigChangeLayout(EVIEW_LAYOUT_TYPE::LMAIN, 4, 4);
	this->close();
}

void CW3SelectViewLayout::slotRowChanged(int row)
{
	m_nRow = row;

	if(m_nRow > 0 && m_nCol > 0)
		ui.m_btnSelectionLayout->setEnabled(true);
	else
		ui.m_btnSelectionLayout->setEnabled(false);
}

void CW3SelectViewLayout::slotColChanged(int col)
{
	m_nCol = col;

	if(m_nRow > 0 && m_nCol > 0)
		ui.m_btnSelectionLayout->setEnabled(true);
	else
		ui.m_btnSelectionLayout->setEnabled(false);
}

void CW3SelectViewLayout::slotSelectViewIdx(int idx)
{
	m_nSelectedViewIdx = idx;
}
