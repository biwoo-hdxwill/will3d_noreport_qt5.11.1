#include "view_menu.h"

#include <qtoolbutton.h>
#include <qboxlayout.h>
#include <QWidgetAction>
#include <qevent.h>
#include <qgraphicsitem.h>

#include "../../Common/Common/global_preferences.h"

namespace {
	constexpr int kMarginRect = 2;
	constexpr float kSizeRect = 30.0f;
	const QBrush kDefaultBrush(Qt::white);
	const QBrush kSelectedBrush(QColor(130, 171, 255));

	const QString kMaxBtnStylesheet(
		"QToolButton#Maximize { border-width: 0px; background-color: transparent; padding: 0px; width: 20px; height: 15px; image: url(:/image/viewmenu/maximize_normal.png); }"
		"QToolButton#Maximize:hover { image: url(:/image/viewmenu/maximize_pressed.png); }"
		"QToolButton#Maximize:pressed { image: url(:/image/viewmenu/maximize_pressed.png); }"
	);
	const QString kMinBtnStylesheet(
		"QToolButton#Maximize { border-width: 0px; background-color: transparent; padding: 0px; width: 20px; height: 15px; image: url(:/image/viewmenu/minimize_normal.png); }"
		"QToolButton#Maximize:hover { image: url(:/image/viewmenu/minimize_pressed.png); }"
		"QToolButton#Maximize:pressed { image: url(:/image/viewmenu/minimize_pressed.png); }"
	);

	const QString kLayoutBtnStylesheet(
		"QToolButton#SelectLayout { border-width: 0px; background-color: transparent; padding: 0px; width: 20px; height: 15px; image: url(:/image/viewmenu/layoutchange_normal.png); }"
		"QToolButton#SelectLayout:hover { image: url(:/image/viewmenu/layoutchange_pressed.png); }"
		"QToolButton#SelectLayout:pressed { image: url(:/image/viewmenu/layoutchange_pressed.png); }"
	);

	const QString kCloseBtnStylesheet(
		"QToolButton#Close { border-width: 0px; background-color: transparent; padding: 0px; width: 20px; height: 15px; image: url(:/image/viewmenu/close_normal.png); }"
		"QToolButton#Close:hover { image: url(:/image/viewmenu/close_pressed.png); }"
		"QToolButton#Close:pressed { image: url(:/image/viewmenu/close_pressed.png); }"
	);
} // end of namespace

/*
	smseo : MPR 에서 사용할 Light box 의 레이아웃이 4X5라서 입력 파라메터를 추가했음.
	만약 Cross section에서 row, column count를 변경하고자 한다면
	"../../Common/Common/define_pano.h" 의 kMaxCrossSection를 주의해야 한다.
*/
LayoutSelectionView::LayoutSelectionView(QWidget* pParent, const int& row_count, const int& col_count)
	: QGraphicsView(pParent), row_count_(row_count), col_count_(col_count)
{
	this->setMouseTracking(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	this->setScene(new QGraphicsScene(this));
	this->setSceneRect(0.0f, 0.0f, kSizeRect*col_count_, kSizeRect*row_count_);

	rect_items_.reserve(row_count_*col_count_);
	const QPen rect_pen(Qt::darkGray);
	const QBrush rect_brush(Qt::white);
	for (int id = 0; id < row_count_ * col_count_; ++id)
	{
		QGraphicsRectItem* rect = this->scene()->addRect(0.0f, 0.0f, 0.0f, 0.0f);
		rect->setPen(rect_pen);
		rect->setBrush(rect_brush);
		rect_items_.push_back(rect);
	}

	this->setFixedSize(kSizeRect*col_count_ + kMarginRect, kSizeRect*row_count_ + kMarginRect);
}

LayoutSelectionView::~LayoutSelectionView()
{

}

void LayoutSelectionView::resizeEvent(QResizeEvent* pEvent)
{
	QGraphicsView::resizeEvent(pEvent);
	const float blockWidth = this->sceneRect().width() / col_count_;
	const float blockHeight = this->sceneRect().height() / row_count_;

	for (int r = 0; r < row_count_; ++r)
	{
		for (int c = 0; c < col_count_; ++c)
		{
			rect_items_[r + c * row_count_]->setRect(blockWidth * c, blockHeight * r, blockWidth, blockHeight);
		}
	}
}

void LayoutSelectionView::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);
	GetSelectedRectIndex(mapToScene(event->pos()));

	for (int r = 0; r < row_count_; ++r)
	{
		for (int c = 0; c < col_count_; ++c)
		{
			if (r < selected_rect_index_.rx() && c < selected_rect_index_.ry())
			{
				rect_items_[r + c * row_count_]->setBrush(kSelectedBrush);
			}
			else
			{
				rect_items_[r + c * row_count_]->setBrush(kDefaultBrush);
			}
		}
	}
}

void LayoutSelectionView::mouseReleaseEvent(QMouseEvent *event)
{
	QGraphicsView::mouseReleaseEvent(event);
	emit sigClicked(selected_rect_index_.rx(), selected_rect_index_.ry());
}

void LayoutSelectionView::GetSelectedRectIndex(const QPointF& scene_pos)
{
	for (int r = 0; r < row_count_; ++r)
	{
		for (int c = 0; c < col_count_; ++c)
		{
			if (rect_items_[r + c * row_count_]->contains(scene_pos))
			{
				selected_rect_index_.setX(r + 1);
				selected_rect_index_.setY(c + 1);
				return;
			}
		}
	}
	selected_rect_index_ = QPoint(0, 0);
}

ViewMenu::ViewMenu(bool enable_grid, QWidget* parent)
	: QWidget(parent)
{
	this->setContentsMargins(0, 0, 0, 0);
	this->setObjectName("ViewMenus");
	this->setStyleSheet("QWidget#ViewMenus { border-width: 0px; background-color: transparent; padding: 0px;}");

	QHBoxLayout* main_layout = new QHBoxLayout();
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);

	if (enable_grid)
	{
		select_layout_ = new QToolButton();
		select_layout_->setEnabled(true);
		select_layout_->setObjectName("SelectLayout");
		select_layout_->setStyleSheet(kLayoutBtnStylesheet);
		main_layout->addWidget(select_layout_);

#ifdef WILL3D_EUROPE
		bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
		if(!btn_enable)
			select_layout_->hide();
#endif // WILL3D_EUROPE
	}

	maximize_ = new QToolButton();
	maximize_->setObjectName("Maximize");
	maximize_->setStyleSheet(kMaxBtnStylesheet);
	main_layout->addWidget(maximize_);

	connections();

	this->setLayout(main_layout);

#ifdef WILL3D_EUROPE
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (!btn_enable)
		maximize_->hide();
#endif // WILL3D_EUROPE
}

ViewMenu::~ViewMenu()
{

}

void ViewMenu::setMaximize(const bool& max)
{
	maximized_ = max;
	maximize_->setStyleSheet(max ? kMinBtnStylesheet : kMaxBtnStylesheet);
}

void ViewMenu::connections()
{
	connect(maximize_, SIGNAL(clicked()), this, SLOT(slotMaximizeonoff()));

	if (select_layout_)
		connect(select_layout_, SIGNAL(clicked()), this, SLOT(slotClickedLayoutBtn()));
}

void ViewMenu::slotMaximizeonoff()
{
	maximized_ = !maximized_;
	setMaximize(maximized_);
	emit sigMaximizeonoff(maximized_);
}

void ViewMenu::slotClickedLayoutBtn()
{
	LayoutSelectionView *select_layout = new LayoutSelectionView(this, row_count_, col_count_);
	connect(select_layout, &LayoutSelectionView::sigClicked, this, &ViewMenu::slotSelectLayout);

	QWidgetAction *waction = new QWidgetAction(this);
	waction->setDefaultWidget(select_layout);

	layout_selection_widget_container_.clear();
	layout_selection_widget_container_.addAction(waction);
	layout_selection_widget_container_.exec(this->mapToGlobal(QPoint(2, select_layout_->height())));
}

void ViewMenu::slotSelectLayout(int row, int column)
{
	layout_selection_widget_container_.close();

	emit sigSelectLayout(row, column);
}

void ViewMenu::HideMaximizeButton()
{
	maximize_->setVisible(false);
	this->layout()->removeWidget(maximize_);
}

void ViewMenu::AddCloseButton()
{
	close_ = new QToolButton();
	close_->setObjectName("Close");
	close_->setStyleSheet(kCloseBtnStylesheet);
	this->layout()->addWidget(close_);

	connect(close_, &QToolButton::pressed, [=]() { emit sigClose(); });

#ifdef WILL3D_EUROPE
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (!btn_enable)
		close_->hide();
#endif // WILL3D_EUROPE
}

void ViewMenu::SetRowColCount(const int & row_count, const int & col_count)
{
	row_count_ = row_count;
	col_count_ = col_count;
}

#ifdef WILL3D_EUROPE
void ViewMenu::ShowLayoutSelectionView(const QPoint& global_pos)
{
	LayoutSelectionView* select_layout = new LayoutSelectionView(this, row_count_, col_count_);
	connect(select_layout, &LayoutSelectionView::sigClicked, this, &ViewMenu::slotSelectLayout);

	QWidgetAction* waction = new QWidgetAction(this);
	waction->setDefaultWidget(select_layout);

	layout_selection_widget_container_.clear();
	layout_selection_widget_container_.addAction(waction);
	layout_selection_widget_container_.exec(global_pos);

	select_layout->deleteLater();
}
#endif // WILL3D_EUROPE
