#include "tab_slot_layout.h"

TabSlotLayout::TabSlotLayout(QWidget *parent)
	: QWidget(parent) {
	main_layout_ = new QVBoxLayout;
	this->setLayout(main_layout_);

	QSizePolicy SizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	SizePolicy.setHorizontalStretch(0);
	SizePolicy.setVerticalStretch(0);
	setSizePolicy(SizePolicy);

	this->layout()->setContentsMargins(QMargins(0, 0, 0, 0));

	setMouseTracking(true);
}

void TabSlotLayout::setViewLayout(QLayout* layout) {
	QVBoxLayout* tabLayout = (QVBoxLayout*)this->layout();

	if (view_layout_)
		tabLayout->removeItem(view_layout_);

	view_layout_ = layout;
	tabLayout->addLayout(view_layout_);
}
