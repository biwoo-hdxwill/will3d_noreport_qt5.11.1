#include "W3GLNerve.h"
/*=========================================================================

File:			class CW3GLNerve
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-05-27
Last date:		2016-05-27

=========================================================================*/
#include "../../Common/GLfunctions/WGLSLprogram.h"

CW3GLNerve::CW3GLNerve()
    : CW3GLObject(GL_TRIANGLES)
{
	glm::vec3 colr = glm::vec3(1.0f, 0.0f, 0.0f);
	m_material.Ks = glm::vec3(1.0f);
	m_material.Ka = colr * 0.2f;
	m_material.Kd = colr;
	m_material.Shininess = 10.0f;
}

CW3GLNerve::~CW3GLNerve() {
}

void CW3GLNerve::setUniformColor(GLuint program) {
	WGLSLprogram::setUniform(program, "Material.Ka", m_material.Ka);
	WGLSLprogram::setUniform(program, "Material.Ks", m_material.Ks);
	WGLSLprogram::setUniform(program, "Material.Shininess", m_material.Shininess);
	WGLSLprogram::setUniform(program, "Material.Kd", m_material.Kd);
	WGLSLprogram::setUniform(program, "alpha", m_alpha);
}
