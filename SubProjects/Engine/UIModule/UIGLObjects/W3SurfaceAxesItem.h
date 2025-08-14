#pragma once
/*=========================================================================

File:			class CW3SurfaceAxesItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-29
Last modify:	2016-02-18

=========================================================================*/
#include <qpoint.h>
#include <qpair.h>
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "W3SurfaceItem.h"

class CW3VBOSTL;

class UIGLOBJECTS_EXPORT CW3SurfaceAxesItem : public CW3BaseSurfaceItem {
	enum eItemType {
		ITEM_UNKNOWN_TYPE = -1,
		X_AXIS_ARROW,
		Y_AXIS_ARROW,
		Z_AXIS_ARROW,
		X_AXIS_TORUS,
		Y_AXIS_TORUS,
		Z_AXIS_TORUS,
		ITEM_TYPE_END,
	};

public:
	CW3SurfaceAxesItem();
	~CW3SurfaceAxesItem();

public:
	void clearVAOVBO();

	void draw(GLuint program);

	glm::vec3 translate(const glm::vec3& transGLCoord, bool is_move_axes = true);
	QPair<float, glm::vec3> rotate(const glm::vec3& curGLCoord, const glm::vec3& lastGLCoord, bool is_move_axes = true);

	void render_for_pick(GLuint program);
	void pick(const QPointF & pickPoint, bool * isUpdateGL, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);
	const bool isPicking() { return (static_cast<int>(m_itemType) >= 0) ? true : false; }
	const bool isSelectTranslate(void) const;
	const bool isSelectRotate(void) const;

private:
	void initializeAxes();

private:
	CW3VBOSTL* m_pArrow = nullptr;
	CW3VBOSTL* m_pTorus = nullptr;

	eItemType m_itemType = ITEM_UNKNOWN_TYPE;
	glm::vec3 m_pickPos = vec3(0.0f, 0.0f, 0.0f);
};
