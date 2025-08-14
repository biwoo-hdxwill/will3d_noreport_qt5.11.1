#pragma once

/*=========================================================================

File:			class CW3SurfaceArchItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-02-22
Last modify:	2016-02-24

=========================================================================*/
#include "W3SurfaceItem.h"

#include <QPointF>
class CW3VBOSphere;

class UIGLOBJECTS_EXPORT CW3SurfaceArchItem : public CW3BaseSurfaceItem
{
	enum eItemType
	{
		ITEM_UNKNOWN_TYPE = -1, 
		LOWER0,
		LOWER1,
		LOWER2,
		UPPER0,
		UPPER1,
		UPPER2,
		ITEM_TYPE_END,
	};

public:
	CW3SurfaceArchItem();
	~CW3SurfaceArchItem();

public:
	void clear();
	void clearVAOVBO();

	void initializeArch(const std::vector<vec3>& points);
	void draw(GLuint program, GLenum cullFace = GL_BACK, bool isTransform = true);
	void drawControl(GLuint program, GLenum cullFace = GL_BACK);
	void drawOutline(GLuint program);
	void translateControl(const glm::vec3& transGLCoord);
	void pick(const QPointF& pickPoint, bool* isUpdateGL, GLuint program);

	bool isPicking() { return (static_cast<int>(m_itemType) >= 0) ? true : false; }

	std::vector<glm::vec3> getControlPoints() { return m_ctrlPoint; }

	//void getObjectPara(glm::vec4* objectpara, int& objectcnt);


private:
	void initializeVBO(void);
	void setOriginMatrix(GLuint program);

private:
	unsigned int m_vaoArch = 0;
	std::vector<unsigned int> m_vboArch;

	unsigned int m_vaoOutline = 0;
	std::vector<unsigned int> m_vboOutline;

	std::vector<glm::vec3> m_oriCtrlPoint;
	std::vector<glm::vec3> m_ctrlPoint;
	std::vector<CW3VBOSphere*> m_ctrlItem;

	std::vector<unsigned int> m_archIndices;
	std::vector<unsigned int> m_outlineIndices;
	eItemType m_itemType;
	glm::vec3  m_pickPos;

	bool	m_isChangeArea = false;
	bool	m_isChangeCtrl = false;

	//glm::vec4 m_objectpara[256]; //boolean operator
	int m_objectCnt;
};
