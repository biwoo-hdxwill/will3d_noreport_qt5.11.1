#include "window_implant_sagittal.h"
#include <Engine/Common/Common/language_pack.h>

WindowImplantSagittal::WindowImplantSagittal(QWidget *parent)
	: Window(parent), rotate_(new QDoubleSpinBox) {
	Initialize();
}
WindowImplantSagittal::~WindowImplantSagittal() {}

void WindowImplantSagittal::Initialize() {
	rotate_->setRange(-180, 180);
	rotate_->setDecimals(0);
	rotate_->setSingleStep(1);
	rotate_->setValue(0);
	rotate_->setSuffix(QString::fromLocal8Bit("°"));
	rotate_->setObjectName(lang::LanguagePack::txt_rotate());

	std::vector<QAbstractSpinBox*> spin_boxes;
	spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(rotate_.get()));

	InitViewMenu(lang::LanguagePack::txt_selected_implant());
	Window::AddSpinBox(spin_boxes);
}
