#pragma once
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
#include <qicon.h>

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;
class CW3View3DImplantPreview;

class QTabWidget;
class QHBoxLayout;
class QVBoxLayout;
class QToolButton;
class QTableWidget;
class QTableWidgetItem;

class UIFRAME_EXPORT CW3ImplantDBDlg : public CW3Dialog
{
	Q_OBJECT

public:
	CW3ImplantDBDlg(CW3VREngine* vr_engine, CW3MPREngine* mpr_engine, CW3ResourceContainer* resource_container, QWidget* parent = nullptr);
	virtual ~CW3ImplantDBDlg();

public slots:
	virtual void accept() override;
	virtual void reject() override;

private slots:
	void slotUpdateGeneralProductTable(int, int, int, int);
	void slotUpdateGeneralImplantTable(int, int, int, int);
	void slotClickedImplantTable(int, int);

	void slotUpdateCustomProductTable(int current_row, int current_column, int previous_row, int previous_column);
	void slotUpdateCustomImplantTable(int current_row, int current_column, int previous_row, int previous_column);

	void slotApply();
	void slotCancel();
	void slotAdd();

	void slotGeneralManufacturerItemChecked(QTableWidgetItem* item);
	void slotGeneralProductItemChecked(QTableWidgetItem* item);

	void slotCustomManufacturerItemChecked(QTableWidgetItem* item);
	void slotCustomProductItemChecked(QTableWidgetItem* item);

	void slotCustomManufacturerCellClicked(int row, int column);
	void slotCustomProductCellClicked(int row, int column);
	void slotCustomImplantCellClicked(int row, int column);

	void slotTabChanged(int index);

	void slotAddCustomImplantButtonClicked();
	void slotModifyCustomImplantButtonClicked();
	void slotRemoveCustomImplantButtonClicked();

	void slotAddCustomImplant(
		const QString& manufacturer,
		const QString& product,
		const QString& model,
		const float coronal_diameter,
		const float apical_diameter,
		const float length
	);
	void slotModifyCustomImplant(
		const QString& manufacturer,
		const QString& product,
		const QString& model,
		const float coronal_diameter,
		const float apical_diameter,
		const float length
	);

protected:
	virtual void closeEvent(QCloseEvent* e);

signals:
	void sigProductUpdate(int);
	void sigCustomImplantUpdated();
	void sigFavoriteUpdated();

private:
	enum class Tab
	{
		GENERAL,
		CUSTOM

	};

	enum class CustomTableType
	{
		NONE,
		MANUFACTURER,
		PRODUCT,
		IMPLANT
	};

	void SetLayout();
	QTabWidget* CreateTabWidget();
	QHBoxLayout* CreateButtonLayout();
	QHBoxLayout* CreateGeneralLayout();
	QHBoxLayout* CreateCustomLayout();

	void SetButtonMode(Tab mode);

	void InitFavoriteDatabase();
	void InitGeneralData();
	void InitCustomData();
	void InitGeneralTables();
	void InitCustomTables();
	void connections();
	void disconnections();

	void UpdateManufacturerList(
		const QString& selected_manufacturer_name = QString());
	void UpdateProductList(const QString& selected_product_name = QString());
	void UpdateImplantList();

	void UpdateCustomManufacturerList(const QString& selected_manufacturer_name = QString());
	void UpdateCustomProductList(const QString& selected_product_name = QString());
	void UpdateCustomImplantList();

	void UpdatePreview();

	// Database 관련 함수 모음
	void connectDB();
	void disConnectDB();
	void createDBTable_favorites(const QString& tableName);
	void updateDBTable_favorites();

	bool existDBTable_favorites(const QString& table_name);
	bool existDBRecord_favorites(const QString& manufacturer_name);
	bool existDBRecord_favorites(const QString& manufacturer_name,
		const QString& product_name);

	bool insert_favorites_temp(const QString& manufacturer_name);
	void insert_favorites_temp(const QString& manufacturer_name,
		const QString& product_name);

	void delete_favorites_temp(const QString& manufacturer_name);
	void delete_favorites_temp(const QString& manufacturer_name,
		const QString& product_name);

	void CopyTable(const QString& from_table_name, const QString& to_table_name);

	void UpdateProductDBTable();

	void UpdateDBTable(const QStringList& implant_path);

	void RemoveSelectedManufacturer();
	void RemoveSelectedProduct();
	void RemoveSelectedImplant();

	void RemoveManufacturerToDatabase(int manufacturer_id);
	void RemoveProductToDatabase(int product_id);
	void RemoveImplantToDatabase(int implant_id);

	QStringList GetManufacturerList();
	QStringList GetProductList();

private:
	QTableWidget* general_manufacturer_list_ = nullptr;
	QTableWidget* general_product_list_ = nullptr;
	QTableWidget* general_implant_list_ = nullptr;

	QTableWidget* custom_manufacturer_list_ = nullptr;
	QTableWidget* custom_product_list_ = nullptr;
	QTableWidget* custom_implant_list_ = nullptr;

	int curr_implant_row_ = -1;  // 현재 선택된 임플란트 테이블의 row

	CW3View3DImplantPreview* general_preview_ = nullptr;
	//CW3View3DImplantPreview* custom_preview_ = nullptr;

	QToolButton* apply_button_ = nullptr;
	QToolButton* cancel_button_ = nullptr;
	QToolButton* add_button_ = nullptr;

	QToolButton* custom_add_button_ = nullptr;
	QToolButton* custom_remove_button_ = nullptr;
	QToolButton* custom_modify_button_ = nullptr;

	Tab current_tab_ = Tab::GENERAL;

	CustomTableType focused_table_ = CustomTableType::NONE;
};
