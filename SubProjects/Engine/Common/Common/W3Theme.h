#pragma once

/*=========================================================================

File:			class CW3Theme
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-03-23
Last modify:	2016-03-23

=========================================================================*/

#include "common_global.h"

#include "W3Style.h"
#include "W3Enum.h"

class COMMON_EXPORT CW3Theme {
public:
	typedef struct _TOOLVBAR_SIZE_INFO {
		QMargins marginBox;
		QMargins marginContents;
		int spacingS = 0;
		int spacingM = 0;
		int spacingL = 0;
	} toolVBarSizeInfo;

public:
	CW3Theme();
	~CW3Theme();
	CW3Theme(const CW3Theme& t) {}
public:
	static CW3Theme* getInstance() {
		if (m_pInstance == nullptr) {
			m_pInstance = new CW3Theme();
			atexit(destroy);
		}
		return m_pInstance;
	}

	void setAppTheme(QApplication* app);

	QFont systemFont() { return m_fontSystem; }

	QString appToolButtonStyleSheet();
	QString appQSliderStyleSheet();
	QString appQScrollBarStyleSheet();
	QString appQRadioButtonStyleSheet();
	QString appQCheckBoxStyleSheet();
	QString appQGroupBoxStyleSheet();
	QString appQComboBoxStyleSheet();
	QString appQDoubleSpinBoxStyleSheet();
	QString ViewSpinBoxStyleSheet();
	QString appQLabelStyleSheet();
	QString appQMenuStyleSheet();
	QString appQLineEditStyleSheet();
	QString appAllInOneStyleSheet();
	QString BlankLineStyleSheet();

	QString titlebarStyleSheet();
	QString tabWestStyleSheet();
	QString toolbarStyleSheet();
	QString toolBoxStyleSheet();
	QString toolIconButtonStyleSheet();
	QString ChangeToolButtonImageStyleSheet(const QString& path,
											const QString& disabled_path);
	QString toolTaskStyleSheet();
	QString toolOTFStyleSheet();
	QString toolVisibilityStyleSheet();
	QString toolTMJModeSelectionStyleSheet();
	QString toolTMJRectStylesheet();
	QString toolGeneralTextStyleSheet();
	QString toolOrientationStyleSheet();
	QString toolEndoExplorerStyleSheet();
	QString toolEndoPathStyleSheet();
	QString toolTMJStyleSheet();
	QString toolNerveDrawModifyStylesheet(bool checked);
	QString appDialogStyleSheet();
	QString LightDialogStyleSheet();
	QString messageBoxStyleSheet();

	QString DicomInfoBoxStyleSheet();
	QString LineSeparatorStyleSheet();
	QString ColoredToolButtonStyleSheet(QColor color);

	QString FileSystemTreeWidgetStyleSheet();

	QString GlobalPreferencesDialogTabStyleSheet();
	QString GlobalPreferencesDialogToolButtonStyleSheet();
	QString GlobalPreferencesDialogComboBoxStyleSheet();
	QString GlobalPreferencesDialogSpinBoxStyleSheet();
	QString GlobalPreferencesDialogLineEditStyleSheet();
	QString GlobalPreferencesDialogListViewStyleSheet();

	QString FileTabToolButtonStyleSheet();
	QString FileTabRadioButtonStyleSheet();
	QString FileTabLabelStyleSheet();
	QString FileTabLineEditStyleSheet();
	QString FileTabExplorerTreeViewStyleSheet();
	QString FileTabStudyTreeHeaderStyleSheet();
	QString FileTabStudyTreeWidgetStyleSheet();
	QString FileTabDicomSummaryTableWidgetStyleSheet();
	QString FileTabSpinBoxStyleSheet();
	QString FileTabGroupBoxStyleSheet();

	QString cephIndicatorListStyleSheet(bool bAnalysisMode = 0);
	QString cephIndicatorBarStyleSheet();
	QString cephSurgeryBarStyleSheet();
	QString cephTracingBarStyleSheet();
	QString reportWindowStyleSheet();

	QString ViewMenuBarStyleSheet();
	QString ViewMenuBarButtonStyleSheet();
	QString ViewMenuBarOnOffSwitchStyleSheet(const QColor& bg_color, const QString& img_path);

	QString TableWidgetHeaderStyleSheet();
	QString NerveToolStyleSheet();
	QString MeasureListDlgStyleSheet();
	inline CW3TabWestStyle* tabWestStyle() const { return m_tabWestStyle; }
	inline CWToolMenuIconStyle* toolMenuIconStyle() const { return m_toolMenuIconStyle; }
	inline CW3ToolIconButtonStyle* toolIconButtonStyle() const { return m_toolIconButtonStyle; }
	inline CW3ToolTaskStyle* toolTaskStyle() const { return m_toolTaskStyle; }
	inline CW3CephTracingTaskToolStyle* cephTracingTaskToolStyle() const { return m_cephTracingTaskToolStyle; }

	QString AddImplantStyleSheet();
	QString DefaultImplantButtonStyleSheet();
	QString SelecedtImplantButtonStyleSheet();
	QString PlacedImplantButtonStyleSheet();

	inline QMargins getTabWestMargins() const { return m_tabWestMargins; }

	inline toolVBarSizeInfo getToolVBarSizeInfo() const { return m_toolVBarSizeInfo; }
	inline int getSizeWidthVBar() const { return m_sizeW_Vbar; }
	inline int getSizeHeightTitleBar() const { return m_sizeH_titlebar; }
	inline const QSize& size_tool_icon() const { return size_tool_icon_; }
	inline const int tool_icon_sub_menu_height() { return tool_icon_sub_menu_height_; }

	inline bool isLowResolution() const { return m_isLowResolution; }

	inline int fontsize_regular() const { return fontsize_regular_; }
	inline int fontsize_small() const { return fontsize_small_; }

	inline QSize size_button() const { return m_size_button; }

private:
	static void destroy() {
		if (m_pInstance != nullptr)
			delete m_pInstance;
	}

private:
	static CW3Theme* m_pInstance;

private:
	QSize size_global_preferences_dialog_;
	QSize size_global_preferences_tab_;
	QSize m_size_tabWest;
	QSize m_size_tool_task;
	QSize m_size_tool_menu;
	QSize m_size_tool_otf;
	QSize size_tool_icon_;
	QSize m_size_tool_tmj;
	QSize m_size_button;

	int tool_icon_sub_menu_height_ = 0;
	int m_sizeH_titlebar;
	int m_sizeW_Vbar;

	int fontsize_regular_;
	int fontsize_small_;

	QMargins m_tabWestMargins;

	toolVBarSizeInfo m_toolVBarSizeInfo;

	CW3TabWestStyle* m_tabWestStyle;
	CWToolMenuIconStyle* m_toolMenuIconStyle;
	CW3ToolIconButtonStyle* m_toolIconButtonStyle;
	CW3ToolTaskStyle* m_toolTaskStyle;
	CW3CephTracingTaskToolStyle* m_cephTracingTaskToolStyle;

	QFont m_fontSystem;

	bool m_isLowResolution;
};
