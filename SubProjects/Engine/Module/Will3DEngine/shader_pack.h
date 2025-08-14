#pragma once

/**=================================================================================================

Project:		Will3DEngine
File:			shader_pack.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-19
Last modify: 	2018-09-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "will3dengine_global.h"
#include <QString>

class WILL3DENGINE_EXPORT ShaderPack {
public:
	ShaderPack() = delete;

	ShaderPack(const ShaderPack&) = delete;
	ShaderPack& operator=(const ShaderPack&) = delete;

	enum SHADER {
		SHADER_FRONT_FACE_CUBE = 0,
		SHADER_FRONT_FACE_FINAL,
		SHADER_BACK_FACE_CUBE,
		SHADER_RAYCASTING,
		SHADER_RAY_FIRST_HIT,
		SHADER_TEXTURING_SURFACE,
		SHADER_RENDER_SLICE,
		SHADER_RENDER_SLICE_NERVE,
		SHADER_RENDER_SLICE_IMPLANT,
		SHADER_RENDER_SLICE_IMPLANT_WIRE,
		SHADER_RENDER_IMAGE,
		SHADER_RENDER_IMAGE_WINDOWING,
		SHADER_RENDER_SURFACE,
		SHADER_BONEDENSITY,
		SHADER_PICK_OBJECT,
		SHADER_END
	};

public:

	static void GetShaderFilePath(const SHADER& shader_id,
								  QString* vertex_path,
								  QString* fragment_path);
};
