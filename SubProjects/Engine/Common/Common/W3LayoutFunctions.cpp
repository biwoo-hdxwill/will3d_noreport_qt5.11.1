#include "W3LayoutFunctions.h"

#include <QWidget>

void CW3LayoutFunctions::setVisibleWidgets(QLayout* layout, bool isVisible) {
	for (int i = 0; i < layout->count(); i++) {
		if (layout->itemAt(i)->widget() == nullptr) {
			if (layout->itemAt(i)->layout() != nullptr)
				setVisibleWidgets(layout->itemAt(i)->layout(), isVisible);
		} else {
			layout->itemAt(i)->widget()->setVisible(isVisible);
		}
	}
}

void CW3LayoutFunctions::RemoveWidgetsAll(QLayout* layout) {
	std::vector<QWidget*> remove_widgets;
	std::vector<QSpacerItem*> remove_spacer_items;
	
	for (int i = 0; i < layout->count(); i++) {
		if (layout->itemAt(i)->widget())
		{
			QWidget* widget = layout->itemAt(i)->widget();
			remove_widgets.push_back(widget);
		}
		else if (layout->itemAt(i)->spacerItem())
		{
			QSpacerItem* spacer_item = layout->itemAt(i)->spacerItem();
			remove_spacer_items.push_back(spacer_item);
		}
		else if (layout->itemAt(i)->layout())
		{
			RemoveWidgetsAll(layout->itemAt(i)->layout());
		}
	}

	for (auto& elem : remove_widgets) {
		layout->removeWidget(elem);
		elem->setParent(nullptr);
	}

	for (auto& elem : remove_spacer_items)
	{
		layout->removeItem(elem);
	}
}

void CW3LayoutFunctions::RemoveWidgets(QLayout* layout) {
	std::vector<QWidget*> remove_widgets;
	std::vector<QSpacerItem*> remove_spacer_items;

	for (int i = 0; i < layout->count(); i++) {
		if (layout->itemAt(i)->widget())
		{
			QWidget* widget = layout->itemAt(i)->widget();
			remove_widgets.push_back(widget);
		}
		else if (layout->itemAt(i)->spacerItem())
		{
			QSpacerItem* spacer_item = layout->itemAt(i)->spacerItem();
			remove_spacer_items.push_back(spacer_item);
		}
	}

	for (auto& elem : remove_widgets) {
		layout->removeWidget(elem);
		elem->setParent(nullptr);
	}

	for (auto& elem : remove_spacer_items)
	{
		layout->removeItem(elem);
	}
}
