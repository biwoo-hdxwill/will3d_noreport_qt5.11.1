#pragma once

/**=================================================================================================

Project:		Will3DEngine
File:			gl_resource_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-18
Last modify: 	2018-09-18

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "will3dengine_global.h"

#include "defines.h"
#include "shader_pack.h"
#include <memory>

class TexturePack;
class ShaderCompiler;
class CW3TF;
class CW3Image3D;

class WILL3DENGINE_EXPORT GLresourceMgr {
public:
  GLresourceMgr();
  ~GLresourceMgr();

  GLresourceMgr(const GLresourceMgr&) = delete;
  GLresourceMgr& operator=(const GLresourceMgr&) = delete;

public:
  void Initialize(const Will3DEngine::GLdeviceInfo& device_info);
  void ClearGL();

  void SetVolumeTexture(const Will3DEngine::VolType& vol_type, const CW3Image3D& vol,
						unsigned int* handler, unsigned int * texture_id, int* down_factor);
  void SetTfTexture(const CW3TF& tf, unsigned int* handler, unsigned int * texture_id);
  void ClearVolumeTexture(const Will3DEngine::VolType & vol_type);
  void ClearTf();
  unsigned int GetProgramID(const ShaderPack::SHADER& shader_id);

  unsigned int tmpGetVolTexHandler(const Will3DEngine::VolType& vol_type);
  unsigned int tmpGetTfTexHandler();

  inline bool IsValid() const { return initialized_; }
private:
  void InstallShaderPrograms();

  int GetDownSamplingRatio(const int & width, const int & height, const int & depth);

private:
  std::unique_ptr<TexturePack> will3d_textures_;
  std::unique_ptr<ShaderCompiler> shader_compiler_;
  bool initialized_ = false;
  QString gpu_vendor_;
  int available_gpu_memory_kb_;
  int max_tex_axis_size_;

  int reserved_gpu_memory_kb_ = 0;
};
