#pragma once
/*=========================================================================

File:			class CW3VBOs
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-29
Last modify:	2016-04-25

=========================================================================*/
#include "../../Common/Common/W3Types.h"
#include "../../Common/GLfunctions/WGLHeaders.h"

#include "uiglobjects_global.h"

class UIGLOBJECTS_EXPORT CW3VBOSTL {
public:
	CW3VBOSTL(const QString& stlPath);
	~CW3VBOSTL();

public:
	void clearVAOVBO();
	void render();
	void quarterRender();

	int getVertexArrayHandle();

private:
	void InitVBO();
	void initVAO();

private:
	unsigned int vaoHandle = 0;
	unsigned int vboHandle[3] = { 0, 0, 0 };

	std::vector<glm::vec3> vertices_;
	std::vector<glm::vec3> normals_;
	std::vector<unsigned int> indices_;

	S3DMeshData m_s3MeshData;
};

class UIGLOBJECTS_EXPORT CW3VBOSphere {
public:
	CW3VBOSphere();
	~CW3VBOSphere();

	void createSphere(float radius, int slices, int stacks, glm::vec3 position = vec3(0.0));
	void clearVAOVBO();
	void render();

	int getVertexArrayHandle() { return vaoHandle; }

private:
	unsigned int vaoHandle = 0;
	unsigned int vboHandle[3] = { 0, 0, 0 };

	std::vector<GLuint> indices;

	float m_radius;
	int m_slices;
	int m_stacks;
	glm::vec3 m_position;
};
