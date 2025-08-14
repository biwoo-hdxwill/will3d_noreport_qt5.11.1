#include "dicom_loader.h"

#include <ctime>

#include <QApplication>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QElapsedTimer>
#include <QDebug>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>
#include <QFileSystemModel>
#ifndef WILL3D_VIEWER
#include <QSqlQueryModel>
#include <QSqlError>
#include <QSqlRecord>
#endif
#include <QSpinBox>
#include <QSplitter>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTreeView>
#include <QHeaderView>
#include <QRadioButton>
#include <QTreeWidget>
#include <QToolButton>
#include <QTableWidget>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Define.h"

#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Core/W3DicomIO/W3dicomio.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/datatypes.h"
#include "../../Core/W3ProjectIO/project_io.h"
#include "../../Core/W3ProjectIO/project_io_flie.h"
#endif
#include "../UIPrimitive/W3DicomLoaderScrollbar.h"
#include "W3DicomLoaderThumbnailView.h"
#include "group_box.h"
#include "file_system_tree_widget.h"

namespace
{
	const QString kDcmExtension = QString("dcm");
	const QString kConnectionName("DicomLoader");
}

DicomLoader::DicomLoader(QWidget* parent) : QMainWindow(parent)
{
	SetLayout();
#ifndef WILL3D_VIEWER
	InitDatabaseConnection();
#endif
}

DicomLoader::~DicomLoader()
{
#ifndef WILL3D_VIEWER
	SAFE_DELETE_OBJECT(sql_query_model_);

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);

	if (db.isOpen())
		db.close();

	db = QSqlDatabase();
	db.removeDatabase(kConnectionName);
#endif

	SAFE_DELETE_OBJECT(displays_[0]);
	SAFE_DELETE_OBJECT(displays_[1]);
	SAFE_DELETE_OBJECT(displays_[2]);

	SAFE_DELETE_OBJECT(image_range_scroll_);

	SAFE_DELETE_OBJECT(image_bufs_[0]);
	SAFE_DELETE_OBJECT(image_bufs_[1]);
	SAFE_DELETE_OBJECT(image_bufs_[2]);

	for (auto &i : org_patients_)
	{
		for (auto &j : i->dicom_infos)
		{
			j->files.clear();
			SAFE_DELETE_OBJECT(j);
		}
		i->dicom_infos.clear();
		SAFE_DELETE_OBJECT(i);
	}
	org_patients_.clear();

	SAFE_DELETE_OBJECT(fsm_patient_image_dir_);
	SAFE_DELETE_OBJECT(import_to_db_action_);
	SAFE_DELETE_OBJECT(delete_from_recent_action_);
	SAFE_DELETE_OBJECT(delete_from_db_action_);
	SAFE_DELETE_OBJECT(study_tree_);
}

void DicomLoader::SetLayout()
{
	QWidget* central_widget = new QWidget(this);
	QHBoxLayout* main_layout = new QHBoxLayout(central_widget);
#if 0
	QSplitter* splitter = new QSplitter();
	splitter->setOrientation(Qt::Horizontal);
	splitter->setHandleWidth(5);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 2);

	splitter->addWidget(CreateExplorerWidget());
	splitter->addWidget(CreateDataWidget());

	main_layout->addWidget(splitter);
#else
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(3);
	main_layout->addWidget(CreateExplorerWidget());
	main_layout->addWidget(CreateDataWidget());
	main_layout->setStretch(0, 2);
	main_layout->setStretch(1, 5);
#endif

	setCentralWidget(central_widget);
}
#ifndef WILL3D_VIEWER
void DicomLoader::InitDatabaseConnection()
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

#if defined(__APPLE__)
	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", kConnectionName);
	db.setHostName("127.0.0.1");
	db.setPort(port);
	db.setDatabaseName("Will3D");
	db.setUserName("root");
#else
	QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", kConnectionName);
#if MARIA_DB
	db.setDatabaseName(QString("DRIVER={%1};SERVER=127.0.0.1;DATABASE=Will3D;Port=%2;UID=root;PWD=2002;WSID=.;").arg(driver_name).arg(port));
#else
	db.setDatabaseName("DRIVER={SQL Server};SERVER=127.0.0.1\\HDXWILL2014;DATABASE=Will3D;Port=1433;UID=sa;PWD=2002;WSID=.;");
#endif
#endif

	sql_query_model_ = new QSqlQueryModel();

	if (!db.open())
	{
		PrintErrorWill3DDB();
	}
	else
	{
		common::Logger::instance()->Print(common::LogType::INF, "Open database");

		UpdateImplantDatabase(db);

		db.close();
	}
}

void DicomLoader::UpdateImplantDatabase(QSqlDatabase& db)
{
	///////////////////////////////////////////////
	// v1.0.2 update implant list
	QFile update_file("./will3d_update_implant.sql");
	if (update_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QTextStream in(&update_file);
		in.setCodec("UTF-8");

		QString read_all = in.readAll();
		QStringList query_list = read_all.split("\n", QString::SkipEmptyParts);

		QString query;
		for (int i = 0; i < query_list.size(); i++)
		{
			QString query_line = query_list.at(i);
			if (query_line.startsWith("--") ||
				query_line.startsWith("/*"))
			{
				continue;
			} if (query_line.contains(";", Qt::CaseInsensitive))
			{
				query += query_line;
				sql_query_model_->setQuery(query, db);
				common::Logger::instance()->Print(common::LogType::INF, query.toStdString());
				common::Logger::instance()->Print(common::LogType::INF, query_line.toStdString());
				common::Logger::instance()->Print(common::LogType::INF, "===== END");
				query.clear();
			}
			else
			{
				query += query_line;
			}
		}

		if (!CheckSqlError(sql_query_model_))
		{
			db.close();
		}
		else
		{
			common::Logger::instance()->Print(common::LogType::INF, "updated implant database");
			update_file.close();
			update_file.remove();
		}
	}
	else
	{
		common::Logger::instance()->Print(common::LogType::ERR, "failed to open implant database update file");
	}
	///////////////////////////////////////////////
}
#endif
QWidget* DicomLoader::CreateGroupBoxWidget(GroupBox* group_box)
{
	QWidget* widget = new QWidget(this);
	QVBoxLayout* root_layout = new QVBoxLayout(widget);

	widget->setContentsMargins(0, 0, 0, 0);
	root_layout->setContentsMargins(0, 0, 0, 0);

	root_layout->addWidget(group_box);

	return widget;
}

GroupBox* DicomLoader::CreateEmptyGroupBox(const QString& caption)
{
	GroupBox* group_box = new GroupBox();
	group_box->SetCaptionName(caption);
	group_box->SetContentsMargins(10, 10, 10, 10);
	group_box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	return group_box;
}

QWidget* DicomLoader::CreateGroupBox(const QString& caption, QVBoxLayout* contents_layout)
{
	GroupBox* group_box = CreateEmptyGroupBox(caption);
	group_box->AddLayout(contents_layout);

	return CreateGroupBoxWidget(group_box);
}

QWidget* DicomLoader::CreateGroupBox(const QString& caption, QHBoxLayout* contents_layout)
{
	GroupBox* group_box = CreateEmptyGroupBox(caption);
	group_box->AddLayout(contents_layout);

	return CreateGroupBoxWidget(group_box);
}

QWidget* DicomLoader::CreateGroupBox(const QString& caption, QGridLayout* contents_layout)
{
	GroupBox* group_box = CreateEmptyGroupBox(caption);
	group_box->AddLayout(contents_layout);

	return CreateGroupBoxWidget(group_box);
}

void DicomLoader::InitTreeView(QTreeView* tree_view, QFileSystemModel* fsm, const QString& root_path)
{
	if (!tree_view)
		tree_view = new QTreeView();

	if (!fsm)
		fsm = new QFileSystemModel(this);

	tree_view->setStyleSheet(CW3Theme::getInstance()->FileTabExplorerTreeViewStyleSheet());
	tree_view->setContentsMargins(5, 5, 5, 5);
	tree_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	fsm->setFilter(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden | QDir::System);
	QModelIndex index = fsm->setRootPath(root_path);
	tree_view->setModel(fsm);
	tree_view->sortByColumn(0, Qt::SortOrder::AscendingOrder);
	tree_view->setExpandsOnDoubleClick(false);
	tree_view->setColumnHidden(1, true);
	tree_view->setColumnHidden(2, true);
	tree_view->setHeaderHidden(true);
	QHeaderView* header = tree_view->header();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(true);
	tree_view->setHeader(header);

	if (index.isValid())
	{
		tree_view->setRootIndex(index);
		tree_view->setCurrentIndex(index);
		tree_view->expand(index);
		tree_view->scrollTo(index);
	}
}

void DicomLoader::InitStudyTreeWidget()
{
	if (!study_tree_)
		study_tree_ = new QTreeWidget();

	connect(study_tree_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(slotSeriesTreeClicked(QTreeWidgetItem*, int)));
	connect(study_tree_, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this, SLOT(slotSeriesTreeClicked(QTreeWidgetItem*, int)));
	connect(study_tree_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(slotSeriesTreeDoubleClicked(QTreeWidgetItem*, int)));

	study_tree_->setStyleSheet(CW3Theme::getInstance()->FileTabStudyTreeWidgetStyleSheet());

	QStringList labels{
		lang::LanguagePack::txt_id(),
		lang::LanguagePack::txt_patient_name(),
		lang::LanguagePack::txt_gender(),
		lang::LanguagePack::txt_birth_date(),
		lang::LanguagePack::txt_scan_date(),
		lang::LanguagePack::txt_modality(),
		lang::LanguagePack::txt_images(),
		lang::LanguagePack::txt_description(),
		"No.",
		lang::LanguagePack::txt_path()
	};
	study_tree_->setFocusPolicy(Qt::NoFocus);
	study_tree_->setEditTriggers(QTreeWidget::NoEditTriggers);
	study_tree_->setDragDropOverwriteMode(false);
	study_tree_->setSelectionMode(QTreeWidget::SingleSelection);
	study_tree_->setSortingEnabled(true);
	study_tree_->setItemDelegate(new GridDelegate(study_tree_));
	study_tree_->setColumnCount(10);
	study_tree_->setHeaderLabels(labels);
	study_tree_->sortByColumn(0, Qt::AscendingOrder);
	study_tree_->hideColumn(8);
	QHeaderView* header = study_tree_->header();
	header->setStyleSheet(CW3Theme::getInstance()->FileTabStudyTreeHeaderStyleSheet());
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setCascadingSectionResizes(true);
	header->setSortIndicatorShown(true);
	header->setStretchLastSection(true);
	study_tree_->setHeader(header);
	study_tree_->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(study_tree_, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotStudyTreeMenu(QPoint)));

	study_tree_menu_ = new QMenu(study_tree_);
	import_to_db_action_ = study_tree_menu_->addAction("Import to database");
	delete_from_recent_action_ = study_tree_menu_->addAction("Delete from recent");
	delete_from_recent_action_->setVisible(false);
	delete_from_db_action_ = study_tree_menu_->addAction("Delete from database");
	delete_from_db_action_->setVisible(false);

	connect(import_to_db_action_, SIGNAL(triggered()), this, SLOT(slotImportToDB()));
	connect(delete_from_recent_action_, SIGNAL(triggered()), this, SLOT(slotDeleteFromRecent()));
	connect(delete_from_db_action_, SIGNAL(triggered()), this, SLOT(slotDeleteFromDB()));
}

void DicomLoader::InitDicomSummaryTableWidget()
{
	if (!dicom_summary_table_)
		dicom_summary_table_ = new QTableWidget();

	dicom_summary_table_->setStyleSheet(CW3Theme::getInstance()->FileTabDicomSummaryTableWidgetStyleSheet());

	dicom_summary_table_->setColumnCount(2);
	dicom_summary_table_->setFocusPolicy(Qt::NoFocus);
	dicom_summary_table_->setEditTriggers(QTableWidget::NoEditTriggers);
	dicom_summary_table_->setDragDropOverwriteMode(false);
	dicom_summary_table_->setSelectionMode(QTableWidget::NoSelection);
	dicom_summary_table_->setHorizontalScrollMode(QTableWidget::ScrollPerPixel);
	dicom_summary_table_->setShowGrid(true);
	dicom_summary_table_->setWordWrap(false);
	dicom_summary_table_->setCornerButtonEnabled(false);
	dicom_summary_table_->setAlternatingRowColors(true);
	QHeaderView* horizontal_header = dicom_summary_table_->horizontalHeader();
	QHeaderView* vertical_header = dicom_summary_table_->verticalHeader();
	horizontal_header->setVisible(false);
	vertical_header->setVisible(false);
	horizontal_header->setHighlightSections(false);
	vertical_header->setHighlightSections(false);
	horizontal_header->setStretchLastSection(true);
}

QWidget* DicomLoader::CreateExplorerWidget()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	path_input_ = new QLineEdit();
	QSplitter* splitter = new QSplitter();
	explorer_tree_ = new FileSystemTreeWidget("", splitter);
	patient_image_dir_tree_ = new QTreeView(splitter);
	fsm_patient_image_dir_ = new QFileSystemModel(this);

	InitTreeView(patient_image_dir_tree_, fsm_patient_image_dir_, "./");

	connect(path_input_, SIGNAL(returnPressed()), this, SLOT(slotPathEditFinished()));
	connect(explorer_tree_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(slotExplorerTreeClicked(QTreeWidgetItem*, int)));
	connect(patient_image_dir_tree_, SIGNAL(clicked(QModelIndex)), this, SLOT(slotPatientFolderViewClicked(QModelIndex)));
	connect(patient_image_dir_tree_, SIGNAL(activated(QModelIndex)), this, SLOT(slotPatientFolderViewClicked(QModelIndex)));

	ApplyPreferences();

	patient_image_dir_tree_->setVisible(false);

	path_input_->setStyleSheet(CW3Theme::getInstance()->FileTabLineEditStyleSheet());

	splitter->setOrientation(Qt::Vertical);
	splitter->setHandleWidth(5);
	splitter->setStretchFactor(0, 3);
	splitter->setStretchFactor(1, 1);

	contents_layout->setSpacing(5);

	contents_layout->addWidget(path_input_);
	contents_layout->addWidget(splitter);

	return CreateGroupBox(lang::LanguagePack::txt_open_path(), contents_layout);
}

QWidget* DicomLoader::CreateDataWidget()
{
	QWidget* widget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(widget);

	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(3);
	layout->addWidget(CreateDataListWidget());
	layout->addWidget(CreateDicomInformationWidget());

	return widget;
}

QWidget* DicomLoader::CreateDataListWidget()
{
	QVBoxLayout* contents_layout = new QVBoxLayout();
	QHBoxLayout* search_layout = new QHBoxLayout();
	QHBoxLayout* patient_id_layout = new QHBoxLayout();
	QHBoxLayout* patient_name_layout = new QHBoxLayout();
	QHBoxLayout* radio_layout = new QHBoxLayout();
	QLabel* patient_id_label = new QLabel();
	QLabel* patient_name_label = new QLabel();
	patient_id_input_ = new QLineEdit();
	patient_name_input_ = new QLineEdit();
	QToolButton* search_button = new QToolButton();
	file_system_radio_ = new QRadioButton();
	database_radio_ = new QRadioButton();
	recent_radio_ = new QRadioButton();
	study_tree_ = new QTreeWidget();
	InitStudyTreeWidget();

	file_system_radio_->setChecked(true);

	connect(search_button, SIGNAL(clicked()), this, SLOT(slotSearch()));
	connect(patient_id_input_, SIGNAL(returnPressed()), this, SLOT(slotSearchID()));
	connect(patient_name_input_, SIGNAL(returnPressed()), this, SLOT(slotSearchName()));

	connect(file_system_radio_, SIGNAL(toggled(bool)), this, SLOT(slotRadioFileSystemClicked(bool)));
	connect(database_radio_, SIGNAL(toggled(bool)), this, SLOT(slotRadioDatabaseClicked(bool)));
	connect(recent_radio_, SIGNAL(toggled(bool)), this, SLOT(slotRadioRecentClicked(bool)));

	patient_id_label->setText(lang::LanguagePack::txt_patient_id());
	patient_name_label->setText(lang::LanguagePack::txt_patient_name() + " :");

	search_button->setText(lang::LanguagePack::txt_search());

	file_system_radio_->setText(lang::LanguagePack::txt_file_system());
	database_radio_->setText(lang::LanguagePack::txt_database());
	recent_radio_->setText(lang::LanguagePack::txt_recent());

	patient_id_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	patient_name_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());

	patient_id_input_->setStyleSheet(CW3Theme::getInstance()->FileTabLineEditStyleSheet());
	patient_name_input_->setStyleSheet(CW3Theme::getInstance()->FileTabLineEditStyleSheet());

	search_button->setStyleSheet(CW3Theme::getInstance()->FileTabToolButtonStyleSheet());

	file_system_radio_->setStyleSheet(CW3Theme::getInstance()->FileTabRadioButtonStyleSheet());
	database_radio_->setStyleSheet(CW3Theme::getInstance()->FileTabRadioButtonStyleSheet());
	recent_radio_->setStyleSheet(CW3Theme::getInstance()->FileTabRadioButtonStyleSheet());

	/*patient_id_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	patient_name_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);*/

	search_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	file_system_radio_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	database_radio_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	recent_radio_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	patient_id_layout->setSpacing(4);
	patient_id_layout->addWidget(patient_id_label);
	patient_id_layout->addWidget(patient_id_input_);

	patient_name_layout->setSpacing(4);
	patient_name_layout->addWidget(patient_name_label);
	patient_name_layout->addWidget(patient_name_input_);

	search_layout->setSpacing(20);
	search_layout->addLayout(patient_id_layout);
	search_layout->addLayout(patient_name_layout);
	search_layout->addWidget(search_button);

	radio_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	radio_layout->setSpacing(60);
	radio_layout->addWidget(file_system_radio_);
	radio_layout->addWidget(database_radio_);
	radio_layout->addWidget(recent_radio_);

	contents_layout->setSpacing(10);

	contents_layout->addLayout(search_layout);
	contents_layout->addLayout(radio_layout);
	contents_layout->addWidget(study_tree_);

	return CreateGroupBox(lang::LanguagePack::txt_search_patient(), contents_layout);
}

QWidget* DicomLoader::CreateDicomInformationWidget()
{
	QGridLayout* contents_layout = new QGridLayout();
	QHBoxLayout* roi_title_layout = new QHBoxLayout();
	QHBoxLayout* roi_thumbnail_layout = new QHBoxLayout();
	QHBoxLayout* roi_controller_layout = new QHBoxLayout();
	QHBoxLayout* roi_input_value_layout = new QHBoxLayout();
	QHBoxLayout* roi_input_value_left_layout = new QHBoxLayout();
	QHBoxLayout* roi_input_value_right_layout = new QHBoxLayout();
	QHBoxLayout* roi_x_min_label_layout = new QHBoxLayout();
	QHBoxLayout* roi_x_max_label_layout = new QHBoxLayout();
	QHBoxLayout* roi_width_label_layout = new QHBoxLayout();
	QVBoxLayout* roi_x_width_label_layout = new QVBoxLayout();
	QVBoxLayout* roi_x_width_value_layout = new QVBoxLayout();
	QVBoxLayout* roi_y_height_label_layout = new QVBoxLayout();
	QVBoxLayout* roi_y_height_value_layout = new QVBoxLayout();
	QVBoxLayout* open_dicom_button_layout = new QVBoxLayout();

	QLabel* roi_title_label = new QLabel();
	QLabel* dicom_summary_title_label = new QLabel();
	QLabel* roi_x_min_label = new QLabel();
	QLabel* roi_x_max_label = new QLabel();
	QLabel* roi_y_min_label = new QLabel();
	QLabel* roi_y_max_label = new QLabel();
	QLabel* roi_width_label = new QLabel();
	QLabel* roi_height_label = new QLabel();

	dicom_summary_table_ = new QTableWidget();

	QToolButton* reset_roi_button = new QToolButton();
	QToolButton* open_dicom_button = new QToolButton();

	x_min_dspin_ = new QSpinBox();
	x_max_dspin_ = new QSpinBox();
	y_min_dspin_ = new QSpinBox();
	y_max_dspin_ = new QSpinBox();

	roi_width_ = new QLineEdit();
	roi_height_ = new QLineEdit();

	image_range_scroll_ = new CW3DicomLoaderScrollbar();
	for (int i = 0; i < 3; i++)
	{
		image_bufs_[i] = nullptr;
		displays_[i] = new CW3DicomLoaderThumbnailView(i);
		QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		size_policy.setHeightForWidth(true);
		displays_[i]->setSizePolicy(size_policy);
	}

	InitDicomSummaryTableWidget();

	for (int i = 0; i < 3; i++)
	{
		connect(displays_[i], SIGNAL(sigWheelTranslate(int, int)), this, SLOT(slotWheelTranslate(int, int)));
		connect(displays_[i], SIGNAL(sigSyncThumbnailRect(QRectF, int)), this, SLOT(slotChangeSelectedArea(QRectF, int)));
		connect(x_min_dspin_, SIGNAL(valueChanged(int)), displays_[i], SLOT(slotSetGuideRectLeft(int)));
		connect(x_max_dspin_, SIGNAL(valueChanged(int)), displays_[i], SLOT(slotSetGuideRectRight(int)));
		connect(y_min_dspin_, SIGNAL(valueChanged(int)), displays_[i], SLOT(slotSetGuideRectTop(int)));
		connect(y_max_dspin_, SIGNAL(valueChanged(int)), displays_[i], SLOT(slotSetGuideRectBottom(int)));
		for (int j = 0; j < 3; j++)
			if (i != j)
				connect(displays_[i], SIGNAL(sigSyncThumbnailRect(QRectF, int)), displays_[j], SLOT(slotSyncThumbnailRect(QRectF, int)));
	}

	connect(image_range_scroll_, SIGNAL(sigScrollTranslated(int, float)), this, SLOT(slotScrollChanged(int, float)));

	connect(reset_roi_button, SIGNAL(clicked()), this, SLOT(slotResetROI()));
	connect(open_dicom_button, SIGNAL(clicked()), this, SLOT(slotOpenDicom()));

	roi_width_->setEnabled(false);
	roi_height_->setEnabled(false);

	x_min_dspin_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	x_max_dspin_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	y_min_dspin_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	y_max_dspin_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	roi_width_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	roi_height_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	roi_width_->setText("0");
	roi_height_->setText("0");

	roi_title_label->setText(lang::LanguagePack::txt_selected_volume_of_interest());
	dicom_summary_title_label->setText(lang::LanguagePack::txt_dicom_summary());

	reset_roi_button->setText(lang::LanguagePack::txt_reset());
	open_dicom_button->setText(lang::LanguagePack::txt_open_dicom_file());

	roi_x_min_label->setText("x (min.) :");
	roi_x_max_label->setText("x (max.) :");
	roi_y_min_label->setText("y (min.) :");
	roi_y_max_label->setText("y (max.) :");
	roi_width_label->setText(lang::LanguagePack::txt_width() + " :");
	roi_height_label->setText(lang::LanguagePack::txt_height() + " :");

	roi_title_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	dicom_summary_title_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());

	reset_roi_button->setStyleSheet(CW3Theme::getInstance()->FileTabToolButtonStyleSheet());
	open_dicom_button->setStyleSheet(CW3Theme::getInstance()->FileTabToolButtonStyleSheet());

	roi_x_min_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	roi_x_max_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	roi_y_min_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	roi_y_max_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	roi_width_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());
	roi_height_label->setStyleSheet(CW3Theme::getInstance()->FileTabLabelStyleSheet());

	x_min_dspin_->setStyle(new ViewSpinBoxStyle());
	x_min_dspin_->setStyleSheet(CW3Theme::getInstance()->FileTabSpinBoxStyleSheet());
	x_max_dspin_->setStyle(new ViewSpinBoxStyle());
	x_max_dspin_->setStyleSheet(CW3Theme::getInstance()->FileTabSpinBoxStyleSheet());
	y_min_dspin_->setStyle(new ViewSpinBoxStyle());
	y_min_dspin_->setStyleSheet(CW3Theme::getInstance()->FileTabSpinBoxStyleSheet());
	y_max_dspin_->setStyle(new ViewSpinBoxStyle());
	y_max_dspin_->setStyleSheet(CW3Theme::getInstance()->FileTabSpinBoxStyleSheet());

	roi_width_->setStyleSheet(CW3Theme::getInstance()->FileTabLineEditStyleSheet());
	roi_height_->setStyleSheet(CW3Theme::getInstance()->FileTabLineEditStyleSheet());

	roi_x_min_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	roi_x_max_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	roi_y_min_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	roi_y_max_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	roi_width_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	roi_height_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	x_min_dspin_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	x_max_dspin_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	y_min_dspin_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	y_max_dspin_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	image_range_scroll_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	reset_roi_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	open_dicom_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	roi_x_min_label->setAlignment(Qt::AlignRight);
	roi_x_max_label->setAlignment(Qt::AlignRight);
	roi_y_min_label->setAlignment(Qt::AlignRight);
	roi_y_max_label->setAlignment(Qt::AlignRight);
	roi_width_label->setAlignment(Qt::AlignRight);
	roi_height_label->setAlignment(Qt::AlignRight);

	roi_x_width_label_layout->setSpacing(5);
	roi_x_width_label_layout->addWidget(roi_x_min_label);
	roi_x_width_label_layout->addWidget(roi_x_max_label);
	roi_x_width_label_layout->addWidget(roi_width_label);

	roi_x_width_value_layout->setSpacing(5);
	roi_x_width_value_layout->addWidget(x_min_dspin_, Qt::AlignLeft);
	roi_x_width_value_layout->addWidget(x_max_dspin_, Qt::AlignLeft);
	roi_x_width_value_layout->addWidget(roi_width_, Qt::AlignLeft);

	roi_y_height_label_layout->setSpacing(5);
	roi_y_height_label_layout->addWidget(roi_y_min_label);
	roi_y_height_label_layout->addWidget(roi_y_max_label);
	roi_y_height_label_layout->addWidget(roi_height_label);

	roi_y_height_value_layout->setSpacing(5);
	roi_y_height_value_layout->addWidget(y_min_dspin_, Qt::AlignLeft);
	roi_y_height_value_layout->addWidget(y_max_dspin_, Qt::AlignLeft);
	roi_y_height_value_layout->addWidget(roi_height_, Qt::AlignLeft);

	roi_input_value_left_layout->setSpacing(4);
	roi_input_value_left_layout->addLayout(roi_x_width_label_layout);
	roi_input_value_left_layout->addLayout(roi_x_width_value_layout);

	roi_input_value_right_layout->setSpacing(4);
	roi_input_value_right_layout->addLayout(roi_y_height_label_layout);
	roi_input_value_right_layout->addLayout(roi_y_height_value_layout);

	roi_input_value_layout->setSpacing(20);
	roi_input_value_layout->addLayout(roi_input_value_left_layout);
	roi_input_value_layout->addLayout(roi_input_value_right_layout);

	roi_title_layout->setContentsMargins(0, 0, 0, 0);
	roi_title_layout->addWidget(roi_title_label, 0, Qt::AlignLeft | Qt::AlignVCenter);
	roi_title_layout->addWidget(reset_roi_button, 0, Qt::AlignRight | Qt::AlignVCenter);

	roi_controller_layout->setContentsMargins(0, 0, 0, 0);
	roi_controller_layout->setSpacing(20);
	roi_controller_layout->addLayout(roi_input_value_layout);
	roi_controller_layout->addWidget(image_range_scroll_, Qt::AlignTop);
	roi_controller_layout->setStretch(0, 1);
	roi_controller_layout->setStretch(1, 2);

	roi_thumbnail_layout->setContentsMargins(0, 0, 0, 0);
	roi_thumbnail_layout->setSpacing(10);
	for (int i = 0; i < 3; i++)
		roi_thumbnail_layout->addWidget(displays_[i]);

	open_dicom_button_layout->addWidget(open_dicom_button);

	contents_layout->setVerticalSpacing(5);
	contents_layout->setHorizontalSpacing(20);
	contents_layout->setColumnStretch(0, 3);
	contents_layout->setColumnStretch(1, 1);
	contents_layout->setRowStretch(0, 1);
	contents_layout->setRowStretch(1, 10);
	contents_layout->setRowStretch(2, 1);

	contents_layout->addLayout(roi_title_layout, 0, 0);
	contents_layout->addWidget(dicom_summary_title_label, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);
	contents_layout->addLayout(roi_thumbnail_layout, 1, 0);
	contents_layout->addWidget(dicom_summary_table_, 1, 1);
	contents_layout->addLayout(roi_controller_layout, 2, 0);
	contents_layout->addLayout(open_dicom_button_layout, 2, 1, Qt::AlignBottom);

	return CreateGroupBox(lang::LanguagePack::txt_selected_file_information(), contents_layout);
}

// slots
#ifndef WILL3D_VIEWER
void DicomLoader::slotOpenDicom()
{
#if 0
	open_from_external_program_ = false;
	project_path_ = QString();
	project_status_.load_project = false;
	open_only_trd_ = false;
#endif

	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start slotOpenDicom");

	QTreeWidgetItem* item = study_tree_->currentItem();
	OpenDicom(item);

	common::Logger::instance()->Print(common::LogType::INF, "end slotOpenDicom : " + QString::number(timer.elapsed()).toStdString() + " ms");
}
#endif
#ifndef WILL3D_VIEWER
void DicomLoader::slotPathEditFinished()
{
	explorer_tree_->SelectPath(path_input_->text());
}

void DicomLoader::slotExplorerTreeClicked(QTreeWidgetItem* item, int column)
{
	const QString& selected_path = item->text(2);
	path_input_->setText(selected_path);
	ReadFolder(selected_path);
	item->setExpanded(true);
}

void DicomLoader::slotPatientFolderViewClicked(QModelIndex index)
{
	QString selected_path = fsm_patient_image_dir_->fileInfo(index).absoluteFilePath();
	ReadFolder(selected_path);

	patient_image_dir_tree_->expand(index);
	patient_image_dir_tree_->scrollTo(index);
	patient_image_dir_tree_->setCurrentIndex(index);
}

void DicomLoader::slotSearch()
{
	search_id_ = patient_id_input_->text();
	SearchStudy();

	search_name_ = patient_name_input_->text();
	SearchStudy();
}

void DicomLoader::slotSearchID()
{
	search_id_ = patient_id_input_->text();
	SearchStudy();
}

void DicomLoader::slotSearchName()
{
	search_name_ = patient_name_input_->text();
	SearchStudy();
}

void DicomLoader::slotWheelTranslate(int pos, int wheel)
{
	int nSign = wheel > 0 ? 1 : -1;
	switch (pos)
	{
	case 0:	image_range_scroll_->setStart(image_range_scroll_->getStart() + nSign);
		break;
	case 1:	image_range_scroll_->setMiddle(image_range_scroll_->getMiddle() + nSign);
		break;
	case 2: image_range_scroll_->setEnd(image_range_scroll_->getEnd() + nSign);
		break;
	}
}

void DicomLoader::slotScrollChanged(int index, float pos)
{
	QList<QTreeWidgetItem*> list = study_tree_->selectedItems();
	if (list.count() != 1)
		return;

	QTreeWidgetItem* item = list.at(0);

	if (item->childCount() == 0)
	{
		int patient_index = GetPatientIndex(item);
		PatientInfo* patient = search_results_.at(patient_index);
		if (!patient)
			return;

		int series_index = GetStudyIndex(item);
		DicomInfo* dicom_info = patient->dicom_infos.at(series_index);
		if (!dicom_info)
			return;

		CW3DicomIO dicomIO;
		QByteArray ba = dicom_info->files.at((int)pos).toLocal8Bit();
		std::string file_path = ba.constData();
		dicomIO.getImageBufFromFile(file_path, image_bufs_[index]);
		//displays_[index]->setImage(image_bufs_[index], dicom_info->width, dicom_info->height, dicom_info->win_width, dicom_info->win_center);

		// auto ww,wl
		short win_width = static_cast<short>(dicom_info->win_width);
		short win_center = static_cast<short>(dicom_info->win_center);

		if (dicom_info->win_width <= 1 ||
			dicom_info->win_center <= 1)
		{
			short min = 32767;
			short max = -32767;
			for (int i = 0; i < dicom_info->width * dicom_info->height; i++)
			{
				short val = image_bufs_[index][i];
				if (val < min)	min = val;
				if (max < val)	max = val;
			}

			if (dicom_info->win_width <= 1)
				win_width = max - min;
			if (dicom_info->win_center <= 1)
				win_center = min + win_width / 2;
		}
		//

		displays_[index]->setImage(image_bufs_[index], dicom_info->width, dicom_info->height, win_width, win_center);

		return;
	}
}

void DicomLoader::slotRadioFileSystemClicked(bool checked)
{
	if (!checked)
		return;

	import_to_db_action_->setVisible(true);
	delete_from_recent_action_->setVisible(false);
	delete_from_db_action_->setVisible(false);

	use_file_system_ = true;
	study_tree_->hideColumn(study_tree_->columnCount() - 1);

	ResetStudyTree();
	ResetDicomInfoTable();
	ResetThumbnail();

	if (path_input_->text().size() <= 0)
		return;

	if (load_second_volume_)
	{
		second_volume_path_ = "";
#if 0
		slotPatientFolderViewClicked(patient_image_dir_tree_->currentIndex());
#else
		slotExplorerTreeClicked(explorer_tree_->currentItem(), 0);
#endif
	}
	else
	{
		selected_volume_path_ = "";
		slotExplorerTreeClicked(explorer_tree_->currentItem(), 0);
	}
}

void DicomLoader::slotRadioDatabaseClicked(bool checked)
{
	if (!checked)
		return;

	import_to_db_action_->setVisible(false);
	delete_from_recent_action_->setVisible(false);
	delete_from_db_action_->setVisible(true);

	use_file_system_ = false;
	study_tree_->showColumn(study_tree_->columnCount() - 1);

	ResetStudyTree();
	ResetDicomInfoTable();
	ResetThumbnail();

	if (!load_second_volume_)
		selected_volume_path_ = "";
	else
		second_volume_path_ = "";

	GetStudyListFromDB(false);
}

void DicomLoader::slotRadioRecentClicked(bool checked)
{
	if (!checked)
		return;

	import_to_db_action_->setVisible(true);
	delete_from_recent_action_->setVisible(true);
	delete_from_db_action_->setVisible(false);

	use_file_system_ = false;
	study_tree_->showColumn(study_tree_->columnCount() - 1);

	ResetStudyTree();
	ResetDicomInfoTable();
	ResetThumbnail();

	if (!load_second_volume_)
		selected_volume_path_ = "";
	else
		second_volume_path_ = "";

	GetStudyListFromDB(true);
}

void DicomLoader::slotResetROI()
{
	for (int i = 0; i < 3; i++)
		displays_[i]->setEmpty();

	image_range_scroll_->setRange(0, 0);

	InitROIControllerAndDicomSummary();
}

void DicomLoader::slotStudyTreeMenu(const QPoint& pos)
{
	current_study_tree_item_ = study_tree_->itemAt(pos);
	if (!current_study_tree_item_)
		return;

	if (current_study_tree_item_->childCount() > 0)
		return;

	study_tree_menu_->exec(study_tree_->viewport()->mapToGlobal(pos));
}

void DicomLoader::slotImportToDB()
{
	int patient_index = GetPatientIndex(current_study_tree_item_);
	int study_index = GetStudyIndex(current_study_tree_item_);

	// 임시코드 : willmaster에서 영상을 열었을 경우 임시 폴더에 파일이 생성되었다가
	// willmaster 종료시 삭제되므로 will3d db에 저장할 수 없게 해야함,
	// 추후에 마우스 오른쪽 버튼 메뉴에서 완전히 삭제 예정.
	if (!open_from_external_program_)
		InsertDB(patient_index, study_index, false);
}

void DicomLoader::slotDeleteFromRecent()
{
	int patient_index = GetPatientIndex(current_study_tree_item_);
	int study_index = GetStudyIndex(current_study_tree_item_);

	DeleteFromRecent(patient_index, study_index);

	ResetStudyTree();
	ResetDicomInfoTable();
	ResetThumbnail();

	GetStudyListFromDB(true);

	QTreeWidgetItem* pItem = study_tree_->topLevelItem(patient_index);
	if (pItem)
		pItem->setExpanded(true);
}

void DicomLoader::slotDeleteFromDB()
{
	int patient_index = GetPatientIndex(current_study_tree_item_);
	int study_index = GetStudyIndex(current_study_tree_item_);

	DeleteFromDB(patient_index, study_index);

	if (current_study_tree_item_->text(5).compare("PRJ") == 0)
	{
		int patientIndex = GetPatientIndex(current_study_tree_item_);
		if (search_results_.size() <= patientIndex)
			return;

		PatientInfo* patient = search_results_.at(patientIndex);
		if (!patient)
			return;

		int study_index = GetStudyIndex(current_study_tree_item_);
		if (patient->dicom_infos.size() <= study_index)
			return;

		DicomInfo* dicom_info = patient->dicom_infos.at(study_index);
		if (!dicom_info)
			return;

		QFile project(dicom_info->project_path);
		project.remove();
	}

	ResetStudyTree();
	ResetDicomInfoTable();
	ResetThumbnail();

	GetStudyListFromDB(false);

	QTreeWidgetItem* item = study_tree_->topLevelItem(patient_index);
	if (item)
		item->setExpanded(true);
}

void DicomLoader::slotSeriesTreeClicked(QTreeWidgetItem* item, int index)
{
	if (current_study_tree_item_ == item)
	{
		return;
	}

	if (current_study_tree_item_)
	{
		QTreeWidgetItem* top_level_item = nullptr;

		top_level_item = (current_study_tree_item_->childCount() == 0) ? current_study_tree_item_->parent() : current_study_tree_item_;
		if (top_level_item)
		{
			top_level_item->setIcon(0, QIcon(QPixmap(":/image/dicomloader/id.png")));
		}
	}

	current_study_tree_item_ = item;

	InitROIControllerAndDicomSummary();
}

void DicomLoader::slotSeriesTreeDoubleClicked(QTreeWidgetItem* item, int column)
{
	OpenDicom(item);
}

void DicomLoader::slotChangeSelectedArea(QRectF area, int index)
{
	x_min_dspin_->setMaximum(displays_[index]->left_max() + 1);
	x_max_dspin_->setMinimum(displays_[index]->right_min());
	y_min_dspin_->setMaximum(displays_[index]->top_max() + 1);
	y_max_dspin_->setMinimum(displays_[index]->bottom_min());

	x_min_dspin_->setValue(area.left() + 1);
	x_max_dspin_->setValue(area.right());
	y_min_dspin_->setValue(area.top() + 1);
	y_max_dspin_->setValue(area.bottom());
}
#endif
// slots

// public
#ifndef WILL3D_VIEWER
void DicomLoader::InsertDBforProject(PatientInfo* patient)
{
	const QString& patient_Id = patient->patient_id;
	const QString& patient_name = patient->patient_name;

	DicomInfo* dicom_info = patient->dicom_infos.at(0);

	const QString& study_uid = dicom_info->study_uid;
	const QString& series_uid = dicom_info->series_uid;
	const QString& study_date = dicom_info->study_date;
	const QString& modality = dicom_info->modality;
	QString num_image = QString::number(dicom_info->num_image);
	const QString& description = dicom_info->description;
	const QString& file_dir = dicom_info->file_dir;
	const QString& project_path = dicom_info->project_path;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (!db.open())
	{
		PrintErrorWill3DDB();
		return;
	}

	QString query = QString("insert into series values('%1', '%2', '%3', N'%4', '%5', '%6', '%7', N'%8', N'%9', N'%10', %11)")
		.arg(study_uid, series_uid, patient_Id, patient_name, study_date, modality, num_image, description, file_dir)
		.arg(project_path, QString::number(0));

	sql_query_model_->setQuery(query, db);

	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	db.close();
}
#endif
std::vector<QString> DicomLoader::GetSelectedFileList()
{
	std::vector<QString> result;

	QList<QTreeWidgetItem*> list = study_tree_->selectedItems();
	if (list.count() != 1)
		return result;

	QTreeWidgetItem* item = list.at(0);

	if (item->childCount() == 0)
	{
		int patient_index = GetPatientIndex(item);
		PatientInfo* patient_info = search_results_.at(patient_index);
		if (!patient_info)
			return result;

		int series_index = GetStudyIndex(item);
		DicomInfo* dicom_info = patient_info->dicom_infos.at(series_index);
		if (!dicom_info)
			return result;

		result = dicom_info->files;
	}
	return result;
}

CW3Point3D DicomLoader::GetSelectedRange()
{
	return CW3Point3D(image_range_scroll_->getStart(), image_range_scroll_->getMiddle(), image_range_scroll_->getEnd());
}

QRect DicomLoader::GetSelectedArea()
{
	return displays_[0]->getSelectedRect();
}
#ifndef WILL3D_VIEWER
void DicomLoader::SetEnableLoadSecondVolume(bool second)
{
	load_second_volume_ = second;

	if (load_second_volume_)
	{
		QString willmaster_dicom_path = GetWillmasterDicomPath();
		if (willmaster_dicom_path.length() > 0 && current_first_volume_patient_id_.length() > 0)
		{
#if 1
			willmaster_dicom_path = willmaster_dicom_path + "/dicom/" + current_first_volume_patient_id_;			
#endif
			QDir dir(willmaster_dicom_path);
			bool exists = dir.exists();

			common::Logger::instance()->Print(common::LogType::DBG, std::string("willmasterDicomPath :") +
				willmaster_dicom_path.toStdString() + std::string("/") + std::string("exists :") + std::to_string(exists));

			if (exists && fsm_patient_image_dir_)
			{
				patient_image_dir_tree_->setRootIndex(fsm_patient_image_dir_->setRootPath(willmaster_dicom_path));
				
				file_system_radio_->setChecked(true);
				patient_image_dir_tree_->setVisible(true);
			}
			else
			{
				patient_image_dir_tree_->setVisible(false);
			}
		}
		else
		{
			patient_image_dir_tree_->setVisible(false);
		}
	}
	else
	{
		patient_image_dir_tree_->setVisible(false);
	}
}
#endif
void DicomLoader::ReadInputFile(const QString& path)
{
	trd_file_path_from_external_program_ = "";
	open_only_trd_ = false;

	QByteArray ba = path.toLocal8Bit();
	std::string read_file_path = ba.constData();

	common::Logger::instance()->Print(common::LogType::INF,
		std::string("readScriptFile : ") + read_file_path);

	QFile script(path);
	if (!script.open(QIODevice::ReadOnly))
		return;

	QFileInfo file_info(path);
	QString suffix = file_info.suffix();

	QStringList file_list;
	int mode_trd = 0;

	if (suffix.compare("txt") == 0)
	{
		QTextStream in(&script);

		bool is_file_list = false;
		bool end = in.atEnd();
		while (!in.atEnd())
		{
			QString line = in.readLine();

			if (line.length() < 1)
				continue;

			if (line.left(1).compare("#") == 0)
			{
				is_file_list = false;
				continue;
			}

			if (is_file_list)
			{
				QFileInfo file_info(line);

				if (file_info.suffix().compare("dcm") == 0)
					file_list.push_back(line);
				else if (file_info.suffix().compare("trd") == 0 && (mode_trd == 1 || mode_trd == 2))
					trd_file_path_from_external_program_ = line;

				continue;
			}

			QStringList line_list = line.split(" ");

			if (line_list.at(0).compare("Filepath", Qt::CaseInsensitive) == 0)
			{
				is_file_list = true;
			}
			else if (line_list.at(0).compare("NUMIMAGES", Qt::CaseInsensitive) == 0)
			{
				if (line_list.size() < 2)
					return;

				if (line_list.at(1).toInt() < 1)
					return;
			}
			else if (line_list.at(0).compare("TRD", Qt::CaseInsensitive) == 0)
			{
				mode_trd = line_list.at(1).toInt();
				if (mode_trd == 2)
					open_only_trd_ = true;
			}
		}

		script.close();

		project_status_.load_project = false;
	}
	else if (suffix.compare("w3d") == 0)
	{
		project_path_ = path;
		project_status_.load_project = true;
	}

	common::Logger::instance()->Print(common::LogType::DBG, std::string("file list size : ") + std::to_string(file_list.size()));

	SetOpenFromExternalProgramMode(true);

	ResetStudyTree();
	ResetDicomInfoTable();

	if (trd_file_path_from_external_program_.length() > 0 && open_only_trd_)
	{
		common::Logger::instance()->Print(common::LogType::INF, trd_file_path_from_external_program_.toStdString());
		common::Logger::instance()->Print(common::LogType::INF, std::string("m_bOpenOnlyTRD : ")
			+ std::to_string(open_only_trd_));

		QFileInfo trd(trd_file_path_from_external_program_);
		if (trd.exists())
		{
			emit sigSetTRDFromExternalProgram(trd_file_path_from_external_program_, open_only_trd_);
		}
		else
		{
			common::Logger::instance()->Print(common::LogType::ERR, std::string("DicomLoader::open:")
				+ trd_file_path_from_external_program_.toStdString()
				+ std::string(" not found."));
		}
	}

#if 0
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
	progress->setRange(0, 100);
	progress->setValue(0);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	QFuture<void> result;
	if (project_status_.load_project)
	{
		result = QtConcurrent::run(this, &DicomLoader::LoadDicomFromPath, file_info.absolutePath());
	}
	else
	{
		result = QtConcurrent::run(this, &DicomLoader::LoadDicomFromFileList, file_list);
	}

	watcher.setFuture(result);
	progress->exec();
	watcher.waitForFinished();
#else

	if (project_status_.load_project)
	{
		LoadDicomFromPath(file_info.absolutePath());
	}
	else
	{
		LoadDicomFromFileList(file_list);
	}
#endif
	search_results_ = org_patients_;
	SetStudyTree();

	QTreeWidgetItem* top_level_item = study_tree_->topLevelItem(0);
	if (top_level_item)
	{
		top_level_item->setExpanded(true);
		top_level_item->setIcon(0, QIcon(QPixmap(":/image/dicomloader/id_on.png")));

		QTreeWidgetItem* child = top_level_item->child(0);
		if (child)
		{
			study_tree_->itemClicked(child, 0);
			study_tree_->setCurrentItem(child);
		}
	}
}

void DicomLoader::ApplyPreferences()
{
	if (explorer_tree_)
	{
		explorer_tree_->AddFavorites(GlobalPreferences::GetInstance()->preferences_.general.files.favorite_open_paths);
	}
}
// public

void DicomLoader::LoadDicomFromPath(const QString& path)
{
	LoadDicomFromPath(path, "", "");
}

void DicomLoader::LoadDicomFromFileList(QStringList& file_list)
{
	LoadDicomFromFileList(file_list, "", "");
}

void DicomLoader::LoadDicomFromPath(const QString& path,
	const QString& study_uid,
	const QString& series_uid)
{
	common::Logger::instance()->Print(common::LogType::INF, std::string("start DicomLoader::LoadDicomFromPath"));

	QElapsedTimer timer;
	timer.start();

	QDir dir(path);
	dir.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden);
	QStringList entry_list = dir.entryList();
	QStringList file_list;
	for (int i = 0; i < entry_list.size(); i++)
	{
		QString file_path = dir.absoluteFilePath(entry_list.at(i));
		if (file_path.right(3).toLower() == QString("dcm"))
		{
			file_list.push_back(file_path);
		}
	}

	if (file_list.size() < 2)
	{
		return;
	}

	common::Logger::instance()->Print(common::LogType::INF, std::string("DicomLoader::LoadDicomFromPath file_list first : ") + file_list.at(0).toStdString());
	common::Logger::instance()->Print(common::LogType::INF, std::string("DicomLoader::LoadDicomFromPath file_list last : ") + file_list.at(file_list.size() - 1).toStdString());

	common::Logger::instance()->Print(common::LogType::INF, std::string("DicomLoader::LoadDicomFromPath : ") + QString::number(timer.elapsed()).toStdString());
	timer.restart();

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	QObject::connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	auto future = QtConcurrent::run(this, &DicomLoader::LoadDicomFromFileList, file_list, study_uid, series_uid);
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	common::Logger::instance()->Print(common::LogType::INF, std::string("end DicomLoader::LoadDicomFromPath : ") + QString::number(timer.elapsed()).toStdString());
}

void DicomLoader::LoadDicomFromFileList(QStringList& file_list,
	const QString& study_uid,
	const QString& series_uid)
{
	common::Logger::instance()->Print(common::LogType::INF, std::string("start DicomLoader::LoadDicomFromFileList"));

	QElapsedTimer timer;
	timer.start();

	using namespace dcm::tags;

	image_range_scroll_->setRange(0, 0);

	if (file_list.size() < 2)
		return;

	QFileInfo file_info(file_list.at(0));

	if (!load_second_volume_)
		selected_volume_path_ = file_info.absolutePath();
	else
		second_volume_path_ = file_info.absolutePath();

#if 0
	for (int index = 0; index < file_list.size(); index++)
	{
		QFileInfo file_info(file_list.at(index));
		if (file_info.suffix().compare("dcm", Qt::CaseInsensitive) != 0)
			file_list.removeAt(index);
}
#endif

	bool head_first = true;

	int dicom_index = 0;
	CW3DicomIO dicom_io;
	HeaderList hd;

	auto func_read_header = [](const HeaderList& hd, const std::string& str) -> std::string
	{
		auto iter = hd.find(str);
		if (iter != hd.end())
		{
			return iter->second.toLocal8Bit().toStdString();
		}
		else
		{
			return std::string();
		}
	};

	data_sets_.clear();
	data_sets_.reserve(file_list.size());
	for (int i = 0; i < file_list.size(); ++i)
	{
		DataSet data_set;
		data_set.file_path = file_list.at(i);
		QByteArray ba = data_set.file_path.toLocal8Bit();
		std::string file_path = ba.constData();

		if (dicom_io.getHeaderInfoFromFile(file_path, hd, true))
		{
			data_set.patient_id = hd[kPatientID];
			data_set.patient_name = hd[kPatientName];
			data_set.patient_gender = hd[kPatientGender];
			data_set.patient_birth = hd[kPatientBirthDate];
			data_set.study_uid = func_read_header(hd, kStudyInstanceUID);
			data_set.series_uid = func_read_header(hd, kSeriesInstanceUID);
			data_set.study_date = func_read_header(hd, kStudyDate);
			data_set.series_date = func_read_header(hd, kSeriesDate);
			data_set.study_time = func_read_header(hd, kStudyTime);
			data_set.series_time = func_read_header(hd, kSeriesTime);
			data_set.modality = func_read_header(hd, kModality);
			data_set.kvp = func_read_header(hd, kKVP);
			data_set.ma = func_read_header(hd, kXRayTubeCurrent);
			data_set.exp_time = func_read_header(hd, kExposureTime);
			data_set.pixel_spacing = func_read_header(hd, kPixelSpacing);
			data_set.slice_thickness = func_read_header(hd, kSliceThickness);
			data_set.bits_allocated = func_read_header(hd, kBitsAllocated);
			data_set.bits_stored = func_read_header(hd, kBitsStored);
			data_set.pixel_representation = atoi(func_read_header(hd, kPixelRepresentation).c_str());
			data_set.slope = (float)atof(func_read_header(hd, kRescaleSlope).c_str());
			data_set.intercept = (float)atof(func_read_header(hd, kRescaleIntercept).c_str());
			data_set.height = atoi(func_read_header(hd, kRows).c_str());
			data_set.width = atoi(func_read_header(hd, kColumns).c_str());
			data_set.win_width = static_cast<int>(atof(func_read_header(hd, kWindowWidth).c_str()));
			data_set.win_width = (data_set.win_width < 1) ? 1 : data_set.win_width;
			data_set.win_center = static_cast<int>(atof(func_read_header(hd, kWindowCenter).c_str()));
			data_set.description = hd[kStudyDescription];
			data_set.image_position_patient = static_cast<float>(atof(func_read_header(hd, kImagePositionPatient).c_str()));
			data_set.instance_number = atoi(func_read_header(hd, kInstanceNumber).c_str());
			data_set.slice_location = static_cast<float>(atof(func_read_header(hd, kSliceLocation).c_str()));
			//image_position_patient0 = (float)atof(func_read_header(hd, kImagePositionPatient).c_str());

			data_sets_.push_back(data_set);
		}

		/*QString second_file_path = file_list.at(1);

		ba = second_file_path.toLocal8Bit();
		file_path = ba.constData();

		if (dicom_io.getHeaderInfoFromFile(file_path, hd, true))
		{
			image_position_patient1 = (float)atof(func_read_header(hd, kImagePositionPatient).c_str());
		}

		data_sets_.push_back(data_set);

		bool is_z_order_inversed = image_position_patient1 > image_position_patient0;
		if (is_z_order_inversed)
		{
			common::Logger::instance()->Print(common::LogType::INF,
				std::string("Z order inversed"));
			std::reverse(data_sets_.begin(), data_sets_.end());
		}*/
	}

	auto CompareDataSet = [](DataSet obj1, DataSet obj2)
	{
		return obj1.instance_number < obj2.instance_number;
	};
	qSort(data_sets_.begin(), data_sets_.end(), CompareDataSet);

	common::Logger::instance()->Print(common::LogType::INF, std::string("DicomLoader::LoadDicomFromFileList : ") + QString::number(timer.elapsed()).toStdString());
	timer.restart();

	for (auto &ds : data_sets_)
	{ // m_listDataSet에 있는 file 하나에 대해서 새로 수진자, study를 만들거나 기존에 있는 수진자의 study file list에 추가
		if (ds.patient_id.isEmpty())
			ds.patient_id = "empty";

		if (ds.study_uid.empty() ||
			ds.series_uid.empty())
			continue;

		if (!use_file_system_ &&
			(QString::fromLocal8Bit(ds.study_uid.c_str()).compare(study_uid, Qt::CaseInsensitive) != 0 ||
				QString::fromLocal8Bit(ds.series_uid.c_str()).compare(series_uid, Qt::CaseInsensitive) != 0))
		{
			continue;
		}

		bool found_patient = false;
		bool found_study = false;

		while (!found_patient && !found_study) // 해당 file의 정보와 같은 수진자와 수진자의 study가 모두 존재할 때까지
		{
			for (auto &i : org_patients_) // 현재 수진자 list를 탐색
			{
				if (i->patient_id.compare(ds.patient_id, Qt::CaseInsensitive) == 0) // 읽은 file의 수진자 정보와 수진자 list의 수진자 정보가 같은 경우
				{
					found_patient = true; // 수진자 list에 현재 file의 수진자 정보와 같은 수진자 있음

					for (auto &j : i->dicom_infos) // 수진자가 가진 study list를 탐색
					{
						if (!use_file_system_)
						{
							if ((j->study_uid.compare(QString::fromLocal8Bit(ds.study_uid.c_str()), Qt::CaseInsensitive) != 0) ||
								(j->series_uid.compare(QString::fromLocal8Bit(ds.series_uid.c_str()), Qt::CaseInsensitive) != 0))
							{
								continue;
							}

							if (j->files.size() == 0)
								SetDicomInfo(j, ds);
						}
						else
						{
							if ((j->study_uid.compare(QString::fromLocal8Bit(ds.study_uid.c_str()), Qt::CaseInsensitive) != 0) ||
								(j->series_uid.compare(QString::fromLocal8Bit(ds.series_uid.c_str()), Qt::CaseInsensitive) != 0) ||
								(j->width != ds.width) ||
								(j->height != ds.height))
							{
								continue;
							}
						}

						found_study = true;

						if (dicom_io.CheckFile(ds.file_path))
						{
#if 1
							if (j->files.size() == 0)
							{
								j->image_position_patient0 = ds.image_position_patient;
							}
							else if (j->files.size() == 1)
							{
								j->image_position_patient1 = ds.image_position_patient;
								head_first = j->image_position_patient1 < j->image_position_patient0;
								j->is_head_first = head_first;
							}
							j->files.push_back(ds.file_path);
#else
							j->files.push_back(ds.file_path);
#endif
						}
						else
						{
							QByteArray ba = ds.file_path.toLocal8Bit();
							std::string file_path_std_string = ba.constData();
							common::Logger::instance()->Print(common::LogType::ERR, "Invalid dicom file : " + file_path_std_string);
						}

						j->num_image = j->files.size();
					}

					if (found_study)
					{
						continue;
					}

					// 읽은 file의 정보와 같은 수진자가 있으나 해당 study, dicom_info 는 없는 경우
					// 해당 study, dicom_info 정보를 갖는 객체를 만들고 현재 file path를 넣어 수진자에 전달
					DicomInfo* dicomInfo = new DicomInfo;
					SetDicomInfo(dicomInfo, ds);

					if (!load_second_volume_)
						dicomInfo->file_dir = selected_volume_path_;
					else
						dicomInfo->file_dir = second_volume_path_;

					if (dicom_io.CheckFile(ds.file_path))
					{
						if (dicomInfo->files.size() == 0)
						{
							dicomInfo->image_position_patient0 = ds.image_position_patient;
						}
						dicomInfo->files.push_back(ds.file_path);
					}
					else
					{
						QByteArray ba = ds.file_path.toLocal8Bit();
						std::string file_path_std_string = ba.constData();
						common::Logger::instance()->Print(common::LogType::ERR, "Invalid dicom file : " + file_path_std_string);
					}

					i->dicom_infos.push_back(dicomInfo);
				}
			}

			if (found_patient)
			{
				continue;
			}

			// 읽은 file의 정보와 같은 수진자가 없는 경우
			// 해당 수진자 정보를 갖는 객체를 만듦
			PatientInfo* patientInfo = new PatientInfo;
			SetPatientInfo(patientInfo, ds);

			org_patients_.push_back(patientInfo);
		}
	}

	for (auto& patient : org_patients_)
	{
		for (auto& dicom_info : patient->dicom_infos)
		{
			if (!dicom_info->is_head_first)
			{
				common::Logger::instance()->Print(common::LogType::INF, std::string("This is not head first : reverse vector"));
				std::reverse(dicom_info->files.begin(), dicom_info->files.end());
				dicom_info->is_head_first = true;
			}
		}
	}

	common::Logger::instance()->Print(common::LogType::INF, std::string("end DicomLoader::LoadDicomFromFileList : ") + QString::number(timer.elapsed()).toStdString());
}

void DicomLoader::ReadFolder(const QString& path)
{
	if (!load_second_volume_)
	{
		if (selected_volume_path_ == path)
			return;
		selected_volume_path_ = path;
	}
	else
	{
		if (second_volume_path_ == path)
			return;
		second_volume_path_ = path;
	}

	if (!use_file_system_)
	{
		file_system_radio_->setChecked(true);
	}
	else
	{
		ResetStudyTree();
		ResetDicomInfoTable();

		this->LoadDicomFromPath(path);

		search_results_ = org_patients_;
		SetStudyTree();
	}
}

void DicomLoader::ResetDicomInfoTable()
{
	dicom_summary_table_->clearContents();
	dicom_summary_table_->setRowCount(0);
}

void DicomLoader::ResetStudyTree()
{
	for (auto &i : org_patients_)
	{
		for (auto &j : i->dicom_infos)
		{
			j->files.clear();
			SAFE_DELETE_OBJECT(j);
		}
		i->dicom_infos.clear();
		SAFE_DELETE_OBJECT(i);
	}
	org_patients_.clear();
	search_results_.clear();

	data_sets_.clear();
	study_tree_->clear();
	study_tree_->doItemsLayout();

	current_study_tree_item_ = nullptr;
}

void DicomLoader::SetStudyTree()
{
	if (search_results_.size() == 0)
		return;

	if (data_sets_.size() == 0)
		study_tree_->clear();

	for (int i = 0; i < search_results_.size(); i++)
	{
		PatientInfo* patient_info = search_results_.at(i);

		QStringList patient;
		patient.push_back(patient_info->patient_id);
		patient.push_back(patient_info->patient_name);
		patient.push_back(patient_info->patient_gender);
		patient.push_back(patient_info->patient_birth);
		patient.push_back("");
		patient.push_back("");
		patient.push_back("");
		patient.push_back("");
		patient.push_back(QString::number(i));

		QTreeWidgetItem* item = AddStudyTreeRoot(patient);

		int index = 0;

		for (auto &j : patient_info->dicom_infos)
		{
			index++;

			QString number;
			number.sprintf("%02d", index);

			QStringList study;
			study.push_back(number);
			study.push_back("");
			study.push_back("");
			study.push_back("");
			study.push_back(j->series_date);
			study.push_back(j->modality);
			study.push_back(QString::number(j->num_image));
			study.push_back(j->description);
			study.push_back("");
			study.push_back(j->file_dir);

			AddStudyTreeChild(item, study);
		}
	}
}

void DicomLoader::SetPatientInfo(PatientInfo* patient, const DataSet& data)
{
	patient->patient_id = data.patient_id;
	patient->patient_name = data.patient_name;
	patient->patient_gender = data.patient_gender;
	patient->patient_birth = data.patient_birth;
}

void DicomLoader::SetDicomInfo(DicomInfo* dicom, const DataSet& data)
{
	dicom->study_uid = QString::fromLocal8Bit(data.study_uid.c_str());
	dicom->series_uid = QString::fromLocal8Bit(data.series_uid.c_str());

	dicom->study_date = QString::fromLocal8Bit(data.study_date.c_str());
	dicom->series_date = QString::fromLocal8Bit(data.series_date.c_str());
	dicom->study_time = QString::fromLocal8Bit(data.study_time.c_str());
	dicom->series_time = QString::fromLocal8Bit(data.series_time.c_str());

	dicom->modality = QString::fromLocal8Bit(data.modality.c_str());

	dicom->kvp = QString::fromLocal8Bit(data.kvp.c_str());
	dicom->ma = QString::fromLocal8Bit(data.ma.c_str());
	dicom->exp_time = QString::fromLocal8Bit(data.exp_time.c_str());

	dicom->pixel_spacing = QString::fromLocal8Bit(data.pixel_spacing.c_str());
	dicom->slice_thickness = QString::fromLocal8Bit(data.slice_thickness.c_str());

	dicom->bits_allocated = QString::fromLocal8Bit(data.bits_allocated.c_str());
	dicom->bits_stored = QString::fromLocal8Bit(data.bits_stored.c_str());
	dicom->pixel_representation = data.pixel_representation;

	dicom->intercept = data.intercept;
	dicom->slope = data.slope;

	dicom->width = data.width;
	dicom->height = data.height;

	dicom->win_width = data.win_width;
	dicom->win_center = data.win_center;

	dicom->is_head_first = data.is_head_first;

	dicom->description = data.description;
}

QTreeWidgetItem* DicomLoader::AddStudyTreeRoot(const QStringList& text)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(study_tree_);
	item->setIcon(0, QIcon(QPixmap(":/image/dicomloader/id.png")));

	for (int i = 0; i < text.size(); i++)
		item->setText(i, text.at(i));

	item->setExpanded(false);

	return item;
}

void DicomLoader::AddStudyTreeChild(QTreeWidgetItem* parent, const QStringList& text)
{
	QTreeWidgetItem* item = new QTreeWidgetItem();
	for (int i = 0; i < text.size(); i++)
		item->setText(i, text.at(i));
	parent->addChild(item);
}

void DicomLoader::SearchStudy()
{
	study_tree_->clear();
	ResetDicomInfoTable();
	ResetThumbnail();

	search_results_.clear();

	if (search_id_.length() <= 0 && search_name_.length() <= 0)
	{
		search_results_ = org_patients_;
	}
	else
	{
		for (int i = 0; i < org_patients_.size(); i++)
		{
			PatientInfo* patient_info = org_patients_.at(i);

			if (search_id_.length() <= 0)
			{
				if (patient_info->patient_name.contains(search_name_, Qt::CaseInsensitive))
				{
					search_results_.push_back(patient_info);
				}
			}
			else if (search_name_.length() <= 0)
			{
				if (patient_info->patient_id.contains(search_id_, Qt::CaseInsensitive))
				{
					search_results_.push_back(patient_info);
				}
			}
			else
			{
				if (patient_info->patient_id.contains(search_id_, Qt::CaseInsensitive) &&
					patient_info->patient_name.contains(search_name_, Qt::CaseInsensitive))
				{
					search_results_.push_back(patient_info);
				}
			}
		}
	}

	SetStudyTree();
}

void DicomLoader::ResetThumbnail()
{
	for (int i = 0; i < 3; i++)
	{
		SAFE_DELETE_OBJECT(image_bufs_[i]);
		displays_[i]->setEmpty();
	}

	image_range_scroll_->setRange(0, 0);

	x_min_dspin_->setMinimum(0);
	x_min_dspin_->setMaximum(0);

	x_max_dspin_->setMinimum(0);
	x_max_dspin_->setMaximum(0);

	y_min_dspin_->setMinimum(0);
	y_min_dspin_->setMaximum(0);

	y_max_dspin_->setMinimum(0);
	y_max_dspin_->setMaximum(0);

	x_min_dspin_->setValue(0);
	x_max_dspin_->setValue(0);
	y_min_dspin_->setValue(0);
	y_max_dspin_->setValue(0);

	roi_width_->setText("0");
	roi_height_->setText("0");
}

int DicomLoader::GetPatientIndex(QTreeWidgetItem* item)
{
	return item->parent()->text(8).toInt();
}

int DicomLoader::GetStudyIndex(QTreeWidgetItem* item)
{
	return item->text(0).toInt() - 1;
}
#ifndef WILL3D_VIEWER
void DicomLoader::GetStudyListFromDB(bool recent)
{
	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (!db.open())
	{
		PrintErrorWill3DDB();
		return;
	}

	QString query = QString("select * from patient order by patient_id");
	sql_query_model_->setQuery(query, db);

	if (!CheckSqlError(sql_query_model_))
		return;

	QList<QSqlRecord> patient_record;
	for (int i = 0; i < sql_query_model_->rowCount(); i++)
		patient_record.push_back(sql_query_model_->record(i));

	for (int i = 0; i < patient_record.size(); i++)
	{
		QString patient_id = patient_record.at(i).value("patient_id").toString();
		QString patient_name = patient_record.at(i).value("patient_name").toString();

		query = QString("select * from series\
					 where patient_id = '%1' and patient_name = N'%2'\
					 and bool_recent = %3 order by description")
			.arg(patient_id, patient_name, QString::number(recent));
		sql_query_model_->setQuery(query, db);

		if (!CheckSqlError(sql_query_model_))
		{
			db.close();
			return;
		}

		if (sql_query_model_->rowCount() > 0)
		{
			PatientInfo* patient_info = new PatientInfo;

			patient_info->patient_id = patient_id;
			patient_info->patient_name = patient_name;
			patient_info->patient_gender = patient_record.at(i).value("gender").toString();
			patient_info->patient_birth = patient_record.at(i).value("birth_date").toString();

			for (int j = 0; j < sql_query_model_->rowCount(); j++)
			{
				DicomInfo* dicom_info = new DicomInfo;

				dicom_info->study_uid = sql_query_model_->record(j).value("study_uid").toString();
				dicom_info->series_uid = sql_query_model_->record(j).value("series_uid").toString();
				dicom_info->series_date = sql_query_model_->record(j).value("series_date").toString();
				dicom_info->modality = sql_query_model_->record(j).value("modality").toString();
				dicom_info->num_image = sql_query_model_->record(j).value("img_num").toInt();
				dicom_info->description = sql_query_model_->record(j).value("description").toString();
				dicom_info->file_dir = sql_query_model_->record(j).value("file_path").toString();
				dicom_info->project_path = sql_query_model_->record(j).value("project_path").toString();

				patient_info->dicom_infos.push_back(dicom_info);
			}

			org_patients_.push_back(patient_info);
		}
	}
	db.close();

	search_results_ = org_patients_;

	SetStudyTree();
}

void DicomLoader::PrintErrorWill3DDB()
{
	common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::PrintErrorWill3DDB : Can't open database.");
	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_07(), CW3MessageBox::Critical);
	msgBox.exec();
}

bool DicomLoader::CheckSqlError(QSqlQueryModel* model)
{
	QSqlError error = model->lastError();
	if (error.type() != QSqlError::ErrorType::NoError)
	{
		QString err = model->lastError().text();
		common::Logger::instance()->Print(common::LogType::INF, std::string("DicomLoader::CheckSqlError : ") + err.toStdString());
		CW3MessageBox msgBox("Will3D", err, CW3MessageBox::Critical);
		//msgBox.exec();

		QSqlDatabase::database(kConnectionName).close();
		return false;
	}
	return true;
}

void DicomLoader::OpenDicom(QTreeWidgetItem* item)
{
	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start OpenDicom");

	if (!item)
		return;

	if (item->childCount() != 0)
		return;

	int patient_index = GetPatientIndex(item);
	int series_index = GetStudyIndex(item);

	if (search_results_.size() <= patient_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::open: open error patient index.");
		return;
	}

	PatientInfo* patient_info = search_results_.at(patient_index);
	if (!patient_info)
		return;

	if (patient_info->dicom_infos.size() <= series_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::open: open error dicom index.");
		return;
	}

	DicomInfo* dicom_info = patient_info->dicom_infos.at(series_index);
	if (!dicom_info)
		return;

	if (dicom_info->modality.compare("CT", Qt::CaseInsensitive) != 0 &&
		dicom_info->modality.compare("MR", Qt::CaseInsensitive) != 0 &&
		dicom_info->modality.compare("PRJ", Qt::CaseInsensitive) != 0 &&
		dicom_info->modality.compare("PX", Qt::CaseInsensitive) != 0 &&
		dicom_info->modality.compare("US", Qt::CaseInsensitive) != 0)
	{
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_73(), CW3MessageBox::Information);
		msgBox.exec();
	}

	if (item->text(5).compare("PRJ") != 0 &&
		!open_from_external_program_)
		project_status_.load_project = false;

	if (!load_second_volume_)
	{
		opened_volume_path_ = dicom_info->file_dir;
	}
	else
	{
		second_volume_path_ = dicom_info->file_dir;
	}

	if (!open_from_external_program_)
		InsertDB(patient_index, series_index, true);

	if (!load_second_volume_)
	{
		PatientInfo* patient_info = search_results_.at(patient_index);
		if (patient_info)
		{
			current_first_volume_patient_id_ = patient_info->patient_id;
			current_first_volume_patient_name_ = patient_info->patient_name;
		}
	}

	if (!load_second_volume_)
	{
		if (selected_volume_path_.length() <= 0)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::open: invalid first vol path.");
			return;
		}
	}
	else
	{
		if (second_volume_path_.length() <= 0)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::open: invalid second vol path.");
			return;
		}
	}

	emit sigLoadDicomFinished();

	if (trd_file_path_from_external_program_.length() > 0)
	{
		common::Logger::instance()->Print(common::LogType::INF, trd_file_path_from_external_program_.toStdString());
		common::Logger::instance()->Print(common::LogType::INF, std::string("open_only_trd_ : ") + std::to_string(open_only_trd_));

		QFileInfo trd(trd_file_path_from_external_program_);
		if (trd.exists())
			emit sigSetTRDFromExternalProgram(trd_file_path_from_external_program_, open_only_trd_);
		else
		{
			common::Logger::instance()->Print(common::LogType::ERR, std::string("DicomLoader::open:") + trd_file_path_from_external_program_.toStdString() + std::string(" not found."));
		}
	}

	common::Logger::instance()->Print(common::LogType::INF, "end OpenDicom : " + QString::number(timer.elapsed()).toStdString() + " ms");
}

void DicomLoader::InsertDB(int patient_index, int study_index, bool recent)
{
	if (search_results_.size() <= patient_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::insertDB: patient index");
		return;
	}

	PatientInfo* patient_info = search_results_.at(patient_index);
	if (!patient_info)
		return;

	if (patient_info->dicom_infos.size() <= study_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::insertDB: dicom index");
		return;
	}

	DicomInfo* dicom_info = patient_info->dicom_infos.at(study_index);
	if (!dicom_info)
		return;

	const QString& patient_id = patient_info->patient_id;
	const QString& patient_name = patient_info->patient_name;
	const QString& patient_gender = patient_info->patient_gender;
	const QString& patient_birth = patient_info->patient_birth;

	const QString& study_uid = dicom_info->study_uid;
	const QString& series_uid = dicom_info->series_uid;
	const QString& study_date = dicom_info->study_date;
	const QString& modality = dicom_info->modality;
	QString num_image = QString::number(dicom_info->num_image);
	const QString& description = dicom_info->description;
	const QString& file_dir = dicom_info->file_dir;
	const QString& project_path = dicom_info->project_path;

	if (modality.compare("PRJ") == 0)
		return;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);

	if (!db.open())
	{
		PrintErrorWill3DDB();
		return;
	}

	QString query = QString("select * from patient where patient_id = '%1' and patient_name = N'%2'")
		.arg(patient_id, patient_name);
	sql_query_model_->setQuery(query, db);
	if (sql_query_model_->rowCount() == 0)
	{
		query = QString("insert into patient values('%1', N'%2', '%3', '%4', '%5')")
			.arg(patient_id, patient_name, patient_gender, patient_birth, "");
		sql_query_model_->setQuery(query, db);
	}

	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	query = QString("select * from series where study_uid = '%1' and series_uid = '%2' and modality = '%3' and bool_recent = %4")
		.arg(study_uid, series_uid, modality, QString::number(recent ? 1 : 0));
	sql_query_model_->setQuery(query, db);
	if (sql_query_model_->rowCount() == 0)
	{
		query = QString("insert into series values('%1', '%2', '%3', N'%4', '%5', '%6', '%7', N'%8', N'%9', '', %10)")
			.arg(study_uid, series_uid, patient_id, patient_name, study_date, modality, num_image, description, file_dir)
			.arg(QString::number(recent ? 1 : 0));
		sql_query_model_->setQuery(query, db);
	}

	CheckSqlError(sql_query_model_);
	db.close();
}

void DicomLoader::DeleteFromDB(int patient_index, int series_index)
{
	if (search_results_.size() <= patient_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"DicomLoader::deleteDB: invalid patient index");
		return;
	}

	PatientInfo* patient = search_results_.at(patient_index);
	if (!patient)
		return;

	if (patient->dicom_infos.size() <= series_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"DicomLoader::deleteDB: invalid series index");
		return;
	}

	DicomInfo* dicom_info = patient->dicom_infos.at(series_index);
	if (!dicom_info)
		return;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (!db.open())
	{
		PrintErrorWill3DDB();
		return;
	}

	QString query = QString("delete from series where study_uid = '%1'"
		" and series_uid = '%2' and project_path = N'%3' and bool_recent = 0")
		.arg(dicom_info->study_uid, dicom_info->series_uid, dicom_info->project_path);
	sql_query_model_->setQuery(query, db);
	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	// series 에서 해당 정보로 지운 후, series에 해당 환자의
	// patient_id, patient_name 로 검색되는 내용이 없으면
	// patient 에서 해당 환자정보를 삭제한다.
	query = QString("select * from series where patient_id = '%1' and patient_name = '%2'")
		.arg(patient->patient_id, patient->patient_name);
	sql_query_model_->setQuery(query, db);
	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	if (sql_query_model_->rowCount() == 0)
	{
		query = QString("delete from patient where patient_id = '%1' and patient_name = N'%2'")
			.arg(patient->patient_id, patient->patient_name);
		sql_query_model_->setQuery(query, db);
		if (!CheckSqlError(sql_query_model_))
		{
			db.close();
			return;
		}
	}

	db.close();
}

void DicomLoader::DeleteFromRecent(int patient_index, int series_index)
{
	if (search_results_.size() <= patient_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"DicomLoader::deleteDB: invalid patient index");
		return;
	}

	PatientInfo* patient = search_results_.at(patient_index);
	if (!patient)
		return;

	if (patient->dicom_infos.size() <= series_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR,
			"DicomLoader::deleteDB: invalid series index");
		return;
	}

	DicomInfo* dicom_info = patient->dicom_infos.at(series_index);
	if (!dicom_info)
		return;

	QSqlDatabase db = QSqlDatabase::database(kConnectionName);
	if (!db.open())
	{
		PrintErrorWill3DDB();
		return;
	}

	QString query = QString("delete from series where study_uid = '%1'"
		" and series_uid = '%2' and project_path = N'%3' and bool_recent = 1")
		.arg(dicom_info->study_uid, dicom_info->series_uid, dicom_info->project_path);
	sql_query_model_->setQuery(query, db);
	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	// series 에서 해당 정보로 지운 후, series에 해당 환자의
	// patient_id, patient_name 로 검색되는 내용이 없으면
	// patient 에서 해당 환자정보를 삭제한다.
	query = QString("select * from series where patient_id = '%1' and patient_name = '%2'")
		.arg(patient->patient_id, patient->patient_name);
	sql_query_model_->setQuery(query, db);
	if (!CheckSqlError(sql_query_model_))
	{
		db.close();
		return;
	}

	if (sql_query_model_->rowCount() == 0)
	{
		query = QString("delete from patient where patient_id = '%1' and patient_name = N'%2'")
			.arg(patient->patient_id, patient->patient_name);
		sql_query_model_->setQuery(query, db);
		if (!CheckSqlError(sql_query_model_))
		{
			db.close();
			return;
		}
	}

	db.close();
}
#endif
void DicomLoader::resizeEvent(QResizeEvent* event)
{
	QMainWindow::resizeEvent(event);

	int center_x[3];
	center_x[0] = (displays_[0]->width() + displays_[0]->x() + displays_[0]->x()) / 2;
	center_x[1] = (displays_[1]->width() + displays_[1]->x() + displays_[1]->x()) / 2;
	center_x[2] = (displays_[2]->width() + displays_[2]->x() + displays_[2]->x()) / 2;

	int center_y[3];
	center_y[0] = (displays_[0]->height() + displays_[0]->y() + displays_[0]->y()) / 2;
	center_y[1] = (displays_[1]->height() + displays_[1]->y() + displays_[1]->y()) / 2;
	center_y[2] = (displays_[2]->height() + displays_[2]->y() + displays_[2]->y()) / 2;

	int half = 1;
	if (displays_[0]->width() < displays_[0]->height())
		half = displays_[0]->width() * 0.5f;
	else
		half = displays_[0]->height() * 0.5f;

	displays_[0]->setGeometry(center_x[0] - half, center_y[0] - half, half * 2, half * 2);
	displays_[1]->setGeometry(center_x[1] - half, center_y[1] - half, half * 2, half * 2);
	displays_[2]->setGeometry(center_x[2] - half, center_y[2] - half, half * 2, half * 2);
}

void DicomLoader::AddDicomInfoTable(const QString& name, const QString& value)
{
	QTableWidgetItem* name_item = new QTableWidgetItem();
	name_item->setText(name);

	QTableWidgetItem* value_item = new QTableWidgetItem();
	value_item->setText(value);

	dicom_summary_table_->setRowCount(dicom_summary_table_->rowCount() + 1);

	dicom_summary_table_->setItem(dicom_summary_table_->rowCount() - 1, 0, name_item);
	dicom_summary_table_->setItem(dicom_summary_table_->rowCount() - 1, 1, value_item);
}
#ifndef WILL3D_VIEWER
QString DicomLoader::GetWillmasterDicomPath()
{
	bool use_willmaster = GlobalPreferences::GetInstance()->preferences_.general.network_setting.use_willmaster;
	if (!use_willmaster)
		return QString();

#if 0
	bool network = GlobalPreferences::GetInstance()->preferences_.general.network_setting.connection_type == GlobalPreferences::ConnectionType::Network;
	QString ip_address = GlobalPreferences::GetInstance()->preferences_.general.network_setting.ip_address;
	if (!network)
		ip_address = "127.0.0.1";

	int port = GlobalPreferences::GetInstance()->preferences_.general.database.port;
	int odbc_version = GlobalPreferences::GetInstance()->preferences_.general.database.version;

	QSqlDatabase db_willmaster = QSqlDatabase();
	db_willmaster.removeDatabase("WillMaster");

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

#if defined(__APPLE__)
	db_willmaster = QSqlDatabase::addDatabase("QMYSQL", "WillMaster");
	db_willmaster.setHostName(ip_address);
	db_willmaster.setPort(port);
	db_willmaster.setDatabaseName("willmaster");
	db_willmaster.setUserName("root");
#else
	db_willmaster = QSqlDatabase::addDatabase("QODBC", "WillMaster");
#if MARIA_DB
	db_willmaster.setDatabaseName(QString("DRIVER={%1};SERVER=%2;DATABASE=willmaster;Port=%3;UID=root;PWD=2002;WSID=.;").arg("SQL Server").arg(ip_address).arg(port));
#else
	db_willmaster.setDatabaseName(QString("DRIVER={SQL Server};SERVER=%1\\HDXWILL;DATABASE=willmaster;Port=1433;UID=sa;PWD=2002;WSID=.;").arg(ip_address));
#endif
#endif
	QSqlQueryModel query_model_willmaster;

	if (!db_willmaster.open())
	{
		PrintErrorWill3DDB();
		return QString();
	}

	QString query = QString("select storage_localpath from storagemgr where storage_no = 0");
	query_model_willmaster.setQuery(query, db_willmaster);

	if (!CheckSqlError())
	{
		db_willmaster.close();
		return QString();
	}

	QString willmaster_dicom_path = query_model_willmaster.record(0).value("storage_localpath").toString();
	//QString willmaster_dicom_path = query_model_willmaster.record(0).value("STORAGE_PATH").toString();	

	db_willmaster.close();
	//db_willmaster = QSqlDatabase();
	//db_willmaster.removeDatabase("WillMaster");

	return willmaster_dicom_path;
#else
	bool network = GlobalPreferences::GetInstance()->preferences_.general.network_setting.connection_type == GlobalPreferences::ConnectionType::Network;
	QString ip_address = !network ? "127.0.0.1" : GlobalPreferences::GetInstance()->preferences_.general.network_setting.ip_address;

	return  "//" + ip_address;
#endif
}
#endif
void DicomLoader::SetOpenFromExternalProgramMode(bool enable)
{
	open_from_external_program_ = enable;

	// (임시)willmaster 에서 열어도 수동으로 다른 폴더나 db 에 있는 영상 열 수 있도록 개방
	return;

	path_input_->setDisabled(enable);
	explorer_tree_->setDisabled(enable);
	patient_image_dir_tree_->setVisible(false);
	patient_image_dir_tree_->setDisabled(enable);
	patient_id_input_->setDisabled(enable);
	patient_name_input_->setDisabled(enable);
	file_system_radio_->setDisabled(enable);
	database_radio_->setDisabled(enable);
	recent_radio_->setDisabled(enable);
}

void DicomLoader::InitROIControllerAndDicomSummary()
{
	if (!current_study_tree_item_)
		return;

	QTreeWidgetItem* top_level_item = nullptr;

	top_level_item = (current_study_tree_item_->childCount() == 0) ? current_study_tree_item_->parent() : current_study_tree_item_;
	if (top_level_item)
		top_level_item->setIcon(0, QIcon(QPixmap(":/image/dicomloader/id_on.png")));

	ResetDicomInfoTable();
	ResetThumbnail();

	if (current_study_tree_item_->childCount() != 0)
	{
		return;
	}

	int patient_index = GetPatientIndex(current_study_tree_item_);
	if (search_results_.size() <= patient_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::InitROIControllerAndDicomSummary: patient index");
		return;
	}

	PatientInfo* patient_info = search_results_.at(patient_index);
	if (!patient_info)
		return;

	int study_index = GetStudyIndex(current_study_tree_item_);
	if (patient_info->dicom_infos.size() <= study_index)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::InitROIControllerAndDicomSummary: dicom index");
		return;
	}

	DicomInfo* dicom_info = patient_info->dicom_infos.at(study_index);
	if (!dicom_info)
		return;

	// for database, recent
	if (!use_file_system_)
	{
		if (dicom_info->files.size() <= 0)
		{
			this->LoadDicomFromPath(dicom_info->file_dir,
				dicom_info->study_uid,
				dicom_info->series_uid);
		}

		if (current_study_tree_item_->text(5).compare("PRJ") != 0 &&
			!open_from_external_program_)
		{
			project_status_.load_project = false;

			if (!load_second_volume_)
				selected_volume_path_ = dicom_info->file_dir;
			else
				second_volume_path_ = dicom_info->file_dir;
		}
	}

	///////////////////////////////////////////////
	// v1.0.2 save/load volume range & area
	float load_range_start = -1.0f;
	float load_range_end = -1.0f;
	QRectF load_area_rect;
	///////////////////////////////////////////////

	if (current_study_tree_item_->text(5).compare("PRJ") == 0)
	{
#ifndef WILL3D_VIEWER
		dicom_info->modality = "PRJ";
		dicom_info->file_dir = selected_volume_path_;
		project_path_ = dicom_info->project_path;

		std::string strLogMsg = QString("selected project path : %1").arg(project_path_).toStdString();
		common::Logger::instance()->Print(common::LogType::INF, strLogMsg);

		QFile project(project_path_);

		// load
		if (!project.open(QIODevice::ReadOnly))
		{
			common::Logger::instance()->Print(common::LogType::ERR,
				"CW3TabMgr::loadProject: failed to load project.");
			return;
		}

		ProjectIO io(project::Purpose::LOAD, project_path_);
		ProjectIOFile& file_io = io.GetFileTabIO();
		project::LoadVolInfo vol_info;
		file_io.LoadVolumeInfo(vol_info);
		project_status_.load_project = true;

		load_range_start = vol_info.range_start;
		load_range_end = vol_info.range_end;

		load_area_rect = QRectF(vol_info.area_x, vol_info.area_y,
			vol_info.width, vol_info.height);
#endif
		/////////////////////////////////////////////////
	}
	else if (!open_from_external_program_)
	{
		project_status_.load_project = false;
	}

	if (dicom_info->files.size() <= 0)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::InitROIControllerAndDicomSummary: no file");
		return;
	}

	// display dicom_info info
	AddDicomInfoTable("Series Time", dicom_info->series_time);
	AddDicomInfoTable("Modality", dicom_info->modality);
	AddDicomInfoTable("kVp", dicom_info->kvp);
	AddDicomInfoTable("mA", dicom_info->ma);
	AddDicomInfoTable("Exposure Time", dicom_info->exp_time + QString(" sec"));
	AddDicomInfoTable("Pixel Spacing", dicom_info->pixel_spacing + QString(" mm"));
	AddDicomInfoTable("Slice Thickness", dicom_info->slice_thickness + QString(" mm"));
	AddDicomInfoTable("Bits Allocated", dicom_info->bits_allocated);
	AddDicomInfoTable("Bits Stored", dicom_info->bits_stored);
	AddDicomInfoTable("Pixel Representation", QString::number(dicom_info->pixel_representation));
	AddDicomInfoTable("Rescale Intercept", QString::number(dicom_info->intercept));
	AddDicomInfoTable("Rescale slope", QString::number(dicom_info->slope));
	AddDicomInfoTable("Rows", QString::number(dicom_info->width) + QString(" px"));
	AddDicomInfoTable("Columns", QString::number(dicom_info->height) + QString(" px"));
	AddDicomInfoTable("Window Width", QString::number(dicom_info->win_width));
	AddDicomInfoTable("Window Center", QString::number(dicom_info->win_center));

	dicom_summary_table_->resizeColumnsToContents();
	dicom_summary_table_->resizeRowsToContents();

	dicom_summary_table_->setColumnWidth(0, dicom_summary_table_->columnWidth(0) * 1.5f);

	if (dicom_info->width <= 0 || dicom_info->height <= 0)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DicomLoader::slotSeriesTreeClicked: img size");
		return;
	}

	for (int i = 0; i < 3; i++)
	{
		SAFE_DELETE_OBJECT(image_bufs_[i]);
		image_bufs_[i] = new short[dicom_info->width * dicom_info->height];
		memset(image_bufs_[i], 0, dicom_info->width * dicom_info->height * sizeof(short));
		displays_[i]->setEmpty();
	}

	int first = 0;
	int middle = (int)(dicom_info->files.size() / 2.0f);
	int last = dicom_info->files.size() - 1;
	CW3DicomIO dicom_io;

	QByteArray ba = dicom_info->files.at(first).toLocal8Bit();
	std::string filePath = ba.constData();

	dicom_io.getImageBufFromFile(filePath, image_bufs_[0]);

	displays_[0]->setImage(image_bufs_[0], dicom_info->width, dicom_info->height, dicom_info->win_width, dicom_info->win_center, load_area_rect);

	if (first != last)
	{
		ba = dicom_info->files.at(middle).toLocal8Bit();
		filePath = ba.constData();
		dicom_io.getImageBufFromFile(filePath, image_bufs_[1]);
		displays_[1]->setImage(image_bufs_[1], dicom_info->width, dicom_info->height, dicom_info->win_width, dicom_info->win_center, load_area_rect);

		ba = dicom_info->files.at(last).toLocal8Bit();
		filePath = ba.constData();
		dicom_io.getImageBufFromFile(filePath, image_bufs_[2]);
		displays_[2]->setImage(image_bufs_[2], dicom_info->width, dicom_info->height, dicom_info->win_width, dicom_info->win_center, load_area_rect);
	}
	if (load_range_start >= 0.0f && load_range_end > 0.0f)
		image_range_scroll_->setValidRange(first, last, load_range_start, load_range_end);
	else
		image_range_scroll_->setRange(first, last);

	x_min_dspin_->setMinimum(1);
	x_min_dspin_->setMaximum(displays_[0]->left_max() + 1);

	x_max_dspin_->setMinimum(displays_[0]->right_min());
	x_max_dspin_->setMaximum(dicom_info->width);

	y_min_dspin_->setMinimum(1);
	y_min_dspin_->setMaximum(displays_[0]->top_max() + 1);

	y_max_dspin_->setMinimum(displays_[0]->bottom_min());
	y_max_dspin_->setMaximum(dicom_info->height);

	if (load_area_rect.width() > 0.0f && load_area_rect.height() > 0.0f)
	{
		x_min_dspin_->setValue(load_area_rect.x() + 1);
		x_max_dspin_->setValue(load_area_rect.x() + load_area_rect.width());
		y_min_dspin_->setValue(load_area_rect.y() + 1);
		y_max_dspin_->setValue(load_area_rect.y() + load_area_rect.height());
	}
	else
	{
		x_min_dspin_->setValue(1);
		x_max_dspin_->setValue(dicom_info->width);
		y_min_dspin_->setValue(1);
		y_max_dspin_->setValue(dicom_info->height);
	}

	roi_width_->setText(QString::number(dicom_info->width));
	roi_height_->setText(QString::number(dicom_info->height));

	bool is_enable_ui = !project_status_.load_project;
	displays_[0]->setEnabled(is_enable_ui);
	displays_[1]->setEnabled(is_enable_ui);
	displays_[2]->setEnabled(is_enable_ui);

	image_range_scroll_->setEnabled(is_enable_ui);

	x_min_dspin_->setEnabled(is_enable_ui);
	x_max_dspin_->setEnabled(is_enable_ui);
	y_min_dspin_->setEnabled(is_enable_ui);
	y_max_dspin_->setEnabled(is_enable_ui);
}
