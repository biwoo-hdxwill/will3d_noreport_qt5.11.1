#pragma once

/**=================================================================================================

Project:		Will3DEngine
File:			texture_pack.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-19
Last modify: 	2018-09-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "will3dengine_global.h"

#include "defines.h"

class CW3Image3D;
class CW3TF;

class WILL3DENGINE_EXPORT TexturePack {
public:
  TexturePack();
  ~TexturePack();
  TexturePack(const TexturePack&) = delete;
  TexturePack& operator=(const TexturePack&) = delete;

  enum class GL_TEXTURE_ID : unsigned int {
	GL_TEXTURE_DEFAULT = 0x84C0, //GL_TEXTURE0
	GL_TEXTURE_DEFAULT_ = 0,

	GL_TEXTURE_VOL = 0x84C1, //GL_TEXTURE1
	GL_TEXTURE_VOL_ = 1,

	GL_TEXTURE_TF = 0x84C2, //GL_TEXTURE2
	GL_TEXTURE_TF_ = 2,

	GL_TEXTURE_VOL_SECOND = 0x84C3, //GL_TEXTURE3
	GL_TEXTURE_VOL_SECOND_ = 3,

	GL_TEXTURE_VOL_PANO = 0x84C4, //GL_TEXTURE4
	GL_TEXTURE_VOL_PANO_ = 4,

	GL_TEXTURE_END = 0x84C5, //GL_TEXTURE5
	GL_TEXTURE_END_ = 5
  };

public:
  void ClearGL();
  void ClearVolumeTexture(const Will3DEngine::VolType & vol_type);
  void ClearTf();

  void SetVolumeTexture(const Will3DEngine::VolType & vol_type, unsigned short ** data,
						int width, int height, int depth,
						unsigned int* out_handler);
  void SetVolumeTexture(const Will3DEngine::VolType& vol_type, unsigned short *data,
						int width, int height, int depth,
						unsigned int* out_handler);
  void SetTfTexture(float* data, int width, int height, unsigned int* out_handler);

  inline unsigned int tex_vol_handler(const Will3DEngine::VolType& vol_type) const { return tex_vol_handler_[vol_type]; }
  inline unsigned int tex_tf_handler() const { return tex_tf_handler_; }

private:
  unsigned int tex_vol_handler_[Will3DEngine::VOL_TYPE_END];
  unsigned int tex_tf_handler_ = 0;

  int tf_width_ = 0;
  int tf_height_ = 0;

  unsigned int pbo_ = 0;

};
