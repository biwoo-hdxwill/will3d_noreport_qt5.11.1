#include "W3CephCoordSysDlg.h"

#include <QRadioButton>
#include <QHBoxLayout>
#include <QToolButton>

#include "../../Common/Common/language_pack.h"

CW3CephCoordSysDlg::CW3CephCoordSysDlg(QWidget * parent)
	: CW3Dialog(tr("Coordinate System"), parent) {
	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(15, 10, 15, 5);

	QVBoxLayout* contentLayout = new QVBoxLayout;
	contentLayout->setSpacing(5);

	QRadioButton* radio = new QRadioButton();
	radio->setText("based on FH Plane");
	radio->setChecked(true);
	contentLayout->addWidget(radio);

	hLayout->addLayout(contentLayout);

	QHBoxLayout* commandLayout = new QHBoxLayout;
	commandLayout->setContentsMargins(15, 5, 15, 10);
	commandLayout->setSpacing(5);
	commandLayout->setAlignment(Qt::AlignRight);
	m_btnOK = new QToolButton();
	m_btnOK->setText(lang::LanguagePack::txt_ok());
	m_btnCancel = new QToolButton();
	m_btnCancel->setText(lang::LanguagePack::txt_cancel());
	commandLayout->addWidget(m_btnOK);
	commandLayout->addWidget(m_btnCancel);

	m_contentLayout->addLayout(hLayout);
	m_contentLayout->addLayout(commandLayout);

	connect(m_btnOK, SIGNAL(clicked()), this, SLOT(slotClickedOK()));
	connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(hide()));

	hide();
}

CW3CephCoordSysDlg::~CW3CephCoordSysDlg() {
}

void CW3CephCoordSysDlg::slotClickedOK() {
	this->hide();
}
