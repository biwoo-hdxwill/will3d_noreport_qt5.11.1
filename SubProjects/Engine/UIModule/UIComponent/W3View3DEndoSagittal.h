#pragma once
/*=========================================================================

File:			class CW3View3DEndoSagital
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2015-12-03
Last date:		2016-04-21

=========================================================================*/
#include <memory>
#include <QGraphicsTextItem>

#include "../../Common/GLfunctions/W3GLTypes.h"

#include "W3View3Dslice.h"
#include "uicomponent_global.h"

class QMenu;
class CW3JobMgr;
class ViewBorderItem;
class CW3Spline3DItem;
#ifndef WILL3D_VIEWER
class ProjectIOEndo;
#endif

///////////////////////////////////////////////////////////////////////////
//
//	* CW3View3DEndoSagital
//	좌하단 view
//	Endo path를 그리기 위한 view 
//	volume의 sagital plane을 2D로 rendering하고 slice 단위로 탐색 가능
//	CW3View3DEndoModify의 modify plane과 CW3View3DEndoSlice의 위치를 표시
//	CW3Spline3DItem를 가지고 path와 control point, camera pos를 rendering
//	
///////////////////////////////////////////////////////////////////////////

class TextItem : public QGraphicsTextItem
{
public:
	TextItem(const QString& text) 
		: QGraphicsTextItem(text) { }

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* o, QWidget* w) 
	{
		painter->setBrush(QBrush(QColor("#ffffcc")));
		painter->drawRect(boundingRect());
		QGraphicsTextItem::paint(painter, o, w);
	}
};

class UICOMPONENT_EXPORT CW3View3DEndoSagittal : public CW3View3Dslice 
{
	Q_OBJECT
public:
	CW3View3DEndoSagittal(CW3VREngine *VREngine, CW3MPREngine *MPRengine, CW3JobMgr *JobMgr,
						  CW3ResourceContainer *Rcontainer, common::ViewTypeID eType, QWidget *pParent = 0);
	~CW3View3DEndoSagittal(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOEndo& out);
	void importProject(ProjectIOEndo& in);
#endif

	virtual void reset();

	void reoriUpdate(glm::mat4 *m);
	void setVisiblePath(int state);

public slots:
	//	* connected SIGNAL : CW3View3DEndo::sigSetCameraPos(const int, const bool)
	//	* const int index : spline point list에서 카메라의 위치 index
	//	* const bool isControlPoint : 해당 point가 spline point인지 control point인지 여부
	void slotSetCameraPos(const int index, const bool isControlPoint);

	void slotSelectPath(const EndoPathID& path_id);
	//	* connected SIGNAL : CW3ENDOViewMgr::sigDeletePath(int)
	//	* int pathNum : 삭제한 endo path slot 번호(0 ~ 4)
	void slotDeletePath(const EndoPathID& path_id);

	//	* connected SIGNAL : CW3View3DEndo::sigUpdate()
	//	* connected SIGNAL : CW3View3DEndoModify::sigUpdate()
	void slotUpdate();

	//	* connected SIGNAL : CW3View3DEndoModify::sigUpdatePoint(int)
	//	* int index : 선택된 control point index
	void slotUpdatePoint(int index);

	void slotDrawAirway(int state);	// by jdk 160906

signals:
	//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
	//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
	//	* CW3Spline3DItem * : 3D Spline 객체 포인터
	//	* const bool : 목적지 view의 상태 reset 여부
	void sigUpdateEndoPath(CW3Spline3DItem *, const bool);

	//	* connected SLOT : CW3View3DEndo::slotUpdate()
	//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
	void sigUpdate();

	//	* connected SLOT : CW3View3DEndo::slotUpdatePoint(int)
	//	* connected SLOT : CW3View3DEndoModify::slotUpdatePoint(int)
	//	* int : 선택된 control point index
	void sigUpdatePoint(int);

	//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
	void sigUpdatePick();

	//  * connected SLOT : CW3ENDOViewMgr::slotSetEnableDeleteButton(const int, const bool);
	//  * const int : m_nCurPathNum
	//  * const bool : enable
	void sigSetEnableEndoPath(const int, const bool);

	//void sigSegmentAirway(GLenum &, unsigned int &, int &);
	void sigSegAirway(std::vector<tri_STL>&);
	void sigSetAirwaySize(double);	// by jdk 160906
	void sigAirwayDisabled(bool);
	void sigSetPathNum(int);
	void sigSpline3DEnd();

protected:
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;

	virtual void init();
	virtual void changedDeltaSlider(int delta) override;
	virtual void clearGL();
	virtual void setInitScale() override;

	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent * event) override;

	virtual void ResetView() override;
	virtual void FitView() override;

private:
	glm::vec3 transformSliceToVolume(const QPointF &ptScene);
	glm::vec3 transformDistSliceToVolume(const QPointF &ptScene);

	bool isPathEmpty(const int index);

	int getNearestPointInPath(glm::vec3 point);

	void setCameraPos(const int index, const bool isControlPoint);
	void resetView();
	void drawPlaneRect(unsigned int vao, float lineWidth, const glm::vec4& color);
	void initializeMenus();
	void connections();

	void segAirway();	// by jdk 160811
	void runSegAirway();
	void insertAirwayToDB();
	
	void DisplayDICOMInfo(const QPointF& scene_pos);

private slots:
	void slotDeletePathAct();
	void slotDeletePathPointAct();
	void slotInsertPathPointAct();

private:
	std::unique_ptr<ViewBorderItem> border_;
	CW3JobMgr * m_pgJobMgr;
	glm::mat4 m_mvpItem;

	int m_nCameraPosInPath = 0;	// endo path에서 현재 카메라 위치
	int m_nPickedPointIndex = -1;	// 선택된 control point index
	int m_nCurPathNum = 0;	// 현재 path slot 번호(0 ~ 4);
	float m_fSlicePos = 0.0f; // volume상에서의 sagital slice 좌표(0 ~ volume.x.width)

	float m_fPlaneDepth = 0.0f;	// projection의 near를 -1, far를 +1이라고 했을 때, m_fSlicePos의 depth. shader에서 사용

	CW3Spline3DItem *m_pSpline3D;	// 3D Spline item 을 rendering 하기 위한 객체
	std::vector<glm::vec3> m_listEndoControlPointData[5];	// 5개의 path를 저장하고 불러오기 위해 control point set을 저장하는 컨테이너

	QMenu* m_pSplineMenu = nullptr;
	QMenu* m_pEllipseMenu = nullptr;
	QAction* m_pDeletePathAct = nullptr;
	QAction* m_pDeletePathPointAct = nullptr;
	QAction* m_pInsertPathPointAct = nullptr;

	bool m_bIsHoveredPathSpline = false;	// point 가 아닌 Path 가 hover 되었는지
	bool m_bIsHoveredPathPoint = false;	// Path가 아닌 point 가 hover 되었는지

	double m_dAirwaySize = 0;
	std::vector<tri_STL> m_meshSTL;

	bool m_bVisiblePath = true;

	TextItem* text_ = nullptr;
};
