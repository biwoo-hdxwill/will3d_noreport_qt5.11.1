#pragma once

/**=================================================================================================

Project: 			UIComponent
File:				viewitems.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-28
Last modify:		2017-08-28

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <memory>

#include <QObject>
#include <QGraphicsScene>

#if defined(__APPLE__)
#include <glm/detail/type_mat.hpp>
#else
#include <GL/glm/detail/type_mat.hpp>
#endif

#include "../UIViewController/view_render_param.h"

class SimpleTextItem;
class CW3ViewRuler;
class CW3ViewAlignTextItem;
class ViewNavigatorItem;
class CW3Slider_2DView;
class CW3FilteredTextItem;
class ImageFilterSelector;
class ViewBorderItem;
class CW3TextItem;
class GridLines;
class DirectionTextItem;

class Viewitems : public QObject {
	Q_OBJECT
public:
	Viewitems();
	~Viewitems();

	Viewitems(const Viewitems&) = delete;
	Viewitems& operator=(const Viewitems&) = delete;

	enum ItemType {
		RULER = 0,
		ALIGN_TEXTS,
		NAVIGATION,
		FILTERED_TEXT,
		SLIDER,
		BORDER,
		HU_TEXT,
		GRID,
		DIRECTION_TEXT,
		SHARPEN_FILTER,
		ITEM_TYPE_END
	};

signals:
	void sigRotateMatrix(const glm::mat4&);
	void sigSliderValueChanged(int);
	void sigActiveFilteredItem(const QString& text);
	void sigActiveSharpenItem(const int index);

public:
	void InitItem(const ItemType& type, QGraphicsScene* scene);
	void SetEnabledItem(const ItemType& type, const bool& isEnable);
	void SetRulerColor(const QColor& color);
	void SetBorderColor(const QColor & color);
	void SetSliderTextColor(const QColor & color);

	void HideAllUI(const bool& is_hide);
	void HideText(const bool& is_hide);
	void ViewEnableStatusChanged(const bool& view_enable);

	void resizeEvent(const ViewRenderParam& view_param);
	bool EditSliderValue(const int& delta);

	void SetViewRulerItem(const ViewRenderParam& view_param);
	void SetGridItem(const ViewRenderParam& view_param);
	void SetWorldAxisDirection(const glm::mat4 & rotate_mat, const glm::mat4 & view_mat);
	void SetSliderValue(const int& value);
	void SetSliderRange(const int& min, const int& max);
	void GetSliderRange(int* min, int* max);
	void SetSliderInvertedAppearance(const bool& is_enable);
	void SetSliderInterval(const int& interval);
	void AddFilteredItem(const QString& text);
	void ChangeFilteredItemText(const QString& text);
	void ChangeSharpenLevel(const int level);
	void SetDirectionTextItem(const QString& text, const bool& is_align_left);

	bool IsUnderMouse();
	const bool IsTextVisible() const;
	const bool IsUIVisible() const;

	int GetSliderValue() const;

	void SetHUValue(const QString& text);
	void SetGridOnOff(const bool& visible);

	void ApplyPreferences();
	
private:
	void InitViewRulerItem(QGraphicsScene* scene);
	void InitAlignItem(QGraphicsScene* scene);
	void InitNavigationItem(QGraphicsScene* scene);
	void InitSlider(QGraphicsScene* scene);
	void InitBorder(QGraphicsScene * scene);
	void InitHUValueLabel(QGraphicsScene* scene);
	void InitFilteredItem(QGraphicsScene* scene);
	void InitGridItem(QGraphicsScene* scene);
	void InitDirectionItem(QGraphicsScene * scene);
	void InitSharpenItem(QGraphicsScene * scene);
	void PrintLogAndAssert(const char* msg);
	void SetVisibleItems();
	void SetVisibleItem(const ItemType& type, const bool& isVisible);

private slots:
	void slotSliderValueChanged(int value);

private:
	CW3ViewRuler* view_ruler_ = nullptr;
	GridLines* grid_ = nullptr;
	CW3ViewAlignTextItem* align_item_ = nullptr;
	ViewNavigatorItem* navigation_item_ = nullptr;
	CW3FilteredTextItem* filtered_item_ = nullptr;
	DirectionTextItem* direction_item_ = nullptr;
	ImageFilterSelector* sharpen_item_ = nullptr;

	struct Slider {
		QGraphicsProxyWidget* proxy = nullptr;
		CW3Slider_2DView* content = nullptr;
		CW3TextItem* text = nullptr;
		int interval = 1;
	};
	Slider slider_;

	ViewBorderItem* view_border_ = nullptr;
	SimpleTextItem* HU_value_label_ = nullptr;

	std::vector<QGraphicsItem*> casted_items_;
	std::vector<bool> enabled_items_;

	bool show_rulers_ = false;
	bool show_slice_numbers_ = false;
	bool hide_all_ui_ = false;	
	bool view_enable_ = true;
};
