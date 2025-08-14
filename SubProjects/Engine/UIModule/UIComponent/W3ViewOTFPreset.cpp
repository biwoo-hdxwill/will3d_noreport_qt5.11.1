#include "W3ViewOTFPreset.h"
/*=========================================================================

File:			class CW3ViewOTFPreset
Language:		C++11
Library:		Qt 5.2.0
Author:			JUNG DAE GUN
First date:		2015-10-01
Last modify:	2015-10-01

=========================================================================*/
#include <QCheckBox>
#include <QToolButton>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QFileInfo>

CW3ViewOTFPreset::CW3ViewOTFPreset(int id, const QString& imgPath, bool isWritable, QWidget *parent) :
	m_nId(id), m_bIsWritable(isWritable), QGraphicsView(parent) {
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QGraphicsScene *scene = new QGraphicsScene(this);
	this->setScene(scene);
	this->setBackgroundBrush(QBrush(Qt::black));
	this->setMouseTracking(true);

	QFileInfo info(imgPath);
	if (info.exists()) {
		m_strViewName = info.fileName();
		m_strViewName = m_strViewName.split(".").at(0);
	} else {
		switch (id) {
		case 0:
			m_strViewName = "01_teeth";
			break;
		case 1:
			m_strViewName = "02_gray";
			break;
		case 2:
			m_strViewName = "03_soft_tissue1";
			break;
		case 3:
			m_strViewName = "04_soft_tissue2";
			break;
		case 4:
			m_strViewName = "05_bone";
			break;
		case 5:
			m_strViewName = "06_mip";
			break;
		case 6:
			m_strViewName = "07_xray";
			break;
		case 7:
			m_strViewName = "08_custom2";
			break;
		case 8:
			m_strViewName = "09_custom3";
			break;
		case 9:
			m_strViewName = "10_custom4";
			break;
		default:
			break;
		}
	}

	// image
	QPixmap img(imgPath, "BMP");
	bool isNull = img.isNull();

	m_pImgItem = nullptr;
	if (!isNull) {
		m_pImgItem = this->scene()->addPixmap(img);
		m_pImgItem->setZValue(0.0f);
	}

	// favorite checkbox
	QCheckBox *favorite = new QCheckBox();
	favorite->setStyleSheet(
		"QCheckBox {background-color: transparent;}"
		"QCheckBox::indicator:checked {image: url(:/image/otf/checkbox_checked.png);}"
		"QCheckBox::indicator:unchecked {image: url(:/image/otf/checkbox_unchecked.png);}"
	);
	m_pProxyWidgetFavoriteChk = this->scene()->addWidget(favorite);
	m_pProxyWidgetFavoriteChk->setZValue(1.0f);
	if (isNull)
		m_pProxyWidgetFavoriteChk->setVisible(false);

	connect(favorite, SIGNAL(stateChanged(int)), this, SLOT(slotSetFavorite(int)));

	// write/overwrite button
	m_pProxyWidgetWriteBtn = nullptr;
	if (m_bIsWritable) {
		QToolButton *write = new QToolButton();
		QString btnStyle =
			"QToolButton {background-color: transparent;}"
			"QToolButton {border: 0px transparent;}"
			"QToolButton {width: 34px; height: 34px;}";

		m_strWriteBtnStyle = btnStyle + "QToolButton {image: url(:/image/otf/button_write.png) center no-repeat;}";
		m_strOverwriteBtnStyle = btnStyle + "QToolButton {image: url(:/image/otf/button_overwrite.png) center no-repeat;}";

		if (isNull) {
			write->setStyleSheet(m_strWriteBtnStyle);
			connect(write, SIGNAL(clicked()), this, SLOT(slotWrite()));
		} else {
			write->setStyleSheet(m_strOverwriteBtnStyle);
			connect(write, SIGNAL(clicked()), this, SLOT(slotOverwrite()));
		}

		m_pProxyWidgetWriteBtn = this->scene()->addWidget(write);
		m_pProxyWidgetWriteBtn->setZValue(1.0f);
	}
}

CW3ViewOTFPreset::~CW3ViewOTFPreset() {}

void CW3ViewOTFPreset::resizeEvent(QResizeEvent *event) {
	QGraphicsView::resizeEvent(event);

	this->setSceneRect(0, 0, this->width(), this->height());

	if (m_pImgItem) {
		QPixmap img = m_pImgItem->pixmap();
		m_pImgItem->setPixmap(img.scaled(this->width(), this->height()));
	}

	if (m_pProxyWidgetFavoriteChk)
		m_pProxyWidgetFavoriteChk->setPos(10, 10);

	if (m_pProxyWidgetWriteBtn)
		m_pProxyWidgetWriteBtn->setPos(
			this->width() - m_pProxyWidgetWriteBtn->boundingRect().width() - 10,
			this->height() - m_pProxyWidgetWriteBtn->boundingRect().height() - 10);
}

void CW3ViewOTFPreset::setChecked(bool check) {
	if (m_pProxyWidgetFavoriteChk) {
		QCheckBox *checkbox = (QCheckBox *)m_pProxyWidgetFavoriteChk->widget();
		if (check)
			checkbox->setCheckState(Qt::Checked);
		else
			checkbox->setCheckState(Qt::Unchecked);
	}
}

void CW3ViewOTFPreset::setPreset(const QString& imgPath) {
	QPixmap img(imgPath);
	if (!img.isNull()) {
		m_pImgItem = this->scene()->addPixmap(img);
		m_pImgItem->setZValue(0.0f);
		QPixmap img = m_pImgItem->pixmap();
		m_pImgItem->setPixmap(img.scaled(this->width(), this->height()));

		if (m_pProxyWidgetFavoriteChk)
			m_pProxyWidgetFavoriteChk->setVisible(true);

		if (m_pProxyWidgetWriteBtn) {
			QToolButton *btn = (QToolButton *)m_pProxyWidgetWriteBtn->widget();
			btn->disconnect();
			btn->setStyleSheet(m_strOverwriteBtnStyle);
			connect(btn, SIGNAL(clicked()), this, SLOT(slotOverwrite()));
		}
	}
}

void CW3ViewOTFPreset::slotSetFavorite(int check) {
	emit sigSetFavorite(m_nId, check);
}

void CW3ViewOTFPreset::slotWrite() {
	emit sigWrite(m_nId);
}

void CW3ViewOTFPreset::slotOverwrite() {
	emit sigOverwrite(m_nId);
}
