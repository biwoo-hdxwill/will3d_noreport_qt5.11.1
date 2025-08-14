#include "W3ViewInfoTextItems.h"

#include <QApplication>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"

#include "W3TextItem.h"

CW3ViewInfoTextItems::CW3ViewInfoTextItems()
{
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	m_pTextDate = new CW3TextItem(font, "Date", Qt::white, false, this);
	m_pTextPatientID = new CW3TextItem(font, "ID", Qt::white, false, this);
	m_pTextPatientName = new CW3TextItem(font, "Name", Qt::white, false, this);
	m_pTextSeriesNumber = new CW3TextItem(font, "Series", Qt::white, false, this);
	m_pTextKvp = new CW3TextItem(font, "Kvp", Qt::white, false, this);
	m_pTextMa = new CW3TextItem(font, "Ma", Qt::white, false, this);
	m_pTextModality = new CW3TextItem(font, "Modality", Qt::white, false, this);

	font.setPixelSize(font.pixelSize() + 3);
	font.setWeight(QFont::Bold);

	m_pTextViewName = new CW3TextItem(this);
	m_pTextViewName->setFont(font);
	m_pTextViewName->setTextColor(Qt::white);
	m_pTextViewName->setHoverEnabled(false);

}

CW3ViewInfoTextItems::~CW3ViewInfoTextItems()
{
	SAFE_DELETE_OBJECT(m_pTextDate);
	SAFE_DELETE_OBJECT(m_pTextPatientID);
	SAFE_DELETE_OBJECT(m_pTextPatientName);
	SAFE_DELETE_OBJECT(m_pTextSeriesNumber);
	SAFE_DELETE_OBJECT(m_pTextKvp);
	SAFE_DELETE_OBJECT(m_pTextMa);
	SAFE_DELETE_OBJECT(m_pTextModality);
	SAFE_DELETE_OBJECT(m_pTextViewName);
}

void CW3ViewInfoTextItems::setEnabled(bool isEnable)
{
	this->setVisible(isEnable);
	m_isEnable = isEnable;
	QGraphicsItem::setEnabled(isEnable);
}

void CW3ViewInfoTextItems::setVisible(bool isEnable)
{
	if (!m_isEnable)
		return;

	QGraphicsItem::setVisible(isEnable);
}

void CW3ViewInfoTextItems::setPatientID(const QString & id)
{
	m_pTextPatientID->setPlainText(QString("ID : ") + id);
}

void CW3ViewInfoTextItems::setPatientName(const QString & name)
{
	m_pTextPatientName->setPlainText(name);
}

void CW3ViewInfoTextItems::setPatientName(const QString & name, const QString & sex)
{
	m_pTextPatientName->setPlainText(name + QString(" [") + sex + QString("]"));
}

void CW3ViewInfoTextItems::setSeriesDate(const QString & date)
{
	m_pTextDate->setPlainText(QString("Series : ") + date);
}

void CW3ViewInfoTextItems::setSeriesNumber(const QString & number)
{
	m_pTextSeriesNumber->setPlainText(QString("SeriesNumber : ") + number);
}

void CW3ViewInfoTextItems::setKvp(const QString & kVp)
{
	m_pTextKvp->setPlainText(kVp + QString(" [kVp]"));
}

void CW3ViewInfoTextItems::setXRayTubeCurrent(const QString & mA)
{
	m_pTextMa->setPlainText(mA + QString(" [mA]"));
}

void CW3ViewInfoTextItems::setModality(const QString & modality)
{
	m_pTextModality->setPlainText(modality);
}

void CW3ViewInfoTextItems::setViewName(const QString viewName)
{
	m_pTextViewName->setPlainText(viewName);
}

void CW3ViewInfoTextItems::setPosItem(int view_height)
{
	m_pTextViewName->setPos(mapToScene(common::ui_define::kViewMarginWidth, common::ui_define::kViewMarginHeight*0.7f));

	m_pTextDate->setPos(mapToScene(common::ui_define::kViewMarginWidth, common::ui_define::kViewSpacing * 2.0f));
	m_pTextPatientID->setPos(mapToScene(common::ui_define::kViewMarginWidth, common::ui_define::kViewSpacing * 3.0f));
	m_pTextPatientName->setPos(mapToScene(common::ui_define::kViewMarginWidth, common::ui_define::kViewSpacing * 4.0f));
	m_pTextSeriesNumber->setPos(mapToScene(common::ui_define::kViewMarginWidth, common::ui_define::kViewSpacing * 5.0f));


	float txtBottomPos = view_height - common::ui_define::kViewMarginHeight - m_pTextModality->sceneBoundingRect().height();

	m_pTextKvp->setPos(mapToScene(common::ui_define::kViewMarginWidth, txtBottomPos - common::ui_define::kViewSpacing*2.0f));
	m_pTextMa->setPos(mapToScene(common::ui_define::kViewMarginWidth, txtBottomPos - common::ui_define::kViewSpacing));
	m_pTextModality->setPos(mapToScene(common::ui_define::kViewMarginWidth, txtBottomPos));
}
