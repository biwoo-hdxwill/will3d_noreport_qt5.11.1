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
#include "uiglobjects_defines.h"

class ImplantData;

class UIGLOBJECTS_EXPORT ImplantObjGL : public GLObject {
public:
	ImplantObjGL(int implant_id);
	~ImplantObjGL();

public:
	virtual void Initialize() override;

	void GetLengthAndDiameter(float * diameter, float * length) const;

	void GetBoundingBoxRange(glm::vec3* max, glm::vec3* min) const;
	glm::mat4 GetTranslate(UIGLObjects::ObjCoordSysType type) const;
	glm::mat4 GetRotate(UIGLObjects::ObjCoordSysType type) const;
	const bool& IsCollided() const;
	bool IsSelected() const;
	bool IsValid() const;
	inline const int& id() const { return id_; }

private:
	const ImplantData& GetImplantDataFromResource() const;

private:
	float ver_tex_scale_ = 1.0f;
	int id_ = -1;
};
