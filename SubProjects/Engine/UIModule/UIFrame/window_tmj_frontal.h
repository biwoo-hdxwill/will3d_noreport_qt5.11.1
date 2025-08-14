#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window_tmj_frontal.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-28
Last modify: 	2018-11-28

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <QDoubleSpinBox>
#include <QToolButton>

#include "../../Common/Common/W3Enum.h"
#include "window.h"

class UIFRAME_EXPORT WindowTmjFrontal : public Window
{
	Q_OBJECT
public:
	explicit WindowTmjFrontal(const TMJDirectionType& type, QWidget *parent = 0);

	virtual ~WindowTmjFrontal() override;

	WindowTmjFrontal(const WindowTmjFrontal&) = delete;
	WindowTmjFrontal& operator=(const WindowTmjFrontal&) = delete;

public:
	QDoubleSpinBox* GetWidth() { return  spin_box_.width.get(); }
	QDoubleSpinBox* GetHeight() { return  spin_box_.height.get(); }
	QToolButton* GetReset() { return reset_.get(); }
	QToolButton* GetUndo() { return undo_.get(); }
	QToolButton* GetRedo() { return redo_.get(); }

	void Set3DCutMode(const bool& on);
	void ApplyPreferences();

private:
	virtual void Initialize() override;
private:
	void InitSpinboxes();

private:
	struct SpinBox {
		std::unique_ptr<QDoubleSpinBox> width;
		std::unique_ptr<QDoubleSpinBox> height;
	};

	SpinBox spin_box_;
	TMJDirectionType type_;

	std::unique_ptr<QToolButton> reset_ = nullptr;
	std::unique_ptr<QToolButton> undo_ = nullptr;
	std::unique_ptr<QToolButton> redo_ = nullptr;
};

