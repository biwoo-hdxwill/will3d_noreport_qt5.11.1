#pragma once
/*=========================================================================

File:			class WGSLprogram
Language:		C++11
Library:		Qt 5.4.0, glew, glm
Author:			Hong Jung
First date:		2015-11-13
Last modify:	2016-04-15

=========================================================================*/
#include <string>

#include "WGLHeaders.h"
#include "W3GLTypes.h"

#include "glfunctions_global.h"

class GLSLProgramException : public std::runtime_error {
public:
	GLSLProgramException(const std::string& msg) :
		std::runtime_error(msg) { }
};

namespace WGLSLShader {
	enum WGLSLShaderType {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		COMPUTE = GL_COMPUTE_SHADER
	};
};

#define GL_ERROR() WGLSLprogram::checkForOpenGLError(__FILE__, __LINE__) // For OpenGL API debugging

class GLFUNCTIONS_EXPORT WGLSLprogram {
public:
	static void createShaderProgram(const QString& strPathVS, const QString& strPathFS, GLuint& handlePgm);

	static int checkForOpenGLError(const char* file, int line);

	static bool createShaderProgramE(GLuint &handle);
	static bool compileShaderE(const char * fileName, WGLSLShader::WGLSLShaderType type, GLuint &handle);
	static bool compileShaderE(QString fileName, WGLSLShader::WGLSLShaderType type, GLuint &handle);
	static bool attachLinkProgramE(GLuint &prog, GLuint &shader1, GLuint &shader2);

	static void setUniform(const GLuint &handle, const char *name, float x, float y, float z);
	static void setUniform(const GLuint &handle, const char *name, float w, float h);
	static void setUniform(const GLuint &handle, const char *name, const vec2 &v);
	static void setUniform(const GLuint &handle, const char *name, const vec3 &v);
	static void setUniform(const GLuint &handle, const char *name, const glm::i32vec3 &v);
	static void setUniform(const GLuint &handle, const char *name, const vec4 &v);
	static void setUniform(const GLuint &handle, const char *name, const mat4 &m);
	static void setUniform(const GLuint &handle, const char *name, const mat3 &m);
	static void setUniform(const GLuint &handle, const char *name, const GLint N, const glm::vec2 *m);
	static void setUniform(const GLuint &handle, const char *name, const GLint N, const GLint *m);
	static void setUniform(const GLuint &handle, const char *name, float val);
	static void setUniform(const GLuint &handle, const char *name, int val);
	static void setUniform(const GLuint &handle, const char *name, bool val);
	static void setUniform(const GLuint &handle, const char *name, GLuint val);
	static void setUniform(const GLuint &handle, const char *name, const int size, const glm::ivec3 *v); // by jdk 160509
	static void setUniform(const GLuint &handle, const char *name, const int size, const vec4 *v); // by jdk 160509

private:
	static bool fileExists(const std::string & fileName);
	static std::string getExtension(const char * fileName);

	static void compileShader(const char * fileName, WGLSLShader::WGLSLShaderType type, GLuint &handle) throw (GLSLProgramException);
	static void compileShader(const std::string & source, WGLSLShader::WGLSLShaderType type, GLuint &handle, const char *fileName = NULL) throw (GLSLProgramException);

	static void createShaderProgram(GLuint &handle) throw(GLSLProgramException);

	static void attachLinkProgram(GLuint &prog, GLuint &shader1, GLuint &shader2) throw(GLSLProgramException);
};
