#include "project_io_mpr_engine.h"

#include <H5Cpp.h>

#include "io_functions.h"
#include "project_path_info.h"

using namespace H5;
using namespace project;

ProjectIOMPREngine::ProjectIOMPREngine(const project::Purpose& purpose, const std::shared_ptr<H5::H5File>& file) 
	: file_(file)
{
	if (purpose == project::Purpose::SAVE)
	{
		Group mpr_engine_group = file_->createGroup(group::kMPREngine);
		Group center_in_volume_group = mpr_engine_group.createGroup(group::kCenterInVolume);
		for (int i = 0; i < 2; ++i)
		{
			center_in_volume_group.createGroup(std::to_string(i));
		}
	}
}

ProjectIOMPREngine::~ProjectIOMPREngine() {}

void ProjectIOMPREngine::SaveCenterInVol(const int volume_id, const glm::vec3& org, const glm::vec3& mpr, const glm::vec3& si)
{
	bool exists = file_->exists(group::kMPREngine);
	if (!exists)
	{
		return;
	}

	Group mpr_engine_group = file_->openGroup(group::kMPREngine);
	Group center_in_volume_group = mpr_engine_group.openGroup(group::kCenterInVolume);
	Group volume_id_group = center_in_volume_group.openGroup(std::to_string(volume_id));

	IOFunctions::WriteVec3(volume_id_group, ds::kMPRCenterInVolumeOrg, org);
	IOFunctions::WriteVec3(volume_id_group, ds::kMPRCenterInVolume, mpr);
	IOFunctions::WriteVec3(volume_id_group, ds::kSICenterInVolume, si);
}

void ProjectIOMPREngine::LoadCenterInVol(const int volume_id, glm::vec3& org, glm::vec3& mpr, glm::vec3& si)
{
	bool exists = file_->exists(group::kMPREngine);
	if (!exists)
	{
		return;
	}

	Group mpr_engine_group = file_->openGroup(group::kMPREngine);
	Group center_in_volume_group = mpr_engine_group.openGroup(group::kCenterInVolume);
	Group volume_id_group = center_in_volume_group.openGroup(std::to_string(volume_id));

	org = IOFunctions::ReadVec3(volume_id_group, ds::kMPRCenterInVolumeOrg);
	mpr = IOFunctions::ReadVec3(volume_id_group, ds::kMPRCenterInVolume);
	si = IOFunctions::ReadVec3(volume_id_group, ds::kSICenterInVolume);
}
