/*=========================================================================

File:			class CW3ImplantDBDlg
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Sang Keun Park, Seok Man Seo
First date:		2016-05-25
Last modify:	2018-04-02

comment:
임플란트 폴더로 부터 정보를 읽어서 DB에 작성하는 프로그램.
=========================================================================*/
#include "W3ImplantDBDlg.h"

#include <QtSql/qsqlqueryModel.h>
#include <QtSql/qsqlrecord.h>
#include <qboxlayout.h>
#include <QHeaderView>
#include <QLabel>
#include <QSQLError>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QDirIterator>
#include <QTabWidget>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"
#include "../UIComponent/W3View3DImplantPreview.h"
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/UIModule/UIFrame/custom_implant_dialog.h>
#include "../../Resource/Resource/implant_resource.h"

#define SIMPLE_CUSTOM_PRODUCT_HEADER 1
#if _DEBUG
#define ENABLE_ADD_IMPLANT 0
#else
#define ENABLE_ADD_IMPLANT 0
#endif

using namespace implant_resource;

namespace
{
	const int kCustomButtonWidth = 100;
	const int kCommonButtonWidth = 170;
	const int kDlgWidth = 820;
	const int kDlgHeight = 500;
	const int kSectionHeight = 23;
	const QString kConnectionName("ImplantDBDlg");
	const QString kFavoriteDBName("Implant_Favorites");
	const QString kFavoriteTempDBName("Implant_Favorites_temp");
}  // end of namespace

CW3ImplantDBDlg::CW3ImplantDBDlg(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer,
	QWidget *pParent)
	: CW3Dialog(lang::LanguagePack::txt_implant_library(), pParent)
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool enable_custom = settings.value("IMPLANT/enable_custom", false).toBool();

	setFixedSize(kDlgWidth, kDlgHeight);

	general_preview_ = new CW3View3DImplantPreview(VREngine, MPRengine, Rcontainer, common::ViewTypeID::IMPLANT_PREVIEW);

#if ENABLE_ADD_IMPLANT
	enable_custom = false;
#endif

	if (!enable_custom)
	{
		general_manufacturer_list_ = new QTableWidget(0, 2, this);
		general_product_list_ = new QTableWidget(0, 5, this);
		general_implant_list_ = new QTableWidget(0, 4, this);

		// Control Button
		apply_button_ = new QToolButton();
		cancel_button_ = new QToolButton();
		apply_button_->setText(lang::LanguagePack::txt_apply());
		apply_button_->setFixedWidth(kCommonButtonWidth);
		cancel_button_->setText(lang::LanguagePack::txt_cancel());
		cancel_button_->setFixedWidth(kCommonButtonWidth);

#if ENABLE_ADD_IMPLANT
		add_button_ = new QToolButton();
		add_button_->setText("Add");
		add_button_->setFixedWidth(kCommonButtonWidth);
#endif

		// Layout setting
		QHBoxLayout *rb_layout = new QHBoxLayout;
		rb_layout->setSpacing(10);
		rb_layout->addWidget(general_implant_list_);
		rb_layout->setStretch(0, 2);
		rb_layout->addWidget(general_preview_);
		rb_layout->setStretch(1, 1);

		QVBoxLayout *right_layout = new QVBoxLayout;
		right_layout->setSpacing(10);
		right_layout->addWidget(general_product_list_);
		right_layout->setStretch(0, 1);
		right_layout->addLayout(rb_layout);
		right_layout->setStretch(1, 1);

		QHBoxLayout *central_layout = new QHBoxLayout;
		central_layout->addWidget(general_manufacturer_list_);
		central_layout->setStretch(0, 1);
		central_layout->addLayout(right_layout);
		central_layout->setStretch(1, 3);

		QHBoxLayout *button_layout = new QHBoxLayout;
		button_layout->setAlignment(Qt::AlignCenter);
		button_layout->setSpacing(10);
		button_layout->setContentsMargins(0, 0, 0, 0);
		button_layout->addWidget(apply_button_);
		button_layout->addWidget(cancel_button_);

#if ENABLE_ADD_IMPLANT
		button_layout->addWidget(add_button_);
#endif

		m_contentLayout->setContentsMargins(9, 9, 9, 9);
		m_contentLayout->setSpacing(10);
		m_contentLayout->addLayout(central_layout);
		m_contentLayout->addLayout(button_layout);

		InitGeneralTables();
	}
	else
	{
		//custom_preview_ = new CW3View3DImplantPreview(VREngine, MPRengine, Rcontainer, common::ViewTypeID::IMPLANT_PREVIEW);

		SetLayout();
	}

	connections();
	connectDB();

	InitFavoriteDatabase();

	InitGeneralData();
	if (enable_custom)
	{
		InitCustomData();
	}
}

CW3ImplantDBDlg::~CW3ImplantDBDlg()
{
	disConnectDB();
}

void CW3ImplantDBDlg::InitFavoriteDatabase()
{
	if (!existDBTable_favorites(kFavoriteDBName) ||
		!existDBTable_favorites(kFavoriteTempDBName))
	{
		QSqlDatabase db = QSqlDatabase::database(kConnectionName);
		db.open();
		createDBTable_favorites(kFavoriteDBName);
		createDBTable_favorites(kFavoriteTempDBName);
		updateDBTable_favorites();
		db.close();
	}
}

void CW3ImplantDBDlg::InitGeneralData()
{
	disconnections();
	UpdateManufacturerList();
	UpdateProductList();
	UpdateImplantList();
	connections();

#if _DEBUG
	UpdateProductDBTable();
#endif
}

void CW3ImplantDBDlg::InitCustomData()
{
	disconnections();
	UpdateCustomManufacturerList();
	UpdateCustomProductList();
	UpdateCustomImplantList();
	connections();
}

//입력되어 있는 DB정보를 이용하여 Manufacture list, Product table, implant
//table을 업데이트
void CW3ImplantDBDlg::InitGeneralTables()
{
	////////////////////////////////////////////////////////
	// Manufacturer List
	general_manufacturer_list_->verticalHeader()->setVisible(false);
	general_manufacturer_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	general_manufacturer_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	general_manufacturer_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	general_manufacturer_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	general_manufacturer_list_->setShowGrid(false);

	QStringList manufacturer_header_label = {
		"  " + lang::LanguagePack::txt_manufacturer(),
		QString("id")
	};
	general_manufacturer_list_->setHorizontalHeaderLabels(manufacturer_header_label);
	general_manufacturer_list_->setColumnHidden(1, true);

	QHeaderView *header = general_manufacturer_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	////////////////////////////////////////////////////////
	// Product Table
	general_product_list_->verticalHeader()->setVisible(false);
	general_product_list_->setMouseTracking(true);
	general_product_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	general_product_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	general_product_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	general_product_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	general_product_list_->setShowGrid(false);

	QStringList product_header_labels = {
		"  " + lang::LanguagePack::txt_product_line(),
		"  " + lang::LanguagePack::txt_diameter(),
		"  " + lang::LanguagePack::txt_length(),
		"  " + lang::LanguagePack::txt_implant(),
		QString("id")
	};
	general_product_list_->setHorizontalHeaderLabels(product_header_labels);
	general_product_list_->setColumnHidden(4, true);

	header = general_product_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->resizeSection(1, 82);
	header->resizeSection(2, 82);
	header->resizeSection(3, 70);
	header->setStretchLastSection(false);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	////////////////////////////////////////////////////////
	// Implant Table
	general_implant_list_->verticalHeader()->setVisible(false);
	general_implant_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	general_implant_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	general_implant_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	general_implant_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	general_implant_list_->setShowGrid(false);

	header = general_implant_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->resizeSection(1, 82);
	header->resizeSection(2, 82);
	header->setStretchLastSection(false);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	QStringList implant_header_labels = {
		"  " + lang::LanguagePack::txt_model(),
		"  " + lang::LanguagePack::txt_diameter(),
		"  " + lang::LanguagePack::txt_length(),
		QString("id")
	};
	general_implant_list_->setHorizontalHeaderLabels(implant_header_labels);
	general_implant_list_->setColumnHidden(3, true);
}

void CW3ImplantDBDlg::InitCustomTables()
{
	////////////////////////////////////////////////////////
	// Manufacturer List
	custom_manufacturer_list_->verticalHeader()->setVisible(false);
	custom_manufacturer_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	custom_manufacturer_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	custom_manufacturer_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	custom_manufacturer_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	custom_manufacturer_list_->setShowGrid(false);

	QStringList manufacturer_header_label = {
		"  " + lang::LanguagePack::txt_manufacturer(),
		QString("id")
	};
	custom_manufacturer_list_->setHorizontalHeaderLabels(manufacturer_header_label);
	custom_manufacturer_list_->setColumnHidden(1, true);

	QHeaderView* header = custom_manufacturer_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	////////////////////////////////////////////////////////
	// Product Table
	custom_product_list_->verticalHeader()->setVisible(false);
	custom_product_list_->setMouseTracking(true);
	custom_product_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	custom_product_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	custom_product_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	custom_product_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	custom_product_list_->setShowGrid(false);

#if !SIMPLE_CUSTOM_PRODUCT_HEADER
	QStringList product_header_labels = {
		"  " + lang::LanguagePack::txt_product_line(),
		"  " + lang::LanguagePack::txt_diameter(),
		"  " + lang::LanguagePack::txt_length(),
		"  " + lang::LanguagePack::txt_implant(),
		QString("id")
	};
	custom_product_list_->setHorizontalHeaderLabels(product_header_labels);
	custom_product_list_->setColumnHidden(4, true);

	header = custom_product_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(QHeaderView::Interactive);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->resizeSection(1, 82);
	header->resizeSection(2, 82);
	header->resizeSection(3, 70);
	header->setStretchLastSection(false);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());
#else
	QStringList product_header_labels = {
		"  " + lang::LanguagePack::txt_product_line(),
		QString("id")
	};
	custom_product_list_->setHorizontalHeaderLabels(product_header_labels);
	custom_product_list_->setColumnHidden(1, true);

	header = custom_product_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(QHeaderView::Interactive);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->setStretchLastSection(false);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());
#endif
	////////////////////////////////////////////////////////
	// Implant Table
	custom_implant_list_->verticalHeader()->setVisible(false);
	custom_implant_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	custom_implant_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	custom_implant_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	custom_implant_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	custom_implant_list_->setShowGrid(false);

	header = custom_implant_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(QHeaderView::Interactive);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->resizeSection(1, 125);
	header->resizeSection(2, 120);
	header->resizeSection(3, 82);
	header->resizeSection(4, 70);
	header->setStretchLastSection(false);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	QStringList implant_header_labels = {
		"  " + lang::LanguagePack::txt_model(),
		"  " + lang::LanguagePack::txt_coronal_diameter(),
		"  " + lang::LanguagePack::txt_apical_diameter(),
		"  " + lang::LanguagePack::txt_length(),
		"  " + lang::LanguagePack::txt_type(),
		QString("id")
	};
	custom_implant_list_->setHorizontalHeaderLabels(implant_header_labels);
	custom_implant_list_->setColumnHidden(5, true);
}

void CW3ImplantDBDlg::SetLayout()
{
	m_contentLayout->setContentsMargins(9, 9, 9, 9);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addWidget(CreateTabWidget());
	m_contentLayout->addLayout(CreateButtonLayout());
}

QTabWidget* CW3ImplantDBDlg::CreateTabWidget()
{
	QTabWidget* tab_widget = new QTabWidget();
	QWidget* general_tab = new QWidget();
	QWidget* custom_tab = new QWidget();

	tab_widget->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogTabStyleSheet());

	general_tab->setLayout(CreateGeneralLayout());
	custom_tab->setLayout(CreateCustomLayout());

	tab_widget->insertTab(static_cast<int>(Tab::GENERAL), general_tab, lang::LanguagePack::txt_general());
	tab_widget->insertTab(static_cast<int>(Tab::CUSTOM), custom_tab, lang::LanguagePack::txt_custom());

	tab_widget->setCurrentIndex(0);
	current_tab_ = Tab::GENERAL;

	connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(slotTabChanged(int)));

	return tab_widget;
}

QHBoxLayout* CW3ImplantDBDlg::CreateButtonLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(10);
	layout->setAlignment(Qt::AlignCenter);

	custom_add_button_ = new QToolButton();
	custom_modify_button_ = new QToolButton();
	custom_remove_button_ = new QToolButton();

	apply_button_ = new QToolButton();
	cancel_button_ = new QToolButton();

	custom_add_button_->setText(lang::LanguagePack::txt_add());
	custom_add_button_->setFixedWidth(kCustomButtonWidth);
	custom_modify_button_->setText(lang::LanguagePack::txt_modify());
	custom_modify_button_->setFixedWidth(kCustomButtonWidth);
	custom_remove_button_->setText(lang::LanguagePack::txt_remove());
	custom_remove_button_->setFixedWidth(kCustomButtonWidth);

	apply_button_->setText(lang::LanguagePack::txt_apply());
	apply_button_->setFixedWidth(kCommonButtonWidth);
	cancel_button_->setText(lang::LanguagePack::txt_cancel());
	cancel_button_->setFixedWidth(kCommonButtonWidth);

	QHBoxLayout* custom_layout = new QHBoxLayout();
	QHBoxLayout* common_layout = new QHBoxLayout();

	custom_layout->setAlignment(Qt::AlignLeft);
	common_layout->setAlignment(Qt::AlignRight);

	custom_layout->addWidget(custom_add_button_);
	custom_layout->addWidget(custom_modify_button_);
	custom_layout->addWidget(custom_remove_button_);

	common_layout->addWidget(apply_button_);
	common_layout->addWidget(cancel_button_);

	layout->addLayout(custom_layout, 1);
	layout->addLayout(common_layout, 1);

	SetButtonMode(current_tab_);

	return layout;
}

QHBoxLayout* CW3ImplantDBDlg::CreateGeneralLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();

	general_manufacturer_list_ = new QTableWidget(0, 2, this);
	general_product_list_ = new QTableWidget(0, 5, this);
	general_implant_list_ = new QTableWidget(0, 4, this);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->setSpacing(10);
	bottom_layout->addWidget(general_implant_list_);
	bottom_layout->setStretch(0, 2);
	bottom_layout->addWidget(general_preview_);
	bottom_layout->setStretch(1, 1);

	QVBoxLayout* right_layout = new QVBoxLayout();
	right_layout->setSpacing(10);
	right_layout->addWidget(general_product_list_);
	right_layout->setStretch(0, 1);
	right_layout->addLayout(bottom_layout);
	right_layout->setStretch(1, 1);

	layout->addWidget(general_manufacturer_list_);
	layout->setStretch(0, 1);
	layout->addLayout(right_layout);
	layout->setStretch(1, 3);

	InitGeneralTables();

	return layout;
}

QHBoxLayout* CW3ImplantDBDlg::CreateCustomLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();

	custom_manufacturer_list_ = new QTableWidget(0, 2, this);
#if !SIMPLE_CUSTOM_PRODUCT_HEADER
	custom_product_list_ = new QTableWidget(0, 5, this);
#else
	custom_product_list_ = new QTableWidget(0, 2, this);
#endif
	custom_implant_list_ = new QTableWidget(0, 6, this);

	QHBoxLayout* bottom_layout = new QHBoxLayout();
	bottom_layout->setSpacing(10);
	bottom_layout->addWidget(custom_implant_list_);
	/*bottom_layout->setStretch(0, 2);
	bottom_layout->addWidget(custom_preview_);
	bottom_layout->setStretch(1, 1);*/

	QVBoxLayout* right_layout = new QVBoxLayout();
	right_layout->setSpacing(10);
	right_layout->addWidget(custom_product_list_);
	right_layout->setStretch(0, 1);
	right_layout->addLayout(bottom_layout);
	right_layout->setStretch(1, 1);

	layout->addWidget(custom_manufacturer_list_);
	layout->setStretch(0, 1);
	layout->addLayout(right_layout);
	layout->setStretch(1, 3);

	InitCustomTables();

	return layout;
}

void CW3ImplantDBDlg::SetButtonMode(Tab mode)
{
	if (!custom_add_button_ ||
		!custom_remove_button_ ||
		!custom_modify_button_)
	{
		return;
	}

	bool custom_button_visible = current_tab_ == Tab::CUSTOM;
	custom_add_button_->setVisible(custom_button_visible);
	custom_modify_button_->setVisible(custom_button_visible);
	custom_remove_button_->setVisible(custom_button_visible);
}

// SIGNAL & SLOT connect
void CW3ImplantDBDlg::connections()
{
	connect(general_manufacturer_list_, &QTableWidget::currentCellChanged, this,
		&CW3ImplantDBDlg::slotUpdateGeneralProductTable);
	connect(general_manufacturer_list_, &QTableWidget::itemChanged, this,
		&CW3ImplantDBDlg::slotGeneralManufacturerItemChecked);

	connect(general_product_list_, &QTableWidget::currentCellChanged, this,
		&CW3ImplantDBDlg::slotUpdateGeneralImplantTable);
	connect(general_product_list_, &QTableWidget::itemChanged, this,
		&CW3ImplantDBDlg::slotGeneralProductItemChecked);

	connect(general_implant_list_, &QTableWidget::cellClicked, this,
		&CW3ImplantDBDlg::slotClickedImplantTable);

	connect(apply_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotApply);
	connect(cancel_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotCancel);
#if ENABLE_ADD_IMPLANT
	connect(add_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotAdd);
#endif

	if (custom_manufacturer_list_)
	{
		connect(custom_manufacturer_list_, &QTableWidget::cellClicked, this, &CW3ImplantDBDlg::slotCustomManufacturerCellClicked);
		connect(custom_manufacturer_list_, &QTableWidget::currentCellChanged, this, &CW3ImplantDBDlg::slotUpdateCustomProductTable);
		connect(custom_manufacturer_list_, &QTableWidget::itemChanged, this, &CW3ImplantDBDlg::slotCustomManufacturerItemChecked);
	}

	if (custom_product_list_)
	{
		connect(custom_product_list_, &QTableWidget::cellClicked, this, &CW3ImplantDBDlg::slotCustomProductCellClicked);
		connect(custom_product_list_, &QTableWidget::currentCellChanged, this, &CW3ImplantDBDlg::slotUpdateCustomImplantTable);
		connect(custom_product_list_, &QTableWidget::itemChanged, this, &CW3ImplantDBDlg::slotCustomProductItemChecked);
	}

	if (custom_implant_list_)
	{
		connect(custom_implant_list_, &QTableWidget::cellClicked, this, &CW3ImplantDBDlg::slotCustomImplantCellClicked);
	}

	if (custom_add_button_)
	{
		connect(custom_add_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotAddCustomImplantButtonClicked);
	}

	if (custom_modify_button_)
	{
		connect(custom_modify_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotModifyCustomImplantButtonClicked);
	}

	if (custom_remove_button_)
	{
		connect(custom_remove_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotRemoveCustomImplantButtonClicked);
	}
}

// SIGNAL & SLOT disconnect
void CW3ImplantDBDlg::disconnections()
{
	disconnect(general_manufacturer_list_, &QTableWidget::currentCellChanged, this,
		&CW3ImplantDBDlg::slotUpdateGeneralProductTable);
	disconnect(general_manufacturer_list_, &QTableWidget::itemChanged, this,
		&CW3ImplantDBDlg::slotGeneralManufacturerItemChecked);

	disconnect(general_product_list_, &QTableWidget::currentCellChanged, this,
		&CW3ImplantDBDlg::slotUpdateGeneralImplantTable);
	disconnect(general_product_list_, &QTableWidget::itemChanged, this,
		&CW3ImplantDBDlg::slotGeneralProductItemChecked);

	disconnect(general_implant_list_, &QTableWidget::cellClicked, this,
		&CW3ImplantDBDlg::slotClickedImplantTable);

	disconnect(apply_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotApply);
	disconnect(cancel_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotCancel);
#if ENABLE_ADD_IMPLANT
	disconnect(add_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotAdd);
#endif

	if (custom_manufacturer_list_)
	{
		disconnect(custom_manufacturer_list_, &QTableWidget::currentCellChanged, this, &CW3ImplantDBDlg::slotUpdateCustomProductTable);
		disconnect(custom_manufacturer_list_, &QTableWidget::itemChanged, this, &CW3ImplantDBDlg::slotCustomManufacturerItemChecked);
	}

	if (custom_product_list_)
	{
		disconnect(custom_product_list_, &QTableWidget::currentCellChanged, this, &CW3ImplantDBDlg::slotUpdateCustomImplantTable);
		disconnect(custom_product_list_, &QTableWidget::itemChanged, this, &CW3ImplantDBDlg::slotCustomProductItemChecked);
	}

	if (custom_add_button_)
	{
		disconnect(custom_add_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotAddCustomImplantButtonClicked);
	}

	if (custom_modify_button_)
	{
		disconnect(custom_modify_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotModifyCustomImplantButtonClicked);
	}

	if (custom_remove_button_)
	{
		disconnect(custom_remove_button_, &QToolButton::clicked, this, &CW3ImplantDBDlg::slotRemoveCustomImplantButtonClicked);
	}
}

// Manufacturer table를 업데이트
void CW3ImplantDBDlg::UpdateManufacturerList(
	const QString &selected_manufacturer_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery("SELECT * FROM Manufacturer2 ORDER BY Manufacturer_name ASC", db);

	int row_count = qm.rowCount();
	general_manufacturer_list_->setRowCount(
		row_count);  // product table의 row 갯수를 지정해준다.

	int curr_manufacturer_index = 0;
	for (int i = 0; i < row_count; i++)
	{
		// Manufacturer name
		QString manufacturer_name =
			qm.record(i).value("Manufacturer_name").toString();
		QTableWidgetItem *manufacturer_item =
			new QTableWidgetItem(manufacturer_name);

		if (existDBRecord_favorites(manufacturer_name))
			manufacturer_item->setCheckState(Qt::Checked);
		else
			manufacturer_item->setCheckState(Qt::Unchecked);

		general_manufacturer_list_->setItem(i, 0, manufacturer_item);

		if (selected_manufacturer_name == manufacturer_name)
			curr_manufacturer_index = i;

		// Manufacturer id
		general_manufacturer_list_->setItem(
			i, 1,
			new QTableWidgetItem(qm.record(i).value("Manufacturer_id").toString()));

		general_manufacturer_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (selected_manufacturer_name == "")
		general_manufacturer_list_->setCurrentCell(0, 0);
	else
		general_manufacturer_list_->setCurrentCell(curr_manufacturer_index, 0);

	common::Logger::instance()->Print(common::LogType::INF,
		"Update Manufacturer List");
}

// manufacturer list의 인덱스를 입력받아 product table를 업데이트
void CW3ImplantDBDlg::UpdateProductList(const QString &selected_product_name)
{
	QString manufacturer_id =
		general_manufacturer_list_->item(general_manufacturer_list_->currentRow(), 1)->text();
	QString manufacturer_name =
		general_manufacturer_list_->item(general_manufacturer_list_->currentRow(), 0)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(QString("SELECT * FROM Product2 WHERE Manufacturer_id='%1'"
		" ORDER BY Product_name ASC")
		.arg(manufacturer_id),
		db);

	int row_count = qm.rowCount();
	general_product_list_->setRowCount(
		row_count);  // product table의 row 갯수를 지정해준다.

	int curr_product_index = 0;
	for (int i = 0; i < row_count; i++)
	{
		// Product Name
		QString product_name = qm.record(i).value("Product_name").toString();
		QTableWidgetItem *product_name_item = new QTableWidgetItem(product_name);

		if (existDBRecord_favorites(manufacturer_name, product_name))
			product_name_item->setCheckState(Qt::Checked);
		else
			product_name_item->setCheckState(Qt::Unchecked);

		general_product_list_->setItem(i, 0, product_name_item);

		if (selected_product_name == product_name) curr_product_index = i;

		// Product Implant Diameter Min~Max Range
		QString Product_diameter =
			qm.record(i).value("Product_implantDiameterMin").toString() +
			QString("~") +
			qm.record(i).value("Product_implantDiameterMax").toString();
		general_product_list_->setItem(i, 1, new QTableWidgetItem(Product_diameter));

		// Product Implant Length Min~Max Range
		QString Product_length =
			qm.record(i).value("Product_implantLengthMin").toString() +
			QString("~") +
			qm.record(i).value("Product_implantLengthMax").toString();
		general_product_list_->setItem(i, 2, new QTableWidgetItem(Product_length));

		// Product implant count
		general_product_list_->setItem(
			i, 3,
			new QTableWidgetItem(
				qm.record(i).value("Product_implantCount").toString()));

		// Product id
		general_product_list_->setItem(
			i, 4,
			new QTableWidgetItem(qm.record(i).value("Product_id").toString()));

		general_product_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (selected_product_name == "")
		general_product_list_->setCurrentCell(0, 0);
	else
		general_product_list_->setCurrentCell(curr_product_index, 0);

	common::Logger::instance()->Print(common::LogType::INF,
		"Update Product List");
}

// Product table의 인덱스를 입력받아 Implant table를 업데이트
void CW3ImplantDBDlg::UpdateImplantList()
{
	QString product_id =
		general_product_list_->item(general_product_list_->currentRow(), 4)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(QString("SELECT * FROM Implants2 WHERE Product_id='%1'"
		" ORDER BY Implant_diameter ASC, implant_length ASC, sub_category ASC")
		.arg(product_id),
		db);

	int rowCountImplant = qm.rowCount();

	general_implant_list_->setRowCount(rowCountImplant);  // implant table의 row 갯수를 지정해준다.
	if (rowCountImplant < 1)
	{
		db.close();
		return;
	}

	for (int i = 0; i < rowCountImplant; i++)
	{
		const QString implant_name = qm.record(i).value("Implant_name").toString();
		const QString implant_path = qm.record(i).value("Implant_path").toString();
		QTableWidgetItem *item_name = new QTableWidgetItem(implant_name);
		if (implant_path != "")
			item_name->setIcon(QIcon(":/image/implantTrue.png"));
		else
			item_name->setIcon(QIcon(":/image/implantFalse.png"));

		general_implant_list_->setItem(i, 0, item_name);
		general_implant_list_->setItem(
			i, 1,
			new QTableWidgetItem(qm.record(i).value("Implant_diameter").toString() +
				"mm"));
		general_implant_list_->setItem(
			i, 2,
			new QTableWidgetItem(qm.record(i).value("Implant_length").toString() +
				"mm"));
		general_implant_list_->setItem(
			i, 3,
			new QTableWidgetItem(qm.record(i).value("Implant_id").toString()));
		general_implant_list_->setRowHeight(i, kSectionHeight);
	}
	general_implant_list_->setCurrentCell(0, 0);

	db.close();

	UpdatePreview();
	common::Logger::instance()->Print(common::LogType::INF,
		"Update Implant List");
}

void CW3ImplantDBDlg::UpdateCustomManufacturerList(const QString& selected_manufacturer_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery("SELECT * FROM custom_manufacturer ORDER BY Manufacturer_name ASC", db);

	int row_count = qm.rowCount();
	custom_manufacturer_list_->setRowCount(row_count);

	int curr_manufacturer_index = 0;
	for (int i = 0; i < row_count; i++)
	{
		// Manufacturer name
		QString manufacturer_name = qm.record(i).value("Manufacturer_name").toString();
		QTableWidgetItem* manufacturer_item = new QTableWidgetItem(manufacturer_name);

		if (existDBRecord_favorites(kCustomImplantManufacturerNamePrefix + manufacturer_name))
		{
			manufacturer_item->setCheckState(Qt::Checked);
		}
		else
		{
			manufacturer_item->setCheckState(Qt::Unchecked);
		}

		custom_manufacturer_list_->setItem(i, 0, manufacturer_item);

		if (selected_manufacturer_name == kCustomImplantManufacturerNamePrefix + manufacturer_name)
		{
			curr_manufacturer_index = i;
		}

		// Manufacturer id
		custom_manufacturer_list_->setItem(
			i,
			1,
			new QTableWidgetItem(qm.record(i).value("Manufacturer_id").toString())
		);

		custom_manufacturer_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (selected_manufacturer_name == "")
	{
		custom_manufacturer_list_->setCurrentCell(0, 0);
	}
	else
	{
		custom_manufacturer_list_->setCurrentCell(curr_manufacturer_index, 0);
	}

	common::Logger::instance()->Print(common::LogType::INF, "Update Custom Manufacturer List");
}

void CW3ImplantDBDlg::UpdateCustomProductList(const QString& selected_product_name)
{
	if (custom_manufacturer_list_->rowCount() < 1)
	{
		custom_product_list_->clearContents();
		return;
	}

	QString manufacturer_id = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 1)->text();
	QString manufacturer_name = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(
		QString("SELECT * FROM custom_product WHERE Manufacturer_id='%1'"
			" ORDER BY Product_name ASC")
		.arg(manufacturer_id),
		db
	);

	int row_count = qm.rowCount();
	custom_product_list_->setRowCount(row_count);

	int curr_product_index = 0;
	for (int i = 0; i < row_count; i++)
	{
		// Product Name
		QString product_name = qm.record(i).value("Product_name").toString();
		QTableWidgetItem* product_item = new QTableWidgetItem(product_name);

		if (existDBRecord_favorites(kCustomImplantManufacturerNamePrefix + manufacturer_name, product_name))
		{
			product_item->setCheckState(Qt::Checked);
		}
		else
		{
			product_item->setCheckState(Qt::Unchecked);
		}

		custom_product_list_->setItem(i, 0, product_item);

		if (selected_product_name == product_name)
		{
			curr_product_index = i;
		}

#if !SIMPLE_CUSTOM_PRODUCT_HEADER
		// Product Implant Diameter Min~Max Range
		QString Product_diameter =
			qm.record(i).value("Product_implantDiameterMin").toString() +
			QString("~") +
			qm.record(i).value("Product_implantDiameterMax").toString();
		custom_product_list_->setItem(i, 1, new QTableWidgetItem(Product_diameter));

		// Product Implant Length Min~Max Range
		QString Product_length =
			qm.record(i).value("Product_implantLengthMin").toString() +
			QString("~") +
			qm.record(i).value("Product_implantLengthMax").toString();
		custom_product_list_->setItem(i, 2, new QTableWidgetItem(Product_length));

		// Product implant count
		custom_product_list_->setItem(
			i,
			3,
			new QTableWidgetItem(qm.record(i).value("Product_implantCount").toString())
		);

		// Product id
		custom_product_list_->setItem(
			i,
			4,
			new QTableWidgetItem(qm.record(i).value("Product_id").toString())
		);
#else
		// Product id
		custom_product_list_->setItem(
			i,
			1,
			new QTableWidgetItem(qm.record(i).value("Product_id").toString())
		);
#endif
		custom_product_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (selected_product_name == "")
	{
		custom_product_list_->setCurrentCell(0, 0);
	}
	else
	{
		custom_product_list_->setCurrentCell(curr_product_index, 0);
	}

	common::Logger::instance()->Print(common::LogType::INF, "Update Custom Product List");
}

void CW3ImplantDBDlg::UpdateCustomImplantList()
{
	if (custom_product_list_->rowCount() < 1)
	{
		custom_implant_list_->clearContents();
		return;
	}

	int test = custom_product_list_->columnCount();
	QString product_id = custom_product_list_->item(custom_product_list_->currentRow(), custom_product_list_->columnCount() - 1)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(
		QString("SELECT * FROM custom_implants WHERE Product_id='%1'"
			" ORDER BY coronal_diameter ASC, length ASC")
		.arg(product_id),
		db
	);

	int row_count = qm.rowCount();

	custom_implant_list_->setRowCount(row_count);
	if (row_count < 1)
	{
		db.close();
		return;
	}

	for (int i = 0; i < row_count; i++)
	{
		const QString implant_name = qm.record(i).value("Implant_name").toString();
		QTableWidgetItem* implant_item = new QTableWidgetItem(implant_name);
		implant_item->setIcon(QIcon(":/image/implantTrue.png"));

		custom_implant_list_->setItem(i, 0, implant_item);
		custom_implant_list_->setItem(
			i,
			1,
			new QTableWidgetItem(qm.record(i).value("coronal_diameter").toString() + "mm")
		);
		custom_implant_list_->setItem(
			i,
			2,
			new QTableWidgetItem(qm.record(i).value("apical_diameter").toString() + "mm")
		);
		custom_implant_list_->setItem(
			i,
			3,
			new QTableWidgetItem(qm.record(i).value("length").toString() + "mm")
		);
		custom_implant_list_->setItem(
			i,
			4,
			new QTableWidgetItem(qm.record(i).value("type").toString())
		);
		custom_implant_list_->setItem(
			i,
			5,
			new QTableWidgetItem(qm.record(i).value("Implant_id").toString())
		);
		custom_implant_list_->setRowHeight(i, kSectionHeight);
	}
	custom_implant_list_->setCurrentCell(0, 0);

	db.close();

	common::Logger::instance()->Print(common::LogType::INF, "Update Custom Implant List");
}

// 현재 선택되어 있는 임플란트의 정보를 렌더링
void CW3ImplantDBDlg::UpdatePreview()
{
	QTableWidgetItem *itemManufacturer =
		general_manufacturer_list_->item(general_manufacturer_list_->currentRow(), 0);
	QString manufacturerProduct = itemManufacturer->text();

	QTableWidgetItem *itemProduct =
		general_product_list_->item(general_product_list_->currentRow(), 0);
	QString currentProduct = itemProduct->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QString implant_id =
		general_implant_list_->item(general_implant_list_->currentRow(), 3)->text();
	QSqlQueryModel qm;
	qm.setQuery(
		QString(
			"select Implant_path, Implant_diameter, Implant_length from implants2"
			" where Implant_id = '%1'")
		.arg(implant_id),
		db);
	QString currentImplantPath = qm.record(0).value("Implant_path").toString();
	float diameter = qm.record(0).value("Implant_diameter").toFloat();
	float length = qm.record(0).value("Implant_length").toFloat();
	db.close();

	/*if (diameter <= 0.0f || length <= 0.0f) return;*/

	general_preview_->setImplant(manufacturerProduct, currentProduct, currentImplantPath,
		diameter, length);
	common::Logger::instance()->Print(common::LogType::INF, "Update Preview");
}

// Manufacture List의 선택된 row가 변경되면(다른 Manufacturer) 실행되는 slot
void CW3ImplantDBDlg::slotUpdateGeneralProductTable(int currentRow, int currentColumn,
	int previousRow,
	int previousColumn)
{
	if (currentRow == previousRow) return;

	general_manufacturer_list_->setCurrentCell(currentRow, 0);

	disconnections();
	UpdateProductList();
	UpdateImplantList();
	connections();
}

// Product table의 선택된 row가 변경되면(다른 product) 실행되는 slot
void CW3ImplantDBDlg::slotUpdateGeneralImplantTable(int currentRow, int currentColumn,
	int previousRow,
	int previousColumn)
{
	if (currentRow == previousRow) return;

	general_product_list_->setCurrentCell(currentRow, 0);

	disconnections();
	UpdateImplantList();
	connections();
}


void CW3ImplantDBDlg::slotUpdateCustomProductTable(
	int current_row, 
	int current_column, 
	int previous_row, 
	int previous_column
)
{
	if (current_row == previous_row)
	{
		return;
	}

	custom_manufacturer_list_->setCurrentCell(current_row, 0);

	disconnections();
	UpdateCustomProductList();
	UpdateCustomImplantList();
	connections();
}

void CW3ImplantDBDlg::slotUpdateCustomImplantTable(
	int current_row, 
	int current_column, 
	int previous_row, 
	int previous_column
)
{
	if (current_row == previous_row)
	{
		return;
	}

	custom_product_list_->setCurrentCell(current_row, 0);

	disconnections();
	UpdateCustomImplantList();
	connections();
}

// Manufacturer table에서 선택된 Manufacturer의 check box 상태에 따라 즐겨찾기
// 임시DB 업데이트
void CW3ImplantDBDlg::slotGeneralManufacturerItemChecked(QTableWidgetItem *item)
{
	QString manufacturer_name = item->text();

	if (item->checkState() == Qt::Checked)
	{  // favorites db에 등록
		if (insert_favorites_temp(manufacturer_name))
		{
			disconnections();
			UpdateManufacturerList(manufacturer_name);
			UpdateProductList();
			UpdateImplantList();
			connections();
		}
	}
	else
	{  // favorites db에서 삭제
		QSqlDatabase::database(kConnectionName).open();
		bool exist = existDBRecord_favorites(manufacturer_name);
		QSqlDatabase::database(kConnectionName).close();

		if (exist)
		{
			delete_favorites_temp(manufacturer_name);

			disconnections();
			UpdateManufacturerList(manufacturer_name);
			UpdateProductList();
			UpdateImplantList();
			connections();
		}
	}
}

// Product table에서 선택된 product의 check box 상태에 따라 즐겨찾기 임시DB
// 업데이트
void CW3ImplantDBDlg::slotGeneralProductItemChecked(QTableWidgetItem *item)
{
	QString product_name = item->text();
	QString manufacturer_name =
		general_manufacturer_list_->item(general_manufacturer_list_->currentRow(), 0)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	bool bExist = existDBRecord_favorites(manufacturer_name, product_name);
	db.close();

	if (item->checkState() == Qt::Checked)
	{  // favorites db에 등록
		if (!bExist)
		{
			insert_favorites_temp(manufacturer_name, product_name);

			disconnections();
			UpdateManufacturerList(manufacturer_name);
			UpdateProductList(product_name);
			UpdateImplantList();
			connections();
		}
	}
	else
	{  // favorites db에서 삭제
		if (bExist)
		{
			delete_favorites_temp(manufacturer_name, product_name);

			disconnections();
			UpdateManufacturerList(manufacturer_name);
			UpdateProductList(product_name);
			UpdateImplantList();
			connections();
		}
	}
}

void CW3ImplantDBDlg::slotCustomManufacturerItemChecked(QTableWidgetItem* item)
{
	QString manufacturer_name = 
		kCustomImplantManufacturerNamePrefix + 
		item->text();

	if (item->checkState() == Qt::Checked)
	{  // favorites db에 등록
		if (insert_favorites_temp(manufacturer_name))
		{
			disconnections();
			UpdateCustomManufacturerList(manufacturer_name);
			UpdateCustomProductList();
			UpdateCustomImplantList();
			connections();
		}
	}
	else
	{  // favorites db에서 삭제
		QSqlDatabase::database(kConnectionName).open();
		bool exist = existDBRecord_favorites(manufacturer_name);
		QSqlDatabase::database(kConnectionName).close();

		if (exist)
		{
			delete_favorites_temp(manufacturer_name);

			disconnections();
			UpdateCustomManufacturerList(manufacturer_name);
			UpdateCustomProductList();
			UpdateCustomImplantList();
			connections();
		}
	}
}

void CW3ImplantDBDlg::slotCustomProductItemChecked(QTableWidgetItem* item)
{
	QString product_name = item->text();
	QString manufacturer_name = 
		kCustomImplantManufacturerNamePrefix + 
		custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	bool bExist = existDBRecord_favorites(manufacturer_name, product_name);
	db.close();

	if (item->checkState() == Qt::Checked)
	{  // favorites db에 등록
		if (!bExist)
		{
			insert_favorites_temp(manufacturer_name, product_name);

			disconnections();
			UpdateCustomManufacturerList(manufacturer_name);
			UpdateCustomProductList(product_name);
			UpdateCustomImplantList();
			connections();
		}
	}
	else
	{  // favorites db에서 삭제
		if (bExist)
		{
			delete_favorites_temp(manufacturer_name, product_name);

			disconnections();
			UpdateCustomManufacturerList(manufacturer_name);
			UpdateCustomProductList(product_name);
			UpdateCustomImplantList();
			connections();
		}
	}
}

// APPLY 버튼 클릭시 기존 즐겨찾기 DB 삭제 후 재생성(초기화)
// 즐겨찾기 임시 DB의 내용을 즐겨찾기 DB에 복사
void CW3ImplantDBDlg::slotApply()
{
	accept();
}

// CANCEL 버튼 클릭시 즐겨찾기 임시 DB 삭제 후 재생성(초기화)
// 즐겨찾기 DB의 내용을 즐겨찾기 임시 DB에 복사
void CW3ImplantDBDlg::slotCancel()
{
	reject();
}

void CW3ImplantDBDlg::accept()
{
	CopyTable(kFavoriteTempDBName, kFavoriteDBName);
	CW3Dialog::accept();
}

void CW3ImplantDBDlg::reject()
{
	CopyTable(kFavoriteDBName, kFavoriteTempDBName);
	CW3Dialog::reject();
}

// 다이얼로그의 x 버튼 클릭시
void CW3ImplantDBDlg::closeEvent(QCloseEvent *e) { slotCancel(); }

// Implant table에서 임플란트가 선택되면 해당 임플란트의 stl경로, diameter,
// length를 가져와서 등록
void CW3ImplantDBDlg::slotClickedImplantTable(int row, int column)
{
	if (curr_implant_row_ != general_implant_list_->currentRow())
	{
		QTableWidgetItem *itemManufacturer =
			general_manufacturer_list_->item(general_manufacturer_list_->currentRow(), 0);
		QString manufacturerProduct = itemManufacturer->text();

		QTableWidgetItem *itemProduct =
			general_product_list_->item(general_product_list_->currentRow(), 0);
		QString currentProduct = itemProduct->text();

		QSqlDatabase db = QSqlDatabase::database(kConnectionName);
		db.open();
		QString implant_id =
			general_implant_list_->item(general_implant_list_->currentRow(), 3)->text();
		QSqlQueryModel qm;
		qm.setQuery(QString("select Implant_path, Implant_diameter, Implant_length "
			"from implants2"
			" where Implant_id = '%1'")
			.arg(implant_id),
			db);
		QString currentImplantPath = qm.record(0).value("Implant_path").toString();
		float diameter = qm.record(0).value("Implant_diameter").toFloat();
		float length = qm.record(0).value("Implant_length").toFloat();
		db.close();

		curr_implant_row_ = general_implant_list_->currentRow();

		/* if (diameter <= 0.0f || length <= 0.0f) return; */

		general_preview_->setImplant(manufacturerProduct, currentProduct,
			currentImplantPath, diameter, length);
	}
}

/////////////////////////////////////////////////////////////////////////////////////
// Database
/////////////////////////////////////////////////////////////////////////////////////
// DB연결
void CW3ImplantDBDlg::connectDB()
{
	int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;
	int odbc_version = GlobalPreferences::GetInstance()->preferences_.general.database.version;

	QSettings odbc_settings("C:/Windows/ODBCINST.INI", QSettings::IniFormat);
	QStringList odbc_group_list = odbc_settings.childGroups();
	QString driver_name = "";
	for (int i = 0; i < odbc_group_list.size(); ++i)
	{
		if (odbc_group_list.at(i).indexOf("MariaDB ODBC") == 0)
		{
			driver_name = odbc_group_list.at(i);
		}
	}

	driver_name.remove(" (32 bit)");
	driver_name.remove(" (64 bit)");

	QSqlDatabase db = QSqlDatabase::database();
#if defined(__APPLE__)
	db = QSqlDatabase::addDatabase("QMYSQL", kConnectionName);
	db.setHostName("127.0.0.1");
	db.setPort(port);
	db.setDatabaseName("Implant");
	db.setUserName("root");
#else
	db = QSqlDatabase::addDatabase("QODBC", kConnectionName);
#if MARIA_DB
	db.setDatabaseName(QString(
		"DRIVER={%1};"
		"SERVER={127.0.0.1};"
		"DATABASE=Implant;"
		"Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port));
#else
	db.setDatabaseName(
		"DRIVER={SQL "
		"Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Implant;Port=1433;UID=sa;"
		"PWD=2002;WSID=.;");
#endif
#endif

	if (!db.open())
	{
		std::string err_msg = lang::LanguagePack::msg_77().toStdString() +
			db.lastError().text().toStdString();
		common::Logger::instance()->Print(common::LogType::ERR, err_msg);
		return;
	}
	db.close();
}

// DB연결 해제
void CW3ImplantDBDlg::disConnectDB()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (db.isOpen())
	{
		db.close();
		db = QSqlDatabase();
		QSqlDatabase::removeDatabase(kConnectionName);
	}
}

// 즐겨찾기용 DB Table 생성
// 연속적으로 Query 문 내부에서 사용하는 함수로 db open & close 는 외부에서 해
// 준다.
void CW3ImplantDBDlg::createDBTable_favorites(const QString &tableName)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	QSqlQueryModel query;
	// 먼저 기존 테이블을 지운다
	query.setQuery(QString("DROP Table %1").arg(tableName), db);

	query.clear();
	// 즐겨찾기용 DB를 생성한다.
	query.setQuery(QString("CREATE TABLE %1("
		"id int(11) NOT NULL AUTO_INCREMENT,"
		"Manufacturer_name varchar(60) NOT NULL,"
		"Product_name  varchar(60) NOT NULL,"
		"PRIMARY KEY (id));")
		.arg(tableName),
		db);
}

// 즐겨찾기 DB 초기화서 사용
// Product2 DB에서 제조자와 제품을 모두 가져와 즐겨찾기 DB에 기록
// 즐겨찾기 임시 DB에도 동일하게 기록
void CW3ImplantDBDlg::updateDBTable_favorites()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel queryProduct, queryTemp;
	queryProduct.setQuery(
		QString("SELECT Product_name, Manufacturer_id FROM Product2"), db);

	int countProduct = queryProduct.rowCount();
	for (int i = 0; i < countProduct; i++)
	{
		int manufacturer_id =
			queryProduct.record(i).value("Manufacturer_id").toInt();
		queryTemp.setQuery(QString("SELECT Manufacturer_name FROM Manufacturer2 "
			"WHERE Manufacturer_id = '%1'")
			.arg(manufacturer_id),
			db);
		QString manufacturer_name =
			queryTemp.record(0).value("Manufacturer_name").toString();

		QString product_name =
			queryProduct.record(i).value("Product_name").toString();
		queryTemp.setQuery(
			QString(
				"INSERT INTO %1 (Manufacturer_name, Product_name)"
				" values('%2', '%3')")
			.arg(kFavoriteDBName)
			.arg(manufacturer_name)
			.arg(product_name),
			db);
		// Original data를 temp로 복사
		queryTemp.setQuery(
			QString(
				"INSERT INTO %1 (Manufacturer_name, Product_name)"
				" values('%2', '%3')")
			.arg(kFavoriteTempDBName)
			.arg(manufacturer_name)
			.arg(product_name),
			db);
	}

	db.close();
}

//즐겨찾기 DB에 해당 테이블(strTable)이 있는지 검사
bool CW3ImplantDBDlg::existDBTable_favorites(const QString &table_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QString q_str =
		QString("SELECT 1 FROM Information_schema.tables WHERE table_name = '%1'")
		.arg(table_name);
	QSqlQueryModel query;
	// query.setQuery(QString("SELECT count(*) as cnt FROM sysobjects WHERE name =
	// '%1'").arg(table_name), db); // mssql 전용 구문
	query.setQuery(q_str, db);

	auto table_count = query.record(0).value(0).toInt();
	db.close();
	// table 갯수, 0이면 해당 테이블이 없음.
	return table_count == 0 ? false : true;
}

// 즐겨찾기 DB에 해당 제조자(strManufacture)의 해당 제품(product_name)이 있는지
// 검사 연속적으로 Query 문 내부에서 사용하는 함수로 db open & close 는 외부에서
// 해 준다.
bool CW3ImplantDBDlg::existDBRecord_favorites(const QString &manufacturer_name,
	const QString &product_name)
{
	QSqlQueryModel query;
	query.setQuery(
		QString("SELECT count(*) as cnt FROM %1"
			" WHERE Manufacturer_name = '%2' AND Product_name = '%3'")
		.arg(kFavoriteTempDBName)
		.arg(manufacturer_name)
		.arg(product_name),
		QSqlDatabase::database(kConnectionName));

	// table 갯수, 0이면 해당 테이블이 없음.
	return query.record(0).value("cnt").toInt() == 0 ? false : true;
}

// 즐겨찾기 DB에 해당 제조자(strManufacture)가 있는지 검사
// 연속적으로 Query 문 내부에서 사용하는 함수로 db open & close 는 외부에서 해
// 준다.
bool CW3ImplantDBDlg::existDBRecord_favorites(const QString &manufacturer_name)
{
	QSqlQueryModel query;
	query.setQuery(QString("SELECT count(*) as cnt FROM %1"
		" WHERE Manufacturer_name = '%2'")
		.arg(kFavoriteTempDBName)
		.arg(manufacturer_name),
		QSqlDatabase::database(kConnectionName));

	// table 갯수, 0이면 해당 테이블이 없음.
	return query.record(0).value("cnt").toInt() == 0 ? false : true;
}

// Implant_Favorites_temp table에 입력 받은 값을 record를 입력
bool CW3ImplantDBDlg::insert_favorites_temp(const QString &manufacturer_name)
{
	QString manufacturer = manufacturer_name;
	bool is_custom = manufacturer_name[0] == kCustomImplantManufacturerNamePrefix;
	QString manufacturer_table_name = is_custom ? "custom_manufacturer" : "Manufacturer2";
	QString product_table_name = is_custom ? "custom_product" : "Product2";
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel queryProduct, query;
	query.setQuery(QString("SELECT Manufacturer_id FROM %1 WHERE "
		"Manufacturer_name='%2'")
		.arg(manufacturer_table_name).arg(manufacturer.remove(kCustomImplantManufacturerNamePrefix)),
		db);

	int manufacturer_id = query.record(0).value("Manufacturer_id").toInt();
	queryProduct.setQuery(
		QString("SELECT Product_name FROM %1 WHERE Manufacturer_id='%2'")
		.arg(product_table_name).arg(manufacturer_id),
		db);

	bool bUpdate = false;
	QString product_name = "";
	for (int i = 0; i < queryProduct.rowCount(); i++)
	{
		product_name = queryProduct.record(i).value("Product_name").toString();
		if (!existDBRecord_favorites(manufacturer_name, product_name))
		{
			query.setQuery(QString("INSERT INTO %1 "
				"(Manufacturer_name, Product_name)"
				" values('%2', '%3')")
				.arg(kFavoriteTempDBName)
				.arg(manufacturer_name)
				.arg(product_name),
				db);
			bUpdate = true;
		}
	}
	db.close();
	return bUpdate;
}

// Implant_Favorites_temp table에 입력 받은 값을 record를 입력
void CW3ImplantDBDlg::insert_favorites_temp(const QString &manufacturer_name,
	const QString &product_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;
	query.setQuery(
		QString("INSERT INTO %1"
			" (Manufacturer_name, Product_name) values('%2', '%3')")
		.arg(kFavoriteTempDBName)
		.arg(manufacturer_name)
		.arg(product_name),
		db);
	db.close();
}

// Implant_Favorites_temp table에 입력 받은 조건에 맞는 record를 삭제
void CW3ImplantDBDlg::delete_favorites_temp(const QString &manufacturer_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;
	query.setQuery(
		QString(
			"DELETE FROM %1 WHERE Manufacturer_name = '%2'")
		.arg(kFavoriteTempDBName)
		.arg(manufacturer_name),
		db);
	db.close();
}

// Implant_Favorites_temp table에 입력 받은 조건에 맞는 record를 삭제
void CW3ImplantDBDlg::delete_favorites_temp(const QString &manufacturer_name,
	const QString &product_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;
	query.setQuery(QString("DELETE FROM %1 WHERE"
		" Manufacturer_name = '%2' AND Product_name ='%3'")
		.arg(kFavoriteTempDBName)
		.arg(manufacturer_name)
		.arg(product_name),
		db);
	db.close();
}

// from_table_name의 내용을 to_table_name에 복사
void CW3ImplantDBDlg::CopyTable(const QString &from_table_name,
	const QString &to_table_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	createDBTable_favorites(to_table_name);

	QSqlQueryModel query;
	query.setQuery(QString("INSERT INTO %1 ( SELECT * FROM %2 )")
		.arg(to_table_name, from_table_name),
		db);
	db.close();
}

void CW3ImplantDBDlg::UpdateProductDBTable()
{
	QSqlQueryModel query, queryProduct;
	QString strQuery = "";

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	queryProduct.setQuery(QString("SELECT Product_id FROM product2"), db);

	while (queryProduct.canFetchMore()) //row count가 255개 이상일 경우 rowCount()가 제대로 가져오지 못하기 때문(max 256)에 사용
		queryProduct.fetchMore();

	int rowCountProduct = queryProduct.rowCount();

	for (int i = 0; i < rowCountProduct; i++)
	{
		QString product_id = queryProduct.record(i).value("Product_id").toString();

		query.setQuery(QString("SELECT count(*) as cnt FROM Implants2 WHERE Product_id = '%1'").arg(product_id), db);

		QString implantCnt = query.record(0).value("cnt").toString(); //Implant 갯수

		query.clear();
		query.setQuery(QString("SELECT Implant_diameter, Implant_length FROM Implants2 WHERE Product_id = '%1'").arg(product_id), db);

		float minValue_diameter = 100.0f;
		float maxValue_diameter = 0.0f;

		float minValue_length = 100.0f;
		float maxValue_length = 0.0f;

		float value_diameter = 0.0f;
		float value_length = 0.0f;

		for (int j = 0; j < implantCnt.toInt(); j++)
		{
			value_diameter = query.record(j).value("Implant_diameter").toFloat();

			if (value_diameter < minValue_diameter)
				minValue_diameter = value_diameter;

			if (maxValue_diameter < value_diameter)
				maxValue_diameter = value_diameter;

			value_length = query.record(j).value("Implant_length").toFloat();

			if (value_length < minValue_length)
				minValue_length = value_length;

			if (maxValue_length < value_length)
				maxValue_length = value_length;
		}

		query.clear();
		strQuery =
			QString("UPDATE Product2 SET "
				"Product_implantDiameterMin = '%1',Product_implantDiameterMax = '%2',Product_implantLengthMin = '%3',Product_implantLengthMax = '%4', Product_implantCount = '%5' WHERE Product_id = '%6'")
			.arg(minValue_diameter).arg(maxValue_diameter).arg(minValue_length).arg(maxValue_length).arg(implantCnt).arg(product_id);
		query.setQuery(strQuery, db);
	}
}

void CW3ImplantDBDlg::slotAdd()
{
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::DirectoryOnly);
	dlg.setViewMode(QFileDialog::Detail);

	QString root_path = "";
	QString root_name = "";
	if (dlg.exec())
	{
		root_path = dlg.directory().path();
		root_name = dlg.directory().dirName();
	}

	if (root_path == "")
	{
		return;
	}
	QStringList nameFilter;
	nameFilter << "*.*";

	QDirIterator dir_Iterator(root_path, nameFilter, QDir::Files, QDirIterator::Subdirectories);
	QStringList Implants_list;
	int remove_length = root_path.length() - root_name.length();
	while (dir_Iterator.hasNext())
	{
		Implants_list << dir_Iterator.next().remove(0, remove_length);
	}

	UpdateDBTable(Implants_list);
}

void CW3ImplantDBDlg::UpdateDBTable(const QStringList& implant_path)
{
	QString manufacturer_name = "";
	QString product_name = "";
	QString model_name = "";
	QString part = "";

	QSqlQueryModel query;

	int manufacturer_id = 0;
	int product_id = 0;
	int implant_id = 0;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	for (int i = 0; i < implant_path.size(); ++i)
	{
		QFileInfo implant(implant_path.at(i));
		if (implant.suffix().compare("wim", Qt::CaseInsensitive) != 0)
		{
			continue;
		}

		// 0: RootFolderName 1: Manufacturer, 2: Implant or Abutment, 3:Product, 4:Model(.stl)
		int manufacturer_index = 0;
		int implant_index = 1;
		int product_index = 2;
		int model_index = 3;

		QStringList implant_info_list = implant_path.at(i).split("/");
		part = implant_info_list.at(implant_index);

		if (part == "Implant")
		{
			// Manufacturer
			manufacturer_name = implant_info_list.at(manufacturer_index);
			query.setQuery(QString("SELECT Manufacturer_id FROM Manufacturer2 WHERE Manufacturer_name='%1'")
				.arg(manufacturer_name), db);
			manufacturer_id = query.record(0).value("Manufacturer_id").toInt();

			if (manufacturer_id == 0)
			{
				query.setQuery(QString("INSERT INTO Manufacturer2 (Manufacturer_name) values('%1')")
					.arg(manufacturer_name), db);
			}

			// Product
			product_name = implant_info_list.at(product_index);

			query.setQuery(QString("SELECT Manufacturer_id FROM Manufacturer2 WHERE Manufacturer_name='%1'")
				.arg(manufacturer_name), db);
			manufacturer_id = query.record(0).value("Manufacturer_id").toInt();

			query.setQuery(QString("SELECT Product_id FROM Product2 WHERE Product_name='%1' and Manufacturer_id='%2'")
				.arg(product_name).arg(manufacturer_id), db);
			product_id = query.record(0).value("Product_id").toInt();

			if (product_id == 0)
			{
				query.setQuery(QString("INSERT INTO Product2 (Product_name, Product_implantDiameterMin,"
					" Product_implantDiameterMax, Product_implantLengthMin,"
					" Product_implantLengthMax, Product_implantCount, Manufacturer_id)"
					" values('%1','0.0','0.0','0.0','0.0','0','%2')")
					.arg(product_name).arg(manufacturer_id), db);
			}

			// Implant
			model_name = implant_info_list.at(model_index);
			model_name.chop(4);

			query.setQuery(QString("SELECT Product_id FROM Product2 WHERE Product_name='%1' and Manufacturer_id='%2'")
				.arg(product_name).arg(manufacturer_id), db);
			product_id = query.record(0).value("Product_id").toInt();

			query.setQuery(QString("SELECT Implant_id FROM Implants2 WHERE Implant_name='%1' and Product_id='%2'")
				.arg(model_name).arg(product_id), db);
			implant_id = query.record(0).value("Implant_id").toInt();

			if (implant_id == 0)
			{
				query.setQuery(QString("INSERT INTO Implants2 (Implant_name, Product_id,"
					" Implant_path, Implant_diameter, Implant_length)"
					" values('%1','%2','%3','0.0','0.0')")
					.arg(model_name).arg(product_id).arg("Implant/" + implant_path.at(i)), db);
			}
		} /*else { Abutment Part }*/
	}
}

void CW3ImplantDBDlg::slotTabChanged(int index)
{
	current_tab_ = static_cast<Tab>(index);

	SetButtonMode(current_tab_);
}

void CW3ImplantDBDlg::slotAddCustomImplantButtonClicked()
{
	CustomImplantDialog custom_implant_dialog;
	custom_implant_dialog.SetManufacturerList(GetManufacturerList());
	custom_implant_dialog.SetProductList(GetProductList());
	connect(&custom_implant_dialog,
		SIGNAL(sigThrowData(QString, QString, QString, float, float, float)),
		this,
		SLOT(slotAddCustomImplant(QString, QString, QString, float, float, float))
	);
	connect(this, SIGNAL(sigCustomImplantUpdated()), &custom_implant_dialog, SLOT(accept()));
	if (custom_implant_dialog.exec())
	{
		// Do something?
	}
}

void CW3ImplantDBDlg::slotModifyCustomImplantButtonClicked()
{
	int current_manufacturer_row = custom_manufacturer_list_->currentRow();
	int current_product_row = custom_product_list_->currentRow();
	int current_implant_row = custom_implant_list_->currentRow();
	if (current_manufacturer_row < 0 ||
		current_product_row < 0 ||
		current_implant_row < 0)
	{
		return;
	}

	QStringList manufacturer_list;
	QStringList product_list;
	for (int i = 0; i < custom_manufacturer_list_->rowCount(); ++i)
	{
		manufacturer_list.push_back(custom_manufacturer_list_->item(i, 0)->text());
	}
	for (int i = 0; i < custom_product_list_->rowCount(); ++i)
	{
		product_list.push_back(custom_product_list_->item(i, 0)->text());
	}

	CustomImplantDialog custom_implant_dialog;
	custom_implant_dialog.SetManufacturerList(GetManufacturerList());
	custom_implant_dialog.SetProductList(GetProductList());

	QString manufacturer_name = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();
	QString product_name = custom_product_list_->item(custom_product_list_->currentRow(), 0)->text();
	QString implant_name = custom_implant_list_->item(custom_implant_list_->currentRow(), 0)->text();
	float coronal_diameter = custom_implant_list_->item(custom_implant_list_->currentRow(), 1)->text().remove("mm").toFloat();
	float apical_diameter = custom_implant_list_->item(custom_implant_list_->currentRow(), 2)->text().remove("mm").toFloat();
	float length = custom_implant_list_->item(custom_implant_list_->currentRow(), 3)->text().remove("mm").toFloat();

	custom_implant_dialog.SetData(
		manufacturer_name,
		product_name,
		implant_name,
		coronal_diameter,
		apical_diameter,
		length
	);

	connect(&custom_implant_dialog,
		SIGNAL(sigThrowData(QString, QString, QString, float, float, float)),
		this,
		SLOT(slotModifyCustomImplant(QString, QString, QString, float, float, float))
	);
	connect(this, SIGNAL(sigCustomImplantUpdated()), &custom_implant_dialog, SLOT(accept()));
	if (custom_implant_dialog.exec())
	{
		// Do something?
	}
}

void CW3ImplantDBDlg::slotRemoveCustomImplantButtonClicked()
{
	switch (focused_table_)
	{
	case CW3ImplantDBDlg::CustomTableType::NONE:
		break;
	case CW3ImplantDBDlg::CustomTableType::MANUFACTURER:
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_85(), CW3MessageBox::Question);
		if (message_box.exec())
		{
			RemoveSelectedManufacturer();
		}
	}
	break;
	case CW3ImplantDBDlg::CustomTableType::PRODUCT:
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_86(), CW3MessageBox::Question);
		if (message_box.exec())
		{
			RemoveSelectedProduct();
		}
	}
	break;
	case CW3ImplantDBDlg::CustomTableType::IMPLANT:
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_87(), CW3MessageBox::Question);
		if (message_box.exec())
		{
			RemoveSelectedImplant();
		}
	}
	break;
	default:
		break;
	}
}

void CW3ImplantDBDlg::RemoveSelectedManufacturer()
{
	int current_manufacturer_row = custom_manufacturer_list_->currentRow();
	if (current_manufacturer_row < 0)
	{
		return;
	}

	QString manufacturer_name = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();
	QString manufacturer_id = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), custom_manufacturer_list_->columnCount() - 1)->text();

	RemoveManufacturerToDatabase(manufacturer_id.toInt());

	disconnections();
	UpdateCustomManufacturerList();
	UpdateCustomProductList();
	UpdateCustomImplantList();
	connections();

	manufacturer_name = kCustomImplantManufacturerNamePrefix + manufacturer_name;

	if (existDBRecord_favorites(manufacturer_name))
	{
		delete_favorites_temp(manufacturer_name);
		emit sigFavoriteUpdated();
	}
}

void CW3ImplantDBDlg::RemoveSelectedProduct()
{
	int current_manufacturer_row = custom_manufacturer_list_->currentRow();
	int current_product_row = custom_product_list_->currentRow();
	if (current_manufacturer_row < 0 ||
		current_product_row < 0)
	{
		return;
	}

	QString manufacturer_name = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();
	QString product_name = custom_product_list_->item(custom_product_list_->currentRow(), 0)->text();
	QString product_id = custom_product_list_->item(custom_product_list_->currentRow(), custom_product_list_->columnCount() - 1)->text();

	RemoveProductToDatabase(product_id.toInt());

	disconnections();
	UpdateCustomProductList();
	UpdateCustomImplantList();
	connections();

	manufacturer_name = kCustomImplantManufacturerNamePrefix + manufacturer_name;

	if (existDBRecord_favorites(manufacturer_name, product_name))
	{
		delete_favorites_temp(manufacturer_name, product_name);
		emit sigFavoriteUpdated();
	}
}

void CW3ImplantDBDlg::RemoveSelectedImplant()
{
	int current_manufacturer_row = custom_manufacturer_list_->currentRow();
	int current_product_row = custom_product_list_->currentRow();
	int current_implant_row = custom_implant_list_->currentRow();
	if (current_manufacturer_row < 0 ||
		current_product_row < 0 ||
		current_implant_row < 0)
	{
		return;
	}

	QString manufacturer_name = custom_manufacturer_list_->item(custom_manufacturer_list_->currentRow(), 0)->text();
	QString product_name = custom_product_list_->item(custom_product_list_->currentRow(), 0)->text();
	QString implant_id = custom_implant_list_->item(custom_implant_list_->currentRow(), custom_implant_list_->columnCount() - 1)->text();

	RemoveImplantToDatabase(implant_id.toInt());

	disconnections();
	UpdateCustomImplantList();
	connections();

	manufacturer_name = kCustomImplantManufacturerNamePrefix + manufacturer_name;

	if (existDBRecord_favorites(manufacturer_name, product_name))
	{
		emit sigFavoriteUpdated();
	}
}

void CW3ImplantDBDlg::RemoveManufacturerToDatabase(int manufacturer_id)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;

	query.setQuery(QString(
		"SELECT Product_id FROM custom_product WHERE"
		" Manufacturer_id = '%1'")
		.arg(manufacturer_id),
		db
	);

	std::vector<int> product_id_list;
	for (int i = 0; i < query.rowCount(); ++i)
	{
		int product_id = query.record(i).value("Product_id").toInt();
		product_id_list.push_back(product_id);
	}
	db.close();

	for (int i = 0; i < product_id_list.size(); ++i)
	{
		RemoveProductToDatabase(product_id_list.at(i));
	}

	db.open();
	query.setQuery(QString(
		"DELETE FROM custom_manufacturer WHERE"
		" Manufacturer_id = '%1'")
		.arg(manufacturer_id),
		db
	);
	db.close();
}

void CW3ImplantDBDlg::RemoveProductToDatabase(int product_id)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;

	query.setQuery(QString(
		"SELECT Implant_id FROM custom_implants WHERE"
		" Product_id = '%1'")
		.arg(product_id),
		db
	);

	std::vector<int> implant_id_list;
	for (int i = 0; i < query.rowCount(); ++i)
	{
		int implant_id = query.record(i).value("Implant_id").toInt();
		implant_id_list.push_back(implant_id);
	}
	db.close();

	for (int i = 0; i < implant_id_list.size(); ++i)
	{
		RemoveImplantToDatabase(implant_id_list.at(i));
	}

	db.open();
	query.setQuery(QString(
		"DELETE FROM custom_product WHERE"
		" Product_id = '%1'")
		.arg(product_id),
		db
	);
	db.close();
}

void CW3ImplantDBDlg::RemoveImplantToDatabase(int implant_id)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;
	query.setQuery(QString(
		"DELETE FROM custom_implants WHERE"
		" Implant_id = '%1'")
		.arg(implant_id),
		db
	);
	db.close();
}

void CW3ImplantDBDlg::slotAddCustomImplant(
	const QString& manufacturer,
	const QString& product,
	const QString& model,
	const float coronal_diameter,
	const float apical_diameter,
	const float length
)
{
	qDebug() << "add custom implant :" << manufacturer << product << model << coronal_diameter << apical_diameter << length;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	QSqlQueryModel query;

	int manufacturer_id = 0;
	int product_id = 0;
	int implant_id = 0;

	QString implant_type = (coronal_diameter == apical_diameter) ? lang::LanguagePack::txt_straight() : lang::LanguagePack::txt_tapered();

	// Manufacturer
	db.open();
	query.setQuery(
		QString("SELECT Manufacturer_id FROM custom_manufacturer WHERE Manufacturer_name='%1'")
		.arg(manufacturer),
		db
	);
	manufacturer_id = query.record(0).value("Manufacturer_id").toInt();

	if (manufacturer_id == 0)
	{
		query.setQuery(
			QString("INSERT INTO custom_manufacturer (Manufacturer_name) values('%1')")
			.arg(manufacturer),
			db
		);

		if (db.lastError().type() != QSqlError::ErrorType::NoError)
		{
			// TODO: error 팝업
			db.close();
			return;
		}

		disconnections();
		UpdateCustomManufacturerList();
		connections();
	}
	db.close();

	// Product
	db.open();
	query.setQuery(
		QString("SELECT Manufacturer_id FROM custom_manufacturer WHERE Manufacturer_name='%1'")
		.arg(manufacturer),
		db
	);
	manufacturer_id = query.record(0).value("Manufacturer_id").toInt();

	query.setQuery(
		QString("SELECT Product_id FROM custom_product WHERE Product_name='%1' and Manufacturer_id='%2'")
		.arg(product).arg(manufacturer_id),
		db
	);
	product_id = query.record(0).value("Product_id").toInt();

	if (product_id == 0)
	{
		query.setQuery(
			QString("INSERT INTO custom_product ("
				"Product_name, Product_implantDiameterMin,"
				" Product_implantDiameterMax, Product_implantLengthMin,"
				" Product_implantLengthMax, Product_implantCount,"
				" Manufacturer_id"
				")"
				" values ('%1','0.0','0.0','0.0','0.0','0','%2')")
			.arg(product).arg(manufacturer_id),
			db
		);

		if (db.lastError().type() != QSqlError::ErrorType::NoError)
		{
			// TODO: error 팝업
			db.close();
			return;
		}

		disconnections();
		UpdateCustomProductList();
		connections();
	}
	db.close();

	// Implant
	db.open();
	query.setQuery(
		QString("SELECT Product_id FROM custom_product WHERE Product_name='%1' and Manufacturer_id='%2'")
		.arg(product).arg(manufacturer_id),
		db
	);
	product_id = query.record(0).value("Product_id").toInt();

	query.setQuery(
		QString("SELECT Implant_id FROM custom_implants WHERE"
			"Implant_name='%1' and Product_id='%2' and coronal_diameter='%3' and apical_diameter='%4' and length='%5' and type='%6'")
		.arg(model).arg(product_id).arg(coronal_diameter).arg(apical_diameter).arg(length).arg(implant_type),
		db
	);
	implant_id = query.record(0).value("Implant_id").toInt();

	if (implant_id == 0)
	{
		query.setQuery(
			QString("INSERT INTO custom_implants ("
				"Implant_name, Product_id, coronal_diameter, apical_diameter, length, type"
				")"
				" values ('%1','%2','%3','%4','%5','%6')")
			.arg(model).arg(product_id).arg(coronal_diameter).arg(apical_diameter).arg(length).arg(implant_type),
			db
		);

		if (db.lastError().type() != QSqlError::ErrorType::NoError)
		{
			// TODO: error 팝업
			db.close();
			return;
		}

		db.close();

		disconnections();
		UpdateCustomImplantList();
		connections();

		emit sigCustomImplantUpdated();

		if (existDBRecord_favorites(kCustomImplantManufacturerNamePrefix + manufacturer, product))
		{
			emit sigFavoriteUpdated();
		}
	}

	db.close();
}

void CW3ImplantDBDlg::slotModifyCustomImplant(
	const QString& manufacturer,
	const QString& product,
	const QString& model,
	const float coronal_diameter,
	const float apical_diameter,
	const float length
)
{
	qDebug() << "modify custom implant :" << manufacturer << product << model << coronal_diameter << apical_diameter << length;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel query;

	int manufacturer_id = 0;
	int product_id = 0;

	QString implant_type = (coronal_diameter == apical_diameter) ? lang::LanguagePack::txt_straight() : lang::LanguagePack::txt_tapered();

	QString old_product_id = custom_product_list_->item(custom_product_list_->currentRow(), custom_product_list_->columnCount() - 1)->text();
	QString old_implant_id = custom_implant_list_->item(custom_implant_list_->currentRow(), custom_implant_list_->columnCount() - 1)->text();
	QString old_implant_name = custom_implant_list_->item(custom_implant_list_->currentRow(), 0)->text();
	float old_coronal_diameter = custom_implant_list_->item(custom_implant_list_->currentRow(), 1)->text().remove("mm").toFloat();
	float old_apical_diameter = custom_implant_list_->item(custom_implant_list_->currentRow(), 2)->text().remove("mm").toFloat();
	float old_length = custom_implant_list_->item(custom_implant_list_->currentRow(), 3)->text().remove("mm").toFloat();

	query.setQuery(
		QString("SELECT Manufacturer_id FROM custom_manufacturer WHERE Manufacturer_name='%1'")
		.arg(manufacturer),
		db
	);
	manufacturer_id = query.record(0).value("Manufacturer_id").toInt();
	if (manufacturer_id == 0)
	{
		// TODO: error 팝업
		db.close();
		return;
	}

	query.setQuery(
		QString("SELECT Product_id FROM custom_product WHERE Product_name='%1' and Manufacturer_id='%2'")
		.arg(product).arg(manufacturer_id),
		db
	);
	product_id = query.record(0).value("Product_id").toInt();
	if (product_id == 0)
	{
		// TODO: error 팝업
		db.close();
		return;
	}

	query.setQuery(
		QString("UPDATE custom_implants SET"
			" Implant_name='%1', Product_id='%2', coronal_diameter='%3', apical_diameter='%4', length='%5', type='%6'"
			" WHERE"
			" Implant_name='%7' and Product_id='%8' and coronal_diameter='%9' and apical_diameter='%10' and length='%11'")
		.arg(model).arg(product_id).arg(coronal_diameter).arg(apical_diameter).arg(length).arg(implant_type)
		.arg(old_implant_name).arg(old_product_id).arg(old_coronal_diameter).arg(old_apical_diameter).arg(old_length),
		db
	);

	if (db.lastError().type() != QSqlError::ErrorType::NoError)
	{
		// TODO: error 팝업
		db.close();
		return;
	}

	db.close();

	disconnections();
	UpdateCustomImplantList();
	connections();

	emit sigCustomImplantUpdated();

	if (existDBRecord_favorites(kCustomImplantManufacturerNamePrefix + manufacturer, product))
	{
		emit sigFavoriteUpdated();
	}
}

void CW3ImplantDBDlg::slotCustomManufacturerCellClicked(int row, int column)
{
	focused_table_ = CustomTableType::MANUFACTURER;
}

void CW3ImplantDBDlg::slotCustomProductCellClicked(int row, int column)
{
	focused_table_ = CustomTableType::PRODUCT;
}

void CW3ImplantDBDlg::slotCustomImplantCellClicked(int row, int column)
{
	focused_table_ = CustomTableType::IMPLANT;
}

QStringList CW3ImplantDBDlg::GetManufacturerList()
{
	QStringList list;
	for (int i = 0; i < custom_manufacturer_list_->rowCount(); ++i)
	{
		list.push_back(custom_manufacturer_list_->item(i, 0)->text());
	}
	return list;
}

QStringList CW3ImplantDBDlg::GetProductList()
{
	QStringList list;
#if 0
	for (int i = 0; i < custom_product_list_->rowCount(); ++i)
	{
		list.push_back(custom_product_list_->item(i, 0)->text());
	}
#else
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(
		QString("SELECT Product_name FROM custom_product ORDER BY Product_name ASC"),
		db
	);

	int row_count = qm.rowCount();

	for (int i = 0; i < row_count; i++)
	{
		QString product_name = qm.record(i).value("Product_name").toString();
		list.push_back(product_name);
	}
	db.close();
#endif
	return list;
}
