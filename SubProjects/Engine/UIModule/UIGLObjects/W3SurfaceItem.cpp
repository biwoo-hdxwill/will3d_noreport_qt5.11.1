#include "W3SurfaceItem.h"

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

using namespace UIGLObjects;

CW3BaseSurfaceItem::CW3BaseSurfaceItem() {
	m_material.Ks = vec3(0.55f);
	m_material.Ka = vec3(0.1f);
	m_material.Kd = vec3(1.0, 0.0, 0.0);
	m_material.Shininess = 15.0f;
}

CW3BaseSurfaceItem::~CW3BaseSurfaceItem() {}

void CW3BaseSurfaceItem::editTransformMat(const glm::mat4& matrix, TransformType type) {
	switch (type) {
	case TransformType::TRANSLATE: m_transform.translate = matrix * m_transform.translate; break;
	case TransformType::ROTATE: m_transform.rotate = matrix * m_transform.rotate; break;
	case TransformType::SCALE: m_transform.scale = matrix * m_transform.scale; break;
	case TransformType::ARCBALL: m_transform.arcball = matrix * m_transform.arcball; break;
	case TransformType::REORIENTATION: m_transform.reorien = matrix * m_transform.reorien; break;
	default:
		break;
	}
}
void CW3BaseSurfaceItem::setTransformMat(const glm::mat4& matrix, TransformType type) {
	switch (type) {
	case TransformType::TRANSLATE: m_transform.translate = matrix; break;
	case TransformType::ROTATE: m_transform.rotate = matrix; break;
	case TransformType::SCALE: m_transform.scale = matrix; break;
	case TransformType::ARCBALL: m_transform.arcball = matrix; break;
	case TransformType::REORIENTATION: m_transform.reorien = matrix; break;
	default:
		break;
	}
}
glm::mat4 CW3BaseSurfaceItem::getTransformMat(TransformType type) {
	switch (type) {
	case TransformType::TRANSLATE: return m_transform.translate;
	case TransformType::ROTATE: return m_transform.rotate;
	case TransformType::SCALE: return m_transform.scale;
	case TransformType::ARCBALL: return m_transform.arcball;
	case TransformType::REORIENTATION: return m_transform.reorien;
	default:
		return mat4(1.0f);
	}
}

glm::mat4 CW3BaseSurfaceItem::getSaveTransformMat(UIGLObjects::TransformType type) {
	switch (type) {
	case TransformType::TRANSLATE: return m_saveTransform.translate;
	case TransformType::ROTATE: return m_saveTransform.rotate;
	case TransformType::SCALE: return m_saveTransform.scale;
	case TransformType::ARCBALL: return m_saveTransform.arcball;
	case TransformType::REORIENTATION: return m_saveTransform.reorien;
	default:
		return mat4(1.0f);
	}
}

glm::mat4 CW3BaseSurfaceItem::getVolTexTransformMat(void) {
	mat4 invScale = glm::inverse(m_transform.scale);
	mat4 transTex = m_transform.translate;
	transTex[3][0] *= invScale[0][0];
	transTex[3][1] *= invScale[1][1];
	transTex[3][2] *= invScale[2][2];

	mat4 rotTex = glm::translate(mat3(m_transform.scale)*m_centroid)
		*m_transform.rotate*glm::translate(mat3(m_transform.scale)*-m_centroid);
	rotTex = invScale * rotTex*m_transform.scale;

	return transTex * rotTex;
}

bool CW3BaseSurfaceItem::pushModelMat() {
	m_isPushModel = true;
	m_saveTransform.translate = m_transform.translate;
	m_saveTransform.rotate = m_transform.rotate;
	m_saveTransform.scale = m_transform.scale;
	m_saveTransform.reorien = m_transform.reorien;
	return true;
}

bool CW3BaseSurfaceItem::popModelMat() {
	if (m_isPushModel) {
		m_transform.translate = m_saveTransform.translate;
		m_transform.rotate = m_saveTransform.rotate;
		m_transform.scale = m_saveTransform.scale;
		m_isPushModel = false;
		return true;
	} else {
		return false;
	}
}

void CW3BaseSurfaceItem::setUniformColor(GLuint program) {
	WGLSLprogram::setUniform(program, "Material.Ka", m_material.Ka);
	WGLSLprogram::setUniform(program, "Material.Ks", m_material.Ks);
	WGLSLprogram::setUniform(program, "Material.Shininess", m_material.Shininess);
	WGLSLprogram::setUniform(program, "Material.Kd", m_material.Kd);
	WGLSLprogram::setUniform(program, "alpha", m_alpha);
}

void CW3BaseSurfaceItem::setMatrix(GLuint program) {
	mat4 rotModel = glm::translate(mat3(m_transform.scale)*m_centroid)*
		m_transform.rotate*
		glm::translate(mat3(m_transform.scale)*-m_centroid);

	m_model = m_transform.translate*rotModel*m_transform.scale*m_model;
	m_mv = m_view * m_transform.arcball*m_transform.reorien*m_model;
	WGLSLprogram::setUniform(program, "ModelViewMatrix", m_mv);
	WGLSLprogram::setUniform(program, "NormalMatrix",
							 glm::mat3(glm::vec3(m_mv[0]), glm::vec3(m_mv[1]), glm::vec3(m_mv[2])));

	m_mvp = m_projection * m_mv;
	WGLSLprogram::setUniform(program, "MVP", m_mvp);
}

void CW3BaseSurfaceItem::readPickInfo(int x, int y, unsigned char* _pickID, glm::vec3* _pickGLCoord, GLenum format, GLenum type) {
	vec4 pickColor = CW3GLFunctions::readPickColor(vec2(x, y), format, type);

#if 0
	if (pickColor.x < 255.0f && pickColor.y < 255.0f && pickColor.z < 255.0f)
		std::cout << pickColor.x << ", " << pickColor.y << ", " << pickColor.z << std::endl;
#endif

	if (type == GL_FLOAT) {
		*_pickID = (unsigned char)(pickColor.w *255.0f);
		*_pickGLCoord = (vec3(pickColor.xyz) * 2.0f) - 1.0f;
	} else {
		*_pickID = pickColor.w;
		*_pickGLCoord = (vec3(pickColor.xyz) / 255.0f) - 1.0f;
	}
}

void CW3BaseSurfaceItem::setAlpha(float alpha) {
	m_alpha = alpha;
	m_isTransparency = (m_alpha == 1.0f) ? false : true;
}

CW3SurfaceItem::CW3SurfaceItem() {}
CW3SurfaceItem::~CW3SurfaceItem() {}

void CW3SurfaceItem::initSurfaceFillColor(const std::vector<glm::vec3>& vertices,
										  const std::vector<unsigned int>& indices,
										  Orientation orien) {
	std::vector<vec3> normals;
	CW3ElementGenerator::GenerateSmoothNormals(vertices, indices, normals, orien);

	if (m_vbo.size()) {
		glDeleteBuffers(m_vbo.size(), &m_vbo[0]);
		m_vbo.clear();
	}
	if (m_vao) {
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	m_vbo.resize(3, 0);
	CW3GLFunctions::initVAOVBO(&m_vao, &m_vbo[0], vertices, normals, indices);

	m_Nindices = indices.size();
}
void CW3SurfaceItem::initSurfaceFillTexture(const std::vector<glm::vec3>& vertices,
											const std::vector<glm::vec2>& texCoords,
											const std::vector<unsigned int>& indices,
											Orientation orien) {
	if (m_vbo.size()) {
		glDeleteBuffers(m_vbo.size(), &m_vbo[0]);
		m_vbo.clear();
	}
	if (m_vao) {
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	m_vbo.resize(3, 0);
	CW3GLFunctions::initVAOVBO(&m_vao, &m_vbo[0], vertices, texCoords, indices);

	m_Nindices = indices.size();
}
void CW3SurfaceItem::clearVAOVBO() {
	if (m_vbo.size()) {
		glDeleteBuffers(m_vbo.size(), &m_vbo[0]);
		m_vbo.clear();
	}
	if (m_vao) {
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}
	m_Nindices = 0;
}
void CW3SurfaceItem::draw(GLuint program) {
	if (!m_vao)
		return;

	this->setUniformColor(program);
	m_model = mat4(1.0f);
	this->setMatrix(program);

	render();
}
void CW3SurfaceItem::draw(GLuint program, GLenum cullFace) {
	if (!m_vao)
		return;

	this->setUniformColor(program);

	GLboolean enableCullFace = GL_FALSE;
	glGetBooleanv(GL_CULL_FACE, &enableCullFace);

	glEnable(GL_CULL_FACE);
	glCullFace(cullFace);

	m_model = mat4(1.0f);
	this->setMatrix(program);

	render();

	if (enableCullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

const bool CW3SurfaceItem::isReadyVAO() const {
	return m_vao ? true : false;
}
