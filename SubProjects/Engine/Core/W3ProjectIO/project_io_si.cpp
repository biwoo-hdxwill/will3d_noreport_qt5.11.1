#include "project_io_si.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"

#include "project_path_info.h"
#include "io_functions.h"
#include "project_io_view.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetSIGroup(H5File* file, const ProjectIOSI::ViewType& view_type) {
	switch (view_type) {
	case ProjectIOSI::ViewType::AXIAL:
		return file->openGroup(group::kViewSIAxial);
	case ProjectIOSI::ViewType::SAGITTAL:
		return file->openGroup(group::kViewSISagittal);
	case ProjectIOSI::ViewType::CORONAL:
		return file->openGroup(group::kViewSICoronal);
	case ProjectIOSI::ViewType::VR:
		return file->openGroup(group::kViewSIVR);
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "Project IO SI : invalid view Type");
		return Group();
	}
}
} // end of namespace

ProjectIOSI::ProjectIOSI(const project::Purpose& purpose,
						 const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabSI);
		file_->createGroup(group::kViewSIAxial);
		file_->createGroup(group::kViewSISagittal);
		file_->createGroup(group::kViewSICoronal);
		file_->createGroup(group::kViewSIVR);
	}
}

ProjectIOSI::~ProjectIOSI() {
}

void ProjectIOSI::InitSITab() {
	Group tab_grp = file_->openGroup(group::kTabSI);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOSI::IsInit() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabSI);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

void ProjectIOSI::InitializeView(ProjectIOSI::ViewType view_type) {
	curr_view_type_ = view_type;

	view_io_.reset(
		new ProjectIOView(file_, GetSIGroup(file_.get(), curr_view_type_)));
}

ProjectIOView & ProjectIOSI::GetViewIO() {
	return *(view_io_.get());
}

void ProjectIOSI::SaveSecondToFirst(const glm::mat4 & mat) {
	Group tab_grp = file_->openGroup(group::kTabSI);
	IOFunctions::WriteMatrix(tab_grp, ds::kSISecondToFirst, mat);
	tab_grp.close();
}

void ProjectIOSI::SaveSecondTransformMatrix(const glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kSISecondTransform, mat);
	curr_view_group.close();
}

void ProjectIOSI::SaveSecondRotate(const glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kSISecondRotate, mat);
	curr_view_group.close();
}

void ProjectIOSI::SaveSecondTranslate(const glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kSISecondTranslate, mat);
	curr_view_group.close();
}

void ProjectIOSI::SaveSecondRotateForMPR(const glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kSISecondRotateForMPR, mat);
	curr_view_group.close();
}

void ProjectIOSI::SaveSecondTranslateForMPR(const glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	IOFunctions::WriteMatrix(curr_view_group, ds::kSISecondTranslateForMPR, mat);
	curr_view_group.close();
}

void ProjectIOSI::LoadSecondToFirst(glm::mat4& mat) {
	Group tab_grp = file_->openGroup(group::kTabSI);
	mat = IOFunctions::ReadMatrix(tab_grp, ds::kSISecondToFirst);
	tab_grp.close();
}

void ProjectIOSI::LoadSecondTransformMatrix(glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kSISecondTransform);
	curr_view_group.close();
}

void ProjectIOSI::LoadSecondRotate(glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kSISecondRotate);
	curr_view_group.close();
}

void ProjectIOSI::LoadSecondTranslate(glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kSISecondTranslate);
	curr_view_group.close();
}

void ProjectIOSI::LoadSecondRotateForMPR(glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kSISecondRotateForMPR);
	curr_view_group.close();
}

void ProjectIOSI::LoadSecondTranslateForMPR(glm::mat4 & mat) {
	Group curr_view_group = GetSIGroup(file_.get(), curr_view_type_);
	mat = IOFunctions::ReadMatrix(curr_view_group, ds::kSISecondTranslateForMPR);
	curr_view_group.close();
}
