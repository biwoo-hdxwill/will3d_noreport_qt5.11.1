#pragma once
/*=========================================================================
File:			pacs_dialog.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-06-01
Last modify:	2021-08-20

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <GL/glm/vec3.hpp>

#include "../../Common/Common/W3Dialog.h"
#include "../../Common/Common/w3_struct.h"

#include "uiframe_global.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

class CW3MPREngine;
class PacsCommonWidget;
class PacsMPRViewWidget;
class PacsMPRSettingWidget;
class Pacs3DViewWidget;
class Pacs3DSettingWidget;
class PacsPanoViewWidget;
class PacsPanoSettingWidget;
class PacsLightboxSettingWidget;
class UIFRAME_EXPORT PacsDialog : public CW3Dialog
{
	Q_OBJECT
public:
	PacsDialog(TabType tab_type, const QStringList& view_list, QWidget* parent = 0);
	virtual ~PacsDialog();

	PacsDialog(const PacsDialog&) = delete;
	const PacsDialog& operator=(const PacsDialog&) = delete;

	void EmitMPRViewInitialize(CW3MPREngine* mpr_engine);
	void EmitPanoViewInitialize();
	void SetLightBoxViewInfo(const int filter, const int thickness);
	void PanoUpdate();

signals:
	void sigChangeMPRType(const MPRViewType mpr_type);
	void sigPanoUpdate(int value);
	void sigPanoUpdateThickness(int thickness);
	void sigNerveVisibility(bool is_visible);
	void sigImplantVisibility(bool is_visible);

	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& full_path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& full_path, const int instance_number, const int rows, const int columns);
	void sigRequestCreateDCMFiles(bool nerve_visible, bool implant_visible, int filter, int thickness);
	void sigRequestGetPanoCrossSectionViewInfo(int& filter_level, int& thickness);
	void sigPACSSend(const QStringList& server_info);

public slots:
	void slotSetMPRPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth);

private slots:
	void slotChangeViewType(const int index);
	void slotSendButtonClick();

private:
	void CreateWidget();
	void MPRSetLayout();
	void MPRConnections();
	void PanoSetLayout();
	void PanoConnections();

	QHBoxLayout* CreateMPRContentsLayout();
	QVBoxLayout* CreateMPRContentsLeftLayout();
	QVBoxLayout* CreateMPRContentsRightLayout();

	QHBoxLayout* CreatePanoContentsLayout();
	QVBoxLayout* CreatePanoContentsRightLayout();

	QHBoxLayout* CreateButtonLayout();

private:
	TabType tab_type_ = TabType::TAB_UNKNOWN;
	QStringList view_list_;
	bool initial_setting_ = false;

	MPRViewType mpr_view_type_ = MPRViewType::MPR_END;
	bool is_mpr_lightbox_ = false;

	//MPR Tab
	PacsCommonWidget* common_widget_ = nullptr;
	PacsMPRViewWidget* mpr_view_widget_ = nullptr;
	PacsMPRSettingWidget* mpr_setting_widget_ = nullptr;
	PacsLightboxSettingWidget* lightbox_setting_widget_ = nullptr;
	Pacs3DViewWidget* vr_view_widget_ = nullptr;
	Pacs3DSettingWidget* vr_setting_widget_ = nullptr;

	//Pano Tab
	PacsPanoViewWidget* pano_view_widget_ = nullptr;
	PacsPanoSettingWidget* pano_setting_widget_ = nullptr;
};
