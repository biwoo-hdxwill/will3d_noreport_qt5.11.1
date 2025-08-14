#pragma once
/**=================================================================================================

Project: 			UIGLObjects
File:				view_plane_obj_gl.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-02
Last modify:		2017-08-02

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

#include "gl_object.h"
class UIGLOBJECTS_EXPORT ViewPlaneObjGL : public GLObject 
{
public:
	ViewPlaneObjGL();
	~ViewPlaneObjGL();

public:
	inline void set_vertex_scale(const glm::vec3& scale) { ver_tex_scale_ = scale; }

	virtual void Initialize() override;

private:
	glm::vec3 ver_tex_scale_ = glm::vec3(1.0f);
};
