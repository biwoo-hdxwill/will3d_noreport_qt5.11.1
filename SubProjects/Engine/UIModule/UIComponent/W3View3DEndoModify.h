#pragma once
/*=========================================================================

File:			class CW3View3DEndoModify
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-28
Last modify:	2016-04-21

=========================================================================*/
#include <memory>

#include "W3View3D.h"
#include "uicomponent_global.h"

///////////////////////////////////////////////////////////////////////////
//
//	* CW3View3DEndoModify
//	우하단 view
//	Endo path를 정밀하게 수정하기 위한 view
//	volume에 endo path가 rendering된 화면에서 CW3View3DEndoSagital에 있는
//	control point를 선택하면 해당 point 위치를 기준으로 modify plane이 그려짐
//	CW3Spline3DItem를 가지고 path와 control point, camera pos를 rendering
//	
///////////////////////////////////////////////////////////////////////////
class CW3TextItem_switch;
class CW3Spline3DItem;
class ViewBorderItem;

class UICOMPONENT_EXPORT CW3View3DEndoModify : public CW3View3D {
	Q_OBJECT
public:
	CW3View3DEndoModify(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
						CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
						QWidget *pParent = 0);
	~CW3View3DEndoModify(void);
	
	virtual void reset();

	void reoriUpdate(glm::mat4* m);
	void UpdateVR(bool is_high_quality);
	void setVisiblePath(int state);

public slots:
	//	* connected SIGNAL : CW3View3DEndo::sigSetCameraPos(const int, const bool)
	//	* const int index : spline point list에서 카메라의 위치 index
	//	* const bool isControlPoint : 해당 point가 spline point인지 control point인지 여부
	void slotSetCameraPos(const int index, const bool isControlPoint);

	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdateEndoPath(CW3Spline3DItem *, const bool)
	//	* CW3Spline3DItem *path : 3D Spline 객체 포인터
	//	* const bool reset : view의 상태 reset 여부
	void slotUpdateEndoPath(CW3Spline3DItem *path, const bool reset);

	//	* connected SIGNAL : CW3View3DEndo::sigUpdate()
	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdate()
	void slotUpdate();

	//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdatePoint(int)
	//	* int index : 선택된 control point index
	void slotUpdatePoint(int index);
	
	void slotModifyModeOnOff(bool isModifyMode);

signals:
	//	* connected SLOT : CW3View3DEndoSagital::slotSetMPROverlayPlane(vec3, vec3, vec3, vec3)
	//	* vec3 p0, p1, p2, p3 : CW3View3DEndoModify의 modify plane의 네 꼭짓점 좌표
	void sigSetMPROverlayPlane(vec3, vec3, vec3, vec3);

	//	* connected SLOT : CW3View3DEndo::slotUpdate()
	//	* connected SLOT : CW3View3DEndoSagital::slotUpdate()
	void sigUpdate();

	//	* connected SLOT : CW3View3DEndo::slotUpdatePoint(int)
	//	* connected SLOT : CW3View3DEndoSagital::slotUpdatePoint(int)
	//	* int : 선택된 control point index
	void sigUpdatePoint(int index);

	//	* connected SLOT : CW3View3DEndoSagital::slotUpdate()
	void sigUpdatePick();

	void sigAirwayDisabled(bool);	// by jdk 160906

protected:
	virtual bool mouseMoveEventW3(QMouseEvent *event);
	virtual void render3D();
	virtual void drawBackground(QPainter *painter, const QRectF &rect);
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void clearGL();
	virtual void keyPressEvent(QKeyEvent * event) override;

#ifdef WILL3D_EUROPE
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
#endif // WILL3D_EUROPE


private:
	virtual void HideUI(bool bToggled) override;
	glm::vec3 transformDistSliceToVolume(QPointF ptScene);
	void setModelSlice();
	void setCameraPos(const int index, const bool isControlPoint);
	void resetView();

	void connections();

private:
	std::unique_ptr<ViewBorderItem> border_;
	glm::mat4 m_modelSlice;

	unsigned int m_vaoCameraDir = 0;	// 카메라 방향을 표시하기 위한 사각뿔 형태의 vao

	std::vector<glm::vec3> *m_pgvEndoControlPoint = nullptr;;	// CW3Spline3DItem의 control point set

	int m_nModifyPointIndex = -1;	// modify plane을 만들기 위한 기준이 되는 control point index
	int m_nPickedPointIndex = -1;	// 선택된 control point index

	float m_fViewDepth = 0.0f;	// modify mode일 때 modify plane(선택한 control point)의 depth
	float m_fPlaneDepth = 1.0f;	// projection의 near를 -1, far를 +1이라고 했을 때, m_fViewDepth의 depth. shader에서 사용

	CW3Spline3DItem *m_pgSpline3D = nullptr;;	// 3D Spline item 을 rendering 하기 위한 객체

	bool m_bVisiblePath = true;
	CW3TextItem_switch *m_pModifyModeOnOff;
};
