#include "thyoo_W3View3D.h"
/*=========================================================================

File:			class CW3View3D
Language:		C++11
Library:        Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-11
				2016-04-18(Tae Hoon Yoo)

=========================================================================*/
#include <qmath.h>
#include <QApplication>
#include <QOpenGLWidget>
#include <QDebug>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/Resource/W3TF.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../UIPrimitive/W3TextItem_switch.h"
#include "../UIPrimitive/measure_tools.h"

#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3ActiveBlock.h"
#include "../../Module/VREngine/W3Render3DParam.h"

#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>

CW3View3D_thyoo::CW3View3D_thyoo(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					 common::ViewTypeID eType, bool isMPRused, QWidget *pParent)
	: CW3View2D_thyoo(VREngine, MPRengine, eType, isMPRused, pParent) {
	m_pShadeSwitch = new CW3TextItem_switch(QString("Shade"));
	m_pShadeSwitch->setCurrentState(true);
	m_pShadeSwitch->setVisible(false);
	this->scene()->addItem(m_pShadeSwitch);

	if (!m_is2Dused) {
		m_projection = glm::mat4(1.0f);
		m_scale = 1.0f;
	}

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	for (int i = 0; i < 6; i++) {
		m_lpTextAlign[i] = new CW3TextItem();
		m_lpTextAlign[i]->setFont(font);
	}

	m_lpTextAlign[0]->setPlainText("A");
	m_lpTextAlign[1]->setPlainText("P");
	m_lpTextAlign[2]->setPlainText("L");
	m_lpTextAlign[3]->setPlainText("R");
	m_lpTextAlign[4]->setPlainText("I");
	m_lpTextAlign[5]->setPlainText("S");

	if (m_eViewType == common::ViewTypeID::CEPH || m_eViewType == common::ViewTypeID::FACE_AFTER) {
		for (int i = 0; i < 6; i++) {
			if (m_lpTextAlign[i]) {
				m_lpTextAlign[i]->setVisible(true);
				scene()->addItem(m_lpTextAlign[i]);
			}
		}
	}

	connect(m_lpTextAlign[0], SIGNAL(sigPressed()), this, SLOT(slotVRAlignA()));
	connect(m_lpTextAlign[1], SIGNAL(sigPressed()), this, SLOT(slotVRAlignP()));
	connect(m_lpTextAlign[2], SIGNAL(sigPressed()), this, SLOT(slotVRAlignL()));
	connect(m_lpTextAlign[3], SIGNAL(sigPressed()), this, SLOT(slotVRAlignR()));
	connect(m_lpTextAlign[4], SIGNAL(sigPressed()), this, SLOT(slotVRAlignI()));
	connect(m_lpTextAlign[5], SIGNAL(sigPressed()), this, SLOT(slotVRAlignS()));

	ApplyPreferences();
}

CW3View3D_thyoo::~CW3View3D_thyoo() {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(measure_3d_manager_);
}
#ifndef WILL3D_VIEWER
void CW3View3D_thyoo::ExportProjectForMeasure3D(ProjectIOView& out)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void CW3View3D_thyoo::ImportProjectForMeasure3D(ProjectIOView& in)
{
	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(scene());
	}
	measure_3d_manager_->ImportProject(in);
}
#endif
void CW3View3D_thyoo::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
#if 1
	common_tool_type_ = type;

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetType(type);
	}

	measure_tools_->SetMeasureType(common_tool_type_);
#endif
}

void CW3View3D_thyoo::reset() {
	CW3View2D_thyoo::reset();

	m_is3Dready = false;

	m_eReconType = common::ReconTypeID::VR;

	m_isChanging = false;
	m_camFOV = 1.0f;

	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();
	   
	m_texBuffer.clear();
	m_texHandler.clear();
	m_texNum.clear();
	m_texNum_.clear();
	m_depthHandler.clear();

	m_vaoCUBE = 0;
	m_vaoPlane = 0;

	m_FBHandler = 0;

	m_width3Dview = 0;
	m_height3Dview = 0;

	m_rotAngle = 0.0f;
	m_rotAxis = vec3(1.0f);
	m_arcMat = mat4(1.0f);
	m_reorienMat = mat4(1.0f);

	m_clipParams.isEnable = false;

	if (!m_is2Dused) {
		m_projection = glm::mat4(1.0f);
		m_scale = 1.0f;
	}
}

void CW3View3D_thyoo::initClipValues(const MPRClipID& clip_plane, const bool& is_clipping,
									 const int& lower, const int& upper) {
	m_clipParams.isEnable = is_clipping;

	vec3 norm;
	switch (clip_plane) {
	case MPRClipID::AXIAL: norm = mat3(m_reorienMat)*vec3(0.0f, 0.0f, -1.0f); break;
	case MPRClipID::CORONAL: norm = mat3(m_reorienMat)*vec3(0.0f, 1.0f, 0.0f); break;
	case MPRClipID::SAGITTAL: norm = mat3(m_reorienMat)*vec3(1.0f, 0.0f, 0.0f); break;
	}

	float ld = ((static_cast<float>(lower) / 100.0f) - 0.5f)*2.0f;
	float ud = -((static_cast<float>(upper) / 100.0f) - 0.5f)*2.0f;

	m_clipParams.planes.clear();
	m_clipParams.planes.push_back(vec4(-norm, ld));
	m_clipParams.planes.push_back(vec4(norm, ud));
}

void CW3View3D_thyoo::setMIP(bool isMIP) {
	m_isMIP = isMIP;
	this->scene()->update();
}

void CW3View3D_thyoo::slotShadeOnFromOTF(bool isShade) {
	m_pShadeSwitch->setCurrentState(isShade);
	this->scene()->update();
}

void CW3View3D_thyoo::initializeGL() {
	m_PROGfrontfaceCUBE = m_pgVREngine->getThyooPROGfrontface();
	m_PROGfrontfaceFinal = m_pgVREngine->getThyooPROGforntfaceFinal();
	m_PROGbackfaceCUBE = m_pgVREngine->getThyooPROGbackface();
	m_PROGraycasting = m_pgVREngine->getThyooPROGRayCasting();
	m_PROGfinal = m_pgVREngine->getPROGfinal();
	m_PROGsurface = m_pgVREngine->getPROGsurface();
	m_PROGpick = m_pgVREngine->getPROGpickWithCoord();

	CW3VolumeRenderParam* pRenderParam = m_pgVREngine->getVRparams(0);
	m_isChanging = false;

	m_model = glm::mat4(1.0f);

	mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
							glm::vec3(0.0f, 0.0f, 0.0f),
							glm::vec3(0.0f, 1.0f, 0.0f));

	mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
	m_mvpForFinal = glm::mat4(1.0f);
	m_mvpForFinal = projection * view;

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", m_mvpForFinal);

	glUseProgram(m_PROGraycasting);
	WGLSLprogram::setUniform(m_PROGraycasting, "MVP", m_mvpForFinal);

	glUseProgram(m_PROGfinal);
	WGLSLprogram::setUniform(m_PROGfinal, "MVP", m_mvpForFinal);

	m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);

#if kRenderActiveCube
	m_pgVREngine->setActiveIndex(&m_vaoCUBE, 0);
#else

	unsigned int *vbo = m_pgVREngine->getVolVBO();
	CW3GLFunctions::initVAO(&m_vaoCUBE, vbo);
#endif

	m_vVolRange = glm::vec3(pRenderParam->m_pgVol->width(),
							pRenderParam->m_pgVol->height(),
							pRenderParam->m_pgVol->depth());

	m_model = glm::scale(m_vVolRange);

	m_scale = 1.0f;

	m_camFOV = glm::length(m_vVolRange);

	setViewMatrix();
	setProjection();

	m_stepSize = pRenderParam->m_stepSize;

	glUseProgram(m_PROGsurface);

	float camFOV = glm::length(m_vVolRange);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Intensity", vec3(1.0f));
	vec4 lightPos = glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
	WGLSLprogram::setUniform(m_PROGsurface, "Light.Position",
							 glm::lookAt(glm::vec3(0.0f, -camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f))
							 * lightPos);

	glUseProgram(m_PROGfrontfaceCUBE);
	WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "VolTexBias", pRenderParam->m_volTexBias);
	WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "VolTexTransformMat", mat4(1.0));

	glUseProgram(m_PROGbackfaceCUBE);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexBias", pRenderParam->m_volTexBias);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", mat4(1.0));

	for (int i = 0; i < TEX_END; i++) {
		m_texBuffer.push_back(GL_COLOR_ATTACHMENT0 + i);
		m_texHandler.push_back(0);
		m_texNum.push_back(GL_TEXTURE10 + i);
		m_texNum_.push_back(10 + i);
	}

	for (int i = 0; i < DEPTH_END; i++) {
		m_depthHandler.push_back(0);
	}

	if (m_pWorldAxisItem) {
		m_pWorldAxisItem->setVisible(!hide_all_view_ui_);
		//m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
	}

	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(scene());
	}
	measure_3d_manager_->set_pixel_spacing(m_pgVREngine->getVol(0)->pixelSpacing());
	measure_3d_manager_->set_slice_thickness(m_pgVREngine->getVol(0)->sliceSpacing());
	measure_3d_manager_->set_volume_range(m_vVolRange);
	measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	measure_3d_manager_->SetType(common_tool_type_);
}

void CW3View3D_thyoo::slotTFupdated(bool isUpdate) {
	if (!m_is3Dready || !isVisible())
		return;

	m_isChanging = true;

	if (isUpdate && m_pGLWidget && m_pGLWidget->context()) {
		m_pGLWidget->makeCurrent();
		m_pgVREngine->setActiveIndex(&m_vaoCUBE, 0);
		m_pGLWidget->doneCurrent();
	}

	this->scene()->update();
}

void CW3View3D_thyoo::slotRenderCompleted() {
	m_isChanging = false;

	if (isVisible())
		this->scene()->update();
}

void CW3View3D_thyoo::basicRayCasting(void) {
	CW3GLFunctions::printError(__LINE__, "CW3View3D::basicRayCasting");
	////recompile for test
	//m_pgVREngine->recompileRaycasting();
	//m_PROGfrontfaceCUBE = m_pgVREngine->getThyooPROGfrontface();
	//m_PROGfrontfaceFinal = m_pgVREngine->getThyooPROGforntfaceFinal();
	//m_PROGbackfaceCUBE = m_pgVREngine->getThyooPROGbackface();
	//m_PROGraycasting = m_pgVREngine->getThyooPROGRayCasting();
	////recompile for test end

	CW3VolumeRenderParam* pRenderParam = m_pgVREngine->getVRparams(0);

	m_pgVREngine->setVolTextureUniform(m_PROGraycasting, pRenderParam->m_texHandlerVol);
	m_pgVREngine->setTFTextureUniform();

	setMVP(m_rotAngle, m_rotAxis);

	int width = this->width();
	int height = this->height();

	if (m_isChanging) {
		width *= low_res_frame_buffer_resize_factor_;
		height *= low_res_frame_buffer_resize_factor_;

		glUseProgram(m_PROGraycasting);
		WGLSLprogram::setUniform(m_PROGraycasting, "StepSize", m_stepSize * low_res_step_size_factor_);
	} else {
		glUseProgram(m_PROGraycasting);
		WGLSLprogram::setUniform(m_PROGraycasting, "StepSize", m_stepSize);
	}

	if (m_FBHandler == 0 || m_width3Dview != width || m_height3Dview != height) {
		CW3GLFunctions::initFrameBufferMultiTexture(m_FBHandler, m_depthHandler, m_texHandler, width, height, m_texNum);

		m_width3Dview = width;
		m_height3Dview = height;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandler);

	glViewport(0, 0, width, height);

	//////// Front Face
	glDrawBuffer(m_texBuffer[TEX_ENTRY_POSITION]);
	{
		glUseProgram(m_PROGfrontfaceCUBE);

		CW3GLFunctions::clearView(true, GL_BACK);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", m_mvp);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "VolTexTransformMat", glm::mat4(1.0f));

#if kRenderActiveCube
		CW3GLFunctions::drawView(m_vaoCUBE, m_pgVREngine->GetActiveIndices(0), GL_BACK);
#else
		CW3GLFunctions::drawView(m_vaoCUBE, 36, GL_FRONT);
#endif
	}

	////////// Back Face
	glDrawBuffer(m_texBuffer[TEX_EXIT_POSITION]);
	{
		glUseProgram(m_PROGbackfaceCUBE);
		CW3GLFunctions::clearView(true, GL_FRONT);
		WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", m_mvp);
		WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", glm::mat4(1.0f));

#if kRenderActiveCube
		CW3GLFunctions::drawView(m_vaoCUBE, m_pgVREngine->GetActiveIndices(0), GL_FRONT);
#else
		CW3GLFunctions::drawView(m_vaoCUBE, 36, GL_FRONT);
#endif
		this->drawBackFaceSurface();
	}

	//////// Extract Front Face
	glDepthFunc(GL_LESS);
	glClearDepth(1.0f);
	GLenum textures[2] = { m_texBuffer[TEX_ENTRY_POSITION], m_texBuffer[TEX_EXIT_POSITION] };
	glDrawBuffers(2, textures);
	{
		glUseProgram(m_PROGfrontfaceFinal);

		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", m_clipParams.isEnable);
		if (m_clipParams.isEnable) {
			for (int i = 0; i < m_clipParams.planes.size(); i++) {
				WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), m_clipParams.planes[i]);
			}
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)m_clipParams.planes.size());
		} else {
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", 0);
		}

		//CW3GLFunctions::drawView(m_vaoPlane, 6, GL_BACK);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
	}

	////////// Ray Casting
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_RAYCASTING]);
	glDrawBuffer(m_texBuffer[TEX_RAYCASTING]);
	{
		glUseProgram(m_PROGraycasting);
		CW3GLFunctions::clearView(true, GL_BACK);

		unsigned int pass = glGetSubroutineIndex(m_PROGraycasting, GL_FRAGMENT_SHADER, "basicRayCasting");
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass);

		WGLSLprogram::setUniform(m_PROGraycasting, "BMVP", m_mvp*glm::inverse(pRenderParam->m_volTexBias));
		WGLSLprogram::setUniform(m_PROGraycasting, "isShade", m_pShadeSwitch->getCurrentState());
		WGLSLprogram::setUniform(m_PROGraycasting, "VolTexelSize", pRenderParam->m_volTexelSize);
		bool mip = m_pgVREngine->IsMIP();
		bool xray = m_pgVREngine->IsXRAY();
		WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", mip);
		WGLSLprogram::setUniform(m_PROGraycasting, "isXRAY", xray);

		int window_width, window_level;
		auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
		if (mip) {
			window_level = vol.windowCenter();
			window_width = vol.windowWidth();
		} else if (xray) {
#if 1
			window_level = (float)pRenderParam->getTissueBoneThreshold();
			window_width = vol.windowWidth();
#else
			window_level = vol.windowCenter() * 1.5f;
			window_width = vol.windowWidth() * 0.5f;
#endif
		} else {
			window_width = 65535;
			window_level = 32767;
		}

		WGLSLprogram::setUniform(m_PROGraycasting, "WindowLevel", (float)window_level);
		WGLSLprogram::setUniform(m_PROGraycasting, "WindowWidth", (float)window_width);
		WGLSLprogram::setUniform(m_PROGraycasting, "InvVolTexScale", pRenderParam->m_invVolTexScale);

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);
		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		//CW3GLFunctions::drawView(m_vaoPlane, 6, GL_BACK);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_DEFAULT]);
}

void CW3View3D_thyoo::blendingGL() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBHandler);

	glDrawBuffer(m_texBuffer[TEX_FINAL]);
	{
		CW3GLFunctions::clearView(true, GL_BACK);

		glUseProgram(m_PROGfinal);

		glActiveTexture(m_texNum[TEX_RAYCASTING]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_RAYCASTING]);
		WGLSLprogram::setUniform(m_PROGfinal, "FinalImage", m_texNum_[TEX_RAYCASTING]);
		WGLSLprogram::setUniform(m_PROGfinal, "MVP", m_mvpForFinal);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		//CW3GLFunctions::drawView(m_vaoPlane, 6, GL_BACK);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);

		this->drawTransparencySurface();

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}
}

void CW3View3D_thyoo::drawFinal() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pGLWidget->defaultFramebufferObject());
#if defined(__APPLE__)
	glViewport(0, 0, this->width() * 2, this->height() * 2);
#else
	glViewport(0, 0, this->width(), this->height());
#endif
	CW3GLFunctions::clearView(true, GL_BACK);

	this->drawSurface();
	glGetError();

	//CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/TEX_FINAL.png", m_texHandler[TEX_FINAL], GL_RGBA, GL_UNSIGNED_BYTE);

	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(m_PROGfinal);
	glActiveTexture(m_texNum[TEX_FINAL]);
	glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_FINAL]);
	WGLSLprogram::setUniform(m_PROGfinal, "FinalImage", m_texNum_[TEX_FINAL]);
	WGLSLprogram::setUniform(m_PROGfinal, "MVP", m_mvpForFinal);
	//CW3GLFunctions::drawView(m_vaoPlane, 6, GL_BACK);
	CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
	glUseProgram(0);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	drawOverrideSurface();
}

void CW3View3D_thyoo::renderingGL(void) {
	if (!this->isVisible())
		return;

	if (m_pgVREngine->isVRready()) {
		CW3GLFunctions::printError(__LINE__, "CW3View3D_thyoo::renderingGL");

		if (!m_is3Dready) {
			initializeGL();
			m_is3Dready = true;
		}

		if (!m_vaoPlane)
			m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);

#if kRenderActiveCube
		if (!m_vaoCUBE)
			m_pgVREngine->setActiveIndex(&m_vaoCUBE, 0);
#else
		if (!m_vaoCUBE) {
			unsigned int *vbo = m_pgVREngine->getVolVBO();
			CW3GLFunctions::initVAO(&m_vaoCUBE, vbo);
		}
#endif

		basicRayCasting();
		blendingGL();
		drawFinal();

		CW3GLFunctions::printError(__LINE__, "CW3View3D_thyoo::renderingGL");

		m_isChanging = false;
		m_rotAngle = 0.0f;
	} else {
		CW3GLFunctions::clearView(true);
	}
}

void CW3View3D_thyoo::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();

	this->renderingGL();

	//if (m_pWorldAxisItem)
	//{
	//	m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
	//}

	painter->endNativePainting();
}

void CW3View3D_thyoo::setMVP(float rotAngle, glm::vec3 rotAxis) {
	mat4 view = m_view * m_arcMat;
	m_mvp = m_projection * view * m_reorienMat * m_model;
}

void CW3View3D_thyoo::setReorientation(const glm::mat4& reorienMat) {
	m_reorienMat = reorienMat;
}

void CW3View3D_thyoo::setViewMatrix() {
	{
		m_view = glm::lookAt(glm::vec3(0.0f, -m_camFOV * 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}
}

void CW3View3D_thyoo::setProjection() {
	float width = this->width();
	float height = this->height();

	if (width > height) {
		float ratio = width / height*m_camFOV;

		proj_left_ = -ratio / m_scale + m_WglTrans;
		proj_right_ = ratio / m_scale + m_WglTrans;
		proj_bottom_ = -m_camFOV / m_scale - m_HglTrans;
		proj_top_ = m_camFOV / m_scale - m_HglTrans;

		m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, m_camFOV*20.0f);
		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneHinView;
	} else {
		float ratio = height / width*m_camFOV;

		proj_left_ = -m_camFOV / m_scale + m_WglTrans;
		proj_right_ = m_camFOV / m_scale + m_WglTrans;
		proj_bottom_ = -ratio / m_scale - m_HglTrans;
		proj_top_ = ratio / m_scale - m_HglTrans;

		m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, m_camFOV*20.0f);
		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneWinView;
	}

	setViewRulerValue();
}

void CW3View3D_thyoo::resizeScene() {
	CW3View2D_thyoo::resizeScene();

	m_pShadeSwitch->setPos(QPointF(-m_sceneWinView * 0.75f, -m_sceneHinView * 0.75f));

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

void CW3View3D_thyoo::resizeEvent(QResizeEvent *pEvent) {
	if (!isVisible())
		return;

	CW3View2D_thyoo::resizeEvent(pEvent);

	if (m_eViewType == common::ViewTypeID::CEPH ||
		m_eViewType == common::ViewTypeID::FACE_AFTER) {
		float horizontalSpacing = m_lpTextAlign[0]->sceneBoundingRect().width();
		for (int i = 0; i < 6; i++) {
			if (m_lpTextAlign[i]) {
				m_lpTextAlign[i]->setPos(
					mapToScene(width() - common::ui_define::kViewMarginWidth - (horizontalSpacing * (6 - i)),
							   common::ui_define::kViewFilterOffsetY));
			}
		}
	}

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

void CW3View3D_thyoo::mousePressEvent(QMouseEvent* event)
{
	last_scene_pos_ = curr_scene_pos_ = mapToScene(event->pos());
	last_view_pos_ = curr_view_pos_ = event->pos();

	if (measure_3d_manager_ &&
		(common_tool_type_ == CommonToolTypeOnOff::M_RULER ||
			common_tool_type_ == CommonToolTypeOnOff::M_ANGLE ||
			common_tool_type_ == CommonToolTypeOnOff::M_DEL))
	{
		vec3 volume_pos;
		bool volume_picked = volumeTracking(event->pos(), volume_pos);

		if (volume_picked || common_tool_type_ == CommonToolTypeOnOff::M_DEL)
		{
			bool update;
			measure_3d_manager_->MousePressEvent(event->button(), volume_pos, update);

			if (update)
			{
				scene()->update();
			}
		}
	}
	else
	{
		CW3View2D_thyoo::mousePressEvent(event);
	}
}

void CW3View3D_thyoo::mouseMoveEvent(QMouseEvent *event) {
	Qt::MouseButtons buttons = event->buttons();
	CW3Cursor::SetViewCursor(common_tool_type_);
	curr_view_pos_ = event->pos();
	curr_scene_pos_ = mapToScene(event->pos());

	if (measure_3d_manager_)
	{
		if (!measure_3d_manager_->started() && buttons == Qt::NoButton)
		{
			bool update = false;
			uint pick_program = m_pgVREngine->getPROGpickWithCoord();
			m_pGLWidget->makeCurrent();
			measure_3d_manager_->Pick(size(), event->pos(), &update, pick_program);
			m_pGLWidget->doneCurrent();

			if (update)
			{
				scene()->update();
			}
		}
		else
		{
			vec3 volume_pos;
			bool volume_picked = volumeTracking(event->pos(), volume_pos);

			if (volume_picked)
			{
				bool update;
				measure_3d_manager_->MouseMoveEvent(buttons, volume_pos, update);

				if (update)
				{
					scene()->update();
				}
			}
		}
	}

	if (buttons == Qt::RightButton) {
		QGraphicsView::mouseMoveEvent(event);
		QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
		m_isChanging = true;

		ArcBallRotate();

		if (m_pWorldAxisItem) {
			m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
		}
		last_view_pos_ = curr_view_pos_;
		scene()->update();
	} else {
		CW3View2D_thyoo::mouseMoveEvent(event);
	}
}

void CW3View3D_thyoo::mouseReleaseEvent(QMouseEvent *event) {
	CW3View2D_thyoo::mouseReleaseEvent(event);

	m_isChanging = false;
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());

	emit sigRenderCompleted();
	scene()->update();
}

void CW3View3D_thyoo::wheelEvent(QWheelEvent *event) {
	float dist = -event->delta() / 5.0f;

	curr_view_pos_ = QPointF(last_view_pos_.x() + dist, last_view_pos_.y() + dist);

	mouseScaleEvent();
}

void CW3View3D_thyoo::ArcBallRotate() {
	glm::vec3 v1 = ArcBallVector(last_view_pos_);
	glm::vec3 v2 = ArcBallVector(curr_view_pos_);

	float arcball_rotate_angle = 0.0f;
	if (glm::length(v1 - v2) < 0.001f)
	{
		m_rotAxis.x = 1.0f;
		m_rotAxis.y = 0.0f;
		m_rotAxis.z = 0.0f;
	}
	else
	{
		arcball_rotate_angle =
			std::acos(std::min(1.0f, glm::dot(v1, v2))) * 180.0f / M_PI;
		arcball_rotate_angle *= common::kArcballSensitivity;

		m_rotAxis = glm::cross(v1, v2);
		m_rotAxis = glm::normalize(m_rotAxis);
	}

	setRotateMatrix(glm::rotate(glm::radians(arcball_rotate_angle), m_rotAxis) * m_arcMat);
}

glm::vec3 CW3View3D_thyoo::ArcBallVector(QPointF &v) {
	float w = static_cast<float>(this->width());
	float h = static_cast<float>(this->height());
	vec3 ABvector =
		vec3(2.0f * v.x() / w - 1.0f, 0.0f, -(2.0f * v.y() / h - 1.0f));

	float xzSq = ABvector.x * ABvector.x + ABvector.z * ABvector.z;
	if (xzSq < 1.0f)
	{
		ABvector.y = sqrt(1.0f - xzSq);
	}
	else
	{
		ABvector = glm::normalize(ABvector);
	}
	return ABvector;
}

void CW3View3D_thyoo::slotReoriupdate(glm::mat4 *M) {
	m_model = *M*glm::scale(m_vVolRange);

	scene()->update();
}

//FrameBuffer는 반드시 m_pGLWidget->defaultFramebufferObject()이어야 한다.
bool CW3View3D_thyoo::volumeTracking(const QPointF& mousePos, glm::vec3& ptOutVolume) {
	try {
		m_pGLWidget->makeCurrent();
		{
			CW3VolumeRenderParam* pRenderParam = m_pgVREngine->getVRparams(0);

			int width = m_width3Dview;
			int height = m_height3Dview;

			if (mousePos.x() > width || mousePos.y() > height)
				throw std::runtime_error("mousePosition is not in range.");

#if 0
			CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/TEX_ENTRY_POSITION.png", m_texHandler[TEX_ENTRY_POSITION], GL_RGBA, GL_UNSIGNED_BYTE);
			CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/TEX_EXIT_POSITION.png", m_texHandler[TEX_EXIT_POSITION], GL_RGBA, GL_UNSIGNED_BYTE);
#endif

			int ptX = mousePos.x();
			int ptY = this->height() - mousePos.y();

			vec4 start;
			vec4 end;

			glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandler);

			glReadBuffer(m_texBuffer[TEX_ENTRY_POSITION]);
			glReadPixels(ptX, ptY, 1, 1, GL_RGBA, GL_FLOAT, &start);

			glReadBuffer(m_texBuffer[TEX_EXIT_POSITION]);
			glReadPixels(ptX, ptY, 1, 1, GL_RGBA, GL_FLOAT, &end);

#if 0
			qDebug() << "start :" << start.x << start.y << start.z << start.a;
			qDebug() << "end :" << end.x << end.y << end.z << end.a;
#endif

			if (printGLError(__LINE__))
				throw std::runtime_error("glError.");

			vec4 dir = end - start;
			dir.w = glm::length(vec3(dir));
			dir = vec4(vec3(dir) / dir.w, dir.w);

			float lengthAccum = 0.0;

			float len = dir[3];

			if (len == 0.0f || std::numeric_limits<float>::infinity() == len)
				throw std::runtime_error("position not found");

			float deltaX = m_stepSize * 2.0f; //voxel Size
			float currIntensity = 0.0;
			vec3 vVolRange = m_vVolRange;
			unsigned short** volData = pRenderParam->m_pgVol->getData();

			vec3 currPoint = vec3(start);
			vec3 deltaDir = vec3(dir)*deltaX;
			int MaxTexSize = m_pgVREngine->getMaxTexAxisSize();

			float alpha = 0.0;

			vec3 vVolMaxIdx(vVolRange.x - 1, vVolRange.y - 1, vVolRange.z - 1);

			int tfWidth, tfHeight;
			m_pgVREngine->getVolTFTexSize(&tfWidth, &tfHeight);

			const auto& res_tf = ResourceContainer::GetInstance()->GetTfResource();
			float* tfData = new float[tfWidth*tfHeight * 4];
			memcpy(tfData, res_tf.getTF(), sizeof(float)*tfWidth*tfHeight * 4);

			for (; lengthAccum < len; lengthAccum += deltaX) {
				vec3 volIdx = vVolMaxIdx * currPoint;
				int ix, iy, iz;
				ix = (volIdx.x < 0.0) ? 0 : (volIdx.x > vVolMaxIdx.x) ? (int)vVolMaxIdx.x : (int)volIdx.x;
				iy = (volIdx.y < 0.0) ? 0 : (volIdx.y > vVolMaxIdx.y) ? (int)vVolMaxIdx.y : (int)volIdx.y;
				iz = (volIdx.z < 0.0) ? 0 : (volIdx.z > vVolMaxIdx.z) ? (int)vVolMaxIdx.z : (int)volIdx.z;

				int ixy = ix + iy * vVolRange.x;
				float intensity = volData[iz][ixy];
				vec2 tfIdx = vec2((intensity - (static_cast<int>(intensity / (float)MaxTexSize)*MaxTexSize)),
								  static_cast<int>(intensity / MaxTexSize));

				int idx = static_cast<int>(tfIdx.x*4.0) +
					static_cast<int>(tfIdx.y*4.0*(float)tfWidth) + 3;

				alpha += tfData[idx];
				if (alpha > 0.1f) {
					break;
				}
				currPoint += deltaDir;
			}

			if (alpha < 0.1f)
				throw std::runtime_error("position not found");

			mat4 invTexBias = glm::inverse(pRenderParam->m_volTexBias);

			ptOutVolume = vec3(invTexBias*vec4(currPoint, 1.0f));
			SAFE_DELETE_ARRAY(tfData);
		}
		m_pGLWidget->doneCurrent();

		return true;
	} catch (std::exception& e) {
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3View3D::volumeTracking: " + err_msg);
		return false;
	}

	return true;
}

bool CW3View3D_thyoo::printGLError(unsigned int line) {
	if (glGetError() != GL_NO_ERROR) {
		CW3GLFunctions::printError(line, "thyoo_view3d failed.");
		return true;
	} else
		return false;
}

void CW3View3D_thyoo::ResetView() {
	if (!isVisible())
		return;
	if (!m_is3Dready)
		return;

	m_scalePre = m_scale;
	setRotateMatrix(mat4(1.0f));
	emit sigRenderCompleted();
	CW3View2D_thyoo::ResetView();

	if (m_pWorldAxisItem) {
		m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
	}

	m_scalePre = 1.0f;

	scene()->update();
}

void CW3View3D_thyoo::FitView() {
	if (!isVisible())
		return;

	m_scalePre = m_scale;
	CW3View2D_thyoo::FitView();
	m_scalePre = 1.0f;
}

void CW3View3D_thyoo::HideUI(bool bToggled) {
	CW3View2D_thyoo::HideUI(bToggled);

	if (m_eViewType == common::ViewTypeID::CEPH ||
		m_eViewType == common::ViewTypeID::FACE_AFTER) {
		for (int i = 0; i < 6; i++) {
			if (m_lpTextAlign[i]) {
				m_lpTextAlign[i]->setVisible(!hide_all_view_ui_);
			}
		}
	}
}

void CW3View3D_thyoo::slotVRAlignS() {
	setRotateMatrix(glm::rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)));
	setVRAlign();
}

void CW3View3D_thyoo::slotVRAlignI() {
	setRotateMatrix(glm::rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f)));
	setVRAlign();
}

void CW3View3D_thyoo::slotVRAlignL() {
	setRotateMatrix(glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)));
	emit sigRotateMat(m_arcMat);
	setVRAlign();
}

void CW3View3D_thyoo::slotVRAlignR() {
	setRotateMatrix(glm::rotate(glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f)));
	setVRAlign();
}

void CW3View3D_thyoo::slotVRAlignA() {
	setRotateMatrix(mat4(1.0f));
	setVRAlign();
}

void CW3View3D_thyoo::slotVRAlignP() {
	setRotateMatrix(glm::rotate(glm::radians(180.0f), vec3(0.0f, 0.0f, 1.0f)));
	setVRAlign();
}

void CW3View3D_thyoo::setVRAlign() {
	glm::mat4 mv = m_view * m_arcMat;
	if (m_pWorldAxisItem)
		m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);

	emit sigRotateMat(m_arcMat);
	scene()->update();
}

void CW3View3D_thyoo::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		{
			if (m_FBHandler) {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDeleteFramebuffers(1, &m_FBHandler);
				m_FBHandler = 0;
			}
			if (m_depthHandler.size() > 0) {
				glDeleteRenderbuffers(m_depthHandler.size(), &m_depthHandler[0]);
				m_depthHandler.assign(m_depthHandler.size(), 0);
			}
			if (m_texHandler.size() > 0) {
				glDeleteTextures(m_texHandler.size(), &m_texHandler[0]);
				m_texHandler.assign(m_texHandler.size(), 0);
			}
			if (m_vaoPlane) {
				glDeleteVertexArrays(1, &m_vaoPlane);
				m_vaoPlane = 0;
			}
			if (m_vaoCUBE) {
				glDeleteVertexArrays(1, &m_vaoCUBE);
				m_vaoCUBE = 0;
			}
		}
		m_pGLWidget->doneCurrent();
	}

	if (measure_3d_manager_)
	{
		measure_3d_manager_->ClearVAOVBO();
	}

	CW3View2D_thyoo::clearGL();
}

void CW3View3D_thyoo::mouseScaleEvent() {
	m_isChanging = true;
	CW3View2D_thyoo::mouseScaleEvent();
}

void CW3View3D_thyoo::mousePanningEvent() {
	m_isChanging = true;
	CW3View2D_thyoo::mousePanningEvent();
}

void CW3View3D_thyoo::setRotateMatrix(const glm::mat4 & mat) {
	m_arcMat = mat;
	emit sigRotateMat(mat);
}

void CW3View3D_thyoo::HideMeasure(bool toggled)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(!toggled);

		scene()->update();
	}
}

void CW3View3D_thyoo::DeleteAllMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->Clear();

		scene()->update();
	}
}

void CW3View3D_thyoo::DeleteUnfinishedMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->DeleteUnfinishedItem();

		scene()->update();
	}
}

void CW3View3D_thyoo::keyPressEvent(QKeyEvent* event)
{
	glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);

	if (event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Down)
	{
		rotate_axis = glm::vec3(-1.0f, 0.0f, 0.0f);
	}

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up)
	{
		m_arcMat = glm::rotate(glm::radians(-1.0f), rotate_axis) * m_arcMat;
	}
	else if (event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
		m_arcMat = glm::rotate(glm::radians(1.0f), rotate_axis) * m_arcMat;
	}

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
		setRotateMatrix(m_arcMat);
		if (m_pWorldAxisItem)
		{
			m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
		}
		scene()->update();
	}

	CW3View2D_thyoo::keyPressEvent(event);
}

void CW3View3D_thyoo::ApplyPreferences()
{
	CW3View2D_thyoo::ApplyPreferences();

	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;
	switch (volume_rendering_quality)
	{
	case GlobalPreferences::Quality2::High:
		low_res_frame_buffer_resize_factor_ = 0.5f;
		low_res_step_size_factor_ = 2.0f;
		break;
	case GlobalPreferences::Quality2::Low:
		low_res_frame_buffer_resize_factor_ = 0.25f;
		low_res_step_size_factor_ = 8.0f;
		break;
	}
}
