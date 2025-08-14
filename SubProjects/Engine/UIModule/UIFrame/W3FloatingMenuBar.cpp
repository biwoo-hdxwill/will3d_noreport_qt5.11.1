#include "W3FloatingMenuBar.h"

CW3FloatingMenuBar::CW3FloatingMenuBar(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QSizePolicy SizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	SizePolicy.setHorizontalStretch(0);
	SizePolicy.setVerticalStretch(0);
	setSizePolicy(SizePolicy);

	this->setFixedWidth(MENUBAR_SIZE_LEFT);
}

CW3FloatingMenuBar::~CW3FloatingMenuBar()
{

}
