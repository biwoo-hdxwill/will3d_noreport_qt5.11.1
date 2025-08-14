#pragma once

#include <QObject>
#include <QGraphicsItem>
#include <QRectF>
#include "uiprimitive_global.h"

class CW3TextItem;

class UIPRIMITIVE_EXPORT CW3ViewInfoTextItems : public QObject, public QGraphicsItem
{
	Q_OBJECT
	Q_INTERFACES(QGraphicsItem)
public:
	CW3ViewInfoTextItems();
	~CW3ViewInfoTextItems();

public:
	void setEnabled(bool isEnable);
	void setVisible(bool isEnable);

	void setPatientID(const QString& id);
	void setPatientName(const QString& name);
	void setPatientName(const QString& name, const QString & sex);
	void setSeriesDate(const QString& date);
	void setSeriesNumber(const QString& number);
	void setKvp(const QString& kVp);
	void setXRayTubeCurrent(const QString& mA);
	void setModality(const QString& modality);
	void setViewName(const QString viewName);

	void setPosItem(int view_height);


	inline bool isEnable() const { return m_isEnable; }
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {};
	virtual QRectF boundingRect() const { return m_rect; }
private:

	QRectF m_rect;

	CW3TextItem* m_pTextDate = nullptr;
	CW3TextItem* m_pTextPatientID = nullptr;
	CW3TextItem* m_pTextPatientName = nullptr;
	CW3TextItem* m_pTextSeriesNumber = nullptr;
	CW3TextItem* m_pTextKvp = nullptr;
	CW3TextItem* m_pTextMa = nullptr;
	CW3TextItem* m_pTextModality = nullptr;
	CW3TextItem* m_pTextViewName = nullptr;

	bool m_isEnable = true;

};
