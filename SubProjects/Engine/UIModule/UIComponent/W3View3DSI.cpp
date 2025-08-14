#include "W3View3DSI.h"
/*=========================================================================

File:			class CW3View3DSI
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-04-22
Last date:		2016-04-22

=========================================================================*/

#include <QDebug>
#include <qmath.h>
#include <qevent.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qopenglwidget.h>
#include <QOpenGLFramebufferObject>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_si.h"
#endif
#include "../UIGLObjects/W3SurfaceAxesItem.h"
#include "../UIPrimitive/view_border_item.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VREngine.h"

using namespace UIGLObjects;

namespace {
const glm::mat4 kIMat = glm::mat4(1.0f);
}

CW3View3DSI::CW3View3DSI(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
						 CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
						 QWidget *pParent) :
	CW3View3D(VREngine, MPRengine, Rcontainer, eType, pParent) {
	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());
}

CW3View3DSI::~CW3View3DSI() {}
#ifndef WILL3D_VIEWER
void CW3View3DSI::exportProject(ProjectIOSI& out) {
	CW3View3D::exportProject(out.GetViewIO());
	out.SaveSecondTranslate(m_translateSecond);
	out.SaveSecondRotate(m_rotateSecond);
	out.SaveSecondTranslateForMPR(m_translateSecondForMPR);
	out.SaveSecondRotateForMPR(m_rotateSecondForMPR);
}

void CW3View3DSI::importProject(ProjectIOSI& in) {
	CW3View3D::importProject(in.GetViewIO());
	in.LoadSecondTranslate(m_translateSecond);
	in.LoadSecondRotate(m_rotateSecond);
	in.LoadSecondTranslateForMPR(m_translateSecondForMPR);
	in.LoadSecondRotateForMPR(m_rotateSecondForMPR);
}
#endif
void CW3View3DSI::UpdateVR(bool is_high_quality) {
  m_pRender3DParam->m_isLowRes = !is_high_quality;
  RenderAndUpdate();
}
void CW3View3DSI::reset() {
	CW3View3D::reset();

	m_bShowSecondVolume = false;
	m_isTranslatingSecondVolume = false;
	m_isRotatingSecondVolume = false;

	m_secondToFirstModelOrig = kIMat;
	m_secondTransform = kIMat;
	m_translateSecondForMPR = kIMat;
	m_rotateSecondForMPR = kIMat;

	m_PROGsurface = 0;
	m_PROGpick = 0;

	SAFE_DELETE_OBJECT(m_pAxes);
}

void CW3View3DSI::drawBackground(QPainter *painter, const QRectF &rect) {
	CW3View3D::drawBackground(painter, rect);

	painter->beginNativePainting();

	if (m_pAxes && m_bShowSecondVolume) {
		//m_pAxes->setTransformMat(m_translateSecond, TRANSLATE);
		m_pAxes->setTransformMat(m_rotate, ARCBALL);
		m_pAxes->setProjViewMat(m_projection, m_view);

		glEnable(GL_DEPTH_TEST);

		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		glFrontFace(GL_CCW);
		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(m_PROGsurface);
		m_pAxes->draw(m_PROGsurface);
		glUseProgram(0);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}

	painter->endNativePainting();

	m_isOnlyItemUpdate = false;
}

void CW3View3DSI::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		if (m_pAxes)
			m_pAxes->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	}

	CW3View3D::clearGL();
}

void CW3View3DSI::resizeEvent(QResizeEvent * event) {
	CW3View3D::resizeEvent(event);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
							scene_size.x(), scene_size.y()));
}

void CW3View3DSI::RenderAndUpdate() {
	if (!isVisible())
		return;
	render3D();
	scene()->update();
}

void CW3View3DSI::init() {
	CW3View3D::init();

	m_PROGsurface = m_pgVREngine->getPROGsurface();
	m_PROGpick = m_pgVREngine->getPROGpickWithCoord();

	m_pAxes = new CW3SurfaceAxesItem();
	m_pAxes->setTransformMat(glm::scale(glm::vec3(glm::length(m_vVolRange / 3.0f))), SCALE);
	m_pAxes->setTransformMat(m_translateSecond * glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)), TRANSLATE);
	m_pAxes->setTransformMat(m_rotateSecond * glm::rotate(float(M_PI), vec3(1.0f, 0.0f, 0.0f)), ROTATE);

	glUseProgram(m_PROGsurface);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Intensity", vec3(1.0f));
	vec4 lightPos = glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Position", m_view * lightPos);
}

void CW3View3DSI::SecondVolumeLoaded(const glm::mat4& secondModel) {
	m_secondToFirstModelOrig = secondModel;
	m_secondToFirstModel = secondModel;

	if (!m_pRender3DParam->m_pgMainVolume[0])
		m_pRender3DParam->m_pgMainVolume[0] = m_pgVREngine->getVRparams(0);
	m_pRender3DParam->m_pgMainVolume[1] = m_pgVREngine->getVRparams(1);
	m_scaleMatSecond = glm::scale(*m_pgVREngine->getVolRange(1) * m_pgMPRengine->getBasePixelSize(1) / m_pgMPRengine->getBasePixelSize(0)); // * movingPixelSize/refPixelSize

	m_translateSecond = kIMat;
	m_rotateSecond = kIMat;

	m_translateSecondForMPR = kIMat;
	m_rotateSecondForMPR = kIMat;

	if (m_pAxes) {
		m_pAxes->setTransformMat(m_translateSecond * glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)), TRANSLATE);
		m_pAxes->setTransformMat(m_rotateSecond * glm::rotate(float(M_PI), vec3(1.0f, 0.0f, 0.0f)), ROTATE);
	}

	m_modelSecond = m_rotate * m_translateSecond*m_rotateSecond*m_origModel*m_secondToFirstModel*m_scaleMatSecond;
	m_bShowSecondVolume = true;

	//RenderAndUpdate();
}

void CW3View3DSI::VisibleMain(bool state) {
	qDebug() << "CW3View3DSI::VisibleMain :" << state;

	if (!m_pRender3DParam)
		return;

	if (!m_pRender3DParam->m_pgMainVolume[0])
		return;

	if (state) {
		m_drawVolId = 0;
		m_isDrawBoth = false;

		m_pRender3DParam->m_pgMainVolume[0]->m_isShown = true;

		if (m_pRender3DParam->m_pgMainVolume[1])
			m_pRender3DParam->m_pgMainVolume[1]->m_isShown = false;

		m_pgVREngine->setTFtoVolume(m_drawVolId);

		if (m_pGLWidget) {
			m_pGLWidget->makeCurrent();
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId],
										 m_drawVolId);
			m_pGLWidget->doneCurrent();
		}

		m_bShowSecondVolume = false;
		RenderAndUpdate();
	}
}

void CW3View3DSI::VisibleSecond(bool state) {
	qDebug() << "CW3View3DSI::VisibleSecond :" << state;

	if (!m_pRender3DParam)
		return;

	if (!m_pRender3DParam->m_pgMainVolume[1])
		return;

	if (state) {
		m_drawVolId = 1;
		m_isDrawBoth = false;

		m_pRender3DParam->m_pgMainVolume[1]->m_isShown = true;

		if (m_pRender3DParam->m_pgMainVolume[0])
			m_pRender3DParam->m_pgMainVolume[0]->m_isShown = false;

		m_pgVREngine->setTFtoVolume(m_drawVolId);

		if (m_pGLWidget) {
			m_pGLWidget->makeCurrent();
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
			m_pGLWidget->doneCurrent();
		}

		m_bShowSecondVolume = true;

		RenderAndUpdate();
	}
}

void CW3View3DSI::VisibleBoth(bool state) {
	qDebug() << "CW3View3DSI::VisibleBoth :" << state;

	if (!m_pRender3DParam)
		return;

	if (!m_pRender3DParam->m_pgMainVolume[1] ||
		!m_pRender3DParam->m_pgMainVolume[0])
		return;

	if (state) {
		m_drawVolId = 0;
		m_isDrawBoth = true;

		m_pRender3DParam->m_pgMainVolume[1]->m_isShown = true;
		m_pRender3DParam->m_pgMainVolume[0]->m_isShown = true;

		m_pgVREngine->setTFtoVolume(0);
		m_pgVREngine->setTFtoVolume(1);

		if (m_pGLWidget) {
			m_pGLWidget->makeCurrent();
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
			m_pGLWidget->doneCurrent();
		}

		m_isReconSwitched = true;
		m_bShowSecondVolume = true;
		RenderAndUpdate();
	}
}

bool CW3View3DSI::mousePressEventW3(QMouseEvent *event) {
	if (!m_is3Dready)
		return false;

	CW3View3D::mousePressEventW3(event);

	if (event->buttons() & Qt::LeftButton) {
		if (m_pAxes) {
			if (m_pAxes->isSelectTranslate()) {
				m_isTranslatingSecondVolume = true;
				return true;
			} else if (m_pAxes->isSelectRotate()) {
				m_isRotatingSecondVolume = true;
				return true;
			}
		}
	}

	return false;
}

bool CW3View3DSI::mouseMoveEventW3(QMouseEvent *event) {
	if (!m_is3Dready)
		return false;

	QPointF scene_pos = mapToScene(event->pos());
	if (event->buttons() & Qt::LeftButton) {
		if (m_isTranslatingSecondVolume) {
			QPointF transPos = scene_pos - last_scene_pos_;
			transPos -= (QPoint(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL));

			vec3 glTrans = vec3(transPos.x()*m_scaleSceneToGL / ((proj_right_ - proj_left_)*0.5f),
								-transPos.y()*m_scaleSceneToGL / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);

			vec3 vTrans = m_pAxes->translate(glTrans);
			m_translateSecond *= glm::translate(vTrans);

			m_pRender3DParam->m_isLowRes = true;

			last_scene_pos_ = scene_pos;

			setModel();

			vec3 trans = vTrans;
			trans.x = -trans.x;
			m_translateSecondForMPR *= glm::translate(trans);

			m_secondTransform = m_translateSecondForMPR * m_rotateSecondForMPR;
			m_pgMPRengine->SecondTransform(m_secondTransform);
			emit sigSetTranslateMatSecondVolume(&m_translateSecond);

			return true;
		} else if (m_isRotatingSecondVolume) {
			QPointF scene_offset = m_scaleSceneToGL*(scene_pos - m_pntCurViewCenterinScene);
			vec3 glCur(scene_offset.x() / ((proj_right_ - proj_left_)*0.5f),
					   -scene_offset.y() / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);

			QPointF prev_scene_pffset = m_scaleSceneToGL * (last_scene_pos_ - m_pntCurViewCenterinScene);
			vec3 glLast(prev_scene_pffset.x() / ((proj_right_ - proj_left_)*0.5f),
						-prev_scene_pffset.y() / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);

			QPair<float, vec3> pairRot = m_pAxes->rotate(glCur, glLast);
			m_rotateSecond = glm::rotate(pairRot.first, pairRot.second) * m_rotateSecond;

			m_pRender3DParam->m_isLowRes = true;

			last_scene_pos_ = scene_pos;

			setModel();

			vec3 rotAxisForMPR = pairRot.second;
			rotAxisForMPR.x = -rotAxisForMPR.x; // x축 반전
			m_rotateSecondForMPR = glm::rotate(-pairRot.first, rotAxisForMPR) * m_rotateSecondForMPR;

			m_secondTransform = m_translateSecondForMPR * m_rotateSecondForMPR;
			m_pgMPRengine->SecondTransform(m_secondTransform);

			emit sigSetRotateMatSecondVolume(&m_rotateSecond);

			return true;
		}
	} else {
		if (m_pGLWidget) {
			m_pGLWidget->makeCurrent();
			if (m_bShowSecondVolume) {
				glUseProgram(m_PROGpick);
				QOpenGLFramebufferObject fbo(this->width(), this->height(), QOpenGLFramebufferObject::Depth);
				fbo.bind();
				CW3GLFunctions::clearView(true, GL_BACK);
				glViewport(0, 0, this->width(), this->height());

				m_pAxes->render_for_pick(m_PROGpick);
				bool shouldRedraw = false;
				m_pAxes->pick(event->pos(), &shouldRedraw);
				fbo.release();
				glUseProgram(0);
				if (shouldRedraw)
					m_isOnlyItemUpdate = true;
			}
			m_pGLWidget->doneCurrent();
		}
	}

	if (CW3View3D::mouseMoveEventW3(event))
		return true;

	return false;
}

bool CW3View3DSI::mouseReleaseEventW3(QMouseEvent *event) {
	if (!m_is3Dready)
		return false;

	m_isTranslatingSecondVolume = false;
	m_isRotatingSecondVolume = false;

	return CW3View3D::mouseReleaseEventW3(event);
}

void CW3View3DSI::ResetMatrixToAuto() {
	m_secondToFirstModel = m_secondToFirstModelOrig;

	m_translateSecond = kIMat;
	m_rotateSecond = kIMat;

	m_translateSecondForMPR = kIMat;
	m_rotateSecondForMPR = kIMat;

	m_pAxes->setTransformMat(glm::translate(glm::vec3(0.0f, 0.0f, 0.0f)), TRANSLATE);
	m_pAxes->setTransformMat(glm::rotate(float(M_PI), vec3(1.0f, 0.0f, 0.0f)), ROTATE);

	setModel();

	m_secondTransform = m_translateSecondForMPR * m_rotateSecondForMPR;
	m_pgMPRengine->SecondTransform(m_secondTransform);
	emit sigSetTranslateMatSecondVolume(&m_translateSecond);
	emit sigSetRotateMatSecondVolume(&m_rotateSecond);
	RenderAndUpdate();
}

#ifdef WILL3D_EUROPE
void CW3View3DSI::mousePressEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mousePressEvent(event);
}

void CW3View3DSI::mouseReleaseEvent(QMouseEvent* event)
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

void CW3View3DSI::mouseMoveEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mouseMoveEvent(event);
}

void CW3View3DSI::keyPressEvent(QKeyEvent* event)
{
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
	
	CW3View3D::keyPressEvent(event);
}

void CW3View3DSI::keyReleaseEvent(QKeyEvent* event)
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
