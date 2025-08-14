#include "project_io.h"
#include <H5Cpp.h>
#include <fstream>
#include <qdir.h>

#include "../../Common/Common/W3Logger.h"
#include "project_path_info.h"
#include "project_io_general.h"
#include "project_io_flie.h"
#include "project_io_mpr.h"
#include "project_io_panorama.h"
#include "project_io_implant.h"
#include "project_io_tmj.h"
#include "project_io_face.h"
#include "project_io_ceph.h"
#include "project_io_si.h"
#include "project_io_endo.h"

#include "project_io_measure_resource.h"
#include "project_io_mpr_engine.h"
#include "project_io_pano_engine.h"
#include "project_io_vtosto.h"

using namespace H5;
using namespace project;

namespace {
const std::string kProjTmpPath = "./proj_tmp/";
const std::string kTmpFileName = "temp.h5";
} // end of namespace

ProjectIO::ProjectIO(const project::Purpose& purpose, const QString& path) : purpose_(purpose) {
	if (purpose_ == project::Purpose::SAVE) {
		const auto raw_str = path.left(path.lastIndexOf(".")).toLocal8Bit();
		save_file_name_ = raw_str;
		file_ = std::make_shared<H5File>(save_file_name_ + ".h5", H5F_ACC_TRUNC);
		file_->createGroup(group::kTab);
		file_->createGroup(group::kResource);
	} else {
		std::string hdf_path;
		CopyProjFile(path, hdf_path);
		file_ = std::make_shared<H5File>(hdf_path, H5F_ACC_RDWR);
	}

	general_io_ = std::make_unique<ProjectIOGeneral>(purpose_, file_);
	file_io_ = std::make_unique<ProjectIOFile>(purpose_, file_);
	mpr_io_ = std::make_unique<ProjectIOMPR>(purpose_, file_);
	pano_io_ = std::make_unique<ProjectIOPanorama>(purpose_, file_);
	implant_io_ = std::make_unique<ProjectIOImplant>(purpose_, file_);
	tmj_io_ = std::make_unique<ProjectIOTMJ>(purpose_, file_);
	ceph_io_ = std::make_unique<ProjectIOCeph>(purpose_, file_);
	face_io_ = std::make_unique<ProjectIOFace>(purpose_, file_);
	si_io_ = std::make_unique<ProjectIOSI>(purpose_, file_);
	endo_io_ = std::make_unique<ProjectIOEndo>(purpose_, file_);
	mpr_engine_io_ = std::make_unique<ProjectIOMPREngine>(purpose_, file_);
	pano_engine_io_ = std::make_unique<ProjectIOPanoEngine>(purpose_, file_);
	vtosto_io_ = std::make_unique<ProjectIOVTOSTO>(purpose_, file_);
	measure_io_ = std::make_unique<ProjectIOMeasureResource>(purpose_, file_);
}

void ProjectIO::CopyProjFile(const QString& proj_path, std::string& hdf_path) {
	if (!QDir(kProjTmpPath.c_str()).exists())
		QDir().mkdir(kProjTmpPath.c_str());

	QFileInfo file_info(proj_path);
	QString file_name_qstr(file_info.fileName());
	const auto raw_str = file_name_qstr.left(file_name_qstr.lastIndexOf(".")).toLocal8Bit();
	std::string file_name = raw_str;

	hdf_path = kProjTmpPath + kTmpFileName;
	std::string proj_path_name = proj_path.toLocal8Bit();
	std::ifstream  src(proj_path_name, std::ios::binary);
	std::ofstream  dst(hdf_path, std::ios::binary);

	common::Logger::instance()->Print(common::LogType::INF, "ProjectIO::CopyProjFile : File path");
	common::Logger::instance()->Print(common::LogType::INF, proj_path_name);
	common::Logger::instance()->Print(common::LogType::INF, hdf_path);
	dst << src.rdbuf();
}

#include <QDebug>
ProjectIO::~ProjectIO() {
	general_io_.reset(nullptr);
	file_io_.reset(nullptr);
	mpr_io_.reset(nullptr);
	pano_io_.reset(nullptr);
	implant_io_.reset(nullptr);
	tmj_io_.reset(nullptr);
	ceph_io_.reset(nullptr);
	face_io_.reset(nullptr);
	si_io_.reset(nullptr);
	endo_io_.reset(nullptr);
	mpr_engine_io_.reset(nullptr);
	pano_engine_io_.reset(nullptr);
	vtosto_io_.reset(nullptr);
	file_->close();
	
	if (purpose_ == project::Purpose::SAVE) {
		std::string proj_file_name = save_file_name_ + ".w3d";
		std::string hdf_name = save_file_name_ + ".h5";
		int result = rename(hdf_name.c_str(), proj_file_name.c_str());

		if (result != 0) {
			common::Logger::instance()->Print(common::LogType::ERR,
											  "Project IO : File format conversion failed");

//20250214LIN Viewer인 경우에는 기존 .w3d파일을 존재되면 똑같은 이름으로 .w3d파일로 rename하면 result 0이 아닐 거다. 
//그래서 기존 .w3d파일을 삭제하고 다시 생성.
//#ifdef WILL3D_VIEWER
			if (std::ifstream(proj_file_name).good()) {
				if (std::remove(proj_file_name.c_str()) != 0) {
					common::Logger::instance()->Print(common::LogType::ERR,
						"Project IO : Cannot delete original file");
				}
			}

			std::string proj_file_name = save_file_name_ + ".w3d";
			std::string hdf_name = save_file_name_ + ".h5";
			int result = std::rename(hdf_name.c_str(), proj_file_name.c_str());

			if (result != 0) {
				common::Logger::instance()->Print(common::LogType::ERR,
					"Project IO : Failed to rename file");
			}
//#endif
		}
	} else {
		RemoveProjFile();
	}
}

void ProjectIO::MoveGeneralIO(std::unique_ptr<ProjectIOGeneral>* general_io) {
	*general_io = std::move(general_io_);
}

void ProjectIO::MoveFileTabIO(std::unique_ptr<ProjectIOFile>* file_io) {
	*file_io = std::move(file_io_);
}

void ProjectIO::MoveMPRTabIO(std::unique_ptr<ProjectIOMPR>* mpr_io) {
	*mpr_io = std::move(mpr_io_);
}

void ProjectIO::MovePanoTabIO(std::unique_ptr<ProjectIOPanorama>* pano_io) {
	*pano_io = std::move(pano_io_);
}

void ProjectIO::MoveImplantTabIO(std::unique_ptr<ProjectIOImplant>* implant_io) {
	*implant_io = std::move(implant_io_);
}

void ProjectIO::MoveTMJTabIO(std::unique_ptr<ProjectIOTMJ>* tmj_io) {
	*tmj_io = std::move(tmj_io_);
}

void ProjectIO::MoveCephTabIO(std::unique_ptr<ProjectIOCeph>* ceph_io) {
	*ceph_io = std::move(ceph_io_);
}

void ProjectIO::MoveFaceTabIO(std::unique_ptr<ProjectIOFace>* face_io) {
	*face_io = std::move(face_io_);
}

void ProjectIO::MoveSITabIO(std::unique_ptr<ProjectIOSI>* si_io) {
	*si_io = std::move(si_io_);
}

void ProjectIO::MoveEndoTabIO(std::unique_ptr<ProjectIOEndo>* endo_io) {
	*endo_io = std::move(endo_io_);
}

void ProjectIO::MoveMPREngineIO(std::unique_ptr<ProjectIOMPREngine>* mpr_engine_io)
{
	*mpr_engine_io = std::move(mpr_engine_io_);
}

void ProjectIO::MovePanoEngineIO(std::unique_ptr<ProjectIOPanoEngine>* pano_engine_io) {
	*pano_engine_io = std::move(pano_engine_io_);
}

void ProjectIO::MoveVTOSTOIO(std::unique_ptr<ProjectIOVTOSTO>* vtosto_io) {
	*vtosto_io = std::move(vtosto_io_);
}

void ProjectIO::MoveMeasureResourceIO(std::unique_ptr<ProjectIOMeasureResource>* measure_io) {
	*measure_io = std::move(measure_io_);
}

ProjectIOGeneral & ProjectIO::GetGeneralIO() {
	return *(general_io_.get());
}

ProjectIOFile & ProjectIO::GetFileTabIO() {
	return *(file_io_.get());
}

ProjectIOMPR & ProjectIO::GetMPRTabIO() {
	return *(mpr_io_.get());
}

ProjectIOPanorama & ProjectIO::GetPanoTabIO() {
	return *(pano_io_.get());
}

ProjectIOImplant& ProjectIO::GetImplantTabIO() {
	return *(implant_io_.get());
}

ProjectIOMPREngine & ProjectIO::GetMPREngineIO()
{
	return *(mpr_engine_io_.get());
}

ProjectIOPanoEngine & ProjectIO::GetPanoEngineIO() {
	return *(pano_engine_io_.get());
}

ProjectIOTMJ& ProjectIO::GetTMJTabIO() {
	return *(tmj_io_.get());
}

ProjectIOCeph & ProjectIO::GetCephTabIO() {
	return *(ceph_io_.get());
}

ProjectIOFace& ProjectIO::GetFaceTabIO() {
	return *(face_io_.get());
}

ProjectIOSI & ProjectIO::GetSITabIO() {
	return *(si_io_.get());
}

ProjectIOEndo & ProjectIO::GetEndoTabIO() {
	return *(endo_io_.get());
}

ProjectIOVTOSTO& ProjectIO::GetVTOSTOIO() {
	return *(vtosto_io_.get());
}

ProjectIOMeasureResource & ProjectIO::GetMeasureResourceIO() {
	return *(measure_io_.get());
}

bool ProjectIO::IsPanoProjectExists() {
	if (pano_io_.get() == nullptr)
		return false;
	return pano_io_->IsInitPano();
}

bool ProjectIO::IsImplantProjectExists() {
	if (implant_io_.get() == nullptr)
		return false;

	return implant_io_->IsInitImplant();
}

void ProjectIO::RemoveProjFile() {
	QDir dir(kProjTmpPath.c_str());
	if (dir.exists()) {
		dir.removeRecursively();
	}
}
