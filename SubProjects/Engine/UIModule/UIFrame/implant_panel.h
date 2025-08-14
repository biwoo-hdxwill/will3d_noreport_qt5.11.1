#pragma once
/*=========================================================================

File:			class CW3AddImplant
Language:		C++11
Library:		Qt 5.2.0
Author:			Jung Dae Gun
First date:		2016-05-09
Last modify:	2016-05-09

=========================================================================*/
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <QFrame>
#include "../../Common/Common/W3Enum.h"
#include "../UIPrimitive/W3ImplantButton.h"

#include "uiframe_global.h"

class QToolButton;
class QComboBox;
class QRadioButton;
class QSqlDatabase;
class QSpacerItem;
class QHBoxLayout;
class QFrame;

class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;
class ImplantButton;

namespace implant_resource
{
	typedef struct _ImplantInfo ImplantInfo;
}

class UIFRAME_EXPORT ImplantPanel : public QFrame
{
	Q_OBJECT
public:
	ImplantPanel(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
		CW3ResourceContainer* Rcontainer, QWidget* pParent = 0);

	virtual ~ImplantPanel();

public:
	void SetApplicationUIMode(const bool& is_maximize);
	void importProject();
	void reset();

	QString GetCurrentImplantFilePath() const;
	QString GetCurrentManufacturer() const;
	QString GetCurrentProduct() const;
	QString GetCurrentSubCategory();
	float GetCurrentDiameter() const;
	float GetCurrentLength();
	void SetImplantButtonStatusSelected(int implant_id);
	void SetImplantButtonStatusDefault(int implant_id);
	void SetImplantSpecAsPrevModel(int implant_id);

	void SyncArchType(const ArchTypeID& arch_type);

signals:
	void sigAddImplant(const implant_resource::ImplantInfo& implant_params);
	void sigChangeImplant(const implant_resource::ImplantInfo& implant_params);
	void sigSelectImplant(int id, bool selected);
	void sigDeleteImplant(int id);
	void sigDeleteAllImplants();

	void sigArchSelectionChanged(const ArchTypeID& arch_type);

	void sigCancelAddImplant();

public slots:
	void slotImplantDBSetting();
	void slotDeleteAllImplants();
	void slotImplantComboChanged_Manufacturer(const QString& manufacturer_namer);
	void slotImplantComboChanged_Product(const QString& strProduct);
	void slotImplantComboChanged_Diameter(const QString& strDiameter);
	void slotImplantComboChanged_Length(const QString& strLength);

private slots:
	void slotShowImplantListDialog();
	void slotShowPreferenceDialog();

	void slotAddImplantFromButton(int index);
	void slotSelectImplantFromButton(int, ImplantButton::ButtonStatus);
	void slotDeleteImplantFromButton(int);

	void slotImplantSelected(int);

	void slotFavoriteUpdated();

private:
	void updateManufacturer();
	void updateProduct();
	void UpdateDiameter();
	void UpdateLength();
	void UpdateImplantInfoAndChangeModel();

	void UpdateImplantInfo();
	void ChangeSelectedImplantModel();

	void GetImplantSpecs(
		const QString& product_name,
		const QString& implant_name, 
		QString& diameter,
		QString& length,
		QString& platform_diameter,
		QString& total_length,
		QString& sub_category
	);
	void GetImplantName(const QString& product_name, const QString& diameter,
		const QString& length, const QString& sub_category, QString& implant_name);
	QString GetCustomImplantName(
		const QString& product_name,
		const QString& coronal_diameter,
		const QString& apical_diameter,
		const QString& length
	);

	int QueryManufacturerID(const QString& manufacturer_name, const bool is_custom_implant, QSqlDatabase& db);
	int QueryProductID(const int manufacturer_id, const QString& product_name, const bool is_custom_implant, QSqlDatabase& db);

	void connections();
	void connectDB();
	void disconnectDB();
	void BlockComboboxSignals(const bool& block);

	bool existDBRecord(const QString& manufacturer_name, const QString& strProduct, const bool is_custom_implant);
	bool existDBRecord(const QString& manufacturer_name, const bool is_custom_implant);

	void LoadPreset(int implant_index);
	void LoadDisplayedPanelsImplantSpec();
	void SaveDisplayedPanelsImplantSpec();

	void SyncSelectedImplantButton(int index);
	void SetImplantSpecWithButtonIndex(int index);

	bool IsSelectedImplantModelChanged(const QString& implant_path, int& selected_id);

	QString MakeLengthPlusSubCategoryFormat(const QString& length, const QString& sub_category);
	QString GetLengthFromComboBoxItem(const QString& item);
	QString GetSubCategoryFromComboBoxItem(const QString& item);

private:
	CW3VREngine* m_pgVREngine;
	CW3MPREngine* m_pgMPREngine;
	CW3ResourceContainer* m_pgRcontainer;

	ImplantButton implant_buttons_[28];
	QComboBox* manufacturer_;
	QComboBox* product_;
	QComboBox* diameter_;
	QComboBox* length_;
	QString implant_file_path_ = "";

	QRadioButton* arch_maxilla_ = nullptr;
	QRadioButton* arch_mandible_ = nullptr;

	QToolButton* implant_library_;
	QToolButton* clear_all_;
	QToolButton* preference_;
	QToolButton* implant_list_;

	// application resize 시에 형태를 조절하기 위한 ui items
	QSpacerItem* button_spacer_ = nullptr;
	QFrame* black_line_ = nullptr;
	QFrame* white_line_ = nullptr;
	QHBoxLayout* implant_select_layout_ = nullptr;

	float platform_diameter_ = 0.0f;
	float total_length_ = 0.0f;
	float custom_apical_diameter_ = 0.0f;
};
