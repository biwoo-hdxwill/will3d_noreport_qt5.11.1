#pragma once
/*=========================================================================

File:			class ProjectIOImplant
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-16
Last modify:	2016-07-16

=========================================================================*/
#include <memory>
#include "datatypes.h"
#include "w3projectio_global.h"
namespace H5 {
class H5File;
}
class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOImplant {
public:
	ProjectIOImplant(const project::Purpose& purpose,
					 const std::shared_ptr<H5::H5File>& file);
	~ProjectIOImplant();

	ProjectIOImplant(const ProjectIOImplant&) = delete;
	ProjectIOImplant& operator=(const ProjectIOImplant&) = delete;

	enum class ViewType {
		IMP_PANO,
		IMP_ARCH,
		IMP_CS,
		IMP_SAGITTAL,
		IMP_IMPLANT3D
	};

public:
	void InitImplantTab();
	bool IsInitImplant();
	void InitializeView(ProjectIOImplant::ViewType view_type, int cs_view_id = 0);
	ProjectIOView& GetViewIO();

	void SaveCSAngle(float degree);

	void LoadCSAngle(float& degree);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
	int curr_view_id_;
};

