#include "W3View3Dslice.h"
/*=========================================================================

File:			class CW3View3Dslice
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-03
Last modify:	2016-04-21

=========================================================================*/
#include <qgraphicsproxywidget.h>
#include <qopenglwidget.h>
#include <QApplication>

#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Resource/Resource/W3Image3D.h"

#include "../UIPrimitive/measure_tools.h"
#include "../UIPrimitive/W3TextItem.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

namespace {
// TODO smseo : 볼륨 좌표를 넣기 전 임시 값
const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

CW3View3Dslice::CW3View3Dslice(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
							   CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
							   QWidget *pParent)
	: CW3View3D(VREngine, MPRengine, Rcontainer, eType, pParent) {
	recon_type_ = common::ReconTypeID::MPR;

	if (direction_text_)
		direction_text_->setVisible(false);

	m_bIsXray = false;

	m_origBackVector = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	setInitScale();
}

CW3View3Dslice::~CW3View3Dslice(void) {}

void CW3View3Dslice::reset() {
	CW3View3D::reset();

	m_is3Dready = false;
	m_bDoubleClickPressed = false;

	m_curSlice = 0.0f;
	m_curSliceInVolume = 0.0f;

	m_Ztranslate = glm::mat4(1.0f);
	m_invModel = glm::mat4(1.0f);
	m_invModel0 = glm::mat4(1.0f);
	m_modelSlice = glm::mat4(1.0f);

	m_bIsXray = false;
}

void CW3View3Dslice::setMVPslice() {
	m_mvp = m_projection * m_view*m_modelSlice;
}

void CW3View3Dslice::renderGL() {
	switch (recon_type_) {
	case common::ReconTypeID::MIP:
	case common::ReconTypeID::VR:
		render3D();
		break;
	default:
		break;
	}
}

void CW3View3Dslice::init() {
	CW3View3D::init();

	m_modelSlice = glm::scale(glm::vec3(m_camFOV));

	m_volvertTotexCoord = glm::translate(glm::vec3(0.5f))*glm::scale(glm::vec3(-0.5f, 0.5f, 0.5f));

	m_is3Dready = true;

	setWLWW();

	int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;
	unsigned int PROGslice = m_pgVREngine->getPROGslice();
	glUseProgram(PROGslice);
	WGLSLprogram::setUniform(PROGslice, "WindowLevel",
		(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
	WGLSLprogram::setUniform(PROGslice, "WindowWidth",
		(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
	glUseProgram(0);

	unsigned int PROGsliceCanal = m_pgVREngine->getPROGsliceCanal();
	glUseProgram(PROGsliceCanal);
	WGLSLprogram::setUniform(PROGsliceCanal, "WindowLevel",
		(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
	WGLSLprogram::setUniform(PROGsliceCanal, "WindowWidth",
		(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
	glUseProgram(0);

	if (recon_type_ == common::ReconTypeID::MPR ||
		recon_type_ == common::ReconTypeID::X_RAY)
		if (m_pProxySlider)
			m_pProxySlider->setVisible(!hide_all_view_ui_);

	setProjection();
}

void CW3View3Dslice::resizeEvent(QResizeEvent *pEvent) {
	if (!isVisible())
		return;

	CW3View3D::resizeEvent(pEvent);
}

void CW3View3Dslice::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}
void CW3View3Dslice::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
}
void CW3View3Dslice::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}
void CW3View3Dslice::mouseDoubleClickEvent(QMouseEvent *event) {
	QGraphicsView::mouseDoubleClickEvent(event);
}

void CW3View3Dslice::equidistanceSpline(std::vector<glm::vec3> &out, std::vector<glm::vec3> &normal,
										std::vector<glm::vec3> &in, glm::vec3 &TopTranslate,
										glm::vec3 &upVector) {
	out.clear();
	normal.clear();
	out.push_back(in.front() + TopTranslate);

	glm::vec3 addTail = in.back() * 2.0f - in.at(in.size() - 2);
	in.push_back(addTail);

	for (int i = 1; i < in.size() - 1; i++) {
		glm::vec3 p1 = out.back();
		glm::vec3 p2 = in.at(i) + TopTranslate;
		glm::vec3 vec = p2 - p1;

		int len = static_cast<int>(glm::length(vec));

		vec = glm::normalize(vec);

		glm::vec3 norm = -glm::cross(upVector, vec);

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

bool CW3View3Dslice::mouseMoveEventW3(QMouseEvent *event) {
	QPointF scene_pos = mapToScene(event->pos());
	
	if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_)) {
		if (measure_tools_->IsDrawing()) {
			measure_tools_->ProcessMouseMove(scene_pos, kTempPos);
			return false;
		}
	}
	
	Qt::MouseButtons buttons = event->buttons();
	if (buttons == Qt::NoButton) {
		// empty if statement
	} else {
		if (measure_tools_->IsSelected() &&
			measure_tools_->IsMeasureInteractionAvailable(common_tool_type_)) {
			measure_tools_->ProcessMouseMove(scene_pos, kTempPos);
			return false;
		} else if (common_tool_type_ == common::CommonToolTypeOnOff::V_LIGHT) {
			if (recon_type_ == common::ReconTypeID::MPR) {
				m_nAdjustWindowLevel += (last_scene_pos_.x() - scene_pos.x()) * 2;
				m_nAdjustWindowWidth += (last_scene_pos_.y() - scene_pos.y()) * 2;

				int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;

				m_pGLWidget->makeCurrent();

				unsigned int PROGslice = m_pgVREngine->getPROGslice();
				glUseProgram(PROGslice);
				WGLSLprogram::setUniform(PROGslice, "WindowLevel",
					(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
				WGLSLprogram::setUniform(PROGslice, "WindowWidth",
					(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
				glUseProgram(0);

				unsigned int PROGsliceCanal = m_pgVREngine->getPROGsliceCanal();
				glUseProgram(PROGsliceCanal);
				WGLSLprogram::setUniform(PROGsliceCanal, "WindowLevel",
					(float)m_nAdjustWindowLevel / (float)m_pgVREngine->getVol(0)->getMax());
				WGLSLprogram::setUniform(PROGsliceCanal, "WindowWidth",
					(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
				glUseProgram(0);

				m_pGLWidget->doneCurrent();

				last_scene_pos_ = scene_pos;
				scene()->update();
				return true;
			}
		} else if (common_tool_type_ == common::CommonToolTypeOnOff::V_PAN ||
				   common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR) {
			m_pRender3DParam->m_isLowRes = true;
			PanningView(scene_pos);
			return true;
		} else if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
				   common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R) {
			m_pRender3DParam->m_isLowRes = true;
			ScaleView(event->pos());
			return true;
		}
	}

	return false;
}

void CW3View3Dslice::InvertView(bool bToggled) {
	m_bInvertWindowWidth = bToggled;

	int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;

	if (isVisible()) {
		m_pGLWidget->makeCurrent();

		unsigned int PROGslice = m_pgVREngine->getPROGslice();
		glUseProgram(PROGslice);
		WGLSLprogram::setUniform(PROGslice, "WindowWidth",
			(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
		glUseProgram(0);

		unsigned int PROGsliceCanal = m_pgVREngine->getPROGsliceCanal();
		glUseProgram(PROGsliceCanal);
		WGLSLprogram::setUniform(PROGsliceCanal, "WindowWidth",
			(float)windowWidth / (float)m_pgVREngine->getVol(0)->getMax());
		glUseProgram(0);

		m_pGLWidget->doneCurrent();

		scene()->update();
	}
}

void CW3View3Dslice::setInitScale() {
	m_scale = m_initScale;
}
