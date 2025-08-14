#include "W3ToolVBar.h"

#include <QVBoxLayout>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

CW3ToolVBar::CW3ToolVBar(QWidget* parent) : QScrollArea(parent) {
	this->setObjectName("ToolVBar");

	this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setWidgetResizable(true);
	this->setStyleSheet(CW3Theme::getInstance()->toolbarStyleSheet());
	this->setFixedWidth(CW3Theme::getInstance()->getSizeWidthVBar());

	main_layout_.reset(new QVBoxLayout());
	main_layout_->setSpacing(0);
	main_layout_->setContentsMargins(0, 0, 0, 0);
	main_layout_->setAlignment(Qt::AlignTop);

	QWidget* contents_widget = new QWidget;
	contents_widget->setLayout(main_layout_.get());
	this->setWidget(contents_widget);

#ifndef WILL3D_VIEWER
	setTabTools(TabType::TAB_FILE);
#else
	setTabTools(TabType::TAB_MPR);
#endif

	setVisible(false);
}

CW3ToolVBar::~CW3ToolVBar() {
	CW3LayoutFunctions::RemoveWidgets(main_layout_.get());
}

void CW3ToolVBar::setTabTools(const TabType& tab_type) {
	bool is_tool_list_empty = false;
	ToolMgr* tool_mgr = ToolMgr::instance();
	tool_mgr->RemoveTools(main_layout_.get());
	switch (tab_type) {
	default:
		is_tool_list_empty = true;
		break;
	case TabType::TAB_MPR:
		is_tool_list_empty = tool_mgr->GetMPRTools(main_layout_.get());
		break;
	case TabType::TAB_PANORAMA: 
		tool_mgr->GetPanoTools(main_layout_.get());
		break;
//20250210 LIN
//#ifndef WILL3D_VIEWER
	case TabType::TAB_IMPLANT: 
		tool_mgr->GetImplantTools(main_layout_.get());
		break;
//#endif
	case TabType::TAB_TMJ:
		tool_mgr->GetTMJTools(main_layout_.get());
		break;
#ifndef WILL3D_LIGHT
	case TabType::TAB_3DCEPH:
		tool_mgr->GetCephTools(main_layout_.get());
		break;
	case TabType::TAB_FACESIM:
		tool_mgr->GetFaceTools(main_layout_.get());
		break;
	case TabType::TAB_SI:
		tool_mgr->GetSITools(main_layout_.get());
		break;
	case TabType::TAB_ENDO: 
		tool_mgr->GetEndoTools(main_layout_.get());
		break;
#endif
	}
	tool_mgr->SetVisibleTools(main_layout_.get(), true);

	setVisible(!is_tool_list_empty);
}
