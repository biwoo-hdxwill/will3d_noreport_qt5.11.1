#include "user_list_item.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

UserListItem::UserListItem(QWidget* parent) : QWidget(parent) {
	QHBoxLayout* layout = new QHBoxLayout();
	QVBoxLayout* text_layout = new QVBoxLayout();
	QLabel* icon_label = new QLabel();
	name_label_ = new QLabel();
	user_type_label_ = new QLabel();

	QImage icon;
	icon.load(":/image/usermanager/icon.png");
	icon_label->setPixmap(QPixmap::fromImage(icon));
	icon_label->setContentsMargins(0, 0, 0, 0);
	icon_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	icon_label->setAlignment(Qt::AlignCenter);

	text_layout->setSpacing(0);
	text_layout->addWidget(name_label_);
	text_layout->addWidget(user_type_label_);

	layout->setSpacing(8);
	layout->addWidget(icon_label);
	layout->addLayout(text_layout);

	setLayout(layout);
}

UserListItem::~UserListItem() {
}

void UserListItem::SetName(const QString& name) {
	name_label_->setText(name);
}

void UserListItem::SetUserType(const QString& user_type) {
	user_type_label_->setText(user_type);
}
