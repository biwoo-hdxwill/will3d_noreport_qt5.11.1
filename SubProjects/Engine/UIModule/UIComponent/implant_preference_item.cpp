#include "implant_preference_item.h"

#include <iostream>
#include <qevent.h>
#include <qboxlayout.h>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/color_will3d.h"

namespace {
const QString kTokenData(":");
const QString kTokenSpec("/");
} // end of namespace

ImplantPreferenceItem::ImplantPreferenceItem(int implant_number, QWidget *parent)
	: QFrame(parent) {
	InitUI(implant_number);
}

ImplantPreferenceItem::~ImplantPreferenceItem() {
}

void ImplantPreferenceItem::ResetItem(const int implant_number) {
	disconnect(&use_preset_, SIGNAL(stateChanged(int)), this, SLOT(slotUsePresetClicked(int)));
	use_preset_.setText(QString::number(implant_number));
	use_preset_.setChecked(false);
	use_preset_.setEnabled(false);
	manufacturer_.setText(kTokenData);
	product_.setText(kTokenData);
	spec_.setText(kTokenData);
	connect(&use_preset_, SIGNAL(stateChanged(int)), this, SLOT(slotUsePresetClicked(int)));
}

void ImplantPreferenceItem::UpdateItemInfo(const int implant_number,
										   const QString& manufacturer_name,
										   const QString& product_name,
										   const QString& diameter,
										   const QString& length) {
	disconnect(&use_preset_, SIGNAL(stateChanged(int)), this, SLOT(slotUsePresetClicked(int)));
	use_preset_.setText(QString::number(implant_number));
	use_preset_.setChecked(true);
	use_preset_.setEnabled(true);
	manufacturer_.setText(kTokenData + manufacturer_name);
	product_.setText(kTokenData + product_name);
	spec_.setText(kTokenData + diameter + kTokenSpec + length);
	connect(&use_preset_, SIGNAL(stateChanged(int)), this, SLOT(slotUsePresetClicked(int)));
}

QString ImplantPreferenceItem::implant_number() const noexcept {
	return use_preset_.text();
}

QString ImplantPreferenceItem::manufacturer() const noexcept {
	QStringList man = manufacturer_.text().split(kTokenData);
	return man.at(1);
}

QString ImplantPreferenceItem::product() const noexcept {
	QStringList product = product_.text().split(kTokenData);
	return product.at(1);
}

QString ImplantPreferenceItem::diameter() const noexcept {
	QStringList spec_raw = spec_.text().split(kTokenData);
	QStringList spec = spec_raw.at(1).split(kTokenSpec);
	return spec.at(0);
}

QString ImplantPreferenceItem::length() const noexcept {
	QStringList spec_raw = spec_.text().split(kTokenData);
	QStringList spec = spec_raw.at(1).split(kTokenSpec);
	if (spec.size() < 2)
		return QString();
	return spec.at(1);
}

void ImplantPreferenceItem::SyncSelectedStatus(bool status) {
	current_selected_ = status;
	this->setStyleSheet(QString("QFrame{ background-color: %1; }")
						.arg(ColorGeneral::kBackground.name()));
}

void ImplantPreferenceItem::leaveEvent(QEvent * event) {
	if (!current_selected_) {
		this->setStyleSheet(QString("QFrame{ background-color: %1; }")
							.arg(ColorGeneral::kBackground.name()));
	}
	QFrame::leaveEvent(event);
}

void ImplantPreferenceItem::enterEvent(QEvent * event) {
	if (!current_selected_) {
		this->setStyleSheet(QString("QFrame{ background-color: %1; }")
						.arg(ColorGeneral::kBackgroundHighlight.name()));
	}
	QFrame::enterEvent(event);
}

void ImplantPreferenceItem::mouseReleaseEvent(QMouseEvent * event) {
	current_selected_ ^= true;
	emit sigSetCurrentItemInfo(manufacturer(), product(), diameter(), length());
	QFrame::mouseReleaseEvent(event);

	event->ignore();
}

void ImplantPreferenceItem::slotUsePresetClicked(int state) {
	int implant_number = use_preset_.text().toInt();
	ResetItem(implant_number);
	emit sigClearCurrentItem(implant_number);
}

void ImplantPreferenceItem::InitUI(int index) {
	ResetItem(index);

	QVBoxLayout* main_layout = new QVBoxLayout;
	main_layout->setSpacing(8);
	main_layout->setContentsMargins(6, 6, 6, 6);
	main_layout->addWidget(&use_preset_);
	main_layout->addWidget(new QLabel(lang::LanguagePack::txt_manufacturer()));
	main_layout->addWidget(&manufacturer_);
	main_layout->addWidget(new QLabel(lang::LanguagePack::txt_product_line()));
	main_layout->addWidget(&product_);
	main_layout->addWidget(new QLabel(lang::LanguagePack::txt_diameter()
									  + "/" + lang::LanguagePack::txt_length()));
	main_layout->addWidget(&spec_);

	this->setLayout(main_layout);
	this->setMinimumHeight(145);
	connect(&use_preset_, SIGNAL(stateChanged(int)), this, SLOT(slotUsePresetClicked(int)));

	this->setStyleSheet(QString("QFrame{ background-color: %1; }")
						.arg(ColorGeneral::kBackground.name()));
}
