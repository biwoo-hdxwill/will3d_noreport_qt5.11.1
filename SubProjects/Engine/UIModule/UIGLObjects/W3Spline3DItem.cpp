#include "W3Spline3DItem.h"
/*=========================================================================

File:			class CW3Spline3DItem
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun
First date:		2016-02-19
Last modify:	2016-04-07

=========================================================================*/
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/common.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
namespace {
const float kPointSize = 8.0f;
const float kLineWidth = 2.0f;
} // end of namespace

CW3Spline3DItem::CW3Spline3DItem() {
	m_vboLines[0] = 0;
	m_vboPoints[0] = 0;
	m_vboCameraDir[0] = 0;
	m_vboCameraPosPoint[0] = 0;
}

CW3Spline3DItem::~CW3Spline3DItem() {
	clear();
}

///////////////////////////////////////////////////////////////////////////
//	volume 각 축간의 비율을 설정
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::setVolRange(glm::vec3 volRange) {
	m_volRange = volRange;
}

///////////////////////////////////////////////////////////////////////////
//	spline, control point들이 특정 plane의 앞/뒤에 있는지를 확인하기 위한
//	기준 depth를 설정
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::setPlaneDepth(float depth) {
	m_fPlaneDepth = depth;
}

///////////////////////////////////////////////////////////////////////////
//	spline을 만들기 위한 control point를 컨테이너에 추가
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::addPoint(glm::vec3 point) {
	m_vControlPoint.push_back(point);
}

///////////////////////////////////////////////////////////////////////////
//	현재 control point set의 마지막 point를 설정한 point 좌표로 이동
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::moveLastPoint(glm::vec3 point) {
	movePoint((int)m_vControlPoint.size() - 1, point);
}

///////////////////////////////////////////////////////////////////////////
//	해당 index의 control point 위치를 설정한 point 좌표로 이동
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::movePoint(int index, glm::vec3 point) {
	if (m_vControlPoint.size() > index)
		m_vControlPoint.at(index) = point;
}

///////////////////////////////////////////////////////////////////////////
//	현재 control point set의 마지막 point를 삭제
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::removeLastPoint() {
	if (!m_vControlPoint.empty())
		m_vControlPoint.pop_back();
}

///////////////////////////////////////////////////////////////////////////
//	해당 program과 mvp를 가지고 control point, spline, camera pos를 rendering
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::draw(GLuint program, const mat4& mvp) {
	if (!isPathValid())
		return;

	m_nModifyPointIndex = -1;

	drawPath(program, mvp, kLineWidth, m_lineColor, false);
	drawControlPoints(program, mvp, kPointSize, m_pointColor, false);
	glEnable(GL_DEPTH_TEST);
	drawCameraPos(program, mvp, m_cameraPosColor, m_cameraDirColor, false);
	glDisable(GL_DEPTH_TEST);
}

///////////////////////////////////////////////////////////////////////////
//	해당 program과 mvp를 가지고 control point, spline, camera pos를 rendering
//	단, modifyPointIndex에 해당하는 control point만 picking 검사
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::drawModifyMode(GLuint program, const mat4& mvp, int modifyPointIndex) {
	if (!isPathValid())
		return;

	m_nModifyPointIndex = modifyPointIndex;

	drawPath(program, mvp, kLineWidth, m_lineColor, true);
	drawControlPoints(program, mvp, kPointSize, m_pointColor, true);
	glEnable(GL_DEPTH_TEST);
	drawCameraPos(program, mvp, m_cameraPosColor, m_cameraDirColor, true);
	glDisable(GL_DEPTH_TEST);
}

///////////////////////////////////////////////////////////////////////////
//	저장된 control point set을 가지고 spline과 각 spline point의 normal vector를 생성
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::generateSpline() {
	if (m_vControlPoint.size() < 2)
		return;

	std::vector<glm::vec3> VolCoordArch;
	Common::generateCubicSpline(m_vControlPoint, VolCoordArch);

	int N = (int)VolCoordArch.size() - 1;
	//printf("N: %d\n", VolCoordArch.size());
	//printf("generateSpline Path (x, y, z) = (%f, %f, %f)\n", VolCoordArch.at(N).x, VolCoordArch.at(N).y, VolCoordArch.at(N).z);

	m_vPath.clear();
	m_vNormalArchData.clear();

	glm::vec3 upVector;
	glm::vec3 TopTransInVol_(0.0f, 0.0f, 0.0f);
	glm::vec4 TopTransInVol;
	glm::mat4 T;

	TopTransInVol = T * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	upVector = glm::normalize(glm::vec3(TopTransInVol.xyz));

	// spline을 등간격으로 설정
	equidistanceSpline(m_vPath, m_vNormalArchData, VolCoordArch, TopTransInVol_, upVector);

	N = (int)m_vPath.size() - 1;
	//printf("N: %d\n", N);
	//printf("generateSpline Ctrl (x, y, z) = (%f, %f, %f)\n", m_vControlPoint.at(1).x, m_vControlPoint.at(1).y, m_vControlPoint.at(1).z);

	//printf("generateSpline Path (x, y, z) = (%f, %f, %f)\n", m_vPath.at(N).x, m_vPath.at(N).y, m_vPath.at(N).z);
}

///////////////////////////////////////////////////////////////////////////
//	3D picking
//	마우스 위치에서 선택된 object index를 return
//	선택된 object index는 각 control point의 index와 같고
//	control point index + 1 이라면 spline이 선택된 것임
///////////////////////////////////////////////////////////////////////////
int CW3Spline3DItem::pick(const QPointF& point, GLuint program, const mat4& mvp) {
	pickLine(point, program, mvp);

	if (m_nPickedPointIndex == 0)
		return m_nPickedPointIndex;

	for (int i = 0; i < m_vControlPoint.size(); i++)
		pickPoint(point, program, mvp, i);

	return m_nPickedPointIndex;
}

///////////////////////////////////////////////////////////////////////////
//	3D picking
//	targetIndex에 해당하는 control point 하나에 대한 picking 검사
///////////////////////////////////////////////////////////////////////////
int CW3Spline3DItem::pickPoint(const QPointF& point, GLuint program, const mat4& mvp, int targetIndex) {
	if (!isPathValid())
		return -1;

	glUseProgram(program);
	WGLSLprogram::setUniform(program, "MVP", mvp);

	float pVertPointsCoord[3];
	int sizeVertPoints = 3;

	WGLSLprogram::setUniform(program, "Index", targetIndex + 1);	// 각 point 구분을 위한 index를 각각 다르게 전달

	if (m_vboPoints[0]) {
		glDeleteBuffers(1, m_vboPoints);
		m_vboPoints[0] = 0;
	}

	glGenBuffers(1, m_vboPoints);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboPoints[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVertPoints * sizeof(float), (float*)&(m_vControlPoint.at(targetIndex)), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_vaoPoints) {
		glDeleteVertexArrays(1, &m_vaoPoints);
		m_vaoPoints = 0;
	}

	if (m_vaoPoints) {
		glDeleteVertexArrays(1, &m_vaoPoints);
		m_vaoPoints = 0;
	}

	glGenVertexArrays(1, &m_vaoPoints);
	glBindVertexArray(m_vaoPoints);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboPoints[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(m_vaoPoints);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(kPointSize + 2.0f);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_POINT_SMOOTH);

	glUseProgram(0);

	m_nPickedPointIndex = readPickInfo(point.x(), point.y());

	return m_nPickedPointIndex;
}

///////////////////////////////////////////////////////////////////////////
//	3D picking
//	spline에 대한 picking 검사
///////////////////////////////////////////////////////////////////////////
int CW3Spline3DItem::pickLine(const QPointF& point, GLuint program, const mat4& mvp) {
	if (!isPathValid())
		return -1;

	int sizePath = m_vPath.size();
	float *pVertLinesCoord = nullptr;
	int sizeVertLines = ((sizePath - 1) * 2) * 3;
	W3::p_allocate_1D(&pVertLinesCoord, sizeVertLines);
	memset(pVertLinesCoord, 0.0f, sizeVertLines * sizeof(float));

	glUseProgram(program);
	WGLSLprogram::setUniform(program, "MVP", mvp);
	int numPoints = m_vControlPoint.size();
	WGLSLprogram::setUniform(program, "Index", numPoints + 1);	// spline의 picking을 위한 index는 control point와 구분하기 위해 control point의 size + 1 로 설정

	//vec3 mp1, mp2;
	for (int i = 0; i < sizePath - 1; i++) {
		//mp1 = vec3(m_vPath.at(i).x() / m_volRange.x(),
		//	m_vPath.at(i).y() / m_volRange.y(),
		//	m_vPath.at(i).z() / m_volRange.z());

		//mp2 = vec3(m_vPath.at(i + 1).x() / m_volRange.x(),
		//	m_vPath.at(i + 1).y() / m_volRange.y(),
		//	m_vPath.at(i + 1).z() / m_volRange.z());

		//mp1 = mp1 * 2.0f - 1.0f;
		//mp2 = mp2 * 2.0f - 1.0f;

		//pVertLinesCoord[i * 6] = mp1.x;		pVertLinesCoord[i * 6 + 1] = mp1.y;	pVertLinesCoord[i * 6 + 2] = mp1.z;
		//pVertLinesCoord[i * 6 + 3] = mp2.x;	pVertLinesCoord[i * 6 + 4] = mp2.y;	pVertLinesCoord[i * 6 + 5] = mp2.z;

		pVertLinesCoord[i * 6] = m_vPath.at(i).x;
		pVertLinesCoord[i * 6 + 1] = m_vPath.at(i).y;
		pVertLinesCoord[i * 6 + 2] = m_vPath.at(i).z;
		pVertLinesCoord[i * 6 + 3] = m_vPath.at(i + 1).x;
		pVertLinesCoord[i * 6 + 4] = m_vPath.at(i + 1).y;
		pVertLinesCoord[i * 6 + 5] = m_vPath.at(i + 1).z;
	}

	if (m_vboLines[0]) {
		glDeleteBuffers(1, m_vboLines);
		m_vboLines[0] = 0;
	}

	glGenBuffers(1, m_vboLines);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboLines[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVertLines * sizeof(float), pVertLinesCoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_vaoLines) {
		glDeleteVertexArrays(1, &m_vaoLines);
		m_vaoLines = 0;
	}

	glGenVertexArrays(1, &m_vaoLines);
	glBindVertexArray(m_vaoLines);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboLines[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnable(GL_LINE_SMOOTH);
	glBindVertexArray(m_vaoLines);
	glLineWidth(kLineWidth + 2.0f);
	glDrawArrays(GL_LINES, 0, (sizePath - 1) * 2);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_LINE_SMOOTH);

	SAFE_DELETE_ARRAY(pVertLinesCoord);

	glUseProgram(0);

	m_nPickedPointIndex = readPickInfo(point.x(), point.y());

	return m_nPickedPointIndex;
}

///////////////////////////////////////////////////////////////////////////
//	pick된 object index return
///////////////////////////////////////////////////////////////////////////
int CW3Spline3DItem::getSelectedPoint() {
	return m_nPickedPointIndex;
}

///////////////////////////////////////////////////////////////////////////
//	pick된 control point의 depth 값 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::getSelectedControlPointDepth(float &depth) {
	if (!isPathValid())
		return false;

	if (m_nPickedPointIndex > -1 && m_nPickedPointIndex < m_vControlPoint.size()) {
		depth = m_vControlPoint.at(m_nPickedPointIndex).x*m_volRange.x;
		return true;
	} else
		return false;
}

///////////////////////////////////////////////////////////////////////////
//	index에 해당하는 control point의 depth 값 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::getControlPointDepth(int index, float &depth) {
	if (!isPathValid())
		return false;

	if (index > 0 && index < m_vControlPoint.size()) {
		depth = m_vControlPoint.at(index).x*m_volRange.x;
		return true;
	} else
		return false;
}

///////////////////////////////////////////////////////////////////////////
//	index에 해당하는 spline point의 depth 값 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::getPathPointDepth(int index, float &depth) {
	if (!isPathValid())
		return false;

	if (index > 0 && index < m_vPath.size()) {
		depth = m_vPath.at(index).x;
		return true;
	} else
		return false;
}

///////////////////////////////////////////////////////////////////////////
//	현재 modify mode인지 여부 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::isModifyMode() {
	return m_bIsModifyMode;
}

///////////////////////////////////////////////////////////////////////////
//	spline 그리기가 종료 되었는지 여부 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::isEnd() {
	return m_bIsEndEdit;
}

///////////////////////////////////////////////////////////////////////////
//	spline 그리기를 종료하고 정상 여부 return
//	control point가 2개 미만이면 spline을 생성할 수 없으므로 false를 return
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::endEdit() {
	if (m_vControlPoint.size() < 2)
		m_bIsEndEdit = false;
	else
		m_bIsEndEdit = true;

	return m_bIsEndEdit;
}

///////////////////////////////////////////////////////////////////////////
//	spline 초기화
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::clear() {
	m_bIsEndEdit = false;
	m_bIsModifyMode = false;
	m_nPickedPointIndex = -1;
	m_nModifyPointIndex = -1;
	m_fPlaneDepth = 1.0f;

	m_eye = vec3(0.0f);
	m_dir = vec3(0.0f);

	m_vPath.clear();
	m_vControlPoint.clear();
	m_vNormalArchData.clear();

	if (m_vboLines[0]) {
		glDeleteBuffers(1, m_vboLines);
		m_vboLines[0] = 0;
	}

	if (m_vaoLines) {
		glDeleteVertexArrays(1, &m_vaoLines);
		m_vaoLines = 0;
	}

	if (m_vboPoints[0]) {
		glDeleteBuffers(1, m_vboPoints);
		m_vboPoints[0] = 0;
	}

	if (m_vaoPoints) {
		glDeleteVertexArrays(1, &m_vaoPoints);
		m_vaoPoints = 0;
	}

	if (m_vboCameraDir[0]) {
		glDeleteBuffers(1, m_vboCameraDir);
		m_vboCameraDir[0] = 0;
	}

	if (m_vaoCameraDir) {
		glDeleteVertexArrays(1, &m_vaoCameraDir);
		m_vaoCameraDir = 0;
	}

	if (m_vboCameraPosPoint[0]) {
		glDeleteBuffers(1, m_vboCameraPosPoint);
		m_vboCameraPosPoint[0] = 0;
	}

	if (m_vaoCameraPosPoint) {
		glDeleteVertexArrays(1, &m_vaoCameraPosPoint);
		m_vaoCameraPosPoint = 0;
	}
}
void CW3Spline3DItem::clearVAOVBO() {
	if (m_vboLines[0]) {
		glDeleteBuffers(1, m_vboLines);
		m_vboLines[0] = 0;
	}

	if (m_vaoLines) {
		glDeleteVertexArrays(1, &m_vaoLines);
		m_vaoLines = 0;
	}

	if (m_vboPoints[0]) {
		glDeleteBuffers(1, m_vboPoints);
		m_vboPoints[0] = 0;
	}

	if (m_vaoPoints) {
		glDeleteVertexArrays(1, &m_vaoPoints);
		m_vaoPoints = 0;
	}

	if (m_vboCameraDir[0]) {
		glDeleteBuffers(1, m_vboCameraDir);
		m_vboCameraDir[0] = 0;
	}

	if (m_vaoCameraDir) {
		glDeleteVertexArrays(1, &m_vaoCameraDir);
		m_vaoCameraDir = 0;
	}

	if (m_vboCameraPosPoint[0]) {
		glDeleteBuffers(1, m_vboCameraPosPoint);
		m_vboCameraPosPoint[0] = 0;
	}

	if (m_vaoCameraPosPoint) {
		glDeleteVertexArrays(1, &m_vaoCameraPosPoint);
		m_vaoCameraPosPoint = 0;
	}
}

///////////////////////////////////////////////////////////////////////////
//	control point 또는 spline을 dis 만큼 이동
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::translatePath(glm::vec3 dist) {
	if (m_nPickedPointIndex < 0)
		return;

	if (m_nPickedPointIndex < m_vControlPoint.size()) {
		m_vControlPoint.at(m_nPickedPointIndex) += dist;
	} else {
		for (int i = 0; i < m_vControlPoint.size(); i++) {
			m_vControlPoint.at(i) += dist;
		}
	}

	generateSpline();
}

///////////////////////////////////////////////////////////////////////////
//	control point 또는 spline을 dis 만큼 이동
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::setModifyMode(bool modify) {
	m_bIsModifyMode = modify;
}

///////////////////////////////////////////////////////////////////////////
//	카메라 위치 eye와 방향 dir을 설정
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::setCameraPos(vec3 eye, vec3 dir) {
	m_eye = eye;
	m_dir = dir;
}

///////////////////////////////////////////////////////////////////////////
//	카메라 위치를 표시할 점을 그리기 위한 vbo 설정
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::setCameraPosPoint(int index, bool isControlPoint) {
	//if (!isPathValid())
	//	return;

	//std::vector<QVector3D> element;
	//if (isControlPoint)
	//	element = m_vControlPoint;
	//else
	//	element = m_vPath;

	//int sizePoints = 1;

	////float pVertPointCoord[3];
	//int sizeVertPoint = 3;

	////vec3 mp;
	////mp = vec3(element.at(index).x() / m_volRange.x(),
	////	element.at(index).y() / m_volRange.y(),
	////	element.at(index).z() / m_volRange.z());

	////mp = mp * 2.0f - 1.0f;

	////pVertPointCoord[0] = mp.x;	pVertPointCoord[1] = mp.y;	pVertPointCoord[2] = mp.z;

	//if (m_vboCameraPosPoint[0])
	//{
	//	glDeleteBuffers(1, m_vboCameraPosPoint);
	//	m_vboCameraPosPoint[0] = 0;
	//}

	//glGenBuffers(1, m_vboCameraPosPoint);

	//glBindBuffer(GL_ARRAY_BUFFER, m_vboCameraPosPoint[0]);
	//glBufferData(GL_ARRAY_BUFFER, sizeVertPoint * sizeof(float), (float*)&(element.at(index)), GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////
//	생성된 spline point set의 포인터를 return
///////////////////////////////////////////////////////////////////////////
std::vector<glm::vec3> *CW3Spline3DItem::getPath() {
	return &m_vPath;
}

///////////////////////////////////////////////////////////////////////////
//	control point set의 포인터를 return
///////////////////////////////////////////////////////////////////////////
std::vector<glm::vec3> *CW3Spline3DItem::getControlPoint() {
	return &m_vControlPoint;
}

///////////////////////////////////////////////////////////////////////////
//	control point set을 return
///////////////////////////////////////////////////////////////////////////
std::vector<glm::vec3> CW3Spline3DItem::getControlPointData() {
	return m_vControlPoint;
}

///////////////////////////////////////////////////////////////////////////
//	각 spline point의 normal vecter set의 포인터를 return
///////////////////////////////////////////////////////////////////////////
std::vector<glm::vec3> *CW3Spline3DItem::getNormalArchData() {
	return &m_vNormalArchData;
}

///////////////////////////////////////////////////////////////////////////
//	spline을 등간격으로 설정
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::equidistanceSpline(std::vector<glm::vec3> &out, std::vector<glm::vec3> &normal,
										 std::vector<glm::vec3> &in, glm::vec3 &TopTranslate, glm::vec3 &upVector) {
	out.push_back(in.front() + TopTranslate);

	glm::vec3 addTail = in.back() * 2.0f - in.at(in.size() - 2);
	in.push_back(addTail);

	for (int i = 1; i < (int)in.size() - 1; i++) {
		glm::vec3 p1 = out.back();
		glm::vec3 p2 = in.at(i) + TopTranslate;
		glm::vec3 vec = (p2 - p1)*m_volRange*0.5f;

		int len = static_cast<int>(glm::length(vec));

		vec = glm::normalize(vec);

		vec = vec / m_volRange * 2.0f;

		glm::vec3 norm = glm::normalize(-glm::cross(upVector, vec));

		for (int j = 0; j < len; j++) {
			out.push_back(p1 + vec * float(j + 1));
			if (normal.size() == 0) {
				normal.push_back(norm);
			}

			normal.push_back(norm);
		}
	}

	in.pop_back();
}

///////////////////////////////////////////////////////////////////////////
//	spline을 GL_LINES로 rendering
//	useModifyMode가 true일 때는 modify plane을 사용하는 mode로
//	control point의 수정이 주 기능이기 때문에 spline을 어둡게 함
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::drawPath(GLuint program, const mat4& mvp, float lineWidth, const glm::vec4& color, bool useModifyMode) {
	if (!isPathValid())
		return;

	int sizePath = m_vPath.size();
	float *pVertLinesCoord = nullptr;
	int sizeVertLines = ((sizePath - 1) * 2) * 3;
	W3::p_allocate_1D(&pVertLinesCoord, sizeVertLines);
	memset(pVertLinesCoord, 0.0f, sizeVertLines * sizeof(float));

	glUseProgram(program);
	WGLSLprogram::setUniform(program, "MVP", mvp);

	vec4 finalColor = color;
	if (useModifyMode && m_bIsModifyMode) {
		finalColor *= 0.5;
		finalColor.a = 1.0f;
	}
	WGLSLprogram::setUniform(program, "Color", finalColor);

	if (useModifyMode && !m_bIsModifyMode) {
		WGLSLprogram::setUniform(program, "PlaneDepth", 1.0f);
	} else {
		WGLSLprogram::setUniform(program, "PlaneDepth", m_fPlaneDepth);
	}

	//vec3 mp1, mp2;
	for (int i = 0; i < sizePath - 1; i++) {
		//mp1 = vec3(m_vPath.at(i).x() / m_volRange.x(),
		//	m_vPath.at(i).y() / m_volRange.y(),
		//	m_vPath.at(i).z() / m_volRange.z());

		//mp2 = vec3(m_vPath.at(i + 1).x() / m_volRange.x(),
		//	m_vPath.at(i + 1).y() / m_volRange.y(),
		//	m_vPath.at(i + 1).z() / m_volRange.z());

		//mp1 = mp1 * 2.0f - 1.0f;
		//mp2 = mp2 * 2.0f - 1.0f;

		pVertLinesCoord[i * 6] = m_vPath.at(i).x;		pVertLinesCoord[i * 6 + 1] = m_vPath.at(i).y;	pVertLinesCoord[i * 6 + 2] = m_vPath.at(i).z;
		pVertLinesCoord[i * 6 + 3] = m_vPath.at(i + 1).x;	pVertLinesCoord[i * 6 + 4] = m_vPath.at(i + 1).y;	pVertLinesCoord[i * 6 + 5] = m_vPath.at(i + 1).z;
	}

	//printf("(x, y, z) = (%f, %f, %f)\n", pVertLinesCoord[3], pVertLinesCoord[4], pVertLinesCoord[5]);

	if (m_vboLines[0]) {
		glDeleteBuffers(1, m_vboLines);
		m_vboLines[0] = 0;
	}

	glGenBuffers(1, m_vboLines);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboLines[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVertLines * sizeof(float), pVertLinesCoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_vaoLines) {
		glDeleteVertexArrays(1, &m_vaoLines);
		m_vaoLines = 0;
	}

	glGenVertexArrays(1, &m_vaoLines);
	glBindVertexArray(m_vaoLines);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboLines[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(m_vaoLines);

	float finalWidth = lineWidth;
	if (!useModifyMode && m_nPickedPointIndex == m_vControlPoint.size())
		finalWidth = lineWidth + 2.0f;

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(finalWidth);
	glDrawArrays(GL_LINES, 0, (sizePath - 1) * 2);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_LINE_SMOOTH);

	SAFE_DELETE_ARRAY(pVertLinesCoord);

	glUseProgram(0);
}

///////////////////////////////////////////////////////////////////////////
//	useModifyMode가 true일 때는 modify plane을 사용하는 mode로
//	한번에 하나의 control point를 수정할 수 있기 때문에
//	현재 수정하려는 control point를 강조함
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::drawControlPoints(GLuint program, const mat4& mvp, float pointSize, const glm::vec4& color, bool useModifyMode) {
	if (!isPathValid())
		return;

	int sizePoints = m_vControlPoint.size();
	//float pVertPointsCoord[3];
	int sizeVertPoints = 3;

	glUseProgram(program);
	WGLSLprogram::setUniform(program, "MVP", mvp);

	if (useModifyMode && !m_bIsModifyMode) {
		WGLSLprogram::setUniform(program, "PlaneDepth", 1.0f);
	} else {
		WGLSLprogram::setUniform(program, "PlaneDepth", m_fPlaneDepth);
	}

	//vec3 mp;
	for (int i = 0; i < sizePoints; i++) {
		vec4 finalColor = color;
		float finalSize = pointSize;

		if (useModifyMode && m_bIsModifyMode) {
			finalColor *= 0.5f;
			finalColor.a = 1.0f;

			if (i == m_nPickedPointIndex) {
				finalColor = vec4(vec3(1.0f) - vec3(color), 1.0f);
				finalSize = pointSize + 2.0f;
			} else if (i == m_nModifyPointIndex) {
				finalColor = color * 2.0f;
				finalSize = pointSize;
			} else {
				finalSize = pointSize - 2.0f;
			}
		} else if (!useModifyMode) {
			if (i == m_nPickedPointIndex) {
				finalColor = vec4(vec3(1.0f) - vec3(color), 1.0f);
				finalSize = pointSize + 2.0f;
			}
		}

		WGLSLprogram::setUniform(program, "Color", finalColor);

		//mp = vec3(m_vControlPoint.at(i).x() / m_volRange.x(),
		//	m_vControlPoint.at(i).y() / m_volRange.y(),
		//	m_vControlPoint.at(i).z() / m_volRange.z());

		//mp = mp * 2.0f - 1.0f;

		//pVertPointsCoord[0] = mp.x;	pVertPointsCoord[1] = mp.y;	pVertPointsCoord[2] = mp.z;

		if (m_vboPoints[0]) {
			glDeleteBuffers(1, m_vboPoints);
			m_vboPoints[0] = 0;
		}

		glGenBuffers(1, m_vboPoints);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboPoints[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeVertPoints * sizeof(float), (float*)&(m_vControlPoint.at(i)), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (m_vaoPoints) {
			glDeleteVertexArrays(1, &m_vaoPoints);
			m_vaoPoints = 0;
		}

		glGenVertexArrays(1, &m_vaoPoints);
		glBindVertexArray(m_vaoPoints);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboPoints[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(m_vaoPoints);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(finalSize);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisable(GL_POINT_SMOOTH);
	}

	glUseProgram(0);
}

///////////////////////////////////////////////////////////////////////////
//	카메라 위치와 방향을 표시하는 object를 rendering
//	useModifyMode가 true일 때는 modify plane을 사용하는 mode로
//	control point의 수정이 주 기능이기 때문에 카메라 위치 표시 object를 어둡게 함
///////////////////////////////////////////////////////////////////////////
void CW3Spline3DItem::drawCameraPos(GLuint program, const mat4& mvp, const glm::vec4& posColor, const glm::vec4& dirColor, bool useModifyMode) {
	if (!isPathValid())
		return;

	if (!isEnd())
		return;

	int sizeLines = 1;
	float *pVertLinesCoord = nullptr;
	int sizeVertLines = sizeLines * 2 * 3;
	W3::p_allocate_1D(&pVertLinesCoord, sizeVertLines);
	memset(pVertLinesCoord, 0.0f, sizeVertLines * sizeof(float));

	glUseProgram(program);
	WGLSLprogram::setUniform(program, "MVP", mvp);

	vec4 finalColor = dirColor;
	if (useModifyMode && m_bIsModifyMode) {
		finalColor *= 0.5;
		finalColor.a = 1.0f;
	}
	WGLSLprogram::setUniform(program, "Color", finalColor);

	if (useModifyMode && !m_bIsModifyMode) {
		WGLSLprogram::setUniform(program, "PlaneDepth", 1.0f);
	} else {
		WGLSLprogram::setUniform(program, "PlaneDepth", m_fPlaneDepth);
	}

	glm::vec3 mEye = m_eye / m_volRange;
	vec3 mCenter = mEye - (m_dir * 0.1f);

	pVertLinesCoord[0] = mEye.x;	pVertLinesCoord[1] = mEye.y;	pVertLinesCoord[2] = mEye.z;
	pVertLinesCoord[3] = mCenter.x;	pVertLinesCoord[4] = mCenter.y;	pVertLinesCoord[5] = mCenter.z;

	if (m_vboCameraDir[0]) {
		glDeleteBuffers(1, m_vboCameraDir);
		m_vboCameraDir[0] = 0;
	}

	glGenBuffers(1, m_vboCameraDir);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboCameraDir[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVertLines * sizeof(float), pVertLinesCoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_vaoCameraDir) {
		glDeleteVertexArrays(1, &m_vaoCameraDir);
		m_vaoCameraDir = 0;
	}

	glGenVertexArrays(1, &m_vaoCameraDir);
	glBindVertexArray(m_vaoCameraDir);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboCameraDir[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(m_vaoCameraDir);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(kLineWidth * 2.0f);
	glDrawArrays(GL_LINES, 0, sizeLines * 2);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_LINE_SMOOTH);

	int sizeVertPoint = 3;

	if (m_vboCameraPosPoint[0]) {
		glDeleteBuffers(1, m_vboCameraPosPoint);
		m_vboCameraPosPoint[0] = 0;
	}

	glGenBuffers(1, m_vboCameraPosPoint);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboCameraPosPoint[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeVertPoint * sizeof(float), pVertLinesCoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_vaoCameraPosPoint) {
		glDeleteVertexArrays(1, &m_vaoCameraPosPoint);
		m_vaoCameraPosPoint = 0;
	}

	glGenVertexArrays(1, &m_vaoCameraPosPoint);
	glBindVertexArray(m_vaoCameraPosPoint);

	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboCameraPosPoint[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	finalColor = posColor;
	if (useModifyMode && m_bIsModifyMode) {
		finalColor *= 0.5;
		finalColor.a = 1.0f;
	}
	WGLSLprogram::setUniform(program, "Color", finalColor);

	glBindVertexArray(m_vaoCameraPosPoint);

	glEnable(GL_POINT_SMOOTH);
	glPointSize(kPointSize * 2.0f);
	glDrawArrays(GL_POINTS, 0, 1);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_POINT_SMOOTH);

	glUseProgram(0);
}

///////////////////////////////////////////////////////////////////////////
//	pick된 object index를 return
///////////////////////////////////////////////////////////////////////////
int CW3Spline3DItem::readPickInfo(int x, int y) {
	unsigned char res[4] = { 0, 0, 0, 0 };
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, viewport[3] - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res);

	return res[0] - 1;
}

///////////////////////////////////////////////////////////////////////////
//	path(control point, spline point)가 정상적으로 들어가 있는지 확인
///////////////////////////////////////////////////////////////////////////
bool CW3Spline3DItem::isPathValid() {
	if (m_vPath.size() < 2 && m_vControlPoint.size() < 2)
		return false;
	else
		return true;
}
