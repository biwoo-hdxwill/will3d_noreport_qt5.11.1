#pragma once
/*=========================================================================

File:			class ProjectIO
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-09
Last modify:	2016-07-09

=========================================================================*/
#include <memory>
#include <string>

#include "datatypes.h"
#include "w3projectio_global.h"

namespace H5 {
class H5File;
}

class ProjectIOGeneral;
class ProjectIOFile;
class ProjectIOMPREngine;
class ProjectIOMPR;
class ProjectIOPanoEngine;
class ProjectIOPanorama;
class ProjectIOImplant;
class ProjectIOCeph;
class ProjectIOFace;
class ProjectIOTMJ;
class ProjectIOSI;
class ProjectIOEndo;
class ProjectIOVTOSTO;
class ProjectIOMeasureResource;

class W3PROJECTIO_EXPORT ProjectIO {
public:
	ProjectIO(const project::Purpose& purpose, const QString& path);
	~ProjectIO();

	ProjectIO(const ProjectIO&) = delete;
	ProjectIO& operator=(const ProjectIO&) = delete;

public:
	void MoveGeneralIO(std::unique_ptr<ProjectIOGeneral>* general_io);
	void MoveFileTabIO(std::unique_ptr<ProjectIOFile>* file_io);
	void MoveMPRTabIO(std::unique_ptr<ProjectIOMPR>* mpr_io);
	void MovePanoTabIO(std::unique_ptr<ProjectIOPanorama>* pano_io);
	void MoveImplantTabIO(std::unique_ptr<ProjectIOImplant>* implant_io);
	void MoveTMJTabIO(std::unique_ptr<ProjectIOTMJ>* tmj_io);
	void MoveCephTabIO(std::unique_ptr<ProjectIOCeph>* ceph_io);
	void MoveFaceTabIO(std::unique_ptr<ProjectIOFace>* face_io);
	void MoveSITabIO(std::unique_ptr<ProjectIOSI>* si_io);
	void MoveEndoTabIO(std::unique_ptr<ProjectIOEndo>* endo_io);
	void MoveMPREngineIO(std::unique_ptr<ProjectIOMPREngine>* mpr_engine_io);
	void MovePanoEngineIO(std::unique_ptr<ProjectIOPanoEngine>* pano_engine_io);
	void MoveVTOSTOIO(std::unique_ptr<ProjectIOVTOSTO>* vtosto_io);
	void MoveMeasureResourceIO(std::unique_ptr<ProjectIOMeasureResource>* measure_io);

	ProjectIOGeneral& GetGeneralIO();
	ProjectIOFile& GetFileTabIO();
	ProjectIOMPR& GetMPRTabIO();
	ProjectIOPanorama& GetPanoTabIO();
	ProjectIOImplant& GetImplantTabIO();
	ProjectIOTMJ& GetTMJTabIO();
	ProjectIOCeph& GetCephTabIO();
	ProjectIOFace& GetFaceTabIO();
	ProjectIOSI& GetSITabIO();
	ProjectIOEndo& GetEndoTabIO();
	ProjectIOMPREngine& GetMPREngineIO();
	ProjectIOPanoEngine& GetPanoEngineIO();
	ProjectIOVTOSTO& GetVTOSTOIO();
	ProjectIOMeasureResource& GetMeasureResourceIO();

	bool IsPanoProjectExists();
	bool IsImplantProjectExists();

private:
	void CopyProjFile(const QString& proj_path, std::string& hdf_path);
	void RemoveProjFile();

private:
	std::shared_ptr<H5::H5File> file_;
	project::Purpose purpose_;

	std::unique_ptr<ProjectIOGeneral> general_io_;
	std::unique_ptr<ProjectIOFile> file_io_;
	std::unique_ptr<ProjectIOMPR> mpr_io_;
	std::unique_ptr<ProjectIOPanorama> pano_io_;
	std::unique_ptr<ProjectIOImplant> implant_io_;
	std::unique_ptr<ProjectIOTMJ> tmj_io_;
	std::unique_ptr<ProjectIOCeph> ceph_io_;
	std::unique_ptr<ProjectIOFace> face_io_;
	std::unique_ptr<ProjectIOSI> si_io_;
	std::unique_ptr<ProjectIOEndo> endo_io_;
	std::unique_ptr<ProjectIOMPREngine> mpr_engine_io_;
	std::unique_ptr<ProjectIOPanoEngine> pano_engine_io_;
	std::unique_ptr<ProjectIOVTOSTO> vtosto_io_;
	std::unique_ptr<ProjectIOMeasureResource> measure_io_;

	std::string save_file_name_;
};
