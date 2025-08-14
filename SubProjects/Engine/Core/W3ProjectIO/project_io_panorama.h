#pragma once
/*=========================================================================

File:			class ProjectIOPanorama
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-16
Last modify:	2016-07-16

=========================================================================*/
#include <memory>

#include "../../Common/Common/W3Enum.h"
#include "datatypes.h"
#include "w3projectio_global.h"
namespace H5 {
class H5File;
}
class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOPanorama {
public:
	ProjectIOPanorama(const project::Purpose& purpose,
					  const std::shared_ptr<H5::H5File>& file);
	~ProjectIOPanorama();

	ProjectIOPanorama(const ProjectIOPanorama&) = delete;
	ProjectIOPanorama& operator=(const ProjectIOPanorama&) = delete;

	enum class ViewType {
		PANO_PANO,
		PANO_ARCH,
		PANO_CS,
	};

public:
	void InitPanoTab();
	bool IsInitPano();
	void InitializeView(ProjectIOPanorama::ViewType view_type, int cs_view_id = 0);
	ProjectIOView& GetViewIO();

	void SaveOrientDegrees(const ArchTypeID& arch_type, int r, int i, int a);
	void SaveCSAngle(float degree);
	void SaveCSInterval(float interval);
	void SaveCSThickness(float thickness);
	void SaveArchRange(float range);
	void SaveArchThickness(float thickness);

	void LoadOrientDegrees(const ArchTypeID& arch_type, int& r, int& i, int& a);
	void LoadCSAngle(float& degree);
	bool LoadCSInterval(float& interval);
	bool LoadCSThickness(float& thickness);
	bool LoadArchRange(float& range);
	bool LoadArchThickness(float& thickness);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
	int curr_view_id_;
};

