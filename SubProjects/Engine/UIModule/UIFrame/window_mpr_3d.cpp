#include "window_mpr_3d.h"

#include <qapplication.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3Theme.h>

WindowMPR3D::WindowMPR3D(WindowMPR3D::MPR3DMode mode, QWidget *parent) :
	Window(parent), mode_(mode), 
	reset_(new QToolButton), undo_(new QToolButton), redo_(new QToolButton) {
	Initialize();

	Set3DCutMode(false);
}

WindowMPR3D::~WindowMPR3D() {}

void WindowMPR3D::Set3DCutMode(const bool& on)
{
	reset_->setVisible(on);
	undo_->setVisible(on);
	redo_->setVisible(on);
}

void WindowMPR3D::Initialize() {
	bool expand_on = (mode_ == MPR3DMode::VR) ? true : false;
	InitViewMenu((mode_ == MPR3DMode::VR) ?
				 "3D" : lang::LanguagePack::txt_3d_zoom());

	std::vector<QWidget*> buttons = {
		reset_.get(), undo_.get(), redo_.get()
	};

	reset_->setStyleSheet(CW3Theme::getInstance()->ViewMenuBarButtonStyleSheet());
	undo_->setStyleSheet(CW3Theme::getInstance()->ViewMenuBarButtonStyleSheet());
	redo_->setStyleSheet(CW3Theme::getInstance()->ViewMenuBarButtonStyleSheet());

	reset_->setText("Reset");
	undo_->setText("Undo");
	redo_->setText("Redo");

	reset_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	undo_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	redo_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	reset_->setFixedHeight(20);
	undo_->setFixedHeight(20);
	redo_->setFixedHeight(20);

	AddWidget(buttons);
	if (expand_on)
		Window::AddMaximizeButton();
	
	connect(reset_.get(), SIGNAL(pressed()), this, SIGNAL(sig3DCutReset()));
	connect(undo_.get(), SIGNAL(pressed()), this, SIGNAL(sig3DCutUndo()));
	connect(redo_.get(), SIGNAL(pressed()), this, SIGNAL(sig3DCutRedo()));
}
