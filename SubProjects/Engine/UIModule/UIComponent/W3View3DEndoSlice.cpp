#include "W3View3DEndoSlice.h"
/*=========================================================================

File:			class CW3View3DEndoSlice
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last date:		2016-06-17

=========================================================================*/
#include <qevent.h>
#include <qopenglwidget.h>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"

#include "../UIGLObjects/W3GLObject.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/view_border_item.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

namespace {
	const glm::vec3 kInitCameraPos = glm::vec3(0.0f, -1.0f, 0.0f);
	const glm::vec3 kInitCameraCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	const glm::vec3 kInitCameraUpVector = glm::vec3(0.0f, 0.0f, -1.0f);
	const glm::vec3 kInitCameraSideVector = glm::vec3(1.0f, 0.0f, 0.0f);
}

CW3View3DEndoSlice::CW3View3DEndoSlice(
	CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
	QWidget *pParent)
	: CW3View3DEndo(VREngine, MPRengine, Rcontainer, eType, pParent) {
	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());
}

CW3View3DEndoSlice::~CW3View3DEndoSlice(void) {}

void CW3View3DEndoSlice::reset() {
	CW3View3DEndo::reset();
	slotSliceReset();
}

void CW3View3DEndoSlice::init() {
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	CW3View3DEndo::init();

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	setWLWW();
	int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;

	unsigned int PROGendoPlane = m_pgVREngine->getPROGendoPlane();
	glUseProgram(PROGendoPlane);

	WGLSLprogram::setUniform(PROGendoPlane, "WindowLevel",
		(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
	WGLSLprogram::setUniform(PROGendoPlane, "WindowWidth",
		(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	if (ruler_)
		ruler_->setVisible(false);

	if (m_pWorldAxisItem)
		m_pWorldAxisItem->setVisible(false);
}

void CW3View3DEndoSlice::reoriUpdate(glm::mat4 *m) {
	m_inverseM = *m;
	m_inverseM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	m_inverseM = glm::inverse(m_inverseM);

	this->SetNavigatorDirection();
}

void CW3View3DEndoSlice::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();
	if (m_pgVREngine->isVRready()) {
		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
		if (!m_is3Dready)
			init();

		unsigned int PROGendoPlane = m_pgVREngine->getPROGendoPlane();
		glUseProgram(PROGendoPlane);

		int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;
		WGLSLprogram::setUniform(PROGendoPlane, "WindowLevel",
			(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
		WGLSLprogram::setUniform(PROGendoPlane, "WindowWidth",
			(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());

		glUseProgram(0);

		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
		m_mvp = m_projection * m_view * m_model;
		m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_mvp = &m_mvp;
		m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_invModel = &m_modelNealClipPlane;
		m_pRender3DParam->m_width = this->width();
		m_pRender3DParam->m_height = this->height();

		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
		if (!m_pRender3DParam->m_plane->getVAO()) {
			unsigned int vao = 0;
			m_pRender3DParam->m_plane->clearVAOVBO();

			m_pgVREngine->initVAOplane(&vao);
			m_pRender3DParam->m_plane->setVAO(vao);
			m_pRender3DParam->m_plane->setNindices(6);
		}

		m_pgVREngine->RenderSliceEndo(m_pRender3DParam, m_drawVolId, m_isReconSwitched);
	} else {
		CW3GLFunctions::clearView(false);
	}
	painter->endNativePainting();
}

void CW3View3DEndoSlice::resizeEvent(QResizeEvent *pEvent) {
	CW3View3D::resizeEvent(pEvent);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
							scene_size.x(), scene_size.y()));
}

void CW3View3DEndoSlice::resizeScene() {
	CW3View3D::resizeScene();
}

///////////////////////////////////////////////////////////////////////////
//
//	* setProjection()
//	CW3View3DEndo의 projection은 perspective이기 때문에 따로 ortho로 생성
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSlice::setProjection() {
	CW3View3D::setProjection();
}

///////////////////////////////////////////////////////////////////////////
//
//	* mouseMoveEvent(QMouseEvent *event)
//	* mousePressEvent(QMouseEvent *event)
//	* mouseReleaseEvent(QMouseEvent *event)
//	Slice에선 카메라 회전을 막음
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSlice::mouseMoveEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D::mouseMoveEvent(event);

	QPointF scene_pos = mapToScene(event->pos());
	if (event->buttons() & Qt::LeftButton) {
		switch (common_tool_type_) {
		case common::CommonToolTypeOnOff::V_LIGHT:
			m_nAdjustWindowLevel += (last_scene_pos_.x() - scene_pos.x()) * 2;
			m_nAdjustWindowWidth += (last_scene_pos_.y() - scene_pos.y()) * 2;

			last_scene_pos_ = scene_pos;
			scene()->update();
			break;
		default:
			break;
		}
	}
}

void CW3View3DEndoSlice::mousePressEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D::mousePressEvent(event);
}

void CW3View3DEndoSlice::mouseReleaseEvent(QMouseEvent *event) 
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

	CW3View2D::mouseReleaseEvent(event);
}

void CW3View3DEndoSlice::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3DEndo::keyPressEvent(event);
}

void CW3View3DEndoSlice::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3DEndo::keyReleaseEvent(event);
}

///////////////////////////////////////////////////////////////////////////
//
//	* wheelEvent(QWheelEvent *event)
//	Endo line을 따라 전/후진하기 위한 wheel event
//	카메라의 위치는 모두 CW3View3DEndo에서 담당하기 때문에
//	QWheelEvent를 전달하여 결정된 카메라 정보를 다시 받게 됨
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoSlice::wheelEvent(QWheelEvent *event) {
	// wheel event를 CW3View3DEndo로 전달
	//	* connected SLOT : CW3View3DEndo::slotWheelEvent(QWheelEvent *)
	emit sigWheelEvent(event);
}

void CW3View3DEndoSlice::SetNavigatorDirection() {
	glm::mat4 init_view = glm::lookAt(kInitCameraPos, kInitCameraCenter, kInitCameraUpVector);
	glm::vec3 init_side_vector = glm::vec3(init_view[0][0], init_view[1][0], init_view[2][0]);

	glm::vec3 up_vector = glm::vec3(m_view[0][1], m_view[1][1], m_view[2][1]);
	glm::vec3 side_vector = glm::vec3(m_view[0][0], m_view[1][0], m_view[2][0]);

	glm::mat4 plane_rotate = GLTransformFunctions::GetRotMatrixVecToVec(kInitCameraUpVector, up_vector);
	glm::mat4 plane_rotate_right = GLTransformFunctions::GetRightRotateMatrix(plane_rotate,
																			  side_vector, init_side_vector);

	m_pWorldAxisItem->SetWorldAxisDirection(glm::inverse(plane_rotate_right*plane_rotate), init_view);
}
// CW3View3DEndo와 연동을 위해 view matrix를 받아 사용
//	* connected SIGNAL : CW3View3DEndo::sigSliceUpdate(const mat4)
//	*

void CW3View3DEndoSlice::slotSliceUpdate(const mat4 &view, const mat4 &invModel, int slicePos, int sliceRange) {
	m_view = view;


	this->SetNavigatorDirection();
	m_modelNealClipPlane = invModel;

	if (ruler_)
		ruler_->setVisible(show_rulers_);

	if (m_pWorldAxisItem)
		m_pWorldAxisItem->setVisible(true);

	if (isVisible())
		scene()->update();
}

void CW3View3DEndoSlice::slotSliceReset() {
	setViewMatrix();
	m_modelNealClipPlane = glm::mat4(1.0f);

	if (ruler_)
		ruler_->setVisible(false);

	if (m_pWorldAxisItem)
		m_pWorldAxisItem->setVisible(false);
}

void CW3View3DEndoSlice::InvertView(bool bToggled) {
	m_bInvertWindowWidth = bToggled;

	int windowWidth = m_nAdjustWindowWidth;

	if (bToggled)
		windowWidth = -m_nAdjustWindowWidth;

	if (isVisible()) {
		m_pGLWidget->makeCurrent();

		unsigned int PROGendoPlane = m_pgVREngine->getPROGendoPlane();
		glUseProgram(PROGendoPlane);
		WGLSLprogram::setUniform(PROGendoPlane, "WindowWidth",
			(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
		glUseProgram(0);

		m_pGLWidget->doneCurrent();
		scene()->update();
	}
}
