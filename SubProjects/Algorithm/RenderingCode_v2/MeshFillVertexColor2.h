#pragma once

#include <vector>
#include <string>

#include <gl/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#if defined(__APPLE__)
#include <QOpenGLWidget>
#else
#include <QtWidgets/QOpenGLWidget>
#endif

#include "renderingcode_v2_global.h"

/**********************************************************************************************//**
 * @class	RENDERINGCODE_V2_EXPORT
 *
 * @brief	A renderingcode v 2 export.
 *
 * @author	Jun
 * @date	2017-07-04
 **************************************************************************************************/

class RENDERINGCODE_V2_EXPORT MeshFillVertexColor2 {
public: /* member */
	std::string m_vertShaderPath;
	std::string m_fragShaderPath;

	// just store pointers from outside
	std::vector<glm::vec3>* m_pPoints;
	std::vector<std::vector<int>>* m_pTriangles;
	std::vector<glm::vec4>* m_pColors;

	int m_fboWidth;
	int m_fboHeight;

private:
	GLuint m_vbo_verts;
	GLuint m_vbo_idxs;
	GLuint m_vbo_colors;
	GLuint m_vao;
	GLuint m_shaderProg;
	GLuint m_fbo;
	GLuint m_rbo;
	GLuint m_tex_disps;
	GLuint m_tex_vertCoords;

public:
	MeshFillVertexColor2(
		std::vector<glm::vec3>* points,
		std::vector<std::vector<int>>* triangles,
		std::vector<glm::vec4>* colors,
		std::string vertShaderPath,
		std::string fragShaderPath);

	~MeshFillVertexColor2();

	bool initVBOs();
	bool initVAOs();
	bool initPROGs();
	bool initFBOs(int fboWidth, int fboHeight);
	void deinitAllGLObjects(); // make current 해준후에 불러야함!!

	void draw(
		const glm::mat4& M,
		const glm::mat4& V,
		const glm::mat4& P);

	void readTextureDisps(void* outPixels);
	void readTextureVertCoords(void* outPixels);
};
