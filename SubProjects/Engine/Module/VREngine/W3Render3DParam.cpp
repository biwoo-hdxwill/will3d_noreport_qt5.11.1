#include "W3Render3DParam.h"
/*=========================================================================

File:			class CW3Render3DParam
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-20
Last date:		2016-05-24

=========================================================================*/
#include <qopenglwidget.h>

#include "../../Common/Common/W3Memory.h"
#include "../../UIModule/UIGLObjects/W3SurfacePlaneItem.h"
#include "../../UIModule/UIGLObjects/W3GLObject.h"
#include "../../UIModule/UIGLObjects/W3GLNerve.h"

#include "W3VolumeRenderParam.h"
CW3Render3DParam::CW3Render3DParam(QOpenGLWidget *curGL)
	: m_pgCurGLWidget(curGL) {
	for (int i = 0; i < MAX_IMPLANT; i++)
		m_pImplant[i] = nullptr;

	m_plane = new CW3GLObject(GL_TRIANGLES);
	m_photo3D = new CW3GLObject();
}

/*
	smseo : 기존에 view2D에서 생성할 때 안쓰는 GL object 들을 모두 생성하는 코드가 들어있어서
	view 타입에 따라 GLObject를 다르게 생성하게 하기 위한 코드
*/
CW3Render3DParam::CW3Render3DParam(const common::ViewTypeID & view_type, QOpenGLWidget * curGL,
								   bool * collision_list, bool * implant_there) :
	m_pgCurGLWidget(curGL) {

	for (int i = 0; i < MAX_IMPLANT; i++)
		m_pImplant[i] = nullptr;

	switch (view_type) {
	case common::ViewTypeID::ENDO:
	case common::ViewTypeID::ENDO_MODIFY:
	case common::ViewTypeID::ENDO_SAGITTAL:
	case common::ViewTypeID::ENDO_SLICE:
		m_pAirway = new CW3GLObject();
		break;
	case common::ViewTypeID::IMPLANT_PREVIEW:
		m_pImplant[NW3Render3DParam::kImplantIDPreview] = new CW3GLObject();
		break;
	default:
		break;
	}

	// 얘네는 어디 들어갈 지 몰라서 그냥 놔뒀음
	m_plane = new CW3GLObject(GL_TRIANGLES);
	m_photo3D = new CW3GLObject();

	g_is_implant_collided_ = collision_list;
	g_is_implant_exist_ = implant_there;
}

CW3Render3DParam::CW3Render3DParam(QOpenGLWidget * curGL,
								   bool * collision_list, bool * implant_there) :
	m_pgCurGLWidget(curGL) {

	m_plane = new CW3GLObject(GL_TRIANGLES);
	m_pNerve = new CW3GLNerve();
	m_pAirway = new CW3GLObject();
	for (int i = 0; i < MAX_IMPLANT; i++)
		m_pImplant[i] = new CW3GLObject();

	m_photo3D = new CW3GLObject();
	g_is_implant_collided_ = collision_list;
	g_is_implant_exist_ = implant_there;
}

const float CW3Render3DParam::windowing_min() const noexcept {
	return m_VRtextures.windowing_min();
}
const float CW3Render3DParam::windowing_norm() const noexcept {
	return m_VRtextures.windowing_norm();
}
void CW3Render3DParam::set_windowing_min(float min) noexcept {
	m_VRtextures.set_windowing_min(min);
}
void CW3Render3DParam::set_windowing_norm(float norm) noexcept {
	m_VRtextures.set_windowing_norm(norm);
}

CW3Render3DParam::~CW3Render3DParam() {
	ClearGLObjects();

	if (m_pgCurGLWidget && m_pgCurGLWidget->context()) {
		m_pgCurGLWidget->makeCurrent();

		if (m_mainVolume_vao[0]) {
			glDeleteVertexArrays(1, &m_mainVolume_vao[0]);
			m_mainVolume_vao[0] = 0;
		}

		if (m_mainVolume_vao[1]) {
			glDeleteVertexArrays(1, &m_mainVolume_vao[1]);
			m_mainVolume_vao[1] = 0;
		}

		for (int i = 0; i < MAX_IMPLANT; i++) {
			SAFE_DELETE_OBJECT(m_pImplant[i]);
		}

		SAFE_DELETE_OBJECT(m_photo3D);
		SAFE_DELETE_OBJECT(m_plane);
		SAFE_DELETE_OBJECT(m_pNerve);
		SAFE_DELETE_OBJECT(m_pAirway);
		SAFE_DELETE_OBJECT(m_pMPROverlay);

		m_pgCurGLWidget->doneCurrent();
	}
}

void CW3Render3DParam::clearFBOs() {
	if (m_pgCurGLWidget && m_pgCurGLWidget->context()) {
		m_pgCurGLWidget->makeCurrent();

		if (m_fbo) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &m_fbo);
			m_fbo = 0;
		}

		if (m_depthMap) {
			glDeleteRenderbuffers(1, &m_depthMap);
			m_depthMap = 0;
		}

		if (m_texHandler[0]) {
			glDeleteTextures(NW3Render3DParam::kNumTexHandle, m_texHandler);
			for (unsigned int i = 0; i < NW3Render3DParam::kNumTexHandle; i++) {
				m_texHandler[i] = 0;
			}
		}
		m_pgCurGLWidget->doneCurrent();
	}

	if (m_isDerivedVolume) {
		if (m_pgCurGLWidget && m_pgCurGLWidget->context()) {
			if (m_pgMainVolume[0]) {
				m_pgCurGLWidget->makeCurrent();
				if (m_pgMainVolume[0]->m_texHandlerVol) {
					glDeleteTextures(1, &m_pgMainVolume[0]->m_texHandlerVol);
					m_pgMainVolume[0]->m_texHandlerVol = 0;
				}
				m_pgCurGLWidget->doneCurrent();
			}
		}
	}
}

void CW3Render3DParam::clearVAOs() {
	if (m_pgCurGLWidget && m_pgCurGLWidget->context()) {
		m_pgCurGLWidget->makeCurrent();

		if (m_mainVolume_vao[0]) {
			glDeleteVertexArrays(1, &m_mainVolume_vao[0]);
			m_mainVolume_vao[0] = 0;
		}

		if (m_mainVolume_vao[1]) {
			glDeleteVertexArrays(1, &m_mainVolume_vao[1]);
			m_mainVolume_vao[1] = 0;
		}

		for (int i = 0; i < MAX_IMPLANT; i++) {
			if (m_pImplant[i])
				m_pImplant[i]->clearVAOVBO();
		}

		if (m_photo3D)
			m_photo3D->clearVAOVBO();
		if (m_plane)
			m_plane->clearVAOVBO();
		if (m_pNerve)
			m_pNerve->clearVAOVBO();
		if (m_pAirway)
			m_pAirway->clearVAOVBO();
		if (m_pMPROverlay)
			m_pMPROverlay->clearVAOVBO();

		m_pgCurGLWidget->doneCurrent();
	}
}

void CW3Render3DParam::SetCurGLWidget(QOpenGLWidget* curGL) {
	m_pgCurGLWidget = curGL;
}

void CW3Render3DParam::ClearGLObjects() {
	clearVAOs();
	clearFBOs();
	if (m_pgCurGLWidget && m_pgCurGLWidget->context()) {
		m_pgCurGLWidget->makeCurrent();
		if (m_VRtextures.m_texHandler[0]) {
			glDeleteTextures(3, m_VRtextures.m_texHandler);
			m_VRtextures.m_texHandler[0] = 0;
			m_VRtextures.m_texHandler[1] = 0;
			m_VRtextures.m_texHandler[2] = 0;
		}
		m_pgCurGLWidget->doneCurrent();
	}
}

GLuint CW3Render3DParam::defaultFBO() {
	return m_pgCurGLWidget->defaultFramebufferObject();
}

void CW3Render3DParam::makeCurrent() {
	m_pgCurGLWidget->makeCurrent();
}
void CW3Render3DParam::doneCurrent() {
	m_pgCurGLWidget->doneCurrent();
}
