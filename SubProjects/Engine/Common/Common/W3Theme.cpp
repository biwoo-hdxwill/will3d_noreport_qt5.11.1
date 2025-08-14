#include "W3Theme.h"

/*=========================================================================

File:			class CW3Theme
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-03-23
Last modify:	2016-03-23

=========================================================================*/
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QString>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QDebug>

#include "global_preferences.h"
#include "W3Memory.h"
#include "W3Logger.h"
#include "color_will3d.h"
#include "define_ui.h"

CW3Theme* CW3Theme::m_pInstance = nullptr;

namespace {
	const int kMinimumScreenSize = 1280 * 960;
	const int kDirectionTextFontSize = 40;
	const int kImplantBtnBorder = 2;
	const int kImplantBtnRadius = 18;

	const QString kSectionKeyFontSize = "INTERFACE/font_size";
}
CW3Theme::CW3Theme() {
	Q_INIT_RESOURCE(image);

	m_tabWestStyle = new CW3TabWestStyle();
	m_toolMenuIconStyle = new CWToolMenuIconStyle();
	m_toolIconButtonStyle = new CW3ToolIconButtonStyle();
	m_toolTaskStyle = new CW3ToolTaskStyle();
	m_cephTracingTaskToolStyle = new CW3CephTracingTaskToolStyle();
}
CW3Theme::~CW3Theme() {
	SAFE_DELETE_OBJECT(m_tabWestStyle);
	SAFE_DELETE_OBJECT(m_toolMenuIconStyle);
	SAFE_DELETE_OBJECT(m_toolIconButtonStyle);
	SAFE_DELETE_OBJECT(m_toolTaskStyle);
	SAFE_DELETE_OBJECT(m_cephTracingTaskToolStyle);

}
void CW3Theme::setAppTheme(QApplication* app) {
	int id = QFontDatabase::addApplicationFont(":/font/Roboto/Roboto-Regular.ttf");
	//int id = QFontDatabase::addApplicationFont(":/font/Noto Sans/NotoSans-Regular.ttf");
	if (id > -1) {
		QString family = QFontDatabase::applicationFontFamilies(id).at(0);
		m_fontSystem = QFont(family);

		common::Logger::instance()->Print(
			common::LogType::INF, "Application font : " + family.toStdString());
	}

	//get resolution
	QRect mainScreenSize = app->desktop()->availableGeometry(app->desktop()->screen(0));
	
	//afterwards handle size...
	if (mainScreenSize.width()*mainScreenSize.height() <= kMinimumScreenSize)
		m_isLowResolution = true;
	else
		m_isLowResolution = false;

	if (m_isLowResolution) {
		m_sizeW_Vbar = 176;
		m_sizeH_titlebar = 20;

		//size_global_preferences_dialog_ = QSize(384, 480);
		size_global_preferences_tab_ = QSize(72, 16);
		m_size_tabWest = QSize(61, 32);
		m_size_tool_task = QSize(157, 35);
		m_size_tool_menu = QSize(7, 35);
		m_size_tool_otf = QSize(29, 29);
		size_tool_icon_ = QSize(28, 41);
		m_size_tool_tmj = QSize(38, 19);
		m_size_button = QSize(46, 19);
		tool_icon_sub_menu_height_ = 4;

		fontsize_small_ = 9;
		fontsize_regular_ = 10;

		m_toolVBarSizeInfo.marginContents = QMargins(2, 0, 2, 0);
		m_toolVBarSizeInfo.spacingS = 2;
		m_toolVBarSizeInfo.spacingM = 5;
		m_toolVBarSizeInfo.spacingL = 10;
		//int dpiY = app->desktop()->logicalDpiY();
		//m_fontSystem.setPointSize(m_fontSystem.pixelSize() * 72 / dpiY);
		//m_fontSystem.setLetterSpacing(QFont::AbsoluteSpacing, -0.5);

		m_toolVBarSizeInfo.marginBox = QMargins(2, 4, 2, 4);
	} else {
		m_sizeW_Vbar = 210;
		m_sizeH_titlebar = 27;

		//size_global_preferences_dialog_ = QSize(480, 600);
		size_global_preferences_tab_ = QSize(90, 20);
		m_size_tabWest = QSize(80, 40);
		m_size_tool_task = QSize(187, 49);
		m_size_tool_menu = QSize(7, 49);
		m_size_tool_otf = QSize(39, 39);
		size_tool_icon_ = QSize(42, 44);
		m_size_tool_tmj = QSize(58, 19);
		m_size_button = QSize(85, 19);

		tool_icon_sub_menu_height_ = 5;

		fontsize_small_ = 11;
		fontsize_regular_ = 13;

		m_toolVBarSizeInfo.marginContents = QMargins(5, 0, 5, 0);
		m_toolVBarSizeInfo.spacingS = 3;
		m_toolVBarSizeInfo.spacingM = 7;
		m_toolVBarSizeInfo.spacingL = 12;
		//int dpiY = app->desktop()->logicalDpiY();
		//m_fontSystem.setPointSize(m_fontSystem.pixelSize() * 72 / dpiY);
		//m_fontSystem.setLetterSpacing(QFont::AbsoluteSpacing, -0.2);

		m_toolVBarSizeInfo.marginBox = QMargins(5, 10, 5, 10);
	}

	GlobalPreferences::Size gui_size = GlobalPreferences::GetInstance()->preferences_.general.interfaces.gui_size;
	int font_size_level = static_cast<int>(GlobalPreferences::GetInstance()->preferences_.general.interfaces.font_size);

	float gui_size_level = 1.0f;
	switch (gui_size)
	{
	case GlobalPreferences::Size::Small:
		gui_size_level = 1.0f;
		break;
	case GlobalPreferences::Size::Medium:
		gui_size_level = 1.2f;
		break;
	case GlobalPreferences::Size::Large:
		gui_size_level = 1.4f;
		break;
	default:
		break;
	}

	m_sizeW_Vbar = m_sizeW_Vbar * gui_size_level;
	m_sizeH_titlebar = m_sizeH_titlebar * gui_size_level;

	size_global_preferences_tab_ *= gui_size_level;
	m_size_tabWest *= gui_size_level;
	m_size_tool_task *= gui_size_level;
	m_size_tool_menu *= gui_size_level;
	m_size_tool_otf *= gui_size_level;
	size_tool_icon_ *= gui_size_level;
	m_size_tool_tmj *= gui_size_level;
	m_size_button *= gui_size_level;

	tool_icon_sub_menu_height_ *= gui_size_level;

	fontsize_small_ += font_size_level;
	fontsize_regular_ += font_size_level;

	m_fontSystem.setPixelSize(fontsize_regular_);

	m_tabWestMargins.setLeft(m_size_tabWest.width() + 4);
	m_tabWestMargins.setRight(4);
	m_tabWestMargins.setBottom(4);
	m_tabWestMargins.setTop(4);
	//handle size end...

	app->setFont(m_fontSystem);
	
	//qDebug() << QStyleFactory::keys();

	app->setStyle(QStyleFactory::create("Fusion"));

	QPalette darkPalette;
	darkPalette.setColor(QPalette::Window, QColor("#FF2F3243"));
	darkPalette.setColor(QPalette::WindowText, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::Base, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::AlternateBase, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::ToolTipBase, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::ToolTipText, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::Text, QColor("#FF000000"));
	//darkPalette.setColor(QPalette::Text, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::Button, QColor("#FF2F3243"));
	darkPalette.setColor(QPalette::ButtonText, QColor("#FFFFFFFF"));
	darkPalette.setColor(QPalette::BrightText, QColor("#FFFF0000"));
	darkPalette.setColor(QPalette::Link, QColor("#FF22A7EF"));
	darkPalette.setColor(QPalette::Highlight, QColor("#FF22A7EF"));
	darkPalette.setColor(QPalette::HighlightedText, ColorGeneral::kBorder);
	app->setPalette(darkPalette);
	app->setStyleSheet(appAllInOneStyleSheet());
}
QString CW3Theme::appToolButtonStyleSheet() {
	return QString(
		"QToolButton {"
		"font: %1px;"
		"border: 1px solid %9; padding: 0px;"
		"width: %2px; height: %3px;"
		"background-color: %4;"
		"}"
		"QToolButton:hover {background-color: %5;}"
		"QToolButton:pressed {background-color: %6;}"
		"QToolButton:checked {background-color: %7;}"
		"QToolButton:unchecked {background-color: %8;}"
		)
		.arg(fontsize_regular_)
		.arg(m_size_button.width())
		.arg(m_size_button.height())
		.arg(ColorToolButton::kNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name())
		.arg(ColorGeneral::kBorder.name());
}
QString CW3Theme::appQSliderStyleSheet()
{
	return QString(
		"QSlider::handle:horizontal"
		"{"
		"	image: url(:/image/slider/handle.png);"
		"	margin: -10px 0;"
		"}"
		"QSlider::handle:horizontal:pressed"
		"{"
		"	image: url(:/image/slider/handle_pressed.png);"
		"	margin: -10px 0;"
		"}"
		"QSlider::groove:horizontal"
		"{"
		"	border-image: url(:/image/slider/hsliderbase.png);"
		"	margin-top: 2px;"
		"	margin-bottom: 2px;"
		"	height: 4px;"
		"}"
		"QSlider::sub-page:horizontal"
		"{"
		"	border-image: url(:/image/slider/hslider.png);"
		"	background: url(:/image/slider/hslider.png);"
		"	border-top: 2px transparent;"
		"	border-bottom: 2px transparent;"
		"	border-right: 4px transparent;"
		"	border-left: 4px transparent;"
		"}"

		"QSlider::handle#2DView"
		"{"
		"	image: url(:/image/slider/mpr_handle.png);"
		"	margin: 0px 0;"
		"}"
		"QSlider::groove#2DView"
		"{"
		"	image: url(:/image/slider/mpr_base.png);"
		"	margin: 0px 0;"
		"}"
	);
}
QString CW3Theme::appQScrollBarStyleSheet() { //TODO xxxxxxxxxxx
	return QString(
		"QScrollBar:vertical { width: 10px; border: 0px; background: #FFDCDCDC; }"
		"QScrollBar::handle:vertical {"
		"margin: 13px 0px 13px 0px;"
		"margin-left: 1px; margin-right: 1px;"
		"	border-image: url(:/image/scrollbar/vscroll.png) 6 3 6 3;"
		"	background: url(:/image/scrollbar/vscroll.png); "
		"	border-top: 6px transparent;"
		"	border-bottom: 6px transparent;"
		"	border-right: 3px transparent;"
		"	border-left: 3px transparent;"
		"}"
		"QScrollBar::add-line:vertical {"
		"background-image: url(:/image/scrollbar/vscrolldown.png);"
		"background-position: center;"
		"}"
		"QScrollBar::add-line::pressed:vertical {"
		"background-image: url(:/image/scrollbar/vscrolldown.png);"
		"background-position: center;"
		"border: 1px solid #FF2F3243"
		"}"
		"QScrollBar::sub-line:vertical {"
		"background-image: url(:/image/scrollbar/vscrollup.png);"
		"background-position: center;"
		"}"
		"QScrollBar::sub-line::pressed:vertical {"
		"background-image: url(:/image/scrollbar/vscrollup.png);"
		"background-position: center;"
		"border: 1px solid #FF2F3243"
		"}"

		"QScrollBar:horizontal { height: 10px; border: 0px; background: #FFDCDCDC; }"
		"QScrollBar::handle:horizontal {"
		"margin: 0px 13px 0px 13px;"
		"margin-top: 1px; margin-bottom: 1px;"
		"	border-image: url(:/image/scrollbar/hscroll.png) 3 6 3 6;"
		"	background: url(:/image/scrollbar/hscroll.png); "
		"	border-top: 3px transparent;"
		"	border-bottom: 3px transparent;"
		"	border-right: 6px transparent;"
		"	border-left: 6px transparent;"
		"}"
		"QScrollBar::add-line:horizontal {"
		"background-image: url(:/image/scrollbar/hscrollright.png);"
		"background-position: center;"
		"}"
		"QScrollBar::add-line::pressed:horizontal {"
		"background-image: url(:/image/scrollbar/hscrollright.png);"
		"background-position: center;"
		"border: 1px solid #FF2F3243"
		"}"
		"QScrollBar::sub-line:horizontal {"
		"background-image: url(:/image/scrollbar/hscrollleft.png);"
		"background-position: center;"
		"}"
		"QScrollBar::sub-line::pressed:horizontal {"
		"background-image: url(:/image/scrollbar/hscrollleft.png);"
		"background-position: center;"
		"border: 1px solid #FF2F3243"
		"}"
		);
}
QString CW3Theme::appQRadioButtonStyleSheet() {
	return QString(
		"QRadioButton { font: %1px; }"
		"QRadioButton:enabled { color: white; }"
		"QRadioButton:disabled { color: gray; }"
		"QRadioButton::indicator:checked {image: url(:/image/radio/btn_radio_pressed.png);}"
		"QRadioButton::indicator:unchecked {image: url(:/image/radio/btn_radio.png);}"
		"QRadioButton::indicator:unchecked:hover {image: url(:/image/radio/btn_radio_hover.png);}"
		).arg(fontsize_small_);
}
QString CW3Theme::appQCheckBoxStyleSheet() {
	return QString(
		"QCheckBox { font: %1px; }"
		"QCheckBox:enabled { color: white; }"
		"QCheckBox:disabled { color: gray; }"
		"QCheckBox::indicator:checked {image: url(:/image/check/btn_checkbox_checked.png);}"
		"QCheckBox::indicator:unchecked {image: url(:/image/check/btn_checkbox.png);}"
		"QCheckBox::indicator:unchecked:hover {image: url(:/image/check/btn_checkbox_hover.png);}"
		"QCheckBox::indicator:disabled:unchecked {image: url(:/image/check/btn_checkbox_disable_unchecked.png);}"
		"QCheckBox::indicator:disabled:checked {image: url(:/image/check/btn_checkbox_disable_checked.png);}"
		).arg(fontsize_small_);
}
QString CW3Theme::appQGroupBoxStyleSheet() {
	return QString(
		"QGroupBox { font: %1px; }"
		"QGroupBox::indicator:checked {image: url(:/image/check/btn_checkbox_pressed.png);}"
		"QGroupBox::indicator:unchecked {image: url(:/image/check/btn_checkbox.png);}"
		"QGroupBox::indicator:unchecked:hover {image: url(:/image/check/btn_checkbox_hover.png);}"
	).arg(fontsize_small_);
}
QString CW3Theme::appQComboBoxStyleSheet() {
#if 1
	return QString(
		"QComboBox"
		"{"
		"	font: %1px;"
		"	border: 1px solid %2;"
		"	color: %2;"
		"	background: white;"
		"}"
		"QComboBox::drop-down"
		"{"
		"	background-image: url(:/image/combobox/arrow_black.png);"
		"	background-position: center;"
		"	background-repeat: no-repeat;"
		"	subcontrol-origin: padding;"
		"	subcontrol-position: top right;"
		"	width: 18px;"
		"	border-left-width: 1px;"
		"	border-left-color: #FF272727;"
		"	border-left-style: solid;"
		"	border-top-right-radius: 0px;"
		"	border-bottom-right-radius: 0px;"
		"}"
		"QComboBox::item:alternate"
		"{"
		"	background: white;"
		"}"
		"QComboBox::item:selected"
		"{"
		"	border: 1px solid transparent;"
		"	color: white;"
		"	background: #FF272727;"
		"}"
		"QComboBox::indicator"
		"{"
		"	background-color: transparent;"
		"	selection-background-color: transparent;"
		"	color: transparent;"
		"	selection-color: transparent;"
		"}"
		"QComboBox QListView"
		"{"
		"	background-color: white;"
		"	border: 1px solid gray;"
		"}"
	).arg(fontsize_small_)
		.arg(ColorGeneral::kBorder.name());
#else
	return QString(
		/*"QComboBox {"
		"background : black;"
		"    padding: 1px 18px 1px 3px;"
		"}"
		"QComboBox:on {"
		"background : black;"
		"}"*/

		"QComboBox {"
		"font: %1px;"
		"border: 2px solid white;"
		"padding-left: 5px;"
		"color: #FF272727;"
		"background: white;"
		"}"
		"QComboBox::drop-down {"
		"background-image: url(:/image/combobox/arrow_black.png);"
		"background-position: center;"
		"background-repeat: no-repeat;"
		"subcontrol-origin: padding;"
		"subcontrol-position: top right;"
		"width: 18px;"
		"border-left-width: 1px;"
		"border-left-color: #FF272727;"
		"border-left-style: solid;"
		"border-top-right-radius: 0px;"
		"border-bottom-right-radius: 0px;"
		"}"
#if 1
		"QComboBox QAbstractItemView {"
		"background-color: #FF393939;"
		"}"
		"QComboBox::item {"
		"color: #FF272727;"
		"background: white;"
		"}"
		"QComboBox::item:checked {"
		"font-weight: bold;"
		"color: white;"
		"background: #FF272727;"
		"}"
		"QComboBox::item:selected {"
		"color: white;"
		"background: #FF272727;"
		"}"
#else
		"QComboBox QListView {"
		"	background-color: #FF393939;"
		"}"
		"QComboBox::item {"
		"	color: #FF272727;"
		"	background: white;"
		"}"
		"QComboBox::item:selected {"
		"	color: white;"
		"	background: #FF272727;"
		"}"
#endif
	).arg(fontsize_small_);
#endif
}
QString CW3Theme::appQDoubleSpinBoxStyleSheet() { //TODO xxxxxxxxxxxxxxxxxxxxxxxx
	return QString(
		"QDoubleSpinBox {  font: %1px; color: %4; border: 1px solid %4; width %2px; height: %3px; padding-left: 5px; background: #FFFFFFFF; } "
		"QDoubleSpinBox::up-button  { border-left: 1px solid;"
		" width: 18px; height: 10px; background-image: url(:/image/spinbox/arrowup.png);"
		" background-position: center; background-repeat: no-repeat;}"
		"QDoubleSpinBox::up-button:pressed { background-color: #FF22A7EF; }"
		"QDoubleSpinBox::down-button  {  border-left: 1px solid ;"
		" width: 18px; height: 10px; background-image: url(:/image/spinbox/arrowdown.png);"
		" background-position: center; background-repeat: no-repeat;}"
		"QDoubleSpinBox::down-button:pressed { background-color: #FF22A7EF; }"
		)
		.arg(fontsize_small_)
		.arg(m_size_button.width())
		.arg(m_size_button.height())
		.arg(ColorGeneral::kBorder.name());
}
QString CW3Theme::ViewSpinBoxStyleSheet() {
	return QString(
		"QSpinBox { font: %1px; color: %2; background: #FFFFFFFF; } "
		"QDoubleSpinBox { font: %1px; color: %2; background: #FFFFFFFF; } "
	).arg(fontsize_small_)
	.arg(ColorGeneral::kBorder.name());

}
QString CW3Theme::appQLabelStyleSheet() {
	return QString(
		"QLabel { font: %1px; }"
		"QLabel:disabled { color: gray; }"
	).arg(fontsize_small_);
}
QString CW3Theme::appQLineEditStyleSheet() {
	return QString(
		"QLineEdit { font: %1px; }"
		"QLineEdit:disabled { background: gray; }"
	).arg(fontsize_small_);
}
QString CW3Theme::appQMenuStyleSheet() {
	return QString("QMenu { background: %2; color: #FFFFFFFF; border: 1px solid %1;}"
		"QMenu::item { padding-left: 25px; padding-right: 25px; padding-top: 4px; padding-bottom: 4px;}"
		"QMenu::item:selected { color: #FF22A7EF}")
		.arg(ColorGeneral::kBorder.name())
		.arg(ColorGeneral::kCaption.name());
}
QString CW3Theme::appAllInOneStyleSheet() {
	return appToolButtonStyleSheet() +
		appQSliderStyleSheet() +
		appQScrollBarStyleSheet() +
		appQRadioButtonStyleSheet() +
		appQCheckBoxStyleSheet() +
		appQGroupBoxStyleSheet() +
		appQComboBoxStyleSheet() +
		appQLabelStyleSheet() +
		appQLineEditStyleSheet();
}
QString CW3Theme::BlankLineStyleSheet() {
	return QString("#whiteline { background-color: %1; color: #cbccd0; }"
				   "#blackline { background-color: %1; color: gray; margin: 4; }")
		.arg(ColorGeneral::kBackground.name());
}
QString CW3Theme::titlebarStyleSheet() {
	return QString(
		"QLabel {background: %3;}"
		"QWidget#TitleBar {background: %3; border: 0px;}"
		"QGraphicsView { border: 0px; }"
		"QToolButton#close {width: 40px; height: %1px; image: url(:/image/titlebar/btn_windowclose.png); background: %3; border: 0px; }"
		"QToolButton#close:hover { image: url(:/image/titlebar/btn_windowclose_hover.png); }"
		"QToolButton#close:pressed { image: url(:/image/titlebar/btn_windowclose_pressed.png); }"
		"QToolButton#minimize {width: 22px; height: %1px; image: url(:/image/titlebar/btn_windowminimize.png); background: %3; border: 0px;}"
		"QToolButton#minimize:hover { image: url(:/image/titlebar/btn_windowminimize_hover.png); }"
		"QToolButton#minimize:pressed { image: url(:/image/titlebar/btn_windowminimize_pressed.png); }"

		"QToolButton#restore_min {width: 22px; height: %1px; image: url(:/image/titlebar/btn_windowrestore_min.png); background: %3; border: 0px;}"
		"QToolButton#restore_min:hover { image: url(:/image/titlebar/btn_windowrestore_min_hover.png); }"
		"QToolButton#restore_min:pressed { image: url(:/image/titlebar/btn_windowrestore_min_pressed.png); }"

		"QToolButton#restore_max {width: 22px; height: %1px; image: url(:/image/titlebar/btn_windowrestore_max.png); background: %3; border: 0px;}"
		"QToolButton#restore_max:hover { image: url(:/image/titlebar/btn_windowrestore_max_hover.png); }"
		"QToolButton#restore_max:pressed { image: url(:/image/titlebar/btn_windowrestore_max_pressed.png); }"

		"QMenuBar { font: %2px; height: %1px; background: %3; }"
		"QMenuBar::item { margin-top: 3px; padding-left: 10px; padding-right: 10px; padding-top: 0px; padding-bottom: 0px; border: 0px; }"
		"QMenuBar::item:selected { background: %3;}"
		"QMenu { font: %2px; }"
		)
		.arg(m_sizeH_titlebar)
		.arg(fontsize_regular_)
		.arg(ColorTitleBar::kTitleBar.name());
}
QString CW3Theme::tabWestStyleSheet() {
	return QString(
		"QWidget#TabBar {background: %5; padding: 0px; margin: 0px;}"
		"QTabWidget::pane {border-top: 1px solid %5; border-bottom: 1px solid %4; border-right: 1px solid %4; background: %6; padding: 0px; margin: 0px;}"
		"QTabBar::tab:first { border-top: 1px solid %4; }"
		"QTabBar::tab:selected {"
		"	color: white; background: %6;" 
		"	border-right: 1px solid %6;"
		"}"
		"QTabBar::tab:!selected {"
		"	color: #FFA5A5A5; background: %5;"
		"	border-right: 1px solid %4;"
		"}"
		"QTabBar::tab {"
		"	height: %1px; width: %2px;"
		"	border-bottom: 1px solid %4; border-left: 1px solid %4;"
		"	font: bold %3px;"
		"}"
		).arg(m_size_tabWest.height())
		.arg(m_size_tabWest.width())
		.arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name())
		.arg(ColorTabWest::kPane.name())
		.arg(ColorTabWest::kSelectedPane.name());
}
QString CW3Theme::toolbarStyleSheet() {
	return QString(
		"QLabel#EmptySpace { background-color: %1; }")
		.arg(ColorGeneral::kCaption.name());
}
QString CW3Theme::toolBoxStyleSheet() {
	return QString(
		"QFrame#ToolBox { border: 1px solid %2; }"
		"QLabel#Caption {"
		"	font: %1px; "
		"	background-color: %3;"
		"}"
		"QLabel[alignment=\"1\"]#Caption { padding-left: 10px; padding-top: 2px; padding-bottom: 2px; }"
		)
		.arg(fontsize_regular_).arg(ColorGeneral::kBorder.name())
		.arg(ColorGeneral::kCaption.name());
}
QString CW3Theme::toolIconButtonStyleSheet() {
	return QString(
		"QToolButton{ font: %1px; width: %2px; height: %3px; background-color: %5; background-position: top center; background-repeat: no-repeat; }"
		"QToolButton:disabled{background-color: %12;}"
		"QToolTip{ border: 2px; font: %4px; }"
		"QToolButton:hover {background-color: %6;}"
		"QToolButton:pressed {background-color: %7;}"
		"QToolButton:checked {background-color: %8;}"
		"QToolButton:unchecked {background-color: %9;}"
		"QToolButton#Reset{ background-image: url(:/image/buttons/btn_reset.png); }"
		"QToolButton#Reset:disabled{ background-image: url(:/image/buttons/btn_reset_disabled.png); }"
		"QToolButton#Pan{ background-image: url(:/image/buttons/btn_pan.png); }"
		"QToolButton#Pan:disabled{ background-image: url(:/image/buttons/btn_pan_disabled.png); }"
		"QToolButton#Zoom{ background-image: url(:/image/buttons/btn_zoom.png); }"
		"QToolButton#Zoom:disabled{ background-image: url(:/image/buttons/btn_zoom_disabled.png); }"
		"QToolButton#Fit{ background-image: url(:/image/buttons/btn_fit.png); }"
		"QToolButton#Fit:disabled{ background-image: url(:/image/buttons/btn_fit_disabled.png); }"
		"QToolButton#Light{ background-image: url(:/image/buttons/btn_light.png); }"
		"QToolButton#Light:disabled{ background-image: url(:/image/buttons/btn_light_disabled.png); }"
		"QToolButton#Invert{ background-image: url(:/image/buttons/btn_invert.png); }"
		"QToolButton#Invert:disabled{ background-image: url(:/image/buttons/btn_invert_disabled.png); }"
		"QToolButton#Hide{ background-image: url(:/image/buttons/btn_hide.png); }"
		"QToolButton#Hide:disabled{ background-image: url(:/image/buttons/btn_hide_disabled.png); }"
		"QToolButton#HideUI{ background-image: url(:/image/buttons/btn_hideui.png); }"
		"QToolButton#HideUI:disabled{ background-image: url(:/image/buttons/btn_hideui_disabled.png); }"
		"QToolButton#Grid{ background-image: url(:/image/buttons/btn_grid_off.png); }"
		"QToolButton#Grid:disabled{ background-image: url(:/image/buttons/btn_grid_off_disabled.png); }"
		"QToolButton#MeasureList{ background-image: url(:/image/buttons/measurelist.png); }"
		"QToolButton#MeasureList:disabled{ background-image: url(:/image/buttons/measurelist_disabled.png); }"
		"QToolButton#Ruler{ background-image: url(:/image/buttons/btn_ruler.png); }"
		"QToolButton#Ruler:disabled{ background-image: url(:/image/buttons/btn_ruler_disabled.png); }"
		"QToolButton#Tape{ background-image: url(:/image/buttons/btn_tape_line.png); }"
		"QToolButton#Tape:disabled{ background-image: url(:/image/buttons/tape_disabled.png); }"
		"QToolButton#Angle{ background-image: url(:/image/buttons/btn_angle.png); }"
		"QToolButton#Angle:disabled{ background-image: url(:/image/buttons/btn_angle_disabled.png); }"
		"QToolButton#Analysis{ background-image: url(:/image/buttons/btn_profile.png); }"
		"QToolButton#Analysis:disabled{ background-image: url(:/image/buttons/btn_profile_disabled.png); }"
		"QToolButton#Area{ background-image: url(:/image/buttons/btn_area.png); }"
		"QToolButton#Area:disabled{ background-image: url(:/image/buttons/btn_area_disabled.png); }"
		"QToolButton#ROI{ background-image: url(:/image/buttons/btn_roi.png); }"
		"QToolButton#ROI:disabled{ background-image: url(:/image/buttons/btn_roi_disabled.png); }"
		"QToolButton#Draw{ background-image: url(:/image/buttons/btn_draw_freedraw.png); }"
		"QToolButton#Draw:disabled{ background-image: url(:/image/buttons/btn_draw_freedraw_disabled.png); }"
		"QToolButton#Note{ background-image: url(:/image/buttons/btn_note.png); }"
		"QToolButton#Note:disabled{ background-image: url(:/image/buttons/btn_note_disabled.png); }"
		"QToolButton#HideMeasure{ background-image: url(:/image/buttons/btn_hide_measure.png); }"
		"QToolButton#HideMeasure:disabled{ background-image: url(:/image/buttons/btn_hide_measure_disabled.png); }"
		"QToolButton#Delete{ background-image: url(:/image/buttons/btn_delete.png); }"
		"QToolButton#Delete:disabled{ background-image: url(:/image/buttons/btn_delete_disabled.png); }"
		"QToolButton#ImplAngle{ background-image: url(:/image/buttons/btn_implantangle_v3.png); }"
		"QToolButton#ImplAngle:disabled{ background-image: url(:/image/buttons/btn_implantangle_v3_disabled.png); }"
		"QToolButton#Save{ background-image: url(:/image/buttons/btn_save.png); }"
		"QToolButton#Save:disabled{ background-image: url(:/image/buttons/btn_save_disabled.png); }"
		"QToolButton#Capture{ background-image: url(:/image/buttons/btn_capture.png); }"
		"QToolButton#Capture:disabled{ background-image: url(:/image/buttons/btn_capture_disabled.png); }"
		"QToolButton#Print{ background-image: url(:/image/buttons/btn_print.png); }"
		"QToolButton#Print:disabled{ background-image: url(:/image/buttons/btn_print_disabled.png); }"
		"QToolButton#CDExport{ background-image: url(:/image/buttons/btn_cdexport.png); }"
		"QToolButton#CDExport:disabled{ background-image: url(:/image/buttons/btn_cdexport_disabled.png); }"
		"QToolButton#PACS{ background-image: url(:/image/buttons/btn_pacs.png); }"
		"QToolButton#PACS:disabled{ background-image: url(:/image/buttons/btn_pacs_disabled.png); }"
		"QToolButton#Menu { width: %2px; height: %13px; border-top: 0px; margin: 0px; }"
		"QToolButton::menu-button { width: %2px; height: %13px; border: 0px; margin: 0px; }"
		"QToolButton::menu-arrow { image: url(:/image/buttons/btn_more.png); }"

		"QToolButton#3DCutToolSelect { width: %11px; height: %10px; }"
		"QToolButton::menu-button#3DCutToolSelect { width: 16px; height: %10px; border: 0px; margin: 0px; }"
		"QToolButton::menu-arrow#3DCutToolSelect { image: url(:/image/buttons/btn_more_stlexport.png); }"
		"QToolButton#AutoArchMenu { width: %11px; height: %10px; }"
		"QToolButton::menu-button#AutoArchMenu { width: 16px; height: %10px; border: 0px; margin: 0px; }"
		"QToolButton::menu-arrow#AutoArchMenu { image: url(:/image/buttons/btn_more_stlexport.png); }"
		"QMenu { font: %4px; }"
	)
		.arg(fontsize_small_) // 1
		.arg(size_tool_icon_.width()) // 2
		.arg(size_tool_icon_.height()) // 3
		.arg(fontsize_regular_) // 4
		.arg(ColorToolButton::kNormal.name()) // 5
		.arg(ColorToolButton::kHover.name()) // 6
		.arg(ColorToolButton::kPressed.name()) // 7
		.arg(ColorToolButton::kChecked.name()) // 8
		.arg(ColorToolButton::kUnchecked.name()) // 9
		.arg(m_size_tool_task.height()) // 10
		.arg(m_size_tool_menu.width()) // 11
		.arg(ColorToolButton::kDiabled.name()) // 12 
		.arg(tool_icon_sub_menu_height_); // 13
}
QString CW3Theme::ChangeToolButtonImageStyleSheet(const QString& path,
												  const QString& disabled_path) {
	return QString(
		"QToolButton{ font: %1px; width: %2px; height: %3px; background-color: %7;"
		"background-image: url(%5); background-position: top center; background-repeat: no-repeat; }"
		"QToolButton:disabled{ background-image: url(%6); }"
		"QToolTip{ border: 2px; font: %4px; }"
		"QToolButton:hover {background-color: %8;}"
		"QToolButton:pressed {background-color: %9;}"
		"QToolButton:checked {background-color: %10;}"
		"QToolButton:unchecked {background-color: %11;}"
	).arg(fontsize_small_)
	.arg(size_tool_icon_.width())
	.arg(size_tool_icon_.height())
	.arg(fontsize_regular_)
	.arg(path)
	.arg(disabled_path)
	.arg(ColorToolButton::kNormal.name())
	.arg(ColorToolButton::kHover.name())
	.arg(ColorToolButton::kPressed.name())
	.arg(ColorToolButton::kChecked.name())
	.arg(ColorToolButton::kUnchecked.name());
}
QString CW3Theme::toolTaskStyleSheet() {
	//padding Label 10px. width is (165 + 10)px.
	return QString(
		"QToolButton { width: %1px; height: %2px; background-position: left; background-repeat: no-repeat;}"
		// MPR
		"QToolButton#3Dzoom { background-image: url(:/image/buttons/btn_3dzoom.png); }"
		"QToolButton#Oblique { background-image: url(:/image/buttons/btn_oblique.png); }"
		"QToolButton#3DCut { width: %3px; height: %2px; background-image: url(:/image/buttons/btn_vrcut.png); }"
		"QToolButton#STLExport { background-image: url(:/image/buttons/btn_stlexport.png); }"
		"QToolButton#DrawArch { background-image: url(:/image/buttons/btn_arch.png); }"
		// Pano
		"QToolButton#AutoArch { width: %3px; height: %2px; background-image: url(:/image/buttons/btn_arch.png); }"
		"QToolButton#ManualArch { background-image: url(:/image/buttons/btn_arch_manual.png); }"
		// 3D Ceph
		"QToolButton#CoordSystem { background-image: url(:/image/buttons/btn_coordinate system.png); }"
		"QToolButton#Tracing { background-image: url(:/image/buttons/btn_createtracing.png); }"
		"QToolButton#SelectMethod { background-image: url(:/image/buttons/btn_selectmethod.png); }"
		"QToolButton#3DSurgery { background-image: url(:/image/buttons/btn_3dsurgery.png); }"
		"QToolButton#ShowSkin { background-image: url(:/image/buttons/btn_photomapping.png); }"
		// Face Simulation
		"QToolButton#GenerateFace { background-image: url(:/image/buttons/btn_extractsurface.png); }"
		"QToolButton#LoadPhoto { background-image: url(:/image/buttons/btn_loadimage.png); }"
		"QToolButton#PhotoMapping { background-image: url(:/image/buttons/btn_photomapping.png); }"
		"QToolButton#ClearMappingPoint { background-image: url(:/image/buttons/btn_photomapping.png); }"
		"QToolButton#BeforeAfter { background-image: url(:/image/buttons/btn_beforeandafter.png); }"
		// Super Imposition
		"QToolButton#LoadNewVolume { background-image: url(:/image/buttons/btn_load_new_volume.png); }"
		"QToolButton#AutoRegistration { background-image: url(:/image/buttons/btn_load_new_volume.png); }"
		)
		.arg(m_size_tool_task.width())
		.arg(m_size_tool_task.height())
		.arg(m_size_tool_task.width() - m_size_tool_menu.width()-5);
}

QString CW3Theme::toolOTFStyleSheet() {
	return QString(
		"QToolButton:checked {background-color: %3;}"
		"QToolButton:unchecked {background-color: %4;}"
		"QToolButton { padding: 1px; width: %1px; height: %2px; border: 0px solid %4;}"
		"QToolButton#01_teeth {image: url(./tfpresets/01_teeth.bmp); }"
		"QToolButton#02_gray {image: url(./tfpresets/02_gray.bmp); }"
		"QToolButton#03_soft_tissue1 {image: url(./tfpresets/03_soft_tissue1.bmp); }"
		"QToolButton#04_soft_tissue2 {image: url(./tfpresets/04_soft_tissue2.bmp); }"
		"QToolButton#05_bone {image: url(./tfpresets/05_bone.bmp); }"
		"QToolButton#06_mip {image: url(./tfpresets/06_mip.bmp); }"
		"QToolButton#07_xray {image: url(./tfpresets/07_xray.bmp); }"
		"QToolButton#0 {image: url(./tfpresets/00_custom.bmp); }"
		"QToolButton#08_custom2 {image: url(./tfpresets/08_custom2.bmp); }"
		"QToolButton#09_custom3 {image: url(./tfpresets/09_custom3.bmp); }"
		"QToolButton#10_custom4 {image: url(./tfpresets/10_custom4.bmp); }"
		)
		.arg(m_size_tool_otf.width())
		.arg(m_size_tool_otf.height())
		.arg(ColorToolOTFButton::kCheckBorder.name())
		.arg(ColorToolOTFButton::kUncheckBorder.name());
}

QString CW3Theme::toolVisibilityStyleSheet() {
	return QString("QLineEdit { width: %1px; height %2px; }")
		.arg(m_size_button.width())
		.arg(m_size_button.height());
}

QString CW3Theme::toolTMJModeSelectionStyleSheet() {
	return QString(
		"QToolButton#TMJModeSelection { height: 26px;"
		" font: %1px; background-color: #FF181A26;"
		" }"
		"QToolButton#TMJModeSelection:disabled { background-color: %2; }"
		"QToolButton#TMJModeSelection:hover { background-color: #FF5A6282;}"
		"QToolButton#TMJModeSelection:checked { background-color: #FF414A6B;}"
	).arg(fontsize_regular_ + 1).arg(ColorToolButton::kDiabled.name());
}

QString CW3Theme::toolTMJRectStylesheet() {
	return QString(
		"QToolButton#TMJRectButton { width: %1px; height: %2px; }"
		"QToolButton#TMJRectButton:disabled { background-color: %3; }"
		"QLabel#TMJDirectionText { font: 13px; }"
	).arg(m_size_button.width() - 10)
	.arg(m_size_button.height())
	.arg(ColorToolButton::kDiabled.name());
}

QString CW3Theme::toolGeneralTextStyleSheet() {
	return QString(
		"#GeneralText { font: 12px; }"
	);
}

QString CW3Theme::toolOrientationStyleSheet() {
	return QString(
		"QToolButton#AutoOrientation { width: %1px; height: %2px; }"
		"QToolButton#ApplyOrientation { width: %1px; height: %2px; }"
		"QToolButton#ResetOrientation { width: %1px; height: %2px; }"
		)
		.arg(m_size_button.width())
		.arg(m_size_button.height());
}

QString CW3Theme::toolEndoExplorerStyleSheet() {
	QString strNormalStyle = 
		QString("border: 1px solid %1; padding: 0px; width: 24px; height: 19px; background-color: #FF7E7E7E;}").arg(ColorGeneral::kBorder.name());
	QString strHoverStyle("{background-color: #FF5A6282;}");
	QString strPressedStyle("{background-color: #FF1C1E28;}");

	return QString(
		"QToolButton#Start {"
		"background: url(:/image/buttons/icon_endo_start.png) center no-repeat;" + strNormalStyle +
		"QToolButton#Start:hover " + strHoverStyle +
		"QToolButton#Start:pressed " + strPressedStyle +

		"QToolButton#Stop {"
		"background: url(:/image/buttons/icon_endo_stop.png) center no-repeat;" + strNormalStyle +
		"QToolButton#Stop:hover " + strHoverStyle +
		"QToolButton#Stop:pressed " + strPressedStyle +

		"QToolButton#Play {"
		"background: url(:/image/buttons/icon_endo_play.png) center no-repeat;" + strNormalStyle +
		"QToolButton#Play:hover " + strHoverStyle +
		"QToolButton#Play:pressed " + strPressedStyle +

		"QToolButton#End {"
		"background: url(:/image/buttons/icon_endo_end.png) center no-repeat;" + strNormalStyle +
		"QToolButton#End:hover " + strHoverStyle +
		"QToolButton#End:pressed " + strPressedStyle +

		"QToolButton#Prev {"
		"background: url(:/image/buttons/icon_endo_prev.png) center no-repeat;" + strNormalStyle +
		"QToolButton#Prev:hover " + strHoverStyle +
		"QToolButton#Prev:pressed " + strPressedStyle +

		"QToolButton#Next {"
		"background: url(:/image/buttons/icon_endo_next.png) center no-repeat;" + strNormalStyle +
		"QToolButton#Next:hover " + strHoverStyle +
		"QToolButton#Next:pressed " + strPressedStyle
	);
}

QString CW3Theme::toolEndoPathStyleSheet() {
	QString strNormalStyle =
		QString("border: 1px solid %1; padding: 0px; width: 24px; height: 19px; background-color: #FF7E7E7E;}").arg(ColorGeneral::kBorder.name());
	QString strHoverStyle("{background-color: #FF5A6282;}");
	QString strPressedStyle("{background-color: #FF1C1E28;}");
	QString strDisabledStyle("{color: gray; background-color: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF424242, stop:1 #FF343434);}");

	return QString(
		"QToolButton#DeletePath {"
		+ strNormalStyle +
		"QToolButton#DeletePath:hover " + strHoverStyle +
		"QToolButton#DeletePath:pressed " + strPressedStyle + 
		"QToolButton#DeletePath:disabled " + strDisabledStyle
	);
}

QString CW3Theme::toolTMJStyleSheet() {
	return QString(
		"QDoubleSpinBox { width: %1px; height: %2px; border: 0px; }"
	)
	.arg(m_size_tool_tmj.width())
	.arg(m_size_tool_tmj.height());
}

QString CW3Theme::toolNerveDrawModifyStylesheet(bool checked) {
#if 0
	return QString(
		"QToolButton {"
		"font: %1px;"
		"border: 1px solid %9; padding: 0px;"
		"padding-left: 10px;"
		"width: %2px; height: %3px;"
		"background-image: url(:/image/buttons/%10); background-position: left; background-repeat: no-repeat;"
		"background-color: %4;"
		"}"
		"QToolButton:hover {background-color: %5;}"
		"QToolButton:pressed {background-color: %6;}"
		"QToolButton:checked {background-color: %7;}"
		"QToolButton:unchecked {background-color: %8;}"
	)
		.arg(fontsize_regular_)
		.arg(m_size_button.width() + 25)
		.arg(m_size_button.height() + 4)
		.arg(ColorToolButton::kNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name())
		.arg(ColorGeneral::kBorder.name())
		.arg(checked ? QString("nerve_on.png") : QString("nerve_off.png"));
#else
	return QString(
		"QToolButton {"
		"font: bold %1px;"
		"border: 1px solid %9; padding: 0px;"
		"padding-left: 10px;"
		"height: %2px;"
		"background-image: url(:/image/buttons/%10); background-position: left; background-repeat: no-repeat;"
		"background-color: %3;"
		"}"
		"QToolButton:hover {background-color: %4;}"
		"QToolButton:pressed {background-color: %5;}"
		"QToolButton:checked {background-color: %6;}"
		"QToolButton:unchecked {background-color: %7;}"
	)
		.arg(fontsize_regular_ + 3)
		.arg(m_size_button.height() * 2)
		.arg(ColorToolButton::kNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name())
		.arg(ColorGeneral::kBorder.name())
		.arg(checked ? QString("nerve_on.png") : QString("nerve_off.png"));
#endif
}

QString CW3Theme::appDialogStyleSheet() {
	return QString(
		"QFrame#content { background: %2; border: 1px solid %1; }"
		"*#titlebar { border: 0px; background: %1; }"
		"QLabel#titlebar { font: bold %8px; padding-left: 10px; background: %1; }"
		"QLabel { font: %7px; }"
		"#line { background-color: %2; color: #cbccd0; }"
		"QCheckBox { font: %7px; }"
		"QRadioButton { font: %7px; }"
		"QToolButton#titlebar { width: 30px; font: %8px; height: 25px;"
							  " image: url(:/image/dialog/close.png); background: %1; }"
		"QTableWidget { font: %9px; color: %3; background-color: %4; border: 1px solid %5; }"
		"QTableWidget::item { border-bottom: 1px solid %6; }"
		"QTableWidget::item:selected{ background-color: %6; }"
	).arg(ColorGeneral::kCaption.name())
		.arg(ColorGeneral::kBackground.name())
		.arg(ColorGeneral::kTableWidgetText.name())
		.arg(ColorGeneral::kTableWidgetBackground.name())
		.arg(ColorGeneral::kTableWidgetBorder.name())
		.arg(ColorGeneral::kTableWidgetSelectedItem.name())
		.arg(fontsize_regular_)
		.arg(fontsize_regular_ + 1)
		.arg(fontsize_regular_ - 1);
}

QString CW3Theme::LightDialogStyleSheet() {
	return QString(
		"QFrame#content { background: %1; border: 1px solid %8; }"
		"*#titlebar { border: 0px; background: %16; }"
		"QLabel#titlebar { font: bold %17px; color: %1; padding-left: 10px; background: %16; }"
		"QToolButton#titlebar { width: 30px; font: %17px; height: 25px;"
		"	image: url(:/image/dialog/close.png); background: %16; }"
		"QLabel { font: %17px; color: %2; }"
		"QLabel#UnselectedTab { height: %15px; font: 1px; background-color: %5; }"
		"QLabel#SelectedTab { height: %15px; font: 1px; background-color: %7; }"
		"QCheckBox { height: %13px; font: %17px; color: %2; }"
		"QRadioButton { height: %13px; font: %17px; color: %2; }"
		"QLineEdit {"
		"	height: %13px; font: %17px; color: %2;"
		"	border: 1px solid %9; border-radius: 0px;"
		"}"
		"QLineEdit:disabled {"
		"	background-color: %4;"
		"}"
		"QToolButton {"
		"	width: %12px; height: %13px; font: %17px; color: %2;"
		"	border: 1px solid %9; border-radius: 2px;"
		"	background-color: %1;"
		"}"
		"QToolButton:hover { background-color: %5; }"
		"QToolButton:pressed { background-color: %4; }"
		"QToolButton#NoBackground {"
		"	height: %13px; font: %17px; color: %3;"
		"	border: 0px;"
		"	background-color: %1;"
		"	text-decoration: underline;"
		"}"
		"QToolButton#NoBackground:hover { color: %10; }"
		"QToolButton#NoBackground:pressed { color: %11; }"
		"QToolButton#Tab {"
		"	height: %14px; font: bold %17px; color: %4;"
		"	border: 0px;"
		"	background-color: %1;"
		"}"
		"QToolButton#Tab:unchecked { color: %4; }"
		"QToolButton#Tab:checked { color: %6; }"
	).arg(ColorLoginDialog::kBackground.name()) // 1
		.arg(ColorLoginDialog::kComponentText.name()) // 2
		.arg(ColorLoginDialog::kForgotUsernamePasswordText.name()) // 3
		.arg(ColorLoginDialog::kUnselectedTabText.name()) // 4
		.arg(ColorLoginDialog::kUnselectedTabIndicator.name()) // 5
		.arg(ColorLoginDialog::kSelectedTabText.name()) // 6
		.arg(ColorLoginDialog::kSelectedTabIndicator.name()) // 7
		.arg(ColorLoginDialog::kFrameBorder.name()) // 8
		.arg(ColorLoginDialog::kComponentBorder.name()) // 9
		.arg(ColorLoginDialog::kForgotUsernamePasswordTextHover.name()) // 10
		.arg(ColorLoginDialog::kForgotUsernamePasswordTextPressed.name()) // 11
		.arg(130) // 12
		.arg(25) // 13
		.arg(30) // 14
		.arg(7) // 15
		.arg(ColorGeneral::kCaption.name()) // 16
		.arg(fontsize_regular_); // 17
}

QString CW3Theme::messageBoxStyleSheet() {
	return QString(
		"QLabel#Message {font: bold %1px; }"
		"QLabel#DetailedText { font: %2px; }"
	).arg(fontsize_regular_ + 1)
		.arg(fontsize_regular_ - 1);
}

QString CW3Theme::DicomInfoBoxStyleSheet() {
	return QString(
		"QFrame#Frame { border: 1px solid %2; }"
		"QLabel {"
		"	font: %1px;"
		"	background-color: transparent;"
		"}"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::LineSeparatorStyleSheet() {
	return QString(
		"QFrame#Line { color: %1; background-color: transparent; }"
	).arg(ColorGlobalPreferencesDialog::kLine.name());
}

QString CW3Theme::ColoredToolButtonStyleSheet(QColor color) {
	return QString(
		"QToolButton { font: %1px; border: 1px solid %3; padding: 0px; background-color: %2; }"
	).arg(fontsize_regular_)
		.arg(color.name())
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::GlobalPreferencesDialogTabStyleSheet() {
	return QString(
		"QTabWidget::pane {"
		"	border: 1px solid %4;"
		"	background: %6;"
		"	padding: 0px;"
		"	margin: 0px;"
		"	top: -1px;"
		"}"
		"QTabBar::tab:selected {"
		"	background: %6;"
		"	border: 1px solid %4;"
		"	border-bottom: 1px solid %6;"
		"}"
		"QTabBar::tab:!selected {"
		"	background: %5;"
		"	border: 1px solid %4;"
		"}"
		"QTabBar::tab {"
		"	width: %1px; height: %2px;"
		"	color: white;"
		"	font: %3px;"
		"	margin-right: 4px;"
		"}"
	).arg(size_global_preferences_tab_.width())
		.arg(size_global_preferences_tab_.height())
		.arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name())
		.arg(ColorGlobalPreferencesDialog::kUnselectedTab.name())
		.arg(ColorGlobalPreferencesDialog::kSelectedTab.name());
}

QString CW3Theme::GlobalPreferencesDialogToolButtonStyleSheet() {
	return QString(
		"QToolButton { font: %1px; border: 1px solid %7; padding: 0px; background-color: %2; }"
		"QToolButton:hover { background-color: %3; }"
		"QToolButton:pressed { background-color: %4; }"
		"QToolButton:checked { background-color: %5; }"
		"QToolButton:unchecked { background-color: %6; }"
	).arg(fontsize_regular_)
		.arg(ColorGlobalPreferencesDialog::kButtonNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name())
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::GlobalPreferencesDialogComboBoxStyleSheet() {
	return QString(
		"QComboBox { font: %1px; border: 1px solid %2; color: %2; background: #FFFFFFFF; }"
		"QComboBox::drop-down { border-image: url(:/image/spinbox/drop_down.png); width: 14px; }"
		//"QComboBox QAbstractItemView { min-width: 200px; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::GlobalPreferencesDialogSpinBoxStyleSheet() {
	return QString(
		"QSpinBox { font: %1px; color: %2; background: #FFFFFFFF; }"
		"QDoubleSpinBox { font: %1px; color: %2; background: #FFFFFFFF; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::GlobalPreferencesDialogLineEditStyleSheet() {
	return FileTabLineEditStyleSheet();
}

QString CW3Theme::GlobalPreferencesDialogListViewStyleSheet() {
	return QString(
		"QListView { font: %1px; color: %2; background: #FFFFFFFF; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileSystemTreeWidgetStyleSheet() {
	return QString(
		"QTreeView { font: %1px; }"
		"QTreeView::branch:closed:has-children:!has-siblings,"
		"QTreeView::branch:closed:has-children:has-siblings {"
		"	border-image: none;"
		"	image: url(:/image/scrollbar/hscrollright.png);"
		"}"
		"QTreeView::branch:open:has-children:!has-siblings,"
		"QTreeView::branch:open:has-children:has-siblings {"
		"	border-image: none;"
		"	image: url(:/image/scrollbar/vscrolldown.png);"
		"}"
	).arg(fontsize_regular_);
}

QString CW3Theme::FileTabToolButtonStyleSheet() {
	return QString(
		"QToolButton { font: %1px; border: 1px solid %7; padding: 0px; background-color: %2; }"
		"QToolButton:hover { background-color: %3; }"
		"QToolButton:pressed { background-color: %4; }"
		"QToolButton:checked { background-color: %5; }"
		"QToolButton:unchecked { background-color: %6; }"
	).arg(fontsize_regular_)
		.arg(ColorToolButton::kNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name())
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabRadioButtonStyleSheet() {
	return appQRadioButtonStyleSheet() + 
		QString(
		"QRadioButton { font: %1px; }"
	).arg(fontsize_regular_);
}

QString CW3Theme::FileTabLabelStyleSheet() {
	return QString(
		"QLabel { font: %1px; color: #FFFFFFFF; }"
	).arg(fontsize_regular_);
}

QString CW3Theme::FileTabLineEditStyleSheet() {
	return QString(
		"QLineEdit { font: %1px; color: %2; border: 1px solid %2; border-radius: 0px; background: #FFFFFFFF; }"
		"QLineEdit:disabled { background: #FF7C8091; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabExplorerTreeViewStyleSheet() {
	return QString(
		"QTreeView { font: %1px; color: %2; background: #FFFFFFFF; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabStudyTreeHeaderStyleSheet() {
	return QString(
		"QHeaderView { font: %1px; color: #FFFFFFFF; background: %2; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kCaption.name());
}

QString CW3Theme::FileTabStudyTreeWidgetStyleSheet() {
	return QString(
		"QTreeWidget { font: %1px; color: %2; border: 1px solid %2; background: #FFFFFFFF; }"
		"QTreeView::branch:closed:has-children { border-image: none; image: url(:/image/dicomloader/open.png); }"
		"QTreeView::branch:open:has-children { border-image: none; image: url(:/image/dicomloader/close.png); }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabDicomSummaryTableWidgetStyleSheet() {
	return QString(
		"QTableWidget { font: %1px; color: %2; border: 1px solid %2; background: #FFFFFFFF; alternate-background-color: rgb(238, 238, 238);}"
		"QTableWidget::item { border: 1px; padding-left: 10px; padding-top: 0px; padding-bottom: 0px; }"
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabSpinBoxStyleSheet() {
	return QString(
		"QSpinBox { font: %1px; color: %2; background: #FFFFFFFF; } "
		"QDoubleSpinBox { font: %1px; color: %2; background: #FFFFFFFF; } "
	).arg(fontsize_regular_)
		.arg(ColorGeneral::kBorder.name());
}

QString CW3Theme::FileTabGroupBoxStyleSheet() {
	return toolBoxStyleSheet() + 
		QString(
		"QFrame#ToolBox { background-color: %1; }"
	).arg(ColorGeneral::kBackground.name());
}

QString CW3Theme::cephIndicatorListStyleSheet(bool bAnalysisMode) {
	if (!bAnalysisMode) {
		return QString(
			"QToolButton#Caption { border: 0px; width: 380px; height: 27px; font: bold %1px;}"
			"QToolButton#Switch { border: 0px; width: 68px; height: 27px; font: bold %1px;}"
			"QToolButton#Content {height: 22px; font: bold %2px;}"
		).arg(fontsize_regular_)
			.arg(fontsize_small_);
	} else {
		return QString(
			"QToolButton#Caption {width: 165px; height: 27px; font: bold %1px;}"
			"QToolButton#Switch { border: 0px; width: 68px; height: 27px; font: bold %1px;}"
			"QToolButton#Content {height: 22px; font: bold %2px;}"
			"QLabel {font: bold %3px;}"
		).arg(fontsize_regular_)
			.arg(fontsize_small_)
			.arg(fontsize_small_ - 2);
	}
}

QString CW3Theme::cephIndicatorBarStyleSheet() {
	return QString(
		"QWidget#CephIndicatorBar {border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
		"QToolButton {border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
		"QTabWidget::pane {border: 0px; }"
		"QTabBar::tab:last { border-right: 1px solid #FF2F3243; }"
		"QTabBar::tab:selected {"
		"	color: white; background: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF868686, stop:1 #FF2F3243);"
		"}"
		"QTabBar::tab:!selected {"
		"	color: #FFA5A5A5; background: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #FF2F3243, stop:1 #FF505050);"
		"	border-bottom: 1px solid %1;"
		"}"
		"QTabBar::tab {"
		"	height: 34px; width: 111px;  background: #FF2F3243; "
		"	border: 0px;"
		"	border-top: 1px solid %1; border-right: 1px solid %1;"
		"	font: bold;"
		"}"
		"QComboBox#SelectAnalysis {"
		"font: bold %2px;"
		"background-color: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF707070, stop:0.5 #FF646464 stop:1 #FF575757);"
		"border: 1px solid %1;"
		//"border-radius: 3px;"
		"padding: 2px 2px 2px 4px;"
		"color: white;"
		"}"
		"QComboBox#SelectAnalysis QAbstractItemView {"
		"border: 1px solid %1;  background: #FF2F3243; "
		"}"
		"QComboBox#RaceComboBox {"
		"font: bold %3px;  background: #FF2F3243; "
		"border: 1px solid #FF8C8C8C;"
		"border-radius: 3px;"
		"padding: 1px 18px 1px 3px;"
		"color: white;"
		"}"
		"QComboBox#RaceComboBox QAbstractItemView {"
		"border: 1px solid #FF8C8C8C;  background: #FF2F3243; "
		"border-radius: 3px;"
		"}"
		"QComboBox::drop-down {"
		"background-image: url(:/image/combobox/arrow.png);"
		"background-position: center;"
		"background-repeat: no-repeat;"
		"border: 0px;"
		"padding-right: 3px;"
		"}"
		"QComboBox:on {"
		"border: 1px solid %1; "
		"}"
		"QComboBox:hover {"
		"border: 1px solid %1;"
		"background-color: #FF5A6282;"
		"}"
	).arg(ColorGeneral::kBorder.name())
		.arg(fontsize_regular_ + 5)
		.arg(fontsize_small_);
}

QString CW3Theme::cephSurgeryBarStyleSheet() {
	return QString("* { background: #FF2F3243; }"
		"QFrame#SurgeryBar { border: 0px; padding: 0px; margin: 0px;}"
		"QFrame#Content { border-bottom: 1px solid %1;}"
		"QLabel#Caption {"
		" font: bold %2px; "
		"background: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF3E3E3E, stop:0.1 #FF424242, stop:0.2 #FF4C4C4C stop:1 #FF383838);"
		"}"
		"QToolButton#Close {"
		"background: qlineargradient(spread : pad x1 : 0, y1 : 0, x2 : 0, y2 : 1, stop : 0 #FF3E3E3E, stop:0.1 #FF424242, stop:0.2 #FF4C4C4C stop : 1 #FF383838);"
		"border-left: 0px; width: 30px; height: 25px; image: url(:/image/dialog/close.png);"
		"}"
		+ appQDoubleSpinBoxStyleSheet()
	).arg(ColorGeneral::kBorder.name())
		.arg(fontsize_regular_);
}

QString CW3Theme::cephTracingBarStyleSheet() {
	return QString(
		"QWidget#TracingBar { border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
		"QFrame#TracingCommand { border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
		"QLabel {font: bold %2px; }"
		"QLabel#Caption {"
		"background: qlineargradient(spread:pad x1:0, y1:0, x2:0, y2:1, stop:0 #FF3E3E3E, stop:0.1 #FF424242, stop:0.2 #FF4C4C4C stop:1 #FF383838);"
		"}"
		"QLabel#GuideImg {"
		"background: black;"
		"}"
		"QToolButton { border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
		"QToolButton#Tracing { border: 0px; background-image: url(:/image/tracing/uncheck.png); background-position: left; background-repeat: no-repeat; }"
		"QToolButton:hover#Tracing {background-color: #FF22A7EF;}"
		"QScrollArea#listArea { border-bottom: 1px solid %1;  border: 0px; background: #FF2F3243; padding: 0px; margin: 0px;}"
	).arg(ColorGeneral::kBorder.name())
		.arg(fontsize_regular_);
}

QString CW3Theme::reportWindowStyleSheet() {
	return appQScrollBarStyleSheet() +
		QString("QFrame#CW3ReportWindow {"
			"background-color: #FFD0D0D0;"
			"border: 1px solid #FF939393;"
			"}"
			"QFrame#f_toolbar {"
			"background-color: #FFD0D0D0;"
			"border: 1px solid #FF939393;"
			"}"
			"QFrame#f_thumnail {"
			"background-color: #FFD0D0D0;"
			"}"
			"QFrame[frameShape=\"4\"]#CW3ReportWindow {color: #FF939393}"
			"QFrame[frameShape=\"5\"]#CW3ReportWindow {color: #FF939393}"

			"QToolButton#f_undo {"
			"width: 23px; height: 23px; background-color: #FFD0D0D0; border: 0px;"
			" background-image: url(:/image/report/undo.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_redo {"
			"width: 23px; height: 23px; background-color: #FFD0D0D0; border: 0px;"
			" background-image: url(:/image/report/redo.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_saveHTML {"
			"width: 23px; height: 23px; background-color: #FFD0D0D0; border: 0px;"
			" background-image: url(:/image/report/save.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_link {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/link.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_bold {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/bold.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_italic {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/italic.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_underline {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/underLine.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_strikeout {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/strikeOut.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_list_bullet {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/bulletList.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_list_ordered {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/orderedList.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_indent_dec {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/decreaseIndentation.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QToolButton#f_indent_inc {"
			"width: 21px; height: 21px; background-color: #FFE6E6E6; border: 1px solid #FF767676;"
			" background-image: url(:/image/report/increaseIndentation.png); background-position: top center; background-repeat: no-repeat;"
			"}"
			"QLabel#thumbCaption {"
			"color: #FF434343;"
			"background-color: #FFD0D0D0;"
			"font: bold %1px;"
			"}"
		).arg(fontsize_regular_);

}
QString CW3Theme::ViewMenuBarStyleSheet() {
	return QString("QFrame#ViewMenuBar { border: 0px; background: %3; padding: 0px; margin: 0px;}"
				   "QLabel#ViewMenuBarCaption {font: bold %1px;}"
				   "QLabel#ViewMenuBarLabel {font: %2px; }")
		.arg(fontsize_regular_)
		.arg(fontsize_small_)
		.arg(ColorGeneral::kCaption.name());

}
QString CW3Theme::ViewMenuBarButtonStyleSheet() {
	return QString(
		"QToolTip{ border: 2px; font: %2px; }"
		"QToolButton:hover {background-color: %4;}"
		"QToolButton:pressed {background-color: %5;}"
		"QToolButton:checked {background-color: %6;}"
		"QToolButton:unchecked {background-color: %7;}"
		"QToolButton { font: %1px; background-color: %3; }"
		).arg("11")
		.arg(fontsize_regular_)
		.arg(ColorToolButton::kNormal.name())
		.arg(ColorToolButton::kHover.name())
		.arg(ColorToolButton::kPressed.name())
		.arg(ColorToolButton::kChecked.name())
		.arg(ColorToolButton::kUnchecked.name());
}
QString CW3Theme::ViewMenuBarOnOffSwitchStyleSheet(const QColor& bg_color, const QString& img_path) {
	return QString(
		"QToolTip{ border: 2px; font: %1px; }"
		"QToolButton { border: 1px solid %2; background-color: %2; background-image: url(%3);"
		"background-position: top center; background-repeat: no-repeat; }"
	).arg(fontsize_regular_)
	.arg(bg_color.name())
	.arg(img_path);
}
QString CW3Theme::TableWidgetHeaderStyleSheet() {
	return QString("QHeaderView::section { background-color: #FF1C1E29; font-size: 13px; height: 24px; }");
}
QString CW3Theme::NerveToolStyleSheet() {
	return QString("QFrame#NerveToolRecordArea { background-color: #FFFFFFFF; }" 
				   "QFrame#NerveToolRecord {"
				   "background-color: #FFFFFFFF;"
				   "border-bottom: 1px solid #FFD9D9D9;"
				   "}"
				   "QFrame#NerveToolRecord:hover {"
				   "background-color: #4C5A6282;"
				   "border: 0px;"
				   "}"
				   "QLabel#NerveToolVisible {"
				   "background-image: url(:/image/nervetool/nervetool_visible.png);"
				   "background-position: center;"
				   "background-repeat: no-repeat;"
				   "}"
				   "QFrame#NerveToolField { font: %1px; background: %4; }"
				   "QLabel#NerveToolRecordID {"
				   "font: %2px;"
				   "color: %3"
					"}")
		.arg(fontsize_regular_)
		.arg(fontsize_small_)
		.arg(ColorGeneral::kBorder.name())
		.arg(ColorGeneral::kCaption.name());
}

QString CW3Theme::MeasureListDlgStyleSheet() {
	return QString("QFrame#MeasureRecord {"
				   "background-color: #FFFFFFFF;"
				   "border-bottom: 1px solid #FFD9D9D9;"
				   "}"
				   "QFrame#MeasureRecord:hover {"
				   "background-color: #4C5A6282;"
				   "border: 0px;"
				   "}"
				   "QFrame#MeasureListRecordArea { background-color: #FFFFFFFF; }"
				   "QFrame#MeasureListFieldArea { font: %1px; background-color: %2; }"
		).arg(fontsize_regular_)
		.arg(ColorGeneral::kCaption.name());
}

QString CW3Theme::AddImplantStyleSheet() {
	return QString(
		"QFrame#AddImplant { background-color: %1; }"
		"QLabel#ArchSelection { font: bold %2px; color: white; }"
		"QLabel#InfoLabel { font: %2px; color: white; }"
		"QRadioButton#InfoLabel { font: %2px; color: white; }"
		"#whiteline { background-color: %1; color: #cbccd0; }"
		"#blackline { background-color: %1; color: gray; margin: 4; }"
	).arg(ColorImplant::kPanelBackGround.name())
	.arg(fontsize_regular_);
}

QString CW3Theme::DefaultImplantButtonStyleSheet() {
	return QString(
		"QToolButton {"
		"width: %1px; height: %2px; color: white;"
		"border: %3px solid %5; border-radius: %4px;"
		"background-color: %6; font: %9px;"
		"}"
		"QToolButton:hover { background-color: %7; }"
		"QToolButton:pressed { background-color: %8; }"
	).arg(QString::number(common::ui_define::kImplantButtonWidth),
		  QString::number(common::ui_define::kImplantButtonHeight),
		  QString::number(kImplantBtnBorder), QString::number(kImplantBtnRadius)) // 1 2 3 4
		.arg(ColorImplant::kImplantBtnBorder.name()) // 5
		.arg(ColorImplant::kImplantNormal.name()) // 6
		.arg(ColorImplant::kImplantNormalHover.name()) // 7
		.arg(ColorImplant::kImplantSelected.name()) // 8
		.arg(fontsize_regular_ + 1);
}

QString CW3Theme::SelecedtImplantButtonStyleSheet() {
	return QString(
		"QToolButton {"
		"width: %1px; height: %2px; color: white;"
		"border: %3px solid %5; border-radius: %4px;"
		"background-color: %6; font: %9px;"
		"}"
		"QToolButton:hover { background-color: %7; }"
		"QToolButton:pressed { background-color: %8; }"
	).arg(QString::number(common::ui_define::kImplantButtonWidth),
		  QString::number(common::ui_define::kImplantButtonHeight),
		  QString::number(kImplantBtnBorder), QString::number(kImplantBtnRadius)) // 1 2 3 4
		.arg(ColorImplant::kImplantBtnBorder.name())
		.arg(ColorImplant::kImplantSelected.name())
		.arg(ColorImplant::kImplantSelectedHover.name())
		.arg(ColorImplant::kImplantPlaced.name())
		.arg(fontsize_regular_ + 1);
}

QString CW3Theme::PlacedImplantButtonStyleSheet() {
	return QString(
		"QToolButton {"
		"width: %1px; height: %2px; color: white;"
		"border: %3px solid %5; border-radius: %4px;"
		"background-color: %6; font: %9px;"
		"}"
		"QToolButton:hover { background-color: %7; }"
		"QToolButton:pressed { background-color: %8; }"
	).arg(QString::number(common::ui_define::kImplantButtonWidth),
		  QString::number(common::ui_define::kImplantButtonHeight),
		  QString::number(kImplantBtnBorder), QString::number(kImplantBtnRadius)) // 1 2 3 4
		.arg(ColorImplant::kImplantBtnBorder.name())
		.arg(ColorImplant::kImplantPlaced.name())
		.arg(ColorImplant::kImplantPlacedHover.name())
		.arg(ColorImplant::kImplantSelected.name())
		.arg(fontsize_regular_ + 1);
}
