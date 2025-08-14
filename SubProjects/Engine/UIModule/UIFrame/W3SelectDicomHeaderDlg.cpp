#include "W3SelectDicomHeaderDlg.h"
#include <iostream>

CW3SelectDicomHeaderDlg::CW3SelectDicomHeaderDlg(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	// set background color
	//QPalette Pal(palette());
	//Pal.setColor(QPalette::Background, Qt::lightGray);
	//this->setAutoFillBackground(true);
	//this->setPalette(Pal);

	for( W3INT i=0; i<static_cast<int>(EDICOM_HEADER_TYPE::SIZE); i++ )
		m_bList.push_back(true);
}

CW3SelectDicomHeaderDlg::~CW3SelectDicomHeaderDlg(void)
{
}

void CW3SelectDicomHeaderDlg::accept(void)
{
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::MODALITY))	= ui.checkBox_modality->isChecked() ? true : false;
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::SERIES))	= ui.checkBox_series->isChecked() ? true : false;
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::KVP))		= ui.checkBox_kvp->isChecked() ? true : false;
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::DIAMETER))	= ui.checkBox_reconD->isChecked() ? true : false;
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::TCURRENT))	= ui.checkBox_tubeCurr->isChecked() ? true : false;
	m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::FILTER))	= ui.checkBox_filter->isChecked() ? true : false;

	emit sigSetDisplayList(m_bList);
	QDialog::accept();
}

void CW3SelectDicomHeaderDlg::show(void)
{
	ui.checkBox_modality->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::MODALITY)) );
	ui.checkBox_series->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::SERIES)) );
	ui.checkBox_kvp->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::KVP)) );
	ui.checkBox_reconD->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::DIAMETER)) );
	ui.checkBox_tubeCurr->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::TCURRENT)) );
	ui.checkBox_filter->setChecked( m_bList.at(static_cast<int>(EDICOM_HEADER_TYPE::FILTER)) );

	QDialog::show();
}

