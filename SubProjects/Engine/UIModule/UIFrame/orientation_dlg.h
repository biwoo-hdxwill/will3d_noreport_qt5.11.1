#pragma once

/**=================================================================================================

Project:		UIFrame
File:			orientation_dlg.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-14
Last modify: 	2018-02-14

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <memory>
#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QSpinBox;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;

class UIFRAME_EXPORT OrientationDlg : public CW3Dialog {
	Q_OBJECT

public:
	OrientationDlg(QWidget * parent = 0);
	~OrientationDlg();

	OrientationDlg(const OrientationDlg&) = delete;
	OrientationDlg& operator=(const OrientationDlg&) = delete;

signals:
	void sigResetOrientation();
	void sigGridOnOff(bool);

public:
	void SetView(QWidget* orien_view_a,
					QWidget* orien_view_r,
					QWidget* orien_view_i);

	inline QSpinBox* GetOrienA() const { return spin_.rotate_a.get(); }
	inline QSpinBox* GetOrienR() const { return spin_.rotate_r.get(); }
	inline QSpinBox* GetOrienI() const { return spin_.rotate_i.get(); }

  bool IsGridOn() const;

public slots:
	virtual void done(int r) override;
	virtual int exec() override;

private slots:
	void slotClickedSetDefault();
	void slotClickedReset();
	void slotGridOnOff(bool);

private:
	void SetSpinBoxParams(QSpinBox* spin_box);
	QVBoxLayout* SetLayoutOrienA(QWidget* orien_view_a);
	QVBoxLayout* SetLayoutOrienR(QWidget* orien_view_r);
	QVBoxLayout* SetLayoutOrienI(QWidget* orien_view_i);
	QLabel * CreateLabel(const QString & text);

private:
	struct SpinBox {
		std::unique_ptr<QSpinBox> rotate_a;
		std::unique_ptr<QSpinBox> rotate_r;
		std::unique_ptr<QSpinBox> rotate_i;
	};

	std::unique_ptr<QToolButton> set_default_;
	std::unique_ptr<QToolButton> reset_;
	std::unique_ptr<QToolButton> grid_on_ = nullptr;
	std::unique_ptr<QHBoxLayout> view_layout_;
	std::vector<QString> switch_icon_path_ = {
		":/image/viewmenu/toggle_on.png",
		":/image/viewmenu/toggle_off.png"
	};
	SpinBox spin_;
};
