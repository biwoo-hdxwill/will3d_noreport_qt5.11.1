#pragma once
/**=================================================================================================

Project: 			UIComponent
File:				implant_preference_item.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-04-03
Last modify:		2018-04-03

*===============================================================================================**/
#include <QFrame>
#include <qcheckbox.h>
#include <qlabel.h>

#include "uicomponent_global.h"
class UICOMPONENT_EXPORT ImplantPreferenceItem : public QFrame {
	Q_OBJECT

public:
	ImplantPreferenceItem(int implant_number, QWidget *parent);
	virtual ~ImplantPreferenceItem();

	void ResetItem(const int implant_number);
	void UpdateItemInfo(const int implant_number,
						const QString& manufacturer_name,
						const QString& product_name,
						const QString& diameter,
						const QString& length);

	void SyncSelectedStatus(bool);

	inline const bool current_selected() const noexcept { return current_selected_; }

signals:
	void sigClearCurrentItem(int implant_number);
	void sigSetCurrentItemInfo(const QString& manufacturer_name,
							   const QString& product_name,
							   const QString& diameter,
							   const QString& length);

protected:
	virtual void leaveEvent(QEvent * event);
	virtual void enterEvent(QEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

private slots:
	void slotUsePresetClicked(int);

private:
	void InitUI(int implant_number);

	QString implant_number() const noexcept;
	QString manufacturer() const noexcept;
	QString product() const noexcept;
	QString diameter() const noexcept;
	QString length() const noexcept;

private:
	// DB query 및 UI 접근에 용이하게 QString 형태로 저장
	QCheckBox use_preset_;
	QLabel manufacturer_;
	QLabel product_;
	QLabel spec_;

	bool current_selected_ = false;
};
