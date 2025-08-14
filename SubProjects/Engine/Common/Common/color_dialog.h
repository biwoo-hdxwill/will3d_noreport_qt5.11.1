#pragma once
/*=========================================================================

File:			color_dialog.h
Language:		C++11
Library:        Qt 5.14.2
Author:			Lim Tae Kyun
First date:		2020-09-24
Last modify:	2020-09-25

=========================================================================*/

#include <map>

#include "W3Dialog.h"

class QLabel;
class QPushButton;
class QColorDialog;

class COMMON_EXPORT ColorDialog : public CW3Dialog
{
	Q_OBJECT
public:
	ColorDialog(QWidget* parent = nullptr, const Theme theme = Theme::Dark, bool screen_coordinate_visible = false);
	virtual ~ColorDialog();

	void SetCurrentColor(const QColor& color);
	QColor CurrentColor() const;
	QColor SelectedColor() const;

public slots:
	virtual int exec() override;

private:
	void InsertLabelObject();
	void InsertQPushButtonObject();
	void SetLabelText(const QString& key, const QString& text);
	void SetPushButtonText(const QString& key, const QString& text);
	void SetScreenCoordinateLabelVisible(bool screen_coordinate_visible);
	void Connection();
	void SetFocusOKPushButton();

private:
	QColorDialog* color_dlg_ = nullptr;
	std::map<QString, QLabel*> label_map_;
	std::map<QString, QPushButton*> push_button_map_;
};
