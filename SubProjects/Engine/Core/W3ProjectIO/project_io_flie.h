#pragma once
/*=========================================================================

File:			class ProjectIOFile
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-11
Last modify:	2016-07-11

=========================================================================*/
#include <memory>
#include "datatypes.h"
#include "w3projectio_global.h"

class CW3Image3D;
namespace H5 {
class H5File;
}

namespace project {
typedef struct _LoadVolInfo LoadVolInfo;
}

class W3PROJECTIO_EXPORT ProjectIOFile {
public:
	ProjectIOFile(const project::Purpose& purpose,
				  const std::shared_ptr<H5::H5File>& file);
	~ProjectIOFile();

	ProjectIOFile(const ProjectIOFile&) = delete;
	ProjectIOFile& operator=(const ProjectIOFile&) = delete;

public:
	void SaveMainVolume(CW3Image3D* vol);
	void LoadMainVolume(CW3Image3D*& vol);
	void SaveSecondVolume(CW3Image3D* vol);
	void LoadSecondVolume(CW3Image3D*& vol);

	void SaveVolume(CW3Image3D* vol, const int id);
	void LoadVolume(CW3Image3D*& vol, const int id);

	void SaveVolumeInfo(CW3Image3D* vol);
	void LoadVolumeInfo(project::LoadVolInfo& vol_info);

private:
	std::shared_ptr<H5::H5File> file_;
};

