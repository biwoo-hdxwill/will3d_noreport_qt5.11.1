#pragma once
/*=========================================================================

File:			class CW3GLNerve
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-05-27
Last date:		2016-05-27

=========================================================================*/
#include "../../Common/GLfunctions/WGLHeaders.h"

#include "W3GLObject.h"

class UIGLOBJECTS_EXPORT CW3GLNerve : public CW3GLObject {
public:
	CW3GLNerve();
	~CW3GLNerve();

public:
	void setUniformColor(GLuint program);

	inline void setMaterial(const UIGLObjects::Material& material) { m_material = material; }

private:
	UIGLObjects::Material m_material;
};
