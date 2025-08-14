#include "W3DockWidget.h"
#include "../../Common/Common/W3Memory.h"
#include <stdio.h>

CW3DockWidget::CW3DockWidget(QWidget *parent)
	: QDockWidget(parent)
{
	this->setMinimumWidth(150);
	this->setMinimumHeight(300);

	m_pTitleBarWidget = this->titleBarWidget();

	//QWidget *TitleWidgetRec =new QWidget(this);
	//this->setTitleBarWidget(TitleWidgetRec);

}

CW3DockWidget::~CW3DockWidget()
{

}


//////////////////////////////////////////////////////////////////////////
//	protected functions
//////////////////////////////////////////////////////////////////////////
void CW3DockWidget::enterEvent(QEvent* event)
{
	QDockWidget::enterEvent(event);
	
	//if(!this->isFloating())
	//{
	//	QWidget* emptyTitleWidget = this->titleBarWidget();
	//	this->setTitleBarWidget(m_pTitleBarWidget);

	//	SAFE_DELETE_OBJECT(emptyTitleWidget);
	//}
	
}

void CW3DockWidget::leaveEvent(QEvent* event)
{
	QDockWidget::leaveEvent(event);
	
	//if(!this->isFloating())
	//{
	//	QWidget *emptyTitleWidget = new QWidget(this);
	//	this->setTitleBarWidget(emptyTitleWidget);
	//}
	
}

void CW3DockWidget::showTitleBar(void)
{

}
