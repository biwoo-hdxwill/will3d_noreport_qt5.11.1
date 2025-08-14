#pragma once
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
#include <qtoolbutton.h>

#include "uiprimitive_global.h"

class QMenu;
class QAction;

class UIPRIMITIVE_EXPORT ImplantButton : public QToolButton {
	Q_OBJECT
public:
	ImplantButton(QWidget *parent = 0);
	virtual ~ImplantButton();

	enum class ButtonStatus {
		DEFAULT, SELECTED, PLACED
	};

public:
	void setIndex(int index) { index_ = index; }
	void SetStatus(const ButtonStatus& status);
	bool IsSelected() const noexcept;

signals:
	void sigAddImplant(int);
	void sigSelectImplant(int, ImplantButton::ButtonStatus);
	void sigDeleteImplant(int);

protected:
	// 마우스를 클릭 후 Release 했을때 해당 인덱스와 함께 sigMouseClicked 시그널을 전송
	virtual void mouseReleaseEvent(QMouseEvent * e);

private slots:
	void slotDeleteImplant();

private:
	void SelectStyleSheet();
	void ShowImplantMenu(const QPoint& pos);
	void DeleteImplantMenu();

private:
	int index_; // 버튼의 index
	ButtonStatus status_ = ButtonStatus::DEFAULT;

	QMenu* implant_menu_ = nullptr;
	QAction* delete_implant_ = nullptr;
};

