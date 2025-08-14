#include "W3ImplantButton.h"
/*=========================================================================

File:			class CW3EllipseItem
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			Hong Jung, Seo Seok Man
First Date:		2016-05-25
Modify Date:	2018-03-30
Version:		2.0

Copyright (c) 2016 ~ 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <iostream>

#include <qevent.h>
#include <qmenu.h>
#include <qaction.h>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Memory.h"

ImplantButton::ImplantButton(QWidget *parent)
	: QToolButton(parent) {
	this->setFixedSize(common::ui_define::kImplantButtonWidth,
					   common::ui_define::kImplantButtonHeight);
	SelectStyleSheet();

	delete_implant_ = new QAction("Delete Implant", this);
	implant_menu_ = new QMenu(this);
	implant_menu_->addAction(delete_implant_);
	connect(delete_implant_, SIGNAL(triggered()), this, SLOT(slotDeleteImplant()));
}

ImplantButton::~ImplantButton() {
	DeleteImplantMenu();
}

// 마우스를 클릭 후 Release 했을때 선택 상태에 따른 시그널을 전송
void ImplantButton::mouseReleaseEvent(QMouseEvent * e) {
	QToolButton::mouseReleaseEvent(e);

	auto button = e->button();
	if (button == Qt::LeftButton) {
		switch (status_) {
		case ImplantButton::ButtonStatus::DEFAULT:
			SetStatus(ButtonStatus::SELECTED);
			emit sigAddImplant(index_);
			return;
		case ImplantButton::ButtonStatus::SELECTED:
			SetStatus(ButtonStatus::PLACED);
			break;
		case ImplantButton::ButtonStatus::PLACED:
			SetStatus(ButtonStatus::SELECTED);
			break;
		default:
			common::Logger::instance()->Print(common::LogType::ERR,
											  "ImplantButton::mouseReleaseEvent : Wrong Implant status");
			return;
		}
		emit sigSelectImplant(index_, status_);
	} else if (button == Qt::RightButton &&
			   status_ != ButtonStatus::DEFAULT) {
		ShowImplantMenu(e->pos());
	}
}

void ImplantButton::SetStatus(const ImplantButton::ButtonStatus& status) {
	status_ = status;
	SelectStyleSheet();
}

bool ImplantButton::IsSelected() const noexcept {
	return status_ == ButtonStatus::SELECTED ? true : false;
}

void ImplantButton::slotDeleteImplant() {
	SetStatus(ButtonStatus::DEFAULT);
	emit sigDeleteImplant(index_);
}

void ImplantButton::SelectStyleSheet() {
	switch (status_) {
	case ImplantButton::ButtonStatus::DEFAULT:
		this->setStyleSheet(CW3Theme::getInstance()->DefaultImplantButtonStyleSheet());
		break;
	case ImplantButton::ButtonStatus::SELECTED:
		this->setStyleSheet(CW3Theme::getInstance()->SelecedtImplantButtonStyleSheet());
		break;
	case ImplantButton::ButtonStatus::PLACED:
		this->setStyleSheet(CW3Theme::getInstance()->PlacedImplantButtonStyleSheet());
		break;
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "ImplantButton::SelectStyleSheet : Wrong Implant status");
		break;
	}
}

void ImplantButton::ShowImplantMenu(const QPoint& pos) {
	auto g_pos = mapToGlobal(pos);
	implant_menu_->popup(g_pos);
}

void ImplantButton::DeleteImplantMenu() {
	SAFE_DELETE_OBJECT(implant_menu_);
	SAFE_DELETE_OBJECT(delete_implant_);
}
