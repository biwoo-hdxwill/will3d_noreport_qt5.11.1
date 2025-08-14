#include "project_io_panorama.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_pano.h"
#include "project_path_info.h"
#include "io_functions.h"
#include "project_io_view.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetPANOGroup(H5File* file, const ProjectIOPanorama::ViewType& view_type,
					   int cs_view_id = 0) {
	switch (view_type) {
	case ProjectIOPanorama::ViewType::PANO_PANO:
		return file->openGroup(group::kViewPanoPano);
	case ProjectIOPanorama::ViewType::PANO_ARCH:
		return file->openGroup(group::kViewPanoArch);
	case ProjectIOPanorama::ViewType::PANO_CS: 
	{
		Group pano_cs_grp = file->openGroup(group::kViewPanoCS);
		return pano_cs_grp.openGroup(std::to_string(cs_view_id));
	}
	default:
		common::Logger::instance()->Print(common::LogType::ERR,
										  "Project IO MPR : invalid MPR Type");
		return Group();
	}
}
} // end of namespace

ProjectIOPanorama::ProjectIOPanorama(const project::Purpose& purpose,
									 const std::shared_ptr<H5::H5File>& file) :
	file_(file) {
	if (purpose == project::Purpose::SAVE) {
		file_->createGroup(group::kTabPanorama);
		file_->createGroup(group::kViewPanoArch);
		file_->createGroup(group::kViewPanoPano);
		Group pano_pano_cs_grp = file_->createGroup(group::kViewPanoCS);
		for (int cnt = 0; cnt < common::kMaxCrossSection; ++cnt) {
			pano_pano_cs_grp.createGroup(std::to_string(cnt));
		}
		pano_pano_cs_grp.close();
	}
}

ProjectIOPanorama::~ProjectIOPanorama() {}

void ProjectIOPanorama::InitPanoTab() {
	if (IsInitPano())
		return;
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
	tab_grp.close();
}

bool ProjectIOPanorama::IsInitPano() {
	bool exists = false;
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (tab_grp.exists(ds::kIsTabInit))
		exists = true;
	tab_grp.close();
	return exists;
}

void ProjectIOPanorama::InitializeView(ProjectIOPanorama::ViewType view_type, int cs_view_id) {
	curr_view_type_ = view_type;
	curr_view_id_ = cs_view_id;

	view_io_.reset(
		new ProjectIOView(file_, GetPANOGroup(file_.get(), curr_view_type_, curr_view_id_)));
}

ProjectIOView & ProjectIOPanorama::GetViewIO() {
	return *(view_io_.get());
}

void ProjectIOPanorama::SaveOrientDegrees(const ArchTypeID& arch_type, int r, int i, int a) {
	std::string kCurrArchOrientID = arch_type == ArchTypeID::ARCH_MAXILLA ?
		ds::kPanoOrientMaxilla : ds::kPanoOrientMandible;

	int data[] = { r, i, a };
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	DataSet dataset = tab_grp.createDataSet(kCurrArchOrientID, PredType::NATIVE_INT,
											project::io::kDSVec3);
	dataset.write(data, PredType::NATIVE_INT);
	tab_grp.close();
}

void ProjectIOPanorama::SaveCSAngle(float degree)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteFloat(tab_grp, ds::kCSAngle, degree);
	tab_grp.close();
}

void ProjectIOPanorama::SaveCSInterval(float interval)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteFloat(tab_grp, ds::kCSInterval, interval);
	tab_grp.close();
}

void ProjectIOPanorama::SaveCSThickness(float thickness)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteFloat(tab_grp, ds::kCSThickness, thickness);
	tab_grp.close();
}

void ProjectIOPanorama::SaveArchRange(float range)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteFloat(tab_grp, ds::kArchRange, range);
	tab_grp.close();
}

void ProjectIOPanorama::SaveArchThickness(float thickness)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	IOFunctions::WriteFloat(tab_grp, ds::kArchThickness, thickness);
	tab_grp.close();
}

void ProjectIOPanorama::LoadOrientDegrees(const ArchTypeID& arch_type, int & r, int & i, int & a) {
	std::string kCurrArchOrientID = arch_type == ArchTypeID::ARCH_MAXILLA ?
		ds::kPanoOrientMaxilla : ds::kPanoOrientMandible;

	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(kCurrArchOrientID))
		return;
	DataSet dataset = tab_grp.openDataSet(kCurrArchOrientID);
	int data[3];
	dataset.read(data, PredType::NATIVE_INT);

	r = data[0];
	i = data[1];
	a = data[2];
	tab_grp.close();
}

void ProjectIOPanorama::LoadCSAngle(float& degree)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(ds::kCSAngle))
	{
		return;
	}

	degree = IOFunctions::ReadFloat(tab_grp, ds::kCSAngle);
	tab_grp.close();
}

bool ProjectIOPanorama::LoadCSInterval(float& interval)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(ds::kCSInterval))
	{
		return false;
	}

	interval = IOFunctions::ReadFloat(tab_grp, ds::kCSInterval);
	tab_grp.close();
	return true;
}

bool ProjectIOPanorama::LoadCSThickness(float& thickness)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(ds::kCSThickness))
	{
		return false;
	}

	thickness = IOFunctions::ReadFloat(tab_grp, ds::kCSThickness);
	tab_grp.close();
	return true;
}

bool ProjectIOPanorama::LoadArchRange(float& range)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(ds::kArchRange))
	{
		return false;
	}

	range = IOFunctions::ReadFloat(tab_grp, ds::kArchRange);
	tab_grp.close();
	return true;
}

bool ProjectIOPanorama::LoadArchThickness(float& thickness)
{
	Group tab_grp = file_->openGroup(group::kTabPanorama);
	if (!tab_grp.exists(ds::kArchThickness))
	{
		return false;
	}

	thickness = IOFunctions::ReadFloat(tab_grp, ds::kArchThickness);
	tab_grp.close();
	return true;
}
