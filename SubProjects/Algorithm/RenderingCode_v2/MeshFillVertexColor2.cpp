#include "MeshFillVertexColor2.h"

#include <exception>
#include <sstream>
#include <iostream>

#include "../../Engine/Common/GLfunctions/WGLSLprogram.h"
//#include "../../Engine/Common/GLfunctions/W3GLFunctions.h"

//using namespace std;

/////////////////////////////////////////////////// @MeshFillVertexColor /////////////////////////////////////////////////

void MeshFillVertexColor2::deinitAllGLObjects() {
	// delete buffers
	if (m_vbo_verts) { glDeleteBuffers(1, &m_vbo_verts); m_vbo_verts = 0; }
	if (m_vbo_idxs) { glDeleteBuffers(1, &m_vbo_idxs); m_vbo_idxs = 0; }
	if (m_vbo_colors) { glDeleteBuffers(1, &m_vbo_colors); m_vbo_colors = 0; }

	// delete vao
	if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }

	// delete shader
	if (m_shaderProg) { glDeleteProgram(m_shaderProg); m_shaderProg = 0; }

	// delete fbo & textures
	if (m_fbo) { glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
	if (m_rbo) { glDeleteRenderbuffers(1, &m_rbo); m_rbo = 0; }
	if (m_tex_disps) { glDeleteTextures(1, &m_tex_disps); m_tex_disps = 0; }
	if (m_tex_vertCoords) { glDeleteTextures(1, &m_tex_vertCoords); m_tex_vertCoords = 0; }

#if 1
	// MTEST
	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		printf("\n\n ############################# GL ERR0 in MeshFillVertexColor2::deinitAllGLObjects() !!!!!!!!!!!!!!!!!           \n\n");
		std::cout << "glError : " << gluErrorString(glErr) << std::endl;
	}
	//else
	//{
	//	printf("\n\n ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GL_NO_ERROR in MeshFillVertexColor2::deinitAllGLObjects()            \n\n");
	//}
#else
	// MTEST
	// void CW3GLFunctions::printError(int a_lineNum, const std::string& strErrMsg)
#endif
}

MeshFillVertexColor2::MeshFillVertexColor2(
	std::vector<glm::vec3>* points,
	std::vector<std::vector<int>>* triangles,
	std::vector<glm::vec4>* colors,
	std::string vertShaderPath,
	std::string fragShaderPath) {
	m_vertShaderPath = vertShaderPath;
	m_fragShaderPath = fragShaderPath;

	// just copy pointers
	m_pPoints = points;
	m_pTriangles = triangles;
	m_pColors = colors;

	m_fboWidth = 0;
	m_fboHeight = 0;

	m_vbo_verts = 0;
	m_vbo_idxs = 0;
	m_vbo_colors = 0;
	m_vao = 0;
	m_shaderProg = 0;
	m_fbo = 0;
	m_rbo = 0;
	m_tex_disps = 0;
	m_tex_vertCoords = 0;
}

MeshFillVertexColor2::~MeshFillVertexColor2() {
}

bool MeshFillVertexColor2::initVBOs() {
	if (m_vbo_verts == 0) {
		glGenBuffers(1, &m_vbo_verts);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
		{
			std::vector<GLfloat> buf(m_pPoints->size() * 3);
			for (int i = 0; i < m_pPoints->size(); i++) {
				buf[3 * i + 0] = (*m_pPoints)[i][0];
				buf[3 * i + 1] = (*m_pPoints)[i][1];
				buf[3 * i + 2] = (*m_pPoints)[i][2];
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * m_pPoints->size(), buf.data(), GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	if (m_vbo_colors == 0) {
		glGenBuffers(1, &m_vbo_colors);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colors);
		{
			std::vector<GLfloat> buf(m_pColors->size() * 4);
			for (int i = 0; i < m_pColors->size(); i++) {
				buf[4 * i + 0] = m_pColors->at(i)[0]; //r
				buf[4 * i + 1] = m_pColors->at(i)[1]; //g
				buf[4 * i + 2] = m_pColors->at(i)[2]; //b
				buf[4 * i + 3] = m_pColors->at(i)[3]; //a
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * m_pColors->size(), buf.data(), GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	// index buffer !
	if (m_vbo_idxs == 0) {
		glGenBuffers(1, &m_vbo_idxs);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_idxs);
		{
			std::vector<GLuint> buf(m_pTriangles->size() * 3);
			for (int i = 0; i < m_pTriangles->size(); i++) {
				buf[3 * i + 0] = (*m_pTriangles)[i][0];
				buf[3 * i + 1] = (*m_pTriangles)[i][1];
				buf[3 * i + 2] = (*m_pTriangles)[i][2];
			}
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 3 * m_pTriangles->size(), buf.data(), GL_STATIC_DRAW);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	return true;
}

bool MeshFillVertexColor2::initVAOs() {
	bool res = true;

	if (m_vbo_verts == 0 || m_vbo_idxs == 0 || m_vbo_colors == 0) {
		// Err : initVBOs() 먼저 해주세요!
		std::cout << " ERR : initVAOs() was called before initVBOs() !" << std::endl;
		return false;
	}

	if (m_vao == 0) {
		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
			{
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
				glEnableVertexAttribArray(0);
			}

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_idxs);

			glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colors);
			{
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL); // rgba
				glEnableVertexAttribArray(2);
			}
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	return true;
}

bool MeshFillVertexColor2::initPROGs() {
	bool res = true;

	if (m_shaderProg == 0) {
		GLuint vs, fs;

		QString sahderPath = ":/shader/";
		res = res && WGLSLprogram::createShaderProgramE(m_shaderProg);
		QString vertName(m_vertShaderPath.c_str());
		QString fragName(m_fragShaderPath.c_str());
		res = res && WGLSLprogram::compileShaderE(sahderPath + vertName, WGLSLShader::VERTEX, vs);
		res = res && WGLSLprogram::compileShaderE(sahderPath + fragName, WGLSLShader::FRAGMENT, fs);
		res = res && WGLSLprogram::attachLinkProgramE(m_shaderProg, vs, fs);
	}

	return res;
}

bool MeshFillVertexColor2::initFBOs(int fboWidth, int fboHeight) {
	m_fboWidth = fboWidth;
	m_fboHeight = fboHeight;

	bool res = true;

	if (m_tex_disps == 0) {
		glGenTextures(1, &m_tex_disps);
		glBindTexture(GL_TEXTURE_2D, m_tex_disps);
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, fboWidth, fboHeight);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (m_tex_vertCoords == 0) {
		glGenTextures(1, &m_tex_vertCoords);
		glBindTexture(GL_TEXTURE_2D, m_tex_vertCoords);
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, fboWidth, fboHeight);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// init renderbuffer
	if (m_rbo == 0) {
		glGenRenderbuffers(1, &m_rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
		{
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	// init fbo
	if (m_fbo == 0) {
		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex_disps, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_tex_vertCoords, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/* chk frame buffer status */
	GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (complete != GL_FRAMEBUFFER_COMPLETE)
		return false;
	else
		return true;
}

void MeshFillVertexColor2::draw(
	const glm::mat4& M,
	const glm::mat4& V,
	const glm::mat4& P) {
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glUseProgram(m_shaderProg);
	{
		glViewport(0, 0, m_fboWidth, m_fboHeight);

		// l_initializeGL
		glClearColor(0, 0, 0, 0);
		glClearDepth(1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		GLenum colorAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, colorAttachments);

		/* set uniforms */
		WGLSLprogram::setUniform(m_shaderProg, "M", M);
		WGLSLprogram::setUniform(m_shaderProg, "V", V);
		WGLSLprogram::setUniform(m_shaderProg, "P", P);

		/* draw */
		glBindVertexArray(m_vao);
		{
			glDrawElements(GL_TRIANGLES, 3 * m_pTriangles->size(), GL_UNSIGNED_INT, NULL);
		}
		glBindVertexArray(0);
	}
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MeshFillVertexColor2::readTextureDisps(void* outPixels) {
	glBindTexture(GL_TEXTURE_2D, m_tex_disps);
	{
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, outPixels);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MeshFillVertexColor2::readTextureVertCoords(void* outPixels) {
	glBindTexture(GL_TEXTURE_2D, m_tex_vertCoords);
	{
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, outPixels);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}
