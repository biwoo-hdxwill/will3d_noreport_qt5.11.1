#include "WGLSLprogram.h"
/*=========================================================================

File:			class WGSLprogram
Language:		C++11
Library:		Qt 5.4.0, glew, glm
Author:			Hong Jung
First date:		2015-11-13
Last modify:	2016-04-15

=========================================================================*/
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <sys/stat.h>

#include <QString>
#include <QFile>
#include <QTextStream>

#include "../Common/W3Logger.h"

using std::ifstream;
using std::ios;
using std::string;
using std::cout;
using std::endl;
//using std::cerr;

void WGLSLprogram::createShaderProgram(const QString& strPathVS, const QString& strPathFS, GLuint& handlePgm)
{
	GLuint vs = 0, fs = 0;
	compileShaderE(strPathVS, WGLSLShader::VERTEX, vs);
	compileShaderE(strPathFS, WGLSLShader::FRAGMENT, fs);
	createShaderProgramE(handlePgm);
	attachLinkProgramE(handlePgm, vs, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
}

int WGLSLprogram::checkForOpenGLError(const char* file, int line)
{
	// return 1 if an OpenGL error occured, 0 otherwise.
	int retCode = 0;

	GLenum glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		cout << "glError in file " << file
			<< "@line " << line << gluErrorString(glErr) << endl;
		retCode = 1;
		exit(EXIT_FAILURE);
	}
	return retCode;
}

string WGLSLprogram::getExtension(const char *name)
{
	string nameStr(name);

	size_t loc = nameStr.find_last_of('.');
	if (loc != string::npos)
	{
		return nameStr.substr(loc, string::npos);
	}
	return "";
}

bool WGLSLprogram::fileExists(const string &fileName)
{
	struct stat info;
	int ret = -1;

	ret = stat(fileName.c_str(), &info);
	return 0 == ret;
}

bool WGLSLprogram::compileShaderE(const char *filename, WGLSLShader::WGLSLShaderType type, GLuint &handle)
{
	try
	{
		compileShader(filename, type, handle);

		return true;
	}
	catch (GLSLProgramException &e)
	{
		common::Logger::instance()->Print(common::LogType::ERR, e.what());
		//cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	return false;
}

bool WGLSLprogram::compileShaderE(QString filename, WGLSLShader::WGLSLShaderType type, GLuint &handle)
{
	try
	{
		QFile shader(filename);
		if (!shader.open(QIODevice::ReadOnly))
		{
			string message = string("Shader: ") + filename.toStdString().c_str() + " not found.";
			common::Logger::instance()->Print(common::LogType::ERR, message);
			throw GLSLProgramException(message);
		}

		QTextStream stream(&shader);

		QString code = stream.readAll();
		shader.close();

		compileShader(code.toStdString().c_str(), type, handle, filename.toStdString().c_str());

		return true;
	}
	catch (GLSLProgramException &e)
	{
		common::Logger::instance()->Print(common::LogType::ERR, e.what());
		//cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	return false;
}

void WGLSLprogram::compileShader(const char * fileName, WGLSLShader::WGLSLShaderType type, GLuint &handle)
throw(GLSLProgramException)
{
	if (!fileExists(fileName))
	{
		string message = string("Shader: ") + fileName + " not found.";
		common::Logger::instance()->Print(common::LogType::ERR, message);
		throw GLSLProgramException(message);
	}

	ifstream inFile(fileName, ios::in);
	if (!inFile)
	{
		string message = string("Unable to open: ") + fileName;
		common::Logger::instance()->Print(common::LogType::ERR, message);
		throw GLSLProgramException(message);
	}

	// Get file contents
	std::stringstream code;
	code << inFile.rdbuf();
	inFile.close();

	compileShader(code.str(), type, handle, fileName);
}

void WGLSLprogram::compileShader(const string &source, WGLSLShader::WGLSLShaderType type, GLuint &shaderHandle, const char *fileName)
throw(GLSLProgramException)
{
	shaderHandle = glCreateShader(type);

	const char * c_code = source.c_str();
	glShaderSource(shaderHandle, 1, &c_code, NULL);

	// Compile the shader
	glCompileShader(shaderHandle);

	// Check for errors
	int result;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &result);
	if (GL_FALSE == result)
	{
		// Compile failed, get log
		int length = 0;
		string logString;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0)
		{
			char * c_log = new char[length];
			int written = 0;
			glGetShaderInfoLog(shaderHandle, length, &written, c_log);
			logString = c_log;
			delete[] c_log;
		}
		string msg;
		if (fileName)
		{
			msg = string(fileName) + ": shader compliation failed\n";
		}
		else
		{
			msg = "Shader compilation failed.\n";
		}
		msg += logString;

		throw GLSLProgramException(msg);
	}
}

bool WGLSLprogram::createShaderProgramE(GLuint &handle)
{
	try
	{
		createShaderProgram(handle);

		return true;
	}
	catch (GLSLProgramException &e)
	{
		common::Logger::instance()->Print(common::LogType::ERR, e.what());
		//cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	return false;
}

void WGLSLprogram::createShaderProgram(GLuint &handle)
throw(GLSLProgramException)
{
	handle = glCreateProgram();
	if (handle == 0)
	{
		throw GLSLProgramException("Unable to create shader program.");
	}
}

bool WGLSLprogram::attachLinkProgramE(GLuint &progH, GLuint &shader1, GLuint &shader2)
{
	try
	{
		attachLinkProgram(progH, shader1, shader2);

		return true;
	}
	catch (GLSLProgramException &e)
	{
		common::Logger::instance()->Print(common::LogType::ERR, e.what());
		//cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	return false;
}

void WGLSLprogram::attachLinkProgram(GLuint &progH, GLuint &shader1, GLuint &shader2)
throw(GLSLProgramException)
{
	const GLsizei maxCount = 2;
	GLsizei count;
	GLuint shaders[maxCount];
	glGetAttachedShaders(progH, maxCount, &count, shaders);

	for (int i = 0; i < count; i++)
	{
		glDetachShader(progH, shaders[i]);
	}

	glAttachShader(progH, shader1);
	glAttachShader(progH, shader2);

	glLinkProgram(progH);

	int status = 0;
	glGetProgramiv(progH, GL_LINK_STATUS, &status);
	if (GL_FALSE == status)
	{
		// Store log and return false
		int length = 0;
		string logString;

		glGetProgramiv(progH, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			char * c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(progH, length, &written, c_log);
			logString = c_log;
			delete[] c_log;
		}

		throw GLSLProgramException(string("Program link failed:\n") + logString);
	}

	//glUseProgram(progH);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, float x, float y, float z)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform3f(loc, x, y, z);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, float w, float h)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform2f(loc, w, h);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const vec3 &v)
{
	setUniform(handle, name, v.x, v.y, v.z);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const glm::i32vec3 &v)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform3i(loc, v.x, v.y, v.z);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const vec4 &v)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const vec2 &v)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform2f(loc, v.x, v.y);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const GLint N, const glm::vec2 *v)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform2fv(loc, N, (const GLfloat *)v);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const GLint N, const GLint *v)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform1iv(loc, N, (const GLint *)v);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const mat4 &m)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const mat3 &m)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, float val)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform1f(loc, val);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, int val)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform1i(loc, val);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, GLuint val)
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform1ui(loc, val);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, bool val)
{
	GLint loc = glGetUniformLocation(handle, name);
	if (val)
	{
		glUniform1i(loc, 1);
	}
	else
	{
		glUniform1i(loc, 0);
	}
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const int size, const glm::ivec3 *v) // by jdk 160509
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform3iv(loc, (GLsizei)size, (GLint *)v);
}

void WGLSLprogram::setUniform(const GLuint &handle, const char *name, const int size, const vec4 *v) // by jdk 160509
{
	GLint loc = glGetUniformLocation(handle, name);
	glUniform4fv(loc, (GLsizei)size, (GLfloat *)v);
}
