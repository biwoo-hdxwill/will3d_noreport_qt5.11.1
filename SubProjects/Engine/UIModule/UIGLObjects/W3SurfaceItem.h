#pragma once
/*=========================================================================

File:			class CW3SurfaceItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-29
Last modify:	2016-04-25

=========================================================================*/
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLTypes.h"

#include "W3GLObject.h"
#include "uiglobjects_global.h"

class UIGLOBJECTS_EXPORT CW3BaseSurfaceItem : protected CW3GLObject {
public:
	CW3BaseSurfaceItem();
	virtual ~CW3BaseSurfaceItem();

public:
	//virtual void clearVBOs();
	void editTransformMat(const glm::mat4& matrix, UIGLObjects::TransformType type);
	void setTransformMat(const glm::mat4& matrix, UIGLObjects::TransformType type);
	glm::mat4 getTransformMat(UIGLObjects::TransformType type);
	glm::mat4 getSaveTransformMat(UIGLObjects::TransformType type);
	glm::mat4 getVolTexTransformMat(void);
	inline const glm::vec3& getCentroid(void) { return m_centroid; }

	inline void setProjViewMat(const glm::mat4& projection, const glm::mat4& view) { m_projection = projection; m_view = view; }
	inline void setColor(const UIGLObjects::Material& color) { m_material = color; }
	inline const glm::mat4& getMVP() const { return m_mvp; }
	inline const glm::mat4& getVolumeMatrix() const { return m_model; }
	inline const glm::mat4& getCameraMatrix() const { return m_view; }
	inline const glm::mat4& getProjection() const { return m_projection; }

	void setAlpha(float alpha);
	inline const bool isTransparency() const { return m_isTransparency; }

	bool pushModelMat();
	bool popModelMat();
	inline void setShown(bool enable) { m_isShown = enable; }
	bool isShown() { return (this != nullptr) ? m_isShown : false; }

protected:
	void setUniformColor(GLuint program);
	void setMatrix(GLuint program);
	void readPickInfo(int x, int y, unsigned char* _pickID, glm::vec3* _pickGLCoord, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

protected:
	UIGLObjects::TransformMat m_transform;
	UIGLObjects::TransformMat m_saveTransform;
	glm::vec3 m_centroid = glm::vec3(0.0f);

	bool m_isPushModel = false;

	UIGLObjects::Material m_material;

	bool m_isTransparency = false;
	float m_alpha = 1.0f;

	bool m_isShown = false;
};

class UIGLOBJECTS_EXPORT CW3SurfaceItem : public CW3BaseSurfaceItem {
public:
	CW3SurfaceItem();
	virtual ~CW3SurfaceItem();

public:
	void initSurfaceFillColor(const std::vector<glm::vec3>& vertices, const std::vector<unsigned int>& indices, UIGLObjects::Orientation orien = UIGLObjects::Orientation::CCW);
	void initSurfaceFillTexture(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec2>& texCoords, const std::vector<unsigned int>& indices, UIGLObjects::Orientation orien = UIGLObjects::Orientation::CCW);
	void clearVAOVBO();
	void draw(GLuint program);
	void draw(GLuint program, GLenum cullFace);

	const bool isReadyVAO() const;
};
