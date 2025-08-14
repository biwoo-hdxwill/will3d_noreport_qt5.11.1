#include "preferences_pacs_server_widget.h"

#include <QTableWidget>
#include <QLabel>
#include <QToolButton>
#include <QHeaderView>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout> 

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/global_preferences.h"

#include "pacs_add_server_dialog.h"

PreferencesPACSServerWidget::PreferencesPACSServerWidget(QWidget* parent /*= nullptr*/)
	: BaseWidget(parent)
{
	contents_layout()->addLayout(CreateMainContentsLayout());
	SetPacsTableWidget();

	ResetButtonDisable();
}

PreferencesPACSServerWidget::~PreferencesPACSServerWidget()
{
	int size = pacs_server_table_widget_->rowCount();
	for (int i = size - 1; i >= 0; --i)
	{ 
		pacs_server_table_widget_->removeRow(i);
	}
}

void PreferencesPACSServerWidget::slotPACSServerSelectButton()
{
	int index = pacs_server_table_widget_->currentRow();
	if (index == -1)
	{
		return;
	}

	SetSelectedPACSServerInfo(index);
}

void PreferencesPACSServerWidget::slotPACSServerAddButton()
{
	QStringList nickname_list;

	int size = pacs_server_table_widget_->rowCount();
	for (int i = 0; i < size; ++i)
	{
		nickname_list << pacs_server_table_widget_->item(i, 0)->text();
	}

	PACSAddServerDialog add_pacs_server_dlg;
	connect(&add_pacs_server_dlg, &PACSAddServerDialog::sigAddPACSServerInfo, this, &PreferencesPACSServerWidget::slotAddPACSServer);

	add_pacs_server_dlg.set_nickname_list(nickname_list);

	add_pacs_server_dlg.exec();

	disconnect(&add_pacs_server_dlg, &PACSAddServerDialog::sigAddPACSServerInfo, this, &PreferencesPACSServerWidget::slotAddPACSServer);
}

void PreferencesPACSServerWidget::slotPACSServerDeleteButton()
{
	int prev_size = pacs_server_table_widget_->rowCount();
	if (prev_size == 0)
	{
		return;
	}

	int remove_index = pacs_server_table_widget_->currentRow();
	pacs_server_table_widget_->removeRow(remove_index);

	GlobalPreferences::GetInstance()->RemovePACSServer(remove_index);
	int select_num = GlobalPreferences::GetInstance()->preferences_.pacs_server_list.select_num;
	if (select_num == -1)
	{
		nickname_line_edit_->clear();
		server_ae_title_line_edit_->clear();
		server_ip_line_edit_->clear();
		port_line_edit_->clear();
	}
}

void PreferencesPACSServerWidget::slotAddPACSServer(const QString& nickname, const QString& ae_title, const QString& ip, const QString& port)
{
	int index = pacs_server_table_widget_->rowCount();
	pacs_server_table_widget_->insertRow(index);
	pacs_server_table_widget_->setItem(index, 0, new QTableWidgetItem(nickname));
	pacs_server_table_widget_->setItem(index, 1, new QTableWidgetItem(ae_title));
	pacs_server_table_widget_->setItem(index, 2, new QTableWidgetItem(ip));
	pacs_server_table_widget_->setItem(index, 3, new QTableWidgetItem(port));

	GlobalPreferences::GetInstance()->AddPACSServer(nickname, ae_title, ip, port);
}

QVBoxLayout* PreferencesPACSServerWidget::CreateMainContentsLayout()
{
	QVBoxLayout* pacs_layout = new QVBoxLayout();
	{
		pacs_layout->setContentsMargins(kContentsMargins);
		pacs_layout->setSpacing(kSpacing10);

		pacs_layout->addLayout(CreatePACSServerListLayout());
		pacs_layout->addLayout(CreateSelectedPACSServerLayout());
	}

	return pacs_layout;
}

QVBoxLayout* PreferencesPACSServerWidget::CreatePACSServerListLayout()
{
	QVBoxLayout* server_list_layout = new QVBoxLayout();
	{
		QVBoxLayout* table_widget_layout = new QVBoxLayout();
		{
			table_widget_layout->setSpacing(kSpacing10);
			table_widget_layout->setContentsMargins(kStepMargins);

			pacs_server_table_widget_ = new QTableWidget(this);
			pacs_server_table_widget_->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());
			pacs_server_table_widget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
			pacs_server_table_widget_->setSelectionMode(QAbstractItemView::SingleSelection);
			pacs_server_table_widget_->setSelectionBehavior(QAbstractItemView::SelectRows);

			QStringList header_list;
			header_list << "Nickname" << "AE_Title" << "Server_IP" << "Port";
			pacs_server_table_widget_->setColumnCount(4);
			pacs_server_table_widget_->setHorizontalHeaderLabels(header_list);

			QHeaderView* header = pacs_server_table_widget_->horizontalHeader();
			header->setSectionsClickable(false);

			table_widget_layout->addWidget(pacs_server_table_widget_);
		}

		QHBoxLayout* button_layout = new QHBoxLayout();
		{
			button_layout->setSpacing(kSpacing10);
			button_layout->setContentsMargins(kStepMargins);
			button_layout->setAlignment(Qt::AlignBottom | Qt::AlignRight);

			QToolButton* select_button = CreateTextToolButton(lang::LanguagePack::txt_select());
			QToolButton* add_button = CreateTextToolButton(lang::LanguagePack::txt_add());
			QToolButton* delete_button = CreateTextToolButton(lang::LanguagePack::txt_delete());

			connect(select_button, &QToolButton::clicked, this, &PreferencesPACSServerWidget::slotPACSServerSelectButton);
			connect(add_button, &QToolButton::clicked, this, &PreferencesPACSServerWidget::slotPACSServerAddButton);
			connect(delete_button, &QToolButton::clicked, this, &PreferencesPACSServerWidget::slotPACSServerDeleteButton);

			button_layout->addWidget(select_button);
			button_layout->addWidget(add_button);
			button_layout->addWidget(delete_button);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_pacs_server_list(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		server_list_layout->addWidget(CreateHorizontalLine());
		server_list_layout->addWidget(title);
		server_list_layout->addLayout(table_widget_layout);
		server_list_layout->addLayout(button_layout);
	}

	return server_list_layout;
}

QVBoxLayout* PreferencesPACSServerWidget::CreateSelectedPACSServerLayout()
{
	QVBoxLayout* selected_server_layout = new QVBoxLayout();
	{
		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			contents_layout->setSpacing(kSpacing10);
			contents_layout->setContentsMargins(kStepMargins);

			QVBoxLayout* nickname_layout = new QVBoxLayout();
			{
				QLabel* label_caption = CreateLabel("Nickname", QSizePolicy::Preferred, QSizePolicy::Fixed, Qt::AlignCenter);

				nickname_line_edit_ = new QLineEdit(this);
				nickname_line_edit_->setAlignment(Qt::AlignCenter);
				nickname_line_edit_->setReadOnly(true);

				nickname_layout->addWidget(label_caption);
				nickname_layout->addWidget(nickname_line_edit_);
			}

			QVBoxLayout* server_ae_title_layout = new QVBoxLayout();
			{
				QLabel* label_caption = CreateLabel("AE_Title", QSizePolicy::Preferred, QSizePolicy::Fixed, Qt::AlignCenter);

				server_ae_title_line_edit_ = new QLineEdit(this);
				server_ae_title_line_edit_->setAlignment(Qt::AlignCenter);
				server_ae_title_line_edit_->setReadOnly(true);

				server_ae_title_layout->addWidget(label_caption);
				server_ae_title_layout->addWidget(server_ae_title_line_edit_);
			}

			QVBoxLayout* server_ip_layout = new QVBoxLayout();
			{
				QLabel* label_caption = CreateLabel("Server_IP", QSizePolicy::Preferred, QSizePolicy::Fixed, Qt::AlignCenter);

				server_ip_line_edit_ = new QLineEdit(this);
				server_ip_line_edit_->setAlignment(Qt::AlignCenter);
				server_ip_line_edit_->setReadOnly(true);

				server_ip_layout->addWidget(label_caption);
				server_ip_layout->addWidget(server_ip_line_edit_);
			}

			QVBoxLayout* port_layout = new QVBoxLayout();
			{
				QLabel* label_caption = CreateLabel("Port", QSizePolicy::Preferred, QSizePolicy::Fixed, Qt::AlignCenter);

				port_line_edit_ = new QLineEdit(this);
				port_line_edit_->setAlignment(Qt::AlignCenter);
				port_line_edit_->setReadOnly(true);

				port_layout->addWidget(label_caption);
				port_layout->addWidget(port_line_edit_);
			}

			contents_layout->addLayout(nickname_layout);
			contents_layout->addLayout(server_ae_title_layout);
			contents_layout->addLayout(server_ip_layout);
			contents_layout->addLayout(port_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_selected_pacs_server(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		selected_server_layout->addWidget(CreateHorizontalLine());
		selected_server_layout->addWidget(title);
		selected_server_layout->addLayout(contents_layout);
	}

	return selected_server_layout;
}

void PreferencesPACSServerWidget::SetPacsTableWidget()
{
	const GlobalPreferences::PACSServerList& pacs = GlobalPreferences::GetInstance()->preferences_.pacs_server_list;
	if (pacs.server_list.empty())
	{
		return;
	}

	int size = pacs.server_list.size();
	for (int i = 0; i < size; ++i)
	{
		QTableWidgetItem* nickname = new QTableWidgetItem(pacs.server_list[i].nickname);
		nickname->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem* ae_title = new QTableWidgetItem(pacs.server_list[i].ae_title);
		ae_title->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem* ip_address = new QTableWidgetItem(pacs.server_list[i].ip_address);
		ip_address->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem* port = new QTableWidgetItem(pacs.server_list[i].port);
		port->setTextAlignment(Qt::AlignCenter);

		pacs_server_table_widget_->insertRow(i);
		pacs_server_table_widget_->setItem(i, 0, nickname);
		pacs_server_table_widget_->setItem(i, 1, ae_title);
		pacs_server_table_widget_->setItem(i, 2, ip_address);
		pacs_server_table_widget_->setItem(i, 3, port);
	}

	if (pacs.select_num != -1)
	{
		pacs_server_table_widget_->selectRow(pacs.select_num);
		SetSelectedPACSServerInfo(pacs.select_num);
	}
}

void PreferencesPACSServerWidget::SetSelectedPACSServerInfo(const int index)
{
	if (index == -1)
	{
		return;
	}

	nickname_line_edit_->setText(pacs_server_table_widget_->item(index, 0)->text());
	server_ae_title_line_edit_->setText(pacs_server_table_widget_->item(index, 1)->text());
	server_ip_line_edit_->setText(pacs_server_table_widget_->item(index, 2)->text());
	port_line_edit_->setText(pacs_server_table_widget_->item(index, 3)->text());

	GlobalPreferences::GetInstance()->SelectPACSServer(index);
}
