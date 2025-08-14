#include "W3OTFPresetDlg.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QDir>
#include <QSettings>

#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/language_pack.h"
#include "../UIComponent/W3ViewOTFPreset.h"

CW3OTFPresetDlg::CW3OTFPresetDlg(QWidget * parent)
	: CW3Dialog(lang::LanguagePack::txt_preset_setting(), parent) {
	QGridLayout* viewLayout = new QGridLayout();
	viewLayout->setContentsMargins(20, 20, 20, 15);
	viewLayout->setSpacing(5);

	QDir directory("./tfpresets");
	if (directory.exists()) {
		QFileInfoList listInfo = directory.entryInfoList(QDir::Files, QDir::Name);
		for (const auto &i : listInfo) {
			if (i.fileName().compare("00_custom.bmp") == 0)
				continue;

			if (i.suffix().compare("bmp") == 0) {
				m_listPresetFile.push_back(i.filePath());
			}
		}
	} else {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_13(), CW3MessageBox::Critical);
		msgBox.setDetailedText(lang::LanguagePack::msg_14());
		msgBox.exec();
	}

	QString sectionKey = "OTF/favorite";
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	m_listFavorite = settings.value(sectionKey).toStringList();

	if (m_listFavorite.size() < 1) {
		m_listFavorite.append("01_teeth");
		m_listFavorite.append("02_gray");
		m_listFavorite.append("03_soft_tissue1");
		m_listFavorite.append("04_soft_tissue2");
		m_listFavorite.append("05_bone");
		m_listFavorite.append("06_mip");
		m_listFavorite.append("07_xray");
		settings.setValue(sectionKey, m_listFavorite);
	}

	for (int i = 0; i < 10; i++) {
		bool writable = false;
		if (i > 6)
			writable = true;

		QString path = "";
		if (m_listPresetFile.size() > i)
			path = m_listPresetFile.at(i);

		CW3ViewOTFPreset *view = new CW3ViewOTFPreset(i, path, writable);
		m_lpPresetView.push_back(view);

		view->setFixedSize(186, 186);

		viewLayout->addWidget(view, i / 5, i % 5);

		connect(view, SIGNAL(sigSetFavorite(int,int)), this, SLOT(slotSetFavorite(int,int)));
		connect(view, SIGNAL(sigWrite(int)), this, SLOT(slotWrite(int)));
		connect(view, SIGNAL(sigOverwrite(int)), this, SLOT(slotOverwrite(int)));

		// check favorite
		for (int j = 0; j < m_listFavorite.size(); j++) {
			if (view->getViewName().contains(m_listFavorite.at(j))) {
				view->setChecked(true);
				break;
			}
		}
	}
	//vLayout->addLayout();

	QHBoxLayout* commandLayout = new QHBoxLayout();
	commandLayout->setContentsMargins(0, 0, 0, 15);
	commandLayout->setSpacing(20);
	commandLayout->setAlignment(Qt::AlignCenter);
	m_btnOK = new QToolButton();
	m_btnOK->setText(lang::LanguagePack::txt_ok());
	m_btnCancel = new QToolButton();
	m_btnCancel->setText(lang::LanguagePack::txt_cancel());
	commandLayout->addWidget(m_btnOK);
	commandLayout->addWidget(m_btnCancel);

	m_contentLayout->addLayout(viewLayout);
	m_contentLayout->addLayout(commandLayout);

	connect(m_btnOK, SIGNAL(clicked()), this, SLOT(slotClickedOK()));
	connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(slotClickedCancel()));
}

CW3OTFPresetDlg::~CW3OTFPresetDlg() {}

void CW3OTFPresetDlg::slotClickedOK() {
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setValue("OTF/favorite", m_listFavorite);

	accept();
}

void CW3OTFPresetDlg::slotClickedCancel() {
	reject();
}

void CW3OTFPresetDlg::slotSetFavorite(int id, int check) {
	if (m_lpPresetView.size() < 10)
		return;

	CW3ViewOTFPreset *view = m_lpPresetView.at(id);
	if (!view)
		return;

	QString viewName = view->getViewName();

	if (check) {
		if (m_listFavorite.size() == 8) {
			view->setChecked(false);
			return;
		}

		if (!m_listFavorite.contains(viewName))
			m_listFavorite.push_back(viewName);
	} else {
		if (m_listFavorite.size() == 1) {
			view->setChecked(true);
			return;
		}

		m_listFavorite.removeOne(viewName);
	}
}

void CW3OTFPresetDlg::slotWrite(int id) {
	CW3ViewOTFPreset *view = m_lpPresetView.at(id);
	if (!view)
		return;

	QString name = view->getViewName();
	emit sigSavePreset(name);

	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_15(), CW3MessageBox::Information);
	msgBox.setDetailedText(lang::LanguagePack::msg_16());
	msgBox.exec();

	QString imgFilePath = "./tfpresets/" + name + ".bmp";
	view->setPreset(imgFilePath);
}

void CW3OTFPresetDlg::slotOverwrite(int id) {
	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_17(), CW3MessageBox::Question);
	msgBox.setDetailedText(lang::LanguagePack::msg_18());

	if (msgBox.exec())
		slotWrite(id);
}
