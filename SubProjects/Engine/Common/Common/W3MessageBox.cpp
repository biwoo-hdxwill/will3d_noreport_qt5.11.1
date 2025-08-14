#include "W3MessageBox.h"

#include <exception>
#include <iostream>

#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QSignalMapper>
#include <QToolButton>
#include <QVBoxLayout>

#include "W3Theme.h"
#include "language_pack.h"

using std::cout;
using std::endl;
using std::runtime_error;

CW3MessageBox::CW3MessageBox(const QString& title, const QString& text,
	MessageType type, QWidget* parent,
	CW3Dialog::Theme theme)
	: m_type(type), CW3Dialog(title, parent, theme)
{
	m_commandLayout = new QHBoxLayout;
	m_commandLayout->setContentsMargins(15, 0, 15, 15);
	m_commandLayout->setSpacing(5);
	m_commandLayout->setAlignment(Qt::AlignRight);

	using lang::LanguagePack;
	m_btnOK = new QToolButton();
	m_btnOK->setText(LanguagePack::txt_ok());

	m_btnCancel = new QToolButton();
	m_btnCancel->setText(LanguagePack::txt_cancel());

	QImage img;
	QPixmap pixmap;
	QString iconPath;

	switch (type)
	{
	case CW3MessageBox::Question:
		iconPath = QString(":image\\messagebox\\question.png");

		m_commandLayout->addWidget(m_btnOK);
		m_commandLayout->addWidget(m_btnCancel);
		break;
	case CW3MessageBox::Information:
		iconPath = QString(":image\\messagebox\\info.png");

		m_commandLayout->addWidget(m_btnOK);
		break;
	case CW3MessageBox::Warning:
		iconPath = QString(":image\\messagebox\\warning.png");

		m_commandLayout->addWidget(m_btnOK);
		m_commandLayout->addWidget(m_btnCancel);
		break;
	case CW3MessageBox::Critical:
		iconPath = QString(":image\\messagebox\\critical.png");

		m_commandLayout->addWidget(m_btnOK);
		break;
	case CW3MessageBox::ActionRole:
		iconPath = QString(":image\\messagebox\\info.png");
	default:
		break;
	}

	if (img.load(iconPath))
	{
		pixmap = QPixmap::fromImage(img);
		img.scaled(50, 50);
		pixmap = pixmap.scaled(50, 50);
	}
	else
	{
		//...error msg
	}

	QLabel* icon = new QLabel();
	icon->setAlignment(Qt::AlignCenter);
	icon->setPixmap(pixmap);
	icon->setMinimumSize(80, 80);
	icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout* contentLayout = new QHBoxLayout;
	contentLayout->setSpacing(5);
	contentLayout->setContentsMargins(10, 10, 15, 10);

	m_messageLayout = new QVBoxLayout;
	m_messageLayout->setAlignment(Qt::AlignVCenter);
	m_messageLayout->setContentsMargins(0, 0, 0, 0);
	m_messageLayout->setSpacing(0);

	QLabel* message = new QLabel();
	message->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	message->setObjectName("Message");
	message->setText(text);
	message->setMinimumWidth(200);
	message->setMaximumWidth(400);
	message->setWordWrap(true);
	message->setTextInteractionFlags(Qt::TextSelectableByMouse);

	m_messageLayout->addWidget(message);
	contentLayout->addWidget(icon);
	contentLayout->addLayout(m_messageLayout);

	m_contentLayout->addLayout(contentLayout);
	m_contentLayout->addLayout(m_commandLayout);

	message->setStyleSheet(CW3Theme::getInstance()->messageBoxStyleSheet());

	connect(m_btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(reject()));

	m_sigMapper = new QSignalMapper(this);
	connect(m_sigMapper, SIGNAL(mapped(QObject*)), this,
		SLOT(slotClickedButton(QObject*)));

	m_clickedButton = 0;
	m_lblDetailedText = nullptr;

	if (type != CW3MessageBox::ActionRole)
	{
		setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
	}
}

CW3MessageBox::~CW3MessageBox() {}

void CW3MessageBox::setDetailedText(const QString& text)
{
	if (!m_lblDetailedText)
	{
		m_lblDetailedText = new QLabel();
		m_lblDetailedText->setSizePolicy(QSizePolicy::Expanding,
			QSizePolicy::Expanding);
		m_lblDetailedText->setObjectName("DetailedText");
		m_lblDetailedText->setMinimumWidth(200);
		m_lblDetailedText->setMaximumWidth(400);
		m_lblDetailedText->setWordWrap(true);
		m_messageLayout->addWidget(m_lblDetailedText);
		m_lblDetailedText->setStyleSheet(
			CW3Theme::getInstance()->messageBoxStyleSheet());
	}
	m_lblDetailedText->setText(text);
}

QObject* CW3MessageBox::AddButton(const QString& text)
{
	try
	{
		if (m_type != ActionRole)
		{
			throw runtime_error("type is not ActionRole.");
		}

		setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);

		QToolButton* btn = new QToolButton();
		btn->setText(text);

		connect(btn, SIGNAL(clicked()), m_sigMapper, SLOT(map()));
		m_sigMapper->setMapping(btn, dynamic_cast<QObject*>(btn));

		m_commandLayout->addWidget(btn);

		return btn;
	}
	catch (const runtime_error& e)
	{
		cout << "CW3MessageBox::addButton: " << e.what() << endl;
		return nullptr;
	}
}

void CW3MessageBox::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return ||
		event->key() == Qt::Key_Enter ||
		event->key() == Qt::Key_Space)
	{
		accept();
	}
	else if (event->key() == Qt::Key_Escape)
	{
		reject();
	}

	CW3Dialog::keyPressEvent(event);
}

void CW3MessageBox::slotClickedButton(QObject* btn)
{
	m_clickedButton = btn;
	done(static_cast<int>(reinterpret_cast<intptr_t>(btn)));
}
