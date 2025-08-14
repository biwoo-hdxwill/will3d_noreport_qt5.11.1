#include "W3View3D_forMPR.h"
/*=========================================================================

File:			class CW3View3D_forMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last date:		2016-06-04

=========================================================================*/
#include <qgraphicsproxywidget.h>
#include <qmath.h>
#include <QApplication>
#include <QOpenGLFramebufferObject>
#include <QDebug>
#include <QMenu>
#include <QAction>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"

#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3Implant.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/Resource/nerve_resource.h"

#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>
#include "../UIGLObjects/W3GLNerve.h"
#include "../UIGLObjects/W3GLObject.h"
#include "../UIPrimitive/W3FilteredTextItem.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3TextItem_ImplantID.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3ActiveBlock.h"
#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VREngine.h"

namespace
{
	// TODO smseo : 볼륨 좌표를 넣기 전 임시 값
	const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);

	class Vec3Less
	{
	public:
		bool operator()(const glm::vec3 &v0, const glm::vec3 &v1) const
		{
			return v0.x < v1.x || (v0.x == v1.x && v0.y < v1.y) ||
				(v0.x == v1.x && v0.y == v1.y && v0.z < v1.z);
		}
	};

	QColor GetAirwayColor(int i)
	{
		switch (i)
		{
		case 0:
			return QColor::fromRgbF(0.25f, 0.1f, 0.1f);
		case 1:
			return QColor::fromRgbF(0.1f, 0.0f, 0.0f);
		case 2:
			return QColor::fromRgbF(1.0f, 0.0f, 0.0f);
		case 3:
			return QColor::fromRgbF(1.0f, 0.5f, 1.0f);
		case 4:
			return QColor::fromRgbF(1.0f, 1.0f, 0.0f);
		case 5:
			return QColor::fromRgbF(0.0f, 1.0f, 0.0f);
		case 6:
			return QColor::fromRgbF(0.0f, 0.0f, 1.0f);
		case 7:
		default:
			return QColor::fromRgbF(1.0f, 1.0f, 1.0f);
		}
	}
}  // namespace

using namespace NW3Render3DParam;

CW3View3D_forMPR::CW3View3D_forMPR(CW3VREngine *VREngine,
	CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer,
	common::ViewTypeID eType, QWidget *pParent)
	: CW3View2D_forMPR(VREngine, MPRengine, Rcontainer, eType, pParent)
{
	recon_type_ = common::ReconTypeID::VR;

	m_pWorldAxisItem = new ViewNavigatorItem();
	scene()->addItem(m_pWorldAxisItem);
	m_pWorldAxisItem->setVisible(false);
	m_viewForFinal = m_projForFinal = glm::mat4(1.0f);

	m_ToDepth = glm::mat4(1.0f);
	m_ToDepth[2].z = 0.5f;
	m_ToDepth[3].z = 0.5f;

	if (m_eViewType == common::ViewTypeID::MPR_3D ||
		m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
	{
		for (int i = 0; i < 6; i++)
		{
			m_lpTextAlign[i] = new CW3TextItem();
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

	m_vboAirway[0] = 0;
	m_vboAirway[1] = 0;
	m_vboAirway[2] = 0;
	m_vboAirway[3] = 0;

	m_nColorMin = 0;
	m_nColorMax = 0;

	m_rotate = glm::mat4(1.0f);

	if (m_pSlider) m_pSlider->setInvertedAppearance(true);

	measure_tools_->SetViewRenderMode(common::ReconTypeID::VR);

	connections();

	menu_.reset(new QMenu());
	action_save_3d_face_to_ply_.reset(new QAction("Export 3d face to PLY file."));
	action_save_3d_face_to_obj_.reset(new QAction("Export 3d face to OBJ file."));
	menu_->addAction(action_save_3d_face_to_ply_.get());
	menu_->addAction(action_save_3d_face_to_obj_.get());
	connect(action_save_3d_face_to_ply_.get(), SIGNAL(triggered()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(action_save_3d_face_to_obj_.get(), SIGNAL(triggered()), this, SIGNAL(sigSave3DFaceToOBJFile()));
}

CW3View3D_forMPR::~CW3View3D_forMPR(void)
{
	if (m_pGLWidget && m_pGLWidget->context()) this->clearGL();

	if (m_lpTextAlign[0])
		for (int i = 0; i < 6; i++) SAFE_DELETE_OBJECT(m_lpTextAlign[i]);

	SAFE_DELETE_OBJECT(m_pAirwayColorBar);

	for (int i = 0; i < m_lpTextColorBar.size(); i++)
		SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
	m_lpTextColorBar.clear();

	SAFE_DELETE_OBJECT(ui_recon_type_);
	SAFE_DELETE_OBJECT(measure_3d_manager_);
}

void CW3View3D_forMPR::clearGL()
{
	CW3View2D_forMPR::clearGL();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->ClearVAOVBO();
	}
}

void CW3View3D_forMPR::reset()
{
	CW3View2D_forMPR::reset();

	m_is3Dready = false;
	m_camFOV = 1.0f;
	m_rotAxis = vec3(1.0f, 0.0f, 0.0f);
	m_viewForFinal = m_projForFinal = glm::mat4(1.0f);

	m_pRender3DParam->m_isClipped = false;

	recon_type_ = common::ReconTypeID::VR;

	m_ToDepth = glm::mat4(1.0f);
	m_ToDepth[2].z = 0.5f;
	m_ToDepth[3].z = 0.5f;

	if (m_pGLWidget)
	{
		m_pGLWidget->makeCurrent();
		if (m_vboAirway[0])
		{
			glDeleteBuffers(4, m_vboAirway);
			m_vboAirway[0] = 0;
			m_vboAirway[1] = 0;
			m_vboAirway[2] = 0;
			m_vboAirway[3] = 0;
		}
		m_pGLWidget->doneCurrent();
	}

	if (m_eViewType == common::ViewTypeID::MPR_3D)
	{
		SAFE_DELETE_OBJECT(m_pAirwayColorBar);

		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
		m_lpTextColorBar.clear();

		m_nColorMin = 0;
		m_nColorMax = 0;
	}

	m_rotate = glm::mat4(1.0f);
	m_isPassive = false;

	m_pRender3DParam->m_isPerspective = false;
	m_pRender3DParam->m_isShading = true;

	m_origBackVector = glm::vec4(
		0.0f, 0.0f, 1.0f, 0.0f);  // ??? 왜 init 때랑 값이 다르지?
								  // m_origBackVector = glm::vec4(0.0f, -1.0f,
								  // 0.0f, 0.0f); // 원래 init 시의 값
}

void CW3View3D_forMPR::connections()
{
	if (m_lpTextAlign[0])
	{
		connect(m_lpTextAlign[0], SIGNAL(sigPressed()), this, SLOT(slotVRAlignA()));
		connect(m_lpTextAlign[1], SIGNAL(sigPressed()), this, SLOT(slotVRAlignP()));
		connect(m_lpTextAlign[2], SIGNAL(sigPressed()), this, SLOT(slotVRAlignL()));
		connect(m_lpTextAlign[3], SIGNAL(sigPressed()), this, SLOT(slotVRAlignR()));
		connect(m_lpTextAlign[4], SIGNAL(sigPressed()), this, SLOT(slotVRAlignI()));
		connect(m_lpTextAlign[5], SIGNAL(sigPressed()), this, SLOT(slotVRAlignS()));
	}
}

void CW3View3D_forMPR::setInitScale() 
{
	m_initScale = sqrt(5.0f);
	m_scale = m_initScale;
}

void CW3View3D_forMPR::init()
{
	SetMeasure3Dmode();

	m_pRender3DParam->m_pgMainVolume[0] = m_pgVREngine->getVRparams(0);
	m_pRender3DParam->m_pgMainVolume[0]->m_isShown = true;

	m_vVolRange = *m_pgMPRengine->getVolRange(0);
	m_scaleMat = glm::scale(m_vVolRange);
	m_inverseScale = glm::scale(1.0f / m_vVolRange);

	if (m_pRender3DParam->m_photo3D->isVisible())
		m_modelPhotoForTexture = m_inverseScale *
		m_pgRcontainer->getFacePhoto3D()->getSRtoVol() *
		m_scaleMat;

	m_origModel = *m_pgVREngine->getReorientedModel();

	m_camFOV = glm::length(m_vVolRange) * sqrt(2.0f);

	uint surface_program = m_pgVREngine->getPROGsurface();
	glUseProgram(surface_program);
	WGLSLprogram::setUniform(surface_program, "Light.Intensity", vec3(1.0f));
	vec4 lightPos = glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
	WGLSLprogram::setUniform(surface_program, "Light.Position",
		glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * lightPos);
	glUseProgram(0);

	m_pRender3DParam->m_pgMainVolume[0]->m_VolScaleIso = m_vVolRange;

	setInitScale();
	setViewMatrix();
	setProjection();

	m_model = m_rotate * m_origModel * m_scaleMat;

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

	m_viewForFinal =
		glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f));

	m_projForFinal = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);

	m_pRender3DParam->m_plane->setMVP(glm::mat4(1.0f), m_viewForFinal,
		m_projForFinal);
	m_is3Dready = true;

	m_pWorldAxisItem->setVisible(!hide_all_view_ui_);
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	if (ruler_)
	{
		ruler_->setVisible(!hide_all_view_ui_ && show_rulers_);
	}

	if (m_lpTextAlign[0])
	{
		for (int i = 0; i < 6; i++)
			m_lpTextAlign[i]->setVisible(!hide_all_view_ui_);
	}

	if (m_pSlider) m_pSlider->setRange(-m_vVolRange.z, m_vVolRange.z);

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

void CW3View3D_forMPR::slotTFupdated(bool isMinMaxChanged)
{
	if (!isVisible()) return;

	if (m_is3Dready && isMinMaxChanged)
	{
		m_pGLWidget->makeCurrent();
		if (m_isDrawBoth)
		{
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
		}
		else
		{
			m_pgVREngine->setActiveIndex(
				&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
		}

		m_pGLWidget->doneCurrent();
	}

	if (Is3DRenderingMode())
	{
		if (m_is3Dready) m_pRender3DParam->m_isLowRes = true;

		RenderAndUpdate();
	}
}

void CW3View3D_forMPR::slotTFupdateCompleted()
{
	if (!isVisible()) return;

	m_pRender3DParam->m_isLowRes = false;
	RenderAndUpdate();
}

void CW3View3D_forMPR::render3D()
{
	if (!isReadyRender3D() || !m_pGLWidget || !m_pGLWidget->context())
	{
		return;
	}

	m_pGLWidget->makeCurrent();
	setProjection();
	setMVP();

	///////////////////////////////////////////////////////////
	// ready for photo
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_photo3D->isVisible())
	{
		if (m_isFacePhotoUpdated)
		{
			if (m_pRender3DParam->m_photo3D->getVAO())
				m_pRender3DParam->m_photo3D->clearVAOVBO();

			m_isFacePhotoUpdated = false;
		}

		if (!m_pRender3DParam->m_photo3D->getVAO())
		{
			m_pRender3DParam->m_photo3D->setNindices(
				m_pgRcontainer->getFacePhoto3D()->getNindices());

			unsigned int vao = 0;
			CW3GLFunctions::initVAOSR(&vao,
				m_pgRcontainer->getFacePhoto3D()->getVBO());
			m_pRender3DParam->m_photo3D->setVAO(vao);
			m_pRender3DParam->m_photo3D->setTexHandler(
				m_pgRcontainer->getFacePhoto3D()->getTexHandler());
		}

		if (m_pRender3DParam->m_photo3D->getVAO())
		{
			setModelPhotoToMC();

			m_pRender3DParam->m_photo3D->setInvModel(m_modelPhotoForTexture);
			m_pRender3DParam->m_photo3D->setMVP(m_modelPhoto, m_view, m_projection);
		}
	}

	///////////////////////////////////////////////////////////
	// ready for nerve
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_pNerve->isVisible())
	{
		if (!m_pRender3DParam->m_pNerve->getVAO())
		{
			setNerveVAOVBO();
		}

		m_pRender3DParam->m_pNerve->setInvModel(m_inverseScale);
		m_pRender3DParam->m_pNerve->setMVP(m_model * m_inverseScale, m_view,
			m_projection);
	}

	///////////////////////////////////////////////////////////
	// ready for airway
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_pAirway->isVisible())
	{
		setAirwayVAO();
		m_pRender3DParam->m_pAirway->setInvModel(m_inverseScale);
		m_pRender3DParam->m_pAirway->setMVP(m_model, m_view, m_projection);
	}

	///////////////////////////////////////////////////////////
	// ready for implant
	///////////////////////////////////////////////////////////
	if (m_pRender3DParam->m_isImplantShown)
	{
		const auto &res_implant =
			ResourceContainer::GetInstance()->GetImplantResource();
		if (res_implant.IsSetImplant())
		{
			const auto &datas = res_implant.data();
			for (int i = 0; i < MAX_IMPLANT; i++)
			{
				m_pRender3DParam->g_is_implant_exist_[i] = false;

				auto data = datas.find(kImplantNumbers[i]);
				if (data == datas.end()) continue;

				if (!m_pRender3DParam->m_pImplant[i]->getVAO())
				{
					setVAOVBOImplant(i);
				}
				glm::mat4 model =
					data->second->translate_in_vol() * data->second->rotate_in_vol();

				glm::mat4 model_to_texture = glm::scale(glm::vec3(0.5f)) *
					glm::translate(glm::vec3(1.0f)) *
					glm::scale(glm::vec3(-1.0f, 1.0f, 1.0f)) *
					glm::scale(1.0f / m_vVolRange) * model;

				m_pRender3DParam->m_pImplant[i]->setMVP(
					model, m_view * m_rotate * m_origModel, m_projection);
				m_pRender3DParam->m_pImplant[i]->setInvModel(m_inverseScale * model);
				m_pRender3DParam->m_pImplant[i]->setVisible(true);
				m_pRender3DParam->g_is_implant_exist_[i] = true;
				m_pRender3DParam->g_is_implant_collided_[i] = data->second->is_collide();
			}
		}
	}

	///////////////////////////////////////////////////////////
	// ready for raycasting
	///////////////////////////////////////////////////////////
	m_pRender3DParam->m_width = this->width();
	m_pRender3DParam->m_height = this->height();

	if (m_drawVolId == 0)
	{
		m_pRender3DParam->m_pgMainVolume[0]->m_mvp = &m_mvp;
	}
	else
	{
		m_pRender3DParam->m_pgMainVolume[1]->m_mvp = &m_mvpSecond;
	}

	mat4 invModel(1.0f);
	m_pRender3DParam->m_pgMainVolume[0]->m_invModel = &invModel;

	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);

	WGLSLprogram::setUniform(PROGRaycasting, "useSegTmj", false);

#if 0
	if (m_listObjectFlag.size() > 0 && m_listObjectPara.size() > 0)
	{
		WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", true);

		glm::ivec3 vObjectFlag[32];
		for (int i = 0; i < m_listObjectFlag.size(); i++)
			vObjectFlag[i] = m_listObjectFlag.at(i);

		WGLSLprogram::setUniform(PROGRaycasting, "ObjectFlag", m_listObjectFlag.size(), vObjectFlag);

		glm::vec4 vObjectPara[256];
		for (int i = 0; i < m_listObjectPara.size(); i++)
			vObjectPara[i] = m_listObjectPara.at(i);

		WGLSLprogram::setUniform(PROGRaycasting, "ObjectPara", m_listObjectPara.size(), vObjectPara);
	}
	else
	{
		WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", false);
	}
#endif

	if (!m_pRender3DParam->m_plane->getVAO())
	{
		m_pRender3DParam->m_plane->clearVAOVBO();

		unsigned int vao = 0;
		m_pgVREngine->initVAOplane(&vao);
		m_pRender3DParam->m_plane->setVAO(vao);
		m_pRender3DParam->m_plane->setNindices(6);
	}

#if 0
	if (!m_pRender3DParam->m_mainVolume_vao[0])
	{
		if (m_isDrawBoth)
		{
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);

			if (m_pRender3DParam->m_mainVolume_vao[1])
			{
				glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[1]);
				m_pRender3DParam->m_mainVolume_vao[1] = 0;
			}

			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
		}
		else
		{
			if (m_pRender3DParam->m_mainVolume_vao[m_drawVolId])
			{
				glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[m_drawVolId]);
				m_pRender3DParam->m_mainVolume_vao[m_drawVolId] = 0;
			}

			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);
		}
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
	// render
	///////////////////////////////////////////////////////////
	if (m_isDrawBoth)
	{
		m_pRender3DParam->m_pgMainVolume[0]->m_matFirstHitToDepth =
			&m_matTexCoordToDepth;

		m_pRender3DParam->m_pgMainVolume[1]->m_mvp = &m_mvpSecond;
		m_pRender3DParam->m_pgMainVolume[1]->m_matFirstHitToDepth =
			&m_matTexCoordToDepthSecond;

		m_pgVREngine->Render3Dboth(m_pRender3DParam, 0, m_isReconSwitched, true);
	}
	else
	{
		m_pgVREngine->Render3Dboth(m_pRender3DParam, m_drawVolId,
			m_isReconSwitched);
	}

	m_pGLWidget->doneCurrent();
}
void CW3View3D_forMPR::drawBackground(QPainter *painter, const QRectF &rect)
{
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();

	if (m_pgVREngine->isVRready())
	{
		if (!m_is3Dready)
		{
			init();
			m_pGLWidget->doneCurrent();
			render3D();
			m_pGLWidget->makeCurrent();
		}

		m_pgVREngine->Render3Dfinal(m_pRender3DParam);

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
	else
	{
		CW3GLFunctions::clearView(false);
	}

	painter->endNativePainting();
}

void CW3View3D_forMPR::setMVP()
{
	m_mvp = m_projection * m_view * m_model;

	if (m_pRender3DParam->m_pgMainVolume[0])
	{
		m_matTexCoordToDepth = m_ToDepth * m_mvp * m_matTextureToGL;
		m_pRender3DParam->m_pgMainVolume[0]->m_matFirstHitToDepth =
			&m_matTexCoordToDepth;
	}

	if (m_pRender3DParam->m_pgMainVolume[1])
	{
		m_mvpSecond = m_projection * m_view * m_modelSecond;

		m_matTexCoordToDepthSecond = m_ToDepth * m_mvpSecond * m_matTextureToGL;
		m_pRender3DParam->m_pgMainVolume[1]->m_matFirstHitToDepth =
			&m_matTexCoordToDepthSecond;
	}
}

void CW3View3D_forMPR::setViewMatrix()
{
	if (m_pRender3DParam->m_isPerspective)
	{
		m_view =
			glm::lookAt(glm::vec3(0.0f, -m_camFOV * 3.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}
	else
	{
		m_view =
			glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f),
				glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	if (m_pgVREngine->getVRparams(0))
	{
		m_pRender3DParam->m_lightInfo.Intensity = vec3(1.0f);
		m_pRender3DParam->m_lightInfo.Position =
			m_view * glm::scale(m_vVolRange) * vec4(0.0f, -10.0f, 0.0f, 1.0f);
	}
}

void CW3View3D_forMPR::setProjection()
{
	float left, right, m_top, m_bottom;
	if (m_sceneWinView > m_sceneHinView)
	{
		float ratio = m_sceneWinView / m_sceneHinView * m_camFOV;

		left = -ratio / m_scale + m_WglTrans;
		right = ratio / m_scale + m_WglTrans;
		m_bottom = -m_camFOV / m_scale - m_HglTrans;
		m_top = m_camFOV / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneHinView;
	}
	else
	{
		float ratio = m_sceneHinView / m_sceneWinView * m_camFOV;

		left = -m_camFOV / m_scale + m_WglTrans;
		right = m_camFOV / m_scale + m_WglTrans;
		m_bottom = -ratio / m_scale - m_HglTrans;
		m_top = ratio / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_camFOV / m_scale / m_sceneWinView;
	}

	if (m_pRender3DParam->m_isPerspective)
	{
		m_projection = glm::frustum(left * 0.5f, right * 0.5f, m_bottom * 0.5f,
			m_top * 0.5f, m_camFOV * 2.0f, m_camFOV * 4.0f);
	}
	else
	{
		m_projection =
			glm::ortho(left, right, m_bottom, m_top, 0.0f, m_camFOV * 2.0f);
	}

	measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);

	setViewRulerValue();
	SetGridValue();
}

void CW3View3D_forMPR::resizeEvent(QResizeEvent *pEvent)
{
	if (!isVisible()) return;

	CW3View2D_forMPR::resizeEvent(pEvent);
	ResizeAlignTexts();
	ResizeAirwayUIs();

	if (Is3DRenderingMode() && m_pGLWidget && m_pGLWidget->context()) 
		render3D();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

void CW3View3D_forMPR::resizeScene()
{
	if (!isVisible())
	{
		return;
	}

	CW3View2D_forMPR::resizeScene();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetSceneSizeInView(QSizeF(m_sceneWinView, m_sceneHinView));
	}
}

void CW3View3D_forMPR::mousePressEvent(QMouseEvent *event)
{
	if (Is3DRenderingMode())
	{
		Qt::MouseButton button = event->button();

		if (button == Qt::LeftButton)
		{
			if (measure_3d_manager_ &&
				(common_tool_type_ == CommonToolTypeOnOff::M_RULER ||
					common_tool_type_ == CommonToolTypeOnOff::M_ANGLE ||
					common_tool_type_ == CommonToolTypeOnOff::M_LINE ||
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
		}

		is_update_needed_ = false;
		last_view_pos_ = event->pos();
		last_scene_pos_ = mapToScene(last_view_pos_);

		QGraphicsView::mousePressEvent(event);

		QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
			event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);

		if (button == Qt::RightButton)
		{
			CW3Cursor::SetViewCursor(CommonToolTypeOnOff::NONE);
		}

		if (button == Qt::LeftButton)
		{
			if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_))
			{
				measure_tools_->ProcessMousePressed(last_scene_pos_, kTempPos);
			}
		}

		if (button == Qt::RightButton && m_pgRcontainer->getFacePhoto3D() && m_pRender3DParam->m_photo3D->isVisible())
		{
			show_export_3d_face_menu_ = true;
		}
	}
	else
	{
		CW3View2D_forMPR::mousePressEvent(event);
	}
}

void CW3View3D_forMPR::mouseMoveEvent(QMouseEvent *event)
{
	show_export_3d_face_menu_ = false;

	if (Is3DRenderingMode())
	{
		QGraphicsView::mouseMoveEvent(event);
		Qt::MouseButtons buttons = event->buttons();

		CW3Cursor::SetViewCursor(common_tool_type_);
		if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_))
		{
			if (measure_tools_->IsDrawing())
			{
				measure_tools_->ProcessMouseMove(mapToScene(event->pos()), kTempPos);
				return;
			}
		}

		if (measure_3d_manager_ &&
			common_tool_type_ < CommonToolTypeOnOff::V_LIGHT)
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
			else if (event->buttons() != Qt::RightButton)
			{
				m_pGLWidget->makeCurrent();
				vec3 volume_pos;
				bool volume_picked = m_pgVREngine->VolumeTracking(m_pRender3DParam, event->pos(), volume_pos);
				m_pGLWidget->doneCurrent();

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

		if (buttons == Qt::NoButton)
		{
			DrawImplantID(event->pos());
		}
		else if (buttons == Qt::RightButton)
		{
			RotateViewWithArcBall(event->pos());
			is_update_needed_ = true;
		}
		else
		{
			is_update_needed_ = ToolTypeInteractions3DInMove(event->pos());
		}

		if (is_update_needed_)
		{
			RenderAndUpdate();
			is_update_needed_ = false;
		}
	}
	else
	{
		CW3View2D_forMPR::mouseMoveEvent(event);
	}
}

void CW3View3D_forMPR::mouseReleaseEvent(QMouseEvent *event)
{
	if (Is3DRenderingMode())
	{
		bool measure_selected = measure_tools_->IsSelected();

		QGraphicsView::mouseReleaseEvent(event);

		if (common_tool_type_ == CommonToolTypeOnOff::NONE &&
			!measure_selected &&
			!IsControllerTextOnMousePress() &&
			event->button() == Qt::LeftButton)
		{
			m_pGLWidget->makeCurrent();
			vec3 volume_pos;
			bool volume_picked = m_pgVREngine->VolumeTracking(m_pRender3DParam, event->pos(), volume_pos);
			m_pGLWidget->doneCurrent();

			QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
			bool volume_tracking = settings.value("MPR/volume_tracking", false).toBool();
			if (volume_tracking && volume_picked)
			{
				emit sigVolumeClicked(volume_pos);
			}
		}

		bool is_current_measure_available = measure_tools_->IsMeasureInteractionAvailable(common_tool_type_);

		QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(), event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);

		if (IsLabelSelected()) return;

		Qt::MouseButton button = event->button();
		if (is_current_measure_available/* ||
			(button == Qt::LeftButton && common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW)*/)
		{
			CW3Cursor::SetViewCursor(common_tool_type_);
			measure_tools_->ProcessMouseReleased(button, mapToScene(event->pos()), kTempPos);
		}

		m_pRender3DParam->m_isLowRes = false;
		if (m_pRender3DParam->m_isLowResDrawn || is_update_needed_)
		{
			RenderAndUpdate();
		}

		if (show_export_3d_face_menu_)
		{
			menu_->popup(mapToGlobal(event->pos()));
		}
	}
	else
	{
		CW3View2D_forMPR::mouseReleaseEvent(event);
	}
}

void CW3View3D_forMPR::wheelEvent(QWheelEvent *event)
{
	float dist = -event->delta() / 5.0f;

	m_pRender3DParam->m_isLowRes = true;
	QPoint curr_view_pos(last_view_pos_.x() + dist, last_view_pos_.y() + dist);
	ScaleView(curr_view_pos);

	RenderAndUpdate();
}

void CW3View3D_forMPR::setModel(float rotAngle, glm::vec3 rotAxis)
{
	if (!m_isPassive)
	{
		m_rotate = glm::rotate(glm::radians(rotAngle), rotAxis) * m_rotate;
		m_model = m_rotate * m_origModel * m_scaleMat;
		m_modelSecond = m_rotate * m_translateSecond * m_rotateSecond *
			m_origModel * m_secondToFirstModel * m_scaleMatSecond;
	}
}

void CW3View3D_forMPR::setModel()
{
	m_model = m_rotate * m_origModel * m_scaleMat;
	m_modelSecond = m_rotate * m_translateSecond * m_rotateSecond * m_origModel *
		m_secondToFirstModel * m_scaleMatSecond;
}

void CW3View3D_forMPR::setDirectionFromCompass(glm::mat4 &T)
{
	m_rotate = glm::inverse(m_view) * T;
	m_rotate[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	setModel();
}

void CW3View3D_forMPR::setVisible(bool isVisible)
{
	CW3View2D_forMPR::setVisible(isVisible);

	if (isVisible && m_is3Dready && Is3DRenderingMode()) render3D();
}
void CW3View3D_forMPR::setModelPhotoToMC()
{
	m_modelPhoto = m_rotate * m_origModel *
		m_pgRcontainer->getFacePhoto3D()->getSRtoVol() * m_scaleMat;
}

void CW3View3D_forMPR::ArcBallRotate(const QPoint &curr_view_pos)
{
	glm::vec3 v1 = ArcBallVector(QPointF(last_view_pos_));
	glm::vec3 v2 = ArcBallVector(QPointF(curr_view_pos));

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

	setModel(arcball_rotate_angle, m_rotAxis);
	last_view_pos_ = curr_view_pos;
}

glm::vec3 CW3View3D_forMPR::ArcBallVector(QPointF &v)
{
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

void CW3View3D_forMPR::slotUpdateRotate(glm::mat4 *model)
{
	m_isPassive = true;
	m_rotate = *model;
	m_model = m_rotate * m_origModel * m_scaleMat;
	m_pRender3DParam->m_isLowRes = true;

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
	RenderAndUpdate();
}

void CW3View3D_forMPR::slotUpdateScale(float scale)
{
	m_scale = scale;
	m_pRender3DParam->m_isLowRes = true;
	RenderAndUpdate();
}

void CW3View3D_forMPR::slotReoriupdate(glm::mat4 *M)
{
	m_origModel = *M;
	m_rotate = glm::mat4(1.0f);

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	setModel();
	RenderAndUpdate();
}

void CW3View3D_forMPR::VisibleNerve(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_pNerve->setVisible(true);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_pNerve->setVisible(false);
		break;
	}
	RenderAndUpdate();
}

void CW3View3D_forMPR::VisibleImplant(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_isImplantShown = true;
		m_pRender3DParam->g_is_implant_exist_ = m_pgRcontainer->getImplantThereContainer();
		m_pRender3DParam->g_is_implant_collided_ = m_pgRcontainer->getCollisionContainer();
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_isImplantShown = false;
		break;
	}
	RenderAndUpdate();
}

void CW3View3D_forMPR::VisibleFace(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		if (!m_pgRcontainer || !m_pgRcontainer->getFacePhoto3D()) return;

		m_modelPhotoForTexture = m_inverseScale *
			m_pgRcontainer->getFacePhoto3D()->getSRtoVol() *
			m_scaleMat;
		m_pRender3DParam->m_photo3D->setVisible(true);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_photo3D->setVisible(false);
		break;
	}
	RenderAndUpdate();
}

void CW3View3D_forMPR::VisibleSecond(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
	{
		auto *res_container = ResourceContainer::GetInstance();
		m_isDrawBoth = true;
		m_pRender3DParam->m_pgMainVolume[1] = m_pgVREngine->getVRparams(1);
		float main_vol_pixel_size =
			res_container->GetMainVolume().GetBasePixelSize();
		float second_vol_pixel_size =
			res_container->GetSecondVolume().GetBasePixelSize();
		m_scaleMatSecond =
			glm::scale(*m_pgVREngine->getVolRange(1) * second_vol_pixel_size /
				main_vol_pixel_size);
		m_secondToFirstModel =
			res_container->GetSecondVolume().getSecondToFirst();
		m_modelSecond = m_rotate * m_translateSecond * m_rotateSecond *
			m_origModel * m_secondToFirstModel * m_scaleMatSecond;

		if (m_pGLWidget == nullptr) return;

		m_pGLWidget->makeCurrent();
		m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[1], 1);
		m_pGLWidget->doneCurrent();

		RenderAndUpdate();
		break;
	}
	case Qt::CheckState::Unchecked:
		m_isDrawBoth = false;
		RenderAndUpdate();
		break;
	}
}

void CW3View3D_forMPR::VisibleAirway(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_pAirway->setVisible(true);
		drawAirwayColorBar();
		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			m_lpTextColorBar.at(i)->setVisible(true);
		if (m_pAirwayColorBar) m_pAirwayColorBar->setVisible(true);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_pAirway->setVisible(false);
		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			m_lpTextColorBar.at(i)->setVisible(false);
		if (m_pAirwayColorBar) m_pAirwayColorBar->setVisible(false);
		break;
	}
	RenderAndUpdate();
}

void CW3View3D_forMPR::slotTransformedPhotoPoints(glm::mat4 *model)
{
	m_isFacePhotoUpdated = true;

	m_pgRcontainer->getFacePhoto3D()->setSRtoVol(*model);
}

void CW3View3D_forMPR::slotImplantUpdated(int selectedImplantID)
{
	RenderAndUpdate();
}

void CW3View3D_forMPR::ResetView()
{
	if (!isVisible()) return;

	m_rotate = glm::mat4(1.0f);
	m_scalePre = m_scale;
	CW3View2D_forMPR::ResetView();

	setModel();

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	if (m_is3Dready)
	{
		if (recon_type_ != common::ReconTypeID::MPR &&
			recon_type_ != common::ReconTypeID::X_RAY)
			render3D();
	}

	m_scalePre = 1.0f;
}

void CW3View3D_forMPR::FitView()
{
	if (!isVisible()) return;

	m_scalePre = m_scale;
	CW3View2D_forMPR::FitView();

	if (m_is3Dready)
	{
		if (recon_type_ != common::ReconTypeID::MPR &&
			recon_type_ != common::ReconTypeID::X_RAY)
			render3D();
	}

	m_scalePre = 1.0f;
}

void CW3View3D_forMPR::InvertView(bool bToggled)
{
	invert_windowing_ = bToggled;

	changeWLWW();

	if (isVisible()) RenderAndUpdate();
}

void CW3View3D_forMPR::initClipValues(const MPRClipID clipPlane,
	const bool isClipping, const bool isFlip,
	const int lower, const int upper)
{
	m_pRender3DParam->m_isClipped = isClipping;
	if (clipPlane == MPRClipID::MPROVERLAY)
	{
		return;
	}

	m_pRender3DParam->m_clipPlanes.clear();

	vec3 norm;
	switch (clipPlane)
	{
	case MPRClipID::AXIAL:
		norm = mat3(m_origModel) * vec3(0.0f, 0.0f, -1.0f);
		break;
	case MPRClipID::CORONAL:
		norm = mat3(m_origModel) * vec3(0.0f, 1.0f, 0.0f);
		break;
	case MPRClipID::SAGITTAL:
		norm = mat3(m_origModel) * vec3(1.0f, 0.0f, 0.0f);
		break;
	}

	float ld = ((static_cast<float>(lower) / 100.0f) - 0.5f) * 2.0f;
	float ud = -((static_cast<float>(upper) / 100.0f) - 0.5f) * 2.0f;

	m_pRender3DParam->m_clipPlanes.push_back(vec4(-norm, ld));
	m_pRender3DParam->m_clipPlanes.push_back(vec4(norm, ud));
}

void CW3View3D_forMPR::ClipRangeMove(int lower, int upper)
{
	float ld = ((static_cast<float>(lower) / 100.0f) - 0.5f) * 2.0f;
	float ud = -((static_cast<float>(upper) / 100.0f) - 0.5f) * 2.0f;

	m_pRender3DParam->m_clipPlanes[LOWER] =
		(vec4(vec3(m_pRender3DParam->m_clipPlanes[LOWER]), ld));
	m_pRender3DParam->m_clipPlanes[UPPER] =
		(vec4(vec3(m_pRender3DParam->m_clipPlanes[UPPER]), ud));

	if (m_is3Dready && m_pRender3DParam->m_isClipped)
	{
		m_pRender3DParam->m_isLowRes = true;
		RenderAndUpdate();
	}
}

void CW3View3D_forMPR::ClipRangeSet()
{
	if (m_is3Dready && m_pRender3DParam->m_isClipped)
	{
		m_pRender3DParam->m_isLowRes = false;
		RenderAndUpdate();
	}
}

void CW3View3D_forMPR::ClipPlaneChanged(const MPRClipID &clip_plane)
{
	float ld = m_pRender3DParam->m_clipPlanes[LOWER].w;
	float ud = m_pRender3DParam->m_clipPlanes[UPPER].w;
	glm::vec3 norm;
	switch (clip_plane)
	{
	case MPRClipID::AXIAL:
		norm = mat3(m_origModel) * vec3(0.0f, 0.0f, -1.0f);
		break;
	case MPRClipID::CORONAL:
		norm = mat3(m_origModel) * vec3(0.0f, 1.0f, 0.0f);
		break;
	case MPRClipID::SAGITTAL:
		norm = mat3(m_origModel) * vec3(1.0f, 0.0f, 0.0f);
		break;
	default:
		break;
	}

	m_pRender3DParam->m_clipPlanes[LOWER] = vec4(-norm, ld);
	m_pRender3DParam->m_clipPlanes[UPPER] = vec4(norm, ud);

	ClipRangeSet();
}

void CW3View3D_forMPR::ClipEnable(int isEnable)
{
	m_pRender3DParam->m_isClipped = static_cast<bool>(isEnable);
	RenderAndUpdate();
}

void CW3View3D_forMPR::ChangeFaceTransparency(int value)
{
	if (value == 100)
	{
		m_pRender3DParam->m_photo3D->setTransparent(false);
		m_pRender3DParam->m_photo3D->setVisible(true);
	}
	else if (value == 0)
	{
		m_pRender3DParam->m_photo3D->setVisible(false);
	}
	else
	{
		m_pRender3DParam->m_photo3D->setTransparent(true);
		m_pRender3DParam->m_photo3D->setVisible(true);
		m_pRender3DParam->m_photo3D->setAlpha(pow(float(value) / 100.0f, 0.5f));
	}
	RenderAndUpdate();
}

void CW3View3D_forMPR::setVRAlign()
{
	if (!isVisible()) return;

	setModel();

	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);

	RenderAndUpdate();
}

void CW3View3D_forMPR::RenderAndUpdate()
{
	render3D();
	scene()->update();
}

void CW3View3D_forMPR::RotateViewWithArcBall(const QPoint &curr_view_pos)
{
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
	m_pRender3DParam->m_isLowRes = true;
	ArcBallRotate(curr_view_pos);
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
}

void CW3View3D_forMPR::DrawImplantID(const QPoint &curr_view_pos)
{
	if (!m_pRender3DParam->m_isImplantShown) return;

	// int hovered_id = pickingImplant(curr_view_pos, 3);
	// implant_id_text_->set_hovered_id(hovered_id);
	// if (hovered_id >= MAX_IMPLANT || hovered_id < 0) {
	//	implant_id_text_->set_hovered_id(-1);
	//}

	// int prehovered_id = implant_id_text_->prehovered_id();
	// if (prehovered_id != -1) {
	//	CW3View2D_forMPR::drawImplantID(prehovered_id, QPointF(0.0f, 0.0f),
	//false); 	implant_id_text_->set_prehovered_id(-1);
	//}

	// if (hovered_id != -1 && m_pRender3DParam->g_is_implant_exist_[hovered_id])
	// { 	CW3Implant** implant_set = m_pgRcontainer->getImplants();
	//	implant_id_text_->set_prehovered_id(hovered_id);
	//	glm::vec3 implantPosInGL = glm::mat3(m_rotate) *
	//glm::vec3(implant_set[hovered_id]->m_translate[3]); 	implantPosInGL.x *=
	//-1.0f;

	//	float pos_x = implantPosInGL.x / m_scaleSceneToGL +
	//m_pntCurViewCenterinScene.x(); 	float pos_y = implantPosInGL.z /
	//m_scaleSceneToGL + m_pntCurViewCenterinScene.y(); 	QPointF implantPos =
	//QPointF(pos_x, pos_y) - m_sceneTrans;
	//	CW3View2D_forMPR::drawImplantID(hovered_id, implantPos, true);
	//}
}

void CW3View3D_forMPR::ResizeAlignTexts()
{
	if (m_lpTextAlign[0])
	{
		float horizontalSpacing = m_lpTextAlign[0]->sceneBoundingRect().width();
		for (int i = 0; i < 6; i++)
		{
			m_lpTextAlign[i]->setPos(
				mapToScene(width() - common::ui_define::kViewMarginWidth -
				(horizontalSpacing * (6 - i)),
					common::ui_define::kViewFilterOffsetY));
		}
	}
}

void CW3View3D_forMPR::ResizeAirwayUIs()
{
	if (m_eViewType == common::ViewTypeID::MPR_3D)
	{
		int stratColorVal = m_nColorMin / 100;
		int endColorVal = (m_nColorMax / 100) + 1;
		float num_color = abs(endColorVal - stratColorVal);

		float minPosY = 1.75f * 0.5f * height();
		float maxPosY = 0.25f * 0.5f * height();
		float interval = abs(maxPosY - minPosY) / num_color;

		for (int i = 0; i < m_lpTextColorBar.size(); i++)
			m_lpTextColorBar.at(i)->setPos((-0.65 + 1.0f) * 0.5f * width(),
			(minPosY - (interval * i)) - 15.0f);

		if (m_pRender3DParam->m_pAirway->isVisible())
		{
			drawAirwayColorBar();
		}
	}
}

void CW3View3D_forMPR::SetVisibleItems()
{
	CW3View2D_forMPR::SetVisibleItems();
	bool is_text_visible = !(hide_all_view_ui_);

	if (m_lpTextAlign[0])
	{
		for (int i = 0; i < 6; i++) m_lpTextAlign[i]->setVisible(is_text_visible);
	}

	if (ui_recon_type_) ui_recon_type_->setVisible(is_text_visible);
}

bool CW3View3D_forMPR::Is3DRenderingMode()
{
	if (recon_type_ == common::ReconTypeID::VR ||
		recon_type_ == common::ReconTypeID::MIP)
	{
		return true;
	}
	return false;
}

void CW3View3D_forMPR::slotVRAlignS()
{
	m_rotate = glm::rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
	setVRAlign();
}

void CW3View3D_forMPR::slotVRAlignI()
{
	m_rotate = glm::rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
	setVRAlign();
}

void CW3View3D_forMPR::slotVRAlignL()
{
	m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}

void CW3View3D_forMPR::slotVRAlignR()
{
	m_rotate = glm::rotate(glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}

void CW3View3D_forMPR::slotVRAlignA()
{
	m_rotate = mat4(1.0f);
	setVRAlign();
}

void CW3View3D_forMPR::slotVRAlignP()
{
	m_rotate = glm::rotate(glm::radians(180.0f), vec3(0.0f, 0.0f, 1.0f));
	setVRAlign();
}

void CW3View3D_forMPR::SetAirway(std::vector<tri_STL> &mesh)
{
	m_nColorMin = std::numeric_limits<unsigned int>::max();
	m_nColorMax = std::numeric_limits<unsigned int>::min();

	m_vVertices.clear();
	m_vVertexNormals.clear();
	m_vIndices.clear();
	m_vVertexColors.clear();

	std::map<glm::vec3, unsigned int, Vec3Less> mapVertIdx;
	int indexCount = 0;
	std::vector<unsigned int> vCountSameVertex;
	std::vector<glm::vec3> vFaceNormals;

	vec3 volRange = *m_pgVREngine->getVolRange(0);
	float spacing = m_pgVREngine->getVol(0)->pixelSpacing();

	for (int i = 0; i < mesh.size(); i++)
	{
		const auto &meshAt = mesh.at(i);

		float x[3] = { -meshAt.v1.x / spacing / volRange.x * 2.0f - 1.0f,
					  -meshAt.v2.x / spacing / volRange.x * 2.0f - 1.0f,
					  -meshAt.v3.x / spacing / volRange.x * 2.0f - 1.0f };
		float y[3] = { meshAt.v1.y / spacing / volRange.y * 2.0f - 1.0f,
					  meshAt.v2.y / spacing / volRange.y * 2.0f - 1.0f,
					  meshAt.v3.y / spacing / volRange.y * 2.0f - 1.0f };
		float z[3] = { meshAt.v1.z / spacing / volRange.z * 2.0f - 1.0f,
					  meshAt.v2.z / spacing / volRange.z * 2.0f - 1.0f,
					  meshAt.v3.z / spacing / volRange.z * 2.0f - 1.0f };

		float nx = -meshAt.normal.x;
		float ny = -meshAt.normal.y;
		float nz = -meshAt.normal.z;
		glm::vec3 faceNormal(nx, ny, nz);

		for (int j = 0; j < 3; j++)
		{
			glm::vec3 vertex =
				glm::vec3(-x[j], y[j], z[j]);  // by jdk 170203 for airway x축 반전
			if (mapVertIdx.find(vertex) == mapVertIdx.end())
			{
				mapVertIdx[vertex] = indexCount;
				m_vVertices.push_back(vertex);
				m_vVertexColors.push_back(
					vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z));
				m_vIndices.push_back(indexCount++);
				vFaceNormals.push_back(faceNormal);
				vCountSameVertex.push_back(1);
			}
			else
			{
				unsigned int id = mapVertIdx[vertex];
				m_vIndices.push_back(id);
				vFaceNormals.at(id) += faceNormal;
				vCountSameVertex.at(id)++;
			}
		}

		if (meshAt.nColorVal < m_nColorMin)
		{
			m_nColorMin = meshAt.nColorVal;
			m_vColorMin = vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z);
		}

		if (meshAt.nColorVal > m_nColorMax)
		{
			m_nColorMax = meshAt.nColorVal;
			m_vColorMax = vec3(meshAt.fColor.x, meshAt.fColor.y, meshAt.fColor.z);
		}
	}

	for (int i = 0; i < vCountSameVertex.size(); i++)
	{
		glm::vec3 normal = vFaceNormals.at(i) / float(vCountSameVertex.at(i));
		m_vVertexNormals.push_back(normal);
	}

	m_pRender3DParam->m_pAirway->setNindices(m_vIndices.size());

	m_pgVREngine->makeCurrent();
	setAirwayVBO();
	m_pgVREngine->doneCurrent();

	if (isVisible() && m_pGLWidget && m_pGLWidget->context()) RenderAndUpdate();
}

void CW3View3D_forMPR::slotShadeOnFromOTF(bool isShading)
{
	// m_pReconTypeTextItem가 있는 view는 OTF의 shading 옵션 적용하지 않음
	if (ui_recon_type_) return;

	m_pRender3DParam->m_isShading = isShading;

	if (!m_is3Dready || !isVisible()) return;

	RenderAndUpdate();
}

void CW3View3D_forMPR::setMIP(bool isMIP)
{
	if (!isReadyRender3D()) return;

	if (recon_type_ == common::ReconTypeID::MPR ||
		recon_type_ == common::ReconTypeID::X_RAY)
		return;

	if (!ui_recon_type_) return;

	if (isMIP)
		ui_recon_type_->setReconType(ui_primitive::kReconFilterMIP);
	else if (m_pRender3DParam->m_isShading)
		ui_recon_type_->setReconType(ui_primitive::kReconFilterVR);
	else
		ui_recon_type_->setReconType(ui_primitive::kReconFilterVRUnshade);
}

void CW3View3D_forMPR::setNerveVAOVBO()
{
	const NerveResource &res_nerve =
		ResourceContainer::GetInstance()->GetNerveResource();
	const auto &datas = res_nerve.GetNerveDataInVol();

	std::vector<glm::vec3> verts, norms;
	std::vector<uint> indices;

	for (const auto &elem : datas)
	{
		int indices_begin = verts.size();

		verts.insert(verts.end(), elem.second->mesh_vertices().begin(),
			elem.second->mesh_vertices().end());
		norms.insert(norms.end(), elem.second->mesh_normals().begin(),
			elem.second->mesh_normals().end());
		const auto &indices_mesh = elem.second->mesh_indices();
		for (const auto &i : indices_mesh) indices.push_back(indices_begin + i);
	}
	uint vao = 0;
	std::vector<uint> vbo;
	vbo.resize(3, 0);

	CW3GLFunctions::initVAOVBO(&vao, &vbo[0], verts, norms, indices);

	m_pRender3DParam->m_pNerve->setVAO(vao);
	m_pRender3DParam->m_pNerve->setVBO(vbo);
	m_pRender3DParam->m_pNerve->setNindices(indices.size());
}

void CW3View3D_forMPR::setAirwayVBO()
{
	if (m_vboAirway[0])
	{
		glDeleteBuffers(4, m_vboAirway);
		m_vboAirway[0] = 0;
		m_vboAirway[1] = 0;
		m_vboAirway[2] = 0;
		m_vboAirway[3] = 0;
	}

	glGenBuffers(4, m_vboAirway);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[0]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertices.size() * 3 * sizeof(float),
		m_vVertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[1]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertexNormals.size() * 3 * sizeof(float),
		m_vVertexNormals.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[2]);
	glBufferData(GL_ARRAY_BUFFER, m_vVertexColors.size() * 3 * sizeof(float),
		m_vVertexColors.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboAirway[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		m_vIndices.size() * sizeof(unsigned int), m_vIndices.data(),
		GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3View3D_forMPR::setAirwayVAO()
{
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
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboAirway[2]);
	glVertexAttribPointer((GLuint)2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboAirway[3]);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_pRender3DParam->m_pAirway->setVAO(VAO);
	m_pRender3DParam->m_pAirway->setNindices(m_vIndices.size());
}

void CW3View3D_forMPR::drawAirwayColorBar()
{
	int stratColorVal = m_nColorMin / 100;
	int endColorVal = (m_nColorMax / 100) + 1;
	int num_color = abs(endColorVal - stratColorVal);

	float minPosY = 1.75f * 0.5f * height();
	float maxPosY = 0.25f * 0.5f * height();
	float interval = abs(maxPosY - minPosY) / static_cast<float>(num_color);

	if (m_pAirwayColorBar)
	{
		scene()->removeItem(m_pAirwayColorBar);
		SAFE_DELETE_OBJECT(m_pAirwayColorBar);
	}

	m_pAirwayColorBar =
		scene()->addRect((-0.7f + 1.0f) * 0.5f * width(), minPosY,
			0.05f * 0.5f * width(), maxPosY - minPosY);
	m_pAirwayColorBar->setVisible(m_pRender3DParam->m_pAirway->isVisible());

	QFont font = QApplication::font();
	font.setPixelSize(10);

	for (int i = 0; i < m_lpTextColorBar.size(); i++)
	{
		scene()->removeItem(m_lpTextColorBar.at(i));
		SAFE_DELETE_OBJECT(m_lpTextColorBar.at(i));
	}
	m_lpTextColorBar.clear();

	QRectF rect = m_pAirwayColorBar->rect();
	QLinearGradient gradient(0, rect.top(), 0, rect.bottom());
	for (int i = 0; i <= num_color; i++)
	{
		float pos = (float)i / static_cast<float>(num_color);
		QColor color = GetAirwayColor(i);
		gradient.setColorAt(pos, color);

		QGraphicsTextItem *textItem =
			scene()->addText(QString::number((100 * stratColorVal) + (100 * i)));
		if (i == 0)
			textItem->setPlainText(textItem->toPlainText() + " " +
				QString::fromLocal8Bit("mm2"));
		textItem->setPos((-0.65f + 1.0f) * 0.5f * width(),
			(minPosY - (interval * i)) - 15.0f);
		textItem->setFont(font);
		textItem->setDefaultTextColor(Qt::white);
		textItem->setVisible(m_pRender3DParam->m_pAirway->isVisible());
		m_lpTextColorBar.push_back(textItem);
	}

	m_pAirwayColorBar->setBrush(gradient);
}

bool CW3View3D_forMPR::isReadyRender3D() const
{
	if (m_is3Dready && isVisible() && m_pGLWidget && m_pGLWidget->context())
		return true;

	return false;
}

void CW3View3D_forMPR::setPosReconTypeTextItem()
{
	ui_recon_type_->setPos(
		mapToScene(width() - common::ui_define::kViewMarginWidth -
			ui_recon_type_->sceneBoundingRect().width(),
			common::ui_define::kViewSpacing * 4.0f));
}

/**********************************************************************************************
Tool type interactions 3D in move.

@author	Seo Seok Man
@date	2018-02-22

@param	curr_view_pos	The curr view position.

@return	True if re-rendering needed
**********************************************************************************************/
bool CW3View3D_forMPR::ToolTypeInteractions3DInMove(
	const QPoint &curr_view_pos)
{
	QPointF curr_scene_pos = mapToScene(curr_view_pos);

	if (measure_tools_->IsSelected())
	{
		measure_tools_->ProcessMouseMove(mapToScene(curr_view_pos), kTempPos);
		return false;
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_LIGHT)
	{
		m_pRender3DParam->m_isLowRes = true;
		WindowingView(curr_scene_pos);
		return true;
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_PAN ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		m_pRender3DParam->m_isLowRes = true;
		PanningView(curr_scene_pos);
		return true;
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM)
	{
		m_pRender3DParam->m_isLowRes = true;
		ScaleView(curr_view_pos);
		return true;
	}
	return false;
}

bool CW3View3D_forMPR::IsLabelSelected()
{
	if (m_lpTextAlign[0])
	{
		for (int i = 0; i < 6; i++)
		{
			if (m_lpTextAlign[i]->isUnderMouse()) return true;
		}
	}

	if (m_pProxySlider && m_pProxySlider->isUnderMouse()) return true;

	return false;
}

void CW3View3D_forMPR::HideMeasure(bool toggled)
{
	CW3View2D_forMPR::HideMeasure(toggled);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetVisible(!toggled);

		scene()->update();
	}
}

void CW3View3D_forMPR::DeleteAllMeasure()
{
	CW3View2D_forMPR::DeleteAllMeasure();

	if (measure_3d_manager_)
	{
#if 0
		measure_3d_manager_->SetType(Measure3DManager::Type::DELETE_ALL);
#else
		measure_3d_manager_->Clear();

		scene()->update();
#endif
	}
}

void CW3View3D_forMPR::DeleteUnfinishedMeasure()
{
	CW3View2D_forMPR::DeleteUnfinishedMeasure();

	if (measure_3d_manager_)
	{
		measure_3d_manager_->DeleteUnfinishedItem();

		scene()->update();
	}
}

void CW3View3D_forMPR::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	if (type == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		CW3View2D_forMPR::SetCommonToolOnOff(common::CommonToolTypeOnOff::NONE);
		return;
	}
	else
	{
		CW3View2D_forMPR::SetCommonToolOnOff(type);
	}

	if (measure_3d_manager_)
	{
		measure_3d_manager_->SetType(type);
	}
}
#ifndef WILL3D_VIEWER
void CW3View3D_forMPR::exportProject(ProjectIOView& out)
{
	CW3View2D_forMPR::exportProject(out);

	if (measure_3d_manager_)
	{
		measure_3d_manager_->ExportProject(out);
	}
}

void CW3View3D_forMPR::importProject(ProjectIOView& in)
{
	if (!measure_3d_manager_)
	{
		measure_3d_manager_ = new Measure3DManager(scene());
	}
	measure_3d_manager_->ImportProject(in);

	CW3View2D_forMPR::importProject(in);
}
#endif
void CW3View3D_forMPR::keyPressEvent(QKeyEvent* event)
{
	glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);

	if (event->key() == Qt::Key_Up ||
		event->key() == Qt::Key_Down)
	{
		rotate_axis = glm::vec3(-1.0f, 0.0f, 0.0f);
		measure_tools_->ImportMeasureResource();
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

	CW3View2D_forMPR::keyPressEvent(event);
}

void CW3View3D_forMPR::ApplyPreferences()
{
	render3D();

	CW3View2D_forMPR::ApplyPreferences();
}

bool CW3View3D_forMPR::IsControllerTextOnMousePress()
{
	return IsLabelSelected();
}
