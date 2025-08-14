#include "project_io_face.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"

#include "project_path_info.h"
#include "io_functions.h"

#include "project_io_view.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetFaceGroup(H5File* file, const ProjectIOFace::ViewType& view_type) {
	switch (view_type) {
	case ProjectIOFace::ViewType::FACE_MESH:
		return file->openGroup(group::kViewFaceMesh);
	case ProjectIOFace::ViewType::PHOTO:
		return file->openGroup(group::kViewFacePhoto);
	case ProjectIOFace::ViewType::FACE_BEFORE:
		return file->openGroup(group::kViewFaceBefore);
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "Project IO MPR : invalid MPR Type");
		return Group();
	}
}
}

ProjectIOFace::ProjectIOFace(const project::Purpose& purpose,
							 const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabFace);
		file_->createGroup(group::kViewFaceMesh);
		file_->createGroup(group::kViewFacePhoto);
		file_->createGroup(group::kViewFaceBefore);
	}
}

ProjectIOFace::~ProjectIOFace() {}

void ProjectIOFace::InitFaceTab() {
	Group tab_grp = file_->openGroup(group::kTabFace);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOFace::IsInit() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabFace);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

void ProjectIOFace::InitializeView(ProjectIOFace::ViewType view_type) {
	curr_view_type_ = view_type;
	view_io_.reset(
		new ProjectIOView(file_, GetFaceGroup(file_.get(), curr_view_type_)));
}

ProjectIOView & ProjectIOFace::GetViewIO() {
	return *(view_io_.get());
}
