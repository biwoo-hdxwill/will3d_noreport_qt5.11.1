#pragma once

/*=========================================================================

File:			custom_implant_dialog.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2020-01-10
Last Modify:	2020-01-10

Copyright (c) 2018~2020 All rights reserved by HDXWILL.

=========================================================================*/

#include "uiframe_global.h"

#include <Engine/Common/Common/W3Dialog.h>

class QComboBox;
class QRadioButton;
class QButtonGroup;
class QHBoxLayout;

class UIFRAME_EXPORT CustomImplantDialog : public CW3Dialog
{
	Q_OBJECT

public:
	CustomImplantDialog(QWidget* parent = 0);
	virtual ~CustomImplantDialog();

	void SetManufacturerList(const QStringList& list);
	void SetProductList(const QStringList& list);

	void SetData(
		const QString& manufacturer, 
		const QString& product, 
		const QString& model, 
		const float coronal_diameter, 
		const float apical_diameter, 
		const float length
	);

	void GetData(
		QString& manufacturer,
		QString& product,
		QString& model,
		float& coronal_diameter,
		float& apical_diameter,
		float& length
	);

signals:
	void sigThrowData(QString, QString, QString, float, float, float);
	void sigManufacturerChanged(QString);

private slots:
	void slotOK();
	void slotTypeChanged(int index);

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();

private:
	QButtonGroup* type_radio_group_ = nullptr;

	QRadioButton* straight_radio_ = nullptr;
	QRadioButton* tapered_radio_ = nullptr;

	QComboBox* manufacturer_combo_ = nullptr;
	QComboBox* product_combo_ = nullptr;
	QLineEdit* model_edit_ = nullptr;

	QLineEdit* coronal_diameter_edit_ = nullptr;
	QLineEdit* apical_diameter_edit_ = nullptr;
	QLineEdit* length_edit_ = nullptr;
};
