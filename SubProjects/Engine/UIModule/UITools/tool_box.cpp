#include "tool_box.h"

#include <qboxlayout.h>
#include <QLabel>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>

ToolBox::ToolBox(QFrame* parent) :
	QFrame(parent), content_layout_(new QHBoxLayout()),
	caption_(new QLabel) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	content_layout_->setSpacing(1);

	caption_->setObjectName("Caption");
	caption_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QVBoxLayout* main_layout = new QVBoxLayout(this);
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->addWidget(caption_.get());
	main_layout->addLayout(content_layout_.get());
	this->setLayout(main_layout);

	this->setObjectName("ToolBox");
	this->setStyleSheet(CW3Theme::getInstance()->toolBoxStyleSheet());
}

ToolBox::~ToolBox() {
	ClearTools(); // 외부에서 셋해주는 widget 및 layout이 있기 때문에 clear tools 해줘야 함
}

void ToolBox::addToolWidget(QWidget * widget) {
	tool_widget_ = widget;
	content_layout_->addWidget(widget);
}

void ToolBox::setVisible(bool visible) {
	QFrame::setVisible(visible);
	if (tool_widget_)
		tool_widget_->setVisible(visible);
}

void ToolBox::setCaptionName(const QString& name, Qt::Alignment align) {
	caption_->setText(name);
	caption_->setAlignment(align);
}
void ToolBox::setContentsMargins(const QMargins& margin) {
	content_layout_->setContentsMargins(margin);
}
void ToolBox::addToolLayout(QVBoxLayout* layout) {
	content_layout_->addLayout(layout);
	content_layout_->setAlignment(layout, Qt::AlignLeft);
}
void ToolBox::addToolLayout(QHBoxLayout* layout) {
	content_layout_->addLayout(layout);
	content_layout_->setAlignment(layout, Qt::AlignTop | Qt::AlignLeft);
}
void ToolBox::addToolLayout(QGridLayout* layout) {
	content_layout_->addLayout(layout);
	content_layout_->setAlignment(layout, Qt::AlignTop);
}

void ToolBox::setCaptionVisible(const bool & visible) {
  caption_->setVisible(visible);
}

QLayout * ToolBox::GetLayout() {
	return content_layout_.get();
}

void ToolBox::ClearTools() {
	CW3LayoutFunctions::RemoveWidgetsAll(content_layout_.get());
	tool_widget_ = nullptr;
}
