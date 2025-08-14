#include "implant_list_dialog.h"

#include <QLabel>
#include <QToolButton>
#include <QHeaderView>
#include <qboxlayout.h>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"

#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"

namespace
{
	const int kDlgWidth = 500;
	const int kDlgHeight = 240;
	const int kSectionHeight = 20;
} // end of namespace

ImplantListDlg::ImplantListDlg(const QString& patient_id, QWidget *parent)
	: CW3Dialog(lang::LanguagePack::txt_implant_list() + " : " + patient_id, parent),
	close_button_(new QToolButton), implant_list_(new QTableWidget(0, 5, this))
{
	InitUI();
	Connections();
}

ImplantListDlg::~ImplantListDlg()
{
}

void ImplantListDlg::slotClose()
{
	Disconnections();
	reject();
}

void ImplantListDlg::closeEvent(QCloseEvent * e)
{
	slotClose();
}

void ImplantListDlg::InitUI()
{
	setFixedSize(kDlgWidth, kDlgHeight);
	close_button_->setText(lang::LanguagePack::txt_close());

	QHBoxLayout* button_layout = new QHBoxLayout;
	button_layout->setAlignment(Qt::AlignCenter);
	button_layout->setContentsMargins(0, 0, 0, 0);
	button_layout->setSpacing(0);
	button_layout->addWidget(close_button_);

	m_contentLayout->setContentsMargins(9, 9, 9, 9);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addWidget(implant_list_);
	m_contentLayout->addLayout(button_layout);

	implant_list_->verticalHeader()->setVisible(false);
	implant_list_->setMouseTracking(true);
	implant_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	implant_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	implant_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	implant_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	implant_list_->setShowGrid(false);

	QStringList product_header_labels = {
		tr("  ID"),
		"  " + lang::LanguagePack::txt_diameter(),
		"  " + lang::LanguagePack::txt_length(),
		"  " + lang::LanguagePack::txt_manufacturer(),
		"  " + lang::LanguagePack::txt_product(),
	};
	implant_list_->setHorizontalHeaderLabels(product_header_labels);

	QHeaderView *header = implant_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->resizeSection(0, 48);
	header->resizeSection(1, 82);
	header->resizeSection(2, 82);
	header->resizeSection(3, 105);
	header->setStretchLastSection(true);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	UpdateList();
}

void ImplantListDlg::UpdateList()
{
	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_data = implant_resource.data();

	implant_list_->setRowCount(implant_data.size());
	int selected_id = implant_resource.selected_implant_id();
	int row_index = 0;
	int curr_product_index = 0;
	for (const auto& data : implant_data)
	{
		int curr_id = data.second->id();
		if (selected_id == curr_id)
		{
			curr_product_index = row_index;
		}

		QTableWidgetItem *id = new QTableWidgetItem(QString::number(curr_id));
		id->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(row_index, 0, id);

		QTableWidgetItem* dia = new QTableWidgetItem(QString::number(data.second->diameter()));
		dia->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(row_index, 1, dia);

		QTableWidgetItem* len = new QTableWidgetItem(QString::number(data.second->length()));
		len->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(row_index, 2, len);

		QTableWidgetItem* man = new QTableWidgetItem(data.second->manufacturer());
		man->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(row_index, 3, man);

		QTableWidgetItem* pro = new QTableWidgetItem(data.second->product());
		pro->setTextAlignment(Qt::AlignCenter);
		implant_list_->setItem(row_index, 4, pro);

		implant_list_->setRowHeight(row_index, kSectionHeight);
		++row_index;
	}

	if (selected_id < 0)
	{
		implant_list_->setCurrentCell(0, 0);
	}
	else
	{
		implant_list_->setCurrentCell(curr_product_index, 0);
	}
}

void ImplantListDlg::Connections()
{
	connect(implant_list_, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(slotImplantSelected(int, int, int, int)));

	connect(close_button_, SIGNAL(clicked()), this, SLOT(slotClose()));
}

void ImplantListDlg::Disconnections()
{
	disconnect(implant_list_, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(slotImplantSelected(int, int, int, int)));

	disconnect(close_button_, SIGNAL(clicked()), this, SLOT(slotClose()));
}

void ImplantListDlg::slotImplantSelected(int currentRow, int currentColumn,
	int previousRow, int previousColumn)
{
	if (currentRow == previousRow)
	{
		return;
	}

	auto id_item = implant_list_->item(currentRow, 0);
	int id = id_item->data(Qt::DisplayRole).toInt();
	emit sigSelectImplant(id);
}
