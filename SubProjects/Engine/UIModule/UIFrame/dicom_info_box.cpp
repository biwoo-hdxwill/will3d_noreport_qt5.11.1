#include "dicom_info_box.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <Engine/Common/Common/language_pack.h>
#include "../../Common/Common/W3Theme.h"

DicomInfoBox::DicomInfoBox(QFrame* parent)
	: QFrame(parent) {
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	setObjectName("Frame");
	setStyleSheet(CW3Theme::getInstance()->DicomInfoBoxStyleSheet());

	QHBoxLayout* dicom_layout = new QHBoxLayout();
	QVBoxLayout* patient_layout = new QVBoxLayout();
	QVBoxLayout* image_layout = new QVBoxLayout();
	patient_id_label_ = new QLabel();
	patient_name_gender_label_ = new QLabel();
	patient_age_label_ = new QLabel();
	image_kvp_label_ = new QLabel();
	image_ma_label_ = new QLabel();
	image_capture_date_ = new QLabel();

	//SetPatientID("id");
	//SetPatientNameGender("name", "gender");
	//SetPatientAge("age");
	//SetImageKvp("kVp");
	//SetImageMa("mA");

	patient_layout->setContentsMargins(0, 0, 0, 0);
	patient_layout->setSpacing(0);
	patient_layout->addWidget(patient_id_label_);
	patient_layout->addWidget(patient_name_gender_label_);
	patient_layout->addWidget(patient_age_label_);

	image_layout->setContentsMargins(0, 0, 0, 0);
	image_layout->setSpacing(0);
	image_layout->addWidget(image_capture_date_);
	image_layout->addWidget(image_kvp_label_);
	image_layout->addWidget(image_ma_label_);

	dicom_layout->addLayout(patient_layout);
	dicom_layout->addWidget(CreateVerticalLine());
	dicom_layout->addLayout(image_layout);

	setLayout(dicom_layout);
	setFixedWidth(320);
}

DicomInfoBox::~DicomInfoBox() {
}

void DicomInfoBox::SetPatientID(const QString& id) {
	patient_id_label_->setText(lang::LanguagePack::txt_id() + " : " + id);
}

void DicomInfoBox::SetPatientNameGender(const QString& name, const QString& gender) {
	QString gender_string("M");
	if (gender.contains("F", Qt::CaseInsensitive))
		gender_string = "F";
	patient_name_gender_label_->setText(lang::LanguagePack::txt_name() + " : " + name + " [" + gender_string + "]");
}

void DicomInfoBox::SetPatientAge(const QString& age) {
	patient_age_label_->setText(lang::LanguagePack::txt_age() + " : " + age);
}

void DicomInfoBox::SetImageKvp(const QString& kvp) {
	image_kvp_label_->setText("[kVp] : " + kvp);
}

void DicomInfoBox::SetImageMa(const QString& ma) {
	image_ma_label_->setText("[mA] : " + ma);
}

void DicomInfoBox::SetScanDate(const QString & date) {
	image_capture_date_->setText(lang::LanguagePack::txt_scan_date() + " : " + date);
}

QFrame* DicomInfoBox::CreateVerticalLine() {
	QFrame* line = new QFrame();

	line->setObjectName("Line");
	line->setFrameShadow(QFrame::Plain);
	line->setFrameShape(QFrame::VLine);
	line->setLineWidth(0);
	line->setMidLineWidth(0);
	line->setStyleSheet(CW3Theme::getInstance()->LineSeparatorStyleSheet());
	line->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	return line;
}
