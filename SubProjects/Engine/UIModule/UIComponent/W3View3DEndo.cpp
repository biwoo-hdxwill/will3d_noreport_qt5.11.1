#include "W3View3DEndo.h"
/*=========================================================================

File:			class CW3View3DEndo
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-21

=========================================================================*/
#include <QTimer>
#include <QApplication>
#include <qevent.h>
#include <qopenglwidget.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"

#include "../UIGLObjects/W3GLObject.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3ActiveBlock.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

namespace {
const glm::vec3 kInitCameraPos = glm::vec3(0.0f, -1.0f, 0.0f);
const glm::vec3 kInitCameraCenter = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 kInitCameraUpVector = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 kInitCameraSideVector = glm::vec3(1.0f, 0.0f, 0.0f);
}

CW3View3DEndo::CW3View3DEndo(
	CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
	QWidget *pParent)
	: CW3View3D(VREngine, MPRengine, Rcontainer, eType, pParent) {
	m_pRender3DParam->m_isNearClipping = true;

	// QTimer의 interval에 설정한 시간이 지나면 timeout signal 발생
	m_timerAutoPlay = new QTimer(this);
	m_timerAutoPlay->setInterval(500);

	connections();
}

CW3View3DEndo::~CW3View3DEndo(void) {
	SAFE_DELETE_OBJECT(m_timerAutoPlay);

	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();
}

void CW3View3DEndo::reset() {
	CW3View3D::reset();

	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	m_pRender3DParam->m_isNearClipping = true;

	m_pgvEndoControlPoint = nullptr;
	m_pgvEndoPath = nullptr;
	m_pgvNormal = nullptr;
	m_pgSpline3D = nullptr;

	m_nCameraPosInPath = 0;

	m_bIsFreeExploreMode = false;

	m_eye = glm::vec3(0.0f, -m_camFOV, 0.0f);
	m_center = glm::vec3(0.0f, 0.0f, 0.0f);
	m_up = glm::vec3(0.0f, 0.0f, -1.0f);

	m_fFixedRotMatrix = mat4(1.0f);

	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();
}

void CW3View3DEndo::connections() {
	connect(m_timerAutoPlay, SIGNAL(timeout()), this, SLOT(slotAutoPlay()));
}

void CW3View3DEndo::setVisible(bool state) {
	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();

	CW3View3D::setVisible(state);

	if (state)
		this->SetNavigatorDirection();
}

void CW3View3DEndo::reoriUpdate(glm::mat4 *m) {
	m_inverseM = *m;
	m_inverseM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_inverseM = glm::inverse(m_inverseM);

	//m_pWorldAxisItem->SetWorldAxisDirection(m_inverseM, m_viewRotate*m_view);
}

void CW3View3DEndo::UpdateVR(bool is_high_quality) {
  m_pRender3DParam->m_isLowRes = !is_high_quality;
  if (isVisible()) {
	render3D();
	scene()->update();
  }
}

///////////////////////////////////////////////////////////////////////////
//
//	* resetView()
//	endo path를 삭제하거나 변경한 path slot이 비어있을 때, view 상태 초기화
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::resetView() {
	m_pgvEndoControlPoint = nullptr;
	m_pgvEndoPath = nullptr;
	m_pgvNormal = nullptr;

	m_nCameraPosInPath = 0;

	m_bIsFreeExploreMode = false;

	m_eye = glm::vec3(0.0f, -m_camFOV, 0.0f);
	m_center = glm::vec3(0.0f, 0.0f, 0.0f);
	m_up = glm::vec3(0.0f, 0.0f, -1.0f);

	m_fFixedRotMatrix = mat4(1.0f);

	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();

	setProjection();
	setViewMatrix();
}

void CW3View3DEndo::render3D() {
	if (!isReadyRender3D())
		return;

	m_pGLWidget->makeCurrent();

	setProjection();
	setMVP();
	setModelNearClipPlane();

	///////////////////////////////////////////////////////////
	//ready for airway
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_pAirway->isVisible()) {
		setAirwayVAO();
		m_pRender3DParam->m_pAirway->setInvModel(m_inverseScale);
		m_pRender3DParam->m_pAirway->setMVP(m_model, m_view, m_projection);
		//m_pRender3DParam->m_pAirway->setVAO(m_vaoAirway);
		m_pRender3DParam->m_pAirway->setNindices(m_nCntAirwayVertex);
	}

	///////////////////////////////////////////////////////////
	//ready for raycasting
	///////////////////////////////////////////////////////////

	if (!m_pRender3DParam->m_plane->getVAO()) {
		unsigned int vao = 0;
		m_pRender3DParam->m_plane->clearVAOVBO();

		m_pgVREngine->initVAOplane(&vao);
		m_pRender3DParam->m_plane->setVAO(vao);
		m_pRender3DParam->m_plane->setNindices(6);
	}

	if (m_pRender3DParam->m_mainVolume_vao[m_drawVolId]) {
	  glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[m_drawVolId]);
	  m_pRender3DParam->m_mainVolume_vao[m_drawVolId] = 0;
	}
	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);

	m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_mvp = &m_mvp;
	m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_invModel = &m_modelNealClipPlane;
	m_pRender3DParam->m_width = this->width();
	m_pRender3DParam->m_height = this->height();

	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);
	WGLSLprogram::setUniform(PROGRaycasting, "useSegTmj", false);

	m_pgVREngine->Render3DEndo(m_pRender3DParam, m_drawVolId, m_isReconSwitched);

	m_pGLWidget->doneCurrent();

	if (m_pgvEndoPath)
		emit sigSliceUpdate(m_view, m_modelNealClipPlane, m_nCameraPosInPath, m_pgvEndoPath->size() - 1);
	else
		emit sigSliceReset();

	if (!m_bIsFreeExploreMode)
		m_lastView = m_view;
}

void CW3View3DEndo::drawBackground(QPainter *painter, const QRectF &rect) {
	CW3View3D::drawBackground(painter, rect);
}

///////////////////////////////////////////////////////////////////////////
//
//	* setProjection()
//	ortho를 사용하는 다른 view와 별도로 perspective projection을 설정
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::setProjection() {
	float width = this->width();
	float height = this->height();

	if (width > height) {
		float ratio = width / height * m_camFOV;

		proj_left_ = -ratio / m_scale;
		proj_right_ = -proj_left_;
		proj_bottom_ = -m_camFOV / m_scale;
		proj_top_ = -proj_bottom_;
		m_near = 0.01f;
		m_far = m_camFOV * 2.0f;

		m_scaleSceneToGL = m_camFOV / (m_sceneHinView*m_scale);
	} else {
		float ratio = height / width * m_camFOV;

		proj_left_ = -m_camFOV / m_scale;
		proj_right_ = -proj_left_;
		proj_bottom_ = -ratio / m_scale;
		proj_top_ = -proj_bottom_;
		m_near = 0.01f;
		m_far = m_camFOV * 2.0f;

		m_scaleSceneToGL = m_camFOV / (m_sceneWinView*m_scale);
	}

	m_projection = glm::perspective(glm::radians(90.0f), width / height, m_near, m_far);
}

///////////////////////////////////////////////////////////////////////////
//
//	* setViewMatrix()
//	카메라 이동을 위한 view matrix 설정
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::SetNavigatorDirection() {
	glm::mat4 init_view = glm::lookAt(kInitCameraPos, kInitCameraCenter, kInitCameraUpVector);
	glm::vec4 translation = init_view[3];
	init_view = m_view;
	init_view[3] = translation;

	m_pWorldAxisItem->SetWorldAxisDirection(glm::mat4(), init_view);
}
void CW3View3DEndo::setViewMatrix() {
	if (!m_is3Dready) {
		m_eye = kInitCameraPos * m_camFOV;
		m_center = kInitCameraCenter;
		m_up = kInitCameraUpVector;
	}

	m_view = glm::lookAt(m_eye, m_center, m_up);

	if (m_is3Dready)
		this->SetNavigatorDirection();
}

void CW3View3DEndo::changeViewMatrix() {
	// 카메라를 조작하면 view matrix를 변경
	if (m_fCameraMoveStep != 0.0f || m_rotAngle != 0.0f) {
		// 자유탐색 mode가 아니면 고정카메라 시점을 위한 rotate matrix 저장
		// 자유탐색 mode일 때의 카메라 회전은 고정카메라 시점에 적용하지 않음
		if (!m_bIsFreeExploreMode) {
			m_fFixedRotMatrix = m_viewRotate * m_fFixedRotMatrix;
		} else {
			// 자유탐색 mode면 카메라 이동을 적용
			m_view = glm::translate(vec3(0.0f, 0.0f, 1.0f) * m_fCameraMoveStep) * m_view;

			if (m_pgSpline3D) {
				vec4 eye = glm::inverse(m_view * m_model) * vec4(0.0f, 0.0f, -(m_near + 0.1f), 1.0f);
				m_pgSpline3D->setCameraPos(vec3(eye) * m_vVolRange, vec3(m_view[0][2], m_view[1][2], m_view[2][2]));
			}
		}
		m_view = m_viewRotate * m_view;
		m_fCameraMoveStep = 0.0f;
		m_rotAngle = 0.0f;

		this->SetNavigatorDirection();
	}
}

#ifdef WILL3D_EUROPE
void CW3View3DEndo::mousePressEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mousePressEvent(event);
}

void CW3View3DEndo::mouseReleaseEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		if (event->button() == Qt::RightButton)
		{
			emit sigShowButtonListDialog(event->globalPos());
			return;
		}
	}

	CW3View3D::mouseReleaseEvent(event);
}

void CW3View3DEndo::mouseMoveEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mouseMoveEvent(event);
}


void CW3View3DEndo::keyReleaseEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}

	CW3View3D::keyReleaseEvent(event);
}
#endif // WILL3D_EUROPE

void CW3View3DEndo::setModel() {
	m_model = m_origModel * m_scaleMat;
}

void CW3View3DEndo::setModel(float rotAngle, glm::vec3 rotAxis) {
	m_viewRotate = glm::rotate(glm::radians(rotAngle), rotAxis);
}

void CW3View3DEndo::setDirectionFromCompass(glm::mat4 &T) {
	m_view = T;

	if (m_pgSpline3D && !m_bIsFreeExploreMode) {
		m_pgSpline3D->setCameraPos(m_eye, vec3(m_view[0][2], m_view[1][2], m_view[2][2]));
	}
}

void CW3View3DEndo::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();

		if (m_vaoNearClipPlane) {
			glDeleteVertexArrays(1, &m_vaoNearClipPlane);
			m_vaoNearClipPlane = 0;
		}

		m_pGLWidget->doneCurrent();
	}

	CW3View3D::clearGL();
}

///////////////////////////////////////////////////////////////////////////
//
//	* wheelEvent(QWheelEvent *event)
//	wheel을 돌리면 경로탐색 mode일 땐 경로를 따라서,
//	자유탐색 mode일 땐 카메라 방향을 따라 전/후진
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::wheelEvent(QWheelEvent *event) {
	double degrees = event->delta() / 8.0;
	double dir = degrees / 15.0;

	if (m_bIsFreeExploreMode) {
		float freeStep = m_nCameraInterval * -dir;
		m_fCameraMoveStep = freeStep;

		m_viewRotate = glm::rotate(glm::radians(m_rotAngle), m_rotAxis);

		changeViewMatrix();
		render3D();
		scene()->update();

		emit sigUpdate();
	} else if (m_pgvEndoPath) {
		m_fCameraMoveStep = 0.0f;
		m_nCameraPosInPath += (m_nCameraInterval * -(int)dir);

		if (m_nCameraPosInPath < 0)
			m_nCameraPosInPath = 0;
		else if (m_nCameraPosInPath > (int)m_pgvEndoPath->size() - 1)
			m_nCameraPosInPath = m_pgvEndoPath->size() - 1;

		if (!m_bIsFixedCamera)
			m_fFixedRotMatrix = mat4(1.0f);

		// 카메라 위치, 방향 설정
		// 해당 spline point index로 카메라의 eye, center, up을 계산
		// wheel로 이동시엔 spline point를 탐색하므로 control point flag를 false로 전달
		setCameraPos(m_nCameraPosInPath, false);

		changeViewMatrix();
		render3D();
		scene()->update();
		// 다른 view의 카메라도 동일하게 설정
		//	* connected SLOT : CW3View3DEndoSagital::slotSetCameraPos(const int, const bool)
		//	* connected SLOT : CW3View3DEndoModify::slotSetCameraPos(const int, const bool)
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	}
}

bool CW3View3DEndo::mouseMoveEventW3(QMouseEvent *event) {
	// 자동탐색 중엔 마우스 조작 무시
	if (m_timerAutoPlay->isActive())
		return false;

	bool isUpdatedNeeded = CW3View3D::mouseMoveEventW3(event);
	if (isUpdatedNeeded) {
		if (event->buttons() & Qt::RightButton) {
			changeViewMatrix();
			// 자유탐색 mode일 때 spline상의 현재 카메라 위치, 방향 표시
			// 카메라 위치 m_eye와 view matrix의 forward vector 부분만 전달

			if (m_pgSpline3D && !m_bIsFreeExploreMode) {
				m_pgSpline3D->setCameraPos(m_eye, vec3(m_view[0][2], m_view[1][2], m_view[2][2]));
			}

			//emit sigSetCameraPos(m_nCameraPosInPath, false);
			emit sigUpdate();
		}
	}

	return isUpdatedNeeded;
}

///////////////////////////////////////////////////////////////////////////
//
//	* setNearClipPlane()
//	카메라 위치에서의 near plane의 vbo 갱신
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::setModelNearClipPlane() {
	m_modelNealClipPlane = glm::inverse(m_view * m_model)
		*glm::scale(glm::vec3(proj_right_, proj_top_, 1.0f))
		*glm::translate(glm::vec3(0.0f, 0.0f, -(m_near + 0.01f)));
}

///////////////////////////////////////////////////////////////////////////
//
//	* setCameraPos(const int index, const bool isControlPoint)
//	카메라 위치와 옵션에 따른 방향 설정
//	splint point들을 가지고 view의 eye, center, up을 설정
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndo::setCameraPos(int index, bool isControlPoint) {
	if (!m_pgvEndoPath || !m_pgvEndoControlPoint)
		return;

	if (m_pgvEndoPath->size() < 2 || m_pgvEndoControlPoint->size() < 2)
		return;

	int indexInPath = index;

	std::vector<glm::vec3> *listPtEye;
	std::vector<glm::vec3> *listPtCenter;

	// 현재 위치가 control point면 가장 가까운 spline point를 찾아서 카메라 방향을 설정
	if (isControlPoint) {
		listPtEye = m_pgvEndoControlPoint;
		listPtCenter = m_pgvEndoPath;

		m_nCameraPosInPath = getNearestPointFromControlPointInPath(index);
		indexInPath = m_nCameraPosInPath;
	} else {
		listPtEye = m_pgvEndoPath;
		listPtCenter = m_pgvEndoPath;
	}

	// 방향옵션이 forward면 현재 point를 eye, 다음 point를 center로 설정
	if (m_eCameraDir == EndoCameraDir::FORWARD) {
		m_up = m_pgvNormal->at(indexInPath);

		if (index < (int)listPtEye->size() - 1) {
			m_eye = listPtEye->at(index)* m_vVolRange;
			m_center = listPtCenter->at(indexInPath + 1)* m_vVolRange;
		} else {
			// 마지막 point는 바로 전 point에서의 카메라 방향을 사용
			vec3 preEye = listPtCenter->at(indexInPath - 1)* m_vVolRange;
			vec3 preCenter = listPtCenter->at(indexInPath)* m_vVolRange;
			vec3 preDir = preCenter - preEye;

			m_eye = listPtEye->at(index)* m_vVolRange;
			m_center = m_eye + preDir;
		}
	} else if (m_eCameraDir == EndoCameraDir::BACKWARD) {
		// 방향옵션이 BACKWARD면 현재 point를 eye, 앞 point를 center로 설정
		m_up = m_pgvNormal->at(indexInPath);

		if (index > 0) {
			m_eye = listPtEye->at(index)* m_vVolRange;
			m_center = listPtCenter->at(indexInPath - 1)* m_vVolRange;
		} else {
			// 처음 point는 두번째 point에서의 카메라 방향을 사용
			vec3 preEye = listPtCenter->at(indexInPath + 1)* m_vVolRange;
			vec3 preCenter = listPtCenter->at(indexInPath)* m_vVolRange;
			vec3 preDir = preCenter - preEye;

			m_eye = listPtEye->at(index) * m_vVolRange;
			m_center = m_eye + preDir;
		}
	}

	setViewMatrix();

	// 고정카메라 방향 옵션일 때는 저장된 rotate matrix를 사용하여 방향 설정
	if (m_bIsFixedCamera) {
		m_view = m_fFixedRotMatrix * m_view;
		this->SetNavigatorDirection();
	}

	if (m_pgSpline3D) {
		// 자유탐색 mode가 아닐 때, spline위에 카메라 위치 표시를 위한 설정
		if (!m_bIsFreeExploreMode)
			m_pgSpline3D->setCameraPos(m_eye, vec3(m_view[0][2], m_view[1][2], m_view[2][2]));
	}
}

// CW3View3DSagital에서 spline그리기가 완료되면 CW3Spline3DItem을 전달받음
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdateEndoPath(CW3Spline3DItem *, const bool)
void CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *path, const bool reset) {
	if (!m_is3Dready) {
		m_pGLWidget->makeCurrent();
		init();
		m_pGLWidget->doneCurrent();
	}

	if (!path) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3View3DEndo::slotUpdateEndoPath : Endo path is null");
	}

	if (reset)
		resetView();

	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();

	m_pgSpline3D = path;
	if (m_pgSpline3D) {
		m_pgvEndoControlPoint = m_pgSpline3D->getControlPoint();
		m_pgvEndoPath = m_pgSpline3D->getPath();
		m_pgvNormal = m_pgSpline3D->getNormalArchData();

		setCameraPos(m_nCameraPosInPath, false);

		// CW3View3DEndo의 카메라 설정이 완료되면 각 view의 카메라 설정을 위해 signal 발생
		//	* connected SLOT : CW3View3DEndoSagital::slotSetCameraPos(const int, const bool)
		//	* connected SLOT : CW3View3DEndoModify::slotSetCameraPos(const int, const bool)
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	}

	render3D();

	if (isVisible())
		scene()->update();
}

// CW3View3DEndoSlice에서 발생한 wheel event를 받아서 처리
//	* connected SIGNAL : CW3View3DEndoSlice::sigWheelEvent(QWheelEvent *)
void CW3View3DEndo::slotWheelEvent(QWheelEvent *event) {
	wheelEvent(event);
}

// 버튼을 누르면 자유탐색 mode on/off
//	* connected SIGNAL : QPushButton(m_pgBtnFREEonoffENDO)::toggled(bool)
void CW3View3DEndo::slotExploreFreeOnOff(bool bToggled) {
	//m_bIsFreeExploreMode = !m_bIsFreeExploreMode;
	m_bIsFreeExploreMode = bToggled;

	if (!m_bIsFreeExploreMode) {
		// 자유탐색 off 시 전 상태로 복귀하기 위한 현재 view matrix 저장
		m_view = m_lastView;
		this->SetNavigatorDirection();
		render3D();
		scene()->update();
		//emit sigUpdate();
		setCameraPos(m_nCameraPosInPath, false);
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	}
}

void CW3View3DEndo::slotExploreAction(const EndoPlayerID& id) {
	switch (id) {
	case EndoPlayerID::START_POS:
		GoStartExplore();
		break;
	case EndoPlayerID::STOP:
		StopExplore();
		break;
	case EndoPlayerID::PLAY:
		PlayExplore();
		break;
	case EndoPlayerID::END_POS:
		GoEndExplore();
		break;
	case EndoPlayerID::PREV:
		GoPrevExplore();
		break;
	case EndoPlayerID::NEXT:
		GoNextExplore();
		break;
	default:
		break;
	}
}

void CW3View3DEndo::slotExploreChangeParam(const EndoPlayerParamID & param_id,
										   int value) {
	switch (param_id) {
	case EndoPlayerParamID::PLAY_SPEED:
		m_timerAutoPlay->setInterval(value * 100);
		break;
	case EndoPlayerParamID::PLAY_INTERVAL:
		m_nCameraInterval = value;
		break;
	default:
		break;
	}
}

void CW3View3DEndo::slotSetCamPos(const EndoCameraDir & dir) {
	if (dir == EndoCameraDir::FORWARD || dir == EndoCameraDir::BACKWARD) {
		m_eCameraDir = dir;
		m_fFixedRotMatrix = mat4(1.0f);
		m_bIsFixedCamera = false;
	} else if (dir == EndoCameraDir::CAM_FIXED) {
		if (m_bIsFixedCamera)
			return;

		m_bIsFixedCamera = true;
	}

	setCameraPos(m_nCameraPosInPath, false);
	render3D();
	scene()->update();
	emit sigSetCameraPos(m_nCameraPosInPath, false);
}

//	* connected SIGNAL : QTimer(m_timerAutoPlay)::timeout()
void CW3View3DEndo::slotAutoPlay() {
	if (!isVisible()) {
		m_timerAutoPlay->stop();
		return;
	}

	if (m_nCameraPosInPath < (int)m_pgvEndoPath->size() - 1) {
		m_nCameraPosInPath += m_nCameraInterval;

		if (m_nCameraPosInPath < 0)
			m_nCameraPosInPath = 0;
		else if (m_nCameraPosInPath > (int)m_pgvEndoPath->size() - 1)
			m_nCameraPosInPath = m_pgvEndoPath->size() - 1;

		setCameraPos(m_nCameraPosInPath, false);
		render3D();
		scene()->update();
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	} else {
		m_timerAutoPlay->stop();
	}
}

void CW3View3DEndo::PlayExplore() {
	m_bIsFreeExploreMode = false;
	emit sigSetFreeExplorerBtnState(m_bIsFreeExploreMode);

	if (!IsEndoPathExist())
		return;

	if (!m_timerAutoPlay->isActive())
		m_timerAutoPlay->start();
}

void CW3View3DEndo::StopExplore() {
	if (!IsEndoPathExist())
		return;

	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();
}

void CW3View3DEndo::GoStartExplore() {
	m_bIsFreeExploreMode = false;
	emit sigSetFreeExplorerBtnState(m_bIsFreeExploreMode);

	if (!IsEndoPathExist() || m_timerAutoPlay->isActive())
		return;

	m_nCameraPosInPath = 0;
	setCameraPos(m_nCameraPosInPath, false);
	render3D();
	scene()->update();
	emit sigSetCameraPos(m_nCameraPosInPath, false);
}

void CW3View3DEndo::GoEndExplore() {
	m_bIsFreeExploreMode = false;
	emit sigSetFreeExplorerBtnState(m_bIsFreeExploreMode);

	if (!IsEndoPathExist() || m_timerAutoPlay->isActive())
		return;

	m_nCameraPosInPath = m_pgvEndoPath->size() - 1;
	setCameraPos(m_nCameraPosInPath, false);
	render3D();
	scene()->update();
	emit sigSetCameraPos(m_nCameraPosInPath, false);
}

void CW3View3DEndo::GoPrevExplore() {
	m_bIsFreeExploreMode = false;
	emit sigSetFreeExplorerBtnState(m_bIsFreeExploreMode);

	if (!IsEndoPathExist() || m_timerAutoPlay->isActive())
		return;

	if (m_nCameraPosInPath > 0) {
		m_nCameraPosInPath -= m_nCameraInterval;

		if (m_nCameraPosInPath < 0)
			m_nCameraPosInPath = 0;

		setCameraPos(m_nCameraPosInPath, false);
		render3D();
		scene()->update();
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	}
}

void CW3View3DEndo::GoNextExplore() {
	m_bIsFreeExploreMode = false;
	emit sigSetFreeExplorerBtnState(m_bIsFreeExploreMode);

	if (!IsEndoPathExist() || m_timerAutoPlay->isActive())
		return;

	if (m_nCameraPosInPath < (int)m_pgvEndoPath->size() - 1) {
		m_nCameraPosInPath += m_nCameraInterval;

		if (m_nCameraPosInPath > (int)m_pgvEndoPath->size() - 1)
			m_nCameraPosInPath = m_pgvEndoPath->size() - 1;

		setCameraPos(m_nCameraPosInPath, false);
		render3D();
		scene()->update();
		emit sigSetCameraPos(m_nCameraPosInPath, false);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	* getNearestPointFromControlPointInPath(const int controlPointIndex)
//	control point index를 받아 가장 가까운 spline point index를 return
//
///////////////////////////////////////////////////////////////////////////
int CW3View3DEndo::getNearestPointFromControlPointInPath(const int controlPointIndex) {
	const glm::vec3& ptControl = m_pgvEndoControlPoint->at(controlPointIndex);

	int index = 0;
	float preLen = 65535.0f;
	float curLen = 0.0f;
	for (int i = 0; i < m_pgvEndoPath->size(); i++) {
		const glm::vec3& ptPath = m_pgvEndoPath->at(i);
		curLen = glm::length(ptControl - ptPath);

		if (preLen > curLen) {
			index = i;
			preLen = curLen;
		} else {
			continue;
		}
	}

	return index;
}

// Scene update
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdate()
void CW3View3DEndo::slotUpdate() {
	setCameraPos(m_nCameraPosInPath, false);
	render3D();
	scene()->update();
	emit sigSetCameraPos(m_nCameraPosInPath, false);
}

// 각 view에서 control point를 선택하면 해당 위치에서의 카메라 설정을 하고 각 view에 카메라 설정 signal 전달
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdatePoint(int)
//	* connected SIGNAL : CW3View3DEndoModify::sigUpdatePoint(int)
void CW3View3DEndo::slotUpdatePoint(int index) {
	// 자동탐색 mode일 때 자동탐색 멈춤
	if (m_timerAutoPlay->isActive())
		m_timerAutoPlay->stop();

	setCameraPos(index, true);
	render3D();
	scene()->update();
	emit sigSetCameraPos(index, true);
}

void CW3View3DEndo::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D::keyPressEvent(event);
}
