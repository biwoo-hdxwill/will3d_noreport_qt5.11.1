#pragma once
/*=========================================================================

File:			class CW3Spline3DItem
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-02-19
Last modify:	2016-04-20

Copyright (c) 2016 All rights reserved by HDXWILL.
=========================================================================*/
#include <vector>
#include <qpoint.h>

#include "../../Common/GLfunctions/WGLHeaders.h"

#include "uiglobjects_global.h"

///////////////////////////////////////////////////////////////////////////
//
//	* CW3Spline3DItem
//	3D Spline을 그리는 class
//	3D control point들을 입력하면 spline을 등간격으로 만들어
//	control point는 점으로, spline 은 선으로 opengl rendering
//	현재 위치와 카메라 방향을 spline위에 rendering
//
///////////////////////////////////////////////////////////////////////////

class UIGLOBJECTS_EXPORT CW3Spline3DItem {
public:
	CW3Spline3DItem();
	~CW3Spline3DItem();

	void draw(GLuint program, const mat4& mvp);
	void drawModifyMode(GLuint program, const mat4& mvp, int modifyPointIndex);
	void addPoint(glm::vec3 point);
	void moveLastPoint(glm::vec3 point);
	void movePoint(int index, glm::vec3 point);
	void removeLastPoint();
	void generateSpline();
	void setVolRange(glm::vec3 volRange);
	void setPlaneDepth(float depth);
	void clear();
	void clearVAOVBO();
	void translatePath(glm::vec3 dist);
	void setModifyMode(bool modify);
	void setCameraPos(vec3 eye, vec3 dir);
	void setCameraPosPoint(int index, bool isControlPoint);
	int pick(const QPointF& pickPoint, GLuint program, const mat4& mvp);
	int pickPoint(const QPointF& point, GLuint program, const mat4& mvp, int targetIndex);
	int pickLine(const QPointF& point, GLuint program, const mat4& mvp);
	int getSelectedPoint();
	bool getSelectedControlPointDepth(float &depth);
	bool getControlPointDepth(int index, float &depth);
	bool getPathPointDepth(int index, float &depth);
	bool isModifyMode();
	bool isEnd();
	bool endEdit();
	bool isPathValid();
	std::vector<glm::vec3> *getPath();
	std::vector<glm::vec3> *getControlPoint();
	std::vector<glm::vec3> getControlPointData();
	std::vector<glm::vec3> *getNormalArchData();

private:
	void equidistanceSpline(std::vector<glm::vec3> &out, std::vector<glm::vec3> &normal, std::vector<glm::vec3> &in, glm::vec3 &TopTranslate, glm::vec3 &upVector);
	void drawPath(GLuint program, const mat4& mvp, float lineWidth, const glm::vec4& color, bool useModifyMode);
	void drawControlPoints(GLuint program, const mat4& mvp, float pointSize, const glm::vec4& color, bool useModifyMode);
	void drawCameraPos(GLuint program, const mat4& mvp, const glm::vec4& posColor, const glm::vec4& dirColor, bool useModifyMode);
	int readPickInfo(int x, int y);

	unsigned int m_vboLines[1];	// spline을 그리기 위한 vbo
	unsigned int m_vboPoints[1];	// control point를 그리기 위한 vbo
	unsigned int m_vboCameraDir[1];	// 현재 위치에서의 카메라 방향을 그리기 위한 vbo
	unsigned int m_vboCameraPosPoint[1];	// 현재 위치를 나타내는 점을 그리기 위한 vbo

	unsigned int m_vaoLines = 0;	// spline을 그리기 위한 vao
	unsigned int m_vaoPoints = 0;	// control point를 그리기 위한 vao
	unsigned int m_vaoCameraDir = 0;	// 현재 위치에서의 카메라 방향을 그리기 위한 vao
	unsigned int m_vaoCameraPosPoint = 0;	// 현재 위치를 나타내는 점을 그리기 위한 vao

	int m_nPickedPointIndex = -1;	// 선택된 control point index
	int m_nModifyPointIndex = -1;	// CW3View3DEndoModify에서 수정할 control point index

	bool m_bIsModifyMode = false;	// CW3View3DEndoModify에서 수정 mode인지 여부

	float m_fPlaneDepth = 1.0f;	// spline이 특정 plane의 앞/뒤인지 판별하기 위한 기준 depth 값

	bool m_bIsEndEdit = false;	// spline 그리기가 완료 되었는지 여부

	std::vector<glm::vec3> m_vPath;	// spline point set
	std::vector<glm::vec3> m_vControlPoint;	// control point set
	std::vector<glm::vec3> m_vNormalArchData;	// 각 spline point의 normal vector

	glm::vec3 m_volRange;	// volume 각 축간의 상대적 비율

	vec3 m_eye = vec3(0.0f);	// 카메라 위치
	vec3 m_dir = vec3(0.0f);	// 카메라 방향

	vec4 m_lineColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);	// spline color
	vec4 m_pointColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);	// control point color
	vec4 m_cameraPosColor = vec4(0.0f, 1.0f, 0.0f, 0.5f);	// 카메라 위치를 표시하는 점의 color
	vec4 m_cameraDirColor = vec4(1.0f, 0.0f, 1.0f, 0.8f);	// 카메라 방향을 표시하는 선의 color
};
