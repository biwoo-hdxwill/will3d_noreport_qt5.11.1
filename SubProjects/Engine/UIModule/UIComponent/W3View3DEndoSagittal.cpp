#include "W3View3DEndoSagittal.h"
/*=========================================================================

File:			class CW3View3DEndoSagital
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2015-12-03
Last date:		2016-04-21

=========================================================================*/
#include <time.h>
#include <QFutureWatcher>
#include <qopenglwidget.h>
#include <QtConcurrent/QtConcurrent>
#include <QOpenGLFramebufferObject>	// 3d picking을 위한 FBO header
#include <qgraphicsproxywidget.h>
#include <QApplication>
#include <QMenu>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/define_view.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include <Engine/Resource/ResContainer/resource_container.h>
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_endo.h"
#endif
#include "../UIGLObjects/W3Spline3DItem.h"
#include "../UIGLObjects/W3GLObject.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/view_border_item.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

#include "../../../Managers/JobMgr/W3Jobmgr.h"
#include "../../../Managers/DBManager/W3DBM.h"

CW3View3DEndoSagittal::CW3View3DEndoSagittal(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
											 CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer,
											 common::ViewTypeID eType, QWidget *pParent)
	: CW3View3Dslice(VREngine, MPRengine, Rcontainer, eType, pParent), m_pgJobMgr(JobMgr) {
	m_pSpline3D = new CW3Spline3DItem();

	CreateDirectionText();
	direction_text_->setVisible(true);
	direction_text_->setPlainText("A");

	setInitScale();

	initializeMenus();
	connections();

	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());

	text_ = new TextItem(lang::LanguagePack::txt_pick_a_point_of_interest());
	scene()->addItem(text_);
	text_->hide();

	CreateSlider();
}

CW3View3DEndoSagittal::~CW3View3DEndoSagittal(void) {
	SAFE_DELETE_OBJECT(m_pDeletePathAct);
	SAFE_DELETE_OBJECT(m_pDeletePathPointAct);
	SAFE_DELETE_OBJECT(m_pInsertPathPointAct);

	SAFE_DELETE_OBJECT(m_pSplineMenu);
	SAFE_DELETE_OBJECT(m_pEllipseMenu);
	SAFE_DELETE_OBJECT(m_pSpline3D);

	if (m_pGLWidget)
		if (m_pGLWidget->context())
			clearGL();
}
#ifndef WILL3D_VIEWER
void CW3View3DEndoSagittal::exportProject(ProjectIOEndo& out) {
	CW3View3D::exportProject(out.GetViewIO());
	
	if (m_pSpline3D && m_pSpline3D->isEnd())
		out.SaveCurrPathNum(m_nCurPathNum);

	for (int path_id = 0; path_id < 5; ++path_id) {
		const auto& path = m_listEndoControlPointData[path_id];
		if (path.empty())
			continue;
		out.SavePath(path_id, path);
	}

	if (!m_meshSTL.empty())
		out.SaveAirway(m_meshSTL, m_dAirwaySize);
}

void CW3View3DEndoSagittal::importProject(ProjectIOEndo& in) {
	CW3View3D::importProject(in.GetViewIO());

	in.LoadCurrPathNum(m_nCurPathNum);

	for (int path_id = 0; path_id < 5; ++path_id) {
		in.LoadPath(path_id, m_listEndoControlPointData[path_id]);
	}

	in.LoadAirway(m_meshSTL, m_dAirwaySize);

	if (!isPathEmpty(m_nCurPathNum) && m_pSpline3D) {
		for (int i = 0; i < m_listEndoControlPointData[m_nCurPathNum].size(); i++)
			m_pSpline3D->addPoint(m_listEndoControlPointData[m_nCurPathNum].at(i));

		if (m_pSpline3D->endEdit()) {
			m_bLoadProject = true;

			m_pSpline3D->setVolRange(m_vVolRange);
			m_pSpline3D->generateSpline();

			m_nCameraPosInPath = 0;
			setCameraPos(m_nCameraPosInPath, false);

			// 해당 slot에 data가 있으면 CW3View3DEndo, CW3View3DEndoModify에 CW3Spline3DItem을 전달하고 view reset flag를 false로 설정
			//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			//emit view.sigUpdateEndoPath(view.m_pSpline3D, false);
			//emit sigUpdateEndoPath(m_pSpline3D, false);
			//emit sigSetEnableEndoPath(m_nCurPathNum, true);

			m_isItemInitialized = true;
			segAirway();
			emit sigAirwayDisabled(false);
		}

		emit sigSetPathNum(m_nCurPathNum);
	}
}
#endif
void CW3View3DEndoSagittal::reset() {
	CW3View3D::reset();

	for (int i = 0; i < 5; i++)
		if (!isPathEmpty(i))
			m_listEndoControlPointData[i].clear();

	resetView();
	CW3View3D::clearGL();
}

void CW3View3DEndoSagittal::reoriUpdate(glm::mat4 *M) {
	m_origModel = *M;

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	setModel();

	if (isVisible()) {
		//render3D();
		scene()->update();
	}
}

void CW3View3DEndoSagittal::setVisiblePath(int state) {
	switch (state) {
	case Qt::CheckState::Checked:
		m_bVisiblePath = true;
		break;
	case Qt::CheckState::Unchecked:
		m_bVisiblePath = false;
		break;
	}
	scene()->update();
}

void CW3View3DEndoSagittal::connections() {
	connect(m_pDeletePathAct, SIGNAL(triggered()), this, SLOT(slotDeletePathAct()));
	connect(m_pDeletePathPointAct, SIGNAL(triggered()), this, SLOT(slotDeletePathPointAct()));
	connect(m_pInsertPathPointAct, SIGNAL(triggered()), this, SLOT(slotInsertPathPointAct()));
}

void CW3View3DEndoSagittal::init() {
	CW3View3Dslice::init();

	m_fSlicePos = 0.0f;
	m_fPlaneDepth = 0.0f; // -1 ~ 1 : near to far of frustum

	if (m_pSpline3D)
		m_pSpline3D->setVolRange(m_vVolRange);

	if (m_pSlider)
		m_pSlider->setRange(-m_vVolRange.x, m_vVolRange.x);
}

void CW3View3DEndoSagittal::initializeMenus() {
	m_pSplineMenu = new QMenu(this);
	m_pEllipseMenu = new QMenu(this);

	m_pDeletePathAct = new  QAction(lang::LanguagePack::msg_46(), this);
	m_pDeletePathPointAct = new  QAction(lang::LanguagePack::msg_41(), this);
	m_pInsertPathPointAct = new QAction(lang::LanguagePack::msg_42(), this);

	m_pSplineMenu->addAction(m_pInsertPathPointAct);
	m_pSplineMenu->addAction(m_pDeletePathAct);

	m_pEllipseMenu->addAction(m_pDeletePathPointAct);
	m_pEllipseMenu->addAction(m_pDeletePathAct);
}

void CW3View3DEndoSagittal::setInitScale() {
	float viewRatio = float(width()) / float(height());
	float xratio, yratio;
	float glX, glY;

	if (width() > height()) {
		glX = m_camFOV * viewRatio;
		glY = m_camFOV;
	} else {
		glY = m_camFOV;
		glX = m_camFOV / viewRatio;
	}

	xratio = glX / m_vVolRange.y;
	yratio = glY / m_vVolRange.z;

	m_scale = std::min(xratio, yratio);
}

///////////////////////////////////////////////////////////////////////////
//
//	* resetView()
//	endo path를 삭제하거나 변경한 path slot이 비어있을 때, view 상태 초기화
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSagittal::resetView() {
	if (m_pGLWidget) {
		if (m_pGLWidget->context()) {
			m_pGLWidget->makeCurrent();
			m_pSpline3D->clearVAOVBO();
			m_pGLWidget->doneCurrent();
		}
	}
	setInitScale();

	m_fSlicePos = 0.0f;
	m_fPlaneDepth = 0.0f;
	m_nCameraPosInPath = 0;
	m_isItemInitialized = false;
	m_bDoubleClickPressed = false;

	m_pSpline3D->clear();

	m_Ztranslate = glm::mat4(1.0f);

	m_nPickedPointIndex = -1;

	m_meshSTL.clear();
	m_dAirwaySize = 0;

	if (isVisible())
		render3D();

	emit sigSetAirwaySize(m_dAirwaySize);
}

void CW3View3DEndoSagittal::drawBackground(QPainter *painter, const QRectF &rect) {
	CW3View3Dslice::drawBackground(painter, rect);

	painter->beginNativePainting();

	if (m_pgVREngine->isVRready()) {
		if (!m_is3Dready) {
			init();
		}


		if (m_is3Dready && m_bLoadProject) {
			// 해당 slot에 data가 있으면 CW3View3DEndo, CW3View3DEndoModify에 CW3Spline3DItem을 전달하고 view reset flag를 false로 설정
			//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			if (m_pSpline3D) {
				m_pGLWidget->doneCurrent();
				emit sigUpdateEndoPath(m_pSpline3D, false);
				m_pGLWidget->makeCurrent();
			}

			m_bLoadProject = false;
		}

		setMVPslice();

		m_invModel0 = glm::inverse(m_model)*m_Ztranslate;
		m_invModel = m_volvertTotexCoord * m_invModel0*m_modelSlice;

		m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_invModel = &m_invModel;
		m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_mvp = &m_mvp;
		m_pRender3DParam->m_width = this->width();
		m_pRender3DParam->m_height = this->height();

		if (!m_pRender3DParam->m_plane->getVAO()) {
			unsigned int vao = 0;
			m_pRender3DParam->m_plane->clearVAOVBO();

			m_pgVREngine->initVAOplane(&vao);
			m_pRender3DParam->m_plane->setVAO(vao);
			m_pRender3DParam->m_plane->setNindices(6);
		}

		m_pgVREngine->RenderSlice(m_pRender3DParam, m_drawVolId, m_bIsXray);

		if (m_pSpline3D) {
			if ((!m_pSpline3D->isEnd() && m_pSpline3D->isPathValid()) ||
				(m_pSpline3D->isEnd() && m_bVisiblePath)) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// src = src * src.a, dst = dst * (1 - src.a)

				m_mvpItem = m_projection * m_view*m_model;

				m_pSpline3D->setPlaneDepth(m_fPlaneDepth);
				m_pSpline3D->draw(m_pgVREngine->getPROGanno(), m_mvpItem);

				glDisable(GL_BLEND);
			}
		}

		glDisable(GL_DEPTH_TEST);
	} else {
		CW3GLFunctions::clearView(false);
	}

	painter->endNativePainting();
}

void CW3View3DEndoSagittal::leaveEvent(QEvent* event)
{
	CW3View3Dslice::leaveEvent(event);
	
	text_->hide();
}

void CW3View3DEndoSagittal::enterEvent(QEvent* event)
{
	CW3View3Dslice::enterEvent(event);

	if (m_pSpline3D)
	{
		if (m_pSpline3D->isEnd())
		{
			text_->hide();
		}
		else
		{
			text_->show();
		}
	}
}

void CW3View3DEndoSagittal::resizeEvent(QResizeEvent *pEvent) {
	CW3View3Dslice::resizeEvent(pEvent);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
							scene_size.x(), scene_size.y()));
}

void CW3View3DEndoSagittal::mousePressEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (m_pProxySlider && m_pProxySlider->isUnderMouse()) {
		QGraphicsView::mousePressEvent(event);

		QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
					   event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);
		return;

	}

	if (common_tool_type_ == common::CommonToolTypeOnOff::NONE) 
	{
		QGraphicsView::mousePressEvent(event);
		QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
					   event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);
		CW3Cursor::SetViewCursor(common::CommonToolTypeOnOff::NONE);

		last_view_pos_ = event->pos();
		last_scene_pos_ = mapToScene(event->pos());

		if (event->button() & Qt::LeftButton) 
		{
			if (!m_pSpline3D->isEnd())
				return;		

			// control point가 선택 되었으면 해당 point가 있는 slice로 이동하기 위한 pos 값 가져옴
			bool isSelectPoint = m_pSpline3D->getSelectedControlPointDepth(m_fSlicePos);
			if (isSelectPoint) 
			{
				m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, -m_fSlicePos));

				m_fPlaneDepth = m_fSlicePos / m_camFOV;
				m_pSpline3D->setPlaneDepth(m_fPlaneDepth);

				// 현재 선택된 control point index를 각 view에 전달
				//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
				//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
				emit sigUpdatePoint(m_nPickedPointIndex);
			}
		}
	} 
	else 
	{
		CW3View3D::mousePressEvent(event);
		CW3Cursor::SetViewCursor(common::CommonToolTypeOnOff::NONE);
	}
}

void CW3View3DEndoSagittal::mouseMoveEvent(QMouseEvent *event) 
{
	CW3Cursor::SetViewCursor(common::CommonToolTypeOnOff::NONE);

#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!isVisible())
		return;

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
		return QGraphicsView::mouseMoveEvent(event);

	QPoint pos = event->pos() + QPoint(20, 20);
	text_->setPos(pos);

	QPointF scene_pos = mapToScene(event->pos());
	DisplayDICOMInfo(scene_pos);

	if (common_tool_type_ == common::CommonToolTypeOnOff::NONE) 
	{
		QGraphicsView::mouseMoveEvent(event);		

		if (event->buttons() & Qt::LeftButton) 
		{
			if (m_pSpline3D->isEnd() && m_nPickedPointIndex > -1) 
			{
				// path 또는 control point를 이동을 위한 scene에서의 이동거리
				QPointF distInScene = scene_pos - last_scene_pos_;
				// path 또는 control point를 이동
				m_pSpline3D->translatePath(transformDistSliceToVolume(distInScene));
				last_scene_pos_ = scene_pos;

				scene()->update();

				// 선택된 index가 전체 control point list의 size보다 작으면 point, 같다면 path가 선택된 것
				if (m_nPickedPointIndex < m_pSpline3D->getControlPoint()->size())
					// 현재 선택된 control point index를 각 view에 update
					//	* connected SLOT : CW3View3DEndo::slotUpdatePoint(int)
					//	* connected SLOT : CW3View3DEndoModify::slotUpdatePoint(int)
					emit sigUpdatePoint(m_nPickedPointIndex);
				else
					// point가 선택되지 않았을 경우 상태 update
					//	* connected SLOT : CW3View3DEndo::slotUpdate()
					//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
					emit sigUpdate();
			}
		} 
		else 
		{
			if (!m_pSpline3D)
				return;

			// path 그리기 중이면 실시간으로 마우스 위치를 따라 path 가이드 생성
			if (!m_pSpline3D->isEnd() && m_pSpline3D->getControlPoint()->size() > 0) 
			{
				m_pSpline3D->addPoint(transformSliceToVolume(scene_pos));	// scene pos를 volume pos로 변경하여 CW3Spline3DItem으로 전달
				m_pSpline3D->generateSpline();	// 저장된 control point로 Spline 생성
				
				scene()->update();

				m_pSpline3D->removeLastPoint();	// 마지막 추가된 point를 삭제하여 기존 control point set 유지
			}
			// path 그리기가 완료된 경우 picking
			else if (m_pSpline3D->getControlPoint()->size() > 1) 
			{
				m_pGLWidget->makeCurrent();
				{
					QOpenGLFramebufferObject fbo(this->width(), this->height());
					fbo.bind();

					CW3GLFunctions::clearView(true, GL_BACK);
					glViewport(0, 0, this->width(), this->height());

					int prog = m_pgVREngine->getPROGpick();
					// pick된 point index 받아옴
					int newPickedPointIndex = m_pSpline3D->pick(event->pos(), prog, m_mvpItem);

					fbo.release();

					if (m_nPickedPointIndex == m_pSpline3D->getControlPoint()->size()) 
					{
						m_bIsHoveredPathSpline = true;
						m_bIsHoveredPathPoint = false;
					} 
					else if (m_nPickedPointIndex > 0) 
					{
						m_bIsHoveredPathSpline = false;
						m_bIsHoveredPathPoint = true;
					}

					// 선택 상태가 변경될 때만 scene update
					if (m_nPickedPointIndex == newPickedPointIndex)
						return;

					m_nPickedPointIndex = newPickedPointIndex;

					scene()->update();
				}
				m_pGLWidget->doneCurrent();

				// control point가 선택되었을 경우 CW3View3DEndoModify를 update
				//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
				emit sigUpdatePick();
			}
		}
	}

	if (mouseMoveEventW3(event)) {
		scene()->update();
		return;
	}
}

void CW3View3DEndoSagittal::mouseReleaseEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		if (event->button() == Qt::RightButton)
		{
			emit sigShowButtonListDialog(event->globalPos());
			return;
		}
	}
#endif // WILL3D_EUROPE

	if (m_pProxySlider && m_pProxySlider->isUnderMouse() ||
		m_pSlider && m_pSlider->pressed()) {
		QGraphicsView::mouseReleaseEvent(event);

		QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
					   event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);
		CW3Cursor::SetViewCursor(common::CommonToolTypeOnOff::NONE);
		return;
	}

	if (common_tool_type_ != common::CommonToolTypeOnOff::NONE)
		return CW3View2D::mouseReleaseEvent(event);

	QGraphicsView::mouseReleaseEvent(event);

	if (event->button() & Qt::LeftButton) {
		if (!m_pSpline3D)
			return;

		// spline이 생성되지 않았을 때, 마우스를 클릭하면 control point 생성
		if (!m_pSpline3D->isEnd()) {
			QPointF ptScene = mapToScene(event->pos());
			m_pSpline3D->addPoint(transformSliceToVolume(ptScene));
			m_pSpline3D->generateSpline();
		} else if (m_nPickedPointIndex > -1) {
			if (m_listEndoControlPointData[m_nCurPathNum] != m_pSpline3D->getControlPointData()) {
				m_listEndoControlPointData[m_nCurPathNum] = m_pSpline3D->getControlPointData();

				segAirway();
				insertAirwayToDB();
			}

			scene()->update();
		}
	} else if (event->button() & Qt::RightButton) {
		if (!m_pSpline3D)
			return;

		if (!m_pSpline3D->isEnd() || m_nPickedPointIndex < 0)
			return;
		
		if (m_bIsHoveredPathSpline) {
			m_pSplineMenu->popup(this->mapToGlobal(event->pos()));
			m_bIsHoveredPathSpline = false;
		} else if (m_bIsHoveredPathPoint) {
			m_pEllipseMenu->popup(this->mapToGlobal(event->pos()));
			m_bIsHoveredPathPoint = false;
		}
	}

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void CW3View3DEndoSagittal::mouseDoubleClickEvent(QMouseEvent* event) 
{
	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
	{
		QGraphicsView::mouseDoubleClickEvent(event);
		return;

	}

	if (common_tool_type_ != common::CommonToolTypeOnOff::NONE)
		return CW3View2D::mouseDoubleClickEvent(event);

	QGraphicsView::mouseDoubleClickEvent(event);

	if (event->button() & Qt::LeftButton) 
	{
		if (m_isItemInitialized)
			return;

		m_bDoubleClickPressed = true;

		// spline이 생성되지 않았을 때, 마우스를 더블클릭하면 spline 생성하고 초기 카메라 위치 설정
		if (m_pSpline3D->endEdit()) 
		{
			m_pSpline3D->generateSpline();

			m_listEndoControlPointData[m_nCurPathNum] = m_pSpline3D->getControlPointData();

			m_nCameraPosInPath = 0;
			setCameraPos(m_nCameraPosInPath, true);

			// spline이 생성되면 각 view에 CW3Spline3DItem * 을 전달
			//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			emit sigUpdateEndoPath(m_pSpline3D, false);

			// spline이 생성되면 Path tool의 해당 slot 삭제 버튼(X)을 활성화 시킴
			//  * connected SLOT : CW3ENDOViewMgr::slotSetEnableDeleteButton(const int, const bool);
			emit sigSetEnableEndoPath(m_nCurPathNum, true);

			m_isItemInitialized = true;

			segAirway();
			insertAirwayToDB();

			scene()->update();

			emit sigSpline3DEnd();
		} 
		else
		{
			m_pSpline3D->clear();

			m_isItemInitialized = false;
		}
	}
}

void CW3View3DEndoSagittal::segAirway() {
	bool res = false;

	if (m_bLoadProject) {
		res = true;
	} else {
		clock_t start = clock();
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(this, &CW3View3DEndoSagittal::runSegAirway);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
		clock_t end = clock();
		float elapsedTime = static_cast<float>(end - start) / CLOCKS_PER_SEC;

		if (m_meshSTL.size() > 0)
			res = true;

		printf("end doRun : %f, %d, %zd\r\n", elapsedTime, res, m_meshSTL.size());
	}

	if (res) {
		emit sigSetAirwaySize(m_dAirwaySize);
		emit sigSegAirway(m_meshSTL);
	} else {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_20(), CW3MessageBox::Information);
		msgBox.exec();
	}
}

void CW3View3DEndoSagittal::runSegAirway() {
	unsigned short **data = m_pgVREngine->getVol(0)->getData();
	int width = m_pgVREngine->getVol(0)->width();
	int height = m_pgVREngine->getVol(0)->height();
	int depth = m_pgVREngine->getVol(0)->depth();
	int min = m_pgVREngine->getVol(0)->getMin();
	float pixelSpacing = m_pgVREngine->getVol(0)->pixelSpacing();
	float intercept = m_pgVREngine->getVol(0)->intercept();

	// volume down 1/8
	unsigned short **downData = nullptr;
	int downWidth = width;
	int downHeight = height;
	int downDepth = depth;
	unsigned int downFactor = 2;

	W3::volumeDown(&downData, data, downFactor, downWidth, downHeight, downDepth);

	bool res = false;
	m_dAirwaySize = 0;
	m_meshSTL.clear();

	unsigned char **ppSegAirwayMask = nullptr;

	W3::p_allocate_volume(&ppSegAirwayMask, downDepth, downWidth * downHeight);

	for (int i = 0; i < downDepth; i++)
		std::memset(ppSegAirwayMask[i], 0, sizeof(unsigned char) * downWidth * downHeight);

	std::vector<glm::vec3> vecCP, vecPath;

	std::vector<vec3> *srcCP = m_pSpline3D->getControlPoint();
	std::vector<vec3> *srcPath = m_pSpline3D->getPath();

	for (int i = 0; i < srcCP->size(); i++) {
		glm::vec3 temp = (srcCP->at(i) + 1.0f) * 0.5f;
		glm::vec3 v(temp.x * (downWidth), temp.y * (downHeight), temp.z * (downDepth));
		vecCP.push_back(v);
	}

	for (int i = 0; i < srcPath->size(); i++) {
		glm::vec3 temp = (srcPath->at(i) + 1.0f) * 0.5f;
		glm::vec3 v(temp.x * (downWidth), temp.y * (downHeight), temp.z * (downDepth));
		vecPath.push_back(v);
	}

	res = m_pgJobMgr->runAirwaySeg(
		downData, ppSegAirwayMask,
		downWidth, downHeight, downDepth,
		min, pixelSpacing, intercept, vecCP, vecPath,
		downFactor, m_pgVREngine->getVol(0)->getAirTissueThreshold() + intercept);
	if (res) {
		//m_meshSTL = segAirway.getMeshSTL();
		m_meshSTL = m_pgJobMgr->getMeshSTL();
		for (int i = 0; i < m_meshSTL.size(); i++) {
			m_meshSTL.at(i).v1 *= downFactor;
			m_meshSTL.at(i).v2 *= downFactor;
			m_meshSTL.at(i).v3 *= downFactor;
		}

		int cnt = 0;
		for (int i = 0; i < downDepth; i++) {
			for (int j = 0; j < downWidth * downHeight; j++) {
				unsigned char b = ppSegAirwayMask[i][j];
				if (b)
					++cnt;
			}
		}

		m_dAirwaySize = cnt * (pixelSpacing * pixelSpacing * pixelSpacing) * (downFactor * downFactor * downFactor);
	}

	vecCP.clear();
	vecPath.clear();

	SAFE_DELETE_VOLUME(downData, downDepth);
	SAFE_DELETE_VOLUME(ppSegAirwayMask, downDepth);
}

///////////////////////////////////////////////////////////////////////////
//
//	* wheelEvent(QWheelEvent *event)
//	wheel을 돌리면 sagital plane을 slice 단위로 탐색
//	CW3Spline3DItem의 앞/뒤를 나누는 기준 depth 값 update
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSagittal::wheelEvent(QWheelEvent *event) {
	double degrees = event->delta() / 8.0;
	double dir = degrees / 15.0;

	float sliceMin = -m_vVolRange.x;
	float sliceMax = -sliceMin;

	float tmpPos = m_fSlicePos;
	m_fSlicePos += (float)dir;

	if (m_fSlicePos < sliceMin)
		m_fSlicePos = tmpPos;
	else if (m_fSlicePos > sliceMax)
		m_fSlicePos = tmpPos;
	if (m_pSlider)
		m_pSlider->setValue((int)m_fSlicePos);

	m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, -(m_fSlicePos)));

	m_fPlaneDepth = (m_fSlicePos) / (m_camFOV);

	m_pSpline3D->setPlaneDepth(m_fPlaneDepth);

	if (!m_pSpline3D->isEnd()) {
		QPointF ptScene = mapToScene(event->pos());
		m_pSpline3D->addPoint(transformSliceToVolume(ptScene));
		m_pSpline3D->generateSpline();
	}

	scene()->update();

	if (!m_pSpline3D->isEnd())
		m_pSpline3D->removeLastPoint();
}

void CW3View3DEndoSagittal::changedDeltaSlider(int delta) {
	m_fSlicePos = delta;
	m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, -(m_fSlicePos)));

	m_fPlaneDepth = (m_fSlicePos) / (m_camFOV);

	m_pSpline3D->setPlaneDepth(m_fPlaneDepth);

	scene()->update();
}

///////////////////////////////////////////////////////////////////////////
//
//	* transformSliceToVolume(QPointF ptScene)
//	scene pos를 volume pos로 변경
//
///////////////////////////////////////////////////////////////////////////
glm::vec3 CW3View3DEndoSagittal::transformSliceToVolume(const QPointF &ptScene) {
	QPointF scene_pos = ptScene - m_pntCurViewCenterinScene;

	glm::vec4 ptGLCoord = glm::vec4(-m_scaleSceneToGL * scene_pos.x() - m_WglTrans,
									m_scaleSceneToGL * scene_pos.y() + m_HglTrans, 0.0f, 1.0f);
	glm::vec4 ptVolCoord = m_invModel0 * ptGLCoord;
	//printf("(x, y, z) : %f, %f, %f\n", ptVolCoord.x, ptVolCoord.y, ptVolCoord.z);

	// Jung: GL 좌표계로 넘겨주는 것이 일관성 있음
	//return glm::vec3(m_vVolRange.x() - ptVolCoord.x, ptVolCoord.y, ptVolCoord.z);
	return glm::vec3(ptVolCoord.xyz);
}

///////////////////////////////////////////////////////////////////////////
//
//	* transformSliceToVolume(QPointF ptScene)
//	scene 에서의 이동 거리를 volume 에서의 이동 거리로 변경
//
///////////////////////////////////////////////////////////////////////////
glm::vec3 CW3View3DEndoSagittal::transformDistSliceToVolume(const QPointF &ptScene) {
	vec4 ptGLCoord = vec4(-m_scaleSceneToGL * ptScene.x(), m_scaleSceneToGL * ptScene.y(), 0.0f, 0.0f);
	vec4 ptVolCoord = m_invModel0 * ptGLCoord;

	return glm::vec3(ptVolCoord.xyz);
}

// CW3View3DEndo에서 카메라 전/후진을 하면 해당 index를 전달받아 sagital 상의 카메라 위치를 표시
//	* connected SIGNAL : CW3View3DEndo::sigSetCameraPos(const int, const bool)
void CW3View3DEndoSagittal::slotSetCameraPos(const int index, const bool isControlPoint) {
	m_nCameraPosInPath = index;
	setCameraPos(m_nCameraPosInPath, isControlPoint);

	if (isVisible())
		scene()->update();
}

///////////////////////////////////////////////////////////////////////////
//
//	* setCameraPos(const int index, const bool isControlPoint)
//	카메라 위치 표시를 rendering하기 위해 CW3Spline3DItem에 위치를 전달
//	isControlPoint가 true면 control point index, false면 spline point index
//	현재 sagittal plane을 해당 point가 위치한 slice로 이동
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSagittal::setCameraPos(const int index, const bool isControlPoint) {
	if (!m_pSpline3D)
		return;

	if (!m_pSpline3D->isPathValid())
		return;

	std::vector<glm::vec3> *element;
	if (isControlPoint)
		element = m_pSpline3D->getControlPoint();	// 카메라 위치가 control point 일 때
	else
		element = m_pSpline3D->getPath();	// 카메라 위치가 spline point 일 떄

	m_fSlicePos = element->at(index).x * m_vVolRange.x;
	m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, -(m_fSlicePos)));
	m_fPlaneDepth = (m_fSlicePos) / (m_camFOV);

	if (m_pSlider)
		m_pSlider->setValue((int)m_fSlicePos);
}

// endo path slot(1 ~ 5) 버튼을 누르면 현재 slot을 해당 slot으로 변경
// 현재 CW3Spline3DItem이 있다면 control point set을 저장하고, 해당 slot의 data를 로드
// 해당 slot의 data가 없다면 slot 번호만 변경하고 모든 view reset
void CW3View3DEndoSagittal::slotSelectPath(const EndoPathID& path_id) {
	int path_number = static_cast<EndoPathID>(path_id);
	if (m_nCurPathNum == path_number)
		return;

	if (!m_pSpline3D)
		return;

	if (m_pSpline3D->isEnd())
		m_listEndoControlPointData[m_nCurPathNum] = m_pSpline3D->getControlPointData();

	m_nCurPathNum = path_number;

	resetView();

	if (!isPathEmpty(path_number) && m_pSpline3D) {
		for (int i = 0; i < m_listEndoControlPointData[path_number].size(); i++)
			m_pSpline3D->addPoint(m_listEndoControlPointData[path_number].at(i));

		if (m_pSpline3D->endEdit()) {
			m_bVisiblePath = false;

			m_pSpline3D->generateSpline();

			m_nCameraPosInPath = 0;
			setCameraPos(m_nCameraPosInPath, true);

			segAirway();

			// 해당 slot에 data가 있으면 CW3View3DEndo, CW3View3DEndoModify에 CW3Spline3DItem을 전달하고 view reset flag를 false로 설정
			//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
			emit sigUpdateEndoPath(m_pSpline3D, false);

			m_isItemInitialized = true;

			m_bVisiblePath = true;
		}
	} else {
		// 해당 slot에 data가 없으면 CW3View3DEndo, CW3View3DEndoModify에 nullptr를 전달하고 view reset flag를 true로 설정
		//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
		//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
		emit sigUpdateEndoPath(nullptr, true);
	}

	if (isVisible())
		scene()->update();
}

// endo path slot(1 ~ 5) 삭제 버튼을 누르면 현재 slot의 control point set을 삭제
// 삭제한 slot이 현재 slot과 같다면 모든 view reset
void CW3View3DEndoSagittal::slotDeletePath(const EndoPathID& path_id) {
	if (!isPathEmpty(path_id)) {
		m_listEndoControlPointData[path_id].clear();
	}

	if (m_nCurPathNum == path_id) {
		resetView();

		//	* 다른 view의 reset을 위해 spline 객체를 null을 전달하고 reset flag를 true로 설정
		//	* connected SLOT : CW3View3DEndo::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
		//	* connected SLOT : CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *, const bool)
		emit sigUpdateEndoPath(nullptr, true);
	}

	if (isVisible())
		scene()->update();
}

///////////////////////////////////////////////////////////////////////////
//
//	* isPathEmpty(const int index)
//	현재 path slot이 비어있는지 여부 return
//
///////////////////////////////////////////////////////////////////////////
bool CW3View3DEndoSagittal::isPathEmpty(const int index) {
	return (m_listEndoControlPointData[index].size() > 1) ? false : true;
}

///////////////////////////////////////////////////////////////////////////
//
//	* drawPlaneRect(unsigned int vao, float lineWidth, vec4 color)
//	GL_LINES로 테두리를 그리고 GL_QUADS로 내부를 채운 사각형을 rendering
//	현재 slice의 앞/뒤를 판별하기 위해 m_fPlaneDepth를 shader로 전달
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSagittal::drawPlaneRect(unsigned int vao, float lineWidth, const glm::vec4& color) {}

// Scene update
//	* connected SIGNAL : CW3View3DEndoModify::sigUpdate()
void CW3View3DEndoSagittal::slotUpdate() {
	if (isVisible())
		scene()->update();
}

// CW3View3DEndoModify의 modify mode에서 control point를 이동하면 spline update
//	* connected SIGNAL : CW3View3DEndoModify::sigUpdatePoint(int)
void CW3View3DEndoSagittal::slotUpdatePoint(int index) {
	int selectedPoint = m_pSpline3D->getSelectedPoint();
	if (selectedPoint > -1 && selectedPoint < m_pSpline3D->getControlPoint()->size()) {
		if (m_pSpline3D->getControlPointDepth(selectedPoint, m_fSlicePos)) {
			m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, -(m_fSlicePos)));
			m_fPlaneDepth = (m_fSlicePos) / (m_camFOV);
		}
	}

	if (isVisible())
		scene()->update();
}

void CW3View3DEndoSagittal::slotDeletePathAct() {
	emit sigSetEnableEndoPath(m_nCurPathNum, false);
	slotDeletePath(static_cast<EndoPathID>(m_nCurPathNum));
}

void CW3View3DEndoSagittal::slotDeletePathPointAct() {
	if (m_nPickedPointIndex < 0 || m_nPickedPointIndex > m_pSpline3D->getControlPoint()->size())
		return;

	std::vector<glm::vec3> *controlPoint = m_pSpline3D->getControlPoint();

	if (controlPoint->size() < 3)
		return;

	controlPoint->erase(controlPoint->begin() + m_nPickedPointIndex);
	m_pSpline3D->generateSpline();

	m_nPickedPointIndex = -1;
	m_nCameraPosInPath = 0;
	setCameraPos(m_nCameraPosInPath, true);

	emit sigUpdateEndoPath(m_pSpline3D, true);

	segAirway();
	insertAirwayToDB();

	if (isVisible())
		scene()->update();
}

void CW3View3DEndoSagittal::slotInsertPathPointAct() {
	glm::vec3 insertPoint = transformSliceToVolume(last_scene_pos_);
	int nearestIndex = getNearestPointInPath(insertPoint);
	int insertIndex = -1;

	std::vector<glm::vec3> *controlPoint = m_pSpline3D->getControlPoint();

	for (int i = 0; i < controlPoint->size(); i++) {
		int index = getNearestPointInPath(controlPoint->at(i));

		if (nearestIndex < index) {
			insertIndex = i;
			break;
		}
	}

	if (insertIndex == -1)
		return;

	auto iter = controlPoint->begin() + insertIndex;
	controlPoint->insert(iter, insertPoint);
	m_pSpline3D->generateSpline();

	m_nPickedPointIndex = -1;
	m_nCameraPosInPath = 0;
	setCameraPos(m_nCameraPosInPath, true);

	emit sigUpdateEndoPath(m_pSpline3D, false);

	insertAirwayToDB();

	if (isVisible())
		scene()->update();
}

int CW3View3DEndoSagittal::getNearestPointInPath(glm::vec3 point) {
	glm::vec3 ptControl = point;

	float preLen = 65535.0f, curLen = 0.0f;
	int index = 0;
	for (int i = 0; i < m_pSpline3D->getPath()->size(); i++) {
		glm::vec3 ptPath = m_pSpline3D->getPath()->at(i);
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

void CW3View3DEndoSagittal::slotDrawAirway(int state) {}

void CW3View3DEndoSagittal::ResetView() {
	if (isVisible()) {
		if (m_pSlider)
			m_pSlider->setValue((int)m_fSlicePos);

		m_Ztranslate = glm::translate(glm::vec3(0.0f, 0.0f, m_fSlicePos));

		m_fPlaneDepth = (m_fSlicePos) / (m_camFOV);

		if (m_pSpline3D)
			m_pSpline3D->setPlaneDepth(m_fPlaneDepth);

		CW3View2D::ResetView();

		setInitScale();
		setProjection();
	}
}

void CW3View3DEndoSagittal::FitView() {
	if (isVisible()) {
		CW3View2D::FitView();

		setInitScale();
		setProjection();
	}
}

void CW3View3DEndoSagittal::clearGL() {
	if (m_pGLWidget) {
		if (m_pGLWidget->context()) {
			m_pGLWidget->makeCurrent();
			if(m_pSpline3D)
				m_pSpline3D->clearVAOVBO();
			m_pGLWidget->doneCurrent();
		}
	}

	CW3View3D::clearGL();
}

void CW3View3DEndoSagittal::keyPressEvent(QKeyEvent * event) 
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (event->key() == Qt::Key_Escape) 
	{
		if (m_pSpline3D) {
			if (!m_pSpline3D->isEnd()) {
				m_pSpline3D->clear();

				m_isItemInitialized = false;

				if (isVisible())
					scene()->update();
			}
		}
	}
	QWidget::keyPressEvent(event);
}

void CW3View3DEndoSagittal::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3Dslice::keyReleaseEvent(event);
}

void CW3View3DEndoSagittal::insertAirwayToDB() {
	if (!m_pSpline3D)
		return;

	CW3DBM::getInstance()->insertAirway(
		m_pgVREngine->getVol(0)->getHeader()->getStudyID(),
		m_pgVREngine->getVol(0)->getHeader()->getSeriesID(),
		m_nCurPathNum,
		*m_pSpline3D->getControlPoint());
}

void CW3View3DEndoSagittal::DisplayDICOMInfo(const QPointF & scene_pos) {
	glm::vec3 vol_coord = transformSliceToVolume(scene_pos);
	vol_coord = (vol_coord + glm::vec3(1.0f))*0.5f * m_vVolRange;
	glm::vec4 volume_info = ResourceContainer::GetInstance()->GetMainVolume().GetVolumeInfo(vol_coord);

	if (volume_info.w != common::dicom::kInvalidHU) {
		HU_value_->SetText(QString("WL %1\nWW %2\n(%3, %4, %5), %6")
						   .arg(m_nAdjustWindowLevel + ResourceContainer::GetInstance()->GetMainVolume().intercept()).arg(m_nAdjustWindowWidth)
						   .arg(volume_info.x).arg(volume_info.y)
						   .arg(volume_info.z).arg(volume_info.w));
	} else {
		HU_value_->SetText(QString("WL %1\nWW %2\n(-, -, -)")
						   .arg(m_nAdjustWindowLevel + ResourceContainer::GetInstance()->GetMainVolume().intercept()).arg(m_nAdjustWindowWidth));
	}
}
