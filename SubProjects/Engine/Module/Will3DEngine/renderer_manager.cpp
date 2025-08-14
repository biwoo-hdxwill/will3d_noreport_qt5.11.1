#include "renderer_manager.h"

#include "../../Common/Common/W3Logger.h"

RendererManager* RendererManager::instance_ = nullptr;
std::once_flag RendererManager::onceFlag_;

using namespace Will3DEngine;

void RendererManager::SetInstance()
{
	std::call_once(
		RendererManager::onceFlag_,
		[=]() {
		instance_ = new RendererManager(); //해제 되지 않는 포인터.
	});
}

RendererManager& RendererManager::GetInstance()
{
	if (instance_ == nullptr)
	{
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"CW3SliceRenderer::getInstance: Volume Renderer Instance does not exist.");
		atexit(0);
	}

	return *(instance_);
}

void RendererManager::InvertLight(const bool & is_invert) {
	for (int i = 0; i < VOL_TYPE_END; i++) {
		renderer_vol_[i]->InvertWindow(is_invert);
		renderer_slice_[i]->InvertWindow(is_invert);
	}
	renderer_image_->InvertWindow(is_invert);
}

void RendererManager::AdjustWindowWL(const float & delta_width, const float & delta_level) {
	for (int i = 0; i < VOL_TYPE_END; i++) {
		renderer_vol_[i]->AdjustWindowWL(delta_width, delta_level);
		renderer_slice_[i]->AdjustWindowWL(delta_width, delta_level);
	}
	renderer_image_->AdjustWindowWL(delta_width, delta_level);
}

void RendererManager::ResetDeltaWindowWL() {
	for (int i = 0; i < VOL_TYPE_END; i++) {
		renderer_vol_[i]->ResetDeltaWindowWL();
		renderer_slice_[i]->ResetDeltaWindowWL();
	}
	renderer_image_->ResetDeltaWindowWL();
}

RendererManager::RendererManager()
{
	for (int i = 0; i < VOL_TYPE_END; i++)
	{
		renderer_vol_[i] = (new VolumeRenderer);
		renderer_slice_[i] = (new SliceRenderer);
	}

	renderer_image_ = (new ImageRenderer);
	renderer_navigator_ = (new NavigatorOffscreenRenderer);
	implant_slice_renderer_ = (new ImplantSliceRenderer);
	implant_collision_renderer_ = (new ImplantCollisionRenderer);
}

RendererManager::~RendererManager() {
}

void RendererManager::ApplyPreferences()
{
	implant_slice_renderer_->ApplyPreferences();
}
