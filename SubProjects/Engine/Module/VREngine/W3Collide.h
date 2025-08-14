#pragma once
/*=========================================================================

File:			class CW3Collide
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-12-06
Last modify:	2016-12-06


2018.06.04 TODO. by thyoo:
collide.h, collide.cpp파일로 변경해야함. 삭제 될 클래스.
=========================================================================*/
#include <vector>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "vrengine_global.h"

class CW3GLObject;

/// <summary>
/// 3D Surface간의 충돌 검사 클래스.
/// </summary>

class VRENGINE_EXPORT CW3Collide
{
private:
	typedef struct _SUBTRIANGLE {
		int count = 0;
		int index = 0;

		bool operator ==(const _SUBTRIANGLE& arg) {
			return (this->index == arg.index);
		}
	} SubTriangles;

public:
	CW3Collide();
	~CW3Collide() {};

	/// <summary>
	/// Run collision detection.
	/// this function must be called after makecurrent.
	/// </summary>
	/// <param name="progShader">The program shader.</param>
	/// <param name="matProjView">The matrix. projtion and view.</param>
	/// <param name="objs">The objects.</param>
	/// <param name="objModelMats">The object model matrix.</param>
	/// <param name="outColliObjIDs">output. collision object IDs</param>
	void run(GLuint progShader, const glm::mat4& matProjView,
		const std::vector<CW3GLObject*>& objs_, const std::vector<glm::mat4>& objModelMats,
		std::vector<int>& outColliObjIDs);

	/// <summary>
	/// Run collision detection.
	/// this function must be called after makecurrent.
	/// </summary>
	/// <param name="progShader">The program shader.</param>
	/// <param name="matProjView">The matrix. projtion and view.</param>
	/// <param name="objs">The objects.</param>
	/// <param name="objModelMats">The object model matrix.</param>
	/// <param name="outColliObjIDs">output. collision object IDs</param>
	/// <param name="outColliSub">output. collision triangle in object</param>
	void run(GLuint progShader, const glm::mat4& matProjView,
		const std::vector<CW3GLObject*>& objs_, const std::vector<glm::mat4>& objModelMats,
		std::vector<int>& outColliObjIDs,
		std::vector<std::vector<SubTriangles>>& outColliSub);

private:
	enum AlignAxis { X, Y, Z };

	void setupPCS_ObjLevel(std::vector<int>& PCS);
	void setupPCS_SubLevel(std::vector<std::vector<SubTriangles>>& objTs, bool isDivide = true);

	void divideSubTriangle(std::vector<std::vector<SubTriangles>>& objTs, int samples = 2);

private:

	glm::mat4 m_rotAxis[3];

	GLuint m_progShader;
	glm::mat4 m_matProjView;

	std::vector<CW3GLObject*> m_objs;
	std::vector<glm::mat4> m_objModelMats;

	std::vector<CW3GLObject*> m_subObjs;
	std::vector<glm::mat4> m_subObjModelMats;
	std::vector<int> m_subObjDividedN;

	std::vector<int> m_subObjMaxLevel;

	AlignAxis m_curAxis;

	int m_subIterEnd;
	int m_subIter;

	bool m_isOutColliTriangle;

	int cntQuery;
	std::vector<float> eDebug;
};
