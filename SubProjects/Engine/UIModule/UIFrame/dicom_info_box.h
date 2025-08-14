#pragma once

/*=========================================================================

File:			class DicomInfoBox
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-06-12
Last modify:	2018-06-12

=========================================================================*/
#include <QFrame>
#include "uiframe_global.h"

class QLabel;

class UIFRAME_EXPORT DicomInfoBox : public QFrame {
public:
	DicomInfoBox(QFrame* parent = 0);
	~DicomInfoBox();

	void SetPatientID(const QString& id);
	void SetPatientNameGender(const QString& name, const QString& gender);
	void SetPatientAge(const QString& age);
	void SetImageKvp(const QString& kvp);
	void SetImageMa(const QString& ma);
	void SetScanDate(const QString& date);

private:
	QFrame* CreateVerticalLine();

private:
	QLabel* patient_id_label_ = nullptr;
	QLabel* patient_name_gender_label_ = nullptr;
	QLabel* patient_age_label_ = nullptr;
	QLabel* image_kvp_label_ = nullptr;
	QLabel* image_ma_label_ = nullptr;
	QLabel* image_capture_date_ = nullptr;
};
