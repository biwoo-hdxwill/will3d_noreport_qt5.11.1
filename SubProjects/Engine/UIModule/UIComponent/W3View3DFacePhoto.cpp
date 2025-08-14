#include "W3View3DFacePhoto.h"
/*=========================================================================

File:			class CW3View3DFacePhoto
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <qmath.h>
#include <QOpenGLFramebufferObject>
#include <qgl.h>
#include <qopenglwidget.h>
#include <QMouseEvent>
#include <QKeyEvent>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"

#include "../UIGLObjects/W3SurfaceTextEllipseItem.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/view_border_item.h"
#include "../../Module/VREngine/W3VREngine.h"

CW3View3DFacePhoto::CW3View3DFacePhoto(CW3VREngine *VREngine, CW3MPREngine *MPREngine,
									   CW3VTOSTO * VTO, common::ViewTypeID eType, QWidget *pParent)
	: CW3View2D_thyoo(VREngine, MPREngine, eType, true, pParent), m_pgVTOSTO(VTO) {
	m_PROGsurface = m_pgVREngine->getPROGsurface();
	m_PROGsurfaceTexture = m_pgVREngine->getPROGsurfaceTexture();
	m_PROGpick = m_pgVREngine->getPROGpickWithCoord();

	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());

	connect(m_pgVTOSTO, SIGNAL(sigLoadPhoto(QString)), this, SLOT(slotLoadPhoto(QString)));
	setVisible(false);
}

CW3View3DFacePhoto::~CW3View3DFacePhoto(void) {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();
}

void CW3View3DFacePhoto::reset() {
	CW3View2D_thyoo::reset();

	m_is3Dready = false;

	clearPoints();

	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();
}

void CW3View3DFacePhoto::clearPhoto() {
	if (m_texHandlerPlane) {
		glDeleteTextures(1, &m_texHandlerPlane);
		m_texHandlerPlane = 0;
	}

	m_isSetPhoto = false;
}

void CW3View3DFacePhoto::clearPoints() {
	if (m_pEllipse)
		m_pEllipse->clear();
	m_texCtrlPoints.clear();

	if (isVisible())
		this->scene()->update();
}

void CW3View3DFacePhoto::slotLoadPhoto(const QString& FilePath) {
	m_pPhotoImage = QGLWidget::convertToGLFormat(QImage(FilePath));

	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		{
			clearPhoto();

			glActiveTexture(m_texNumPlane);
			glGenTextures(1, &m_texHandlerPlane);

			int width = m_pPhotoImage.width();
			int height = m_pPhotoImage.height();

			glBindTexture(GL_TEXTURE_2D, m_texHandlerPlane);
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width,
							 height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
							 m_pPhotoImage.bits());
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			}

			m_Wglpre = width;
			m_Hglpre = height;

			m_model = glm::scale(vec3(m_Wglpre, m_Hglpre, 1.0f));

			m_pPhoto->setTransformMat(m_model, UIGLObjects::SCALE);
			m_pEllipse->setTransformMat(m_model, UIGLObjects::SCALE);

			m_isSetPhoto = true;
		}
		m_pGLWidget->doneCurrent();
	}

	this->scene()->update();
}

void CW3View3DFacePhoto::initializeGL() {
	m_pEllipse = new CW3SurfaceTextEllipseItem(this->scene());
	m_pEllipse->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
	m_pPhoto = new CW3SurfaceItem();

	initPhotoGL();

	glUseProgram(m_PROGsurface);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Intensity", vec3(1.0f));
	vec4 lightPos = vec4(0.0f, -10.0f, 0.0f, 1.0f);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Position", m_view * lightPos);
}

void CW3View3DFacePhoto::initPhotoGL() {
	std::vector<vec3> vertPlane{
		vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f)
	};

	std::vector<vec2> texPlane{
		vec2(0.0f, 0.0f), vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f), vec2(0.0f, 1.0f)
	};

	std::vector<unsigned int> indices{
		0, 1, 3, 1, 2, 3
	};

	m_pPhoto->initSurfaceFillTexture(vertPlane, texPlane, indices);
}

void CW3View3DFacePhoto::renderingGL(void) {
	if (!this->isVisible())
		return;

	if (!m_pgVREngine->isVRready()) {
		CW3GLFunctions::clearView(true);
		return;
	}

	if (!m_is3Dready) {
		initializeGL();
		m_is3Dready = true;
	}

	if (!m_pPhoto->isReadyVAO()) {
		initPhotoGL();
	}

	if (m_isSetPhoto && !m_texHandlerPlane) {
		glActiveTexture(m_texNumPlane);
		glGenTextures(1, &m_texHandlerPlane);

		glBindTexture(GL_TEXTURE_2D, m_texHandlerPlane);
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_pPhotoImage.width(), m_pPhotoImage.height(),
						 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pPhotoImage.bits());
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
	}

	glViewport(0, 0, this->width(), this->height());
	setProjection();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pGLWidget->defaultFramebufferObject());
	{
		glUseProgram(m_PROGsurface);
		CW3GLFunctions::clearView(true);

		m_pEllipse->setProjViewMat(m_projection, m_view);
		m_pEllipse->draw(m_PROGsurface);

		glUseProgram(m_PROGsurfaceTexture);
		glBindTexture(GL_TEXTURE_2D, m_texHandlerPlane);
		WGLSLprogram::setUniform(m_PROGsurfaceTexture, "tex1", m_texNumPlane_);
		m_pPhoto->setProjViewMat(m_projection, m_view);
		m_pPhoto->draw(m_PROGsurfaceTexture);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);
	}
}

void CW3View3DFacePhoto::ResetView() {
	m_scale = 1.0f;
	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;

	setProjection();

	scene()->update();
}

void CW3View3DFacePhoto::FitView() {
	ResetView();
}

void CW3View3DFacePhoto::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	this->renderingGL();
}

void CW3View3DFacePhoto::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		if (m_pEllipse)
			m_pEllipse->clearVAOVBO();
		if (m_pPhoto)
			m_pPhoto->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	}

	CW3View2D_thyoo::clearGL();
}

void CW3View3DFacePhoto::resizeScene() {
	CW3View2D_thyoo::resizeScene();
	if (m_pEllipse)
		m_pEllipse->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
}

void CW3View3DFacePhoto::resizeEvent(QResizeEvent * event) {
	CW3View2D_thyoo::resizeEvent(event);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
							scene_size.x(), scene_size.y()));
}

void CW3View3DFacePhoto::mouseReleaseEvent(QMouseEvent *event) 
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

	CW3View2D_thyoo::mouseReleaseEvent(event);

	float ptX = (static_cast<float>(event->pos().x()) / this->width())*2.0f - 1.0f;
	float ptY = (static_cast<float>(this->height() - event->pos().y()) / this->height())*2.0f - 1.0f;

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN)
		return;

	if (event->button() == Qt::LeftButton &&
		!(event->modifiers() & Qt::ControlModifier)) {
		try {
			if (m_pEllipse == nullptr)
				throw std::exception("Ellipse is nullptr");
			if (m_pEllipse->getPickIndex() >= 0 || !m_isSetPhoto) {
				return;
			}

			glm::vec4 ptGL = glm::inverse(m_projection*m_view*m_model)*vec4(ptX, ptY, 0.0f, 1.0f);

			std::map<QString, vec3> points = m_pEllipse->getPositions();

			int i = 1;
			while (1) {
				if (points.find(QString("%1").arg(i)) == points.end()) {
					m_pEllipse->addItem(QString("%1").arg(i), vec3(ptGL));

					ptGL = ptGL * 0.5f + vec4(0.5f);
					m_texCtrlPoints[QString("%1").arg(i)] = vec2(ptGL.x, ptGL.y);
					break;
				} else
					++i;

				if (i > 1000) {
					throw std::exception("Infinite loop.");
				}
			}
			this->scene()->update();
		} catch (const std::exception& e) {
			common::Logger::instance()->Print(common::LogType::ERR,
											  "CW3View3DFacePhoto::mouseReleaseEvent: " + std::string(e.what()));
		}
	} else if (event->button() == Qt::RightButton) {
		emit sigSetFaceMapping();
	}
}

void CW3View3DFacePhoto::mousePressEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D_thyoo::mousePressEvent(event);

	last_view_pos_ = event->pos();
}

void CW3View3DFacePhoto::mouseMoveEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D_thyoo::mouseMoveEvent(event);

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN)
		return;

	try {
		if (event->buttons() & Qt::LeftButton) {
			if (m_pEllipse == nullptr)
				throw std::exception("Ellipse is nullptr");

			int nPickIdx = m_pEllipse->getPickIndex();
			if (nPickIdx >= 0) {
				QPointF transPos = event->pos() - last_view_pos_;
				vec4 glTrans = vec4(transPos.x() / this->width()*2.0f, -transPos.y() / this->height()*2.0f, 0.0f, 1.0f);

				glm::vec3 pointPos = m_pEllipse->getPointPosition(nPickIdx);

				m_mvp = m_projection * m_view*m_model;
				mat4 iMVP = glm::inverse(m_mvp);
				vec3 ptGL = pointPos + vec3(iMVP*glTrans);
				m_pEllipse->editPoint(nPickIdx, ptGL);

				QString strSelected = m_pEllipse->getTextList().at(nPickIdx)->toPlainText();

				ptGL = ptGL * 0.5f + vec3(0.5f);
				m_texCtrlPoints[strSelected] = vec2(ptGL.x, ptGL.y);
				this->scene()->update();
			}

			last_view_pos_ = event->pos();
		} else if (!(event->buttons() & Qt::RightButton)) {
			if (m_pEllipse == nullptr)
				throw std::exception("Ellipse is nullptr");

			m_pGLWidget->makeCurrent();
			{
				glUseProgram(m_PROGpick);
				QOpenGLFramebufferObject fbo(this->width(), this->height(), QOpenGLFramebufferObject::Depth);
				fbo.bind();
				CW3GLFunctions::clearView(true, GL_BACK);
				glViewport(0, 0, this->width(), this->height());
				bool update;
				m_pEllipse->pick(event->pos(), &update, m_PROGpick);

				fbo.release();
				glUseProgram(0);

				if (update)
					this->scene()->update();
			}
			m_pGLWidget->doneCurrent();
		}
	} catch (const std::exception& e) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3View3DFacePhoto::mouseMoveEvent: " + std::string(e.what()));
	}
}

void CW3View3DFacePhoto::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D_thyoo::keyPressEvent(event);
}

void CW3View3DFacePhoto::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View2D_thyoo::keyReleaseEvent(event);
}
