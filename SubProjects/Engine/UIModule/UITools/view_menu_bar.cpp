#include "view_menu_bar.h"

#include <QLabel>
#include <QLayout>
#include <QAbstractSpinBox>

#include "../../Common/Common/W3Style.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3LayoutFunctions.h"
#include "../../Common/Common/global_preferences.h"

#include "view_menu.h"

ViewMenuBar::ViewMenuBar(const QString& caption, QFrame* parent) :
	QFrame(parent) {
	this->setObjectName("ViewMenuBar");
	this->setStyleSheet(CW3Theme::getInstance()->ViewMenuBarStyleSheet());

	main_layout_.reset(new QHBoxLayout);
	main_layout_->setContentsMargins(10, 1, 5, 0);
	main_layout_->setAlignment(Qt::AlignLeft);

	QLabel* caption_label = CreateLabel(caption);
	caption_label->setObjectName("ViewMenuBarCaption");
	main_layout_->addWidget(caption_label);

	QLabel* spacer = new QLabel(this);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	main_layout_->addWidget(spacer);

	content_layout_.reset(new QHBoxLayout);
	content_layout_->setContentsMargins(0, 0, 0, 0);
	content_layout_->setAlignment(Qt::AlignLeft);
	main_layout_->addLayout(content_layout_.get());

	this->setLayout(main_layout_.get());
}

ViewMenuBar::~ViewMenuBar() {}
void ViewMenuBar::ClearContents() {
	CW3LayoutFunctions::setVisibleWidgets(content_layout_.get(), false);
	CW3LayoutFunctions::RemoveWidgetsAll(content_layout_.get());
	spin_labels_.clear();
}
void ViewMenuBar::SetMaximize(const bool& maximize) {
	if (view_menus_)
		view_menus_->setMaximize(maximize);
}

void ViewMenuBar::SetMaximumRowColCount(const int& row, const int& col) {
	if (view_menus_)
		view_menus_->SetRowColCount(row, col);
}

void ViewMenuBar::AddSpinBox(std::vector<QAbstractSpinBox*>& spin_boxes) {
	spin_labels_.resize(spin_boxes.size());
	for(int i = 0; i < spin_boxes.size(); i++) {
		spin_labels_[i].reset(CreateLabel(spin_boxes[i]->objectName() + " : "));
		spin_labels_[i]->setObjectName("ViewMenuBarLabel");

		spin_boxes[i]->setAlignment(Qt::AlignCenter);
		spin_boxes[i]->setFixedSize(90, 17);
		spin_boxes[i]->setStyle(new ViewSpinBoxStyle);
		spin_boxes[i]->setStyleSheet(CW3Theme::getInstance()->ViewSpinBoxStyleSheet());

		content_layout_->addWidget(spin_labels_[i].get());
		content_layout_->addWidget(spin_boxes[i]);

#ifdef WILL3D_EUROPE
		bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
		if (!btn_enable)
		{
			spin_boxes[i]->hide();
			spin_labels_[i]->hide();
		}		
#endif // WILL3D_EUROPE
	}
}
void ViewMenuBar::AddWidget(std::vector<QWidget*> widget) {
	for (const auto& elem : widget) {
		content_layout_->addWidget(elem);
	}
}

void ViewMenuBar::AddMaximizeButton() {
	InitGridMaximizeButton(false);
}

void ViewMenuBar::AddGridButton() {
	InitGridMaximizeButton(true);
}

void ViewMenuBar::AddCloseButton() {
	view_menus_->AddCloseButton();

	connect(view_menus_.get(), &ViewMenu::sigClose, [=]() { emit sigClose(); });
}

void ViewMenuBar::HideMaximizeButton() {
	view_menus_->HideMaximizeButton();
}

#ifdef WILL3D_EUROPE
void ViewMenuBar::ShowLayoutSelectionView(const QPoint& global_pos)
{
	view_menus_->ShowLayoutSelectionView(global_pos);
}
#endif // WILL3D_EUROPE

void ViewMenuBar::InitGridMaximizeButton(const bool& enable_grid) {
	view_menus_.reset(new ViewMenu(enable_grid));
	main_layout_->addWidget(view_menus_.get());

	if (enable_grid) {
		connect(view_menus_.get(), &ViewMenu::sigSelectLayout, this, &ViewMenuBar::sigSelectLayout);
	}

	connect(view_menus_.get(), &ViewMenu::sigMaximizeonoff, this, &ViewMenuBar::sigMaximizeonoff);

#ifdef WILL3D_EUROPE
	bool btn_enable = GlobalPreferences::GetInstance()->preferences_.europe_window_btn_enable_;
	if (!btn_enable)
		view_menus_.get()->hide();
#endif // WILL3D_EUROPE
}

QLabel* ViewMenuBar::CreateLabel(const QString& text) {
	QLabel* label = new QLabel(this);
	label->setContentsMargins(0, 0, 0, 0);
	label->setMinimumHeight(20);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	label->setText(text);

	return label;
}
