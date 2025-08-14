#pragma once
/*=========================================================================

File:			class ImplantPreferenceDlg
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Seo Seok Man
First date:		2018-04-03
Last modify:	2018-04-03

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <qsqlerror.h>
#include <map>
#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QTableWidget;
class QTableWidgetItem;
class QToolButton;
class QRadioButton;
class QLabel;

class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;
class CW3View3DImplantPreview;
class ImplantPreferenceItem;

namespace implant_preset
{
	// ini 에서 preset을 가져오기 위한 문자열들
	const QString kDisplayedPanelsSectionName("PANEL_DISPLAYED_IMPLANT");
	const QString kSectionName("IMPLANT_PRESET_");
	const QString kKeyManufacturer("/manufacturer");
	const QString kKeyProduct("/product");
	const QString kKeyModel("/model");
}  // namespace implant_preset

class UIFRAME_EXPORT ImplantPreferenceDlg : public CW3Dialog
{
	Q_OBJECT

public:
	ImplantPreferenceDlg(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
		CW3ResourceContainer* Rcontainer, QWidget* pParent);
	~ImplantPreferenceDlg();

protected:
	virtual void closeEvent(QCloseEvent* e) override;

	private slots:
	void slotClose();
	void slotDeleteAll();
	void slotApply();
	void slotLRChanged(bool);
	void slotClearCurrentItem(int implant_number);
	void slotSetCurrentItemInfo(const QString& manufacturer_name,
		const QString& product_name,
		const QString& diameter, const QString& length);

	void slotUpdateProductTable(int, int, int, int);
	void slotUpdateImplantTable(int, int, int, int);
	void slotClickedImplantTable(int, int);

private:
	enum class CurrentPreset { RIGHT, LEFT };

private:
	void Connections();
	void Disconnections();
	void LoadPresets();
	void UpdatePreset(int preset_index);
	void ClearPreset(int preset_index);
	void ClearTables();

	// about DB
	void ConnectDB();
	void DisconnectDB();
	bool GetImplantSpecs(const QString &manufacturer_name,
		const QString &product_name,
		const QString &implant_name,
		QString &diameter, QString &length);
	int QueryManufacturerID(const QString& manufacturer_name);

	void UpdateManufacturerList(const QString& manufacturer_name = QString());
	void UpdateProductList(const QString& selected_product_name = QString());
	void UpdateImplantList(const QString& diameter = QString(),
		const QString& length = QString());
	void UpdatePreview(const bool active = true);

	void PrintDBErrMsg(const QSqlError& err, const char* msg);

private:
	ImplantPreferenceItem* preset_items_[14];
	QTableWidget* manufacturer_list_ = nullptr;
	QTableWidget* product_list_ = nullptr;
	QTableWidget* implant_list_ = nullptr;

	int curr_implant_row_ = -1;  // 현재 선택된 임플란트 테이블의 row

	CW3View3DImplantPreview* preview_;

	QToolButton* delete_all_ = nullptr;
	QToolButton* close_ = nullptr;
	QToolButton* apply_ = nullptr;

	QLabel* thumbnail_ = nullptr;
	QRadioButton* left_ = nullptr;
	QRadioButton* right_ = nullptr;
	CurrentPreset current_preset_ = CurrentPreset::RIGHT;
};
