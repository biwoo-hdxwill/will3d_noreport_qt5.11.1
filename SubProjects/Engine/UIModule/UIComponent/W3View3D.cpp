#include "W3View3D.h"
/*=========================================================================

File:			class CW3View3D
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last date:		2016-06-04

=========================================================================*/
#include <QApplication>
#include <qgraphicsproxywidget.h>
#include <qmath.h>

#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_view.h"
#endif
#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>
#include "../UIGLObjects/W3GLObject.h"
#include "../UIGLObjects/W3GLNerve.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3FilteredTextItem.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3ActiveBlock.h"
#include "../../Module/VREngine/W3Render3DParam.h"

namespace {
// TODO smseo : 볼륨 좌표를 넣기 전 임시 값
const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

using namespace NW3Render3DParam;

CW3View3D::CW3View3D(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					 CW3ResourceContainer *Rcontainer,
					 common::ViewTypeID eType, QWidget *pParent)
	: CW3View2D(VREngine, MPRengine, Rcontainer, eType, pParent) {
	if (m_eViewType == common::ViewTypeID::SUPERIMPOSITION ||
		m_eViewType == common::ViewTypeID::ENDO_MODIFY ||
		m_eViewType == common::ViewTypeID::ENDO) {
		recon_type_ = common::ReconTypeID::VR;
	}

	m_pWorldAxisItem = new ViewNavigatorItem();
	scene()->addItem(m_pWorldAxisItem);

	m_camFOV = 1.0f;
	m_rotAngle = 0.0f;
	m_rotAxis = vec3(1.0f, 0.0f, 0.0f);

	m_viewForFinal = m_projForFinal = glm::mat4(1.0f);

	m_ToDepth = glm::mat4(1.0f);
	m_ToDepth[2].z = 0.5f;
	m_ToDepth[3].z = 0.5f;

	if (m_eViewType == common::ViewTypeID::SUPERIMPOSITION ||
		m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		QFont font = QApplication::font();
		font.setPixelSize(font.pixelSize() - 1);

		for (int i = 0; i < 6; i++) {
			m_lpTextAlign[i] = new CW3TextItem();
			m_lpTextAlign[i]->setFont(font);

			m_lpTextAlign[i]->setVisible(false);
			scene()->addItem(m_lpTextAlign[i]);
		}

		m_lpTextAlign[0]->setPlainText("A");
		m_lpTextAlign[1]->setPlainText("P");
		m_lpTextAlign[2]->setPlainText("L");
		m_lpTextAlign[3]->setPlainText("R");
		m_lpTextAlign[4]->setPlainText("I");
		m_lpTextAlign[5]->setPlainText("S");
	}

	if (m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	} else if (m_eViewType == common::ViewTypeID::ENDO_SAGITTAL) {
		m_rotate = glm::rotate(glm::radians(90.0f),
							   vec3(0.0f, 1.0f, 0.0f))*glm::rotate(glm::radians(-90.0f),
																   vec3(1.0f, 0.0f, 0.0f));
	} else {
		m_rotate = glm::mat4(1.0f);
	}

	if (m_pSlider)
		m_pSlider->setInvertedAppearance(true);

	m_origBackVector = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);

	connections();
}

CW3View3D::~CW3View3D(void) {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(m_pAirwayColorBar);
	SAFE_DELETE_OBJECT(recon_type_selection_ui_);

	for (int i = 0; i < m_lpTextColorBar.size(); i++)
		SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
	m_lpTextColorBar.clear();

	for (int i = 0; i < 6; i++) {
		SAFE_DELETE_OBJECT(m_lpTextAlign[i]);
	}

	SAFE_DELETE_OBJECT(measure_3d_manager_);
}

void CW3View3D::clearGL()
{
	CW3View2D::clearGL();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->ClearVAOVBO();
	}
}
#ifndef WILL3D_VIEWER
void CW3View3D::exportProject(ProjectIOView & out) {
	CW3View2D::exportProject(out);
	out.SaveRotateMatrix(m_rotate);
	out.SaveVolRange(m_vVolRange);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void CW3View3D::importProject(ProjectIOView & in) {
	CW3View2D::importProject(in);
	in.LoadRotateMatrix(m_rotate);
	in.LoadVolRange(m_vVolRange);
	
	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(scene());
	}
	measure_3d_manager_->ImportProject(in);
}
#endif
void CW3View3D::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type) {
#if 0
	if (type >= common::CommonToolTypeOnOff::M_RULER && type < common::CommonToolTypeOnOff::M_DEL)
		common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	else
		common_tool_type_ = type;
	measure_tools_->SetMeasureType(common_tool_type_);
#else
	CW3View2D::SetCommonToolOnOff(type);

	if (measure_3d_manager_ && m_eViewType != common::ViewTypeID::ENDO)
	{
		measure_3d_manager_->SetType(type);
	}
#endif
}

void CW3View3D::reset() {
	CW3View2D::reset();

	m_is3Dready = false;

	m_pRender3DParam->m_isClipped = false;

	if (m_eViewType == common::ViewTypeID::SUPERIMPOSITION ||
		m_eViewType == common::ViewTypeID::ENDO_MODIFY ||
		m_eViewType == common::ViewTypeID::ENDO) {
		recon_type_ = common::ReconTypeID::VR;
	}

	m_camFOV = 1.0f;
	m_rotAngle = 0.0f;
	m_rotAxis = vec3(1.0f, 0.0f, 0.0f);
	m_viewForFinal = m_projForFinal = glm::mat4(1.0f);

	m_ToDepth = glm::mat4(1.0f);
	m_ToDepth[2].z = 0.5f;
	m_ToDepth[3].z = 0.5f;

	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		if (m_vboAirway[0]) {
			glDeleteBuffers(4, m_vboAirway);
			m_vboAirway[0] = 0;
			m_vboAirway[1] = 0;
			m_vboAirway[2] = 0;
			m_vboAirway[3] = 0;
		}
		m_pGLWidget->doneCurrent();
	}

	if (m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		SAFE_DELETE_OBJECT(m_pAirwayColorBar);

		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
		m_lpTextColorBar.clear();

		m_nNumColor = 0;
		m_nColorMin = 0;
		m_nColorMax = 0;
	}

	if (m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	} else if (m_eViewType == common::ViewTypeID::ENDO_SAGITTAL) {
		m_rotate = glm::rotate(glm::radians(90.0f),
							   vec3(0.0f, 1.0f, 0.0f))*glm::rotate(glm::radians(-90.0f),
																   vec3(1.0f, 0.0f, 0.0f));
	} else {
		m_rotate = glm::mat4(1.0f);
	}

	m_isPassive = false;
	m_isMinMaxChanged = false;

	m_pRender3DParam->m_isPerspective = false;
	m_pRender3DParam->m_isShading = true;

	m_origBackVector = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

}

void CW3View3D::slotChangeMIP(bool isEnable) {
	m_pRender3DParam->m_isMIP = isEnable;
	RenderAndUpdate();
}

void CW3View3D::slotChangedValueSlider(int value) {
	if (!m_pSlider)
		return;

	if ((m_is3Dready || m_is2Dready) && m_pSlider->pressed()) {
		changedDeltaSlider(value);
	}
}

void CW3View3D::connections() {
	if (m_lpTextAlign[0]) {
		connect(m_lpTextAlign[0], SIGNAL(sigPressed()), this, SLOT(slotVRAlignA()));
		connect(m_lpTextAlign[1], SIGNAL(sigPressed()), this, SLOT(slotVRAlignP()));
		connect(m_lpTextAlign[2], SIGNAL(sigPressed()), this, SLOT(slotVRAlignL()));
		connect(m_lpTextAlign[3], SIGNAL(sigPressed()), this, SLOT(slotVRAlignR()));
		connect(m_lpTextAlign[4], SIGNAL(sigPressed()), this, SLOT(slotVRAlignI()));
		connect(m_lpTextAlign[5], SIGNAL(sigPressed()), this, SLOT(slotVRAlignS()));
	}
}

void CW3View3D::setInitScale() {
	m_initScale = sqrt(5.0f);
	m_scale = m_initScale;
}

void CW3View3D::init() {

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	measure_tools_->SetViewRenderMode(common::ReconTypeID::VR);
	m_pRender3DParam->m_pgMainVolume[0] = m_pgVREngine->getVRparams(0);
	m_pRender3DParam->m_pgMainVolume[0]->m_isShown = true;

	m_vVolRange = *m_pgMPRengine->getVolRange(0);
	m_scaleMat = glm::scale(m_vVolRange);
	m_inverseScale = glm::scale(1.0f / m_vVolRange);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");

	m_origModel = *m_pgVREngine->getReorientedModel();

	m_camFOV = glm::length(m_vVolRange)*sqrt(2.0f);

	m_pRender3DParam->m_pgMainVolume[0]->m_VolScaleIso = m_vVolRange;

	setInitScale();
	setViewMatrix();
	setProjection();

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	m_model = m_rotate * m_origModel*m_scaleMat;

	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
	CW3GLFunctions::printError(__LINE__, "View3D Init failed.");

	m_VolCenter = *m_pgMPRengine->getMPRrotCenterOrigInVol(0);

	if (m_pRender3DParam->m_plane->getVAO())
		m_pRender3DParam->m_plane->clearVAOVBO();

	setWLWW();

	unsigned int vao = 0;
	m_pgVREngine->initVAOplane(&vao);
	m_pRender3DParam->m_plane->setVAO(vao);
	m_pRender3DParam->m_plane->setNindices(6);

	m_viewForFinal = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
								 glm::vec3(0.0f, 0.0f, 0.0f),
								 glm::vec3(0.0f, 1.0f, 0.0f));

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	m_projForFinal = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);

	m_pRender3DParam->m_plane->setMVP(glm::mat4(1.0f), m_viewForFinal, m_projForFinal);
	m_is3Dready = true;

	m_pWorldAxisItem->setVisible(!hide_all_view_ui_);
	m_pGLWidget->doneCurrent();
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
	m_pGLWidget->makeCurrent();
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	if (ruler_) {
		ruler_->setVisible(!hide_all_view_ui_ && show_rulers_);
	}

	if (m_lpTextAlign[0]) {
		for (int i = 0; i < 6; i++)
			m_lpTextAlign[i]->setVisible(!hide_all_view_ui_);
	}

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");
	if (m_pSlider)
		m_pSlider->setRange(-m_vVolRange.z, m_vVolRange.z);

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

void CW3View3D::slotTFupdated(bool isMinMaxChanged) {
	if (!isVisible())
		return;

	if (recon_type_ == common::ReconTypeID::VR ||
		recon_type_ == common::ReconTypeID::MIP) {
		if (m_is3Dready)
			m_pRender3DParam->m_isLowRes = true;

		RenderAndUpdate();
	}
}

void CW3View3D::slotTFupdateCompleted() {
	if (!isVisible())
		return;

	m_pRender3DParam->m_isLowRes = false;
	RenderAndUpdate();
}

void CW3View3D::render3D() {
	if (!isReadyRender3D())
		return;

	setProjection();
	setMVP();
	m_pGLWidget->makeCurrent();

	///////////////////////////////////////////////////////////
	//ready for airway
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_pAirway && m_pRender3DParam->m_pAirway->isVisible()) {
		setAirwayVAO();
		m_pRender3DParam->m_pAirway->setInvModel(m_inverseScale);
		m_pRender3DParam->m_pAirway->setMVP(m_model, m_view, m_projection);
	}

	///////////////////////////////////////////////////////////
	//ready for raycasting
	///////////////////////////////////////////////////////////
	m_pRender3DParam->m_width = this->width();
	m_pRender3DParam->m_height = this->height();

	if (m_drawVolId == 0) {
		m_pRender3DParam->m_pgMainVolume[0]->m_mvp = &m_mvp;
	} else {
		m_pRender3DParam->m_pgMainVolume[1]->m_mvp = &m_mvpSecond;
	}

	mat4 invModel(1.0f);
	m_pRender3DParam->m_pgMainVolume[0]->m_invModel = &invModel;

	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);

	WGLSLprogram::setUniform(PROGRaycasting, "useSegTmj", false);
	WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", false);

	if (!m_pRender3DParam->m_plane->getVAO()) {
		m_pRender3DParam->m_plane->clearVAOVBO();

		unsigned int vao = 0;
		m_pgVREngine->initVAOplane(&vao);
		m_pRender3DParam->m_plane->setVAO(vao);
		m_pRender3DParam->m_plane->setNindices(6);
	}

#if 0
	if (!m_pRender3DParam->m_mainVolume_vao[0]) {

		if (m_isDrawBoth) {
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);

			if (m_pRender3DParam->m_mainVolume_vao[1]) {
				glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[1]);
				m_pRender3DParam->m_mainVolume_vao[1] = 0;
			}

			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
		} else {
			if (m_pRender3DParam->m_mainVolume_vao[m_drawVolId]) {
				glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[m_drawVolId]);
				m_pRender3DParam->m_mainVolume_vao[m_drawVolId] = 0;
			}

			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
		}
	}
	else {
	  m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
	}
#else
	if (m_isDrawBoth)
	{
		if (!m_pRender3DParam->m_mainVolume_vao[0])
		{
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
		}
		if (!m_pRender3DParam->m_mainVolume_vao[1])
		{
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
		}
	}
	else
	{
		if (!m_pRender3DParam->m_mainVolume_vao[m_drawVolId])
		{
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
		}
	}
#endif

	///////////////////////////////////////////////////////////
	//render
	///////////////////////////////////////////////////////////
	if (m_isDrawBoth) {
		m_pRender3DParam->m_pgMainVolume[0]->m_matFirstHitToDepth = &m_matTexCoordToDepth;

		m_pRender3DParam->m_pgMainVolume[1]->m_mvp = &m_mvpSecond;
		m_pRender3DParam->m_pgMainVolume[1]->m_matFirstHitToDepth = &m_matTexCoordToDepthSecond;

		m_pgVREngine->Render3Dboth(m_pRender3DParam, 0, m_isReconSwitched, true);
	} else {
		m_pgVREngine->Render3Dboth(m_pRender3DParam, m_drawVolId, m_isReconSwitched);
	}

	m_pGLWidget->doneCurrent();
}

void CW3View3D::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();

	if (m_pgVREngine->isVRready()) {
		if (!m_is3Dready) {
			init();
			m_pGLWidget->doneCurrent();
			render3D();
			m_pGLWidget->makeCurrent();
		}

		m_pgVREngine->Render3Dfinal(m_pRender3DParam);

		DrawMeasure3D();
	} else {
		CW3GLFunctions::clearView(false);
	}

	painter->endNativePainting();

	m_isOnlyItemUpdate = false;
}

void CW3View3D::setMVP() {
	m_mvp = m_projection * m_view*m_model;

	if (m_pRender3DParam->m_pgMainVolume[0]) {
		m_matTexCoordToDepth = m_ToDepth * m_mvp*m_matTextureToGL;
		m_pRender3DParam->m_pgMainVolume[0]->m_matFirstHitToDepth = &m_matTexCoordToDepth;
	}

	if (m_pRender3DParam->m_pgMainVolume[1]) {
		m_mvpSecond = m_projection * m_view*m_modelSecond;

		m_matTexCoordToDepthSecond = m_ToDepth * m_mvpSecond*m_matTextureToGL;
		m_pRender3DParam->m_pgMainVolume[1]->m_matFirstHitToDepth = &m_matTexCoordToDepthSecond;
	}
}

void CW3View3D::setViewMatrix() {
	if (m_eViewType == common::ViewTypeID::ENDO_SAGITTAL ||
		m_eViewType == common::ViewTypeID::TMJ_ARCH) {
		m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, m_camFOV),
							 glm::vec3(0.0f, 0.0f, 0.0f),
							 glm::vec3(0.0f, -1.0f, 0.0f));

		if (m_pgVREngine->getVRparams(0)) {
			m_pRender3DParam->m_lightInfo.Intensity = vec3(1.0f);
			m_pRender3DParam->m_lightInfo.Position = m_view * glm::scale(m_vVolRange)*vec4(0.0f, 10.0f, 0.0f, 1.0f);
		}
	} else {
		if (m_pRender3DParam->m_isPerspective)
			m_view = glm::lookAt(glm::vec3(0.0f, -m_camFOV * 3.0f, 0.0f),
								 glm::vec3(0.0f, 0.0f, 0.0f),
								 glm::vec3(0.0f, 0.0f, -1.0f));
		else
			m_view = glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f),
								 glm::vec3(0.0f, 0.0f, 0.0f),
								 glm::vec3(0.0f, 0.0f, -1.0f));

		if (m_pgVREngine->getVRparams(0)) {
			m_pRender3DParam->m_lightInfo.Intensity = vec3(1.0f);
			m_pRender3DParam->m_lightInfo.Position = m_view * glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
		}
	}
}

void CW3View3D::setProjection() {
	float width = m_sceneWinView;
	float height = m_sceneHinView;

	if (width > height) {
		float ratio = width / height*m_camFOV;

		proj_left_ = -ratio / m_scale + m_WglTrans;
		proj_right_ = ratio / m_scale + m_WglTrans;
		proj_bottom_ = -m_camFOV / m_scale - m_HglTrans;
		proj_top_ = m_camFOV / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneHinView;
	} else {
		float ratio = height / width*m_camFOV;

		proj_left_ = -m_camFOV / m_scale + m_WglTrans;
		proj_right_ = m_camFOV / m_scale + m_WglTrans;
		proj_bottom_ = -ratio / m_scale - m_HglTrans;
		proj_top_ = ratio / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneWinView;
	}

	if (m_pRender3DParam->m_isPerspective) {
		m_projection = glm::frustum(proj_left_ * 0.5f, proj_right_ * 0.5f,
									proj_bottom_ * 0.5f, proj_top_ * 0.5f,
									m_camFOV * 2.0f, m_camFOV * 4.0f);
	} else {
		m_projection = glm::ortho(proj_left_, proj_right_,
								  proj_bottom_, proj_top_,
								  0.0f, m_camFOV * 2.0f);
	}

	measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);
	measure_tools_->UpdateProjection();

	setViewRulerValue();
	SetGridValue();
}

void CW3View3D::resizeEvent(QResizeEvent *pEvent) {
	if (!isVisible())
		return;

	CW3View2D::resizeEvent(pEvent);

	if (m_lpTextAlign[0]) {
		float horizontalSpacing = m_lpTextAlign[0]->sceneBoundingRect().width();
		for (int i = 0; i < 6; i++) {
			m_lpTextAlign[i]->setPos(
				mapToScene(width() - common::ui_define::kViewMarginWidth - (horizontalSpacing * (6 - i)),
						   common::ui_define::kViewFilterOffsetY));
		}
	}

	if (m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		int stratColorVal = m_nColorMin / 100;
		int endColorVal = (m_nColorMax / 100) + 1;
		m_nNumColor = abs(endColorVal - stratColorVal);

		float minPosY = (-(-0.75f) + 1.0f) * 0.5f * height();
		float maxPosY = (-(0.75f) + 1.0f) * 0.5f * height();
		float interval = abs(maxPosY - minPosY) / (float)m_nNumColor;

		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			if (m_lpTextColorBar.at(i))
				m_lpTextColorBar.at(i)->setPos((-0.65 + 1.0f) * 0.5f * width(), (minPosY - (interval * i)) - 15.0f);

		if (m_pAirwayColorBar && m_pRender3DParam->m_pAirway->isVisible())
			drawAirwayColorBar();
	}

	if (recon_type_ == common::ReconTypeID::VR ||
		recon_type_ == common::ReconTypeID::MIP) {
		if (m_pGLWidget && m_pGLWidget->context())
			render3D();
	}

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

void CW3View3D::resizeScene() {
	if (!isVisible())
		return;

	CW3View2D::resizeScene();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

bool CW3View3D::mousePressEventW3(QMouseEvent *event) {
	last_scene_pos_ = mapToScene(event->pos());
	last_view_pos_ = event->pos();

	return false;
}

void CW3View3D::mousePressEvent(QMouseEvent *event) {
	if (measure_3d_manager_ &&
		(common_tool_type_ == CommonToolTypeOnOff::M_RULER ||
			common_tool_type_ == CommonToolTypeOnOff::M_ANGLE ||
			common_tool_type_ == CommonToolTypeOnOff::M_DEL))
	{
		m_pGLWidget->makeCurrent();
		vec3 volume_pos;
		bool volume_picked = m_pgVREngine->VolumeTracking(m_pRender3DParam, event->pos(), volume_pos);
		m_pGLWidget->doneCurrent();

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

	CW3View2D::mousePressEvent(event);

	if (!m_isReconSwitched)
		mousePressEventW3(event);
}

void CW3View3D::RotateWithArcBall(const QPoint & curr_view_pos) {
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
	m_pRender3DParam->m_isLowRes = true;
	ArcBallRotate(curr_view_pos);
}

void CW3View3D::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);

	if (measure_3d_manager_ &&
		common_tool_type_ < CommonToolTypeOnOff::V_LIGHT)
	{
		if (!measure_3d_manager_->started() && event->buttons() == Qt::NoButton)
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
			m_pGLWidget->makeCurrent();
			vec3 volume_pos;
			bool volume_picked = m_pgVREngine->VolumeTracking(m_pRender3DParam, event->pos(), volume_pos);
			m_pGLWidget->doneCurrent();

			if (volume_picked)
			{
				bool update;
				measure_3d_manager_->MouseMoveEvent(event->buttons(), volume_pos, update);

				if (update)
				{
					scene()->update();
				}
			}
		}
	}

	if (mouseMoveEventW3(event)) {
		RenderAndUpdate();
	} else {
		if (m_isOnlyItemUpdate)
			scene()->update();
	}
}

bool CW3View3D::mouseMoveEventW3(QMouseEvent *event) {
	Qt::MouseButtons buttons = event->buttons();
	CW3Cursor::SetViewCursor(common_tool_type_);

	QPointF scene_pos = mapToScene(event->pos());

	if (buttons == Qt::NoButton) {
		// empty if statement
	} else if (buttons == Qt::RightButton) {
		if (recon_type_ == common::ReconTypeID::VR ||
			recon_type_ == common::ReconTypeID::MIP) {
			if (event->buttons() & Qt::RightButton) {
				RotateWithArcBall(event->pos());

				if (m_eViewType != common::ViewTypeID::ENDO && m_pWorldAxisItem) {
					m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
				}

				return true;
			}
		}
	} else {
		if (event->buttons() & Qt::LeftButton) {
			switch (common_tool_type_) {
			case common::CommonToolTypeOnOff::V_LIGHT:
				m_nAdjustWindowLevel += (last_scene_pos_.x() - scene_pos.x()) * 2;
				m_nAdjustWindowWidth += (last_scene_pos_.y() - scene_pos.y()) * 2;
				changeWLWW();
				last_scene_pos_ = scene_pos;
				m_pRender3DParam->m_isLowRes = true;
				return true;
			case common::CommonToolTypeOnOff::V_PAN:
			case common::CommonToolTypeOnOff::V_PAN_LR:
				PanningView(scene_pos);
				m_pRender3DParam->m_isLowRes = true;
				return true;
			case common::CommonToolTypeOnOff::V_ZOOM:
			case common::CommonToolTypeOnOff::V_ZOOM_R:
				m_pRender3DParam->m_isLowRes = true;
				ScaleView(event->pos());
				return true;
			default:
				if (measure_tools_->IsSelected())
					measure_tools_->ProcessMouseMove(mapToScene(event->pos()), kTempPos);
				break;
			}
		} else {
			if (common_tool_type_ >= common::CommonToolTypeOnOff::M_RULER &&
				common_tool_type_ <= common::CommonToolTypeOnOff::M_DEL) {
				if (measure_tools_->IsDrawing())
					measure_tools_->ProcessMouseMove(mapToScene(event->pos()), kTempPos);
			}
		}
	}
	return false;
}

bool CW3View3D::mouseReleaseEventW3(QMouseEvent *event) {
	m_pRender3DParam->m_isLowRes = false;
	m_isOnlyItemUpdate = false;

	return m_pRender3DParam->m_isLowResDrawn;
}

void CW3View3D::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	bool is_current_measure_available =
		measure_tools_->IsMeasureInteractionAvailable(common_tool_type_);

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (m_lpTextAlign[0]) {
		for (int i = 0; i < 6; i++) {
			if (m_lpTextAlign[i]->isUnderMouse())
				return;
		}
	}

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
		return;

	if (mouseReleaseEventW3(event))
		RenderAndUpdate();

	CW3Cursor::SetViewCursor(common_tool_type_);
	Qt::MouseButton button = event->button();
	if (is_current_measure_available) {
		measure_tools_->ProcessMouseReleased(button, mapToScene(event->pos()), kTempPos);
		return;
	}
}

void CW3View3D::wheelEvent(QWheelEvent *event) {
	event->ignore();
}

void CW3View3D::setModel(float rotAngle, glm::vec3 rotAxis) {
	if (!m_isPassive) {
		m_rotate = glm::rotate(glm::radians(rotAngle), rotAxis)*m_rotate;
		m_model = m_rotate*m_origModel*m_scaleMat;
		m_modelSecond = m_rotate*m_translateSecond*m_rotateSecond*m_origModel*m_secondToFirstModel*m_scaleMatSecond;
	}
}

void CW3View3D::setModel() {
	m_model = m_rotate*m_origModel*m_scaleMat;

	m_modelSecond = m_rotate*m_translateSecond*m_rotateSecond*m_origModel*m_secondToFirstModel*m_scaleMatSecond;
}

void CW3View3D::setDirectionFromCompass(glm::mat4 &T) {
	m_rotate = glm::inverse(m_view)*T;
	m_rotate[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	setModel();
}

void CW3View3D::setVisible(bool isVisible) {
	CW3View2D::setVisible(isVisible);

	if (isVisible && m_is3Dready) {
		if (recon_type_ == common::ReconTypeID::VR ||
			recon_type_ == common::ReconTypeID::MIP) {
			render3D();
		}
	}

	if (isVisible)
		m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
}
void CW3View3D::setModelPhotoToMC() {
	m_modelPhoto = m_rotate*m_origModel*m_pgRcontainer->getFacePhoto3D()->getSRtoVol()*m_scaleMat;
}

void CW3View3D::ArcBallRotate(const QPoint& curr_view_pos) {
	glm::vec3 v1 = ArcBallVector(QPointF(last_view_pos_));
	glm::vec3 v2 = ArcBallVector(QPointF(curr_view_pos));

	if (glm::length(v1 - v2) < 0.001f) {
		m_rotAxis.x = 1.0f;
		m_rotAxis.y = 0.0f;
		m_rotAxis.z = 0.0f;

		m_rotAngle = 0.0f;
	} else {
		m_rotAngle = std::acos(std::min(1.0f, glm::dot(v1, v2)))*180.0f / M_PI;
		m_rotAngle *= common::kArcballSensitivity;

		m_rotAxis = glm::cross(v1, v2);
		m_rotAxis = glm::normalize(m_rotAxis);
	}

	setModel(m_rotAngle, m_rotAxis);
	last_view_pos_ = curr_view_pos;
}

glm::vec3 CW3View3D::ArcBallVector(QPointF &v) {
	vec3 ABvector;

	if (m_eViewType == common::ViewTypeID::ENDO) {
		ABvector = vec3((2.0f*v.x() / float(this->width()) - 1.0f), -(2.0f*v.y() / float(this->height()) - 1.0f), 0.0f);
		float xySq = ABvector.x*ABvector.x + ABvector.y*ABvector.y;

		if (xySq < 1.0f) {
			ABvector.z = sqrt(1.0f - xySq);
		} else {
			ABvector = glm::normalize(ABvector);
		}

	} else {
		ABvector = vec3(2.0f*v.x() / float(this->width()) - 1.0f, 0.0f, -(2.0f*v.y() / float(this->height()) - 1.0f));

		float xzSq = ABvector.x*ABvector.x + ABvector.z*ABvector.z;

		if (xzSq < 1.0f) {
			ABvector.y = sqrt(1.0f - xzSq);
		} else {
			ABvector = glm::normalize(ABvector);

		}
	}

	return ABvector;
}

void CW3View3D::slotUpdateRotate(glm::mat4 *model) {
	m_isPassive = true;
	m_rotate = *model;
	m_model = m_rotate*m_origModel*m_scaleMat;
	m_pRender3DParam->m_isLowRes = true;
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
	RenderAndUpdate();
}

void CW3View3D::slotUpdateScale(float scale) {
	m_scale = scale;
	m_pRender3DParam->m_isLowRes = true;
	RenderAndUpdate();
}

void CW3View3D::slotReoriupdate(glm::mat4 *M) {
	m_origModel = *M;
	m_rotate = glm::mat4(1.0f);
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	setModel();
	RenderAndUpdate();
}

void CW3View3D::slotDrawAirway(int state) {
	switch (state) {
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_pAirway->setVisible(true);
		drawAirwayColorBar();
		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			if (m_lpTextColorBar.at(i))
				m_lpTextColorBar.at(i)->setVisible(true);
		if (m_pAirwayColorBar)
			m_pAirwayColorBar->setVisible(true);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_pAirway->setVisible(false);
		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			if (m_lpTextColorBar.at(i))
				m_lpTextColorBar.at(i)->setVisible(false);
		if (m_pAirwayColorBar)
			m_pAirwayColorBar->setVisible(false);
		break;
	}
	RenderAndUpdate();
}

void CW3View3D::slotTransformedPhotoPoints(glm::mat4 *model) {
	m_isFacePhotoUpdated = true;

	m_pgRcontainer->getFacePhoto3D()->setSRtoVol(*model);
}

void CW3View3D::ResetView() {
	if (!isVisible())
		return;

	if (m_eViewType == common::ViewTypeID::ENDO_MODIFY) {
		m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	} else if (m_eViewType == common::ViewTypeID::ENDO_SAGITTAL) {
		m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f))*glm::rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	} else {
		m_rotate = glm::mat4(1.0f);
	}

	m_scalePre = m_scale;
	CW3View2D::ResetView();

	setModel();

	if (m_pWorldAxisItem) {
		m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
	}

	if (m_is3Dready) {
		if (recon_type_ != common::ReconTypeID::MPR &&
			recon_type_ != common::ReconTypeID::X_RAY)
			render3D();
	}

	m_scalePre = 1.0f;
}

void CW3View3D::FitView() {
	if (!isVisible())
		return;

	m_scalePre = m_scale;
	CW3View2D::FitView();

	if (m_is3Dready) {
		if (recon_type_ != common::ReconTypeID::MPR &&
			recon_type_ != common::ReconTypeID::X_RAY)
			render3D();
	}

	m_scalePre = 1.0f;
}

void CW3View3D::InvertView(bool bToggled) {
	m_bInvertWindowWidth = bToggled;

	changeWLWW();

	if (isVisible())
		RenderAndUpdate();
}

void CW3View3D::setVRAlign() {
	if (!isVisible())
		return;

	setModel();

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
	RenderAndUpdate();
}

void CW3View3D::RenderAndUpdate() {
	render3D();
	scene()->update();
}

void CW3View3D::slotVRAlignS() {
	m_rotate = glm::rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	setVRAlign();
}
void CW3View3D::slotVRAlignI() {
	m_rotate = glm::rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	setVRAlign();
}
void CW3View3D::slotVRAlignL() {
	m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}
void CW3View3D::slotVRAlignR() {
	m_rotate = glm::rotate(glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}
void CW3View3D::slotVRAlignA() {
	m_rotate = mat4(1.0f);
	setVRAlign();
}
void CW3View3D::slotVRAlignP() {
	m_rotate = glm::rotate(glm::radians(180.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}
void CW3View3D::slotSegAirway(std::vector<tri_STL>& mesh) {
	m_nColorMin = std::numeric_limits<unsigned int>::max();
	m_nColorMax = std::numeric_limits<unsigned int>::min();

	m_vVertices.clear();
	m_vVertexNormals.clear();
	m_vIndices.clear();
	m_vVertexColors.clear();

	class Vec3Less {
	public:
		bool operator()(const glm::vec3& v0, const glm::vec3& v1) const {
			return v0.x < v1.x ||
				(v0.x == v1.x && v0.y < v1.y) ||
				(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
		}
	};

	std::map<glm::vec3, unsigned int, Vec3Less> mapVertIdx;
	int indexCount = 0;
	std::vector<unsigned int>	vCountSameVertex;
	std::vector<glm::vec3>	vFaceNormals;

	vec3 volRange = *m_pgVREngine->getVolRange(0);
	float spacing = m_pgVREngine->getVol(0)->pixelSpacing();

	for (int i = 0; i < mesh.size(); i++) {
		const auto& meshAt = mesh.at(i);

		float x[3] = {
			-meshAt.v1.x / spacing / volRange.x * 2.0f - 1.0f,
			-meshAt.v2.x / spacing / volRange.x * 2.0f - 1.0f,
			-meshAt.v3.x / spacing / volRange.x * 2.0f - 1.0f
		};
		float y[3] = {
			meshAt.v1.y / spacing / volRange.y * 2.0f - 1.0f,
			meshAt.v2.y / spacing / volRange.y * 2.0f - 1.0f,
			meshAt.v3.y / spacing / volRange.y * 2.0f - 1.0f
		};
		float z[3] = {
			meshAt.v1.z / spacing / volRange.z * 2.0f - 1.0f,
			meshAt.v2.z / spacing / volRange.z * 2.0f - 1.0f,
			meshAt.v3.z / spacing / volRange.z * 2.0f - 1.0f
		};

		glm::vec3 faceNormal(-meshAt.normal.x, -meshAt.normal.y, -meshAt.normal.z);

		for (int j = 0; j < 3; j++) {
			glm::vec3 vertex = glm::vec3(-x[j], y[j], z[j]); // by jdk 170203 for airway x축 반전
			if (mapVertIdx.find(vertex) == mapVertIdx.end()) {
				mapVertIdx[vertex] = indexCount;
				m_vVertices.push_back(vertex);
				m_vVertexColors.push_back(vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z));
				m_vIndices.push_back(indexCount++);
				vFaceNormals.push_back(faceNormal);
				vCountSameVertex.push_back(1);
			} else {
				unsigned int id = mapVertIdx[vertex];
				m_vIndices.push_back(id);
				vFaceNormals.at(id) += faceNormal;
				vCountSameVertex.at(id)++;
			}
		}

		if (meshAt.nColorVal < m_nColorMin) {
			m_nColorMin = meshAt.nColorVal;
			m_vColorMin = vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z);
		}

		if (meshAt.nColorVal > m_nColorMax) {
			m_nColorMax = meshAt.nColorVal;
			m_vColorMax = vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z);
		}
	}

	for (int i = 0; i < vCountSameVertex.size(); i++) {
		glm::vec3 normal = vFaceNormals.at(i) / float(vCountSameVertex.at(i));

		m_vVertexNormals.push_back(normal);
	}

	m_nCntAirwayVertex = m_vIndices.size();
	m_pRender3DParam->m_pAirway->setNindices(m_nCntAirwayVertex);

	m_pgVREngine->makeCurrent();
	setAirwayVBO();
	m_pgVREngine->doneCurrent();

	if (isVisible() && m_pGLWidget) {
		if (m_pGLWidget->context())
			RenderAndUpdate();
	}
}

void CW3View3D::slotShadeOnFromOTF(bool isShading) {
	// m_pReconTypeTextItem가 있는 view는 OTF의 shading 옵션 적용하지 않음
	if (recon_type_selection_ui_)
		return;

	m_pRender3DParam->m_isShading = isShading;

	if (!m_is3Dready || !isVisible())
		return;

	RenderAndUpdate();
}

void CW3View3D::setMIP(bool isMIP) {
	if (!isReadyRender3D())
		return;

	if (recon_type_ == common::ReconTypeID::MPR ||
		recon_type_ == common::ReconTypeID::X_RAY)
		return;

	if (!recon_type_selection_ui_)
		return;

	if (isMIP)
		recon_type_selection_ui_->setReconType(ui_primitive::kReconFilterMIP);
	else if (m_pRender3DParam->m_isShading)
		recon_type_selection_ui_->setReconType(ui_primitive::kReconFilterVR);
	else
		recon_type_selection_ui_->setReconType(ui_primitive::kReconFilterVRUnshade);
}

inline QPointF CW3View3D::mapVolToScene(const glm::vec3& ptVol) {
	CW3Image3D* vol = m_pgVREngine->getVol(0);

	glm::vec3 v = ptVol - vec3(vol->width()*0.5f, vol->height()*0.5f, vol->depth()*0.5f);
	v.z = -v.z;
	v = (vec3)(glm::inverse(m_rotate*m_origModel)*vec4(v, 1.0f));
	v = (m_scaleVolToGL*v) / m_scaleSceneToGL;

	return QPointF(v.x, v.y) + m_pntCurViewCenterinScene - m_sceneTrans;
}

inline glm::vec3 CW3View3D::mapSceneToVol(const QPointF& ptScene, bool isApplyBackCoord) {
	CW3Image3D* vol = m_pgVREngine->getVol(0);

	QPointF p(ptScene - m_pntCurViewCenterinScene + m_sceneTrans);

	glm::vec3 v(p.x(), p.y(), 0.0f);
	v = v*m_scaleSceneToGL / m_scaleVolToGL;
	v = (vec3)(m_rotate*m_origModel*vec4(v, 1.0f));
	v.z = -v.z;
	v += vec3(vol->width()*0.5f, vol->height()*0.5f, vol->depth()*0.5f);

	if (isApplyBackCoord)
		v += this->getBackCoordInVol();

	return v;
}

void CW3View3D::setAirwayVBO() {
	if (m_vboAirway[0]) {
		glDeleteBuffers(4, m_vboAirway);
		m_vboAirway[0] = 0;
		m_vboAirway[1] = 0;
		m_vboAirway[2] = 0;
		m_vboAirway[3] = 0;
	}

	glGenBuffers(4, m_vboAirway);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[0]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * 3 * sizeof(float), m_vVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[1]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertexNormals.size() * 3 * sizeof(float), m_vVertexNormals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[2]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertexColors.size() * 3 * sizeof(float), m_vVertexColors.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboAirway[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vIndices.size() * sizeof(unsigned int), m_vIndices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3View3D::setAirwayVAO() {
	if (m_pRender3DParam->m_pAirway->getVAO())
		m_pRender3DParam->m_pAirway->clearVAOVBO();

	//// Create the VAO
	GLuint VAO = 0;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[0]);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[1]);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[2]);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboAirway[3]);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_pRender3DParam->m_pAirway->setVAO(VAO);
	m_pRender3DParam->m_pAirway->setNindices(m_nCntAirwayVertex);
}

void CW3View3D::drawAirwayColorBar() {
	int stratColorVal = m_nColorMin / 100;
	int endColorVal = (m_nColorMax / 100) + 1;
	m_nNumColor = abs(endColorVal - stratColorVal);

	float minPosY = (-(-0.75f) + 1.0f) * 0.5f * height();
	float maxPosY = (-(0.75f) + 1.0f) * 0.5f * height();
	float interval = abs(maxPosY - minPosY) / (float)m_nNumColor;

	if (m_pAirwayColorBar) {
		scene()->removeItem(m_pAirwayColorBar);
		SAFE_DELETE_OBJECT(m_pAirwayColorBar);
	}

	m_pAirwayColorBar = scene()->addRect((-0.7f + 1.0f) * 0.5f * width(), minPosY, 0.05f * 0.5f * width(), maxPosY - minPosY);
	m_pAirwayColorBar->setVisible(m_pRender3DParam->m_pAirway->isVisible());

	QRectF rect = m_pAirwayColorBar->rect();
	QLinearGradient gradient(0, rect.top(), 0, rect.bottom());

	QFont font = QApplication::font();
	font.setPixelSize(10);

	for (int i = 0; i < m_lpTextColorBar.size(); i++) {
		if (m_lpTextColorBar.at(i)) {
			scene()->removeItem(m_lpTextColorBar.at(i));
			SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
		}
	}
	m_lpTextColorBar.clear();

	for (int i = 0; i <= m_nNumColor; i++) {
		float pos = (float)i / (float)m_nNumColor;
		QColor color;

		switch (i) {
		case 0:
			color = QColor::fromRgbF(0.25f, 0.1f, 0.1f);
			break;
		case 1:
			color = QColor::fromRgbF(0.1f, 0.0f, 0.0f);
			break;
		case 2:
			color = QColor::fromRgbF(1.0f, 0.0f, 0.0f);
			break;
		case 3:
			color = QColor::fromRgbF(1.0f, 0.5f, 1.0f);
			break;
		case 4:
			color = QColor::fromRgbF(1.0f, 1.0f, 0.0f);
			break;
		case 5:
			color = QColor::fromRgbF(0.0f, 1.0f, 0.0f);
			break;
		case 6:
			color = QColor::fromRgbF(0.0f, 0.0f, 1.0f);
			break;
		case 7:
			color = QColor::fromRgbF(1.0f, 1.0f, 1.0f);
			break;
		default:
			color = QColor::fromRgbF(0.0f, 0.0f, 0.0f);
			break;
		}

		if (i > 7)
			color = QColor::fromRgbF(1.0f, 1.0f, 1.0f);

		gradient.setColorAt(pos, color);

		QGraphicsTextItem *textItem = scene()->addText(QString::number((100 * stratColorVal) + (100 * i)));
		if (i == 0)
			textItem->setPlainText(textItem->toPlainText() + " " + QString::fromLocal8Bit("mm2"));
		textItem->setPos((-0.65f + 1.0f) * 0.5f * width(), (minPosY - (interval * i)) - 15.0f);
		textItem->setFont(font);
		textItem->setDefaultTextColor(Qt::white);
		textItem->setVisible(m_pRender3DParam->m_pAirway->isVisible());
		m_lpTextColorBar.push_back(textItem);
	}

	m_pAirwayColorBar->setBrush(gradient);
}

bool CW3View3D::isReadyRender3D() const {
	if (m_is3Dready && isVisible() && m_pGLWidget && m_pGLWidget->context())
		return true;
	else
		return false;
}
bool CW3View3D::isRender3D() const {
	if (recon_type_ == common::ReconTypeID::VR)
		return true;
	else
		return false;
}

void CW3View3D::HideUI(bool bToggled) {
	CW3View2D::HideUI(bToggled);

	if (m_lpTextAlign[0]) {
		for (int i = 0; i < 6; i++) {
			m_lpTextAlign[i]->setVisible(!hide_all_view_ui_);
		}
	}

	if (recon_type_selection_ui_)
		recon_type_selection_ui_->setVisible(!hide_all_view_ui_);
}

void CW3View3D::HideMeasure(bool toggled)
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(!toggled);

		scene()->update();
	}
}

void CW3View3D::DeleteAllMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->Clear();

		scene()->update();
	}
}

void CW3View3D::setPosReconTypeTextItem() {
	recon_type_selection_ui_->setPos(
		mapToScene(width() - common::ui_define::kViewMarginWidth - recon_type_selection_ui_->sceneBoundingRect().width(),
				   common::ui_define::kViewSpacing * 4.0f));
}

void CW3View3D::CreateReconText() {
	recon_type_selection_ui_ = new CW3FilteredTextItem(QString("MPR"));
	scene()->addItem(recon_type_selection_ui_);
	recon_type_selection_ui_->addText("X-ray");
	recon_type_selection_ui_->addText("VR : Unshaded");
	recon_type_selection_ui_->addText("VR : Shaded");
}

glm::vec3 CW3View3D::getBackCoordInVol() {
	vec4 zCoordInVol = scaledGLToVol(m_backScalarInGL)*m_origBackVector*m_rotate;
	return vec3(zCoordInVol);
}

void CW3View3D::DrawMeasure3D()
{
	if (measure_3d_manager_)
	{
		uint surface_program = m_pgVREngine->getPROGsurface();
		glUseProgram(surface_program);
		WGLSLprogram::setUniform(surface_program, "Light.Intensity", vec3(1.0f));
		vec4 lightPos = glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
		WGLSLprogram::setUniform(surface_program, "Light.Position",
			glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * lightPos);
		glUseProgram(0);

		measure_3d_manager_->set_reorientation_matrix(m_origModel);
		measure_3d_manager_->set_scale_matrix(m_scaleMat);
		measure_3d_manager_->set_rotate_matrix(m_rotate);
		measure_3d_manager_->set_projection_matrix(m_projection);
		measure_3d_manager_->set_view_matrix(m_view);
		measure_3d_manager_->Draw(surface_program);
	}
}

void CW3View3D::DeleteUnfinishedMeasure()
{
	if (measure_3d_manager_)
	{
		measure_3d_manager_->DeleteUnfinishedItem();

		scene()->update();
	}
}

void CW3View3D::keyPressEvent(QKeyEvent* event)
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
		m_rotate = glm::rotate(glm::radians(-1.0f), rotate_axis) * m_rotate;
	}
	else if (event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
		m_rotate = glm::rotate(glm::radians(1.0f), rotate_axis) * m_rotate;
	}

	if (event->key() == Qt::Key_Left ||
		event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Right ||
		event->key() == Qt::Key_Down)
	{
		setModel();
		m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
		RenderAndUpdate();
	}

	CW3View2D::keyPressEvent(event);
}

void CW3View3D::leaveEvent(QEvent* event)
{
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void CW3View3D::enterEvent(QEvent* event)
{
	QGraphicsView::enterEvent(event);
	CW3Cursor::SetViewCursor(common_tool_type_);
	setFocus();
}
