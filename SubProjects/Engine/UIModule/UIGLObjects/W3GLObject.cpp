#include "W3GLObject.h"
/*=========================================================================

File:			class CW3GLObject
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-20
Last date:		2016-04-20

=========================================================================*/
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/WGLHeaders.h"

CW3GLObject::CW3GLObject(GLenum drawMode)
	: m_drawMode(drawMode) {}

CW3GLObject::~CW3GLObject() {
	clearVAOVBO();
}

void CW3GLObject::setMVP(const glm::mat4& M, const glm::mat4& V, const glm::mat4& P) {
	m_model = M;
	m_view = V;
	m_projection = P;

	m_mv = m_view * m_model;
	m_mvp = m_projection * m_mv;
}

void CW3GLObject::render() {
	if (!m_vao) {
		common::Logger::instance()->Print(common::LogType::DBG,
										  "vao is null.");
		return;
	}

	if (!m_isDrawArrays) {
		glBindVertexArray(m_vao);
		glDrawElements(
			m_drawMode,			// mode
			m_Nindices,			// count
			GL_UNSIGNED_INT,	// type
			(void*)0			// element array buffer offset
		);
	} else {
		glDrawArrays(m_drawMode, 0, m_Nindices);
	}
	glBindVertexArray(0);
}

void CW3GLObject::render(int first, int count) {
	if (!m_vao) {
		common::Logger::instance()->Print(common::LogType::DBG,
										  "vao is null.");
		return;
	}

	glBindVertexArray(m_vao);
	if (!m_isDrawArrays) {
		glDrawRangeElements(
			GL_TRIANGLES,
			first,
			first + m_Nindices,
			m_Nindices,
			GL_UNSIGNED_INT,
			(void*)0
		);
	} else if (m_vbo.size() == 2) {
		glDrawArrays(m_drawMode, first, count);
	}
	glBindVertexArray(0);
}

void CW3GLObject::clearVAOVBO() {
	if (m_vao) {
		glDeleteVertexArrays(1, &m_vao);
		m_vao = 0;
	}

	if (m_vbo.size()) {
		glDeleteBuffers(m_vbo.size(), &m_vbo[0]);
		m_vbo.clear();
	}

	m_Nindices = 0;
}
