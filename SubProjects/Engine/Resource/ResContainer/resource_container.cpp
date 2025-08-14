#include "resource_container.h"

#include "../../Common/Common/W3Logger.h"

ResourceContainer* ResourceContainer::instance_ = nullptr;
std::once_flag ResourceContainer::onceFlag_;

void ResourceContainer::SetInstance()
{
	std::call_once(
		ResourceContainer::onceFlag_,
		[=]() {
		instance_ = new ResourceContainer;
	});
}

ResourceContainer * ResourceContainer::GetInstance()
{
	if (instance_ == nullptr)
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"ResourceContainer::GetInstance: ResourceContainer Instance does not exist.");
		atexit(0);
	}

	return instance_;
}

ResourceContainer::~ResourceContainer() {
}

void ResourceContainer::Reset()
{
	res_main_volume_.reset();
	res_second_volume_.reset();
	//res_face_.reset();
	//res_dicom_header_.reset();
	//res_plane_.reset();
	res_pano_.reset();
	//res_nerve_.reset();
	//res_cross_section_.reset();
	//res_implant_.reset();
	//res_tmj_.reset();
	//res_sagittal_.reset();
	//res_lightbox_.reset();
	//res_tf_.reset();
}
