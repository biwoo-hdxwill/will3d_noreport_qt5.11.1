#include "window_implant_3d.h"

#include <Engine/Common/Common/color_will3d.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3Theme.h>

WindowImplant3D::WindowImplant3D(QWidget *parent)
	: Window(parent), clip_(new QToolButton) {
	Initialize();
}

WindowImplant3D::~WindowImplant3D() {}

void WindowImplant3D::Initialize() {
	std::vector<QWidget*> buttons = { clip_.get() };

	// There is no translation text for "3D"
	InitViewMenu("3D");
	AddWidget(buttons);
	AddMaximizeButton();

	clip_->setObjectName("ViewMenubarButton");
	clip_->setStyleSheet(
		CW3Theme::getInstance()->ViewMenuBarOnOffSwitchStyleSheet(
			ColorGeneral::kCaption, switch_icon_path_.at(0)));
	clip_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	clip_->setFixedSize(50, 20);
	clip_->setCheckable(true);
	
	connect(clip_.get(), SIGNAL(toggled(bool)), this, SLOT(slotClip3DOnOff(bool)));
}

void WindowImplant3D::slotClip3DOnOff(bool on) {
	// on off 에 따라 이미지를 바꿔야 한다.
	if (on)
		clip_->setStyleSheet(
			CW3Theme::getInstance()->ViewMenuBarOnOffSwitchStyleSheet(
				ColorGeneral::kCaption, switch_icon_path_.at(1)));
	else
		clip_->setStyleSheet(
			CW3Theme::getInstance()->ViewMenuBarOnOffSwitchStyleSheet(
				ColorGeneral::kCaption, switch_icon_path_.at(0)));
	// toggle on 되었을 떄 clipping off 해야 함. 기본 상태가 clipping on 임
	emit sigClip3DOnOff(!on); 
}
