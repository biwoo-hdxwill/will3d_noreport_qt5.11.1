#include "project_io_implant.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_pano.h"
#include "project_path_info.h"
#include "io_functions.h"
#include "project_io_view.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetPANOGroup(H5File* file, const ProjectIOImplant::ViewType& view_type,
					   int cs_view_id = 0) {
	switch (view_type) {
	case ProjectIOImplant::ViewType::IMP_PANO:
		return file->openGroup(group::kViewImpPano);
	case ProjectIOImplant::ViewType::IMP_ARCH:
		return file->openGroup(group::kViewImpArch);
	case ProjectIOImplant::ViewType::IMP_IMPLANT3D:
		return file->openGroup(group::kViewImp3D);
	case ProjectIOImplant::ViewType::IMP_SAGITTAL:
		return file->openGroup(group::kViewImpSagittal);
	case ProjectIOImplant::ViewType::IMP_CS:
	{
		Group pano_cs_grp = file->openGroup(group::kViewImpCS);
		return pano_cs_grp.openGroup(std::to_string(cs_view_id));
	}
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "Project IO MPR : invalid MPR Type");
		return Group();
	}
}
} // end of namespace

ProjectIOImplant::ProjectIOImplant(const project::Purpose& purpose,
								   const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabImplant);
		file_->createGroup(group::kViewImpArch);
		file_->createGroup(group::kViewImpSagittal);
		file_->createGroup(group::kViewImpPano);
		file_->createGroup(group::kViewImp3D);
		Group pano_cs_grp = file_->createGroup(group::kViewImpCS);

		for (int cnt = 0; cnt < common::kMaxCrossSection; ++cnt) {
			pano_cs_grp.createGroup(std::to_string(cnt));
		}
		pano_cs_grp.close();
	}
}

ProjectIOImplant::~ProjectIOImplant() {}

void ProjectIOImplant::InitImplantTab() {
	Group tab_grp = file_->openGroup(group::kTabImplant);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOImplant::IsInitImplant() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabImplant);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

void ProjectIOImplant::InitializeView(ProjectIOImplant::ViewType view_type, int cs_view_id) {
	curr_view_type_ = view_type;
	curr_view_id_ = cs_view_id;

	view_io_.reset(
		new ProjectIOView(file_, GetPANOGroup(file_.get(), curr_view_type_, curr_view_id_)));
}

ProjectIOView & ProjectIOImplant::GetViewIO() {
	return *(view_io_.get());
}

void ProjectIOImplant::SaveCSAngle(float degree)
{
	Group tab_grp = file_->openGroup(group::kTabImplant);
	IOFunctions::WriteFloat(tab_grp, ds::kCSAngle, degree);
	tab_grp.close();
}

void ProjectIOImplant::LoadCSAngle(float& degree)
{
	Group tab_grp = file_->openGroup(group::kTabImplant);
	if (!tab_grp.exists(ds::kCSAngle))
	{
		return;
	}

	degree = IOFunctions::ReadFloat(tab_grp, ds::kCSAngle);
	tab_grp.close();
}
