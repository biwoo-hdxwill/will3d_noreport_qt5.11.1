
#pragma once

/*=========================================================================

File:			class CW3TabMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2016-04-14

=========================================================================*/
#include <qwidget.h>
#include <memory>
// IPC 170120
#include <QLocalServer>
#include <QLocalSocket>

#include "../../Engine/Common/Common/w3_struct.h"
#include "../../Engine/Common/Common/W3Types.h"
#include "../../Engine/Common/Common/define_view.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"

#include "tabmgr_global.h"

class QImage;
class BaseTab;
class CW3FILEtab;
class CW3Image3D;
class CW3MPRtab;
class CW3PANOtab;
//20250210 LIN
//#ifndef WILL3D_VIEWER
class CW3IMPLANTtab;
//#endif
class CW3TMJtab;
#ifndef WILL3D_LIGHT
class CW3CEPHtab;
class CW3ENDOtab;
class CW3FACEtab;
class CW3SItab;
#endif
class CW3REPORTtab;
#ifndef WILL3D_LIGHT
class CW3VTOSTO;
#endif
class CW3JobMgr;
class CW3MPREngine;
class CW3VREngine;
class PanoEngine;
class TMJengine;
class CW3ResourceContainer;
class CommonTaskTool;
class MeasureResourceMgr;
class OTFTool;
//20250123 LIN
//#ifndef WILL3D_VIEWER
class ProjectIO;
class ProjectIOFile;
//#endif

#ifdef WILL3D_EUROPE
class ButtonListDialog;
#endif // WILL3D_EUROPE
class TABMGR_EXPORT CW3TabMgr : public QWidget
{
	Q_OBJECT
public:
	CW3TabMgr(CW3VREngine* VREngine, CW3ResourceContainer* Rcontainer,
		QWidget* parent = 0);
	~CW3TabMgr();

	void InitTabs(bool is_init_file_tab);
	void SetRenderQuality(bool is_high_quality);
	void InitOTFPreset(const QString& preset);

	void changeTab(const TabType& tab_type);

	inline bool isVolumeLoded() const { return m_bVolumeLoaded; }

	void setScriptFile(const QString& readFilePath, const QString& outScriptPath);

#ifndef WILL3D_LIGHT
	bool SetEnableOnlyTRDMode();
#endif
	inline CW3MPREngine* mpr_engine() { return m_pMPRengine; };

	void ApplyPreferences();

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void SetApplicationUIMode(const bool& is_maximize);

	bool CloseEvent();
#ifndef WILL3D_VIEWER
	void SaveProject();
	//20250122 LIN
	void SavePanoEngineContent();
#endif
	void CaptureScreenShot();
	void Print();

#ifdef WILL3D_VIEWER
	void ImportCDViewerData();
#endif

signals:
	void sigOTFAuto();
	void sigGetTabSlotGlobalRect(QRect& global_rect);
	void sigGetTabSlotRect(QRect& rect);
	void sigActivateMPRtab();
	void sigActivateSItab();
	void sigSyncBDViewStatus();

	void sigTempSyncCommonTaskEvent();
	void sigChangeTab(TabType);

	void sigSecondDisabled(bool, float*);
	void sigInitProgram();

	void sigSetDicomInfo();

	void sigActiveLoadProject();
	void sigDoneLoadProject();
	void sigOTFPreset(QString);
	void sigOTFAdjust(AdjustOTF&);
	void sigSaveTfPreset(QString);
	void sigOTFManualOnOff();
	// void sigReoriented(float *);
	// void sigSurfaceGened(std::vector<glm::vec3>*, std::vector<glm::vec3>*,
	// std::vector<glm::u32vec3>*); void sigMIPforAutoArch(float*, int, int,
	// float); void sigAutoPanoPoints(std::vector<Coord2D>*);

	void sigSetSoftTissueMin(const float value);
	void sigSendMPRPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth);

public slots:
	void slotChangeTab(TabType tabType);

#ifndef WILL3D_LIGHT
	void slotLoadSecondVolume();
	void slotLoadSecondDicomFromLoader();
#endif
	void slotLoadDicomFromLoader();
#ifndef WILL3D_VIEWER
	void slotLoadProject(QString path);
#endif
	void slotClientConnected();    // IPC
	void slotReadClientMessage();  // IPC

#ifndef WILL3D_LIGHT
	void slotSetTRDFromExternalProgram(const QString path, const bool onlyTRD);
#endif

	void slotGraphicsSceneMousePressEvent(QMouseEvent* event);
	void slotGraphicsSceneMouseReleaseEvent(QMouseEvent* event);

private slots:
#ifndef WILL3D_LIGHT
	void slotSetViewFaceAfter();
#endif

	// common menu bar占쏙옙 占쏙옙占쏙옙풔占?占쏙옙占쌜듸옙
#ifndef WILL3D_VIEWER
	void slotShowImplantAngle();
#endif
	void slotFileTool(const CommonToolTypeFile& type);
	void slotCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void slotCommonToolOnOff(const common::CommonToolTypeOnOff& type);
	void slotCommonToolCancelSelected();
	void slotCommonToolMeasureListOn();

	void slotOTFAdjustDone();

	void slotMeasureSelect(const common::ViewTypeID& view_type, const unsigned int& measure_id);
	void slotMeasureDelete(const common::ViewTypeID& view_type, const unsigned int& measure_id);
	void slotMeasureListCapture(QImage image);

#ifndef WILL3D_LIGHT
	void slotSave3DFaceToPLYFile();
	void slotSave3DFaceToOBJFile();
#endif
#ifndef WILL3D_VIEWER
	void slotLoadProjectFromFile();
#endif

	void slotContinuousCapture(QWidget* source);
	void slotContinuousCapture();

#ifdef WILL3D_EUROPE
	void slotShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
	void slotSyncControlButtonOut();
	void slotButtonToggle(const int id, const bool toggle);
#endif // WILL3D_EUROPE

private:
	void connections();
	void viewerTabConnections();
	void SetTabsInvisible();
	void setVolume(CW3Image3D* volume);
#ifndef WILL3D_LIGHT
	void setSecondVolume(CW3Image3D* volume);
#endif
#ifndef WILL3D_VIEWER
	void ImportProjectWhereChangedTab(const TabType& tab_type);
	void loadProject(const QString& path);
	bool LoadMainVolumeDataFromProject(ProjectIOFile* io_file);
#endif
	void loadProjectForPanoEngineContent(const QString& path);
#ifndef WILL3D_LIGHT
	void LoadSecondVolumeDataFromProject(ProjectIOFile* io_file);
#endif
	void startMessageReceiver();  // IPC

	void setMIP(bool);

	void SaveStatus(QImage thumbnail_image = QImage());
	//20250122 LIN
	void SavePanoEngineContentStatus();
#ifdef WILL3D_VIEWER
	void UpdateViewerW3DFile();
#endif

#ifndef WILL3D_VIEWER
	void CDExport();
#endif
	void SetOTFpresetAndButtonStatus(const QString& preset);
	QImage GetScreenshot(bool select_view, bool print_mode = false);
	QImage GetScreenshot(QWidget* source);
	QImage GetScreenshot(const QRect& target_rect);
	void DrawDicomInfoToImage(QImage* image);

	bool SaveScreenshot(const QImage& image);
	bool SaveImage(const QImage& image, QString& saved_path = QString());

#ifndef WILL3D_LIGHT
	QString GetFolderPathForExport3DFace();
	QString GetFileNameForExport3DFace();
#endif

	QSizeF GetOriginalScreenSize(const unsigned int screen_number = 0);

#ifndef WILL3D_VIEWER
	void ShowPACSDialog();
	void PACSSend(const QStringList& server_info);
#endif

#ifndef WILL3D_VIEWER
	void CopyDirectory(const QString &source_dir, const QString &target_dir);
#endif

private slots:
	void slotUpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number);
	void slotGetMPRPlaneInfo(const MPRViewType mpr_view_type);
	void slotCreateDCMFiles_uchar(unsigned char* data, const QString& middle_path, const int instance_num, const int rows, const int columns);
	void slotCreateDCMFiles_ushort(unsigned short* data, const QString& middle_path, const int instance_num, const int rows, const int columns);
	
private:
	/** @brief	global members */
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3VREngine* m_pgVREngine = nullptr;

	std::shared_ptr<CommonTaskTool> common_task_tool_;
	std::shared_ptr<OTFTool> otf_task_tool_;
	std::unique_ptr<MeasureResourceMgr> measure_res_mgr_;

	CW3JobMgr* m_pJobMgr = nullptr;

	CW3MPREngine* m_pMPRengine = nullptr;

	/** @brief	Tabs */
	TabType tab_type_ = TabType::TAB_UNKNOWN;
	CW3FILEtab* m_pFILEtab = nullptr;
	CW3MPRtab* m_pMPRtab = nullptr;
	CW3PANOtab* m_pPANOtab = nullptr;
//20250210 LIN
//#ifndef WILL3D_VIEWER
	CW3IMPLANTtab* m_pIMPLANTtab = nullptr;
//#endif
	CW3TMJtab* m_pTMJtab = nullptr;
#ifndef WILL3D_LIGHT
	CW3CEPHtab* m_pCEPHtab = nullptr;
	CW3FACEtab* m_pFACEtab = nullptr;
	CW3SItab* m_pSItab = nullptr;
	CW3ENDOtab* m_pENDOtab = nullptr;
#endif
	CW3REPORTtab* m_pREPORTtab = nullptr;

#ifndef WILL3D_LIGHT
	CW3VTOSTO* m_pVTOSTO;
#endif

	QString m_strOutputPath = ".";

	bool m_bIsSIActivated = false;
	bool m_bSaveForExternalProgram = false;
	bool m_bVolumeLoaded = false;

	QLocalServer* m_pServer = nullptr;            // IPC 170120
	QLocalSocket* m_pClientConnection = nullptr;  // IPC 170120

	std::shared_ptr<PanoEngine> pano_engine_;
	std::shared_ptr<TMJengine> tmj_engine_;
	std::vector<BaseTab*> casted_tabs_;

//20250123 LIN 二쇱꽍泥섎━
//#ifndef WILL3D_VIEWER
	std::unique_ptr<ProjectIO> project_in_;
//#endif

	common::CommonToolTypeOnOff last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	bool is_pressed_l_ = false;
	bool is_pressed_r_ = false;
	bool is_pressed_lr_ = false;

	QString project_file_path_;

	QTimer* continuous_capture_timer_ = nullptr;
	QWidget* continuous_capture_source_ = nullptr;

#ifdef WILL3D_EUROPE
	ButtonListDialog* button_list_dialog_ = nullptr;
#endif // WILL3D_EUROPE
};
