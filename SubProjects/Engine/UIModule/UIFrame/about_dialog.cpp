#include "about_dialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QFileInfo>
#include <QDateTime>

#include <Engine/Common/Common/sw_info.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/language_pack.h>

AboutDialog::AboutDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_about(), parent, CW3Dialog::Theme::Light)
{
	SetLayout();
}

AboutDialog::~AboutDialog() {}

void AboutDialog::SetLayout()
{
	m_contentLayout->setContentsMargins(20, 10, 20, 10);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addLayout(CreateContentsLayout());
	m_contentLayout->addLayout(CreateButtonLayout());
}

QVBoxLayout* AboutDialog::CreateContentsLayout()
{
	QLabel* logo_label = new QLabel();
	QImage logo;
	logo.load(":/image/hdxwill_logo.png");
	logo_label->setPixmap(QPixmap::fromImage(logo));
	logo_label->setContentsMargins(0, 20, 0, 20);
	logo_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	logo_label->setAlignment(Qt::AlignCenter);

	QLabel* di_label = new QLabel();
	di_label->setText(lang::LanguagePack::txt_di() + " : " + "#####");

	QLabel* version_label = new QLabel();
	version_label->setText(lang::LanguagePack::txt_pi() + "(" + lang::LanguagePack::txt_version() + ")" + " : " + QString::fromStdString(sw_info::SWInfo::GetSWVersion()));

	QFileInfo info("./Will3D.exe");
	QLabel* release_date_label = new QLabel();
	release_date_label->setText(lang::LanguagePack::txt_release_date() + " : " + info.lastModified().toString("dd.MM.yyyy hh:mm:ss"));

	QLabel* copyright_label = new QLabel();
	copyright_label->setText("Copyright (C) 2018-2020 HDXWILL All rights reserved");

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(10);

	layout->addWidget(logo_label);
	layout->addWidget(version_label);
	layout->addWidget(release_date_label);
	layout->addSpacing(20);
	layout->addWidget(copyright_label);

	return layout;
}

QHBoxLayout* AboutDialog::CreateButtonLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(10);
	layout->setAlignment(Qt::AlignCenter);

	QToolButton* close_button = new QToolButton();

	connect(close_button, SIGNAL(clicked()), this, SLOT(hide()));

	close_button->setText(lang::LanguagePack::txt_close());

	layout->addWidget(close_button);

	return layout;
}
