#pragma once
/*=========================================================================

File:			class CW3SurfaceEllipseItem
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-06-16
Last modify:	2016-06-16

=========================================================================*/
#include <map>
#include <qstring.h>
#include "W3SurfaceItem.h"

class UIGLOBJECTS_EXPORT CW3SurfacePlaneItem : public CW3BaseSurfaceItem
{
public:
	CW3SurfacePlaneItem();
	~CW3SurfacePlaneItem();
	 
public:
	void draw(GLuint program);

	void drawSliceTexture(GLuint progSlice);
	void drawSliceTexture(const QString& planeName, GLuint progSlice);
	void drawOutline(GLuint progSurface);
	void drawOutline(const QString& planeName, GLuint progSurface);

	void clearVAOVBO();

	void addPlane(const QString& planeName, const glm::vec4& eqa);
	void editPlane(const QString& planeName, const glm::vec4 & eqa);

	void setPlaneRightVector(const QString& planeName, const glm::vec3& rightVector);

	void erasePlane(const QString& planeName);
	void deleteAllPlanes();
	glm::vec4 getPlaneEquation(const QString& planeName);

	inline void setVisibleItems(bool isVisible) {
		for (auto& elem : m_isVisible)
			elem.second = isVisible;
	}
	inline void setVisibleItem(const QString& text, bool isVisible) {
		if (m_isVisible.find(text) != m_isVisible.end())
			m_isVisible[text] = isVisible;
	}

	void setVolTexBias(const glm::mat4& bias) { m_volTexBias = bias; }

	void setVertexCoord(float coord) { m_coordVertex = coord; }

protected:
	unsigned int m_vaoPlane;
	std::vector<unsigned int> m_vboPlane;

	unsigned int m_vaoOutline;
	std::vector<unsigned int> m_vboOutline;

	std::vector<unsigned int> m_indicesPlane;
	std::vector<unsigned int> m_outlineIndices;

private:
	void createRect();

	void setPlaneMatrix(const QString& planeName);
	bool checkErrPlane(const QString & planeName);
	///	v1, v2이 시점은 원점이라고 가정하고 v1, v2 normalize한 다음 v1 에서 v2로 옮기는 회전행렬
	glm::mat4 rotVec2Vec(const glm::vec3& v1, const glm::vec3& v2);
	glm::mat4 rotWithAxis(const glm::vec3& axis, float th);

private:
	glm::vec3 m_oriNormal;
	glm::vec3 m_oriRight;
	std::map<QString, glm::vec4> m_planeEquations;
	std::map<QString, glm::vec3> m_planeRightVector;
	std::map<QString, bool> m_isVisible;

	bool m_isReady;
	glm::mat4 m_volTexBias;


	/** The coord vertex 
	볼륨이 (-1 ~ 1) 좌표 공간 안에 있지만 vertex를 (-1 ~ 1)보다 크거나 작게 잡을 때 
	setVertexCoord함수를 사용해서 변경한다.
	*/
	float m_coordVertex = 1.0f; 
};
