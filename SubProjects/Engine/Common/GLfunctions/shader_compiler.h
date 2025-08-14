#pragma once

/**=================================================================================================

Project:		GLfunctions
File:			shader_compiler.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-09-17
Last modify: 	2018-09-17

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "glfunctions_global.h"
#include <map>

class GLFUNCTIONS_EXPORT ShaderCompiler {
public:
	ShaderCompiler();
	~ShaderCompiler();

	ShaderCompiler(const ShaderCompiler&) = delete;
	ShaderCompiler& operator=(const ShaderCompiler&) = delete;

public:
	void ShaderCompileAndAttachProgram(int program_key,
									   const QString& vertex_path,
									   const QString& fragment_path);

	void ClearPrograms();
	unsigned int GetProgramID(int program_key);
private:
	std::map<int, unsigned int> map_program_;
};
