#include "implant_preference_dialog.h"

#include <iostream>

#include <QDebug>
#include <qboxlayout.h>
#include <qframe.h>
#include <qlayoutitem.h>
#include <qpixmap.h>
#include <qradiobutton.h>
#include <qsettings.h>
#include <qsqldatabase.h>
#include <qsqlquery.h>
#include <qsqlquerymodel.h>
#include <qsqlrecord.h>
#include <qtextcodec.h>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolButton>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"
#include "../UIComponent/W3View3DImplantPreview.h"
#include "../UIComponent/implant_preference_item.h"
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Resource/Resource/implant_resource.h>

using namespace implant_resource;

namespace
{
	constexpr int kDlgWidth = 950;
	constexpr int kDlgHeight = 650;
	const int kSectionHeight = 23;
	const QString kConnectionName("ImplantPreferenceDlg");

	constexpr int kPreviewSize = 160;
	constexpr int kRightIndex = 0;
	constexpr int kLeftIndex = 1;
	constexpr int kNumDisplayPresets = 14;
	const int kImplantPreferenceNumbers[2][kNumDisplayPresets] = {
	  { 17, 16, 15, 14, 13, 12, 11,
	  47, 46, 45, 44, 43, 42, 41,},

	  { 21, 22, 23, 24, 25, 26, 27,
	  31, 32, 33, 34, 35, 36, 37 }
	};

	const QString kTokenSize("mm");
	const QString kInvalidManufacturerID("-1");
}  // end of namespace

using namespace implant_preset;

ImplantPreferenceDlg::ImplantPreferenceDlg(CW3VREngine *VREngine,
	CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer,
	QWidget *pParent)
	: CW3Dialog(lang::LanguagePack::txt_implant_preference()),
	preview_(
		new CW3View3DImplantPreview(VREngine, MPRengine, Rcontainer,
			common::ViewTypeID::IMPLANT_PREVIEW)),
	apply_(new QToolButton),
	close_(new QToolButton),
	delete_all_(new QToolButton),
	left_(new QRadioButton),
	right_(new QRadioButton),
	thumbnail_(new QLabel)
{
	manufacturer_list_ = new QTableWidget(0, 2, this);
	product_list_ = new QTableWidget(0, 2, this);
	implant_list_ = new QTableWidget(0, 4, this);

	this->setFixedSize(kDlgWidth, kDlgHeight);

	// Control Button
	delete_all_->setText(lang::LanguagePack::txt_delete_all());
	delete_all_->setFixedWidth(170);
	close_->setText(lang::LanguagePack::txt_close());
	close_->setFixedWidth(170);
	apply_->setText(lang::LanguagePack::txt_apply());
	apply_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	left_->setText(lang::LanguagePack::txt_left());
	right_->setText(lang::LanguagePack::txt_right());

	if (current_preset_ == CurrentPreset::RIGHT)
		right_->setChecked(true);
	else
		left_->setChecked(true);

	thumbnail_->setPixmap(QPixmap(":/image/dialog/preference_thumb_right.png"));
	thumbnail_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	preview_->setFixedSize(kPreviewSize, kPreviewSize - 22);

	////////////////////////////////////////////////////////
	// Manufacturer List
	manufacturer_list_->verticalHeader()->setVisible(false);
	manufacturer_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	manufacturer_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	manufacturer_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	manufacturer_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	manufacturer_list_->setShowGrid(false);
	manufacturer_list_->setFixedSize(130, kPreviewSize);

	QStringList manufacturer_header_label = {
		"  " + lang::LanguagePack::txt_manufacturer() };
	manufacturer_list_->setHorizontalHeaderLabels(manufacturer_header_label);
	manufacturer_list_->setColumnHidden(1, true);

	QHeaderView *header = manufacturer_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setSectionResizeMode(0, QHeaderView::Stretch);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	////////////////////////////////////////////////////////
	// Product Table
	product_list_->verticalHeader()->setVisible(false);
	product_list_->setMouseTracking(true);
	product_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	product_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	product_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	product_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	product_list_->setShowGrid(false);
	product_list_->setFixedSize(kPreviewSize, kPreviewSize);

	QStringList product_header_labels = { "  " +
										 lang::LanguagePack::txt_product_line() };
	product_list_->setHorizontalHeaderLabels(product_header_labels);
	product_list_->setColumnHidden(1, true);

	header = product_list_->horizontalHeader();
	header->setDefaultAlignment(Qt::AlignLeft);
	header->setStretchLastSection(true);
	header->setHighlightSections(false);
	header->setStyleSheet(CW3Theme::getInstance()->TableWidgetHeaderStyleSheet());

	////////////////////////////////////////////////////////
	// Implant Table
	implant_list_->verticalHeader()->setVisible(false);
	implant_list_->setEditTriggers(QAbstractItemView::NoEditTriggers);
	implant_list_->setSelectionBehavior(QAbstractItemView::SelectRows);
	implant_list_->setContextMenuPolicy(Qt::CustomContextMenu);
	implant_list_->setSelectionMode(QAbstractItemView::SingleSelection);
	implant_list_->setShowGrid(false);
	implant_list_->setFixedHeight(kPreviewSize);

	header = implant_list_->horizontalHeader();
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
		"  " + lang::LanguagePack::txt_length()
	};
	implant_list_->setHorizontalHeaderLabels(implant_header_labels);
	implant_list_->setColumnHidden(3, true);

	QHBoxLayout *maxillary_layout_ = new QHBoxLayout();  // 상악
	maxillary_layout_->setSpacing(6);
	maxillary_layout_->setContentsMargins(0, 0, 0, 0);
	QHBoxLayout *mandibular_layout_ = new QHBoxLayout();  // 하악
	mandibular_layout_->setSpacing(6);
	mandibular_layout_->setContentsMargins(0, 0, 0, 0);

	int implant_index =
		current_preset_ == CurrentPreset::RIGHT ? kRightIndex : kLeftIndex;
	for (int idx = 0; idx < kNumDisplayPresets; ++idx)
	{
		if (idx != 0 && idx != 7)
		{
			QFrame *line = new QFrame;
			line->setObjectName("line");
			line->setFrameShape(QFrame::VLine);

			if (idx < 7)
				maxillary_layout_->addWidget(line);
			else
				mandibular_layout_->addWidget(line);
		}

		preset_items_[idx] =
			new ImplantPreferenceItem(kImplantPreferenceNumbers[implant_index][idx], this);
		if (idx < 7)
			maxillary_layout_->addWidget(preset_items_[idx]);
		else
			mandibular_layout_->addWidget(preset_items_[idx]);
	}

	QVBoxLayout *preference_display_layout = new QVBoxLayout();
	preference_display_layout->setSpacing(19);
	preference_display_layout->setContentsMargins(0, 0, 0, 0);
	preference_display_layout->addLayout(maxillary_layout_);
	preference_display_layout->addLayout(mandibular_layout_);

	QHBoxLayout *lr_select_layout = new QHBoxLayout;
	lr_select_layout->setSpacing(10);
	lr_select_layout->setContentsMargins(0, 0, 0, 0);
	lr_select_layout->addWidget(right_);
	lr_select_layout->addWidget(left_);
	lr_select_layout->addItem(
		new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

	QVBoxLayout *preview_layout = new QVBoxLayout;
	preview_layout->setSpacing(0);
	preview_layout->setContentsMargins(0, 0, 0, 0);
	preview_layout->addWidget(preview_);
	preview_layout->addWidget(apply_);

	QHBoxLayout *lib_layout = new QHBoxLayout;
	lib_layout->setSpacing(8);
	lib_layout->setContentsMargins(0, 0, 0, 0);
	lib_layout->addWidget(thumbnail_);
	lib_layout->addWidget(manufacturer_list_);
	lib_layout->addWidget(product_list_);
	lib_layout->addWidget(implant_list_);
	lib_layout->addLayout(preview_layout);

	QHBoxLayout *button_layout = new QHBoxLayout;
	button_layout->setAlignment(Qt::AlignCenter);
	button_layout->setSpacing(10);
	button_layout->setContentsMargins(0, 0, 0, 0);
	button_layout->addWidget(delete_all_);
	button_layout->addWidget(close_);

	QFrame *h_line = new QFrame();
	h_line->setFrameShape(QFrame::HLine);
	h_line->setObjectName("line");
	h_line->setLineWidth(0);
	h_line->setMidLineWidth(0);

	m_contentLayout->setContentsMargins(9, 9, 9, 9);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addWidget(new QLabel(lang::LanguagePack::msg_88()));
	m_contentLayout->addLayout(preference_display_layout);
	m_contentLayout->addWidget(h_line);
	m_contentLayout->addLayout(lr_select_layout);
	m_contentLayout->addLayout(lib_layout);
	m_contentLayout->addLayout(button_layout);

	Connections();
	ConnectDB();

	LoadPresets();
}

ImplantPreferenceDlg::~ImplantPreferenceDlg() {}

void ImplantPreferenceDlg::closeEvent(QCloseEvent *e) { slotClose(); }

void ImplantPreferenceDlg::slotClose()
{
	DisconnectDB();
	this->accept();
}

void ImplantPreferenceDlg::slotDeleteAll()
{
	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_56(),
		CW3MessageBox::Question);

	if (msgBox.exec())
	{
		for (int idx = 0; idx < kNumDisplayPresets; ++idx)
		{
			ClearPreset(idx);
		}
		ClearTables();
	}
}

void ImplantPreferenceDlg::slotApply()
{
	bool implant_not_selected =
		implant_list_->rowCount() == 0 || implant_list_->currentRow() < 0;

	int selected_index = -1;
	for (int index = 0; index < kNumDisplayPresets; ++index)
	{
		if (preset_items_[index]->current_selected())
		{
			selected_index = index;
			break;
		}
	}

	if (implant_not_selected || selected_index < 0)
	{
		// 프리셋 설정 실패 시 다음의 팝업을 띄워야 한다.
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_55(),
			CW3MessageBox::Critical);
		msgBox.exec();
		return;
	}

	UpdatePreset(selected_index);
	ClearTables();
}

void ImplantPreferenceDlg::slotLRChanged(bool is_left)
{
	current_preset_ = is_left ? CurrentPreset::LEFT : CurrentPreset::RIGHT;
	if (current_preset_ == CurrentPreset::RIGHT)
		thumbnail_->setPixmap(QPixmap(":/image/dialog/preference_thumb_right.png"));
	else
		thumbnail_->setPixmap(QPixmap(":/image/dialog/preference_thumb_left.png"));

	LoadPresets();
}

void ImplantPreferenceDlg::slotClearCurrentItem(int implant_number)
{
	// save preset data to INI file
	QString section = kSectionName + QString::number(implant_number);
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
	settings.setValue(section + kKeyManufacturer, "");
	settings.setValue(section + kKeyProduct, "");
	settings.setValue(section + kKeyModel, "");
}

void ImplantPreferenceDlg::slotSetCurrentItemInfo(
	const QString &manufacturer_name, const QString &product_name,
	const QString &diameter, const QString &length)
{
	// sync ui status
	QObject *sender = QObject::sender();
	for (int idx = 0; idx < kNumDisplayPresets; ++idx)
	{
		ImplantPreferenceItem *item = preset_items_[idx];
		if (item != sender && item->current_selected())
		{
			item->SyncSelectedStatus(false);
		}
	}

	Disconnections();
	// 얻어온 문자열로 DB query.
	UpdateManufacturerList(manufacturer_name);
	UpdateProductList(product_name);
	UpdateImplantList(diameter, length);

	if (!diameter.isEmpty() && !length.isEmpty())
	{
		UpdatePreview();
	}

	Connections();
}

void ImplantPreferenceDlg::slotUpdateProductTable(int currentRow,
	int currentColumn,
	int previousRow,
	int previousColumn)
{
	if (currentRow == previousRow) return;

	manufacturer_list_->setCurrentCell(currentRow, 0);

	Disconnections();
	UpdateProductList();
	UpdateImplantList();
	Connections();
}

void ImplantPreferenceDlg::slotUpdateImplantTable(int currentRow,
	int currentColumn,
	int previousRow,
	int previousColumn)
{
	if (currentRow == previousRow) return;

	product_list_->setCurrentCell(currentRow, 0);

	Disconnections();
	UpdateImplantList();
	Connections();
}

void ImplantPreferenceDlg::slotClickedImplantTable(int row, int column)
{
	if (curr_implant_row_ != implant_list_->currentRow())
	{
#if 0
		QTableWidgetItem *itemManufacturer =
			manufacturer_list_->item(manufacturer_list_->currentRow(), 0);
		QString manufacturerProduct = itemManufacturer->text();

		QTableWidgetItem *itemProduct =
			product_list_->item(product_list_->currentRow(), 0);
		QString currentProduct = itemProduct->text();

		QSqlDatabase db = QSqlDatabase::database(kConnectionName);
		db.open();
		QString implant_id =
			implant_list_->item(implant_list_->currentRow(), 3)->text();
		QSqlQueryModel qm;
		qm.setQuery(
			QString(
				"select Implant_path, Implant_diameter, Implant_length from implants2 where Implant_id = '%1'"
			).arg(implant_id),
			db
		);
		QString currentImplantPath = qm.record(0).value("Implant_path").toString();
		float diameter = qm.record(0).value("Implant_diameter").toFloat();
		float length = qm.record(0).value("Implant_length").toFloat();
		db.close();

		curr_implant_row_ = implant_list_->currentRow();

		preview_->setImplant(manufacturerProduct, currentProduct, currentImplantPath, diameter, length);
#else
		UpdatePreview();
#endif
	}
}

void ImplantPreferenceDlg::Connections()
{
	connect(close_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotClose);
	connect(delete_all_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotDeleteAll);
	connect(apply_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotApply);
	connect(left_, &QRadioButton::toggled, this,
		&ImplantPreferenceDlg::slotLRChanged);
	for (int idx = 0; idx < kNumDisplayPresets; ++idx)
	{
		connect(preset_items_[idx], &ImplantPreferenceItem::sigClearCurrentItem,
			this, &ImplantPreferenceDlg::slotClearCurrentItem);
		connect(preset_items_[idx], &ImplantPreferenceItem::sigSetCurrentItemInfo,
			this, &ImplantPreferenceDlg::slotSetCurrentItemInfo);
	}

	connect(manufacturer_list_, &QTableWidget::currentCellChanged, this,
		&ImplantPreferenceDlg::slotUpdateProductTable);
	connect(product_list_, &QTableWidget::currentCellChanged, this,
		&ImplantPreferenceDlg::slotUpdateImplantTable);
	connect(implant_list_, &QTableWidget::cellClicked, this,
		&ImplantPreferenceDlg::slotClickedImplantTable);
}

void ImplantPreferenceDlg::Disconnections()
{
	disconnect(close_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotClose);
	disconnect(delete_all_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotDeleteAll);
	disconnect(apply_, &QToolButton::clicked, this,
		&ImplantPreferenceDlg::slotApply);
	disconnect(left_, &QRadioButton::toggled, this,
		&ImplantPreferenceDlg::slotLRChanged);
	for (int idx = 0; idx < kNumDisplayPresets; ++idx)
	{
		disconnect(preset_items_[idx], &ImplantPreferenceItem::sigClearCurrentItem,
			this, &ImplantPreferenceDlg::slotClearCurrentItem);
		disconnect(preset_items_[idx],
			&ImplantPreferenceItem::sigSetCurrentItemInfo, this,
			&ImplantPreferenceDlg::slotSetCurrentItemInfo);
	}

	disconnect(manufacturer_list_, &QTableWidget::currentCellChanged, this,
		&ImplantPreferenceDlg::slotUpdateProductTable);
	disconnect(product_list_, &QTableWidget::currentCellChanged, this,
		&ImplantPreferenceDlg::slotUpdateImplantTable);
	disconnect(implant_list_, &QTableWidget::cellClicked, this,
		&ImplantPreferenceDlg::slotClickedImplantTable);
}

void ImplantPreferenceDlg::LoadPresets()
{
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	int implant_index =
		current_preset_ == CurrentPreset::RIGHT ? kRightIndex : kLeftIndex;
	for (int idx = 0; idx < kNumDisplayPresets; ++idx)
	{
		ImplantPreferenceItem *item = preset_items_[idx];
		int implant_number = kImplantPreferenceNumbers[implant_index][idx];
		const QString section = kSectionName + QString::number(implant_number);
		const QString manufacturer_name =
			settings.value(section + kKeyManufacturer).toString();
		const QString product_name =
			settings.value(section + kKeyProduct).toString();
		const QString implant_name = settings.value(section + kKeyModel).toString();

		// 얻어온 문자열로 DB query.
		QString diameter, length;
		if (GetImplantSpecs(manufacturer_name, product_name, implant_name, diameter, length))
		{
			item->UpdateItemInfo(implant_number, manufacturer_name, product_name,
				diameter, length);
		}
		else
		{
			item->ResetItem(implant_number);
		}
	}
}

void ImplantPreferenceDlg::UpdatePreset(int preset_index)
{
	int implant_index =
		current_preset_ == CurrentPreset::RIGHT ? kRightIndex : kLeftIndex;
	int implant_number = kImplantPreferenceNumbers[implant_index][preset_index];

	const QString manufacturer_name =
		manufacturer_list_->item(manufacturer_list_->currentRow(), 0)->text();
	const QString product_name =
		product_list_->item(product_list_->currentRow(), 0)->text();
	const QString implant_name =
		implant_list_->item(implant_list_->currentRow(), 0)->text();
	const QString diameter = implant_list_->item(implant_list_->currentRow(), 1)
		->text()
		.split(kTokenSize)
		.at(0);
	const QString length = implant_list_->item(implant_list_->currentRow(), 2)
		->text()
		.split(kTokenSize)
		.at(0);
	preset_items_[preset_index]->UpdateItemInfo(implant_number, manufacturer_name,
		product_name, diameter, length);
	preset_items_[preset_index]->SyncSelectedStatus(false);

	// save preset data to INI file
	QString section = kSectionName + QString::number(implant_number);
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
	settings.setValue(section + kKeyManufacturer, manufacturer_name);
	settings.setValue(section + kKeyProduct, product_name);
	settings.setValue(section + kKeyModel, implant_name);
}

void ImplantPreferenceDlg::ClearPreset(int preset_index)
{
	int implant_index =
		current_preset_ == CurrentPreset::RIGHT ? kRightIndex : kLeftIndex;
	int implant_number = kImplantPreferenceNumbers[implant_index][preset_index];

	preset_items_[preset_index]->ResetItem(implant_number);
	if (preset_items_[preset_index]->current_selected())
		preset_items_[preset_index]->SyncSelectedStatus(false);

	slotClearCurrentItem(implant_number);
}

void ImplantPreferenceDlg::ClearTables()
{
	Disconnections();
	manufacturer_list_->setRowCount(0);
	product_list_->setRowCount(0);
	implant_list_->setRowCount(0);
	// preview에 memory release 하는 코드가 없어 빈 값을 넣어 초기화 하는 방식을
	// 선택함
	preview_->setImplant("", "", "", 0.0f, 0.0f);
	Connections();
}

void ImplantPreferenceDlg::ConnectDB()
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

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
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
		"DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;"
		"DATABASE=Implant;Port=1433;UID=sa;PWD=2002;WSID=.;");
#endif
#endif

	if (!db.open())
	{
		PrintDBErrMsg(db.lastError(), lang::LanguagePack::msg_77().toLocal8Bit());
		return;
	}
	db.close();
}

void ImplantPreferenceDlg::DisconnectDB()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (db.isOpen())
	{
		db.close();
		db = QSqlDatabase();
		QSqlDatabase::removeDatabase(kConnectionName);
	}
}

bool ImplantPreferenceDlg::GetImplantSpecs(const QString &manufacturer_name,
	const QString &product_name,
	const QString &implant_name,
	QString &diameter, QString &length)
{
	if (manufacturer_name.length() < 1 ||
		product_name.length() < 1 ||
		implant_name.length() < 1)
	{
		return false;
	}

	int manufacturer_id = QueryManufacturerID(manufacturer_name);
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (!db.open())
	{
		PrintDBErrMsg(db.lastError(), "DB open failed : ");
		return false;
	}

	bool is_custom_implant = (manufacturer_name[0] == kCustomImplantManufacturerNamePrefix);
	QString product_table_name = (is_custom_implant) ? "custom_product" : "product2";
	QString implant_table_name = (is_custom_implant) ? "custom_implants" : "implants2";
	QString diameter_column_name = (is_custom_implant) ? "coronal_diameter" : "Implant_diameter";
	QString length_column_name = (is_custom_implant) ? "length" : "Implant_length";

	QSqlQuery query(db);
	query.exec(QString("select product_id from %1"
		" where product_name = '%2' and manufacturer_id = '%3'")
		.arg(product_table_name)
		.arg(product_name)
		.arg(manufacturer_id));

	QString product_id;
	while (query.next()) product_id = query.value(0).toString();

	query.exec(QString("select * from %1"
		" where implant_name = '%2' and product_id = '%3'")
		.arg(implant_table_name)
		.arg(implant_name)
		.arg(product_id));

	while (query.next())
	{
		diameter = query.value(diameter_column_name).toString();
		length = query.value(length_column_name).toString();
	}
	db.close();

	if (diameter.isEmpty() || length.isEmpty()) return false;
	return true;
}

int ImplantPreferenceDlg::QueryManufacturerID(const QString & manufacturer_name)
{
	bool is_custom_implant = (manufacturer_name[0] == kCustomImplantManufacturerNamePrefix);
	QString manufacturer_table_name = (is_custom_implant) ? "custom_manufacturer" : "manufacturer2";
	QString new_manufacturer_name = manufacturer_name;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQuery query(db);
	query.exec(
		QString(
			"SELECT Manufacturer_id FROM %1 WHERE "
			"Manufacturer_name = '%2'"
		).arg(manufacturer_table_name)
		.arg(new_manufacturer_name.remove(kCustomImplantManufacturerNamePrefix))
	);

	int manufacturer_id;
	while (query.next()) manufacturer_id = query.value(0).toInt();
	db.close();
	return manufacturer_id;
}

void ImplantPreferenceDlg::UpdateManufacturerList(
	const QString &manufacturer_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	QSqlQueryModel manufacturer_name_qm;

#if 1
	manufacturer_name_qm.setQuery(
		QString("SELECT DISTINCT Manufacturer_name FROM Implant_Favorites"), db);
#else
	manufacturer_name_qm.setQuery(
		"select * from manufacturer2 where manufacturer_name"
		" in (select manufacturer_name from implant_favorites)"
		" ORDER BY Manufacturer_name ASC;",
		db);
#endif

	int rowCountManufacturer = manufacturer_name_qm.rowCount();
	manufacturer_list_->setRowCount(rowCountManufacturer);  // product table의 row 갯수를 지정해준다.

	int nSelectedIndex = 0;
	for (int i = 0; i < rowCountManufacturer; i++)
	{
		QString curr_manufacurer_name = manufacturer_name_qm.record(i).value("Manufacturer_name").toString();
		QTableWidgetItem *manufacturer_item = new QTableWidgetItem(curr_manufacurer_name);
		manufacturer_list_->setItem(i, 0, manufacturer_item);

		if (curr_manufacurer_name == manufacturer_name)
		{
			nSelectedIndex = i;
		}

		// Manufacturer id
		bool is_custom_implant = (curr_manufacurer_name[0] == kCustomImplantManufacturerNamePrefix);
		QString manufacturer_table_name = (is_custom_implant) ? "custom_manufacturer" : "manufacturer2";
		QSqlQueryModel manufacturer_id_qm;
		manufacturer_id_qm.setQuery(
			QString(
				"SELECT Manufacturer_id FROM %1 where Manufacturer_name='%2'"
			).arg(manufacturer_table_name).arg(curr_manufacurer_name.remove(kCustomImplantManufacturerNamePrefix)),
			db
		);

		QString manufacturer_id = kInvalidManufacturerID;
		if (manufacturer_id_qm.rowCount() > 0)
		{
			manufacturer_id = manufacturer_id_qm.record(0).value("Manufacturer_id").toString();
		}

		manufacturer_list_->setItem(
			i, 1,
			new QTableWidgetItem(manufacturer_id)
		);

		manufacturer_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (manufacturer_name.isEmpty())
		manufacturer_list_->setCurrentCell(0, 0);
	else
		manufacturer_list_->setCurrentCell(nSelectedIndex, 0);
}

void ImplantPreferenceDlg::UpdateProductList(
	const QString &selected_product_name)
{
	if (manufacturer_list_->rowCount() < 1 || manufacturer_list_->columnCount() < 2)
	{
		return;
	}

	QString manufacturer_name = manufacturer_list_->item(manufacturer_list_->currentRow(), 0)->text();
	QString manufacturer_id = manufacturer_list_->item(manufacturer_list_->currentRow(), 1)->text();
	if (manufacturer_id == kInvalidManufacturerID)
	{
		return;
	}

	bool is_custom_implant = (manufacturer_name[0] == kCustomImplantManufacturerNamePrefix);
	QString product_table_name = (is_custom_implant) ? "custom_product" : "product2";

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(
		QString("SELECT * FROM %1 WHERE Manufacturer_id='%2' AND"
			" product_name IN (SELECT product_name FROM implant_favorites)"
			" ORDER BY Product_name ASC")
		.arg(product_table_name).arg(manufacturer_id),
		db
	);

	int row_count = qm.rowCount();
	product_list_->setRowCount(row_count);  // product table의 row 갯수를 지정해준다.

	int curr_product_index = 0;
	for (int i = 0; i < row_count; i++)
	{
		// Product Name
		QString product_name = qm.record(i).value("Product_name").toString();
		QTableWidgetItem *product_name_item = new QTableWidgetItem(product_name);
		product_list_->setItem(i, 0, product_name_item);

		if (selected_product_name == product_name) curr_product_index = i;

		// Product id
		product_list_->setItem(
			i, 1,
			new QTableWidgetItem(qm.record(i).value("Product_id").toString())
		);

		product_list_->setRowHeight(i, kSectionHeight);
	}
	db.close();

	if (selected_product_name == "")
		product_list_->setCurrentCell(0, 0);
	else
		product_list_->setCurrentCell(curr_product_index, 0);
}

void ImplantPreferenceDlg::UpdateImplantList(const QString &diameter, const QString &length)
{
	if (product_list_->rowCount() < 1 || product_list_->columnCount() < 2)
	{
		return;
	}

	QString manufacturer_name = manufacturer_list_->item(manufacturer_list_->currentRow(), 0)->text();
	QString product_id = product_list_->item(product_list_->currentRow(), 1)->text();

	bool is_custom_implant = (manufacturer_name[0] == kCustomImplantManufacturerNamePrefix);
	QString implant_table_name = (is_custom_implant) ? "custom_implants" : "implants2";
	QString diameter_column_name = (is_custom_implant) ? "coronal_diameter" : "Implant_diameter";
	QString length_column_name = (is_custom_implant) ? "length" : "Implant_length";

	QString query = QString(
		"SELECT * FROM %1 WHERE Product_id='%2'"
		" ORDER BY %3 ASC, %4 ASC")
		.arg(implant_table_name)
		.arg(product_id)
		.arg(diameter_column_name)
		.arg(length_column_name);

	if (!is_custom_implant)
	{
		query += QString(", sub_category ASC");
	}

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QSqlQueryModel qm;
	qm.setQuery(query, db);

	int rowCountImplant = qm.rowCount();
	implant_list_->setRowCount(rowCountImplant);  // implant table의 row 갯수를 지정해준다.

	int curr_implant_index = 0;
	for (int i = 0; i < rowCountImplant; i++)
	{
		QTableWidgetItem* item_name = new QTableWidgetItem(qm.record(i).value("Implant_name").toString());
		implant_list_->setItem(i, 0, item_name);

		QString diameter_at = qm.record(i).value(diameter_column_name).toString();
		implant_list_->setItem(i, 1, new QTableWidgetItem(diameter_at + "mm"));

		QString length_at = qm.record(i).value(length_column_name).toString();
		implant_list_->setItem(i, 2, new QTableWidgetItem(length_at + "mm"));
		implant_list_->setItem(
			i, 3,
			new QTableWidgetItem(qm.record(i).value("Implant_id").toString())
		);

		implant_list_->setRowHeight(i, kSectionHeight);

		if (diameter_at == diameter && length_at == length)
		{
			curr_implant_index = i;
		}
	}
	db.close();

	if (!diameter.isEmpty() && !length.isEmpty())
	{
		implant_list_->setCurrentCell(curr_implant_index, 0);
	}

	UpdatePreview(false);
}

void ImplantPreferenceDlg::UpdatePreview(const bool active)
{
	QString manufacturer_name = manufacturer_list_->item(manufacturer_list_->currentRow(), 0)->text();
	QString product_name = product_list_->item(product_list_->currentRow(), 0)->text();

	bool is_custom_implant = (manufacturer_name[0] == kCustomImplantManufacturerNamePrefix);
	if (is_custom_implant || !active)
	{
		//preview_->setVisible(false);
		preview_->setImplant("", "", "", 0.0f, 0.0f);
		return;
	}

	QString implant_table_name = /*(is_custom_implant) ? "custom_implants" : */"implants2";
	QString diameter_column_name = /*(is_custom_implant) ? "coronal_diameter" : */"Implant_diameter";
	QString length_column_name = /*(is_custom_implant) ? "length" : */"Implant_length";

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	QString implant_id = implant_list_->item(implant_list_->currentRow(), 3)->text();
	QSqlQueryModel qm;
	qm.setQuery(
		QString(
			"SELECT * FROM %1 WHERE Implant_id = '%2'")
		.arg(implant_table_name)
		.arg(implant_id),
		db
	);
	float diameter = qm.record(0).value(diameter_column_name).toFloat();
	float length = qm.record(0).value(length_column_name).toFloat();

	QString implant_file_path = qm.record(0).value("Implant_path").toString();
	//preview_->setVisible(true);
	preview_->setImplant(manufacturer_name, product_name, implant_file_path, diameter, length);
	common::Logger::instance()->Print(common::LogType::INF, "Update Preview");

	db.close();
}

void ImplantPreferenceDlg::PrintDBErrMsg(const QSqlError &err, const char *msg)
{
	std::string err_msg = std::string(msg) + err.text().toStdString().c_str();
	common::Logger::instance()->Print(common::LogType::ERR, err_msg);
}
