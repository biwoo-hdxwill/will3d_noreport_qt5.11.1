#pragma once

/*=========================================================================

File:			class DicomLoader
Language:		C++11
Library:		Qt 5.8
Author:			Jung Dae Gun
First date:		2018-04-10
Last modify:	2018-04-10

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <QMainWindow>
#include <QPainter>
#include <QStyledItemDelegate>

#include "../../Common/Common/W3Point3D.h"

#include "uiframe_global.h"

class CW3DicomLoaderThumbnailView;
class CW3DicomLoaderScrollbar;
class QFileSystemModel;
#ifndef WILL3D_VIEWER
class QSqlDatabase;
class QSqlQueryModel;
#endif
class QSpinBox;
class QTreeView;
class QTreeWidgetItem;
class QTreeWidget;
class QTableWidget;
class QCompleter;
class GroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QRadioButton;
class FileSystemTreeWidget;

class UIFRAME_EXPORT GridDelegate : public QStyledItemDelegate
{
public:
	explicit GridDelegate(QObject * parent = 0) : QStyledItemDelegate(parent) {}

	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		painter->save();
		painter->setPen(QColor(Qt::lightGray));
		QRect rect = option.rect;
		rect.setX(0);
		painter->drawRect(rect);
		painter->restore();

		QStyledItemDelegate::paint(painter, option, index);
	}
};

class UIFRAME_EXPORT DicomLoader : public QMainWindow
{
	Q_OBJECT
public:
	DicomLoader(QWidget* parent = 0);
	~DicomLoader();

	struct DataSet
	{
		QString file_path;

		QString patient_id;
		QString patient_name;
		QString patient_gender;
		QString patient_birth;

		std::string study_uid;
		std::string series_uid;

		std::string study_date;
		std::string series_date;
		std::string study_time;
		std::string series_time;

		std::string modality;

		std::string kvp;
		std::string ma;
		std::string exp_time;

		std::string pixel_spacing;
		std::string slice_thickness;

		std::string bits_allocated;
		std::string bits_stored;

		int pixel_representation = 0;

		int intercept = 0;
		float slope = 0;

		int width = 0;
		int height = 0;
		int win_width = 0;
		int win_center = 0;

		bool is_head_first = true;

		QString description;

		float image_position_patient = 0.0f;

		int instance_number = 0;
		float slice_location = 0.0f;
	};

	struct DicomInfo
	{
		QString study_uid;
		QString series_uid;

		QString study_date;
		QString series_date;
		QString study_time;
		QString series_time;

		QString modality;

		QString kvp;
		QString ma;
		QString exp_time;

		QString pixel_spacing;
		QString slice_thickness;

		QString bits_allocated;
		QString bits_stored;

		int pixel_representation = 0;

		int intercept = 0;
		float slope = 0;

		int width = 0;
		int height = 0;
		int win_width = 0;
		int win_center = 0;

		int num_image = 0;

		float image_position_patient0 = 0.0f;
		float image_position_patient1 = 0.0f;

		bool is_head_first = true;

		QString description;
		QString file_dir;
		QString project_path;

		std::vector<QString> files;
	};

	struct PatientInfo
	{
		QString patient_id;
		QString patient_name;
		QString patient_gender;
		QString patient_birth;

		std::vector<DicomInfo *> dicom_infos;
	};

#ifndef WILL3D_VIEWER
	void InsertDBforProject(PatientInfo* patient);
#endif
	std::vector<QString> GetSelectedFileList();
	CW3Point3D GetSelectedRange();
	QRect GetSelectedArea();
#ifndef WILL3D_VIEWER
	void SetEnableLoadSecondVolume(bool second);
#endif
	void ReadInputFile(const QString& path);
	void ApplyPreferences();

	inline QString opend_volume_path() { return opened_volume_path_; }
	inline QString project_path() { return project_path_; }
	inline bool is_project() { return project_status_.load_project; }
	inline bool open_only_trd() { return open_only_trd_; }

signals:
	void sigLoadDicomFinished();
	void sigSetTRDFromExternalProgram(const QString, const bool);

public slots:
#ifndef WILL3D_VIEWER
	void slotOpenDicom();
#endif

protected:
	virtual void resizeEvent(QResizeEvent *event) override;

private:
	void SetLayout();
#ifndef WILL3D_VIEWER
	void InitDatabaseConnection();
	void UpdateImplantDatabase(QSqlDatabase& db);
#endif
	QWidget* CreateGroupBoxWidget(GroupBox* group_box);
	GroupBox* CreateEmptyGroupBox(const QString& caption);
	QWidget* CreateGroupBox(const QString& caption, QVBoxLayout* contents_layout);
	QWidget* CreateGroupBox(const QString& caption, QHBoxLayout* contents_layout);
	QWidget* CreateGroupBox(const QString& caption, QGridLayout* contents_layout);
	void InitTreeView(QTreeView* tree_view, QFileSystemModel* fsm, const QString& root_path);
	void InitStudyTreeWidget();
	void InitDicomSummaryTableWidget();
	QWidget* CreateExplorerWidget();
	QWidget* CreateDataWidget();
	QWidget* CreateDataListWidget();
	QWidget* CreateDicomInformationWidget();
	void LoadDicomFromPath(const QString& path);
	void LoadDicomFromPath(const QString& path, const QString& study_uid, const QString& series_uid);
	void LoadDicomFromFileList(QStringList& file_list);
	void LoadDicomFromFileList(QStringList& file_list, const QString& study_uid, const QString& series_uid);
	void ReadFolder(const QString& path);
	void ResetDicomInfoTable();
	void ResetStudyTree();
	void SetStudyTree();
	void SetPatientInfo(PatientInfo* patient, const DataSet& data);
	void SetDicomInfo(DicomInfo* dicom, const DataSet& data);
	QTreeWidgetItem* AddStudyTreeRoot(const QStringList& text);
	void AddStudyTreeChild(QTreeWidgetItem* parent, const QStringList& text);
	void SearchStudy();
	void ResetThumbnail();
	int GetPatientIndex(QTreeWidgetItem* item);
	int GetStudyIndex(QTreeWidgetItem* item);
#ifndef WILL3D_VIEWER
	void OpenDicom(QTreeWidgetItem* item);
	void GetStudyListFromDB(bool recent);
	void PrintErrorWill3DDB();
	bool CheckSqlError(QSqlQueryModel* model);
	void InsertDB(int patient_index, int study_index, bool recent);
	void DeleteFromDB(int patient_index, int study_index);
	void DeleteFromRecent(int patient_index, int study_index);
	QString GetWillmasterDicomPath();
#endif
	void AddDicomInfoTable(const QString& name, const QString& value);
	void SetOpenFromExternalProgramMode(bool enable);
	void InitROIControllerAndDicomSummary();

private slots:
#ifndef WILL3D_VIEWER
	void slotPathEditFinished();
	void slotExplorerTreeClicked(QTreeWidgetItem* item, int column);
	void slotPatientFolderViewClicked(QModelIndex index);
	void slotSearch();
	void slotSearchID();
	void slotSearchName();
	void slotWheelTranslate(int pos, int wheel);
	void slotScrollChanged(int index, float pos);
	void slotRadioFileSystemClicked(bool checked);
	void slotRadioDatabaseClicked(bool checked);
	void slotRadioRecentClicked(bool checked);
	void slotResetROI();
	void slotStudyTreeMenu(const QPoint& pos);
	void slotImportToDB();
	void slotDeleteFromRecent();
	void slotDeleteFromDB();
	void slotSeriesTreeClicked(QTreeWidgetItem* item, int column);
	void slotSeriesTreeDoubleClicked(QTreeWidgetItem* item, int column);
	void slotChangeSelectedArea(QRectF area, int index);
#endif

private:
	QLineEdit* path_input_ = nullptr;
	QLineEdit* patient_id_input_ = nullptr;
	QLineEdit* patient_name_input_ = nullptr;
	QLineEdit* roi_width_ = nullptr;
	QLineEdit* roi_height_ = nullptr;

	QSpinBox* x_min_dspin_ = nullptr;
	QSpinBox* x_max_dspin_ = nullptr;
	QSpinBox* y_min_dspin_ = nullptr;
	QSpinBox* y_max_dspin_ = nullptr;

	QRadioButton* file_system_radio_ = nullptr;
	QRadioButton* database_radio_ = nullptr;
	QRadioButton* recent_radio_ = nullptr;

	QTreeWidget* study_tree_ = nullptr;

	FileSystemTreeWidget* explorer_tree_ = nullptr;
	QTreeView* patient_image_dir_tree_ = nullptr;

	QTableWidget* dicom_summary_table_ = nullptr;
	QTreeWidgetItem* current_study_tree_item_ = nullptr;

	QFileSystemModel* fsm_patient_image_dir_ = nullptr;

	CW3DicomLoaderThumbnailView* displays_[3] = {nullptr,nullptr, nullptr};
	CW3DicomLoaderScrollbar* image_range_scroll_ = nullptr;

	QCompleter* completer_ = nullptr;

	QList<DataSet> data_sets_;
	std::vector<PatientInfo*> org_patients_;
	std::vector<PatientInfo*> search_results_;

	short* image_bufs_[3] = { nullptr, nullptr, nullptr };

#ifndef WILL3D_VIEWER
	QSqlQueryModel* sql_query_model_ = nullptr;
#endif

	QMenu* study_tree_menu_ = nullptr;

	QAction* import_to_db_action_ = nullptr;
	QAction* delete_from_recent_action_ = nullptr;
	QAction* delete_from_db_action_ = nullptr;

	QString selected_volume_path_;
	QString opened_volume_path_;
	QString second_volume_path_;
	QString search_id_;
	QString search_name_;
	QString project_path_;
	QString current_first_volume_patient_id_;
	QString current_first_volume_patient_name_;
	QString trd_file_path_from_external_program_;

	bool use_file_system_ = true;
	bool load_second_volume_ = false;
	bool open_from_external_program_ = false;
	bool open_only_trd_ = false;

	struct ProjectStatus
	{
		bool load_project = false;
		QPoint pos = QPoint(-1, -1);
		QPoint size = QPoint(-1, -1);
	};
	ProjectStatus project_status_;
};
