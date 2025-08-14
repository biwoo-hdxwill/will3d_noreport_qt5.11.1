#pragma once
/*=========================================================================

File:			class ProjectIOGeneral
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-11
Last modify:	2016-07-11

=========================================================================*/
#include <memory>
#include "datatypes.h"
#include "w3projectio_global.h"

namespace H5 {
class H5File;
}

class W3PROJECTIO_EXPORT ProjectIOGeneral {
public:
	ProjectIOGeneral(const project::Purpose& purpose,
					 const std::shared_ptr<H5::H5File>& file);
	~ProjectIOGeneral();

	ProjectIOGeneral(const ProjectIOGeneral&) = delete;
	ProjectIOGeneral& operator=(const ProjectIOGeneral&) = delete;

private:
	std::shared_ptr<H5::H5File> file_;
};

