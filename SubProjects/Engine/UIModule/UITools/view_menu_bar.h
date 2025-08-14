#pragma once
#include <memory>
#include <QFrame>

#include "../../Common/Common/W3Enum.h"

#include "uitools_global.h"

class QAbstractSpinBox;
class QHBoxLayout;
class QLabel;

class ViewMenu;

class UITOOLS_EXPORT ViewMenuBar : public QFrame {
	Q_OBJECT
public:
	ViewMenuBar(const QString& caption, QFrame* parent = 0);
	~ViewMenuBar();

	ViewMenuBar(const ViewMenuBar&) = delete;
	ViewMenuBar& operator=(const ViewMenuBar&) = delete;

public:
	void ClearContents();
	void SetMaximize(const bool& maximize);
	void SetMaximumRowColCount(const int& row, const int& col);

	void AddSpinBox(std::vector<QAbstractSpinBox*>& spin_boxes);
	void AddWidget(std::vector<QWidget*> widget);
	void AddMaximizeButton();	
	void AddGridButton();//GridButton은 Maximize button이 항상 따라다님
	void AddCloseButton();

	void HideMaximizeButton();

#ifdef WILL3D_EUROPE
	void ShowLayoutSelectionView(const QPoint& global_pos);
#endif // WILL3D_EUROPE

signals:
	void sigSelectLayout(int, int);
	void sigMaximizeonoff(bool);
	void sigClose();

private:
	void InitGridMaximizeButton(const bool& enable_grid);
	QLabel* CreateLabel(const QString& text);

private:
	std::unique_ptr<QHBoxLayout> main_layout_;
	std::unique_ptr<QHBoxLayout> content_layout_;
	std::unique_ptr<ViewMenu> view_menus_ = nullptr;
	std::vector<std::unique_ptr<QLabel>> spin_labels_;
};
