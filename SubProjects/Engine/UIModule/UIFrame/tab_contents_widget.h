#pragma once
/**=================================================================================================

Project:		UIFrame
File:			tab_contents_widget.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-20
Last modify: 	2018-11-20

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <QWidget>
#include <memory>
#include "../../Common/Common/W3Enum.h"

#include "uiframe_global.h"

class TabSlotLayout;
class QVBoxLayout;
class QTabWidget;

class UIFRAME_EXPORT TabContentsWidget : public QWidget {
	Q_OBJECT

public:
	explicit TabContentsWidget(QWidget *parent = 0);
	~TabContentsWidget();

public:
	void setTabIdx(TabType eTabType);
	void SetOTFWidget(QWidget* widget);
	QWidget* GetTabSlotWidget() const;
	void initTab();
	void setOnlyTRDMode();

signals:
	void sigChangeTab(TabType eTabType);

public slots:
	void slotMenuClicked(int tabIndex);
private slots:
	void slotSetTabSlotLayout(QLayout * layout);

private:
	TabSlotLayout* m_pTabSlot;
	std::unique_ptr<QVBoxLayout> otf_layout_;
	QVBoxLayout* m_pMainLayout;
	QVBoxLayout* m_pTabLayout;
	QTabWidget* m_pTabWidget;
};

