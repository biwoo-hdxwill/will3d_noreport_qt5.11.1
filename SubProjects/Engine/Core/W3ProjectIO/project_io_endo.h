#pragma once
/*=========================================================================

File:			class ProjectIOEndo
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-22
Last modify:	2016-07-22

=========================================================================*/
#include <vector>
#include <memory>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "../../Common/GLfunctions/W3GLTypes.h"
#include "datatypes.h"
#include "w3projectio_global.h"
class ProjectIOView;

namespace H5 {
class H5File;
}

class W3PROJECTIO_EXPORT ProjectIOEndo {
public:
	ProjectIOEndo(const project::Purpose& purpose,
				  const std::shared_ptr<H5::H5File>& file);
	~ProjectIOEndo();

	ProjectIOEndo(const ProjectIOEndo&) = delete;
	ProjectIOEndo& operator=(const ProjectIOEndo&) = delete;

	enum class ViewType {
		ENDO,
		MODIFY,
		SLICE,
		SAGITTAL
	};

public:
	void InitEndoTab();
	bool IsInit();
	void InitializeView(ProjectIOEndo::ViewType view_type);
	ProjectIOView & GetViewIO();

	void SaveCurrPathNum(const int& number);
	void SavePath(const int& id, const std::vector<glm::vec3>& path);
	void SaveAirway(const std::vector<tri_STL>& airway, const double& airway_size);

	void LoadCurrPathNum(int& number);
	void LoadPath(const int& id, std::vector<glm::vec3>& path);
	void LoadAirway(std::vector<tri_STL>& airway, double& airway_size);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
};

