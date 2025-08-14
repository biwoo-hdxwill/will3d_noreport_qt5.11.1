#pragma once
/*=========================================================================
File:			pacs_common_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-16
Last modify:	2021-08-03

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>
#include "../../Common/Common/W3Enum.h"
#include "uiframe_global.h"

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QLabel;
class UIFRAME_EXPORT PacsCommonWidget : public QWidget
{
	Q_OBJECT

public:
	PacsCommonWidget(const TabType tab_type, const QStringList& view_list, QWidget* parent = 0);
	virtual ~PacsCommonWidget();

	PacsCommonWidget(const PacsCommonWidget&) = delete;
	const PacsCommonWidget& operator=(const PacsCommonWidget&) = delete;

	const int GetViewListIndex() const;

	inline int server_cnt() { return server_cnt_; }
	inline int cur_server_index() { return cur_server_index_; }
	inline bool nerve_visible() { return nerve_visible_; }
	inline bool implant_visible() { return implant_visible_; }

signals:
	void sigChangeViewType(const int index);
	void sigNerveVisibility(bool is_visible);
	void sigImplantVisibility(bool is_visible);
	
private slots:
	void slotSetServerCurrentIndex(const int index);
	void slotNerveVisibility(int state);
	void slotImplantVisibility(int state);
	void slotEmitMPRViewChange(int index);

private:
	void Initialize();
	void SetLayout();

	QHBoxLayout* CreatePACSServerListLayout();
	QHBoxLayout* CreateCheckBoxLayout();
	QVBoxLayout* CreateMPRContentsLayout();
	QVBoxLayout* CreateMPRLightboxContentsLayout();
	QVBoxLayout* CreatePanoContentsLayout();
	
private:
	QVBoxLayout* main_layout_ = nullptr;

	TabType tab_type_ = TabType::TAB_UNKNOWN;
	QStringList view_list_;

	int cur_server_index_ = -1;	
	int server_cnt_ = 0;

	bool nerve_visible_ = false;
	bool implant_visible_ = false;
};
