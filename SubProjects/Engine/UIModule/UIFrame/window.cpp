#include "window.h"

#include <qgridlayout.h>

#include "../../Common/Common/W3LayoutFunctions.h"
#include <Engine/UIModule/UITools/view_menu_bar.h>

/**=================================================================================================
class Window
*===============================================================================================**/

Window::Window(QWidget *parent) : QWidget(parent) {}

Window::~Window(void)
{
	if (view_layout_)
		CW3LayoutFunctions::RemoveWidgetsAll(view_layout_.get());
}

void Window::setVisible(bool is_visible)
{
	QWidget::setVisible(is_visible);

	if (is_visible && !IsSetLayout())
		slotSelectLayout(grid_row_, grid_col_);

	if (view_menu_)
		view_menu_->setVisible(is_visible);

	if (view_layout_)
		CW3LayoutFunctions::setVisibleWidgets(view_layout_.get(), is_visible);
}
void Window::SetMaximize(const bool& maximize)
{
	view_menu_->SetMaximize(maximize);
}

void Window::HideMaximizeButton()
{
	view_menu_->HideMaximizeButton();
}

void Window::SetView(QWidget* view)
{
	if (view_layout_)
	{
		CW3LayoutFunctions::setVisibleWidgets(view_layout_.get(), false);
		CW3LayoutFunctions::RemoveWidgetsAll(view_layout_.get());

		view_layout_.reset();
	}
	views_.clear();
	views_.push_back(view);
}
void Window::SetViews(const std::vector<QWidget*>& views)
{
	if (view_layout_)
	{
		CW3LayoutFunctions::setVisibleWidgets(view_layout_.get(), false);
		CW3LayoutFunctions::RemoveWidgetsAll(view_layout_.get());

		view_layout_.reset();
	}
	views_ = views;
}

void Window::SetViews(const std::vector<QWidget*>& views,
	const int& row, const int& col)
{
	views_ = views;
	SetViewLayout(row, col);
}

void Window::SetViewLayout(const int& grid_row, const int& grid_col)
{
	if (view_layout_)
	{
		CW3LayoutFunctions::setVisibleWidgets(view_layout_.get(), false);
		CW3LayoutFunctions::RemoveWidgetsAll(view_layout_.get());
	}

#if GRID_LAYOUT
	view_layout_.reset(new QGridLayout());
	view_layout_->setSpacing(1);
	view_layout_->setContentsMargins(0, 0, 0, 0);

	grid_row_ = grid_row;
	grid_col_ = grid_col;

	if (views_.size() >= grid_row_ * grid_col_)
	{
		for (int j = 0; j < grid_col_; j++)
		{
			for (int i = 0; i < grid_row_; i++)
			{
				view_layout_->addWidget(views_[j + i * grid_col_], i, j);
			}
		}
	}

	for (int i = 0; i < view_layout_->rowCount(); i++)
	{
		view_layout_->setRowStretch(i, 1);
	}
	for (int i = 0; i < view_layout_->columnCount(); i++)
	{
		view_layout_->setColumnStretch(i, 1);
	}
#else
	view_layout_.reset(new QVBoxLayout());
	view_layout_->setSpacing(1);
	view_layout_->setContentsMargins(0, 0, 0, 0);

	grid_row_ = grid_row;
	grid_col_ = grid_col;

	for (int i = 0; i < grid_row_; i++)
	{
		QHBoxLayout* row = new QHBoxLayout();
		row->setSpacing(1);
		row->setContentsMargins(0, 0, 0, 0);
		view_layout_->addLayout(row, 0);
		for (int j = 0; j < grid_col_; j++)
		{
			QWidget* view = views_[i * grid_col_ + j];
			row->addWidget(view, 0);
		}
	}
#endif

	main_layout_.reset(new QVBoxLayout);
	main_layout_->setContentsMargins(0, 0, 0, 0);
	main_layout_->setSpacing(0);
	main_layout_->addWidget(view_menu_.get());
	main_layout_->addLayout(view_layout_.get());

	this->setLayout(main_layout_.get());

	if (view_layout_)
	{
		CW3LayoutFunctions::setVisibleWidgets(view_layout_.get(), true);
	}
}

bool Window::IsSetLayout()
{
	if (view_layout_.get())
		return true;
	return false;
}

/**********************************************************************************************
LayoutSelectionView의 row_count, col_count를 초기화해주기 위한 함수

@author	Seo Seok Man
@date	2018-10-08
 **********************************************************************************************/
void Window::SetMaximumRowColCount(const int & row, const int & col)
{
	view_menu_->SetMaximumRowColCount(row, col);
}

void Window::InitViewMenu()
{
	view_menu_.reset(new ViewMenuBar(window_title_));
	view_menu_->setVisible(false);
}

void Window::InitViewMenu(const QString & caption)
{
	window_title_ = caption;
	view_menu_.reset(new ViewMenuBar(caption));
	view_menu_->setVisible(false);
}

void Window::AddSpinBox(std::vector<QAbstractSpinBox*> spin_boxes)
{
	view_menu_->AddSpinBox(spin_boxes);
}

void Window::AddWidget(std::vector<QWidget*> widget)
{
	view_menu_->AddWidget(widget);
}

void Window::AddMaximizeButton()
{
	view_menu_->AddMaximizeButton();

	connect(view_menu_.get(), SIGNAL(sigMaximizeonoff(bool)), this, SLOT(slotMaximizeonoff(bool)));
}

void Window::AddGridButton()
{
	view_menu_->AddGridButton();

	connect(view_menu_.get(), &ViewMenuBar::sigSelectLayout, this, &Window::slotSelectLayout);
	connect(view_menu_.get(), &ViewMenuBar::sigMaximizeonoff, this, &Window::slotMaximizeonoff);
}

void Window::AddCloseButton()
{
	view_menu_->AddCloseButton();

	connect(view_menu_.get(), &ViewMenuBar::sigClose, [=]() { emit sigWindowClose(); });
}

void Window::ClearViewMenuContents()
{
	view_menu_->ClearContents();
}

#ifdef WILL3D_EUROPE
void Window::ShowLayoutSelectionView(const QPoint& global_pos)
{
	view_menu_->ShowLayoutSelectionView(global_pos);
}
#endif // WILL3D_EUROPE

void Window::slotSelectLayout(int row, int column)
{
	this->SetViewLayout(row, column);
	emit sigSelectLayout(grid_row_, grid_col_);
}

void Window::slotMaximizeonoff(bool max)
{
	emit sigMaximizeOnOff(max);
}
