#include "W3SurfaceArchItem.h"

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/common.h"
#include "../../Common/Common/W3Logger.h"

#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "W3VBOs.h"

CW3SurfaceArchItem::CW3SurfaceArchItem() {}

CW3SurfaceArchItem::~CW3SurfaceArchItem() {
	if (m_vboArch.size())
		glDeleteBuffers(m_vboArch.size(), &m_vboArch[0]);

	if (m_vaoArch)
		glDeleteVertexArrays(1, &m_vaoArch);

	if (m_vboOutline.size())
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
	if (m_vaoOutline)
		glDeleteVertexArrays(1, &m_vaoOutline);

	while (m_ctrlItem.size()) {
		auto iter = m_ctrlItem.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_ctrlItem.erase(iter);
	}
}
void CW3SurfaceArchItem::clear() {
	m_archIndices.clear();
	m_outlineIndices.clear();
	m_oriCtrlPoint.clear();
	m_ctrlPoint.clear();

	while (m_ctrlItem.size()) {
		auto iter = m_ctrlItem.begin();
		SAFE_DELETE_OBJECT(*iter);
		m_ctrlItem.erase(iter);
	}

	if (m_vboArch.size()) {
		glDeleteBuffers(m_vboArch.size(), &m_vboArch[0]);
		m_vboArch.clear();
	}
	if (m_vaoArch) {
		glDeleteVertexArrays(1, &m_vaoArch);
		m_vaoArch = 0;
	}
	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}
	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}
}
void CW3SurfaceArchItem::clearVAOVBO() {
	if (m_vboArch.size()) {
		glDeleteBuffers(m_vboArch.size(), &m_vboArch[0]);
		m_vboArch.clear();
	}
	if (m_vaoArch) {
		glDeleteVertexArrays(1, &m_vaoArch);
		m_vaoArch = 0;
	}
	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}
	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}
	m_archIndices.clear();
	m_outlineIndices.clear();

	for (auto & elem : m_ctrlItem)
		elem->clearVAOVBO();

	m_isChangeArea = true;
	m_isChangeCtrl = true;
}

void CW3SurfaceArchItem::initializeArch(const std::vector<vec3>& points) {
	if (points.size() != ITEM_TYPE_END) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  std::string("CW3SurfaceArchItem::initializeArch: point count should be count of eItemType."));
		return;
	}

	clear();

	for (int i = 0; i < ITEM_TYPE_END; i++)
		m_ctrlItem.push_back(new CW3VBOSphere());

	m_itemType = ITEM_UNKNOWN_TYPE;
	m_pickPos = vec3(0.0f, 0.0f, 0.0f);

	m_oriCtrlPoint.assign(points.begin(), points.end());
	m_ctrlPoint.assign(points.begin(), points.end());

	this->initializeVBO();

	for (int i = 0; i < ITEM_TYPE_END; i++)
		m_ctrlItem[i]->createSphere(0.025, 30, 30, m_ctrlPoint[i]);
}

void CW3SurfaceArchItem::initializeVBO(void) {
	////////////////////////////////////////////////////////////////////////////////////////////
	//Create arch object vao&vbo.
	////////////////////////////////////////////////////////////////////////////////////////////
	m_archIndices.clear();

	std::vector<vec3> vert, norm, topArch, botArch, topLine, botLine;

	vec3 lower_center = (m_ctrlPoint[LOWER2] - m_ctrlPoint[LOWER0])*0.5f + m_ctrlPoint[LOWER0];
	vec3 upper_center = (m_ctrlPoint[UPPER2] - m_ctrlPoint[UPPER0])*0.5f + m_ctrlPoint[UPPER0];

	std::vector<vec3> ctrlArch;
	ctrlArch.assign(m_ctrlPoint.begin(), m_ctrlPoint.begin() + 3);
	Common::generateCubicSpline(ctrlArch, botArch, 10);

	ctrlArch.clear();
	ctrlArch.assign(m_ctrlPoint.begin() + 3, m_ctrlPoint.begin() + 6);
	Common::generateCubicSpline(ctrlArch, topArch, 10);

	CW3ElementGenerator::generateRectFace(botArch, topArch, vert, norm, m_archIndices); //FACE 1

	CW3ElementGenerator::generateSectorFace(lower_center, botArch, vert, norm, m_archIndices); //FACE 2

	std::reverse(topArch.begin(), topArch.end());
	CW3ElementGenerator::generateSectorFace(upper_center, topArch, vert, norm, m_archIndices); //FACE 3

	topLine.push_back(m_ctrlPoint[UPPER0]);
	topLine.push_back(m_ctrlPoint[UPPER2]);
	botLine.push_back(m_ctrlPoint[LOWER0]);
	botLine.push_back(m_ctrlPoint[LOWER2]);
	CW3ElementGenerator::generateRectFace(topLine, botLine, vert, norm, m_archIndices); //FACE 4

	if (m_vboArch.size())
		glDeleteBuffers(m_vboArch.size(), &m_vboArch[0]);

	if (m_vaoArch) {
		glDeleteVertexArrays(1, &m_vaoArch);
		m_vaoArch = 0;
	}

	m_vboArch.resize(3, 0);
	CW3GLFunctions::initVAOVBO(&m_vaoArch, &m_vboArch[0], vert, norm, m_archIndices);

	if (!m_vaoArch) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  std::string("CW3SurfaceArchItem::initializeVBO: failed to generate VAO."));
		return;
	}
	if (!m_vboArch[0]) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  std::string("CW3SurfaceArchItem::initializeVBO: failed to generate VBO."));
		return;
	}

	m_centroid = vec3(0.0f);
	for (const auto &i : m_ctrlPoint)
		m_centroid += i;

	m_centroid = m_centroid / static_cast<float>(m_ctrlPoint.size());

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//Create outline vao&vbo.
	////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<vec3> outlineVerts;
	m_outlineIndices.clear();

	outlineVerts.push_back(m_ctrlPoint[LOWER0]);
	outlineVerts.push_back(m_ctrlPoint[LOWER2]);
	outlineVerts.push_back(m_ctrlPoint[UPPER2]);
	outlineVerts.push_back(m_ctrlPoint[UPPER0]);

	for (int i = 0; i < (int)outlineVerts.size() - 1; i++) {
		m_outlineIndices.push_back(i);
		m_outlineIndices.push_back(i + 1);
	}
	m_outlineIndices.push_back(3);
	m_outlineIndices.push_back(0);

	int IdxL = outlineVerts.size();
	outlineVerts.insert(outlineVerts.end(), topArch.begin(), topArch.end());
	for (int i = IdxL; i < (int)outlineVerts.size() - 1; i++) {
		m_outlineIndices.push_back(i);
		m_outlineIndices.push_back(i + 1);
	}
	IdxL = outlineVerts.size();
	outlineVerts.insert(outlineVerts.end(), botArch.begin(), botArch.end());
	for (int i = IdxL; i < (int)outlineVerts.size() - 1; i++) {
		m_outlineIndices.push_back(i);
		m_outlineIndices.push_back(i + 1);
	}

	if (m_vboOutline.size()) {
		glDeleteBuffers(m_vboOutline.size(), &m_vboOutline[0]);
		m_vboOutline.clear();
	}

	if (m_vaoOutline) {
		glDeleteVertexArrays(1, &m_vaoOutline);
		m_vaoOutline = 0;
	}

	m_vboOutline.resize(2, 0);
	CW3GLFunctions::initVAOVBO(&m_vaoOutline, &m_vboOutline[0], outlineVerts, m_outlineIndices);

	if (!m_vaoOutline) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  std::string("CW3SurfaceArchItem::initializeVBO: failed to generate VAO_Outline."));
		return;
	}
	if (!m_vboOutline[0]) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  std::string("CW3SurfaceArchItem::initializeVBO: failed to generate VBO_Outline."));
		return;
	}
}

void CW3SurfaceArchItem::draw(GLuint program, GLenum cullFace, bool isTransform) {
	if (m_isChangeArea) {
		this->initializeVBO();
		m_isChangeArea = false;
	}

	mat4 model = mat4(1.0f);

	GLboolean enableCullFace = GL_FALSE;
	glGetBooleanv(GL_CULL_FACE, &enableCullFace);

	glEnable(GL_CULL_FACE);

	glCullFace(cullFace);

	float Coef_Ka = 0.1f;
	float Coef_Kd = 1.0f;
	vec3 color;

	//draw arch area
	m_model = model;

	if (isTransform) {
		setMatrix(program);
	} else {
		setOriginMatrix(program);
	}

	//WGLSLprogram::setUniform(program, "VolTexTransformMat", glm::inverse(m_transform.scale)*m_transform.reorien*m_transform.scale);
	WGLSLprogram::setUniform(program, "VolTexTransformMat", glm::mat4(1.0f));
	color = vec3(0.4f);
	WGLSLprogram::setUniform(program, "Material.Ks", vec3(0.55f));
	WGLSLprogram::setUniform(program, "Material.Shininess", 15.0f);
	WGLSLprogram::setUniform(program, "Material.Ka", vec3(Coef_Ka));
	WGLSLprogram::setUniform(program, "Material.Kd", color*Coef_Kd);
	WGLSLprogram::setUniform(program, "alpha", 0.4f);

	//CW3GLFunctions::initVAO(&m_vaoArch, &m_vboArch[0]);
	glBindVertexArray(m_vaoArch);

	// Draw the triangles !
	glDrawElements(
		GL_TRIANGLES,      // mode
		m_archIndices.size(),    // count
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (enableCullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CW3SurfaceArchItem::drawControl(GLuint program, GLenum cullFace) {
	for (int i = 0; i < ITEM_TYPE_END; i++) {
		if (!m_ctrlItem[i]->getVertexArrayHandle())
			m_ctrlItem[i]->createSphere(0.025, 30, 30, m_ctrlPoint[i]);
	}

	if (m_isChangeCtrl) {
		for (int i = 0; i < ITEM_TYPE_END; i++) {
			m_ctrlItem[i]->createSphere(0.025, 30, 30, m_ctrlPoint[i]);

			if (!m_ctrlItem[i]->getVertexArrayHandle()) {
				common::Logger::instance()->Print(common::LogType::ERR,
												  std::string("CW3SurfaceArchItem::drawControl: failed to create sphere."));
				return;
			}
		}
		m_isChangeCtrl = false;
	}

	//draw control object
	mat4 pushScale = m_transform.scale;

	float maxScale = (m_transform.scale[0][0] > m_transform.scale[1][1]) ? m_transform.scale[0][0] : m_transform.scale[1][1];
	maxScale = (maxScale > m_transform.scale[2][2]) ? maxScale : m_transform.scale[2][2];

	mat4 ratioMat = glm::inverse(glm::scale(vec3(m_transform.scale[0][0], m_transform.scale[1][1], m_transform.scale[2][2]) / maxScale));

	GLboolean enableCullFace = GL_FALSE;
	glGetBooleanv(GL_CULL_FACE, &enableCullFace);

	glEnable(GL_CULL_FACE);
	glCullFace(cullFace);

	float Coef_Ka = 0.1f;
	float Coef_Kd = 1.0f;
	WGLSLprogram::setUniform(program, "Material.Ks", vec3(0.55f));
	WGLSLprogram::setUniform(program, "Material.Shininess", 15.0f);

	vec3 color = vec3(0.6f, 0.6f, 0.6f);
	WGLSLprogram::setUniform(program, "Material.Kd", color*Coef_Kd);
	WGLSLprogram::setUniform(program, "alpha", 1.0f);

	//control point draw
	//mat4 invScale = glm::inverse(m_transform.scale);
	//mat4 rotTex = glm::translate(mat3(m_transform.scale)*m_centroid)*m_rotModel*glm::translate(mat3(m_transform.scale)*-m_centroid);
	//rotTex = invScale*rotTex*m_transform.scale;

	mat4 model = mat4(1.0f);
	for (int i = 0; i < ITEM_TYPE_END; i++) //skip center control point
	{
		m_model = model;
		m_transform.scale = pushScale * glm::translate(m_ctrlPoint[i])*ratioMat*glm::translate(-m_ctrlPoint[i]);
		WGLSLprogram::setUniform(program, "VolTexTransformMat", this->getVolTexTransformMat());
		setMatrix(program);

		if (m_itemType == eItemType(i))
			WGLSLprogram::setUniform(program, "Material.Ka", vec3(color));
		else
			WGLSLprogram::setUniform(program, "Material.Ka", vec3(Coef_Ka));

		m_ctrlItem[i]->render();
	}

	m_transform.scale = pushScale;

	if (enableCullFace)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CW3SurfaceArchItem::drawOutline(GLuint program) {
	if (m_isChangeArea) {
		this->initializeVBO();
		m_isChangeArea = false;
	}

	WGLSLprogram::setUniform(program, "Material.Ka", vec3(1.0));
	//WGLSLprogram::setUniform(program, "Material.Ka", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Ks", vec3(0.0));
	WGLSLprogram::setUniform(program, "Material.Kd", vec3(0.0));
	WGLSLprogram::setUniform(program, "alpha", 1.0f);

	//outline draw
	m_model = mat4(1.0f);
	WGLSLprogram::setUniform(program, "VolTexTransformMat", this->getVolTexTransformMat());
	setMatrix(program);
	glLineWidth(2.0);

	//CW3GLFunctions::initVAOpointOnly(&m_vaoOutline, &m_vboOutline[0]);
	glBindVertexArray(m_vaoOutline);
	{
		//glLineWidth(2.0);

		glDrawElements(
			GL_LINES,      // mode
			m_outlineIndices.size(),    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);
	}
	glBindVertexArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3SurfaceArchItem::pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program) {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//draw control object
	float maxScale = (m_transform.scale[0][0] > m_transform.scale[1][1]) ? m_transform.scale[0][0] : m_transform.scale[1][1];
	maxScale = (maxScale > m_transform.scale[2][2]) ? maxScale : m_transform.scale[2][2];

	mat4 ratioMat = glm::scale(vec3(1.5f))*glm::inverse(glm::scale(vec3(m_transform.scale[0][0], m_transform.scale[1][1], m_transform.scale[2][2]) / maxScale));

	mat4 invScale = glm::inverse(m_transform.scale);
	mat4 rotTex = glm::translate(mat3(m_transform.scale)*m_centroid)*m_transform.rotate*glm::translate(mat3(m_transform.scale)*-m_centroid);
	rotTex = invScale * rotTex*m_transform.scale;

	mat4 pushScale = m_transform.scale;
	mat4 model = mat4(1.0f);
	for (int i = 0; i < ITEM_TYPE_END; i++) {
		if (!m_ctrlItem[i]->getVertexArrayHandle()) {
			common::Logger::instance()->Print(common::LogType::ERR,
											  std::string("CW3SurfaceArchItem::pick: VAO is null."));
			return;
		}

		m_model = model;
		m_transform.scale = pushScale * glm::translate(m_ctrlPoint[i])*ratioMat*glm::translate(-m_ctrlPoint[i]);
		//WGLSLprogram::setUniform(program, "VolTexTransformMat", this->getVolTexTransformMat());
		WGLSLprogram::setUniform(program, "index", i);
		setMatrix(program);
		m_ctrlItem[i]->render();
	}

	m_transform.scale = pushScale;

	unsigned char id;
	vec3 pickPos;
	readPickInfo(pickPoint.x(), pickPoint.y(), &id, &pickPos);

	if (m_itemType != static_cast<eItemType>(id) && m_itemType != static_cast<eItemType>(id - 256))
		*isUpdateGL = true;
	else
		*isUpdateGL = false;

	if (id > ITEM_UNKNOWN_TYPE && id < ITEM_TYPE_END) {
		m_itemType = static_cast<eItemType>(id);
		m_pickPos = pickPos;
	} else {
		m_itemType = ITEM_UNKNOWN_TYPE;
	}
}

void CW3SurfaceArchItem::translateControl(const glm::vec3& transGLCoord) {
	vec4 vRotAxis;
	bool isTranslate;

	switch (m_itemType) {
	case UPPER0: /* fall through */
	case UPPER1: /* fall through */
	case UPPER2: /* fall through */
	case LOWER0: /* fall through */
	case LOWER1: /* fall through */
	case LOWER2:
		isTranslate = true;
		break;
	default:
		isTranslate = false;
	}

	if (isTranslate) {
		mat4 iSRVP = glm::inverse(m_projection*m_view*m_transform.arcball*m_transform.scale);
		vec4 trans = iSRVP * vec4(transGLCoord, 1.0f);

		//int idx = static_cast<int>(m_itemType);
		m_ctrlPoint[m_itemType] += vec3(trans);

		float z = m_ctrlPoint[m_itemType].z;
		int compIdx = (m_itemType + 3) % ITEM_TYPE_END;
		float compZ = m_ctrlPoint[compIdx].z;

		if (m_itemType < UPPER0)
			m_ctrlPoint[m_itemType].z = (z > compZ) ? compZ : z;
		else
			m_ctrlPoint[m_itemType].z = (z < compZ) ? compZ : z;

		m_isChangeArea = m_isChangeCtrl = true;
	}
}

void CW3SurfaceArchItem::setOriginMatrix(GLuint program) {
	m_model = m_transform.reorien*m_transform.scale*m_model;
	mat4 mv = m_view * m_transform.arcball*m_model;
	WGLSLprogram::setUniform(program, "ModelViewMatrix", mv);
	WGLSLprogram::setUniform(program, "NormalMatrix",
							 glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	WGLSLprogram::setUniform(program, "MVP", m_projection * mv);
}
