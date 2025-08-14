#pragma once
/*=========================================================================

File:			class ProjectIOFace
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-16
Last modify:	2016-07-16

=========================================================================*/
#include <memory>
#include <vector>
#include <qpoint.h>
#include "datatypes.h"
#include "w3projectio_global.h"

namespace H5 {
class H5File;
}
class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOFace {
public:
	ProjectIOFace(const project::Purpose& purpose,
				  const std::shared_ptr<H5::H5File>& file);
	~ProjectIOFace();

	ProjectIOFace(const ProjectIOFace&) = delete;
	ProjectIOFace& operator=(const ProjectIOFace&) = delete;

	enum class ViewType {
		FACE_MESH,
		PHOTO,
		FACE_BEFORE
	};

public:
	void InitFaceTab();
	bool IsInit();
	void InitializeView(ProjectIOFace::ViewType view_type);
	ProjectIOView& GetViewIO();

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
};

