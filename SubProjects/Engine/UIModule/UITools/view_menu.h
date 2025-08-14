#pragma once
/*=========================================================================

File:			class CW3ViewMenus
Language:		C++11
Library:		Qt 5.2.0
Author:			Jung Dae Gun
First date:		2016-05-09
Last modify:	2016-05-09

=========================================================================*/
#include <vector>
#include <QGraphicsView>
#include <qmenu.h>

namespace ui_tools {
const int kDefalutLayoutRow = 4;
const int kDefaultLayoutCol = 4;
}

class QToolButton;
class QGraphicsRectItem;

class LayoutSelectionView : public QGraphicsView {
	Q_OBJECT
public:
	explicit LayoutSelectionView(QWidget *pParent = 0,
								 const int& row_count = ui_tools::kDefalutLayoutRow,
								 const int& col_count = ui_tools::kDefaultLayoutCol);
	~LayoutSelectionView();

signals:
	void sigClicked(int, int);

private:
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;

	void GetSelectedRectIndex(const QPointF& scene_pos);

private:
	std::vector<QGraphicsRectItem*> rect_items_;
	QPoint selected_rect_index_; // x : row, y : column

	int row_count_ = ui_tools::kDefalutLayoutRow;
	int col_count_ = ui_tools::kDefaultLayoutCol;
};

class ViewMenu : public QWidget {
	Q_OBJECT
public:
	ViewMenu(bool enable_grid = false, QWidget *parent = 0);
	~ViewMenu();

	void setMaximize(const bool&);
	void HideMaximizeButton();
	void AddCloseButton();
	void SetRowColCount(const int& row_count, const int& col_count);
#ifdef WILL3D_EUROPE
	void ShowLayoutSelectionView(const QPoint& global_pos);
#endif // WILL3D_EUROPE

signals:
	void sigMaximizeonoff(bool);
	void sigSelectLayout(int, int);
	void sigClose();

public slots:
	void slotMaximizeonoff();
	void slotClickedLayoutBtn();
	void slotSelectLayout(int row, int column);

private:
	void connections();

private:
	QToolButton *close_ = nullptr;

	QToolButton *maximize_ = nullptr;
	bool maximized_ = false;
	
	QToolButton *select_layout_ = nullptr;
	QMenu layout_selection_widget_container_;
	int row_count_ = ui_tools::kDefalutLayoutRow;
	int col_count_ = ui_tools::kDefaultLayoutCol;
};
