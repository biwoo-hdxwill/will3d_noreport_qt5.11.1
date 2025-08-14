#include "window_tmj_frontal.h"

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/language_pack.h>
#include <QTextCodec>

/**=================================================================================================
class WindowTmjFrontal
*===============================================================================================**/
WindowTmjFrontal::WindowTmjFrontal(const TMJDirectionType& type, QWidget* parent)
	: type_(type),
	Window(parent),
	reset_(new QToolButton),
	undo_(new QToolButton),
	redo_(new QToolButton) 
{
	Initialize();
}

WindowTmjFrontal::~WindowTmjFrontal(void) {}

void WindowTmjFrontal::Initialize() 
{
	InitSpinboxes();

	QString dir_text = (type_ == TMJ_LEFT) ? lang::LanguagePack::txt_left_frontal()
		: lang::LanguagePack::txt_right_frontal();

	InitViewMenu(dir_text);

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
}

void WindowTmjFrontal::InitSpinboxes()
{
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	double width = settings.value("TMJ/frontal_width", 40.0).toString().toDouble();
	double height = settings.value("TMJ/frontal_height", 40.0).toString().toDouble();

	double width_maximum = settings.value("TMJ/frontal_width_maximum", 200.0).toString().toDouble();
	double height_maximum = settings.value("TMJ/frontal_height_maximum", 200.0).toString().toDouble();

	double width_stride = settings.value("TMJ/frontal_width_stride", 1.0).toString().toDouble();
	double height_stride = settings.value("TMJ/frontal_height_stride", 1.0).toString().toDouble();

	spin_box_.width.reset(new QDoubleSpinBox);
	spin_box_.width->setVisible(false);
	spin_box_.width->setDecimals(2);
	spin_box_.width->setSuffix(" mm");
	spin_box_.width->setRange(0.1, width_maximum);
	spin_box_.width->setValue(width);
	spin_box_.width->setObjectName(lang::LanguagePack::txt_width());
	spin_box_.width->setSingleStep(width_stride);

	spin_box_.height.reset(new QDoubleSpinBox);
	spin_box_.height->setVisible(false);
	spin_box_.height->setDecimals(2);
	spin_box_.height->setValue(0.0);
	spin_box_.height->setSuffix(" mm");
	spin_box_.height->setRange(1.0, height_maximum);
	spin_box_.height->setValue(height);
	spin_box_.height->setObjectName(lang::LanguagePack::txt_height());
	spin_box_.height->setSingleStep(height_stride);
}

void WindowTmjFrontal::Set3DCutMode(const bool& on)
{
	ClearViewMenuContents();
	if (on)
	{
		std::vector<QWidget*> buttons = { reset_.get(), undo_.get(), redo_.get() };
		Window::AddWidget(buttons);
	}
	else 
	{
		std::vector<QAbstractSpinBox*> spin_boxes;
		spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(spin_box_.width.get()));
		spin_boxes.push_back(dynamic_cast<QAbstractSpinBox*>(spin_box_.height.get()));
		Window::AddSpinBox(spin_boxes);
	}

	reset_->setVisible(on);
	undo_->setVisible(on);
	redo_->setVisible(on);

#ifdef WILL3D_EUROPE
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (btn_enable)
	{
		spin_box_.width->setVisible(!on);
		spin_box_.height->setVisible(!on);
	}	
#else
	spin_box_.width->setVisible(!on);
	spin_box_.height->setVisible(!on);
#endif // !WILL3D_EUROPE
}

void WindowTmjFrontal::ApplyPreferences() {
	// double interval_increments =
	// GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.interval_increments;
	// spin_box_.interval->setSingleStep(interval_increments);
	//
	// double thickness_increments =
	// GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.thickness_increments;
	// spin_box_.thickness->setSingleStep(thickness_increments);
}
