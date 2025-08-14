#pragma once

/**=================================================================================================

Project:		Will3DEngine
File:			render_engine.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-19
Last modify: 	2018-09-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "will3dengine_global.h"

#include "defines.h"
#include <memory>

class QOpenGLContext;
class QOffscreenSurface;
class GLresourceMgr;
class CW3Image3D;
class PlaneResource;

class WILL3DENGINE_EXPORT RenderEngine
{
public:
	RenderEngine();
	~RenderEngine();

	RenderEngine(const RenderEngine&) = delete;
	RenderEngine& operator=(const RenderEngine&) = delete;

public:
	bool MakeCurrent();
	bool DoneCurrent();
	bool IsValid() const;

	bool IsUsingGLContextInGPU();

	void EnableGlobalShareContext();
	void SetShareContext(QOpenGLContext* context);

	void SetVRreconType(const QString& otf_name);

	void SetEnableMIP(bool is_mip);
	void SetEnableXray(bool is_xray);

	void SetVolume(const CW3Image3D& vol, const Will3DEngine::VolType& vol_type);
	void ClearVolume(const Will3DEngine::VolType & vol_type);
	void SetVolumeShade(bool is_shade);
	void UpdateTF(bool changed_min_max);

	void ClearTF();
	
	unsigned int tmpGetVolTexHandler(const Will3DEngine::VolType& vol_type);
	unsigned int tmpGetTfTexHandler();
	int tmpGetDownFactor(const Will3DEngine::VolType& vol_type);

	void SavePresetVolumeImage(const QString & file_path);
private:

	void InitializeInternal();
	bool InitGLcontext();
	void InitRenderers();
	void SetOpenGLDeviceInfo();

private:
	std::unique_ptr<QOpenGLContext> context_;
	std::unique_ptr<QOffscreenSurface> surface_;

	std::unique_ptr<GLresourceMgr> gl_resource_mgr_;
	std::shared_ptr<PlaneResource> plane_resource_;

	Will3DEngine::GLdeviceInfo gl_device_info_;
	bool initialized_ = false;
	bool is_enable_shade_vol_ = false;

};
