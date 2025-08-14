#pragma once
/*=========================================================================

File:			class CW3View3DEndo
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-21

=========================================================================*/
#include "uicomponent_global.h"

#include "../../Common/Common/W3Enum.h"
#include "../UIGLObjects/W3Spline3DItem.h"

#include "W3View3D.h"

///////////////////////////////////////////////////////////////////////////
//
//	* CW3View3DEndo
//	우상단 view
//	VR 내부로 들어가는 virtual endoscope view
//	CW3Spline3DItem를 가지고 전/후진 경로를 결정함
//	경로를 따라 이동 가능하고 카메라를 돌려 주변을 둘러볼 수 있음
//	
///////////////////////////////////////////////////////////////////////////

class UICOMPONENT_EXPORT CW3View3DEndo : public CW3View3D {
	Q_OBJECT

public:
	CW3View3DEndo(
		CW3VREngine *VREngine, CW3MPREngine *MPRengine,
		CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
		QWidget *pParent = 0);
	~CW3View3DEndo(void);
	
	virtual void reset();
	virtual void setVisible(bool state);

	void reoriUpdate(glm::mat4* m);
	void UpdateVR(bool is_high_quality);

public slots:
	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdateEndoPath(CW3Spline3DItem *, const bool)
	//	* CW3Spline3DItem *path : 3D Spline 객체 포인터
	//	* const bool reset : view의 상태 reset 여부
	void slotUpdateEndoPath(CW3Spline3DItem *path, const bool reset);

	//	* connected SIGNAL : CW3View3DEndoSlice::sigWheelEvent(QWheelEvent *)
	//	* QWheelEvent *event : wheel event
	void slotWheelEvent(QWheelEvent *event);

	//void slotChangedDeltaSliderFromSlice(int delta);

	void slotExploreFreeOnOff(bool bToggled);
	void slotExploreAction(const EndoPlayerID& id);
	void slotExploreChangeParam(const EndoPlayerParamID& param_id, int value);
	void slotSetCamPos(const EndoCameraDir& dir);

	//	* connected SIGNAL : QTimer(m_timerAutoPlay)::timeout()
	void slotAutoPlay();

	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdate()
	void slotUpdate();

	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdatePoint(int)
	//	* connected SIGNAL : CW3View3DEndoModify::sigUpdatePoint(int)
	//	* int index : 선택된 control point index
	void slotUpdatePoint(int index);

signals:
	//	* connected SLOT : CW3View3DEndoSagital::slotSetCameraPos(const int, const bool)
	//	* connected SLOT : CW3View3DEndoModify::slotSetCameraPos(const int, const bool)
	//	* const int : spline point list에서 카메라의 위치 index
	//	* const bool : 해당 point가 spline point인지 control point인지 여부
	void sigSetCameraPos(const int, const bool);

	//	* connected SLOT : CW3View3DEndoSlice::slotSliceUpdate(const mat4 view)
	//	* const mat4 view : view matrix
	void sigSliceUpdate(const mat4&, const mat4&, int, int);

	//	* connected SLOT : CW3View3DEndoSagital::slotSetNearClipPlane(vec3, vec3, vec3, vec3)
	//	* vec3 p0, p1, p2, p3 : CW3View3DEndoSlice plane의 네 꼭짓점 좌표
	void sigSetNearClipPlane(vec3, vec3, vec3, vec3);

	//	* connected SLOT : CW3View3DEndoModify::slotSetCameraDir(vec3, vec3, vec3, vec3)
	//	* vec3 : 카메라 위치 좌표
	//	* vec3, vec3, vec3, vec3 : 카메라 방향을 표시할 plane의 네 꼭짓점 좌표
	void sigSetCameraDir(vec3, vec3, vec3, vec3, vec3);

	//	* connected SLOT : CW3View3DEndoSagital::slotUpdate()
	//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
	void sigUpdate();

	void sigSliceReset();
	void sigSetFreeExplorerBtnState(bool);

protected:
	virtual bool mouseMoveEventW3(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);
	virtual void render3D();
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void setProjection();
	void SetNavigatorDirection();
	virtual void setViewMatrix();
	virtual void setModel();
	virtual void setModel(float rotAngle, glm::vec3 rotAxis);
	virtual void setDirectionFromCompass(glm::mat4 &T);
	virtual void clearGL();
	virtual void keyPressEvent(QKeyEvent* event) override;
	void changeViewMatrix();
	
#ifdef WILL3D_EUROPE
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
#endif // WILL3D_EUROPE

private:
	void setModelNearClipPlane();
	void setCameraPos(int index, bool isControlPoint);

	void resetView();
	void connections();
	int getNearestPointFromControlPointInPath(const int controlPointIndex);

	inline const bool IsEndoPathExist() const { return m_pgvEndoPath && m_pgvEndoControlPoint; }

	void PlayExplore();
	void StopExplore();

	void GoStartExplore();
	void GoEndExplore();
	void GoPrevExplore();
	void GoNextExplore();

protected:
	unsigned int m_vaoNearClipPlane = 0;	// 이 class를 상속받은 CW3View3DEndoSlice에서도 사용

private:
	vec3 m_eye;	// 카메라 위치
	vec3 m_center;	// 카메라 초점
	vec3 m_up;	// 카메라 up vector

	std::vector<glm::vec3> *m_pgvEndoPath = nullptr;	// CW3Spline3DItem의 spline point set
	std::vector<glm::vec3> *m_pgvEndoControlPoint = nullptr;	// CW3Spline3DItem의 control point set
	std::vector<glm::vec3> *m_pgvNormal = nullptr;	// CW3Spline3DItem의 각 spline point들의 normal vector

	float m_fCameraMoveStep = 0.0f;	// 자유탐색 mode일 때, 카메라가 한번에 이동하는 정도
	int m_nCameraPosInPath = 0;	// endo path에서 현재 카메라 위치
	int m_nCameraInterval = 3;	// 경로탐색 mode일 때, 한번에 이동하는 spline point 개수. interval slider로 설정

	bool m_bIsFreeExploreMode = false;	// 자유탐색 mode 여부
	mat4 m_lastView;	// 경로탐색 mode에서 자유탐색 mode로 전환될 때, 후에 다시 복귀하기 위한 view matrix를 저장
	QTimer *m_timerAutoPlay;	// 자동탐색을 위한 timer

	vec3 m_vertNearClipPlane[4];	// 현재 카메라 위치에서의 plane을 그리기 위한 vertex. volume 내부로 카메라가 들어갔을 때의 front face 조작과 CW3View3DEndoSlice를 그리기 위해 사용
	vec3 m_vertCameraDirPlane[5];	// CW3View3DEndoModify 에서 카메라 방향을 그리기 위해 사용

	EndoCameraDir m_eCameraDir = EndoCameraDir::FORWARD; // 경로탐색 mode에서 전/후진 시 카메라 방향 설정(전방, 후방, 사용자 지정)

	mat4 m_fFixedRotMatrix;	// 경로탐색 mode에서 전/후진 시 카메라 고정을 위한 matrix
	bool m_bIsFixedCamera = false;	//

	CW3Spline3DItem *m_pgSpline3D = nullptr;	// 3D Spline item 을 rendering 하기 위한 객체

	float m_near;
	float m_far;

	glm::mat4 m_modelNealClipPlane;
	glm::mat4 m_viewRotate;
	glm::mat4 m_inverseM;
};
