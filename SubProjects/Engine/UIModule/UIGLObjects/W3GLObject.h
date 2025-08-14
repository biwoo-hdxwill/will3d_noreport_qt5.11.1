#pragma once
/*=========================================================================

File:			class CW3GLObject
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-20
Last date:		2016-04-20

=========================================================================*/
#include <vector>

#include <GL/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "uiglobjects_global.h"
#include "uiglobjects_defines.h"

class UIGLOBJECTS_EXPORT CW3GLObject
{
public:
	CW3GLObject(GLenum drawMode = GL_TRIANGLES);
	~CW3GLObject();

public:
	void render();
	void render(int first, int count);

	void clearVAOVBO();

	void setVAO(const uint& vao) { m_vao = vao; }
	void setVBO(const std::vector<uint>& vbo) { m_vbo = vbo; }
	void setNindices(const uint& nIndices) { m_Nindices = nIndices; }
	void setTexHandler(const uint& texHandler) { m_texHandler = texHandler; }
	void setAlpha(const float& alpha) { m_alpha = alpha; }
	void setTransparent(const bool& enable) { m_isTransparent = enable; }
	void setVisible(const bool& isVisible) { m_isShown = isVisible; }

	//void setMVP(const glm::mat4& mvp) { m_mvp = mvp; }
	//void setMV(const glm::mat4& mv) { m_mv = mv; }
	void setMVP(const glm::mat4& M, const glm::mat4& V, const glm::mat4& P);
	void setInvModel(const glm::mat4& invModel) { m_invModel = invModel; }
	void setMeshColor(const glm::vec3& color) { m_meshColor = color; }

	void setDrawArrays(const bool enable) { m_isDrawArrays = enable; }

	inline const uint getVAO() const noexcept { return m_vao; }
	inline const uint getNindices() const noexcept { return m_Nindices; }
	inline const uint getTexHandler() const noexcept { return m_texHandler; }
	inline const float getAlpha() const noexcept { return m_alpha; }
	inline const bool isTransparent() const noexcept { return m_isTransparent; }
	inline const bool isVisible() const noexcept { return m_isShown; }
	inline const GLenum getDrawMode() const noexcept { return m_drawMode; }

	inline const glm::mat4& getMVP() const noexcept { return m_mvp; }
	inline const glm::mat4& getMV() const noexcept { return m_mv; }
	inline const glm::mat4& getModel() const noexcept { return m_model; }
	inline const glm::mat4& getView() const noexcept { return m_view; }
	inline const glm::mat4& getProjection() const noexcept { return m_projection; }
	inline const glm::mat4& getInvModel() const noexcept { return m_invModel; }
	inline const glm::vec3& getMeshColor() const noexcept { return m_meshColor; }

protected:
	uint m_vao = 0;
	std::vector<uint> m_vbo;

	GLenum	m_drawMode = GL_TRIANGLES;

	uint	m_Nindices = 0;
	uint	m_texHandler = 0;
	float	m_alpha = 1.0f;

	glm::mat4	m_model = glm::mat4(1.0f);
	glm::mat4	m_view = glm::mat4(1.0f);
	glm::mat4	m_projection = glm::mat4(1.0f);
	glm::mat4	m_mvp = glm::mat4(1.0f);
	glm::mat4	m_mv = glm::mat4(1.0f);
	glm::mat4	m_invModel = glm::mat4(1.0f);

	glm::vec3	m_meshColor = glm::vec3(1.0f, 0.0f, 0.0f);
	
	bool m_isTransparent = false;
	bool m_isShown = false;
	bool m_isDrawArrays = false;
};

