#include "W3TitleBar.h"

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QToolButton>

#include <QGraphicsView>
#include <QOpenGLWidget>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Memory.h"

CW3TitleBar::CW3TitleBar(QWidget* parent)
	: QWidget(parent)
{
	setObjectName("TitleBar");
	setStyleSheet(CW3Theme::getInstance()->titlebarStyleSheet());
	setFixedHeight(CW3Theme::getInstance()->getSizeHeightTitleBar() + 4);

	QLabel* logo = new QLabel();
	QPixmap logo_pixmap(":/image/titlebar/logo2_bar.png");
	logo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	logo->setPixmap(logo_pixmap);
	logo->setFixedSize(logo_pixmap.size());

	QHBoxLayout* logo_layout = new QHBoxLayout();
	logo_layout->setSpacing(0);
	logo_layout->setContentsMargins(11, 0, 6, 0);
	logo_layout->setAlignment(Qt::AlignLeft);

	logo_layout->addWidget(logo);

	m_btnMinimize = new QToolButton();
	m_btnRestore = new QToolButton();
	m_btnClose = new QToolButton();
	m_pMenuBar = new QMenuBar();

	QFont font = QApplication::font();
	m_pMenuBar->setFont(font);

	m_pMenuFile = new QMenu(lang::LanguagePack::txt_file(), this);
	m_pMenuFile->setFont(font);
	//m_pMenuFile->addAction("Open Folder");
#ifndef WILL3D_VIEWER
	m_pMenuFile->addAction(lang::LanguagePack::txt_save());
#endif
	m_pMenuFile->addAction(lang::LanguagePack::txt_capture());
	m_pMenuFile->addAction(lang::LanguagePack::txt_print());
#ifndef WILL3D_VIEWER
	m_pMenuFile->addAction(lang::LanguagePack::txt_logout());
#endif
	m_pMenuFile->addAction(lang::LanguagePack::txt_exit());

#ifndef WILL3D_VIEWER
	connect(m_pMenuFile->actions()[0], SIGNAL(triggered()), parent, SLOT(slotSave()));
	connect(m_pMenuFile->actions()[1], SIGNAL(triggered()), parent, SLOT(slotCapture()));
	connect(m_pMenuFile->actions()[2], SIGNAL(triggered()), parent, SLOT(slotPrint()));
	connect(m_pMenuFile->actions()[3], SIGNAL(triggered()), parent, SLOT(slotLogout()));
	connect(m_pMenuFile->actions()[4], SIGNAL(triggered()), this, SIGNAL(sigClose()));
#else
	connect(m_pMenuFile->actions()[0], SIGNAL(triggered()), parent, SLOT(slotCapture()));
	connect(m_pMenuFile->actions()[1], SIGNAL(triggered()), parent, SLOT(slotPrint()));
	connect(m_pMenuFile->actions()[2], SIGNAL(triggered()), this, SIGNAL(sigClose()));
#endif

	m_pMenuEdit = new QMenu(lang::LanguagePack::txt_edit(), this);
	m_pMenuEdit->setFont(font);
	m_pMenuEdit->addAction(lang::LanguagePack::txt_preferences());
	connect(m_pMenuEdit->actions()[0], SIGNAL(triggered()), parent, SLOT(slotPreferences()));

	m_pMenuHelp = new QMenu(lang::LanguagePack::txt_help(), this);
	m_pMenuHelp->setFont(font);
	m_pMenuHelp->addAction(lang::LanguagePack::txt_support());
	m_pMenuHelp->addAction(lang::LanguagePack::txt_about());
	connect(m_pMenuHelp->actions()[0], SIGNAL(triggered()), parent, SLOT(slotSupport()));
	connect(m_pMenuHelp->actions()[1], SIGNAL(triggered()), parent, SLOT(slotAbout()));

	m_pMenuBar->addMenu(m_pMenuFile);
	m_pMenuBar->addMenu(m_pMenuEdit);
	m_pMenuBar->addMenu(m_pMenuHelp);
	m_pMenuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	
	QHBoxLayout* menu_layout = new QHBoxLayout();
	menu_layout->setSpacing(0);
	menu_layout->setContentsMargins(0, 0, 0, 0);
	menu_layout->setAlignment(Qt::AlignVCenter);

	menu_layout->addWidget(m_pMenuBar);
	menu_layout->addStretch(1);

	m_btnClose->setObjectName(tr("close"));
	m_btnMinimize->setObjectName(tr("minimize"));
	m_btnRestore->setObjectName(tr("restore_max"));
	
	QHBoxLayout* button_layout = new QHBoxLayout();
	button_layout->setSpacing(5);
	button_layout->setContentsMargins(5, 0, 5, 0);
	button_layout->setAlignment(Qt::AlignRight);

	button_layout->addWidget(m_btnMinimize);
	button_layout->addWidget(m_btnRestore);
	button_layout->addWidget(m_btnClose);

	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->setSpacing(5);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addLayout(logo_layout);
	mainLayout->addLayout(menu_layout, 1);
	mainLayout->addLayout(button_layout);

	setLayout(mainLayout);

	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 1)
	{
		m_btnMinimize->setVisible(false);
		m_btnRestore->setVisible(false);
		m_btnClose->setVisible(false);
	}

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	connect(m_btnMinimize, SIGNAL(clicked()), this, SLOT(slotMinimizeClicked()));
	connect(m_btnRestore, SIGNAL(clicked()), this, SLOT(slotNormalMaximizeClicked()));
	connect(m_btnClose, SIGNAL(clicked()), this, SIGNAL(sigClose()));
};

CW3TitleBar::~CW3TitleBar()
{

}

void CW3TitleBar::SetMaximized(bool maximized)
{
	maximized_ = maximized;
	SetNormalMaximizeButtonState();
	emit sigShowMaxRestore(maximized_);
}

void CW3TitleBar::slotMinimizeClicked()
{
	parentWidget()->showMinimized();
}

void CW3TitleBar::slotNormalMaximizeClicked() {
	SetMaximized(!maximized_);
}

void CW3TitleBar::SetNormalMaximizeButtonState()
{
	if (maximized_)
	{
		m_btnRestore->setObjectName("restore_min");
	}
	else
	{
		m_btnRestore->setObjectName("restore_max");
	}
	setStyleSheet(CW3Theme::getInstance()->titlebarStyleSheet());
}

void CW3TitleBar::ToggleNormalMaximizeState()
{
	maximized_ = !maximized_;
	SetNormalMaximizeButtonState();
}

void CW3TitleBar::slotCloseClicked() {
	//this->parentWidget()->close();

	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_19(), CW3MessageBox::Warning);

	if (msgBox.exec())
		QCoreApplication::quit();
	//delete this->parentWidget();
}

void CW3TitleBar::mousePressEvent(QMouseEvent *event) {
	QWidget::mousePressEvent(event);
	clickPos = mapToParent(event->pos());
	clicked_ = true;
}

void CW3TitleBar::mouseMoveEvent(QMouseEvent *event) {
	QWidget::mouseMoveEvent(event);

	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 1 || !clicked_)
	{
		return;
	}

	QPoint distance = event->globalPos() - clickPos;

	if (maximized_)
	{
		SetMaximized(false);
	}
	else
	{
		parentWidget()->move(distance);
	}
}

void CW3TitleBar::mouseDoubleClickEvent(QMouseEvent* pEvent) {
	QWidget::mouseDoubleClickEvent(pEvent);

	int maximize_mode = GlobalPreferences::GetInstance()->preferences_.general.interfaces.maximize_type;
	if (maximize_mode == 1)
	{
		return;
	}

	slotNormalMaximizeClicked();
}
