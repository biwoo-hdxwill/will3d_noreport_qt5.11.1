#include "W3View3DEndoModify.h"
/*=========================================================================

File:			class CW3View3DEndoModify
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-28
Last modify:	2016-04-21

=========================================================================*/
#include <qopenglwidget.h>
#include <QOpenGLFramebufferObject>
#include <QApplication>
#include <qevent.h>

#include <Engine/Common/Common/language_pack.h>

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"

#include "../UIGLObjects/W3Spline3DItem.h"
#include "../UIGLObjects/W3GLObject.h"
#include "../UIPrimitive/W3TextItem_switch.h"
#include "../UIPrimitive/view_border_item.h"

#include "../../Module/VREngine/W3ActiveBlock.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

CW3View3DEndoModify::CW3View3DEndoModify(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
	QWidget *pParent)
	: CW3View3D(VREngine, MPRengine, Rcontainer, eType, pParent)
{
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	m_pModifyModeOnOff = new CW3TextItem_switch(lang::LanguagePack::txt_modify());
	m_pModifyModeOnOff->setVisible(false);
	m_pModifyModeOnOff->setFont(font);

	scene()->addItem(m_pModifyModeOnOff);

	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());

	connections();
}

CW3View3DEndoModify::~CW3View3DEndoModify(void)
{
	if (m_pGLWidget)
		if (m_pGLWidget->context())
			clearGL();
}

void CW3View3DEndoModify::reoriUpdate(glm::mat4* m)
{
	this->slotReoriupdate(m);
}

void CW3View3DEndoModify::setVisiblePath(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_bVisiblePath = true;
		break;
	case Qt::CheckState::Unchecked:
		m_bVisiblePath = false;
		break;
	}
	scene()->update();
}

void CW3View3DEndoModify::connections()
{
	if (m_pModifyModeOnOff)
		connect(m_pModifyModeOnOff, SIGNAL(sigState(bool)), this, SLOT(slotModifyModeOnOff(bool)));
}

void CW3View3DEndoModify::reset()
{
	CW3View3D::reset();

	m_pgvEndoControlPoint = nullptr;
	m_pgSpline3D = nullptr;

	m_nModifyPointIndex = -1;
	m_nPickedPointIndex = -1;

	m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	m_model = m_rotate * m_origModel*m_scaleMat;

	resetView();
	CW3View3D::clearGL();
}

void CW3View3DEndoModify::UpdateVR(bool is_high_quality)
{
	m_pRender3DParam->m_isLowRes = !is_high_quality;
	if (isVisible())
	{
		render3D();
		scene()->update();
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	* resetView()
//	endo path를 삭제하거나 변경한 path slot이 비어있을 때, view 상태 초기화
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoModify::resetView()
{
	if (m_pModifyModeOnOff)
		m_pModifyModeOnOff->setCurrentState(false);

	m_pgvEndoControlPoint = nullptr;

	m_nModifyPointIndex = -1;
	m_nPickedPointIndex = -1;

	m_rotate = glm::rotate(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f));
	m_model = m_rotate * m_origModel*m_scaleMat;

	if (m_pGLWidget)
	{
		if (m_pGLWidget->context())
		{
			m_pGLWidget->makeCurrent();
			if (m_vaoCameraDir)
			{
				glDeleteVertexArrays(1, &m_vaoCameraDir);
				m_vaoCameraDir = 0;
			}
			m_pGLWidget->doneCurrent();
		}
	}

	for (int i = 0; i < m_lpTextColorBar.size(); i++)
		scene()->removeItem(m_lpTextColorBar.at(i));
	m_lpTextColorBar.clear();

	m_nNumColor = 0;
	m_nColorMin = 0;
	m_nColorMax = 0;

	m_pRender3DParam->m_pAirway->setVisible(false);
	m_pModifyModeOnOff->setVisible(false);

	if (m_pAirwayColorBar)
		m_pAirwayColorBar->setVisible(false);

	setProjection();

	render3D();
}

void CW3View3DEndoModify::clearGL()
{
	if (m_pGLWidget)
	{
		if (m_pGLWidget->context())
		{
			m_pGLWidget->makeCurrent();
			if (m_vaoCameraDir)
			{
				glDeleteVertexArrays(1, &m_vaoCameraDir);
				m_vaoCameraDir = 0;
			}
			m_pGLWidget->doneCurrent();
		}
	}

	CW3View3D::clearGL();
}

void CW3View3DEndoModify::render3D()
{
	if (!isReadyRender3D())
		return;

	m_pGLWidget->makeCurrent();
	setProjection();

	if (m_pgSpline3D)
	{
		// modify mode일 때 선택한 point를 포함하는 modify plane을 rendering하기 위해
		// volume rendering의 front face를 해당 point의 depth까지 전진시킴
		if (m_pgSpline3D->isModifyMode())
		{
			setModelSlice();

			m_fPlaneDepth = -(m_fViewDepth + (m_camFOV)) / (m_camFOV);
			m_pgSpline3D->setPlaneDepth(m_fPlaneDepth);

			setMVP();

			m_pRender3DParam->m_isNearClipping = true;
		}
		else
		{
			m_pgSpline3D->setPlaneDepth(1.0f);

			m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, m_camFOV*2.0f);

			setMVP();

			m_pRender3DParam->m_isNearClipping = false;
		}
	}
	else
	{
		m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, m_camFOV*2.0f);

		setMVP();

		m_pRender3DParam->m_isNearClipping = false;
	}

	m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_mvp = &m_mvp;
	m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_invModel = &m_modelSlice;
	m_pRender3DParam->m_width = this->width();
	m_pRender3DParam->m_height = this->height();

	if (!m_pRender3DParam->m_plane->getVAO())
	{
		unsigned int vao = 0;
		m_pRender3DParam->m_plane->clearVAOVBO();

		m_pgVREngine->initVAOplane(&vao);
		m_pRender3DParam->m_plane->setVAO(vao);
		m_pRender3DParam->m_plane->setNindices(6);
	}

	if (m_pRender3DParam->m_mainVolume_vao[m_drawVolId])
	{
		glDeleteVertexArrays(1, &m_pRender3DParam->m_mainVolume_vao[m_drawVolId]);
		m_pRender3DParam->m_mainVolume_vao[m_drawVolId] = 0;
	}
	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[m_drawVolId], m_drawVolId);

	if (m_pRender3DParam->m_pAirway->isVisible())
	{
		setAirwayVAO();
		m_pRender3DParam->m_pAirway->setInvModel(m_inverseScale);
		m_pRender3DParam->m_pAirway->setMVP(m_model, m_view, m_projection);
	}

	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);
	WGLSLprogram::setUniform(PROGRaycasting, "useSegTmj", false);

	m_pgVREngine->Render3Dboth(m_pRender3DParam, m_drawVolId, m_isReconSwitched);

	m_pGLWidget->doneCurrent();
}

void CW3View3DEndoModify::drawBackground(QPainter *painter, const QRectF &rect)
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

		////////// Modify plane
		if (m_pgSpline3D)
		{
			if (m_pgSpline3D->isModifyMode())
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

				unsigned int PROGendoPlane = m_pgVREngine->getPROGendoPlane();
				glUseProgram(PROGendoPlane);

				m_pgVREngine->setVolTextureUniform(PROGendoPlane, m_pRender3DParam->m_pgMainVolume[m_drawVolId]->m_texHandlerVol);
				WGLSLprogram::setUniform(PROGendoPlane, "MVP", m_mvp);

				WGLSLprogram::setUniform(PROGendoPlane, "sliceModel", m_modelSlice);
				WGLSLprogram::setUniform(PROGendoPlane, "isForFront", false);
				CW3GLFunctions::drawView(m_pRender3DParam->m_plane->getVAO(), m_pRender3DParam->m_plane->getNindices(), GL_BACK);

				glUseProgram(0);
				glDisable(GL_BLEND);
			}
		}

		////////// endo path
		if (m_pgSpline3D && m_bVisiblePath)
		{
			if (m_pgSpline3D->isPathValid())
			{
				m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, m_camFOV*2.0f);
				setMVP();

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// src = src * src.a, dst = dst * (1 - src.a)

				glEnable(GL_DEPTH_TEST);
				glClearDepth(1.0f);
				glClear(GL_DEPTH_BUFFER_BIT);
				glDepthFunc(GL_LESS);

				m_pgSpline3D->drawModifyMode(m_pgVREngine->getPROGanno(), m_mvp, m_nModifyPointIndex);	// spline 그리기

				glDisable(GL_DEPTH_TEST);
				glDisable(GL_BLEND);
			}
		}

		DrawMeasure3D();
	}
	else
	{
		CW3GLFunctions::clearView(false);
	}

	painter->endNativePainting();

	m_isOnlyItemUpdate = false;
}

void CW3View3DEndoModify::resizeEvent(QResizeEvent *pEvent)
{
	CW3View3D::resizeEvent(pEvent);

	m_pModifyModeOnOff->setPos(
		mapToScene(width() - common::ui_define::kViewMarginWidth - m_pModifyModeOnOff->sceneBoundingRect().width(),
			common::ui_define::kViewSpacing * 5.0f));

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
		scene_size.x(), scene_size.y()));
}

///////////////////////////////////////////////////////////////////////////
//
//	* setMPROverlayPlane()
//	modify plane을 rendering하기 위한 vertex를 생성하고 vbo 갱신
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoModify::setModelSlice()
{
	if (!m_pgvEndoControlPoint)
		return;

	if (m_nModifyPointIndex == -1)
		return;

	vec3 selectedPoint = m_pgvEndoControlPoint->at(m_nModifyPointIndex);
	vec4 mp(selectedPoint, 1.0f);
	vec4 viewp = m_view * m_model * mp;
	m_fViewDepth = viewp.z;

	m_modelSlice = glm::inverse(m_view * m_model) * glm::scale(glm::vec3(proj_right_, proj_top_, 1.0f))*glm::translate(glm::vec3(0.0f, 0.0f, m_fViewDepth));

	m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, -(m_fViewDepth + 0.01f), m_camFOV*2.0f);
}

// CW3View3DSagital에서 spline그리기가 완료되면 CW3Spline3DItem을 전달받음
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdateEndoPath(CW3Spline3DItem *, const bool)
void CW3View3DEndoModify::slotUpdateEndoPath(CW3Spline3DItem *path, const bool reset)
{
	m_pgSpline3D = path;

	if (reset)
	{
		resetView();
	}

	if (m_pgSpline3D)
	{
		m_pModifyModeOnOff->setVisible(true);
		m_pgvEndoControlPoint = m_pgSpline3D->getControlPoint();
		if (m_pgvEndoControlPoint->size() > 0)
		{
			m_nModifyPointIndex = 0;
		}
		emit sigAirwayDisabled(false);
	}
	else
	{
		emit sigAirwayDisabled(true);
		//emit sigSetAirwaySize(0.0f);
	}

	if (isVisible())
	{
		scene()->update();
	}
}

// CW3View3DEndo에서 카메라 전/후진을 하면 해당 index를 전달받아 sagital 상의 카메라 위치를 표시
//	* connected SIGNAL : CW3View3DEndo::sigSetCameraPos(const int, const bool)
void CW3View3DEndoModify::slotSetCameraPos(const int index, const bool isControlPoint)
{
	if (!isReadyRender3D())
		return;

	setCameraPos(index, isControlPoint);

	render3D();

	if (isVisible())
		scene()->update();
}

// Scene update
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdate()
void CW3View3DEndoModify::slotUpdate()
{
	if (isVisible())
		scene()->update();
}

// CW3View3DEndoModify의 modify mode에서 control point를 선택하면 modify mode로 전환
//	* connected SIGNAL : CW3View3DEndoSagital::sigUpdatePoint(int)
void CW3View3DEndoModify::slotUpdatePoint(int index)
{
	if (!m_pgSpline3D)
		return;

	m_pgSpline3D->setModifyMode(true);
	m_nModifyPointIndex = index;

	//m_pModifyModeOnOff->setVisible(true);
	m_pModifyModeOnOff->setCurrentState(true);

	render3D();

	if (isVisible())
		scene()->update();
}

///////////////////////////////////////////////////////////////////////////
//
//	* setCameraPos(const int index, const bool isControlPoint)
//	카메라 위치 표시를 rendering하기 위해 CW3Spline3DItem에 위치를 전달
//
///////////////////////////////////////////////////////////////////////////
void CW3View3DEndoModify::setCameraPos(const int index, const bool isControlPoint)
{
	m_pGLWidget->makeCurrent();
	m_pgSpline3D->setCameraPosPoint(index, isControlPoint);
	m_pGLWidget->doneCurrent();
}

///////////////////////////////////////////////////////////////////////////
//
//	* transformSliceToVolume(QPointF ptScene)
//	scene 에서의 이동 거리를 volume 에서의 이동 거리로 변경
//
///////////////////////////////////////////////////////////////////////////
glm::vec3 CW3View3DEndoModify::transformDistSliceToVolume(QPointF ptScene)
{
	vec4 ptGLCoord = vec4(ptScene.x() / m_sceneWinView, -ptScene.y() / m_sceneHinView, 0.0f, 0.0f);
	vec4 ptVolCoord = m_modelSlice * ptGLCoord;

	return glm::vec3(ptVolCoord.xyz);
}

bool CW3View3DEndoModify::mouseMoveEventW3(QMouseEvent *event)
{
	if (m_pgSpline3D)
	{
		// 정대건이 필요하면 쓰고 아니면 지울 것, 현재 코드에서는 역할 없음
		if (event->buttons() & Qt::RightButton)
		{
			if (m_pgSpline3D->isModifyMode())
				emit sigUpdate();
		}
		else if (event->buttons() & Qt::LeftButton)
		{
			if (m_pgSpline3D->isEnd() && m_nPickedPointIndex > -1)
			{
				QPointF scene_pos = mapToScene(event->pos());
				QPointF distInScene = scene_pos - last_scene_pos_;	// control point를 이동을 위한 scene에서의 이동거리
				last_scene_pos_ = scene_pos;

				m_pgSpline3D->translatePath(transformDistSliceToVolume(distInScene));	// control point를 이동

				m_isOnlyItemUpdate = true;

				// 선택된 index가 전체 control point list의 size보다 작으면 point, 아니면 선택해제
				if (m_nPickedPointIndex < m_pgSpline3D->getControlPoint()->size())
					// 현재 선택된 control point index를 각 view에 update
					//	* connected SLOT : CW3View3DEndo::slotUpdatePoint(int)
					//	* connected SLOT : CW3View3DEndoSagital::slotUpdatePoint(int)
					emit sigUpdatePoint(m_nPickedPointIndex);
				else
					// point가 선택되지 않았을 경우 상태 update
					//	* connected SLOT : CW3View3DEndo::slotUpdate()
					//	* connected SLOT : CW3View3DEndoSagital::slotUpdate()
					emit sigUpdate();

				return false;
			}
		}
		else
		{
			if (m_pgSpline3D->isModifyMode())
			{
				m_pGLWidget->makeCurrent();
				{
					QOpenGLFramebufferObject fbo(this->width(), this->height());
					fbo.bind();

					CW3GLFunctions::clearView(true, GL_BACK);
					glViewport(0, 0, this->width(), this->height());

					int prog = m_pgVREngine->getPROGpick();
					int newPickedPointIndex = m_pgSpline3D->pickPoint(event->pos(), prog, m_mvp, m_nModifyPointIndex);	// 수정할 point 하나에 대해서만 picking 검사

					fbo.release();

					// 선택 상태가 변경될 때만 scene update
					if (m_nPickedPointIndex != newPickedPointIndex)
					{
						m_nPickedPointIndex = newPickedPointIndex;
						// control point가 선택되었을 경우 CW3View3DEndoModify를 update
						//	* connected SLOT : CW3View3DEndoModify::slotUpdate()
						emit sigUpdatePick();
						m_isOnlyItemUpdate = true;
					}
				}
				m_pGLWidget->doneCurrent();

				return false;
			}
		}
	}

	return CW3View3D::mouseMoveEventW3(event);
}

void CW3View3DEndoModify::keyPressEvent(QKeyEvent * event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	// modify mode일 때 spacebar를 누르면 modify mode on/off toggle 가능
	if (event->key() == Qt::Key_Space)
	{
		m_pModifyModeOnOff->setCurrentState(!m_pgSpline3D->isModifyMode());
	}

	CW3View3D::keyPressEvent(event);
}

#ifdef WILL3D_EUROPE
void CW3View3DEndoModify::mousePressEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mousePressEvent(event);
}

void CW3View3DEndoModify::mouseReleaseEvent(QMouseEvent* event)
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

void CW3View3DEndoModify::mouseMoveEvent(QMouseEvent* event)
{
	if (is_control_key_on_)
	{
		return;
	}

	CW3View3D::mouseMoveEvent(event);
}

void CW3View3DEndoModify::keyReleaseEvent(QKeyEvent* event)
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


void CW3View3DEndoModify::slotModifyModeOnOff(bool isModifyMode)
{
	if (!m_pgSpline3D)
		return;

	if (m_nModifyPointIndex < 0)
		return;

	m_pgSpline3D->setModifyMode(isModifyMode);

	if (m_pgSpline3D->isModifyMode())
	{
		emit sigUpdate();
	}

	render3D();
	scene()->update();
}

void CW3View3DEndoModify::HideUI(bool bToggled)
{
	CW3View3D::HideUI(bToggled);

	m_pModifyModeOnOff->setVisible(!hide_all_view_ui_ && m_pgSpline3D);
}
