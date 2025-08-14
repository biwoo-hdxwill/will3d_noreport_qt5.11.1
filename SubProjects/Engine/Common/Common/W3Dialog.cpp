#include "W3Dialog.h"

#include <QDebug>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

#include "W3Theme.h"
#include "W3LayoutFunctions.h"

CW3DialogTitleBar::CW3DialogTitleBar(const QString& strTitle, QWidget* parent)
	: QWidget(parent) {
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(mainLayout);

	m_lblCaption = new QLabel();
	m_lblCaption->setObjectName("titlebar");
	m_lblCaption->setText(strTitle);
	m_btnClose = new QToolButton;
	m_btnClose->setObjectName("titlebar");
	m_btnClose->setFocusPolicy(Qt::ClickFocus);

	mainLayout->addWidget(m_lblCaption);
	mainLayout->addWidget(m_btnClose);

	connect(m_btnClose, SIGNAL(clicked()), this, SLOT(parentClose()));
}

void CW3DialogTitleBar::parentClose() {
	this->parentWidget()->close();
}

void CW3DialogTitleBar::mousePressEvent(QMouseEvent *event) {
	startPos = event->globalPos();
	clickPos = mapToParent(event->pos());
}

void CW3DialogTitleBar::mouseMoveEvent(QMouseEvent *event) {
	parentWidget()->move(event->globalPos() - clickPos);
}

void CW3DialogTitleBar::setTitle(const QString& strTitle) {
	m_lblCaption->setText(strTitle);
}

CW3Dialog::CW3Dialog(const QString& strTitle, QWidget* parent, const Theme theme)
	: QDialog(parent, Qt::FramelessWindowHint | Qt::Window/* | Qt::WindowStaysOnTopHint*/) {
	m_contentWidget = new QFrame();
	m_contentWidget->setObjectName("content");

	m_contentLayout = new QVBoxLayout();
	m_contentLayout->setContentsMargins(0, 0, 0, 0);
	m_contentLayout->setSpacing(0);
	m_contentWidget->setLayout(m_contentLayout);
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	this->setLayout(mainLayout);

	if (!strTitle.isEmpty()) {
		m_titleBar = new CW3DialogTitleBar(strTitle, this);
		mainLayout->addWidget(m_titleBar);
	}
	mainLayout->addWidget(m_contentWidget);

	switch (theme) {
	case Theme::Dark:
		setStyleSheet(CW3Theme::getInstance()->appDialogStyleSheet());
		break;
	case Theme::Light:
		setStyleSheet(CW3Theme::getInstance()->LightDialogStyleSheet());
		break;
	default:
		break;
	}
}

CW3Dialog::~CW3Dialog() 
{
	 CW3LayoutFunctions::RemoveWidgetsAll(m_contentLayout);
}

void CW3Dialog::setTitle(const QString& strTitle) {
	m_titleBar->setTitle(strTitle);
}

void CW3Dialog::SetContentLayout(QLayout* layout) {
  CW3LayoutFunctions::RemoveWidgetsAll(m_contentLayout);
  delete m_contentLayout;
  m_contentLayout = new QVBoxLayout();
  m_contentLayout->setContentsMargins(0, 0, 0, 0);
  m_contentLayout->setSpacing(0);

  m_contentLayout->addLayout(layout);
  m_contentWidget->setLayout(m_contentLayout);
}

QFrame* CW3Dialog::CreateHorizontalLine() {
	QFrame* line = new QFrame();

	line->setObjectName("Line");
	line->setFrameShadow(QFrame::Plain);
	line->setFrameShape(QFrame::HLine);
	line->setLineWidth(0);
	line->setMidLineWidth(0);
	line->setStyleSheet(CW3Theme::getInstance()->LineSeparatorStyleSheet());
	line->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	return line;
}

void CW3Dialog::keyPressEvent(QKeyEvent* event)
{
	if ((event->key() == Qt::Key_Return ||
		event->key() == Qt::Key_Enter) && 
		positive_button_)
	{
		emit positive_button_->clicked();
	}

	QDialog::keyPressEvent(event);
}
