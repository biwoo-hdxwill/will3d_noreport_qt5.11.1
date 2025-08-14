#include "W3FILEtab.h"

#include <QSettings>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QVBoxLayout>

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3Math.h>
#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Common/Common/W3ProgressDialog.h"
#include "../../Engine/Common/Common/W3MessageBox.h"
#include "../../Engine/Common/Common/W3Theme.h"
#include "../../Engine/Common/Common/W3Memory.h"
#include "../../Engine/Common/Common/event_handler.h"
#include "../../Engine/Common/Common/event_handle_common.h"

#include "../../Engine/Resource/Resource/W3Image3D.h"
#include "../../Engine/Resource/ResContainer/resource_container.h"

#include "../../Engine/Core/W3DicomIO/W3dicomio.h"
#include "../../Engine/Core/W3DicomIO/W3DicomIOException.h"
#include "../../Engine/Core/W3CDViewerIO/cd_viewer_io.h"
#ifndef WILL3D_VIEWER
#include "../../Engine/Core/W3ProjectIO/project_io_flie.h"
#endif

using std::runtime_error;
using std::cout;
using std::endl;

CW3FILEtab::CW3FILEtab(CW3ResourceContainer *Rcontainer, QWidget* parent) 
	: BaseTab(parent) 
{

}

CW3FILEtab::~CW3FILEtab(void) 
{
	SAFE_DELETE_OBJECT(dicom_loader_);
	SAFE_DELETE_OBJECT(m_pLayout);
}

#ifndef WILL3D_VIEWER
bool CW3FILEtab::ImportMainVolumeFromProject(ProjectIOFile& in)
{
	CW3Image3D* main_volume = nullptr;
	in.LoadMainVolume(main_volume);
	if (main_volume)
	{
		volume_.reset(main_volume);
		ResourceContainer::GetInstance()->SetMainVolumeResource(volume_);

		return true;
	}
	else
	{
		CW3MessageBox message_box("Will3D", "There is not DICOM data in this project file.\nThis project file saved from 1.2.0.17 and earlier.", CW3MessageBox::Critical);
		message_box.exec();

		return false;
	}
}

void CW3FILEtab::exportProject(ProjectIOFile & out) {
	out.SaveVolumeInfo(volume_.get());

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool include_dicom_data = settings.value("PROJECT/include_dicom_data", false).toBool();
	if (include_dicom_data)
	{
		out.SaveMainVolume(volume_.get());
	}

	if (second_volume_)
	{
		out.SaveSecondVolume(second_volume_.get());
	}
}

void CW3FILEtab::importProject(ProjectIOFile & in) {
	//in.LoadVolumeInfo();
	CW3Image3D* sec_volume = nullptr;
	in.LoadSecondVolume(sec_volume);
	if (sec_volume) {
		m_bLoadSecondVolume = true;

		second_volume_.reset(sec_volume);
		ResourceContainer::GetInstance()->SetSecondVolumeResource(second_volume_);

		//EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetSecondVolume();
	}
}
#endif
void CW3FILEtab::Initialize() {
	if (BaseTab::initialized()) {
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}
	CW3Theme* theme = CW3Theme::getInstance();

	dicom_loader_ = new DicomLoader();

	SetLayout();
  file_tab_rect_ = dicom_loader_->rect();

	connections();
	BaseTab::set_initialized(true);
}

void CW3FILEtab::connections() {
	//connect(m_pDicomLoader, SIGNAL(sigLoadDicomFinished()), this, SLOT(slotLoadDicom()));

	connect(dicom_loader_, SIGNAL(sigLoadDicomFinished()),
			this, SLOT(slotLoadDicomFromLoader()));

	connect(dicom_loader_, SIGNAL(sigSetTRDFromExternalProgram(const QString, const bool)),
			this, SIGNAL(sigSetTRDFromExternalProgram(const QString, const bool)));
}
void CW3FILEtab::slotLoadDicomFromLoader() {
	if (this->getCurrentSetting() == CW3FILEtab::FIRST_VOLUME)
		emit sigLoadDicomFromLoader();
	else if (this->getCurrentSetting() == CW3FILEtab::SECOND_VOLUME)
		emit sigLoadSecondDicomFromLoader();
	else
		assert(false);
}
void CW3FILEtab::SetLayout() {
	m_pLayout = new QVBoxLayout();
	m_pLayout->addWidget(dicom_loader_);
	tab_layout_ = m_pLayout;
}

void CW3FILEtab::SetVisibleWindows(bool isVisible) {
	if (!initialized())
		return;

  if (isVisible)
  {
    dicom_loader_->resize(file_tab_rect_.width(), file_tab_rect_.height());
    dicom_loader_->raise();
  }
  else
  {
    dicom_loader_->resize(QSize(0, 0));
    dicom_loader_->lower();
  }

	//dicom_loader_->setVisible(isVisible);
}

bool CW3FILEtab::loadDicom() {
#if 1
	if (!m_bLoadSecondVolume)
	{
		ResourceContainer::GetInstance()->Reset();
	}
#endif

	QElapsedTimer timer;
	timer.start();
	common::Logger::instance()->Print(common::LogType::INF, "start loadDicom");

	try {
		CW3Point3D range = dicom_loader_->GetSelectedRange();
		QRect rectROI = dicom_loader_->GetSelectedArea();
		std::vector<QString> listFile = dicom_loader_->GetSelectedFileList();

		if (listFile.empty())
			throw runtime_error("dicom loader is not setup.");

		QStringList strFileNames;
		for (int i = range.x(); i <= range.z(); i++)
			strFileNames.append(listFile.at(i));

		m_dicomIO.setArea(rectROI);

		std::vector<std::string> fileNames(strFileNames.size());
		for (int idx = 0; idx < strFileNames.size(); ++idx) {
			QByteArray ba = strFileNames.at(idx).toLocal8Bit();
			fileNames[idx] = ba.constData();
		}

		m_dicomIO.setFiles(fileNames);

		if (!m_bLoadSecondVolume) {
			volume_.reset();
		} else {
			second_volume_.reset();
		}

		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		progress->setRange(0, 100);
		progress->setValue(0);

#if 1
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(this, &CW3FILEtab::runLoadDicom);
		watcher.setFuture(future);

		progress->exec();
		watcher.waitForFinished();

		common::Logger::instance()->Print(common::LogType::INF, "end of runLoadDicom watcher.waitForFinished");
#else
		runLoadDicom();
#endif

		if (m_errLoadDicom != "") {
			CW3MessageBox msgBox("Will3D", m_errLoadDicom, CW3MessageBox::Critical);
			msgBox.exec();
			throw runtime_error(m_errLoadDicom.toStdString().c_str());
		}

		if (!m_bLoadSecondVolume) {
			if (!volume_.get())
				return false;

			///////////////////////////////////////////////
			// v1.0.2 save/load volume range & area
			volume_->set_start_image_num(range.x());
			volume_->set_start_image_x(rectROI.x());
			volume_->set_start_image_y(rectROI.y());
			///////////////////////////////////////////////

			common::Logger::instance()->Print(common::LogType::INF, "dicom volume open");
		} else {
			if (!second_volume_)
				return false;

			common::Logger::instance()->Print(common::LogType::INF, "second dicom volume open");
		}

		//if (m_pDicomLoader->isProject())
		//emit sigLoadProject(m_pDicomLoader->getProjectPath());

		common::Logger::instance()->Print(common::LogType::INF, "end loadDicom : " + QString::number(timer.elapsed()).toStdString() + " ms");

		return true;
	} catch (const runtime_error& e) {
		m_errLoadDicom = QString(e.what());
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3FILEtab::loadDicom: " + m_errLoadDicom.toStdString());
		return false;
	}
}

void CW3FILEtab::runLoadDicom() {
	bool result = false;

	m_errLoadDicom = "";

	try {
		if (!m_bLoadSecondVolume) {
			CW3Image3D* ptr_vol = nullptr;
			result = m_dicomIO.readVolume(0, ptr_vol);

#if 0
			unsigned short** down_data = nullptr;
			int down_width = ptr_vol->width();
			int down_height = ptr_vol->height();
			int down_depth = ptr_vol->depth();
			W3::volumeDown(&down_data, ptr_vol->getData(), 2, down_width, down_height, down_depth);

			ptr_vol->resize(down_width, down_height, down_depth);
			for (int i = 0; i < down_depth; ++i)
			{
				memcpy(ptr_vol->getData()[i], down_data[i], down_width * down_height * sizeof(unsigned short));
			}
#endif

			volume_.reset(ptr_vol);
			ResourceContainer::GetInstance()->SetMainVolumeResource(volume_);
			//EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetMainVolume();
		} else {
			CW3Image3D* ptr_vol = nullptr;
			result = m_dicomIO.readVolume(0, ptr_vol);
			second_volume_.reset(ptr_vol);
			ResourceContainer::GetInstance()->SetSecondVolumeResource(second_volume_);
			//EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetSecondVolume();
		}
	} catch (const CW3DicomIOException& e) {
		m_errLoadDicom = QString(e.what());
		common::Logger::instance()->Print(common::LogType::ERR, m_errLoadDicom.toStdString());
	}

	QString log = QString("end of runLoadDicom - result : %1").arg(result);
	common::Logger::instance()->Print(common::LogType::INF, log.toStdString());
}

void CW3FILEtab::readInputFile(const QString& readFilePath) {
	dicom_loader_->ReadInputFile(readFilePath);

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool auto_open_dicom_files = settings.value("INTERFACE/auto_open_dicom_files", false).toBool();

	if (!auto_open_dicom_files)
	{
		return;
	}

	// timer for auto click open DICOM file button
	auto_open_dicom_file_timer_ = new QTimer(this);
	auto_open_dicom_file_timer_->setSingleShot(true);

	connect(auto_open_dicom_file_timer_, SIGNAL(timeout()), this, SLOT(slotAutoOpenDicomFile()));

	auto_open_dicom_file_timer_->start(1000);
}
#ifdef WILL3D_VIEWER
bool CW3FILEtab::ImportDICOM(void) {
	CW3Image3D* volume = nullptr;
#if 0
	if (!W3CDViewerIO::ImportRawDCM(volume))
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DICOM import falied");

		CW3MessageBox msgBox("Will3D", "Import failed.", CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	volume_.reset(volume);
	ResourceContainer::GetInstance()->SetMainVolumeResource(volume_);
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetSecondVolume();

	CW3MessageBox msgBox("Will3D", "Import successful.", CW3MessageBox::Information);
	msgBox.exec();
#else

#if 1
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	progress->setRange(0, 100);
	progress->setValue(0);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	common::Logger::instance()->Print(common::LogType::ERR, "start ImportRawDCM");

	auto future = QtConcurrent::run([&] { return W3CDViewerIO::ImportRawDCM(volume); });
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	common::Logger::instance()->Print(common::LogType::ERR, "end ImportRawDCM");

	bool success = future.result();
#else
	bool success = W3CDViewerIO::ImportRawDCM(volume);
#endif

	if (!success)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DICOM import falied.");

		CW3MessageBox msgBox("Will3D", "Import failed.", CW3MessageBox::Information);
		msgBox.exec();

		return false;
	}

	common::Logger::instance()->Print(common::LogType::INF, "DICOM import success.");

	//CW3MessageBox msgBox("Will3D", "Import successful.", CW3MessageBox::Information);
	//msgBox.exec();

	if (!volume)
	{
		return false;
	}

	volume_.reset(volume);
	ResourceContainer::GetInstance()->SetMainVolumeResource(volume_);
#endif

	return true;
}
#endif

/*
bool is_include_viewer, bool is_dicom_compressed,
DCMExportMethod export_method, const QString& path
*/
void CW3FILEtab::exportDICOM(QString export_path, bool is_include_viewer, bool is_dicom_compressed) 
{
	if (!volume_.get()) 
	{
		common::Logger::instance()->Print(common::LogType::ERR,"DICOM export failed : image volume :: nullptr");
		CW3MessageBox msgBox("DICOM export failed", "Image volume is not loaded.", CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	if (export_path.isEmpty())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "DICOM export failed");
		CW3MessageBox message_box("DICOM export failed", "export failed.", CW3MessageBox::Information);
		message_box.exec();
		return;
	}

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));
	const CW3Image3D& volume = *volume_; 
#if 1
	QFuture<bool> future;
	if (is_include_viewer) 
	{
		future = QtConcurrent::run([&] { return W3CDViewerIO::ExportRawDCM(volume, export_path, is_dicom_compressed, m_dicomIO.GetDICOMFiles()); });
	}
	else 
	{
		future = QtConcurrent::run([&] { return W3CDViewerIO::ExportDCMOnly(volume, export_path, m_dicomIO.GetDICOMFiles()); });
	}

	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	bool success = future.result();
#else
	bool success = W3CDViewerIO::ExportRawDCM(volume, export_path, is_dicom_compressed);
#endif
	if (!success) {
		//if (!CW3CDViewerIO::ExportRawDCM(is_dicom_compressed, export_path, volume)) { // sync case
		common::Logger::instance()->Print(common::LogType::ERR, "DICOM export failed");
		CW3MessageBox msgBox("DICOM export failed", "export failed.",
							 CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	CW3MessageBox msgBox("DICOM export success", "export success.",
						 CW3MessageBox::Information);
	msgBox.exec();
}

void CW3FILEtab::ApplyPreferences() {
	if (dicom_loader_)
		dicom_loader_->ApplyPreferences();
}

void CW3FILEtab::slotAutoOpenDicomFile()
{
	common::Logger::instance()->Print(common::LogType::INF, "Open dicom files automatically.");

	SAFE_DELETE_OBJECT(auto_open_dicom_file_timer_);

	if (!dicom_loader_)
	{
		return;
	}

#ifndef WILL3D_VIEWER
	dicom_loader_->slotOpenDicom();
#endif
}
