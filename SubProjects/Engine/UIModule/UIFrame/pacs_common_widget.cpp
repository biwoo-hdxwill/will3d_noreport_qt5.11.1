#include "pacs_common_widget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

#include "../../Resource/ResContainer/resource_container.h"

#include "../../Resource/Resource/nerve_resource.h"
#include "../../Resource/Resource/implant_resource.h"

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"

namespace
{
	const int kComboBoxLength = 150;
	const QMargins kMarginZero(0, 0, 0, 0);
	const QString kViewComboboxName = "ViewCombobox";
}

PacsCommonWidget::PacsCommonWidget(const TabType tab_type, const QStringList& view_list, QWidget* parent /*= 0*/)
	: QWidget(parent), tab_type_(tab_type), view_list_(view_list)
{
	Initialize();
	SetLayout();
}

PacsCommonWidget::~PacsCommonWidget()
{
	SAFE_DELETE_LATER(main_layout_);
}

const int PacsCommonWidget::GetViewListIndex() const
{
	QComboBox* view_combo_box = findChild<QComboBox*>(kViewComboboxName);
	if (view_combo_box)
	{
		return view_combo_box->currentIndex();
	}

	return -1;
}

void PacsCommonWidget::slotSetServerCurrentIndex(const int index)
{
	if (cur_server_index_ != index)
	{
		cur_server_index_ = index;
	}
}

void PacsCommonWidget::slotNerveVisibility(int state)
{
	if (state == Qt::CheckState::Checked)
	{
		nerve_visible_ = true;
	}
	else
	{
		nerve_visible_ = false;
	}

	emit sigNerveVisibility(nerve_visible_);
}

void PacsCommonWidget::slotImplantVisibility(int state)
{
	if (state == Qt::CheckState::Checked)
	{
		implant_visible_ = true;
	}
	else
	{
		implant_visible_ = false;
	}

	emit sigImplantVisibility(implant_visible_);
}

void PacsCommonWidget::slotEmitMPRViewChange(int index)
{
	bool swap = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting.mpr_view_list_swap;
	int emit_index = index;
	if (swap)
	{
		if (index == 1)
		{
			emit_index = 2;
		}
		else if (index == 2)
		{
			emit_index = 1;
		}
	}

	emit sigChangeViewType(emit_index);
}

void PacsCommonWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(kMarginZero);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);
}

void PacsCommonWidget::SetLayout()
{
	if (tab_type_ == TabType::TAB_MPR)
	{
		if (view_list_.size() > 1)
		{
			main_layout_->addLayout(CreateMPRContentsLayout());
		}
		else
		{
			main_layout_->addLayout(CreateMPRLightboxContentsLayout());
		}
	}
	else if (tab_type_ == TabType::TAB_PANORAMA)
	{
		main_layout_->addLayout(CreatePanoContentsLayout());
	}
}

QHBoxLayout* PacsCommonWidget::CreatePACSServerListLayout()
{
	QHBoxLayout* pacs_server_list_layout = new QHBoxLayout();
	{
		pacs_server_list_layout->setSpacing(10);

		QLabel* select_label = new QLabel(this);
		select_label->setText(lang::LanguagePack::txt_select() + " " + lang::LanguagePack::txt_server() + " :");

		QComboBox* pacs_server_list_combo_box = new QComboBox(this);
		pacs_server_list_combo_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());
		pacs_server_list_combo_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		pacs_server_list_combo_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());
		pacs_server_list_combo_box->setFixedWidth(kComboBoxLength);

		connect(pacs_server_list_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetServerCurrentIndex(int)));

		QStringList server_nickname_list;
		GlobalPreferences::GetInstance()->GetPACSServerNicknameList(server_nickname_list);
		int index = GlobalPreferences::GetInstance()->GetPACSServerSelectIndex();

		if (server_nickname_list.empty() == false)
		{
			server_cnt_ = server_nickname_list.size();
			pacs_server_list_combo_box->addItems(server_nickname_list);
			if (index != -1)
			{
				pacs_server_list_combo_box->setCurrentIndex(index);
			}
		}

		pacs_server_list_layout->addWidget(select_label);
		pacs_server_list_layout->addWidget(pacs_server_list_combo_box);
	}

	return pacs_server_list_layout;
}

QHBoxLayout* PacsCommonWidget::CreateCheckBoxLayout()
{
	QHBoxLayout* check_box_layout = new QHBoxLayout();
	{
		check_box_layout->setSpacing(10);

		QLabel* visibility_label = new QLabel(this);
		visibility_label->setText(lang::LanguagePack::txt_visibility());

		QCheckBox* nerve_hide_check = new QCheckBox(this);
		QCheckBox* implant_hide_check = new QCheckBox(this);

		nerve_hide_check->setText(lang::LanguagePack::txt_nerve());
		implant_hide_check->setText(lang::LanguagePack::txt_implant());

		connect(nerve_hide_check, &QCheckBox::stateChanged, this, &PacsCommonWidget::slotNerveVisibility);
		connect(implant_hide_check, &QCheckBox::stateChanged, this, &PacsCommonWidget::slotImplantVisibility);

		auto res_container = ResourceContainer::GetInstance();
		const NerveResource& res_nerve = res_container->GetNerveResource();
		if (res_nerve.GetNerveDataInVol().empty())
		{
			nerve_hide_check->setEnabled(false);
		}
		else
		{
			nerve_hide_check->setChecked(true);
		}

		const ImplantResource& res_implant = res_container->GetImplantResource();
		if (res_implant.data().empty())
		{
			implant_hide_check->setEnabled(false);
		}
		else
		{
			implant_hide_check->setChecked(true);
		}

		check_box_layout->addWidget(visibility_label);
		check_box_layout->addWidget(nerve_hide_check);
		check_box_layout->addWidget(implant_hide_check);
	}

	return check_box_layout;
}

QVBoxLayout* PacsCommonWidget::CreateMPRContentsLayout()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	{
		contents_layout->setMargin(0);
		contents_layout->setSpacing(10);

		QHBoxLayout* view_type_combo_box_layout = new QHBoxLayout();
		{
			view_type_combo_box_layout->setSpacing(10);

			QLabel* select_label = new QLabel(this);
			select_label->setText(lang::LanguagePack::txt_select() + " " + lang::LanguagePack::txt_view() + " :");

			QComboBox* view_type_combo_box = new QComboBox(this);
			view_type_combo_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());
			view_type_combo_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
			view_type_combo_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());
			view_type_combo_box->setFixedWidth(kComboBoxLength);
			view_type_combo_box->setObjectName(kViewComboboxName);
			
			bool swap = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting.mpr_view_list_swap;
			if (swap)
			{
				view_list_.swap(1, 2);
			}
			
			view_type_combo_box->addItems(view_list_);
						
			connect(view_type_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(slotEmitMPRViewChange(int)));

			view_type_combo_box_layout->addWidget(select_label);
			view_type_combo_box_layout->addWidget(view_type_combo_box);
		}
		
		contents_layout->addLayout(CreatePACSServerListLayout());
		contents_layout->addLayout(view_type_combo_box_layout);
		contents_layout->addLayout(CreateCheckBoxLayout());
	}

	return contents_layout;
}

QVBoxLayout* PacsCommonWidget::CreateMPRLightboxContentsLayout()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	{
		contents_layout->setMargin(0);
		contents_layout->setSpacing(10);

		contents_layout->addLayout(CreatePACSServerListLayout());
		contents_layout->addLayout(CreateCheckBoxLayout());
	}

	return contents_layout;
}

QVBoxLayout* PacsCommonWidget::CreatePanoContentsLayout()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	{
		contents_layout->setMargin(0);
		contents_layout->setSpacing(10);

		QHBoxLayout* view_type_combo_box_layout = new QHBoxLayout();
		{
			view_type_combo_box_layout->setSpacing(10);

			QLabel* select_label = new QLabel(this);
			select_label->setText(lang::LanguagePack::txt_select() + " " + lang::LanguagePack::txt_view() + " :");

			QComboBox* view_type_combo_box = new QComboBox(this);
			view_type_combo_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());
			view_type_combo_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
			view_type_combo_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());
			view_type_combo_box->setFixedWidth(kComboBoxLength);
			view_type_combo_box->setObjectName(kViewComboboxName);

			view_list_.erase(view_list_.begin() + 1);
			view_type_combo_box->addItems(view_list_);

			connect(view_type_combo_box, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigChangeViewType(int)));

			view_type_combo_box_layout->addWidget(select_label);
			view_type_combo_box_layout->addWidget(view_type_combo_box);
		}
		
		contents_layout->addLayout(CreatePACSServerListLayout());
		contents_layout->addLayout(view_type_combo_box_layout);
		contents_layout->addLayout(CreateCheckBoxLayout());
	}

	return contents_layout;
}
