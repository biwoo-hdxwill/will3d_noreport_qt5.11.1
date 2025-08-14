#pragma once
/*=========================================================================

File:			class ProjectIOMPREngine
Language:		C++11
Library:        Qt 5.8.0
Author:			JUNG DAE GUN
First date:		2019-12-02
Last modify:	2019-12-02

=========================================================================*/

#include "w3projectio_global.h"

#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif

#include "datatypes.h"

namespace H5
{
	class H5File;
}

class W3PROJECTIO_EXPORT ProjectIOMPREngine
{
public:
	ProjectIOMPREngine(const project::Purpose& purpose, const std::shared_ptr<H5::H5File>& file);
	~ProjectIOMPREngine();

	ProjectIOMPREngine(const ProjectIOMPREngine&) = delete;
	ProjectIOMPREngine& operator=(const ProjectIOMPREngine&) = delete;

	void SaveCenterInVol(const int volume_id, const glm::vec3& org, const glm::vec3& mpr, const glm::vec3& si);
	void LoadCenterInVol(const int volume_id, glm::vec3& org, glm::vec3& mpr, glm::vec3& si);

private:
	std::shared_ptr<H5::H5File> file_;
};
