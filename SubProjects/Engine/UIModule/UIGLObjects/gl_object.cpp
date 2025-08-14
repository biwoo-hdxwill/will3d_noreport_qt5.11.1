#include "gl_object.h"

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "../../Common/GLfunctions/W3GLFunctions.h"

GLObject::GLObject(GLenum drawMode)
	: draw_mode_(drawMode) {
}

GLObject::~GLObject() {
	ClearVAOVBO();
}
void GLObject::Render() {
	if (!vao_) {
		Initialize();
	}

	glDisable(GL_CULL_FACE);

	glBindVertexArray(vao_);
	glDrawElements(
		draw_mode_,			// mode
		cnt_indices_,			// count
		GL_UNSIGNED_INT,	// type
		(void*)0			// element array buffer offset
	);

	glBindVertexArray(0);
}

void GLObject::Render(GLenum cullFace) {
	if (!vao_)
	{
		Initialize();
	}

	glEnable(GL_CULL_FACE);
	glCullFace(cullFace);

	glBindVertexArray(vao_);
	glDrawElements(
		draw_mode_,			// mode
		cnt_indices_,			// count
		GL_UNSIGNED_INT,	// type
		(void*)0			// element array buffer offset
	);

	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
}

void GLObject::Render(int first, int count) {
	if (!vao_) {
		Initialize();
	}

	glBindVertexArray(vao_);
	glDrawRangeElements(
		GL_TRIANGLES,
		first,
		first + cnt_indices_,
		cnt_indices_,
		GL_UNSIGNED_INT,
		(void*)0
	);
	glBindVertexArray(0);
}

void GLObject::RenderWire(const float line_width)
{
	if (!vao_)
	{
		Initialize();
	}

	glDisable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(line_width);

	glBindVertexArray(vao_);

	glDrawElements(GL_TRIANGLES, cnt_indices_, GL_UNSIGNED_INT, (GLuint *)NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GLObject::ClearVAOVBO() {
	if (vao_) {
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}

	if (vbo_.size()) {
		glDeleteBuffers(vbo_.size(), &vbo_[0]);
		vbo_.clear();
	}

	cnt_indices_ = 0;
}

void GLObject::InitVAOVBOtexture(const std::vector<glm::vec3>& vert_coord,
								 const std::vector<glm::vec2>& tex_coord,
								 const std::vector<unsigned int>& indices) {
	ClearVAOVBO();

	vbo_.resize(3, 0);
	cnt_indices_ = indices.size();

	CW3GLFunctions::initVAOVBO(&vao_, &vbo_[0], vert_coord, tex_coord, indices);
}

void GLObject::InitVAOVBOmesh(const std::vector<glm::vec3>& vert_coord,
							  const std::vector<glm::vec3>& normal,
							  const std::vector<unsigned int> indices) {
	ClearVAOVBO();

	vbo_.resize(3, 0);
	cnt_indices_ = indices.size();

	CW3GLFunctions::initVAOVBO(&vao_, &vbo_[0], vert_coord, normal, indices);
}
