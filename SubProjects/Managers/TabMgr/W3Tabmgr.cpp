#include "W3Tabmgr.h"

/*=========================================================================

File:			class CW3TabMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2016-04-14

=========================================================================*/
#include <windows.h>
#include <VersionHelpers.h>
#include <ctime>

#include <QDesktopWidget>
#include <QApplication>
#include <QDataStream>
#include <QFile>
#include <QImageWriter>
#include <QKeyEvent>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QScreen>
#include <QTime>
#include <QWindow>
#include <QtConcurrent/QtConcurrent>
#include <QToolButton>
#include <QDebug>
#include <QElapsedTimer>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/W3ProgressDialog.h>
#include <Engine/Common/Common/define_otf.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/language_pack.h>

#include <Engine/Resource/ResContainer/W3ResourceContainer.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/W3ImageHeader.h>
#include <Engine/Resource/Resource/pano_resource.h>
//20250123 LIN #ifndef WILL3D_VIEWER 諛뽰쑝濡????
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/datatypes.h>
//#include <Engine/Core/W3ProjectIO/project_io.h>
#ifndef WILL3D_LIGHT
#include <Engine/Core/W3ProjectIO/project_io_ceph.h>
#include <Engine/Core/W3ProjectIO/project_io_endo.h>
#include <Engine/Core/W3ProjectIO/project_io_face.h>
#include <Engine/Core/W3ProjectIO/project_io_si.h>
#endif
#include <Engine/Core/W3ProjectIO/project_io_flie.h>
#include <Engine/Core/W3ProjectIO/project_io_general.h>
//#include <Engine/Core/W3ProjectIO/project_io_implant.h>
#include <Engine/Core/W3ProjectIO/project_io_measure_resource.h>
//#include <Engine/Core/W3ProjectIO/project_io_mpr_engine.h>
//#include <Engine/Core/W3ProjectIO/project_io_mpr.h>
//#include <Engine/Core/W3ProjectIO/project_io_pano_engine.h>
#include <Engine/Core/W3ProjectIO/project_io_panorama.h>
#include <Engine/Core/W3ProjectIO/project_io_tmj.h>
#ifndef WILL3D_LIGHT
#include <Engine/Core/W3ProjectIO/project_io_vtosto.h>
#endif
#include <Engine/Core/W3ProjectIO/project_path_info.h>
#endif
#include <Engine/Core/W3ProjectIO/project_io_implant.h>
#include <Engine/Core/W3ProjectIO/project_io.h>
#include <Engine/Core/W3ProjectIO/project_io_mpr.h>
#include <Engine/Core/W3ProjectIO/project_io_mpr_engine.h>
#include <Engine/Core/W3ProjectIO/project_io_pano_engine.h>
//=======

#include <Engine/Core/W3DicomIO/dicom_send.h>

#include <Engine/UIModule/UITools/common_task_tool.h>
#include <Engine/UIModule/UITools/otf_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>
#ifndef WILL3D_LIGHT
#include <Engine/UIModule/UIComponent/W3VTOSTO.h>
#endif
#include <Engine/UIModule/UIFrame/W3OTFPresetDlg.h>
#include <Engine/UIModule/UIFrame/capture_dialog.h>
#ifndef WILL3D_VIEWER
#include <Engine/UIModule/UIFrame/pacs_dialog.h>
#include <Engine/UIModule/UIFrame/cd_export_dialog.h>
#include <Engine/UIModule/UIFrame/implant_angle_dialog.h>
#endif
#include <Engine/UIModule/UIFrame/measure_list_dialog.h>
#ifdef WILL3D_EUROPE
#include <Engine/UIModule/UIFrame/button_list_dialog.h>
#endif // WILL3D_EUROPE

#include <Engine/Module/MPREngine/W3MPREngine.h>
#include <Engine/Module/Panorama/pano_engine.h>
#include <Engine/Module/TMJ/tmj_engine.h>
#include <Engine/Module/VREngine/W3VREngine.h>
#include <Engine/Module/Will3DEngine/renderer_manager.h>

#include <Managers/JobMgr/W3Jobmgr.h>
#include "W3FILEtab.h"
#include "W3MPRtab.h"
#include "W3PANOtab.h"
//20250210 LIN
//#ifndef WILL3D_VIEWER
#include "W3IMPLANTtab.h"
//#endif
#include "W3TMJtab.h"
#ifndef WILL3D_LIGHT
#include "W3CEPHtab.h"
#include "W3ENDOtab.h"
#include "W3FACEtab.h"
#include "W3SItab.h"
#endif
#include "W3REPORTtab.h"
#include "measure_resource_mgr.h"

namespace
{
	const QString kExport3DFaceFolderPath("./3d_face/");
	const QString kFolderPath = "/temp/";
}

CW3TabMgr::CW3TabMgr(CW3VREngine* VREngine, CW3ResourceContainer* Rcontainer,
	QWidget* parent)
	: QWidget(parent),
	m_pgVREngine(VREngine),
	m_pgRcontainer(Rcontainer),
	measure_res_mgr_(new MeasureResourceMgr(this)),
	common_task_tool_(new CommonTaskTool(this)),
	otf_task_tool_(new OTFTool(this))
{

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool singleton_process = settings.value("PROCESS/singleton", true).toBool();

	if (singleton_process)
	{
		startMessageReceiver(); // IPC
	}
	ToolMgr::instance()->SetCommonTaskTool(common_task_tool_);
	ToolMgr::instance()->SetOTFTaskTool(otf_task_tool_);

	m_pJobMgr = new CW3JobMgr(m_pgVREngine, m_pgRcontainer);
#ifndef WILL3D_LIGHT
	m_pVTOSTO = new CW3VTOSTO(m_pgVREngine, m_pJobMgr, m_pgRcontainer);
#endif
	m_pMPRengine = new CW3MPREngine();

	this->InitTabs(true);

	connections();

#ifndef WILL3D_VIEWER
	changeTab(TabType::TAB_FILE);
#else
	changeTab(TabType::TAB_MPR);
#endif

	// setScriptFile(readFilePath, outScriptPath);
}

CW3TabMgr::~CW3TabMgr()
{
	SAFE_DELETE_OBJECT(m_pServer);  // IPC 170120

	for (auto& tab : casted_tabs_)
	{
		SAFE_DELETE_OBJECT(tab);
	}

	SAFE_DELETE_OBJECT(m_pMPRengine);
	SAFE_DELETE_OBJECT(m_pJobMgr);

#ifdef WILL3D_EUROPE
	if (button_list_dialog_)
	{
		button_list_dialog_->deleteLater();
	}
#endif // WILL3D_EUROPE
}

void CW3TabMgr::InitTabs(bool is_init_file_tab)
{
	casted_tabs_.clear();
	casted_tabs_.resize(TabType::TAB_END, nullptr);

	if (is_init_file_tab) m_pFILEtab = new CW3FILEtab(m_pgRcontainer, this);

#ifndef WILL3D_VIEWER
	casted_tabs_[TAB_FILE] = dynamic_cast<BaseTab*>(m_pFILEtab);
#endif

	m_pMPRtab = new CW3MPRtab(m_pgVREngine, m_pMPRengine, m_pJobMgr, m_pgRcontainer);
	casted_tabs_[TAB_MPR] = dynamic_cast<BaseTab*>(m_pMPRtab);

	m_pPANOtab = new CW3PANOtab();
	casted_tabs_[TAB_PANORAMA] = dynamic_cast<BaseTab*>(m_pPANOtab);

//20250210 LIN
//#ifndef WILL3D_VIEWER
	m_pIMPLANTtab = new CW3IMPLANTtab(m_pgVREngine, m_pMPRengine, m_pgRcontainer);
	casted_tabs_[TAB_IMPLANT] = dynamic_cast<BaseTab*>(m_pIMPLANTtab);
//#endif

	m_pTMJtab = new CW3TMJtab();
	casted_tabs_[TAB_TMJ] = dynamic_cast<BaseTab*>(m_pTMJtab);

#ifndef WILL3D_LIGHT
	m_pSItab = new CW3SItab(m_pgVREngine, m_pMPRengine, m_pgRcontainer);
	casted_tabs_[TAB_SI] = dynamic_cast<BaseTab*>(m_pSItab);

	m_pENDOtab =
		new CW3ENDOtab(m_pgVREngine, m_pMPRengine, m_pJobMgr, m_pgRcontainer);
	casted_tabs_[TAB_ENDO] = dynamic_cast<BaseTab*>(m_pENDOtab);

	m_pCEPHtab =
		new CW3CEPHtab(m_pgVREngine, m_pMPRengine, m_pVTOSTO, m_pgRcontainer);
	casted_tabs_[TAB_3DCEPH] = dynamic_cast<BaseTab*>(m_pCEPHtab);

	m_pFACEtab =
		new CW3FACEtab(m_pgVREngine, m_pMPRengine, m_pVTOSTO, m_pgRcontainer);
	casted_tabs_[TAB_FACESIM] = dynamic_cast<BaseTab*>(m_pFACEtab);
#endif

	m_pREPORTtab = new CW3REPORTtab();
	casted_tabs_[TAB_REPORT] = dynamic_cast<BaseTab*>(m_pREPORTtab);

	pano_engine_.reset(new PanoEngine);
	m_pMPRtab->SetPanoEngine(pano_engine_);
	m_pPANOtab->SetPanoEngine(pano_engine_);

//20250210 LIN
//#ifndef WILL3D_VIEWER
	m_pIMPLANTtab->SetPanoEngine(pano_engine_);
//#endif

	tmj_engine_.reset(new TMJengine);
	m_pTMJtab->SetTMjengine(tmj_engine_);
}

void CW3TabMgr::SetRenderQuality(bool is_high_quality)
{
	if (casted_tabs_.size() > tab_type_)
		casted_tabs_[tab_type_]->SetRenderModeQuality(is_high_quality);

	if (tab_type_ == TAB_MPR)
		m_pMPRtab->UpdateVRview(is_high_quality);
#ifndef WILL3D_LIGHT
	else if (tab_type_ == TAB_3DCEPH)
		m_pCEPHtab->UpdateVRview(is_high_quality);
	else if (tab_type_ == TAB_FACESIM)
		m_pFACEtab->UpdateVRview(is_high_quality);
	else if (tab_type_ == TAB_SI)
		m_pSItab->UpdateVRview(is_high_quality);
	else if (tab_type_ == TAB_ENDO)
		m_pENDOtab->UpdateVRview(is_high_quality);
#endif
}

void CW3TabMgr::InitOTFPreset(const QString& preset)
{
	otf_task_tool_->InitOTFPreset(preset);
}

void CW3TabMgr::connections()
{
	connect(m_pFILEtab, &CW3FILEtab::sigLoadDicomFromLoader, this, &CW3TabMgr::slotLoadDicomFromLoader);
#ifndef WILL3D_LIGHT
	connect(m_pFILEtab, &CW3FILEtab::sigLoadSecondDicomFromLoader, this, &CW3TabMgr::slotLoadSecondDicomFromLoader);
	connect(m_pFILEtab, &CW3FILEtab::sigSetTRDFromExternalProgram, this, &CW3TabMgr::slotSetTRDFromExternalProgram);
#endif

#ifndef WILL3D_VIEWER
	connect(m_pFILEtab, &CW3FILEtab::sigLoadProject, this, &CW3TabMgr::slotLoadProject);
	connect(common_task_tool_.get(), &CommonTaskTool::sigShowImplantAngle, this, &CW3TabMgr::slotShowImplantAngle);
#endif
	connect(common_task_tool_.get(), &CommonTaskTool::sigFileTool, this, &CW3TabMgr::slotFileTool);
	connect(common_task_tool_.get(), &CommonTaskTool::sigCommonMeasureListOn, this, &CW3TabMgr::slotCommonToolMeasureListOn);
	connect(otf_task_tool_.get(), &OTFTool::sigOTFAuto, this, &CW3TabMgr::sigOTFAuto);
	connect(otf_task_tool_.get(), &OTFTool::sigOTFPreset, this, &CW3TabMgr::sigOTFPreset);
	connect(otf_task_tool_.get(), &OTFTool::sigOTFAdjust, this, &CW3TabMgr::sigOTFAdjust);
	connect(otf_task_tool_.get(), &OTFTool::sigOTFAdjustDone, this, &CW3TabMgr::slotOTFAdjustDone);
	connect(otf_task_tool_.get(), &OTFTool::sigOTFManualOnOff, this, &CW3TabMgr::sigOTFManualOnOff);
}

void CW3TabMgr::viewerTabConnections()
{
	connect(m_pMPRtab, &CW3MPRtab::sigMPRLightboxOn, [=]() { emit sigChangeTab(tab_type_); });
	connect(m_pMPRtab, &CW3MPRtab::sigSendMPRPlaneInfo, this, &CW3TabMgr::sigSendMPRPlaneInfo);

//20250210 LIN
//#ifndef WILL3D_VIEWER
	connect(m_pIMPLANTtab, &CW3IMPLANTtab::sigSyncImplantAngle, common_task_tool_.get(), &CommonTaskTool::slotSyncImplantAngleButton);
	connect(m_pIMPLANTtab, &CW3IMPLANTtab::sigSyncBDViewStatus, this, &CW3TabMgr::sigSyncBDViewStatus);
//#endif

	connect(m_pTMJtab, SIGNAL(sigContinuousCapture(QWidget*)), this, SLOT(slotContinuousCapture(QWidget*)));

#ifndef WILL3D_LIGHT
	connect(m_pCEPHtab, &CW3CEPHtab::sigChangeTab, this, &CW3TabMgr::slotChangeTab);

	connect(m_pFACEtab, &CW3FACEtab::sigChangeTab, this, &CW3TabMgr::slotChangeTab);
	connect(m_pFACEtab, &CW3FACEtab::sigCephNoEventMode, m_pCEPHtab, &CW3CEPHtab::slotEnableCephNoEventMode);
	connect(m_pFACEtab, &CW3FACEtab::sigRequestViewFaceAfter, this, &CW3TabMgr::slotSetViewFaceAfter);
	connect(m_pFACEtab, &CW3FACEtab::sigSetSoftTissueMin, this, &CW3TabMgr::sigSetSoftTissueMin);

	connect(m_pSItab, &CW3SItab::sigLoadSecondVolume, this, &CW3TabMgr::slotLoadSecondVolume);
	connect(m_pSItab, &CW3SItab::sigSetTranslateMatSecondVolume, m_pMPRtab, &CW3MPRtab::slotSetTranslateMatSecondVolume);
	connect(m_pSItab, &CW3SItab::sigSetRotateMatSecondVolume, m_pMPRtab, &CW3MPRtab::slotSetRotateMatSecondVolume);

	connect(m_pENDOtab, &CW3ENDOtab::sigSetAirway, m_pMPRtab, &CW3MPRtab::slotSetAirway);
	connect(m_pENDOtab, &CW3ENDOtab::sigAirwayEnable, m_pMPRtab, &CW3MPRtab::slotAirwayEnable);
	connect(m_pENDOtab, &CW3ENDOtab::sigSetAirwaySize, m_pMPRtab, &CW3MPRtab::slotSetAirwaySize);

	connect(m_pVTOSTO, &CW3VTOSTO::sigPointModelLoadProject, m_pMPRtab, &CW3MPRtab::slotPointModel);
	connect(m_pVTOSTO, &CW3VTOSTO::sigUpdateMPRPhotoLoadProject, m_pMPRtab, &CW3MPRtab::slotFaceEnabled);

	connect(m_pFACEtab, SIGNAL(sigSave3DFaceToPLYFile()), this, SLOT(slotSave3DFaceToPLYFile()));
	connect(m_pFACEtab, SIGNAL(sigSave3DFaceToOBJFile()), this, SLOT(slotSave3DFaceToOBJFile()));
#endif
	connect(m_pJobMgr, &CW3JobMgr::sigSecondDisabledFromSuperTab, m_pMPRtab, &CW3MPRtab::slotSecondDisabled);
	connect(m_pJobMgr, &CW3JobMgr::sigPointModelFromFaceTab, m_pMPRtab, &CW3MPRtab::slotPointModel);
	connect(m_pJobMgr, &CW3JobMgr::sigReorientedFromMPRtab, m_pgVREngine, &CW3VREngine::slotReoriented);
	connect(m_pJobMgr, &CW3JobMgr::sigReorientedFromMPRtab, m_pMPRengine, &CW3MPREngine::slotReoriented);
	connect(m_pJobMgr, &CW3JobMgr::sigUpdateMPRPhotoFromFaceTab, m_pMPRtab, &CW3MPRtab::slotFaceEnabled);

	// connect(m_pFILEtab, SIGNAL(sigSetTRDFromExternalProgram(const QString, const bool)),
	// m_pFACEtab, SLOT(slotSetTRDFromExternalProgram(const QString, const bool)));
	connect(common_task_tool_.get(), &CommonTaskTool::sigCommonToolOnce, this, &CW3TabMgr::slotCommonToolOnce);
	connect(common_task_tool_.get(), &CommonTaskTool::sigCommonToolOnOff, this, &CW3TabMgr::slotCommonToolOnOff);
	connect(this, &CW3TabMgr::sigTempSyncCommonTaskEvent, common_task_tool_.get(), &CommonTaskTool::slotTempSyncMenuEvent, Qt::QueuedConnection);

	for (auto& tab : casted_tabs_)
	{
		connect(tab, &BaseTab::sigCommonToolCancelSelected, this, &CW3TabMgr::slotCommonToolCancelSelected);
	}

	connect(m_pMPRtab, SIGNAL(sigSave3DFaceToPLYFile()), this, SLOT(slotSave3DFaceToPLYFile()));
	connect(m_pMPRtab, SIGNAL(sigSave3DFaceToOBJFile()), this, SLOT(slotSave3DFaceToOBJFile()));

	connect(m_pMPRtab,
		SIGNAL(sigUpdateArch(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)),
		this,
		SLOT(slotUpdateArchFromMPR(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)));

//20250210 LIN
//#ifndef WILL3D_VIEWER
	connect(m_pPANOtab, &CW3PANOtab::sigSyncCrossSectionParams, m_pIMPLANTtab, &CW3IMPLANTtab::slotSyncCrossSectionParams);
	connect(m_pIMPLANTtab, &CW3IMPLANTtab::sigSyncCrossSectionParams, m_pPANOtab, &CW3PANOtab::slotSyncCrossSectionParams);
//#endif

#ifdef WILL3D_EUROPE
	connect(m_pMPRtab, &CW3MPRtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);	
	connect(m_pPANOtab, &CW3PANOtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pIMPLANTtab, &CW3IMPLANTtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pTMJtab, &CW3TMJtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pCEPHtab, &CW3CEPHtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pFACEtab, &CW3FACEtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pSItab, &CW3SItab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
	connect(m_pENDOtab, &CW3ENDOtab::sigShowButtonListDialog, this, &CW3TabMgr::slotShowButtonListDialog);
#endif // WILL3D_EUROPE

}

void CW3TabMgr::setScriptFile(const QString& readFilePath,
	const QString& outScriptPath)
{
	if (readFilePath.isEmpty())
	{
		return;
	}

#ifndef WILL3D_VIEWER
	emit sigChangeTab(TabType::TAB_FILE);
#endif

	if (outScriptPath.isEmpty())
	{
		if (readFilePath.contains(".w3d", Qt::CaseInsensitive))
		{
			project_file_path_ = readFilePath;

			QTimer* load_project_timer = new QTimer(this);
			load_project_timer->setSingleShot(true);
#ifndef WILL3D_VIEWER
			connect(load_project_timer, SIGNAL(timeout()), this, SLOT(slotLoadProjectFromFile()));
#endif
			load_project_timer->start(1000);
		}
		return;
	}

	m_pFILEtab->readInputFile(readFilePath);  // call from willmaster

	QByteArray baOut = outScriptPath.toLocal8Bit();
	std::string stdOutScriptPath = baOut.constData();

	common::Logger::instance()->Print(common::LogType::INF, std::string("outScriptPath Path : ") + stdOutScriptPath);

	QFile script(outScriptPath);
	if (!script.open(QIODevice::ReadOnly)) return;

	QTextStream in(&script);
	while (!in.atEnd())
	{
		QString line = in.readLine();

		if (line.length() < 1) continue;

		if (line.left(1).compare("#") == 0) continue;

		QStringList listLine = line.split("'");

		if (listLine.size() < 3) continue;

		if (listLine.at(0).trimmed().compare("FOLDER", Qt::CaseInsensitive) == 0)
		{
			if (listLine.at(2).trimmed().length() > 0)
			{
				m_strOutputPath = listLine.at(2).trimmed();
				m_bSaveForExternalProgram = true;
				QByteArray ba = m_strOutputPath.toLocal8Bit();
				std::string stdStr = ba.constData();

				common::Logger::instance()->Print(common::LogType::INF,
					std::string("Output Path : ") + stdStr);

#if 1
#if defined(_WIN32)
				// send message to willmaster
				UINT regMsg = RegisterWindowMessage(L"Will3DOpen");
				HWND hwnd = FindWindow(NULL, L"Will-Master");
				if (hwnd)
				{
					PostMessage(hwnd, regMsg, NULL, NULL);
					common::Logger::instance()->Print(
						common::LogType::INF,
						"CW3TabMgr::setScriptFile : open message send");
				}
				else
				{
					common::Logger::instance()->Print(
						common::LogType::ERR,
						"CW3TabMgr::setScriptFile : hwnd is nullptr");
				}
#endif
#endif
			}

			break;
		}
	}

	script.close();

	common::Logger::instance()->Print(common::LogType::INF, std::string("m_bSaveForExternalProgram : ") + QString::number(m_bSaveForExternalProgram).toStdString());

	//emit sigInitProgram();
}

void CW3TabMgr::startMessageReceiver()
{  // IPC
	m_pServer = new QLocalServer(this);
	connect(m_pServer, SIGNAL(newConnection()), this, SLOT(slotClientConnected()));

	if (!m_pServer->listen("Will3D"))
	{
		common::Logger::instance()->Print(common::LogType::ERR, std::string("Unable to start the server : ") + m_pServer->errorString().toStdString());
		SAFE_DELETE_OBJECT(m_pServer);
	}
	else
	{
		common::Logger::instance()->Print(common::LogType::INF, std::string("server generated"));
	}
}

void CW3TabMgr::slotClientConnected()
{  // IPC
	common::Logger::instance()->Print(common::LogType::INF, std::string("server connected"));
	m_pClientConnection = m_pServer->nextPendingConnection();
	connect(m_pClientConnection, SIGNAL(readyRead()), this, SLOT(slotReadClientMessage()));
}

void CW3TabMgr::slotReadClientMessage()
{  // IPC
	unsigned short blockSize = 0;
	QDataStream in(m_pClientConnection);
	in.setVersion(QDataStream::Qt_5_8);

	if (blockSize == 0)
	{
		if (m_pClientConnection->bytesAvailable() < (int)sizeof(quint16)) return;
		in >> blockSize;
	}

	common::Logger::instance()->Print(common::LogType::INF, std::string("block size : ") + QString::number(blockSize).toStdString());

	if (in.atEnd()) return;

	QString readFilePath, outScriptPath, moduleName;
	in >> readFilePath >> outScriptPath >> moduleName;

	common::Logger::instance()->Print(common::LogType::INF, std::string("readFilePath : ") + readFilePath.toStdString());
	common::Logger::instance()->Print(common::LogType::INF, std::string("outScriptPath : ") + outScriptPath.toStdString());

	this->parentWidget()->activateWindow();

	setScriptFile(readFilePath, outScriptPath);

	m_pClientConnection->disconnectFromServer();

	emit sigInitProgram();
}

void CW3TabMgr::slotOTFAdjustDone()
{
	emit m_pgVREngine->sigTFupdateCompleted();
}

void CW3TabMgr::slotMeasureSelect(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	TabType measure_inserted_tab = measure_res_mgr_->GetTabType(view_type);
	if (tab_type_ != measure_inserted_tab)
		emit sigChangeTab(measure_inserted_tab);
	casted_tabs_[measure_inserted_tab]->MoveViewsToSelectedMeasure(view_type,
		measure_id);
}

void CW3TabMgr::slotMeasureDelete(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	// UI 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙퓸占쏙옙 占쏙옙.
	casted_tabs_[tab_type_]->DeleteMeasureUI(view_type, measure_id);
	measure_res_mgr_->slotMeasureDelete(view_type, measure_id);
}

void CW3TabMgr::slotMeasureListCapture(QImage image)
{
	SaveScreenshot(image);
}

void CW3TabMgr::SetOTFpresetAndButtonStatus(const QString& preset)
{
	if (otf_task_tool_->SetOTFButtonStatus(preset))
	{
		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
			preset);
	}
}

void CW3TabMgr::setMIP(bool bOn)
{
	m_pMPRtab->setMIP(bOn);
	m_pPANOtab->setMIP(bOn);
#ifndef WILL3D_LIGHT
	m_pCEPHtab->setMIP(bOn);
#endif
}

void CW3TabMgr::slotChangeTab(TabType tabType)
{
	emit sigChangeTab(tabType);
}


#ifndef WILL3D_LIGHT
void CW3TabMgr::slotLoadSecondDicomFromLoader()
{
	CW3ProgressDialog* progress =
		CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	if (!m_pFILEtab->isProject())
	{
		if (!m_pFILEtab->loadDicom()) return;
	}

	progress->setFormat(CW3ProgressDialog::WAITTING);

	CW3Image3D* volume = m_pFILEtab->getSecondVolume();
	if (volume)
	{
		m_pgVREngine->deleteVRparams(1);

		QFuture<void> ftGetVolume =
			QtConcurrent::run(this, &CW3TabMgr::setSecondVolume, volume);
		watcher.setFuture(ftGetVolume);
		progress->exec();
		watcher.waitForFinished();

		EventHandler::GetInstance()
			->GetCommonEventHandle()
			.EmitSigSetSecondVolume();

		glm::mat4 secondToFirst;
		if (m_pFILEtab->isProject())
		{
			emit sigChangeTab(TabType::TAB_SI);
			m_pSItab->activate();
			m_bIsSIActivated = true;

			emit sigChangeTab(TabType::TAB_MPR);
			m_pMPRtab->slotSecondDisabled(false, nullptr);

			emit sigChangeTab(TabType::TAB_SI);
			secondToFirst = m_pSItab->getSecondToFirst();
			m_pFILEtab->getSecondVolume()->setSecondToFirst(secondToFirst);
			m_pSItab->setSecVolume(secondToFirst);
			emit sigChangeTab(TabType::TAB_MPR);
		}
		else
		{
			//QApplication::processEvents();

			CW3MessageBox msgBox("Will3D", lang::LanguagePack::txt_superimposition(), CW3MessageBox::ActionRole);
			msgBox.show();
			msgBox.raise();
			QApplication::processEvents();

			m_pJobMgr->runSuperImpose(secondToFirst, volume);

			msgBox.hide();

			m_pFILEtab->getSecondVolume()->setSecondToFirst(secondToFirst);
			m_pSItab->setSecVolume(secondToFirst);

			emit sigChangeTab(TabType::TAB_SI);
			m_pSItab->activate();
			m_bIsSIActivated = true;
		}
		m_pFILEtab->setEnableLoadSecondVolume(false);
	}
}
#endif

void CW3TabMgr::slotLoadDicomFromLoader()
{
	if (m_pgVREngine->getVRparams(0) || m_pgRcontainer->getFacePhoto3D())
	{
		m_pgRcontainer->reset();
		m_pgVREngine->reset();

		SAFE_DELETE_OBJECT(m_pMPRengine);
		m_pMPRengine = new CW3MPREngine();
#ifndef WILL3D_LIGHT
		SAFE_DELETE_OBJECT(m_pVTOSTO);
		m_pVTOSTO = new CW3VTOSTO(m_pgVREngine, m_pJobMgr, m_pgRcontainer);
#endif
	}
	CW3ProgressDialog* progress =
		CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

#ifndef WILL3D_VIEWER
#if 0
	if (!m_pFILEtab->loadDicom()) return;
#else
	m_pFILEtab->loadDicom();
#endif
#endif

	QElapsedTimer timer;
	timer.start();

	qDebug() << "start slotLoadDicomFromLoader";

	progress->setFormat(CW3ProgressDialog::WAITTING);

	CW3Image3D* volume = m_pFILEtab->getVolume();
	if (!volume)
	{
#if 0
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"CW3TabMgr::slotLoadDicomFromLoader : Image3D is nullptr");
#else
		common::Logger::instance()->Print(common::LogType::ERR, "CW3TabMgr::slotLoadDicomFromLoader : Image3D is nullptr");
		return;
#endif
	}

#if 0
	const auto& main_volume_resource = ResourceContainer::GetInstance()->GetMainVolume();

	if (!&main_volume_resource)
	{
		std::shared_ptr<CW3Image3D> volume_smart_ptr;
		volume_smart_ptr.reset(volume);
		ResourceContainer::GetInstance()->SetMainVolumeResource(volume_smart_ptr);
	}
#endif

#ifndef WILL3D_VIEWER
	project_in_.reset();
#endif
	measure_res_mgr_->Reset();
	m_pgVREngine->makeCurrent();
	QFuture<void> ftGetVolume = QtConcurrent::run(this, &CW3TabMgr::setVolume, volume);
	watcher.setFuture(ftGetVolume);
	disconnect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));
	progress->setModal(false);
	progress->show();
	progress->run();
	progress->raise();
	watcher.waitForFinished();
	m_pgVREngine->doneCurrent();
	m_pgVREngine->setVolume(volume, 0);

	timer.restart();

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetMainVolume(); // TODO: 占쌈듸옙 占쏙옙占쏙옙 or thread

	qDebug() << "slotLoadDicomFromLoader 3 :" << timer.elapsed();
	timer.restart();

	common_task_tool_->ResetUI();
	otf_task_tool_->ResetUI();

	SAFE_DELETE_OBJECT(m_pMPRtab);
	SAFE_DELETE_OBJECT(m_pPANOtab);
//20250210 LIN
//#ifndef WILL3D_VIEWER
	SAFE_DELETE_OBJECT(m_pIMPLANTtab);
//#endif
	SAFE_DELETE_OBJECT(m_pTMJtab);
#ifndef WILL3D_LIGHT
	SAFE_DELETE_OBJECT(m_pSItab);
	SAFE_DELETE_OBJECT(m_pENDOtab);
	SAFE_DELETE_OBJECT(m_pCEPHtab);
	SAFE_DELETE_OBJECT(m_pFACEtab);
#endif
	SAFE_DELETE_OBJECT(m_pREPORTtab);

	InitTabs(false);
	viewerTabConnections();

	qDebug() << "slotLoadDicomFromLoader 4 :" << timer.elapsed();
	timer.restart();

	pano_engine_->Initialize(*volume);
	tmj_engine_->Initialize(*volume);

	qDebug() << "slotLoadDicomFromLoader 5 :" << timer.elapsed();
	timer.restart();

	emit sigChangeTab(TabType::TAB_MPR);

	QApplication::processEvents();
	progress->setModal(true);
	progress->hide();

	m_pMPRtab->activate(volume->pixelSpacing(), volume->sliceSpacing());

	qDebug() << "slotLoadDicomFromLoader 6 :" << timer.elapsed();
	timer.restart();

#ifndef WILL3D_VIEWER
	if (m_pFILEtab->isProject())
	{
		slotLoadProject(m_pFILEtab->getProjectPath());
	}
#endif

	m_bVolumeLoaded = true;
	m_bIsSIActivated = false;

	emit sigSetDicomInfo();

	qDebug() << "end slotLoadDicomFromLoader :" << timer.elapsed();

}

#ifndef WILL3D_LIGHT
void CW3TabMgr::slotLoadSecondVolume()
{
	m_pFILEtab->setEnableLoadSecondVolume(true);

	emit sigChangeTab(TabType::TAB_FILE);
}
#endif

void CW3TabMgr::SetTabsInvisible()
{
	// smseo : 占쏙옙占?占쏙옙占쏙옙 life time 占쏙옙 占쏙옙占쏙옙占실뤄옙 file tab占쏙옙 null check 占쏙옙 占싼댐옙.
	if (!m_pFILEtab) return;

	for (auto& tab : casted_tabs_)
	{
		if (!tab)
		{
			continue;
		}
		tab->SetVisibleWindows(false);
	}
}

void CW3TabMgr::setVolume(CW3Image3D* volume)
{
	m_pJobMgr->setVolume(volume);

	m_pMPRengine->setVolume(volume, 0);

	m_pgVREngine->readyForSetVolume(volume, 0);
	m_pgVREngine->setMPRengine(m_pMPRengine);
}

#ifndef WILL3D_LIGHT
void CW3TabMgr::setSecondVolume(CW3Image3D* volume)
{
	m_pMPRengine->setVolume(volume, 1);
	m_pgVREngine->readyForSetVolume(volume, 1);
}
#endif
#ifndef WILL3D_VIEWER
void CW3TabMgr::ImportProjectWhereChangedTab(const TabType& tab_type)
{
	if (!project_in_) return;

	bool is_counterpart_project_exist = false;
	switch (tab_type)
	{
	case TabType::TAB_MPR:
	{
		std::unique_ptr<ProjectIOMPR> pio_mpr;
		project_in_->MoveMPRTabIO(&pio_mpr);
		if (pio_mpr.get()) m_pMPRtab->importProject(*pio_mpr.get());
		break;
	}
	case TabType::TAB_PANORAMA:
	{
		is_counterpart_project_exist = project_in_->IsImplantProjectExists();
		std::unique_ptr<ProjectIOPanorama> pio_pano;
		project_in_->MovePanoTabIO(&pio_pano);
		if (pio_pano.get() && pio_pano->IsInitPano())
			m_pPANOtab->importProject(
				*pio_pano.get(),
				is_counterpart_project_exist || m_pIMPLANTtab->initialized());
		else if (is_counterpart_project_exist)
			m_pPANOtab->SyncMeasureResource();
		break;
	}
	case TabType::TAB_IMPLANT:
	{
		is_counterpart_project_exist = project_in_->IsPanoProjectExists();
		std::unique_ptr<ProjectIOImplant> pio_implant;
		project_in_->MoveImplantTabIO(&pio_implant);
		if (pio_implant.get() && pio_implant->IsInitImplant())
			m_pIMPLANTtab->importProject(
				*pio_implant.get(),
				is_counterpart_project_exist || m_pPANOtab->initialized());
		else if (is_counterpart_project_exist)
			m_pIMPLANTtab->SyncMeasureResource();
		break;
	}
	case TabType::TAB_TMJ:
	{
		std::unique_ptr<ProjectIOTMJ> pio_tmj;
		project_in_->MoveTMJTabIO(&pio_tmj);
		if (pio_tmj.get()) m_pTMJtab->importProject(*pio_tmj.get());
		break;
	}
#ifndef WILL3D_LIGHT
	case TabType::TAB_3DCEPH:
	{
		std::unique_ptr<ProjectIOCeph> pio_ceph;
		project_in_->MoveCephTabIO(&pio_ceph);
		if (pio_ceph.get()) m_pCEPHtab->importProject(*pio_ceph.get());
		break;
	}
	case TabType::TAB_FACESIM:
	{
		std::unique_ptr<ProjectIOFace> pio_facesim;
		project_in_->MoveFaceTabIO(&pio_facesim);
		if (pio_facesim.get()) m_pFACEtab->importProject(*pio_facesim.get());
		break;
	}
	case TabType::TAB_SI:
	{
		std::unique_ptr<ProjectIOSI> pio_si;
		project_in_->MoveSITabIO(&pio_si);
		if (pio_si.get()) m_pSItab->importProject(*pio_si.get());
		break;
	}
	case TabType::TAB_ENDO:
	{
		std::unique_ptr<ProjectIOEndo> pio_endo;
		project_in_->MoveEndoTabIO(&pio_endo);
		if (pio_endo.get()) m_pENDOtab->importProject(*pio_endo.get());
		break;
	}
#endif
	default:
		break;
	}
}
#endif
void CW3TabMgr::changeTab(const TabType& tab_type)
{
	common_task_tool_->ReleaseOtherButtons();
	last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;

  if(tab_type != TabType::TAB_REPORT)
	  SetTabsInvisible();

	switch (tab_type)
	{
	case TabType::TAB_PANORAMA:
		break;

//20250210 LIN
//#ifndef WILL3D_VIEWER
	case TabType::TAB_IMPLANT:
		SetOTFpresetAndButtonStatus(common::otf_preset::TEETH);
		break;
//#endif
	case TabType::TAB_MPR:
	case TabType::TAB_TMJ:
#if 0
		SetOTFpresetAndButtonStatus(default_otf_preset);
#endif
		break;
#ifndef WILL3D_LIGHT
	case TabType::TAB_SI:
	case TabType::TAB_ENDO:
#if 0
		SetOTFpresetAndButtonStatus(default_otf_preset);
#endif
		break;
	case TabType::TAB_3DCEPH:
#if 0
		SetOTFpresetAndButtonStatus(default_otf_preset);
#endif
		m_pFACEtab->ReleaseSurgeryViewFromLayout();
		m_pCEPHtab->SetLayout();
		break;
	case TabType::TAB_FACESIM:
#if 0
		SetOTFpresetAndButtonStatus(default_otf_preset);
#endif
		m_pCEPHtab->ReleaseSurgeryViewFromLayout();
		m_pFACEtab->SetLayout();
		break;
#endif
	default:
		break;
	}

	tab_type_ = tab_type;
	casted_tabs_[tab_type_]->set_tab_changed(true);

	measure_res_mgr_->SetCurrentTab(tab_type_);

	const auto& event_common_handler =EventHandler::GetInstance()->GetCommonEventHandle();
	event_common_handler.EmitSigSetTabSlotLayout(casted_tabs_[tab_type_]->GetTabLayout());
	
  casted_tabs_[tab_type_]->SetVisibleWindows(true);
 

#ifndef WILL3D_LIGHT
	if (tab_type_ == TabType::TAB_SI)
	{
		if (!m_bIsSIActivated)
		{
			m_pSItab->activate();
			m_bIsSIActivated = true;
		}
	}
#endif

#ifndef WILL3D_VIEWER
	ImportProjectWhereChangedTab(tab_type);
#endif

#ifndef WILL3D_VIEWER
	bool enable_common_task_tool = (tab_type == TabType::TAB_FILE || tab_type == TabType::TAB_REPORT) ? false : true;
#else
	bool enable_common_task_tool = (tab_type != TabType::TAB_REPORT);
#endif

	if (enable_common_task_tool)
	{
		emit sigTempSyncCommonTaskEvent();
	}

	common_task_tool_->EnableAllButtons(enable_common_task_tool);

	casted_tabs_[tab_type_]->set_tab_changed(false);

#ifdef WILL3D_EUROPE
	if (button_list_dialog_ != nullptr)
	{
		button_list_dialog_->SetCheckedButton(0, false);		//ButtonID::BTN_HIDE_UI	0
		slotCommonToolOnOff(common::CommonToolTypeOnOff::NONE);

		bool hide_ui = button_list_dialog_->GetButtonState(1);	//ButtonID::BTN_HIDE_UI		1
		bool grid_on = button_list_dialog_->GetButtonState(2);	//ButtonID::BTN_GRID_ONOFF	2
		
		casted_tabs_[tab_type_]->SetCommonToolOnce(common::CommonToolTypeOnce::V_HIDE_UI, hide_ui);
		casted_tabs_[tab_type_]->SetCommonToolOnce(common::CommonToolTypeOnce::V_GRID, grid_on);
	}
#endif // WILL3D_EUROPE	
}

#ifndef WILL3D_VIEWER
void CW3TabMgr::SaveProject()
{
	try
	{
		if (!m_pgVREngine->getVol(0))
		{
			CW3MessageBox msg_box("Will3D", lang::LanguagePack::msg_74(),
				CW3MessageBox::Warning);
			msg_box.exec();
			return;
		}

#if 1
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(this, &CW3TabMgr::SaveStatus, GetScreenshot(false));
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
#else
		SaveStatus();
#endif

#if defined(_WIN32)
		// send message to willmaster
		if (m_bSaveForExternalProgram)
		{
			UINT regMsg = RegisterWindowMessage(L"Lucion.ProjectSaved.Notification");
			HWND hwnd = FindWindow(NULL, L"Will-Master");
			if (hwnd) PostMessage(hwnd, regMsg, NULL, NULL);
		}
#endif

		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_30(), CW3MessageBox::Information);
		message_box.setDetailedText(lang::LanguagePack::msg_31());
		message_box.exec();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "CW3TabMgr::slotSaveProject: " << e.what() << std::endl;
	}
}
#endif

//20250122 LIN
#ifndef WILL3D_VIEWER
void CW3TabMgr::SavePanoEngineContent()
{
	try
	{
		if (!m_pgVREngine->getVol(0))
		{
			CW3MessageBox msg_box("Will3D", lang::LanguagePack::msg_74(),
				CW3MessageBox::Warning);
			msg_box.exec();
			return;
		}
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(this, &CW3TabMgr::SavePanoEngineContentStatus);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "CW3TabMgr::slotSaveProject: " << e.what() << std::endl;
	}
}
#endif


//20250122 LIN
#ifndef WILL3D_VIEWER
void CW3TabMgr::SavePanoEngineContentStatus()
{
	// hdd size check
	CW3Image3D* secondVol = m_pFILEtab->getSecondVolume();
	if (secondVol)
	{
		QStorageInfo storage = QStorageInfo::root();
		qint64 availableHdd = storage.bytesAvailable();
		qint64 secondVolSize = secondVol->width() * secondVol->height() *
			secondVol->depth() * sizeof(unsigned short);
		qint64 projectFileSize = secondVolSize + 10485760;
		if (availableHdd < projectFileSize)
		{
			CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_32(), CW3MessageBox::Critical);
			msgBox.exec();
			return;
		}
	}

	CW3Image3D* vol = m_pgVREngine->getVol(0);
	const HeaderList& header = vol->getHeader()->getListCore();
	DicomLoader::PatientInfo patient;
	DicomLoader::DicomInfo series;

	for (auto& i : header)
	{
		if (i.first == "PatientID")
			patient.patient_id = i.second;
		else if (i.first == "PatientName")
			patient.patient_name = i.second;
		else if (i.first == "StudyInstanceUID")
			series.study_uid = i.second;
		else if (i.first == "SeriesInstanceUID")
			series.series_uid = i.second;
		else if (i.first == "StudyDate")
			series.study_date = i.second;
	}

	QDate date = QDate::currentDate();
	QTime time = QTime::currentTime();
	QString year, month, day, hour, minute, second;
	year.sprintf("%04d", date.year());
	month.sprintf("%02d", date.month());
	day.sprintf("%02d", date.day());
	hour.sprintf("%02d", time.hour());
	minute.sprintf("%02d", time.minute());
	second.sprintf("%02d", time.second());

	QString description =
		year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;

	series.modality = "PRJ";
	series.num_image = 1;
	series.description = description;
	series.file_dir = m_pFILEtab->getOpendVolPath();

	patient.dicom_infos.push_back(&series);

	QString strTime = year + month + day + hour + minute + second;
	QString file_name = patient.patient_id + strTime;
	series.project_path = m_strOutputPath + "/" + file_name + ".w3d";

	common::Logger::instance()->Print(common::LogType::INF, "save project w3d : " + series.project_path.toStdString());

	qDebug() << "series.project_path is: " << series.project_path;

	QString curr_path = m_strOutputPath + "/" + "AppFiles" + "/";

	ProjectIO proj_io_save(project::Purpose::SAVE, series.project_path);
	pano_engine_->exportProject(proj_io_save.GetPanoEngineIO(), curr_path);
}
#endif

#ifdef WILL3D_VIEWER
void CW3TabMgr::UpdateViewerW3DFile()
{
	QDir parent_dir(QCoreApplication::applicationDirPath());
	parent_dir.cdUp();

	QString project_path;
	QStringList filters;
	filters << "*.w3d";
	QFileInfoList file_list = parent_dir.entryInfoList(filters, QDir::Files);

	if (!file_list.isEmpty()) {
		project_path = file_list.first().absoluteFilePath();
	}
	qDebug() << "series.project_path is: " << project_path;

	QString curr_path = m_strOutputPath + "/" + "AppFiles" + "/";

	ProjectIO proj_io_save(project::Purpose::SAVE, project_path);
	pano_engine_->exportProject(proj_io_save.GetPanoEngineIO(), curr_path);

}
#endif

#ifndef WILL3D_VIEWER
void CW3TabMgr::SaveStatus(QImage thumbnail_image)
{
	// hdd size check
	CW3Image3D* secondVol = m_pFILEtab->getSecondVolume();
	if (secondVol)
	{
		QStorageInfo storage = QStorageInfo::root();
		qint64 availableHdd = storage.bytesAvailable();
		qint64 secondVolSize = secondVol->width() * secondVol->height() *
			secondVol->depth() * sizeof(unsigned short);
		qint64 projectFileSize = secondVolSize + 10485760;
		if (availableHdd < projectFileSize)
		{
			CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_32(), CW3MessageBox::Critical);
			msgBox.exec();
			return;
		}
	}

	CW3Image3D* vol = m_pgVREngine->getVol(0);
	const HeaderList& header = vol->getHeader()->getListCore();
	DicomLoader::PatientInfo patient;
	DicomLoader::DicomInfo series;

	for (auto& i : header)
	{
		if (i.first == "PatientID")
			patient.patient_id = i.second;
		else if (i.first == "PatientName")
			patient.patient_name = i.second;
		else if (i.first == "StudyInstanceUID")
			series.study_uid = i.second;
		else if (i.first == "SeriesInstanceUID")
			series.series_uid = i.second;
		else if (i.first == "StudyDate")
			series.study_date = i.second;
	}

	QDate date = QDate::currentDate();
	QTime time = QTime::currentTime();
	QString year, month, day, hour, minute, second;
	year.sprintf("%04d", date.year());
	month.sprintf("%02d", date.month());
	day.sprintf("%02d", date.day());
	hour.sprintf("%02d", time.hour());
	minute.sprintf("%02d", time.minute());
	second.sprintf("%02d", time.second());

	QString description =
		year + "-" + month + "-" + day + " " + hour + ":" + minute + ":" + second;

	series.modality = "PRJ";
	series.num_image = 1;
	series.description = description;
	series.file_dir = m_pFILEtab->getOpendVolPath();

	patient.dicom_infos.push_back(&series);

	QString strTime = year + month + day + hour + minute + second;
	QString file_name = patient.patient_id + strTime;
	series.project_path = m_strOutputPath + "/" + file_name + ".w3d";

	common::Logger::instance()->Print(common::LogType::INF, "save project w3d : " + series.project_path.toStdString());

	qDebug() << "series.project_path is: " << series.project_path;
	ProjectIO proj_io_save(project::Purpose::SAVE, series.project_path);
	m_pMPRengine->ExportProject(proj_io_save.GetMPREngineIO());
	pano_engine_->exportProject(proj_io_save.GetPanoEngineIO());

	m_pFILEtab->exportProject(proj_io_save.GetFileTabIO());
	m_pMPRtab->exportProject(proj_io_save.GetMPRTabIO());
	m_pPANOtab->exportProject(proj_io_save.GetPanoTabIO());
	m_pIMPLANTtab->exportProject(proj_io_save.GetImplantTabIO());
	m_pTMJtab->exportProject(proj_io_save.GetTMJTabIO());
#ifndef WILL3D_LIGHT
	m_pVTOSTO->exportProject(proj_io_save.GetVTOSTOIO());
	m_pCEPHtab->exportProject(proj_io_save.GetCephTabIO());
	m_pFACEtab->exportProject(proj_io_save.GetFaceTabIO());
	m_pSItab->exportProject(proj_io_save.GetSITabIO());
	m_pENDOtab->exportProject(proj_io_save.GetEndoTabIO());
#endif

	measure_res_mgr_->ExportProject(proj_io_save.GetMeasureResourceIO());

	if (m_bSaveForExternalProgram)
	{
		QImage thumbnail = thumbnail_image.isNull() ? GetScreenshot(false) : thumbnail_image;
		QString thumbnailFileName = patient.patient_id + strTime + ".jpg";
		QString from_native = QDir::fromNativeSeparators(m_strOutputPath) + "/" + thumbnailFileName;
		QImageWriter imgWriter(from_native, "jpg");
		imgWriter.write(thumbnail);

		common::Logger::instance()->Print(common::LogType::INF, "save project jpg : " + from_native.toStdString());

		QString dicomFileName = patient.patient_id + strTime + ".dcm";
		QString dicomFilePath = m_strOutputPath + "/" + dicomFileName;
		std::string filePath = dicomFilePath.toStdString();

		HeaderList hd;
		hd["PatientID"] = patient.patient_id;
		hd["PatientName"] = patient.patient_name;
		hd["PatientSex"] = patient.patient_gender;
		hd["PatientBirthDate"] = patient.patient_birth;
		hd["StudyInstanceUID"] = series.study_uid;
		hd["SeriesInstanceUID"] = series.series_uid;
		hd["StudyDate"] = series.study_date;
		hd["SeriesDate"] = series.series_date;
		hd["StudyTime"] = series.study_time;
		hd["SeriesTime"] = series.series_time;
		hd["Modality"] = series.modality;
		hd["Rows"] = QString::number(thumbnail.height());
		hd["Columns"] = QString::number(thumbnail.width());
		hd["WindowCenter"] = QString::number(series.win_center);
		hd["WindowWidth"] = QString::number(series.win_width);
		hd["StudyDescription"] = series.description.toLocal8Bit();
		hd["ImageType"] = "DERIVED";
		hd["Manufacturer"] = "HDXWILL";
		hd["StudyDescription"] = "Project";
		hd["DerivationDescription"] = "Project";
		hd["PixelSpacing"] = "0\\0";
		hd["PixelAspectRatio"] = "1\\1";
		hd["ImagePositionPatient"] = "1\\1\\1";
		hd["SliceLocation"] = "1";
		hd["SeriesNumber"] = "1";
		hd["InstanceNumber"] = "1";
		hd["NumberOfFrames"] = "1";
		hd["PlanarConfiguration"] = "0";
		hd["PixelRepresentation"] = "0";
#if 0
		hd["RescaleIntercept"] = QString::number(series.intercept).toStdString();
		hd["RescaleSlope"] = QString::number(series.slope).toStdString();
#else
		hd["RescaleSlope"] = "1";
		hd["RescaleIntercept"] = "0";
#endif
		hd["WindowCenter"] = "128";
		hd["WindowWidth"] = "256";
		hd["BitsAllocated"] = "8";
		hd["BitsStored"] = "8";
		hd["HighBit"] = "7";
		hd["SamplesPerPixel"] = "3";
		hd["PhotometricInterpretation"] = "RGB";

#if 0
		qDebug() << thumbnail.width() << thumbnail.height();
		qDebug() << "thumbnail :" << thumbnail.format() << thumbnail.depth() << thumbnail.bitPlaneCount() << thumbnail.byteCount();
		QImage thumbnail_rgb = thumbnail.convertToFormat(QImage::Format_RGB888);
		qDebug() << thumbnail_rgb.width() << thumbnail_rgb.height();
		qDebug() << "thumbnail_rgb :" << thumbnail_rgb.format() << thumbnail_rgb.depth() << thumbnail_rgb.bitPlaneCount() << thumbnail_rgb.byteCount();
#endif

#if 0
		QPixelFormat format = thumbnail.pixelFormat();
		uchar cC = format.channelCount();
		uchar bpp = format.bitsPerPixel();
#endif
		uchar* data = thumbnail.bits();

		uchar* thumbnail_data_rgb = nullptr;
		W3::p_allocate_1D(&thumbnail_data_rgb, thumbnail.width() * thumbnail.height() * 3);
		memset(thumbnail_data_rgb, 0, sizeof(uchar) * thumbnail.width() * thumbnail.height() * 3);
#if 1
		for (int i = 0; i < thumbnail.width() * thumbnail.height(); ++i)
		{
			thumbnail_data_rgb[i * 3] = data[i * 4 + 2];
			thumbnail_data_rgb[i * 3 + 1] = data[i * 4 + 1];
			thumbnail_data_rgb[i * 3 + 2] = data[i * 4];
		}
#endif

		CW3DicomIO dicomIO;
		if (!dicomIO.writeDicom(filePath, hd, thumbnail_data_rgb))
		{
			common::Logger::instance()->Print(common::LogType::INF, std::string("save project dcm failed."));
		}

		common::Logger::instance()->Print(common::LogType::INF, std::string("project dcm file path : ") + filePath);

		SAFE_DELETE_ARRAY(thumbnail_data_rgb);
	}
	else
	{
		m_pFILEtab->insertDBforProject(&patient);
	}
}
#endif
void CW3TabMgr::CaptureScreenShot()
{
#ifndef WILL3D_VIEWER
	if (tab_type_ == TabType::TAB_FILE) return;
#endif

	QImage image = GetScreenshot(true);
	if (image.isNull())
	{
		return;
	}

	if (GlobalPreferences::GetInstance()->preferences_.capture.include_dicom_info)
	{
		DrawDicomInfoToImage(&image);
	}

	SaveScreenshot(image);
}

void CW3TabMgr::Print()
{
#ifndef WILL3D_VIEWER
	if (tab_type_ == TabType::TAB_FILE) return;
#endif

	QImage image = GetScreenshot(true, true);
	if (image.isNull()) return;

	if (GlobalPreferences::GetInstance()->preferences_.capture.include_dicom_info)
	{
		DrawDicomInfoToImage(&image);
	}

	QPrinter printer(QPrinter::HighResolution);
	printer.setOrientation(QPrinter::Landscape);

	int printPageWidth = printer.pageRect().width();
	int printPageHeight = printer.pageRect().height();

	QPrintDialog* dlg = new QPrintDialog(&printer, 0);

	if (dlg->exec() == QDialog::Accepted)
	{
		QPainter painter(&printer);
		if (!image.isNull())
			painter.drawImage(
				QPoint(0, 0),
				image.scaled(QSize(printPageWidth, printPageHeight),
					Qt::KeepAspectRatio, Qt::SmoothTransformation));
		painter.end();
	}

	delete dlg;
}

#ifndef WILL3D_VIEWER
void CW3TabMgr::CDExport()
{
	CDExportDialog cd_export_dlg;
	if (cd_export_dlg.exec() == QDialog::Rejected)
	{
		return;
	}

	bool is_include_viewer = cd_export_dlg.IsIncludeViewer();
	bool is_dicom_compressed = cd_export_dlg.IsDicomCompressed();
	DCMExportMethod method = cd_export_dlg.ExportMethod();
	QString export_path = cd_export_dlg.Path();

	m_pFILEtab->exportDICOM(export_path, is_include_viewer, is_dicom_compressed);

	//20250210 LIN ===
	QString series_date = m_pFILEtab->getVolume()->getHeader()->getSeriesDate();
	QString pat_id = m_pFILEtab->getVolume()->getHeader()->getPatientName();
	QString ori_output_path = m_strOutputPath;
	QString folder_name = series_date + "_" + pat_id;
	m_strOutputPath = export_path + "/" + folder_name;
	SavePanoEngineContentStatus();
	m_strOutputPath = ori_output_path;
	//===

	//m_pFILEtab->exportDICOM(export_path, is_include_viewer, is_dicom_compressed);
	//QString series_date = m_pFILEtab->getVolume()->getHeader()->getSeriesDate();
	//QString pat_id = m_pFILEtab->getVolume()->getHeader()->getPatientName();
	//QString ori_output_path = m_strOutputPath;
	//QString folder_name = series_date + "_" + pat_id;
	//m_strOutputPath = export_path + "/" + folder_name;
	//SaveProject();
	//m_strOutputPath = ori_output_path;
}
#endif

QImage CW3TabMgr::GetScreenshot(bool select_view, bool print_mode)
{
#if 0 // New : 占싹븝옙 환占썸에占쏙옙 file tab占싱놂옙 占쏙옙 화占쏙옙占쏙옙 캡占식듸옙
	QImage image;

	QScreen* screen = QGuiApplication::primaryScreen();
	QWindow* window = parentWidget()->windowHandle();

	if (!window)
	{
		common::Logger::instance()->Print(common::LogType::ERR, std::string("not found window"));
		return image;
	}
	screen = window->screen();

	if (!screen)
	{
		common::Logger::instance()->Print(common::LogType::ERR, std::string("not found screen"));
		return image;
	}

#if 0
	WId wid = 0;
#else
	WId wid = window->winId();
#endif

	BaseTab* current_tab = casted_tabs_[tab_type_];
	QRect tab_slot_rect;
#if 0
	emit sigGetTabSlotGlobalRect(tab_slot_rect);
#else
	emit sigGetTabSlotRect(tab_slot_rect);
#endif
	QString tab_slot_rect_log = QString("%1, %2, %3, %4").arg(tab_slot_rect.x()).arg(tab_slot_rect.y()).arg(tab_slot_rect.width()).arg(tab_slot_rect.height());
	common::Logger::instance()->Print(common::LogType::DBG, std::string("tab_slot_rect : ") + tab_slot_rect_log.toStdString());

	auto func_fullscreen_screenshot = [&]()
	{
		QPixmap screenshot = screen->grabWindow(
			wid,
			tab_slot_rect.x(),
			tab_slot_rect.y(),
			tab_slot_rect.width(),
			tab_slot_rect.height()
		);
		image = screenshot.toImage();
	};

	if (select_view)
	{
		if (!current_tab)
		{
			return image;
		}

		QStringList view_list = current_tab->GetViewList();
		CaptureDialog capture_dialog(view_list, this);

#if 1
		QWidget* button = nullptr;
		if (print_mode)
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::PRINT);
		}
		else
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::CAPTURE);
		}

		QRect rect = button->rect();
		QPoint pos = button->mapToGlobal(QPoint(0, 0)) + QPoint(0, rect.height());

		qDebug() << "pos :" << pos;

		capture_dialog.move(pos);
#endif

		int result = capture_dialog.exec();
		capture_dialog.hide();

		if (result < 0)
		{
			return image;
		}
		else if (result == 0)
		{
			func_fullscreen_screenshot();
			return image;
		}
		else
		{
#if 0
			image = current_tab->GetScreenshot(result);
#else
			QWidget* source = current_tab->GetScreenshotSource(result);
			QPoint source_pos = tab_slot_rect.topLeft() + source->pos();
			QRect source_rect = source->rect();

			QPixmap screenshot = screen->grabWindow(
				wid,
				source_pos.x(),
				source_pos.y(),
				source_rect.width(),
				source_rect.height()
			);

			image = screenshot.toImage();
#endif
			return image;
		}
	}
	else
	{
		func_fullscreen_screenshot();
		return image;
	}
#elif 1 // DPI Aware
	QImage image;
	BaseTab* current_tab = casted_tabs_[tab_type_];

	QRect target_rect;

	if (select_view)
	{
		if (!current_tab)
		{
			return image;
		}

		QStringList view_list = current_tab->GetViewList();
		CaptureDialog capture_dialog(view_list, this);

		QWidget* button = nullptr;
		if (print_mode)
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::PRINT);
			capture_dialog.setTitle(lang::LanguagePack::txt_print());
		}
		else
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::CAPTURE);
		}

		QRect rect = button->rect();
		QPoint pos = button->mapToGlobal(QPoint(0, 0)) + QPoint(0, rect.height());

		capture_dialog.move(pos);

		int result = capture_dialog.exec();
		capture_dialog.hide();

		if (result < 0)
		{
			return image;
		}
		else if (result == 0)
		{
			emit sigGetTabSlotGlobalRect(target_rect);
		}
		else
		{
			QWidget* source = current_tab->GetScreenshotSource(result);
			return GetScreenshot(source);
		}
	}
	else
	{
		emit sigGetTabSlotGlobalRect(target_rect);
	}

	return GetScreenshot(target_rect);

#else // Original : window 화占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 100% 占쏙옙占쏙옙 크占쏙옙 캡占쏙옙 占쏙옙占쏙옙 크占썩가 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙
	QImage image;
	BaseTab* current_tab = casted_tabs_[tab_type_];

	auto func_fullscreen_screenshot = [&]()
	{
		QScreen* screen = QGuiApplication::primaryScreen();

		QRect tab_slot_rect;
		emit sigGetTabSlotGlobalRect(tab_slot_rect);

		QString tab_slot_rect_log = QString("%1, %2, %3, %4").arg(tab_slot_rect.x()).arg(tab_slot_rect.y()).arg(tab_slot_rect.width()).arg(tab_slot_rect.y());
		common::Logger::instance()->Print(common::LogType::DBG, std::string("tab_slot_rect : ") + tab_slot_rect_log.toStdString());

#if 0
		int screen_number = qApp->desktop()->screenNumber(QCursor::pos());
		common::Logger::instance()->Print(common::LogType::DBG, std::string("screen_number : ") + QString::number(screen_number).toStdString());

		QWidget* desktop_screen = qApp->desktop()->screen(screen_number);

		QPixmap screenshot =
			screen->grabWindow(QApplication::desktop()->winId(),
				desktop_screen->x(), desktop_screen->y(),
				desktop_screen->width(), desktop_screen->height()
			);
#else
		if (const QWindow* window = windowHandle())
		{
			screen = window->screen();
		}

		if (!screen)
		{
			throw std::runtime_error("not found screen");
		}

		QPixmap screenshot =
			screen->grabWindow(0, tab_slot_rect.x(), tab_slot_rect.y(),
				tab_slot_rect.width(), tab_slot_rect.height());
#endif

		image = screenshot.toImage();
	};

	if (select_view)
	{
		if (!current_tab) return image;

		QStringList view_list = current_tab->GetViewList();
		CaptureDialog capture_dialog(view_list, this);

#if 1
		QWidget* button = nullptr;
		if (print_mode)
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::PRINT);
		}
		else
		{
			button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::CAPTURE);
		}

		QRect rect = button->rect();
		QPoint pos = button->mapToGlobal(QPoint(0, 0)) + QPoint(0, rect.height());

		capture_dialog.move(pos);
#endif

		int result = capture_dialog.exec();
		capture_dialog.hide();

		if (result < 0)
			return image;
		else if (result == 0)
		{
			func_fullscreen_screenshot();
			return image;
		}
		else
		{
			image = current_tab->GetScreenshot(result);
			return image;
		}
	}
	else
	{
		func_fullscreen_screenshot();
		return image;
	}
#endif
}

QImage CW3TabMgr::GetScreenshot(QWidget* source)
{
	if (!source)
	{
		return QImage();
	}

	QRect target_rect;
	QRect source_rect = source->rect();
	QPoint source_global_top_left = source->mapToGlobal(QPoint(0, 0));
	QPoint source_global_bottom_right = source->mapToGlobal(QPoint(source_rect.width(), source_rect.height()));
	target_rect.setTopLeft(source_global_top_left);
	target_rect.setBottomRight(source_global_bottom_right);

	return GetScreenshot(target_rect);
}

QImage CW3TabMgr::GetScreenshot(const QRect& target_rect)
{
	QImage image;
	if (!target_rect.isValid())
	{
		return image;
	}

	QList<QScreen*> screens = QGuiApplication::screens();
	if (screens.empty())
	{
		throw std::runtime_error("not found screen");
	}

	int screen_number = qApp->desktop()->screenNumber(parentWidget());

	common::Logger::instance()->Print(common::LogType::DBG, std::string("screen_number : ") + QString::number(screen_number).toStdString());

	QString log = QString("screens : %1 / current : %2").arg(screens.size()).arg(screen_number);
	common::Logger::instance()->Print(common::LogType::DBG, log.toStdString());

	QSizeF original_screen_size = GetOriginalScreenSize(screen_number);

	QSizeF screen_scale_factor(1.0f, 1.0f);

	QWidget* screen_widget = qApp->desktop()->screen(screen_number);

	QSizeF virtual_screen_size(static_cast<float>(screen_widget->width()), static_cast<float>(screen_widget->height()));
	if (original_screen_size != virtual_screen_size)
	{
		screen_scale_factor = QSizeF(original_screen_size.width() / virtual_screen_size.width(), original_screen_size.height() / virtual_screen_size.height());
	}

	if (!IsWindows8OrGreater())
	{
		common::Logger::instance()->Print(common::LogType::DBG, "Windows 7");
		screen_scale_factor = QSizeF(1.0f, 1.0f);
	}

	QString target_rect_log = QString("%1, %2, %3, %4").arg(target_rect.x()).arg(target_rect.y()).arg(target_rect.width()).arg(target_rect.height());
	common::Logger::instance()->Print(common::LogType::DBG, std::string("target_rect : ") + target_rect_log.toStdString());

	QRectF target_rect_applied_dpi;
	float x = 0;
	if (screen_scale_factor == QSizeF(1.0f, 1.0f))
	{
		x = target_rect.x();
	}
	else
	{
		QPoint original_target_center = target_rect.center();
		float original_width = original_screen_size.width();
		float virtual_width = virtual_screen_size.width();

		float original_local_center_x = original_target_center.x() - screen_widget->mapToGlobal(pos()).x();

		float ratio = static_cast<float>(original_local_center_x) / original_width;
		float virtual_local_center_x = static_cast<int>(virtual_width * ratio);

		float new_center_x = original_target_center.x() + (original_local_center_x - virtual_local_center_x) * screen_scale_factor.width();
		x = new_center_x - target_rect.width() * screen_scale_factor.width() * 0.5f;
	}

	target_rect_applied_dpi.setX(round(x));
	target_rect_applied_dpi.setY(round((target_rect.y()) * screen_scale_factor.height()));
	target_rect_applied_dpi.setWidth(round(target_rect.width() * screen_scale_factor.width()));
	target_rect_applied_dpi.setHeight(round(target_rect.height() * screen_scale_factor.height()));

	common::Logger::instance()->Print(common::LogType::DBG, log.toStdString());

	log = QString("target_rect : %1x%2 / target_rect_applied_dpi : %3x%4 / screen_scale_factor : %5, %6")
		.arg(target_rect.size().width())
		.arg(target_rect.size().height())
		.arg(target_rect_applied_dpi.size().width())
		.arg(target_rect_applied_dpi.size().height())
		.arg(screen_scale_factor.width())
		.arg(screen_scale_factor.height());

	common::Logger::instance()->Print(common::LogType::DBG, log.toStdString());

	QPixmap screenshot = screens.at(0)->grabWindow(0, target_rect_applied_dpi.x(), target_rect_applied_dpi.y(), target_rect_applied_dpi.width(), target_rect_applied_dpi.height());
	image = screenshot.toImage();

	return image;
}

void CW3TabMgr::DrawDicomInfoToImage(QImage* image)
{
	CW3Image3D* vol = m_pgVREngine->getVol(0);

	if (vol == nullptr) throw std::runtime_error("not set data");

	CW3ImageHeader* header = vol->getHeader();
	QPainter painter;
	if (painter.begin(image))
	{
		QString patient_info;
		patient_info = QString("ID : %1\r\n%2[%3]\r\n%4\r\n%5[kVp] | %6[mA] | %7")
			.arg(header->getPatientID())
			.arg(header->getPatientName())
			.arg(header->getPatientSex())
			.arg(header->getSeriesDate())
			.arg(header->getKVP())
			.arg(header->getXRayTubeCurrent())
			.arg(header->getModality());
		painter.setPen(Qt::green);
		QRect text_rect = image->rect();
		text_rect.setBottom(text_rect.bottom() - 10);
		text_rect.setLeft(40);
		painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignBottom, patient_info);
		painter.end();
	}
}

void CW3TabMgr::keyPressEvent(QKeyEvent* event)
{
	QWidget::keyPressEvent(event);
}

void CW3TabMgr::keyReleaseEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Escape:
		if (common_task_tool_->RemoveUnfinishiedMeasureTool()) event->accept();
	default:
		break;
	}
	QWidget::keyReleaseEvent(event);
}
#ifndef WILL3D_VIEWER
void CW3TabMgr::slotLoadProject(QString path)
{
	emit sigActiveLoadProject();

#if 0
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	auto future = QtConcurrent::run(this, &CW3TabMgr::loadProject, path);
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();
#else
	loadProject(path);
#endif

	emit sigDoneLoadProject();
}
#endif

//20250122 LIN
void CW3TabMgr::loadProjectForPanoEngineContent(const QString& path)
{
	QFile project(path);
	if (!project.open(QIODevice::ReadOnly))
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"CW3TabMgr::loadProject: failed to load project.");
		return;
	}
	project.close();

	std::string file_name;
	if (path.contains(".w3d"))
	{
		QFileInfo file_info(path);
		QString f_name = file_info.baseName();
		file_name = f_name.toLocal8Bit().data();
	}
	else
	{
		file_name = path.toLocal8Bit().data();
	}
	if (!project_in_)
	{
		project_in_.reset(new ProjectIO(project::Purpose::LOAD, path));
	}

	std::unique_ptr<ProjectIOPanoEngine> pio_pano_engine;
	project_in_->MovePanoEngineIO(&pio_pano_engine);
	if (pio_pano_engine.get())
	{
		pano_engine_->importProject(*pio_pano_engine.get());
		m_pPANOtab->importProjectPanoTool(*pio_pano_engine.get());
		bool is_counterpart_project_exist = false;
		is_counterpart_project_exist = project_in_->IsPanoProjectExists();
		std::unique_ptr<ProjectIOImplant> pio_implant;
		project_in_->MoveImplantTabIO(&pio_implant);
		//if (pio_implant.get() && pio_implant->IsInitImplant())
		//	m_pIMPLANTtab->importProject(
		//		*pio_implant.get(),
		//		is_counterpart_project_exist || m_pPANOtab->initialized());
		//else if (is_counterpart_project_exist)
		//	m_pIMPLANTtab->SyncMeasureResource();
		m_pIMPLANTtab->importProject(
			*pio_implant.get(),
			is_counterpart_project_exist || m_pPANOtab->initialized());
	}
	//...
	changeTab(TabType::TAB_MPR);
	//implant/nerve蹂댁씠寃??섎젮硫?syscstatusmprmenus !
	m_pMPRtab->SyncStatusMPRmenus();
}

#ifndef WILL3D_VIEWER
void CW3TabMgr::loadProject(const QString& path)
{
	QFile project(path);
	if (!project.open(QIODevice::ReadOnly))
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"CW3TabMgr::loadProject: failed to load project.");
		return;
	}
	project.close();

	std::string file_name;
	if (path.contains(".w3d"))
	{
		QFileInfo file_info(path);
		QString f_name = file_info.baseName();
		file_name = f_name.toLocal8Bit().data();
	}
	else
	{
		file_name = path.toLocal8Bit().data();
	}

	if (!project_in_)
	{
		project_in_.reset(new ProjectIO(project::Purpose::LOAD, path));
	}

	std::unique_ptr<ProjectIOMeasureResource> pio_measure_resource;
	project_in_->MoveMeasureResourceIO(&pio_measure_resource);
	if (pio_measure_resource.get() &&
		pio_measure_resource->IsValid())
	{
		measure_res_mgr_->ImportProject(*pio_measure_resource.get());
	}

	std::unique_ptr<ProjectIOMPREngine> pio_mpr_engine;
	project_in_->MoveMPREngineIO(&pio_mpr_engine);
	if (pio_mpr_engine.get())
	{
		m_pMPRengine->ImportProject(*pio_mpr_engine.get());
	}

	std::unique_ptr<ProjectIOPanoEngine> pio_pano_engine;
	project_in_->MovePanoEngineIO(&pio_pano_engine);
	if (pio_pano_engine.get())
	{
		pano_engine_->importProject(*pio_pano_engine.get());
		m_pPANOtab->importProjectPanoTool(*pio_pano_engine.get());
	}

#ifndef WILL3D_LIGHT
	std::unique_ptr<ProjectIOVTOSTO> pio_vtosto;
	project_in_->MoveVTOSTOIO(&pio_vtosto);
	if (pio_vtosto.get()) m_pVTOSTO->importProject(*pio_vtosto.get());
#endif

	std::unique_ptr<ProjectIOFile> pio_file;
	project_in_->MoveFileTabIO(&pio_file);
	if (pio_file.get())
	{
#if 0
		m_pFILEtab->importProject(*pio_file.get());
#else
#ifndef WILL3D_LIGHT
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(this, &CW3TabMgr::LoadSecondVolumeDataFromProject, pio_file.get());
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
#endif
#endif
	}

#ifndef WILL3D_LIGHT
	if (project_in_->GetEndoTabIO().IsInit())
	{
		TabType prev_tab = tab_type_;
		emit sigChangeTab(TabType::TAB_ENDO);
		emit sigChangeTab(prev_tab);
	}

	if (m_pFILEtab->getCurrentSetting() == CW3FILEtab::SECOND_VOLUME)
	{
		slotLoadSecondDicomFromLoader();
	}
	else
#endif
	{
		changeTab(TabType::TAB_MPR);
	}

	m_pMPRtab->SyncStatusMPRmenus();
}
#endif
#ifndef WILL3D_VIEWER
bool CW3TabMgr::LoadMainVolumeDataFromProject(ProjectIOFile* io_file)
{
	return m_pFILEtab->ImportMainVolumeFromProject(*io_file);
}
#endif
#ifndef WILL3D_LIGHT
void CW3TabMgr::LoadSecondVolumeDataFromProject(ProjectIOFile* io_file)
{
	m_pFILEtab->importProject(*io_file);
}
#endif

bool CW3TabMgr::CloseEvent()
{
#ifndef WILL3D_VIEWER
	bool data_opened = true;

	if (!parentWidget())
	{
		data_opened = false;
	}

	if (!&ResourceContainer::GetInstance()->GetMainVolume())
	{
		data_opened = false;
	}

	if (data_opened)
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_33(), CW3MessageBox::ActionRole);
		message_box.setDetailedText(lang::LanguagePack::msg_51());
		QObject* yes_button = message_box.AddButton(lang::LanguagePack::txt_yes());
		QObject* no_button = message_box.AddButton(lang::LanguagePack::txt_no());
		message_box.AddButton(lang::LanguagePack::txt_cancel());

		message_box.exec();

		if (message_box.clickedButton() == yes_button)
		{
			SaveProject();
		}

		if (message_box.clickedButton() == yes_button || message_box.clickedButton() == no_button)
		{
			return true;
		}
	}
	else
#endif
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_19(), CW3MessageBox::Warning);
		if (message_box.exec())
		{
			return true;
		}
	}

	return false;
}

#ifndef WILL3D_LIGHT
void CW3TabMgr::slotSetTRDFromExternalProgram(const QString path,
	const bool onlyTRD)
{
	m_pFACEtab->setTRDFromExternalProgram(path, onlyTRD);

	if (onlyTRD)
	{
		// emit sigChangeTab(ETAB_TYPE::TAB_MPR);
		m_pMPRtab->setOnlyTRDMode();
		changeTab(TabType::TAB_MPR);
		QApplication::processEvents();

		connect(common_task_tool_.get(), &CommonTaskTool::sigCommonToolOnce, this,
			&CW3TabMgr::slotCommonToolOnce);
		connect(common_task_tool_.get(), &CommonTaskTool::sigCommonToolOnOff, this,
			&CW3TabMgr::slotCommonToolOnOff);

		for (auto& tab : casted_tabs_)
		{
			connect(tab, &BaseTab::sigCommonToolCancelSelected, this,
				&CW3TabMgr::slotCommonToolCancelSelected);
		}
	}
}
#endif

void CW3TabMgr::slotGraphicsSceneMousePressEvent(QMouseEvent* event)
{
	if (event->buttons() == (Qt::LeftButton | Qt::RightButton))
	{
		is_pressed_l_ = true;
		is_pressed_r_ = true;
	}
	else if (event->buttons() == Qt::LeftButton)
	{
		is_pressed_l_ = true;
	}
	else if (event->buttons() == Qt::RightButton)
	{
		is_pressed_r_ = true;
	}

	if (is_pressed_l_ && is_pressed_r_)
	{
		auto tool_type = common_task_tool_->CurrToolType();

		if (tool_type != common::CommonToolTypeOnOff::V_ZOOM_R &&
			tool_type != common::CommonToolTypeOnOff::V_PAN_LR)
		{
			last_common_tool_type_ = tool_type;
		}

		common_task_tool_->SetTaskToolOnOff(common::CommonToolTypeOnOff::V_PAN_LR, true);
		is_pressed_lr_ = true;
	}

	if (!is_pressed_lr_ && is_pressed_r_)
	{
		auto tool_type = common_task_tool_->CurrToolType();

		if (tool_type != common::CommonToolTypeOnOff::V_PAN_LR &&
			tool_type != common::CommonToolTypeOnOff::V_ZOOM_R)
		{
			last_common_tool_type_ = tool_type;
		}

		common_task_tool_->SetTaskToolOnOff(common::CommonToolTypeOnOff::V_ZOOM_R, true);
	}
}

void CW3TabMgr::slotGraphicsSceneMouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton) is_pressed_r_ = false;

	if (event->button() == Qt::LeftButton) is_pressed_l_ = false;

	if (!is_pressed_lr_ && !is_pressed_r_ && event->button() == Qt::RightButton)
	{
		common_task_tool_->SetTaskToolOnOff(last_common_tool_type_, true);
#if 1
		if (last_common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR ||
			last_common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R)
		{
			last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
		}
#else
		last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
#endif
	}

	if (!is_pressed_l_ && !is_pressed_r_ && is_pressed_lr_)
	{
		common_task_tool_->SetTaskToolOnOff(last_common_tool_type_, true);
#if 1
		if (last_common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR ||
			last_common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R)
		{
			last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
		}
#else
		last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
#endif
		is_pressed_lr_ = false;
	}
}

#ifndef WILL3D_VIEWER
void CW3TabMgr::slotShowImplantAngle()
{
	ImplantAngleDialog dlg(this);
	dlg.exec();
}
#endif
void CW3TabMgr::slotFileTool(const CommonToolTypeFile& type)
{
  CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_30(), CW3MessageBox::ActionRole);
  QObject* yes_button = nullptr;
  QObject* no_button = nullptr;
	switch (type)
	{
	case CommonToolTypeFile::SAVE:
#ifndef WILL3D_VIEWER

		SaveProject();
#endif

#ifdef WILL3D_VIEWER
    message_box.setDetailedText(lang::LanguagePack::msg_95());
    yes_button = message_box.AddButton(lang::LanguagePack::txt_yes());
    no_button = message_box.AddButton(lang::LanguagePack::txt_no());

    message_box.exec();

    if (message_box.clickedButton() == yes_button)
	  	UpdateViewerW3DFile();
#endif
		break;
	case CommonToolTypeFile::CAPTURE:
		CaptureScreenShot();
		break;
	case CommonToolTypeFile::PRINT:
		Print();
		break;
#ifndef WILL3D_VIEWER
	case CommonToolTypeFile::CDEXPORT:
		CDExport();
		break;
	case CommonToolTypeFile::PACS:
		ShowPACSDialog();
		break;
#endif
	default:
		break;
	}
}

void CW3TabMgr::slotCommonToolOnce(const common::CommonToolTypeOnce& type, bool on)
{
	if (type == common::CommonToolTypeOnce::V_INVERT)
		RendererManager::GetInstance().InvertLight(on);

	casted_tabs_[tab_type_]->SetCommonToolOnce(type, on);
}

void CW3TabMgr::slotCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	if (type == common::CommonToolTypeOnOff::NONE)
	{
		last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	}
	casted_tabs_[tab_type_]->SetCommonToolOnOff(type);
}

void CW3TabMgr::slotCommonToolCancelSelected()
{
	last_common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	common_task_tool_->CancelSelectedTool();
}

void CW3TabMgr::slotCommonToolMeasureListOn()
{
	common::measure::MeasureDataContainer datas;
	measure_res_mgr_->GetMeasureDatas(&datas);
	MeasureListDialog dlg(datas, this);

	connect(&dlg, &MeasureListDialog::sigMeasureDelete, this,
		&CW3TabMgr::slotMeasureDelete);
	connect(&dlg, &MeasureListDialog::sigMeasureChangeMemo,
		measure_res_mgr_.get(), &MeasureResourceMgr::slotMeasureChangeMemo);
	connect(&dlg, &MeasureListDialog::sigMeasureSelect, this,
		&CW3TabMgr::slotMeasureSelect);
	connect(&dlg, &MeasureListDialog::sigMeasureListCapture, this, &CW3TabMgr::slotMeasureListCapture);
	dlg.exec();
}

#ifndef WILL3D_LIGHT
bool CW3TabMgr::SetEnableOnlyTRDMode()
{
	bool enable = m_pFILEtab->isOnlyTRD();
	common_task_tool_->SetOnlyTRDMode(enable);
	return enable;
}

void CW3TabMgr::slotSetViewFaceAfter()
{
	m_pFACEtab->setViewFaceAfter(m_pCEPHtab->getView3DCeph());
}
#endif

void CW3TabMgr::ApplyPreferences()
{
	RendererManager::GetInstance().ApplyPreferences();
	for (auto& elem : casted_tabs_) elem->ApplyPreferences();
}

void CW3TabMgr::SetApplicationUIMode(const bool& is_maximize)
{
	for (auto& tab : casted_tabs_)
	{
		if (tab->initialized()) tab->SetApplicationUIMode(is_maximize);
	}
}

#ifndef WILL3D_LIGHT
void CW3TabMgr::slotSave3DFaceToPLYFile()
{
	if (!m_pVTOSTO)
	{
		return;
	}

	m_pVTOSTO->SavePLY(GetFolderPathForExport3DFace() + GetFileNameForExport3DFace() + ".ply");

	CW3MessageBox message_box("File saved", "path : " + GetFolderPathForExport3DFace(), CW3MessageBox::Information);
	message_box.exec();
}

void CW3TabMgr::slotSave3DFaceToOBJFile()
{
	if (!m_pVTOSTO)
	{
		return;
	}

	m_pVTOSTO->SaveOBJ(GetFolderPathForExport3DFace() + GetFileNameForExport3DFace() + ".obj");

	CW3MessageBox message_box("File saved", "path : " + GetFolderPathForExport3DFace(), CW3MessageBox::Information);
	message_box.exec();
}

QString CW3TabMgr::GetFolderPathForExport3DFace()
{
	QDir export_dir(kExport3DFaceFolderPath);
	if (!export_dir.exists())
	{
		export_dir.mkpath(export_dir.absolutePath());
	}

	QString patient_id = ResourceContainer::GetInstance()->GetMainVolume().getHeader()->getPatientID();
	QDir patient_dir(kExport3DFaceFolderPath + patient_id);
	if (!patient_dir.exists())
	{
		patient_dir.mkpath(patient_dir.absolutePath());
	}

	return patient_dir.absolutePath() + "/";
}

QString CW3TabMgr::GetFileNameForExport3DFace()
{
	QFileInfo info(m_pVTOSTO->m_trdFilePath);
	QString export_file_name = info.completeBaseName();

	QDate date = QDate::currentDate();
	QTime time = QTime::currentTime();
	QString year, month, day, hour, minute, second, msec;
	year.sprintf("%04d", date.year());
	month.sprintf("%02d", date.month());
	day.sprintf("%02d", date.day());
	hour.sprintf("%02d", time.hour());
	minute.sprintf("%02d", time.minute());
	second.sprintf("%02d", time.second());
	msec.sprintf("%03d", time.msec());
	export_file_name =
		export_file_name + "_" +
		year + month + day +
		hour + minute + second + msec;

	return export_file_name;
}
#endif
#ifndef WILL3D_VIEWER
void CW3TabMgr::slotLoadProjectFromFile()
{
	if (project_file_path_.isEmpty())
	{
		return;
	}

	// 占시곤옙 체크占쌔쇽옙 占쏙옙占쏙옙占심몌옙占쏙옙 LoadMainVolumeDataFromProject 占쏙옙占쏙옙占쏙옙占?占싼깍옙占?second 占쏙옙)
	project_in_.reset(new ProjectIO(project::Purpose::LOAD, project_file_path_));
	std::unique_ptr<ProjectIOFile> pio_file;
	project_in_->MoveFileTabIO(&pio_file);
	if (!pio_file.get())
	{
		return;
	}

	bool result = false;
#if 0
	result = m_pFILEtab->ImportMainVolumeFromProject(*pio_file.get();
#else
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	auto future = QtConcurrent::run(this, &CW3TabMgr::LoadMainVolumeDataFromProject, pio_file.get());
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	result = future.result();
#endif

	if (!result)
	{
		return;
	}

	slotLoadDicomFromLoader();
	slotLoadProject(project_file_path_);
}
#endif
#ifdef WILL3D_VIEWER
void CW3TabMgr::ImportCDViewerData()
{
	bool result = m_pFILEtab->ImportDICOM();
	if (!result)
	{
		QCoreApplication::exit();
		return;
	}

	slotLoadDicomFromLoader();

	//===========================
	//20250123 LIN 
	QDir current_dir = QDir::current();
	if (!current_dir.cdUp()) {
		qDebug() << "Failed to access parent directory!";
	}
	QString suffix = "*.w3d";
	QStringList filters;
	filters << suffix;
	QFileInfoList file_info_list = current_dir.entryInfoList(filters, QDir::Files);
	QStringList file_paths;
	for (const QFileInfo &file_info : file_info_list)
	{
		file_paths << file_info.absoluteFilePath();
	}
	if (!file_paths.length()) {
		return;
	} 
	else {
		//?좎씪??w3d?뚯씪留??꾩슂?댁꽌
		project_file_path_ = file_paths[0];
		//slotLoadProject?⑥닔 洹몃?濡?.
		emit sigActiveLoadProject();
		loadProjectForPanoEngineContent(project_file_path_);
		emit sigDoneLoadProject();
		//============================
	}


}
#endif

bool CW3TabMgr::SaveScreenshot(const QImage& image)
{
	try
	{
		QString file_path;
		bool save_image_result = SaveImage(image, file_path);

		if (m_pREPORTtab)
		{
			m_pREPORTtab->addThumbnail(file_path);
		}

		if (save_image_result)
		{
			CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_60(), CW3MessageBox::Information);
			message_box.exec();
		}
		else
		{
			throw std::runtime_error("save image failed.");
		}

		return true;
	}
	catch (std::runtime_error& e)
	{
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR, "CW3TabMgr::slotCaptureScreenShot: " + err_msg);

		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_61(), CW3MessageBox::Critical);
		msgBox.exec();

		return false;
	}
}

bool CW3TabMgr::SaveImage(const QImage& image, QString& saved_path)
{
	CW3Image3D* vol = m_pgVREngine->getVol(0);

	if (vol == nullptr)
	{
		throw std::runtime_error("not set data");
	}
	CW3ImageHeader* header = vol->getHeader();
	QString patient_id = header->getPatientID();
	QString series_date = header->getSeriesDate();
	QString series_time = header->getSeriesTime();

	QString folder_path = QString("%1").arg(GlobalPreferences::GetInstance()->preferences_.general.files.capture_path);

	if (GlobalPreferences::GetInstance()->preferences_.general.files.capture_path_with_patient_folder)
	{
		folder_path += QString("/%1").arg(patient_id);
	}

	QDir dir(folder_path);
	if (!dir.exists())
	{
		dir.mkpath(".");
	}
	int index = 0;
	QString file_path;
	QString file_path_for_willmaster;

	QString image_format = GlobalPreferences::GetInstance()->preferences_.capture.image_format;

	while (1)
	{
		QString file_name;
		if (GlobalPreferences::GetInstance()->preferences_.general.files.capture_path_with_patient_folder)
		{
			file_name = QString("/%1_%2_%3")
				.arg(series_date)
				.arg(series_time)
				.arg(index++);
		}
		else
		{
			file_name = QString("/%1_%2_%3_%4")
				.arg(patient_id)
				.arg(series_date)
				.arg(series_time)
				.arg(index++);
		}

		file_path = folder_path + file_name + "." + image_format;

		if (m_bSaveForExternalProgram)
		{
			file_path_for_willmaster = m_strOutputPath + file_name + ".bmp";
		}

		if (QFile::exists(file_path))
		{
			continue;
		}
		else
		{
			break;
		}
	}

	QImageWriter img_writer(file_path, image_format.toStdString().c_str());
	bool save_image_result = img_writer.write(image);
	if (m_bSaveForExternalProgram)
	{
		QImageWriter img_writer_for_willmaster(file_path_for_willmaster, "bmp");
		img_writer_for_willmaster.write(image);

		QFile info(m_strOutputPath + "/PatientInfo.ini");
		if (info.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QTextStream out(&info);

			QString patient_name = header->getPatientName();
			QString patient_birth_date = header->getPatientBirthDate();
			QString patient_sex = header->getPatientSex();
			out << "[Patient Information]" << endl;
			out << "Patient ID = " << patient_id << endl;
			out << "Patient Name = " << patient_name << endl;
			out << "Patient Birth of Date = " << patient_birth_date << endl;
			out << "Patient Sex = " << patient_sex << endl;

			info.close();
		}
	}

	saved_path = file_path;
	return save_image_result;
}

QSizeF CW3TabMgr::GetOriginalScreenSize(const unsigned int screen_number)
{
	// Get original screen size
	int screen_top_left_pos = qApp->desktop()->screen(screen_number)->pos().x();
	QSizeF screen_size;

	DWORD display_number = 0;
	DISPLAY_DEVICE display_device;
	DEVMODE default_mode;

	// initialize display_device
	ZeroMemory(&display_device, sizeof(display_device));
	display_device.cb = sizeof(display_device);

	// get all display devices
	while (EnumDisplayDevices(NULL, display_number, &display_device, 0))
	{
		ZeroMemory(&default_mode, sizeof(DEVMODE));
		default_mode.dmSize = sizeof(DEVMODE);
		if (!EnumDisplaySettings(display_device.DeviceName, ENUM_REGISTRY_SETTINGS, &default_mode))
		{
			break;
		}

#if 0
		qDebug() << "1 display_device.DeviceName :" << QString::fromUtf16(display_device.DeviceName);
		qDebug() << "1 display_device.DeviceString :" << QString::fromUtf16(display_device.DeviceString);
		qDebug() << "1 display_device.DeviceKey :" << QString::fromUtf16(display_device.DeviceKey);
		qDebug() << "1 display_device.DeviceID :" << QString::fromUtf16(display_device.DeviceID);

		WCHAR GraphicCard[128];
		ZeroMemory(GraphicCard, sizeof(GraphicCard));
		wcscpy(GraphicCard, display_device.DeviceName);

		EnumDisplayDevices(GraphicCard, 0, &display_device, 0);

		qDebug() << "2 display_device.DeviceName :" << QString::fromUtf16(display_device.DeviceName);
		qDebug() << "2 display_device.DeviceString :" << QString::fromUtf16(display_device.DeviceString);
		qDebug() << "2 display_device.DeviceKey :" << QString::fromUtf16(display_device.DeviceKey);
		qDebug() << "2 display_device.DeviceID :" << QString::fromUtf16(display_device.DeviceID);
#endif

		qDebug() << "dmOrientation :" << default_mode.dmOrientation;
		qDebug() << "dmPosition :" << default_mode.dmPosition.x << default_mode.dmPosition.y;

		// Reinit display_device just to be extra clean
		ZeroMemory(&display_device, sizeof(display_device));
		display_device.cb = sizeof(display_device);
		display_number++;

		qDebug() << "screen_top_left_pos :" << screen_top_left_pos;
		qDebug() << "default_mode.dmPosition.x :" << default_mode.dmPosition.x;
		qDebug() << "screen_size :" << screen_size.width() << screen_size.height();
		qDebug() << "default_mode.dmPelsSize :" << default_mode.dmPelsWidth << default_mode.dmPelsHeight;

		if (default_mode.dmPelsWidth <= 0 || default_mode.dmPelsHeight <= 0)
		{
			continue;
		}

		if (screen_top_left_pos == default_mode.dmOrientation)
		{
			screen_size.setWidth(static_cast<float>(default_mode.dmPelsWidth));
			screen_size.setHeight(static_cast<float>(default_mode.dmPelsHeight));
			break;
		}
	} // end while for all display devices

	return screen_size;
}

void CW3TabMgr::slotContinuousCapture(QWidget* source)
{
	if (!source)
	{
		return;
	}

	continuous_capture_source_ = source;

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	if (continuous_capture_timer_ && continuous_capture_timer_->isActive())
	{
		continuous_capture_timer_->stop();
		SAFE_DELETE_OBJECT(continuous_capture_timer_);
	}
	continuous_capture_timer_ = new QTimer(this);
	connect(continuous_capture_timer_, SIGNAL(timeout()), this, SLOT(slotContinuousCapture()));

	continuous_capture_timer_->setInterval(500);
	continuous_capture_timer_->start();

	progress->exec();
}

void CW3TabMgr::slotContinuousCapture()
{
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	BaseTab* current_tab = casted_tabs_[tab_type_];
	if (!continuous_capture_source_ || current_tab != m_pTMJtab)
	{
		progress->hide();
		continuous_capture_timer_->stop();
		SAFE_DELETE_OBJECT(continuous_capture_timer_);
		return;
	}

	progress->hide(false);
	QImage image = GetScreenshot(continuous_capture_source_);
	progress->show();
	if (image.isNull())
	{
		return;
	}

	if (GlobalPreferences::GetInstance()->preferences_.capture.include_dicom_info)
	{
		DrawDicomInfoToImage(&image);
	}

	SaveImage(image);

	bool slide_as_set = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set;
	GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set = true;
	if (!m_pTMJtab->ShiftContinuousView(continuous_capture_source_))
	{
		progress->hide();
		continuous_capture_timer_->stop();
		SAFE_DELETE_OBJECT(continuous_capture_timer_);

		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_60(), CW3MessageBox::Information);
		message_box.exec();
	}
	GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set = slide_as_set;
	QApplication::processEvents();
}

#ifdef WILL3D_EUROPE
void CW3TabMgr::slotShowButtonListDialog(const QPoint& global_pos, const int mpr_type /*= -1*/)
{
	if (button_list_dialog_ == nullptr)
	{
		button_list_dialog_ = new ButtonListDialog();
		connect(button_list_dialog_, &ButtonListDialog::sigSyncControlButtonOut, this, &CW3TabMgr::slotSyncControlButtonOut);
		connect(button_list_dialog_, &ButtonListDialog::sigButtonToggle, this, &CW3TabMgr::slotButtonToggle);
	}
	
	button_list_dialog_->InitButtonSetting(tab_type_, mpr_type);
	button_list_dialog_->move(global_pos.x(), global_pos.y());
	button_list_dialog_->exec();
}

void CW3TabMgr::slotSyncControlButtonOut()
{
	BaseTab* cur_tab = casted_tabs_[tab_type_];
	cur_tab->SyncControlButtonOut();
}

void CW3TabMgr::slotButtonToggle(const int id, const bool toggle)
{
	if (id == 0) // ButtonID::BTN_WINDOWING 0
	{
		if (toggle)
		{
			slotCommonToolOnOff(common::CommonToolTypeOnOff::V_LIGHT);
		}
		else
		{
			slotCommonToolOnOff(common::CommonToolTypeOnOff::NONE);
		}
	}
	else if (id == 1) // ButtonID::BTN_HIDE_UI 1
	{
		if (toggle)
		{
			int grid_id = 2;
			int state = button_list_dialog_->GetButtonState(grid_id);
			if (state == 1)
			{
				button_list_dialog_->SetCheckedButton(grid_id, false);
			}
		}
		casted_tabs_[tab_type_]->SetCommonToolOnce(common::CommonToolTypeOnce::V_HIDE_UI, toggle);		
	}
	else if (id == 2) // ButtonID::BTN_GRID_ONOFF 2
	{
		if (toggle)
		{
			int ui_hide_id = 1;
			int state = button_list_dialog_->GetButtonState(ui_hide_id);
			if (state == 1)
			{
				button_list_dialog_->SetCheckedButton(ui_hide_id, false);
			}
		}
		casted_tabs_[tab_type_]->SetCommonToolOnce(common::CommonToolTypeOnce::V_GRID, toggle);
	}
	else if (id == 3 && toggle) // ButtonID::BTN_IMPALNT_ANGLE 3
	{
		slotShowImplantAngle();
	}
	else if (id == 4 && toggle) // ButtonID::BTN_CD_USB_EXPORT 4
	{
		slotFileTool(CommonToolTypeFile::CDEXPORT);
	}
	else if (id == 5 && toggle) // ButtonID::BTN_STL_EXPORT 5
	{
		if (tab_type_ == TabType::TAB_MPR)
		{
			m_pMPRtab->TaskSTLExportDialogOn();
		}
	}
	else if (id == 6 && toggle) // ButtonID::BTN_MEASURE_LIST 6
	{
		slotCommonToolMeasureListOn();
	}
	else if (id == 7 && toggle) // ButtonID::BTN_LIGHTBOX 7
	{
		//button_list_dialog_->setVisible(false);
		//m_pMPRtab->LightboxOn();
	}
	
	BaseTab* cur_tab = casted_tabs_[tab_type_];
	cur_tab->SyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3TabMgr::slotUpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number)
{
	m_pPANOtab->SetInitArchFromMPR(true);

	m_pPANOtab->ClearArch(arch_type);
	pano_engine_->SetCurrentArchType(arch_type);

	emit sigChangeTab(TabType::TAB_PANORAMA);
#if 1
	QApplication::processEvents();
#endif
	m_pPANOtab->UpdateArchFromMPR(arch_type, points, orientation_matrix, slice_number);

	m_pPANOtab->SetInitArchFromMPR(false);
}

void CW3TabMgr::slotGetMPRPlaneInfo(const MPRViewType mpr_view_type)
{
	if (tab_type_ != TabType::TAB_MPR)
	{
		return;
	}

	QStringList view_list = m_pMPRtab->GetViewList();
	if (view_list.size() < 2)
	{
		return; //cross_section;
	}

	m_pMPRtab->EmitSendMPRPlaneInfo(mpr_view_type);
}

void CW3TabMgr::slotCreateDCMFiles_uchar(unsigned char* data, const QString& middle_path, const int instance_num, const int rows, const int columns)
{
	QString path = QApplication::applicationDirPath() + kFolderPath;
	QDir path_dir(path);
	if (!path_dir.exists())
	{
		path_dir.mkpath(".");
	}

	QString full_path = path + middle_path;

	CW3DicomIO dicom_io;
	dicom_io.CreateSeriesInstanceUID();
	dicom_io.WriteDicomRGB(full_path.toStdString(), data, instance_num, rows, columns);
}

void CW3TabMgr::slotCreateDCMFiles_ushort(unsigned short* data, const QString& middle_path, const int instance_num, const int rows, const int columns)
{
	QString path = QApplication::applicationDirPath() + kFolderPath;
	QDir path_dir(path);
	if (!path_dir.exists())
	{
		path_dir.mkpath(".");
	}

	QString full_path = path + middle_path;

	CW3DicomIO dicom_io;
	dicom_io.CreateSeriesInstanceUID();
	dicom_io.WriteDicom(full_path.toStdString(), data, instance_num, rows, columns);
}

#ifndef WILL3D_VIEWER
void CW3TabMgr::ShowPACSDialog()
{
	if (tab_type_ != TabType::TAB_MPR && tab_type_ != TabType::TAB_PANORAMA)
	{
		return;
	}

	float ori_shifted_value = 0.f;
	float ori_thickness_value = 0.f;
	if (tab_type_ == TabType::TAB_PANORAMA)
	{
		bool is_valid = ResourceContainer::GetInstance()->GetPanoResource().is_valid();
		if (!is_valid)
		{
			return;
		}

		ori_shifted_value = ResourceContainer::GetInstance()->GetPanoResource().shifted_value();
		ori_thickness_value = ResourceContainer::GetInstance()->GetPanoResource().thickness_value();
	}

	BaseTab* current_tab = casted_tabs_[tab_type_];
	QStringList view_list = current_tab->GetViewList();

	PacsDialog pcas_dialog(tab_type_, view_list);

	connect(&pcas_dialog, &PacsDialog::sigCreateDCMFiles_ushort, this, &CW3TabMgr::slotCreateDCMFiles_ushort);
	connect(&pcas_dialog, &PacsDialog::sigCreateDCMFiles_uchar, this, &CW3TabMgr::slotCreateDCMFiles_uchar);
	connect(&pcas_dialog, &PacsDialog::sigPACSSend, this, &CW3TabMgr::PACSSend);

	if (tab_type_ == TabType::TAB_MPR)
	{
		if (view_list.size() > 1)
		{
			connect(&pcas_dialog, &PacsDialog::sigChangeMPRType, this, &CW3TabMgr::slotGetMPRPlaneInfo);
			connect(this, &CW3TabMgr::sigSendMPRPlaneInfo, &pcas_dialog, &PacsDialog::slotSetMPRPlaneInfo);
		}
		else
		{
			connect(&pcas_dialog, &PacsDialog::sigRequestCreateDCMFiles, m_pMPRtab, &CW3MPRtab::slotRequestedCreateLightBoxDCMFiles);
			connect(m_pMPRtab, &CW3MPRtab::sigCreateDCMFiles_ushort, this, &CW3TabMgr::slotCreateDCMFiles_ushort);
			connect(m_pMPRtab, &CW3MPRtab::sigCreateDCMFiles_uchar, this, &CW3TabMgr::slotCreateDCMFiles_uchar);

			int filter = 0;
			int thickness = 0;
			m_pMPRtab->RequestedGetLightBoxViewInfo(filter, thickness);
			pcas_dialog.SetLightBoxViewInfo(filter, thickness);
		}

		pcas_dialog.EmitMPRViewInitialize(m_pMPRengine);
	}
	else if (tab_type_ == TabType::TAB_PANORAMA)
	{

		connect(&pcas_dialog, &PacsDialog::sigRequestGetPanoCrossSectionViewInfo, m_pPANOtab, &CW3PANOtab::slotRequestedGetPanoCrossSectionViewInfo);
		connect(&pcas_dialog, &PacsDialog::sigRequestCreateDCMFiles, m_pPANOtab, &CW3PANOtab::slotRequestedCreateCrossSectionDCMFiles);
		connect(m_pPANOtab, &CW3PANOtab::sigCreateDCMFiles_ushort, this, &CW3TabMgr::slotCreateDCMFiles_ushort);
		connect(m_pPANOtab, &CW3PANOtab::sigCreateDCMFiles_uchar, this, &CW3TabMgr::slotCreateDCMFiles_uchar);

		connect(&pcas_dialog, &PacsDialog::sigPanoUpdateThickness, [&](int thickness)
		{
			if (pano_engine_->SetPanoThickness(thickness))
			{
				pcas_dialog.PanoUpdate();
			}
		});

		connect(&pcas_dialog, &PacsDialog::sigPanoUpdate, [=](int value)
		{
			const auto& pano_res = ResourceContainer::GetInstance()->GetPanoResource();
			int origin_z_pos = pano_res.pano_3d_depth() / 2;
			int shifted_value = value - origin_z_pos;

			pano_engine_->SetPACSPanoShiftedValue(shifted_value);
			pano_engine_->ReconPanoramaPlane();
			pano_engine_->ReconPanoramaNerveMask();
			pano_engine_->ReconPanoramaImplantMask(1.f);
		});

		pcas_dialog.EmitPanoViewInitialize();
	}
	else
	{
		return;
	}

	QWidget* button = common_task_tool_->GetFileButton(CommonTaskTool::FileID::PACS);

	QRect rect = button->rect();
	QPoint pos = button->mapToGlobal(QPoint(0, 0)) + QPoint(0, rect.height());

	pcas_dialog.move(pos);
	pcas_dialog.exec();

	if (tab_type_ == TabType::TAB_PANORAMA)
	{
		pano_engine_->SetPanoThickness(ori_thickness_value);
		pano_engine_->SetPanoramaShiftedValue(ori_shifted_value);
		pano_engine_->ReconPanoramaPlane();
		pano_engine_->ReconPanoramaNerveMask();
		pano_engine_->ReconPanoramaImplantMask(1.f);
	}
}

void CW3TabMgr::PACSSend(const QStringList& server_info)
{
	if (server_info.isEmpty())
	{
		return;
	}

	QString path = QApplication::applicationDirPath() + kFolderPath;

	QStringList filters;
	filters += "*.dcm";
	QDirIterator iter_dir(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
	QList<QString> list;
	while (iter_dir.hasNext())
	{
		iter_dir.next();
		QString name = path + iter_dir.fileName();
		list.push_back(name);
	}

	QString ae_tile = server_info[1];
	QString ip_address = server_info[2];
	int port = server_info[3].toInt();

	if (!list.empty())
	{
		int result = DicomSend::Do(ip_address, port, ae_tile, list);

		QString error_msg;
		if (result >= 1 && result <= 19)
		{
			error_msg = "general error";
		}
		if (result >= 20 && result <= 39)
		{
			error_msg = "input file error";
		}
		else if (result >= 40 && result <= 50)
		{
			error_msg = "output file error";
		}
		else if (result >= 60 && result <= 79)
		{
			error_msg = "network error";
		}
		else if (result >= 80 && result <= 99)
		{
			error_msg = "processing error";
		}
		else if (result >= 100 && result <= 119)
		{
			error_msg = "user-defined error";
		}

		if (error_msg.isEmpty() == false)
		{
			error_msg += "error number : " + QString::number(result);
			common::Logger::instance()->Print(common::LogType::ERR, error_msg.toStdString());
			CW3MessageBox msg_box("Will3D", error_msg, CW3MessageBox::Critical);
			msg_box.exec();
		}
	}

	CW3DicomIO dicom_io;
	dicom_io.InitSeriesInstanceUID();
}
#endif
