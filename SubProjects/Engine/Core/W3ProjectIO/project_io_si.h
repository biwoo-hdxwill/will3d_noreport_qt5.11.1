#pragma once
/*=========================================================================

File:			class ProjectIOSI
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-22
Last modify:	2016-07-22

=========================================================================*/
#include <memory>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include "datatypes.h"
#include "w3projectio_global.h"
class ProjectIOView;

namespace H5 {
class H5File;
}

class W3PROJECTIO_EXPORT ProjectIOSI {
public:
	ProjectIOSI(const project::Purpose& purpose,
				const std::shared_ptr<H5::H5File>& file);
	~ProjectIOSI();

	ProjectIOSI(const ProjectIOSI&) = delete;
	ProjectIOSI& operator=(const ProjectIOSI&) = delete;

	enum class ViewType {
		AXIAL,
		SAGITTAL,
		CORONAL,
		VR
	};

public:
	void InitSITab();
	bool IsInit();
	void InitializeView(ProjectIOSI::ViewType view_type);
	ProjectIOView & GetViewIO();

	void SaveSecondToFirst(const glm::mat4& mat);
	void SaveSecondTransformMatrix(const glm::mat4& mat);
	void SaveSecondRotate(const glm::mat4& mat);
	void SaveSecondTranslate(const glm::mat4& mat);
	void SaveSecondRotateForMPR(const glm::mat4& mat);
	void SaveSecondTranslateForMPR(const glm::mat4& mat);

	void LoadSecondToFirst(glm::mat4& mat);
	void LoadSecondTransformMatrix(glm::mat4& mat);
	void LoadSecondRotate(glm::mat4& mat);
	void LoadSecondTranslate(glm::mat4& mat);
	void LoadSecondRotateForMPR(glm::mat4& mat);
	void LoadSecondTranslateForMPR(glm::mat4& mat);

private:
	std::shared_ptr<H5::H5File> file_;
	std::unique_ptr<ProjectIOView> view_io_;
	ViewType curr_view_type_;
};

