#include "W3View3DFaceMesh.h"
/*=========================================================================

File:			class CW3View3DFaceMesh
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <qmath.h>
#include <qopenglwidget.h>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QtConcurrent/QtConcurrent>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/face_photo_resource.h"

#include <Util/Core/IOParser_v3.h>
#include <Util/Core/path_global_setting.h>
#include "../../Core/Surfacing/MeshPicker.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../UIPrimitive/W3TextItem_switch.h"
#include "../UIPrimitive/view_border_item.h"

using namespace std;
using namespace UIGLObjects;

CW3View3DFaceMesh::CW3View3DFaceMesh(CW3VREngine* VREngine,
	CW3MPREngine* MPREngine, CW3VTOSTO* VTO,
	common::ViewTypeID eType, QWidget* pParent)
	: CW3View3D_thyoo(VREngine, MPREngine, eType, false, pParent),
	m_pgVTOSTO(VTO) {
	m_pShadeSwitch->setVisible(false);

	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());
	setVisible(false);
}

CW3View3DFaceMesh::~CW3View3DFaceMesh(void) {
	if (m_pGLWidget && m_pGLWidget->context()) clearGL();

	SAFE_DELETE_OBJECT(m_pEllipse);
	SAFE_DELETE_OBJECT(m_pFace);
}

void CW3View3DFaceMesh::reset() {
	CW3View3D_thyoo::reset();

	clearPoints();
	clearFace();

	if (m_pGLWidget && m_pGLWidget->context()) clearGL();

	SAFE_DELETE_OBJECT(m_pEllipse);
	SAFE_DELETE_OBJECT(m_pFace);
}

void CW3View3DFaceMesh::clearFace() {
	if (m_pFace && m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		m_pFace->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	}
}

void CW3View3DFaceMesh::clearPoints() {
	if (m_pEllipse) m_pEllipse->clear();
	m_triangleIdxs.clear();
	this->scene()->update();
}

void CW3View3DFaceMesh::generateFaceMesh(double isoValue) {
	CW3ProgressDialog* progress =
		CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	auto future = QtConcurrent::run(
		this, &CW3View3DFaceMesh::generateFaceMeshRunThread, isoValue);
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	m_pGLWidget->makeCurrent();
	const FacePhotoResource& pResFace =
		ResourceContainer::GetInstance()->GetFacePhotoResource();
	m_pFace->initSurfaceFillColor(pResFace.points(), pResFace.indices());
	m_pGLWidget->doneCurrent();

	this->scene()->update();
}

void CW3View3DFaceMesh::generateFaceMeshRunThread(double isoValue) {
	if (!m_pgVTOSTO->flag.generateHead || m_pgVTOSTO->m_isoValue != isoValue) {
		m_pgVTOSTO->setIsoValue(isoValue);
		m_pgVTOSTO->generateHead();
	}

	m_pgVTOSTO->cutFace(glm::vec4(0, -1, 0, -0.2));
}
void CW3View3DFaceMesh::initializeGL() {
	m_PROGfrontfaceCUBE = m_pgVREngine->getThyooPROGfrontface();
	m_PROGfrontfaceFinal = m_pgVREngine->getThyooPROGforntfaceFinal();
	m_PROGbackfaceCUBE = m_pgVREngine->getThyooPROGbackface();
	m_PROGraycasting = m_pgVREngine->getThyooPROGRayCasting();
	m_PROGfinal = m_pgVREngine->getPROGfinal();
	m_PROGsurface = m_pgVREngine->getPROGsurface();
	m_PROGpick = m_pgVREngine->getPROGpickWithCoord();

	m_vVolRange = glm::vec3(m_pgVREngine->getVol(0)->width(),
		m_pgVREngine->getVol(0)->height(),
		m_pgVREngine->getVol(0)->depth());

	m_model = glm::scale(m_vVolRange);

	m_scale = 1.5f;
	m_camFOV = glm::length(m_vVolRange);

	setViewMatrix();
	setProjection();

	m_pEllipse = new CW3SurfaceTextEllipseItem(this->scene());
	m_pEllipse->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
	m_pEllipse->editTransformMat(glm::scale(m_vVolRange), SCALE);

	m_pFace = new CW3SurfaceItem();
	m_pFace->editTransformMat(glm::scale(m_vVolRange), SCALE);

	Material material;
	material.Ka = vec3(0.3f);
	material.Ks = vec3(0.3f);
	material.Kd = vec3(0.3f);
	material.Shininess = 3.0f;
	m_pFace->setColor(material);
}

void CW3View3DFaceMesh::renderingGL(void) {
	if (!this->isVisible()) return;

	if (!m_pgVREngine->isVRready()) {
		CW3GLFunctions::clearView(true);
		return;
	}

	if (!m_is3Dready) {
		initializeGL();
		m_is3Dready = true;
	}

	if (!m_vaoPlane) m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);

#if kRenderActiveCube
	if (!m_vaoCUBE) m_pgVREngine->setActiveIndex(&m_vaoCUBE, 0);
#else
	if (!m_vaoCUBE) {
		unsigned int *vbo = m_pgVREngine->getVolVBO();
		CW3GLFunctions::initVAO(&m_vaoCUBE, vbo);
	}
#endif
	const FacePhotoResource& pResFace =
		ResourceContainer::GetInstance()->GetFacePhotoResource();

	if (m_pgVTOSTO->flag.cutFace && !m_pFace->isReadyVAO() &&
		pResFace.indices().size()) {
		m_pFace->initSurfaceFillColor(pResFace.points(), pResFace.indices());
	}

	glViewport(0, 0, this->width(), this->height());
	this->setMVP(m_rotAngle, m_rotAxis);

	CW3GLFunctions::clearView(true, GL_BACK);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
		m_pGLWidget->defaultFramebufferObject());
	if (m_pFace) {
		glUseProgram(m_PROGsurface);
		m_pFace->draw(m_PROGsurface, GL_BACK);
	}

	glUseProgram(m_PROGsurface);
	m_pEllipse->draw(m_PROGsurface);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);
	m_isChanging = false;
	m_rotAngle = 0.0f;
}

void CW3View3DFaceMesh::resizeScene() {
	CW3View3D_thyoo::resizeScene();
	if (m_pEllipse)
		m_pEllipse->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
}

void CW3View3DFaceMesh::drawBackground(QPainter* painter, const QRectF& rect) {
	QGraphicsView::drawBackground(painter, rect);
	painter->beginNativePainting();
	this->renderingGL();
	painter->endNativePainting();
}

void CW3View3DFaceMesh::resizeEvent(QResizeEvent* event) {
	CW3View3D_thyoo::resizeEvent(event);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(
		QRectF(begin_idx.x(), begin_idx.y(), scene_size.x(), scene_size.y()));
}

void CW3View3DFaceMesh::setMVP(float rotAngle, glm::vec3 rotAxis) {
	CW3View3D_thyoo::setMVP(rotAngle, rotAxis);

	m_pEllipse->editTransformMat(glm::rotate(glm::radians(m_rotAngle), m_rotAxis),
		ARCBALL);
	m_pFace->editTransformMat(glm::rotate(glm::radians(m_rotAngle), m_rotAxis),
		ARCBALL);
	m_pEllipse->setProjViewMat(m_projection, m_view);
	m_pFace->setProjViewMat(m_projection, m_view);
}

void CW3View3DFaceMesh::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		if (m_pFace) m_pFace->clearVAOVBO();
		if (m_pEllipse) m_pEllipse->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	}

	CW3View3D_thyoo::clearGL();
}

void CW3View3DFaceMesh::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::mousePressEvent(event);
}

void CW3View3DFaceMesh::mouseReleaseEvent(QMouseEvent* event)
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

	CW3View3D_thyoo::mouseReleaseEvent(event);

	if (!m_pFace) return;

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM || common_tool_type_ == common::CommonToolTypeOnOff::V_PAN)
		return;

	float ptX = (static_cast<float>(event->pos().x()) / this->width()) * 2.0f - 1.0f;
	float ptY = (static_cast<float>(this->height() - event->pos().y()) / this->height()) * 2.0f - 1.0f;
	if (event->button() == Qt::LeftButton && !(event->modifiers() & Qt::ControlModifier))
	{
		try
		{
			if (m_pEllipse->getPickIndex() >= 0) return;

			const FacePhotoResource& pResFace =
				ResourceContainer::GetInstance()->GetFacePhotoResource();
			auto indices = pResFace.indices();

			std::vector<std::vector<int>> faceTriangles;
			std::vector<int> tri(3);
			for (int i = 0; i < indices.size() / 3; i++) {
				tri[0] = indices[3 * i + 0];
				tri[1] = indices[3 * i + 1];
				tri[2] = indices[3 * i + 2];
				faceTriangles.push_back(tri);
			}

			int triID;
			vec3 selectPoint;
			bool isPick =
				MeshPicker::pick(selectPoint, triID, pResFace.points(), faceTriangles,
					vec2(ptX, ptY), m_pFace->getMVP());

			if (!isPick) throw std::runtime_error("failed to pick.");

			std::map<QString, vec3> points = m_pEllipse->getPositions();

			int i = 1;
			while (1) {
				if (points.find(QString("%1").arg(i)) == points.end()) {
					m_pEllipse->addItem(QString("%1").arg(i), selectPoint);
					m_triangleIdxs[QString("%1").arg(i)] = triID;
					break;
				}
				else
					++i;

				if (i > 1000) {
					throw std::runtime_error("Infinite loop.");
				}
			}

			cout << "CW3View3DFaceMesh::get Point = " << selectPoint.x << ", "
				<< selectPoint.y << ", " << selectPoint.z << endl;
		}
		catch (std::runtime_error& e) {
			cout << "CW3View3DFaceMesh::mouseReleaseEvent: " << e.what() << endl;
		}
	}
	else if (event->button() == Qt::RightButton) {
		emit sigSetFaceMapping();
	}
}

void CW3View3DFaceMesh::mouseMoveEvent(QMouseEvent* event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::mouseMoveEvent(event);

	if (!m_pFace) return;

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN)
		return;

	if (event->buttons() & Qt::LeftButton) {
		int nPickIdx = m_pEllipse->getPickIndex();
		if (nPickIdx >= 0) {
			float ptX =
				(static_cast<float>(event->pos().x()) / this->width()) * 2.0f - 1.0f;
			float ptY = (static_cast<float>(this->height() - event->pos().y()) /
				this->height()) *
				2.0f -
				1.0f;

			const FacePhotoResource& pResFace =
				ResourceContainer::GetInstance()->GetFacePhotoResource();

			auto indices = pResFace.indices();

			std::vector<std::vector<int>> faceTriangles;
			std::vector<int> tri(3);
			for (int i = 0; i < indices.size() / 3; i++) {
				tri[0] = indices[3 * i + 0];
				tri[1] = indices[3 * i + 1];
				tri[2] = indices[3 * i + 2];
				faceTriangles.push_back(tri);
			}

			vec3 selectPoint;
			int triID;
			bool isPick =
				MeshPicker::pick(selectPoint, triID, pResFace.points(), faceTriangles,
					vec2(ptX, ptY), m_pFace->getMVP());

			if (isPick) {
				QString strSelected =
					m_pEllipse->getTextList().at(nPickIdx)->toPlainText();

				m_pEllipse->editItem(strSelected, selectPoint);
				m_triangleIdxs[strSelected] = triID;
				this->scene()->update();
			}
		}

		last_view_pos_ = event->pos();
	}
	else if (!(event->buttons() & Qt::RightButton)) {
		m_pGLWidget->makeCurrent();
		{
			QOpenGLFramebufferObject fbo(this->width(), this->height(),
				QOpenGLFramebufferObject::Depth);
			fbo.bind();
			CW3GLFunctions::clearView(true, GL_BACK);
			glViewport(0, 0, this->width(), this->height());
			bool update;
			glUseProgram(m_PROGpick);

			// object id(uchar)를 framebuffer에 쓴 다음 freame buffer의 해당 위치의
			// 픽셀값을 읽어와서 pick되었는지 조사한다.
			m_pEllipse->pick(event->pos(), &update, m_PROGpick);

			glClearColor(1.0, 1.0, 1.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			fbo.release();
			glUseProgram(0);
			if (update) this->scene()->update();
		}
		m_pGLWidget->doneCurrent();
	}
}

void CW3View3DFaceMesh::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::keyPressEvent(event);
}

void CW3View3DFaceMesh::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::keyReleaseEvent(event);
}
