#include "group_box.h"

#include <QBoxLayout>
#include <QLabel>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"

GroupBox::GroupBox(QFrame* parent)
	: ToolBox(parent) {
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setStyleSheet(CW3Theme::getInstance()->FileTabGroupBoxStyleSheet());
}

GroupBox::~GroupBox() {
}

void GroupBox::SetContentsMargins(int left, int top, int right, int bottom) {
	ToolBox::setContentsMargins(QMargins(left, top, right, bottom));
}

void GroupBox::SetCaptionName(const QString& name, Qt::Alignment align) {
	ToolBox::setCaptionName(name, align);
}

void GroupBox::AddWidget(QWidget* widget) {
	ToolBox::addToolWidget(widget);
}

void GroupBox::AddLayout(QVBoxLayout* layout) {
	ToolBox::addToolLayout(layout);
}

void GroupBox::AddLayout(QHBoxLayout* layout) {
	ToolBox::addToolLayout(layout);
}

void GroupBox::AddLayout(QGridLayout* layout) {
	ToolBox::addToolLayout(layout);
}
