#include "implant_panel.h"

#include <QtSql/qsqlqueryModel.h>
#include <QtSql/qsqlrecord.h>
#include <qabstractitemview.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qsqlquery.h>
#include <QRadioButton>
#include <QtConcurrent/QtConCurrent>

#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/global_preferences.h"

#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/implant_resource.h"

#include "W3ImplantDBDlg.h"
#include "implant_list_dialog.h"
#include "implant_preference_dialog.h"

using namespace implant_resource;

namespace
{
	const QString kLengthAndSubCategorySplitter("-");
	const QString kConnectionName("AddImplant");
	constexpr int kTotalImplantCnt = 28;
}  // end of namespace

ImplantPanel::ImplantPanel(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3ResourceContainer* Rcontainer, QWidget* pParent)
	: QFrame(pParent),
	m_pgVREngine(VREngine),
	m_pgMPREngine(MPRengine),
	m_pgRcontainer(Rcontainer),
	manufacturer_(new QComboBox),
	product_(new QComboBox),
	diameter_(new QComboBox),
	length_(new QComboBox),
	implant_library_(new QToolButton),
	implant_list_(new QToolButton),
	clear_all_(new QToolButton),
	preference_(new QToolButton)
{
	CW3ImplantDBDlg implant_db_dlg(m_pgVREngine, m_pgMPREngine, m_pgRcontainer, this);

	CW3Theme* theme = CW3Theme::getInstance();
	manufacturer_->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	manufacturer_->setStyleSheet(theme->appQComboBoxStyleSheet());
	product_->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	product_->setStyleSheet(theme->appQComboBoxStyleSheet());
	diameter_->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	diameter_->setStyleSheet(theme->appQComboBoxStyleSheet());
	length_->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	length_->setStyleSheet(theme->appQComboBoxStyleSheet());

	manufacturer_->setFixedWidth(140);
	product_->setFixedWidth(140);
	diameter_->setFixedWidth(68);
	length_->setFixedWidth(68);

	product_->view()->setMinimumWidth(250);

	implant_library_->setText(lang::LanguagePack::txt_library());
	implant_library_->setStyleSheet(theme->appToolButtonStyleSheet());

	clear_all_->setText(lang::LanguagePack::txt_clear_all());
	clear_all_->setStyleSheet(theme->appToolButtonStyleSheet());

	preference_->setText(lang::LanguagePack::txt_preference());
	preference_->setStyleSheet(theme->appToolButtonStyleSheet());

	implant_list_->setText(lang::LanguagePack::txt_list());
	implant_list_->setStyleSheet(theme->appToolButtonStyleSheet());

	// spec layout 구성
	QLabel* manufacturer_info = new QLabel(lang::LanguagePack::txt_manufacturer() + " :");
	manufacturer_info->setObjectName("InfoLabel");
	QLabel* product_info = new QLabel(lang::LanguagePack::txt_product() + " :");
	product_info->setObjectName("InfoLabel");
	QLabel* d_and_l_info = new QLabel(lang::LanguagePack::txt_dia() + "/" + lang::LanguagePack::txt_length() + "(mm) :");
	d_and_l_info->setObjectName("InfoLabel");

	QHBoxLayout* manufacturer_layout = new QHBoxLayout;
	manufacturer_layout->setContentsMargins(0, 0, 0, 0);
	manufacturer_layout->setSpacing(0);
	manufacturer_layout->addWidget(manufacturer_info);
	manufacturer_layout->addWidget(manufacturer_);

	QHBoxLayout* product_layout = new QHBoxLayout;
	product_layout->setContentsMargins(0, 0, 0, 0);
	product_layout->setSpacing(0);
	product_layout->addWidget(product_info);
	product_layout->addWidget(product_);

	QHBoxLayout* d_and_l_layout = new QHBoxLayout;
	d_and_l_layout->setContentsMargins(0, 0, 0, 0);
	d_and_l_layout->setSpacing(4);
	d_and_l_layout->addWidget(d_and_l_info);
	d_and_l_layout->addWidget(diameter_);
	d_and_l_layout->addWidget(length_);

	QVBoxLayout* spec_layout = new QVBoxLayout;
	spec_layout->setContentsMargins(0, 3, 0, 3);
	spec_layout->setSpacing(3);
	spec_layout->addLayout(manufacturer_layout);
	spec_layout->addLayout(product_layout);
	spec_layout->addLayout(d_and_l_layout);

	QGridLayout* control_btn_layout = new QGridLayout;
	control_btn_layout->setSpacing(10);
	control_btn_layout->setContentsMargins(0, 0, 0, 0);
	control_btn_layout->addItem(
		new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding), 0,
		0);
	control_btn_layout->addWidget(implant_library_, 1, 0);
	control_btn_layout->addWidget(implant_list_, 1, 1);
	control_btn_layout->addWidget(preference_, 2, 0);
	control_btn_layout->addWidget(clear_all_, 2, 1);
	control_btn_layout->addItem(
		new QSpacerItem(5, 5, QSizePolicy::Minimum, QSizePolicy::Expanding), 3,
		0);

	QHBoxLayout* control_panel_layout = new QHBoxLayout;
	control_panel_layout->setSpacing(10);
	control_panel_layout->setContentsMargins(0, 0, 0, 0);
	control_panel_layout->addLayout(spec_layout);
	control_panel_layout->addLayout(control_btn_layout);

	connectDB();
	updateManufacturer();
	updateProduct();
	UpdateDiameter();
	UpdateLength();
	LoadDisplayedPanelsImplantSpec();

	// Button implant
	QHBoxLayout* maxillary_btn_layout_ = new QHBoxLayout();  // 상악
	maxillary_btn_layout_->setAlignment(Qt::AlignCenter);
	maxillary_btn_layout_->setSpacing(13);
	maxillary_btn_layout_->setContentsMargins(0, 0, 0, 0);
	QHBoxLayout* mandibular_btn_layout_ = new QHBoxLayout();  // 하악
	mandibular_btn_layout_->setAlignment(Qt::AlignCenter);
	mandibular_btn_layout_->setSpacing(13);
	mandibular_btn_layout_->setContentsMargins(0, 0, 0, 0);

	for (int i = 0; i < MAX_IMPLANT; i++)
	{
		implant_buttons_[i].setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		implant_buttons_[i].setText(QString::number(kImplantNumbers[i]));
		implant_buttons_[i].setIndex(i);

		if (i < MAX_IMPLANT / 2)
			maxillary_btn_layout_->addWidget(&implant_buttons_[i]);
		else
			mandibular_btn_layout_->addWidget(&implant_buttons_[i]);
	}

	QVBoxLayout* implant_btn_layout = new QVBoxLayout();
	implant_btn_layout->setAlignment(Qt::AlignCenter);
	implant_btn_layout->setSpacing(13);
	implant_btn_layout->setContentsMargins(0, 0, 0, 0);
	implant_btn_layout->addLayout(maxillary_btn_layout_);
	implant_btn_layout->addLayout(mandibular_btn_layout_);

#if 0
	QVBoxLayout* arch_selection_layout = new QVBoxLayout();
	arch_selection_layout->setContentsMargins(0, 0, 0, 0);
	arch_selection_layout->setSpacing(0);

	QLabel* arch_selection_title =
		new QLabel(lang::LanguagePack::txt_arch_selection());
	arch_selection_title->setObjectName("ArchSelection");
	arch_selection_title->setSizePolicy(QSizePolicy::Preferred,
		QSizePolicy::Maximum);
	arch_selection_layout->addWidget(arch_selection_title);

	QVBoxLayout* arch_layout = new QVBoxLayout();
	arch_layout->setContentsMargins(0, 0, 0, 0);
	arch_layout->setSpacing(9);
	arch_maxilla_ = new QRadioButton();
	arch_maxilla_->setObjectName("InfoLabel");
	arch_maxilla_->setText(lang::LanguagePack::txt_maxilla());
	arch_layout->addWidget(arch_maxilla_);
	connect(arch_maxilla_, &QRadioButton::clicked,
		[=]() { emit sigArchSelectionChanged(ArchTypeID::ARCH_MAXILLA); });

	arch_mandible_ = new QRadioButton();
	arch_mandible_->setObjectName("InfoLabel");
	arch_mandible_->setText(lang::LanguagePack::txt_mandible());
	arch_layout->addWidget(arch_mandible_);
	arch_selection_layout->addLayout(arch_layout);
	connect(arch_mandible_, &QRadioButton::clicked,
		[=]() { emit sigArchSelectionChanged(ArchTypeID::ARCH_MANDLBLE); });
#endif

	button_spacer_ = new QSpacerItem(200, 5, QSizePolicy::Minimum, QSizePolicy::Minimum);
	implant_select_layout_ = new QHBoxLayout();
	implant_select_layout_->setSpacing(0);
	implant_select_layout_->setContentsMargins(0, 0, 0, 0);
	implant_select_layout_->addLayout(implant_btn_layout);

	black_line_ = new QFrame;
	black_line_->setObjectName("blackline");
	black_line_->setFrameStyle(QFrame::VLine | QFrame::Raised);

	QHBoxLayout* left_layout = new QHBoxLayout();
	left_layout->setSpacing(0);
	left_layout->setContentsMargins(0, 0, 0, 0);
#if 0
	left_layout->addLayout(arch_selection_layout);
	left_layout->addWidget(black_line_);
#endif
	left_layout->addLayout(implant_select_layout_, 1);

	QHBoxLayout* main_layout = new QHBoxLayout;
	main_layout->setSpacing(10);
	main_layout->setContentsMargins(20, 10, 10, 20);
	main_layout->addLayout(left_layout, 1);
	//main_layout->setStretch(0, 2);

	white_line_ = new QFrame();
	white_line_->setObjectName("whiteline");
	white_line_->setFrameShape(QFrame::VLine);
	main_layout->addWidget(white_line_);

	main_layout->addLayout(control_panel_layout);
	//main_layout->setStretch(2, 1);

	this->setObjectName("AddImplant");
	this->setLayout(main_layout);
	this->setVisible(false);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->setStyleSheet(theme->AddImplantStyleSheet());

	connections();

	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();
	bool is_maximize = global_preferences->preferences_.general.interfaces.is_maximize;
	SetApplicationUIMode(is_maximize);
}

ImplantPanel::~ImplantPanel() {}

void ImplantPanel::SetApplicationUIMode(const bool& is_maximize)
{
	return;

	if (is_maximize)
	{
		implant_select_layout_->addItem(button_spacer_);
	}
	else
	{
		implant_select_layout_->removeItem(button_spacer_);
	}
	white_line_->setVisible(is_maximize);
	black_line_->setVisible(is_maximize);
}

void ImplantPanel::importProject()
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& implant_datas = res_implant.data();
	if (implant_datas.empty()) return;

	for (const auto& data : implant_datas)
	{
//===
//20250210 LIN 모든 button seleted상태 아니게 만듬
#ifndef WILL3D_VIEWER
		SetImplantButtonStatusSelected(data.second->id());
#else
		for (int index = 0; index < kTotalImplantCnt; ++index)
		{
			if (kImplantNumbers[index] == data.second->id())
			{
				SetImplantSpecWithButtonIndex(index);
				implant_buttons_[index].SetStatus(ImplantButton::ButtonStatus::PLACED);
				SyncSelectedImplantButton(index);
			}
		}
#endif
	}
//20250210 LIN
#ifndef WILL3D_VIEWER
	SetImplantButtonStatusSelected(res_implant.selected_implant_id());
#endif
//===
}

void ImplantPanel::reset()
{
	for (int i = 0; i < MAX_IMPLANT; i++)
		implant_buttons_[i].SetStatus(ImplantButton::ButtonStatus::DEFAULT);

	LoadDisplayedPanelsImplantSpec();
}

QString ImplantPanel::GetCurrentImplantFilePath() const
{
	return implant_file_path_;
}
QString ImplantPanel::GetCurrentManufacturer() const
{
	return manufacturer_->currentText();
}
QString ImplantPanel::GetCurrentProduct() const
{
	return product_->currentText();
}
QString ImplantPanel::GetCurrentSubCategory()
{
	return GetSubCategoryFromComboBoxItem(length_->currentText());
}
float ImplantPanel::GetCurrentDiameter() const
{
	return diameter_->currentText().toFloat();
}
float ImplantPanel::GetCurrentLength()
{
	return GetLengthFromComboBoxItem(length_->currentText()).toFloat();
}

void ImplantPanel::SetImplantButtonStatusSelected(int implant_id)
{
	for (int index = 0; index < kTotalImplantCnt; ++index)
	{
		if (kImplantNumbers[index] == implant_id)
		{
			SetImplantSpecWithButtonIndex(index);
			implant_buttons_[index].SetStatus(ImplantButton::ButtonStatus::SELECTED);
			SyncSelectedImplantButton(index);
			break;
		}
	}
}

void ImplantPanel::SetImplantButtonStatusDefault(int implant_id)
{
	for (int index = 0; index < kTotalImplantCnt; ++index)
	{
		if (kImplantNumbers[index] == implant_id)
		{
			implant_buttons_[index].SetStatus(ImplantButton::ButtonStatus::DEFAULT);
			break;
		}
	}
}

void ImplantPanel::SetImplantSpecAsPrevModel(int implant_id)
{
	for (int index = 0; index < kTotalImplantCnt; ++index)
	{
		if (kImplantNumbers[index] == implant_id)
		{
			SetImplantSpecWithButtonIndex(index);
			break;
		}
	}
}

void ImplantPanel::SyncArchType(const ArchTypeID& arch_type)
{
#if 0
	arch_maxilla_->blockSignals(true);
	arch_mandible_->blockSignals(true);

	if (arch_type == ArchTypeID::ARCH_MANDLBLE)
		arch_mandible_->setChecked(true);
	else if (arch_type == ArchTypeID::ARCH_MAXILLA)
		arch_maxilla_->setChecked(true);

	arch_maxilla_->blockSignals(false);
	arch_mandible_->blockSignals(false);
#endif
}

void ImplantPanel::connections()
{
	connect(manufacturer_, SIGNAL(currentIndexChanged(QString)), this,
		SLOT(slotImplantComboChanged_Manufacturer(QString)));
	connect(product_, SIGNAL(currentIndexChanged(QString)), this,
		SLOT(slotImplantComboChanged_Product(QString)));
	connect(diameter_, SIGNAL(currentIndexChanged(QString)), this,
		SLOT(slotImplantComboChanged_Diameter(QString)));
	connect(length_, SIGNAL(currentIndexChanged(QString)), this,
		SLOT(slotImplantComboChanged_Length(QString)));

	// Implant 버튼 클릭시 해당하는 Implant index를 slotCliked보내줌.
	for (int i = 0; i < MAX_IMPLANT; i++)
	{
		connect(&implant_buttons_[i], SIGNAL(sigAddImplant(int)), this,
			SLOT(slotAddImplantFromButton(int)));
		connect(
			&implant_buttons_[i],
			SIGNAL(sigSelectImplant(int, ImplantButton::ButtonStatus)), this,
			SLOT(slotSelectImplantFromButton(int, ImplantButton::ButtonStatus)));
		connect(&implant_buttons_[i], SIGNAL(sigDeleteImplant(int)), this,
			SLOT(slotDeleteImplantFromButton(int)));
	}

	connect(implant_library_, SIGNAL(clicked()), this,
		SLOT(slotImplantDBSetting()));
	connect(clear_all_, SIGNAL(clicked()), this, SLOT(slotDeleteAllImplants()));
	connect(preference_, SIGNAL(clicked()), this,
		SLOT(slotShowPreferenceDialog()));
	connect(implant_list_, SIGNAL(clicked()), this,
		SLOT(slotShowImplantListDialog()));
}

//제조사 등록
void ImplantPanel::updateManufacturer()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QSqlQueryModel query;
	query.setQuery(
		QString("SELECT DISTINCT Manufacturer_name FROM Implant_Favorites"), db);

	QString manufacturer_namer = "";
	manufacturer_->clear();
	for (int i = 0; i < query.rowCount(); i++)
	{
		manufacturer_namer = query.record(i).value("Manufacturer_name").toString();

		bool is_custom_implant = manufacturer_namer[0] == kCustomImplantManufacturerNamePrefix;

		if (existDBRecord(manufacturer_namer, is_custom_implant))
		{
			manufacturer_->addItem(manufacturer_namer);
		}
	}
	query.clear();
	db.close();

	manufacturer_->blockSignals(true);
	manufacturer_->model()->sort(0);
	manufacturer_->blockSignals(false);
}

//선택된 제조사의 제품 등록
void ImplantPanel::updateProduct()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString currentManufacturer = manufacturer_->currentText();
	QSqlQueryModel query;
	query.setQuery(QString("SELECT Product_name FROM Implant_Favorites WHERE "
		"Manufacturer_name='%1'")
		.arg(currentManufacturer),
		db);

	bool is_custom_implant = currentManufacturer[0] == kCustomImplantManufacturerNamePrefix;

	QString strProduct = "";
	product_->clear();
	for (int i = 0; i < query.rowCount(); i++)
	{
		strProduct = query.record(i).value("Product_name").toString();

		if (existDBRecord(currentManufacturer, strProduct, is_custom_implant))
		{
			product_->addItem(strProduct);
		}
	}
	query.clear();
	db.close();
}

//선택된 제조사의 제품의 Implant diameter 등록
void ImplantPanel::UpdateDiameter()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);
	diameter_->clear();

	QString implants_table_name;
	QString diameter_column_name;
	if (is_custom_implant)
	{
		implants_table_name = "custom_implants";
		diameter_column_name = "coronal_diameter";
	}
	else
	{
		implants_table_name = "Implants2";
		diameter_column_name = "Implant_diameter";
	}

	QSqlQueryModel query;
	query.setQuery(
		QString(
			"SELECT DISTINCT %2 FROM %1 WHERE Product_id = '%3' ORDER BY %2 ASC"
		).arg(implants_table_name).arg(diameter_column_name).arg(product_id),
		db
	);

	QString strDiameter = "";
	for (int i = 0; i < query.rowCount(); i++)
	{
		strDiameter = query.record(i).value(diameter_column_name).toString();
		diameter_->addItem(strDiameter);
	}

	query.clear();
	db.close();
}

//선택된 제조사의 제품의 Implant diameter의  Implant Length 등록
void ImplantPanel::UpdateLength()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);

	QString length_column_name;
	QString query_str;
	if (is_custom_implant)
	{
		length_column_name = "length";
		query_str = QString(
			"SELECT %3 FROM custom_implants"
			" WHERE Product_id = '%1' AND coronal_diameter = '%2'"
			" ORDER BY %3 ASC")
			.arg(product_id)
			.arg(diameter_->currentText())
			.arg(length_column_name);
	}
	else
	{
		length_column_name = "Implant_length";
		query_str = QString(
			"SELECT %3, sub_category FROM Implants2"
			" WHERE Product_id = '%1' AND Implant_diameter = '%2'"
			" ORDER BY %3 ASC, sub_category ASC")
			.arg(product_id)
			.arg(diameter_->currentText())
			.arg(length_column_name);
	}

	QSqlQueryModel query;
	query.setQuery(query_str, db);

	length_->clear();
	for (int i = 0; i < query.rowCount(); i++)
	{
		QString length_string = query.record(i).value(length_column_name).toString();
		QString sub_category_string;
		if (!is_custom_implant)
		{
			sub_category_string = query.record(i).value("sub_category").toString();
		}

		length_->addItem(MakeLengthPlusSubCategoryFormat(length_string, sub_category_string));
	}

	query.clear();
	db.close();
}

void ImplantPanel::UpdateImplantInfoAndChangeModel()
{
	UpdateImplantInfo();
	ChangeSelectedImplantModel();
}

void ImplantPanel::UpdateImplantInfo()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);

	QSqlQueryModel query;
	QString curr_implant_path;
	QString currentDiameter = diameter_->currentText();
	QString currentLength = GetLengthFromComboBoxItem(length_->currentText());

	if (is_custom_implant)
	{
		curr_implant_path = QString(":/stl/custom_implant.wim");

		QString select_query = QString(
			"SELECT apical_diameter"
			" FROM custom_implants WHERE Product_id = '%1'"
			" AND coronal_diameter = '%2'"
			" AND length = '%3'")
			.arg(product_id)
			.arg(currentDiameter)
			.arg(currentLength);

		query.setQuery(select_query, db);

		platform_diameter_ = currentDiameter.toFloat();
		custom_apical_diameter_ = query.record(0).value("apical_diameter").toFloat();
		total_length_ = currentLength.toFloat();
	}
	else
	{
		QString sub_category = GetSubCategoryFromComboBoxItem(length_->currentText());

		QString select_query = QString("SELECT Implant_path, platform_diameter, total_length"
			" FROM Implants2 WHERE Product_id = '%1'"
			" AND Implant_diameter = '%2'"
			" AND Implant_length = '%3'")
			.arg(product_id)
			.arg(currentDiameter)
			.arg(currentLength);
		if (!sub_category.isEmpty())
		{
			select_query += QString(" AND sub_category = '%1'").arg(sub_category);
		}
		select_query += QString(" ORDER BY Implant_diameter ASC");

		query.setQuery(select_query, db);

		int cur_index = length_->currentIndex();
		int record_index = 0;
		if (cur_index > 0)
		{
			QString cur_value = length_->itemText(cur_index);

			int prev_index = cur_index - 1;
			
			while (true)
			{
				QString prev_value = length_->itemText(prev_index);				
				if (prev_value.compare(cur_value) == 0)
				{
					++record_index;
					--prev_index;
				}
				else
				{
					break;
				}
			}
		}

		curr_implant_path = query.record(record_index).value("Implant_path").toString();
		QVariant platform_daimeter = query.record(record_index).value("platform_diameter");
		QVariant total_length = query.record(record_index).value("total_length");

		if (platform_daimeter.isNull())
		{
			platform_diameter_ = currentDiameter.toFloat();
		}
		else
		{
			platform_diameter_ = platform_daimeter.toFloat();
		}

		if (total_length.isNull())
		{
			total_length_ = currentLength.toFloat();
		}
		else
		{
			total_length_ = total_length.toFloat();
		}
	}

	query.clear();
	db.close();

	implant_file_path_ = curr_implant_path;

	common::Logger::instance()->Print(
		common::LogType::DBG,
		std::string("Implant File Path = ") + curr_implant_path.toStdString()
	);
	common::Logger::instance()->Print(
		common::LogType::DBG,
		std::string("Diamter = ") + currentDiameter.toStdString()
	);
	common::Logger::instance()->Print(
		common::LogType::DBG,
		std::string("Length =  ") + currentLength.toStdString()
	);
}

void ImplantPanel::ChangeSelectedImplantModel()
{
	int implant_id = -1;
	QString sub_category = GetSubCategoryFromComboBoxItem(length_->currentText());

#if 1
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int add_implant_id = res_implant.add_implant_id();
	if (add_implant_id > -1)
	{
		implant_id = add_implant_id;
	}
	else
	{
		IsSelectedImplantModelChanged(implant_file_path_, implant_id);
	}

	if (implant_id > -1)
	{
		common::Logger::instance()->Print(common::LogType::DBG, "implant model changed");

		QString manufacturer = GetCurrentManufacturer();
		bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;

		// 선택된 임플란트 스펙으로 로드
		if (is_custom_implant)
		{
			implant_resource::ImplantInfo implant_params(
				GetCurrentImplantFilePath(),
				implant_id,
				GetCurrentManufacturer(),
				GetCurrentProduct(),
				GetCurrentDiameter(),
				custom_apical_diameter_,
				GetCurrentLength()
			);

			emit sigChangeImplant(implant_params);
		}
		else
		{
			implant_resource::ImplantInfo implant_params(
				GetCurrentImplantFilePath(), implant_id, manufacturer,
				GetCurrentProduct(), GetCurrentDiameter(), GetCurrentLength(),
				platform_diameter_, total_length_, sub_category);

			emit sigChangeImplant(implant_params);
		}
	}
#else
	if (IsSelectedImplantModelChanged(implant_file_path_, implant_id))
	{
		common::Logger::instance()->Print(common::LogType::DBG,
			"implant model changed");

		// 선택된 임플란트 스펙으로 로드
		implant_resource::ImplantInfo implant_params(
			GetCurrentImplantFilePath(), implant_id, GetCurrentManufacturer(),
			GetCurrentProduct(), GetCurrentDiameter(), GetCurrentLength(),
			platform_diameter_, total_length_, sub_category);
		emit sigChangeImplant(implant_params);
	}
#endif
}

void ImplantPanel::GetImplantSpecs(
	const QString& product_name,
	const QString& implant_name,
	QString& diameter,
	QString& length,
	QString& platform_diameter,
	QString& total_length,
	QString& sub_category)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);

	QString implant_table_name = (is_custom_implant) ? "custom_implants" : "implants2";
	QString diameter_column_name = (is_custom_implant) ? "coronal_diameter" : "Implant_diameter";
	QString length_column_name = (is_custom_implant) ? "length" : "Implant_length";

	QSqlQuery query(db);
	query.exec(
		QString(
			"select * from %1 where implant_name ='%2' and product_id ='%3'")
		.arg(implant_table_name)
		.arg(implant_name)
		.arg(product_id));

	QString curr_implant_path = QString(":/stl/custom_implant.wim");

	while (query.next())
	{
		diameter = query.value(diameter_column_name).toString();
		length = query.value(length_column_name).toString();

		if (!is_custom_implant)
		{
			curr_implant_path = query.value("implant_path").toString();
			sub_category = query.value("sub_category").toString();

			QVariant platform_diameter_value = query.value("platform_diameter");
			QVariant total_length_value = query.value("total_length");

			if (platform_diameter_value.isNull())
			{
				platform_diameter = diameter;
			}
			else
			{
				platform_diameter = platform_diameter_value.toString();
			}

			if (total_length_value.isNull())
			{
				total_length = length;
			}
			else
			{
				total_length = total_length_value.toString();
			}
		}
		else
		{
			platform_diameter = diameter;
			total_length = length;
		}
	}

	implant_file_path_ = curr_implant_path;
	db.close();
}

void ImplantPanel::GetImplantName(const QString& product_name,
	const QString& diameter,
	const QString& length,
	const QString& sub_category,
	QString& implant_name)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);

	QString select_query = QString("select implant_name from implants2"
		" where implant_diameter = '%1' and implant_length = '%2' "
		"and product_id = '%3'")
		.arg(diameter)
		.arg(length)
		.arg(product_id);
	if (!sub_category.isEmpty())
	{
		select_query += QString(" and sub_category = '%1'").arg(sub_category);
	}

	QSqlQuery query(db);
	query.exec(select_query);

	while (query.next())
	{
		implant_name = query.value("implant_name").toString();
	}
	db.close();
}

QString ImplantPanel::GetCustomImplantName(
	const QString& product_name,
	const QString& coronal_diameter,
	const QString& apical_diameter,
	const QString& length
)
{
	QString implant_name;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();

	QString manufacturer = manufacturer_->currentText();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	int manufacturer_id = QueryManufacturerID(manufacturer, is_custom_implant, db);
	int product_id = QueryProductID(manufacturer_id, product_->currentText(), is_custom_implant, db);

	QString select_query = QString(
		"SELECT implant_name FROM custom_implants WHERE"
		" coronal_diameter = '%1' and"
		" apical_diameter = '%2' and"
		" implant_length = '%3' and"
		" product_id = '%4'")
		.arg(coronal_diameter)
		.arg(apical_diameter)
		.arg(length)
		.arg(product_id);

	QSqlQuery query(db);
	query.exec(select_query);

	while (query.next())
	{
		implant_name = query.value("implant_name").toString();
	}
	db.close();

	return implant_name;
}

int ImplantPanel::QueryManufacturerID(const QString& manufacturer_name, const bool is_custom_implant, QSqlDatabase& db)
{
	QString manufacturer = manufacturer_name;
	QString manufacturer_table_name = is_custom_implant ? "custom_manufacturer" : "Manufacturer2";

	QSqlQuery query(db);
	query.exec(QString("SELECT Manufacturer_id FROM %1 WHERE "
		"Manufacturer_name = '%2'")
		.arg(manufacturer_table_name)
		.arg(manufacturer.remove(kCustomImplantManufacturerNamePrefix)));

	int manufacturer_id;
	while (query.next()) manufacturer_id = query.value(0).toInt();
	return manufacturer_id;
}

int ImplantPanel::QueryProductID(const int manufacturer_id, const QString& product_name, const bool is_custom_implant, QSqlDatabase& db)
{
	QString product_table_name = is_custom_implant ? "custom_product" : "Product2";

	QSqlQuery query(db);
	query.exec(QString("SELECT Product_id FROM %1"
		" WHERE Manufacturer_id = '%2' AND Product_name = '%3'")
		.arg(product_table_name)
		.arg(manufacturer_id)
		.arg(product_->currentText()));

	int product_id;
	while (query.next()) product_id = query.value(0).toInt();
	return product_id;
}

void ImplantPanel::connectDB()
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
		"SERVER=127.0.0.1;"
		"DATABASE=Implant;"
		"Port=%2;"
		"UID=root;"
		"PWD=2002;"
		"WSID=.;")
		.arg(driver_name)
		.arg(port)
	);
#else
	db.setDatabaseName(
		"DRIVER={SQL Server};"
		"SERVER=127.0.0.1\\HDXWILL2014;"
		"DATABASE=Implant;"
		"Port=1433;"
		"UID=sa;"
		"PWD=2002;"
		"WSID=.;"
	);
#endif
#endif
	if (!db.open())
	{
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_77(),
			CW3MessageBox::Critical);
		msgBox.exec();
	}
	db.close();
}

void ImplantPanel::disconnectDB()
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	db.open();
	if (db.isOpen())
	{
		db.close();
		db = QSqlDatabase();
		db.removeDatabase(kConnectionName);
	}
}

void ImplantPanel::BlockComboboxSignals(const bool& block)
{
	manufacturer_->blockSignals(block);
	product_->blockSignals(block);
	diameter_->blockSignals(block);
	length_->blockSignals(block);
}

// 연속적으로 Query 문 내부에서 사용하는 함수로 db open & close 는 외부에서 해
// 준다.
bool ImplantPanel::existDBRecord(const QString& manufacturer_name, const QString& product_name, const bool is_custom_implant)
{
	QString product_table_name = is_custom_implant ? "custom_product" : "Product2";

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	int manufacturer_id = QueryManufacturerID(manufacturer_name, is_custom_implant, db);
	QSqlQueryModel query;
	query.setQuery(
		QString("SELECT count(*) as cnt FROM %1"
			" WHERE Manufacturer_id = '%2' AND Product_name = '%3'")
		.arg(product_table_name)
		.arg(manufacturer_id)
		.arg(product_name),
		db);

	int table_count = query.record(0).value("cnt").toInt();
	return table_count > 0 ? true : false;
}

// 연속적으로 Query 문 내부에서 사용하는 함수로 db open & close 는 외부에서 해
// 준다.
bool ImplantPanel::existDBRecord(const QString& manufacturer_name, const bool is_custom_implant)
{
	QString manufacturer = manufacturer_name;
	QString manufacturer_table_name = is_custom_implant ? "custom_manufacturer" : "Manufacturer2";

	QSqlQueryModel query;
	query.setQuery(QString("SELECT count(*) as cnt FROM %1 WHERE "
		"Manufacturer_name = '%2'")
		.arg(manufacturer_table_name)
		.arg(manufacturer.remove(kCustomImplantManufacturerNamePrefix)),
		QSqlDatabase::database(kConnectionName));

	// table 갯수, 0이면 해당 테이블이 없음.
	return query.record(0).value("cnt").toInt() != 0 ? true : false;
}

void ImplantPanel::SyncSelectedImplantButton(int index)
{
	for (int i = 0; i < MAX_IMPLANT; ++i)
	{
		if (i == index) continue;

		if (implant_buttons_[i].IsSelected())
		{
			implant_buttons_[i].SetStatus(ImplantButton::ButtonStatus::PLACED);
		}
	}
}

void ImplantPanel::SetImplantSpecWithButtonIndex(int index)
{
	int selected_id = kImplantNumbers[index];
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& data = res_implant.data().at(selected_id).get();

	// 찾은 데이터를 포함하는 UI 세팅. 이 때, disconnect combobox signal & slot
	BlockComboboxSignals(true);
	QString manufacurer = data->manufacturer();
	manufacturer_->setCurrentIndex(manufacturer_->findText(manufacurer));

	updateProduct();
	QString product = data->product();
	product_->setCurrentIndex(product_->findText(product));

	UpdateDiameter();
	QString diameter = QString::number(data->diameter());
	diameter_->setCurrentIndex(diameter_->findText(diameter));

	UpdateLength();
	QString length = QString::number(data->length());
	length_->setCurrentIndex(length_->findText(MakeLengthPlusSubCategoryFormat(length, data->sub_category())));
	BlockComboboxSignals(false);

	UpdateImplantInfo();
}

bool ImplantPanel::IsSelectedImplantModelChanged(const QString& implant_path,
	int& selected_id)
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	selected_id = res_implant.selected_implant_id();
	if (selected_id < 0) return false;

	const auto& res_implant_data = res_implant.data();
	if (res_implant_data.find(selected_id) == res_implant_data.end())
	{
		return false;
	}

	const auto& data = res_implant_data.at(selected_id).get();
	if (data->file_path().compare(implant_path,
		Qt::CaseSensitivity::CaseInsensitive) != 0)
		return true;
	return false;
}

void ImplantPanel::LoadPreset(int implant_index)
{
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	QString section = implant_preset::kSectionName +
		QString::number(kImplantNumbers[implant_index]);
	QString manufacturer_name =
		settings.value(section + implant_preset::kKeyManufacturer).toString();
	QString product_name =
		settings.value(section + implant_preset::kKeyProduct).toString();
	QString implant_name =
		settings.value(section + implant_preset::kKeyModel).toString();

	if (manufacturer_name.isEmpty()) return;

	int prev_manufacturer_idx_for_rollback = manufacturer_->currentIndex();
	// 찾은 데이터가 현재 favorites 에서 없다면, Preset 세팅하지 않음
	if (manufacturer_->findText(manufacturer_name) < 0)
	{
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_78(),
			CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	// 찾은 데이터를 포함하는 UI 세팅. 이 때, disconnect combobox signal & slot
	BlockComboboxSignals(true);
	manufacturer_->setCurrentIndex(manufacturer_->findText(manufacturer_name));

	updateProduct();
	if (product_->findText(product_name) < 0)
	{
		manufacturer_->setCurrentIndex(prev_manufacturer_idx_for_rollback);
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_78(),
			CW3MessageBox::Information);
		msgBox.exec();
		return;
	}
	product_->setCurrentIndex(product_->findText(product_name));

	QString diameter, length, platform_diameter, total_length, sub_category;
	GetImplantSpecs(product_name, implant_name, diameter, length, platform_diameter, total_length, sub_category);

	platform_diameter_ = platform_diameter.toFloat();
	total_length_ = total_length.toFloat();

	UpdateDiameter();
	diameter_->setCurrentIndex(diameter_->findText(diameter));

	UpdateLength();
	length_->setCurrentIndex(length_->findText(MakeLengthPlusSubCategoryFormat(length, sub_category)));
	BlockComboboxSignals(false);
}

void ImplantPanel::LoadDisplayedPanelsImplantSpec()
{
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	using namespace implant_preset;
	QString manufacturer_name =
		settings.value(kDisplayedPanelsSectionName + kKeyManufacturer).toString();
	QString product_name =
		settings.value(kDisplayedPanelsSectionName + kKeyProduct).toString();
	QString implant_name =
		settings.value(kDisplayedPanelsSectionName + kKeyModel).toString();

	if (manufacturer_name.isEmpty() || product_name.isEmpty() ||
		implant_name.isEmpty())
		return;

	BlockComboboxSignals(true);
	manufacturer_->setCurrentIndex(manufacturer_->findText(manufacturer_name));

	updateProduct();
	product_->setCurrentIndex(product_->findText(product_name));

	QString diameter, length, platform_diameter, total_length, sub_category;
	GetImplantSpecs(product_name, implant_name, diameter, length, platform_diameter, total_length, sub_category);

	UpdateDiameter();
	diameter_->setCurrentIndex(diameter_->findText(diameter));

	UpdateLength();
	length_->setCurrentIndex(length_->findText(MakeLengthPlusSubCategoryFormat(length, sub_category)));
	BlockComboboxSignals(false);

	UpdateImplantInfo();
}

void ImplantPanel::SaveDisplayedPanelsImplantSpec()
{
	QString curr_manufacturer = manufacturer_->currentText();
	if (curr_manufacturer.isEmpty())
	{
		return;
	}

	bool is_custom_implant = curr_manufacturer[0] == kCustomImplantManufacturerNamePrefix;
	if (is_custom_implant)
	{
		return;
	}

	QString curr_product = product_->currentText();
	QString curr_diameter = diameter_->currentText();
	QString curr_length = GetLengthFromComboBoxItem(length_->currentText());
	QString sub_category = GetSubCategoryFromComboBoxItem(length_->currentText());
	QString curr_implant_name;
	GetImplantName(curr_product, curr_diameter, curr_length, sub_category, curr_implant_name);

	if (curr_implant_name.isEmpty())
	{
		return;
	}

	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	using namespace implant_preset;
	settings.setValue(kDisplayedPanelsSectionName + kKeyManufacturer,
		curr_manufacturer);
	settings.setValue(kDisplayedPanelsSectionName + kKeyProduct, curr_product);
	settings.setValue(kDisplayedPanelsSectionName + kKeyModel, curr_implant_name);
}

void ImplantPanel::slotAddImplantFromButton(int index)
{
	// implant preference 있으면 로드
	LoadPreset(index);

	QString manufacturer = GetCurrentManufacturer();
	bool is_custom_implant = manufacturer[0] == kCustomImplantManufacturerNamePrefix;

	// 선택된 임플란트 스펙으로 로드
	if (is_custom_implant)
	{
		implant_resource::ImplantInfo implant_params(
			GetCurrentImplantFilePath(),
			kImplantNumbers[index],
			GetCurrentManufacturer(),
			GetCurrentProduct(),
			GetCurrentDiameter(),
			custom_apical_diameter_,
			GetCurrentLength()
		);

		emit sigAddImplant(implant_params);
	}
	else
	{
		implant_resource::ImplantInfo implant_params(
			GetCurrentImplantFilePath(), kImplantNumbers[index],
			GetCurrentManufacturer(), GetCurrentProduct(),
			GetCurrentDiameter(), GetCurrentLength(),
			platform_diameter_, total_length_, GetCurrentSubCategory());

		emit sigAddImplant(implant_params);
	}

#if 0
	slotImplantSelected(kImplantNumbers[index]);
#else
	SyncSelectedImplantButton(index);
#endif
}

void ImplantPanel::slotSelectImplantFromButton(
	int index, ImplantButton::ButtonStatus status)
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int add_implant_id = res_implant.add_implant_id();
	if (add_implant_id > -1)
	{
		emit sigCancelAddImplant();
	}
	else
	{
		SetImplantSpecWithButtonIndex(index);

		bool selected =
			status == ImplantButton::ButtonStatus::SELECTED ? true : false;
		emit sigSelectImplant(kImplantNumbers[index], selected);
		SyncSelectedImplantButton(index);
	}
}

void ImplantPanel::slotDeleteImplantFromButton(int index)
{
	emit sigDeleteImplant(kImplantNumbers[index]);
}

void ImplantPanel::slotImplantSelected(int implant_id)
{
	emit sigSelectImplant(implant_id, true);
	SetImplantButtonStatusSelected(implant_id);
}

void ImplantPanel::slotImplantDBSetting()
{
	disconnectDB();

	bool updated = false;
	// implant_db_dlg 의 DB connection 범위를 제한하기 위해
	{
		CW3ImplantDBDlg implant_db_dlg(m_pgVREngine, m_pgMPREngine, m_pgRcontainer, this);
		connect(&implant_db_dlg, SIGNAL(sigFavoriteUpdated()), this, SLOT(slotFavoriteUpdated()));
		updated = implant_db_dlg.exec();
	}

	connectDB();

	if (updated)
	{
		BlockComboboxSignals(true);
		updateManufacturer();
		updateProduct();
		UpdateDiameter();
		UpdateLength();
		UpdateImplantInfo();
		BlockComboboxSignals(false);
	}
}

void ImplantPanel::slotDeleteAllImplants()
{
	for (int i = 0; i < MAX_IMPLANT; i++)
		implant_buttons_[i].SetStatus(ImplantButton::ButtonStatus::DEFAULT);
	emit sigDeleteAllImplants();
}

void ImplantPanel::slotImplantComboChanged_Manufacturer(
	const QString& manufacturer_namer)
{
	BlockComboboxSignals(true);
	updateProduct();
	UpdateDiameter();
	UpdateLength();
	UpdateImplantInfoAndChangeModel();
	BlockComboboxSignals(false);
	SaveDisplayedPanelsImplantSpec();
}

void ImplantPanel::slotImplantComboChanged_Product(const QString& strProduct)
{
	BlockComboboxSignals(true);
	UpdateDiameter();
	UpdateLength();
	UpdateImplantInfoAndChangeModel();
	BlockComboboxSignals(false);
	SaveDisplayedPanelsImplantSpec();
}

void ImplantPanel::slotImplantComboChanged_Diameter(
	const QString& strDiameter)
{
	BlockComboboxSignals(true);

#if 1
	QString previous_length_and_sub_category = length_->currentText();

	UpdateLength();

	int length_combo_index = length_->findText(previous_length_and_sub_category);
	if (length_combo_index > -1 && length_combo_index < length_->count())
	{
		length_->setCurrentIndex(length_combo_index);
	}
#else
	UpdateLength();
#endif

	UpdateImplantInfoAndChangeModel();
	BlockComboboxSignals(false);
	SaveDisplayedPanelsImplantSpec();
}

void ImplantPanel::slotImplantComboChanged_Length(const QString& strLength)
{
	BlockComboboxSignals(true);
	UpdateImplantInfoAndChangeModel();
	BlockComboboxSignals(false);
	SaveDisplayedPanelsImplantSpec();
}

void ImplantPanel::slotShowImplantListDialog()
{
	CW3ImageHeader* header =
		ResourceContainer::GetInstance()->GetMainVolume().getHeader();
	ImplantListDlg dlg(header->getPatientID(), this);
	connect(&dlg, SIGNAL(sigSelectImplant(int)), this,
		SLOT(slotImplantSelected(int)));
	dlg.exec();
	disconnect(&dlg, SIGNAL(sigSelectImplant(int)), this,
		SLOT(slotImplantSelected(int)));
}

void ImplantPanel::slotShowPreferenceDialog()
{
	ImplantPreferenceDlg implant_pref_dlg(m_pgVREngine, m_pgMPREngine,
		m_pgRcontainer, this);
	implant_pref_dlg.exec();
}

QString ImplantPanel::MakeLengthPlusSubCategoryFormat(const QString& length, const QString& sub_category)
{
	if (sub_category.isEmpty())
	{
		return length;
	}
	else
	{
		return length + kLengthAndSubCategorySplitter + sub_category;
	}
}

QString ImplantPanel::GetLengthFromComboBoxItem(const QString& item)
{
	QStringList list = item.split(kLengthAndSubCategorySplitter);
	if (list.length() < 1)
	{
		return QString();
	}

	return list[0];
}

QString ImplantPanel::GetSubCategoryFromComboBoxItem(const QString& item)
{
	QStringList list = item.split(kLengthAndSubCategorySplitter);
	if (list.length() < 2)
	{
		return QString();
	}

	return list[1];
}

void ImplantPanel::slotFavoriteUpdated()
{
	connectDB();
	BlockComboboxSignals(true);
	updateManufacturer();
	updateProduct();
	UpdateDiameter();
	UpdateLength();
	UpdateImplantInfo();
	BlockComboboxSignals(false);
	disconnectDB();
}
