#pragma once

/**=================================================================================================

Project:		UIFrame
File:			window.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-12
Last modify: 	2018-02-12

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>
#include <memory>

#include <QLayout>
#include <QWidget>
#include <QAbstractSpinBox>

#include "uiframe_global.h"

#define GRID_LAYOUT 0

class QGridLayout;
class ViewMenuBar;

class UIFRAME_EXPORT Window : public QWidget
{
	Q_OBJECT
public:
	explicit Window(QWidget *parent = 0);
	virtual ~Window();

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;

public:
	virtual void setVisible(bool is_visible) override;
	void SetMaximize(const bool& maximize);
	void HideMaximizeButton();

	void SetView(QWidget* view);
	void SetViews(const std::vector<QWidget*>& views);
	void SetViews(const std::vector<QWidget*>& views, const int& row, const int& col);

	bool IsSetLayout();
	void SetMaximumRowColCount(const int& row, const int& col);
	inline void SetViewLayoutCount(const int& row, const int& col) noexcept { grid_row_ = row; grid_col_ = col; }
	inline int grid_row() const { return grid_row_; }
	inline int grid_col() const { return grid_col_; }

	inline QString window_title() const { return window_title_; }

#ifdef WILL3D_EUROPE
	void ShowLayoutSelectionView(const QPoint& global_pos);
#endif // WILL3D_EUROPE

signals:
	void sigSelectLayout(int, int);
	void sigMaximizeOnOff(bool is_maximize);
	void sigWindowClose();

protected slots:
	virtual void slotSelectLayout(int row, int column);

private slots:
	void slotMaximizeonoff(bool max);

protected:
	virtual void Initialize() = 0;

	void InitViewMenu();
	void InitViewMenu(const QString& caption);
	void AddSpinBox(std::vector<QAbstractSpinBox*> spin_boxes);
	void AddWidget(std::vector<QWidget*> widget);
	void AddMaximizeButton();
	void AddGridButton();
	void AddCloseButton();
	void ClearViewMenuContents();

protected:
	QString window_title_;

private:
	void SetViewLayout(const int& grid_row, const int& grid_col);

private:
	std::vector<QWidget*> views_;

	std::unique_ptr<QVBoxLayout> main_layout_ = nullptr;
#if GRID_LAYOUT
	std::unique_ptr<QGridLayout> view_layout_ = nullptr;
#else
	std::unique_ptr<QVBoxLayout> view_layout_ = nullptr;
#endif

	std::unique_ptr<ViewMenuBar> view_menu_ = nullptr;

	int grid_row_ = 1;
	int grid_col_ = 1;
};
