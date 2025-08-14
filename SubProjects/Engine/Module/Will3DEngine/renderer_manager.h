/**=================================================================================================

Project:		Will3DEngine
File:			renderer_manager.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-18
Last modify: 	2018-09-18

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#pragma once

#include "will3dengine_global.h"

#include <memory>
#include <mutex>

#include "defines.h"
#include "../Renderer/volume_renderer.h"
#include "../Renderer/slice_renderer.h"
#include "../Renderer/image_renderer.h"
#include "../Renderer/navigator_offscreen_renderer.h"
#include "../Renderer/implant_slice_renderer.h"
#include "implant_collision_renderer.h"

class WILL3DENGINE_EXPORT RendererManager
{

public:
	static void SetInstance();
	static RendererManager& GetInstance();

public:
	void InvertLight(const bool& is_invert);
	void AdjustWindowWL(const float& delta_width, const float& delta_level);
	void ResetDeltaWindowWL();

	inline VolumeRenderer& renderer_vol(Will3DEngine::VolType type) const { return *(renderer_vol_[type]); }
	inline SliceRenderer& renderer_slice(Will3DEngine::VolType type) const { return *(renderer_slice_[type]); }
	inline ImageRenderer& renderer_image() const { return *(renderer_image_); }
	inline NavigatorOffscreenRenderer& renderer_navigator() const { return *(renderer_navigator_); }
	inline ImplantSliceRenderer& renderer_implant() const { return *(implant_slice_renderer_); }
	inline ImplantCollisionRenderer& implant_collision_renderer() const { return *(implant_collision_renderer_); }

	void ApplyPreferences();

private:
	RendererManager();
	~RendererManager();

	RendererManager(const RendererManager&) = delete;
	RendererManager& operator=(const RendererManager&) = delete;
private:

	static RendererManager* instance_;
	static std::once_flag onceFlag_;

	VolumeRenderer* renderer_vol_[Will3DEngine::VOL_TYPE_END];
	SliceRenderer* renderer_slice_[Will3DEngine::VOL_TYPE_END];
	ImageRenderer* renderer_image_;
	NavigatorOffscreenRenderer* renderer_navigator_;
	ImplantSliceRenderer* implant_slice_renderer_;
	ImplantCollisionRenderer* implant_collision_renderer_;

};
