#pragma once

/**=================================================================================================

Project: 			UIGLObjects
File:				gl_object.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-02
Last modify:		2017-08-02

 *===============================================================================================**/
#include <vector>

#include <GL/glew.h>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/vec2.hpp>
#endif

#include <QObject>
#include "uiglobjects_global.h"

class UIGLOBJECTS_EXPORT GLObject : public QObject {
	Q_OBJECT

public:
	GLObject(GLenum drawMode = GL_TRIANGLES);
	virtual ~GLObject();

	GLObject(const GLObject&) = delete;
	GLObject& operator=(const GLObject&) = delete;

public:
	virtual void Render();
	virtual void Render(GLenum cullFace);
	virtual void Render(int first, int count);
	virtual void RenderWire(const float line_width = 1.0f);

	virtual void ClearVAOVBO();

	GLuint vao() const { return vao_; }

	void InitVAOVBOtexture(const std::vector<glm::vec3>& vert_coord,
						   const std::vector<glm::vec2>& tex_coord,
						   const std::vector<unsigned int>& indices);
	void InitVAOVBOmesh(const std::vector<glm::vec3>& vert_coord,
						const std::vector<glm::vec3>& normal,
						const std::vector<unsigned int> indices);

	inline uint cnt_indices() const { return cnt_indices_; }
	inline GLenum draw_mode() const { return draw_mode_; }
signals:
	void sigInitialize();

protected:
	virtual void Initialize() { emit sigInitialize(); };

private:
	GLuint vao_ = 0;
	std::vector<GLuint> vbo_;
	uint	cnt_indices_ = 0;

	GLenum	draw_mode_ = GL_TRIANGLES;
};
