#include "W3View3DMPR.h"
/*=========================================================================

File:			class CW3View3DMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2016-04-21

=========================================================================*/
#include <ctime>
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include <qgraphicsproxywidget.h>
#include <qtimer.h>
#include <QApplication>
#include <QMatrix4x4>
#include <QSettings>
#include <QTextCodec>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_mpr.h"
#endif
#include "../UIGLObjects/W3GLNerve.h"
#include "../UIGLObjects/W3SurfacePlaneItem.h"
#include "../UIPrimitive/W3TextItem_switch.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/annotation_freedraw.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIPrimitive/view_border_item.h"

#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VREngine.h"

using namespace UIGLObjects;
using namespace NW3Render3DParam;

namespace
{
	// TODO smseo : 볼륨 좌표를 넣기 전 임시 값
	const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);
	unsigned int kZoomCubeIndex[36] = { 1, 5, 3, 3, 5, 7, 0, 2, 4, 4, 2, 6,
									   0, 1, 2, 2, 1, 3, 7, 5, 6, 6, 5, 4,
									   2, 3, 6, 6, 3, 7, 1, 0, 5, 5, 0, 4 };
}  // namespace

CW3View3DMPR::CW3View3DMPR(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer,
	common::ViewTypeID eType, QWidget *pParent)
	: CW3View3D_forMPR(VREngine, MPRengine, Rcontainer, eType, pParent),
	wheel_timer_(new QTimer(this))
{
	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);

	m_pMPROverlayOnOff = new CW3TextItem_switch(QString(tr("MPR Overlay")));
	m_pMPROverlayOnOff->setVisible(false);
	m_pMPROverlayOnOff->setFont(font);

	m_pPerspectiveOnOff = new CW3TextItem_switch(QString(tr("Perspective")));
	m_pPerspectiveOnOff->setVisible(false);
	m_pPerspectiveOnOff->setFont(font);

	scene()->addItem(m_pMPROverlayOnOff);
	scene()->addItem(m_pPerspectiveOnOff);

	border_.reset(new ViewBorderItem());
	border_->setZValue(100.0f);
	border_->SetColor(ColorView::k3D);
	border_->setVisible(false);
	scene()->addItem(border_.get());

	m_vboCUBE[0] = 0;
	m_vboCUBE[1] = 0;
	m_vboCUBE[2] = 0;

	wheel_timer_->setInterval(300);
	wheel_timer_->setSingleShot(true);

	connections();
}

CW3View3DMPR::~CW3View3DMPR(void)
{
	SaveMPROverlaySwitch();

	DeleteVRCutPolygonArea();
	if (m_pGLWidget && m_pGLWidget->context())
	{
		clearGL();
	}

	SAFE_DELETE_OBJECT(vr_cut_mask_vol_);
	SAFE_DELETE_OBJECT(wheel_timer_);
}

void CW3View3DMPR::UpdateVR(bool is_high_quality)
{
	m_pRender3DParam->m_isLowRes = !is_high_quality;
	Render3DAndUpdateIfVisible();
}
#ifndef WILL3D_VIEWER
void CW3View3DMPR::exportProject(ProjectIOMPR &out)
{
	CW3View3D_forMPR::exportProject(out.GetViewIO());
	out.SaveRotateMatrix(m_rotate);
}

void CW3View3DMPR::importProject(ProjectIOMPR &in)
{
	CW3View3D_forMPR::importProject(in.GetViewIO());
	in.LoadRotateMatrix(m_rotate);
	setModel();
	m_pWorldAxisItem->SetWorldAxisDirection(m_rotate, m_view);
}
#endif
void CW3View3DMPR::SetCommonToolOnOff(const common::CommonToolTypeOnOff &type)
{
#if 0
	if (type >= common::CommonToolTypeOnOff::M_RULER &&
		type < common::CommonToolTypeOnOff::M_DEL)
		common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	else
		common_tool_type_ = type;
	measure_tools_->SetMeasureType(common_tool_type_);
#else
	CW3View3D_forMPR::SetCommonToolOnOff(type);
#endif
}

void CW3View3DMPR::reset()
{
	CW3View3D_forMPR::reset();
	m_pPerspectiveOnOff->setCurrentState(false);

	m_bToggledClipMPROverlay = false;
	m_bDrawMPROverlay = false;

	InitMPROverlaySwitch();

	event_type_ = MPR3DEventType::NONE;
	SAFE_DELETE_OBJECT(vr_cut_polygon_);
	SAFE_DELETE_OBJECT(vr_cut_freedraw_);

	m_fObiquePlanePos = 0.0f;
	m_fRadius = 0.0f;

	m_bIsZoom3DMode = false;
	m_bIsObliqueMode = false;

	clearGL();
}

void CW3View3DMPR::connections()
{
	connect(m_pMPROverlayOnOff, SIGNAL(sigState(bool)), this,
		SLOT(slotMPROverlayOnOff(bool)));
	connect(m_pPerspectiveOnOff, SIGNAL(sigState(bool)), this,
		SLOT(slotPerspectiveOnOff(bool)));
	connect(wheel_timer_, SIGNAL(timeout()), this, SLOT(slotWheelTimeout()));
}

void CW3View3DMPR::mousePressEvent(QMouseEvent *event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	Qt::MouseButton button = event->button();
	if (IsDefaultInteractionsIn3D(button) ||
		common_tool_type_ != common::CommonToolTypeOnOff::NONE)
	{
		return CW3View3D_forMPR::mousePressEvent(event);
	}
	else if (button == Qt::LeftButton)
	{
		switch (event_type_)
		{
		case MPR3DEventType::VRCUT_DRAW:
			CreateVRCutUIInPress(mapToScene(event->pos()));
			break;
		case MPR3DEventType::VRCUT_SELECT:
			SelectVRCutAreaInPressNew();
			render3D();
			scene()->update();
			return QGraphicsView::mousePressEvent(event);
		case MPR3DEventType::NONE:
			return CW3View3D_forMPR::mousePressEvent(event);
		}
	}
	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(), event->button(),
		event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void CW3View3DMPR::mouseMoveEvent(QMouseEvent *event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	QPointF pt_scene = mapToScene(event->pos());

	if (vr_cut_polygon_)
	{
		switch (cut_tool_)
		{
		case VRCutTool::POLYGON:
		{
			Qt::MouseButtons buttons = event->buttons();
			if (IsDefaultInteractionsIn3D(buttons))
				return CW3View3D_forMPR::mouseMoveEvent(event);

			QPolygonF poly = vr_cut_polygon_->polygon();

			switch (event_type_)
			{
			case MPR3DEventType::VRCUT_DRAW:
				Draw3DCutUI(poly, pt_scene);
				break;
			case MPR3DEventType::VRCUT_SELECT:
				Draw3DCutSelectAreaUI(poly, pt_scene);
				break;
			}
		} break;
		case VRCutTool::FREEDRAW:
			switch (event_type_)
			{
			case MPR3DEventType::VRCUT_DRAW:
				if (vr_cut_freedraw_)
				{
					vr_cut_freedraw_->processMouseMove(pt_scene);
				}
				break;
			case MPR3DEventType::VRCUT_SELECT:
				QPolygonF poly = vr_cut_polygon_->polygon();
				Draw3DCutSelectAreaUI(poly, pt_scene);
				break;
			}
		}
	}
	else if (event_type_ != MPR3DEventType::VRCUT_SELECT)
	{
		return CW3View3D_forMPR::mouseMoveEvent(event);
	}
}

void CW3View3DMPR::mouseReleaseEvent(QMouseEvent *event)
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

	if (cut_tool_ == VRCutTool::FREEDRAW &&
		event_type_ == MPR3DEventType::VRCUT_DRAW)
	{
		if (vr_cut_freedraw_ && vr_cut_polygon_)
		{
			vr_cut_freedraw_->processMouseReleased(mapToScene(event->pos()));
			QPolygonF free_polygon = vr_cut_freedraw_->GetPolygon();
			vr_cut_polygon_->setPolygon(free_polygon);
			SAFE_DELETE_OBJECT(vr_cut_freedraw_);

			EndDraw3DCutUI();
		}

		CW3View2D_forMPR::mouseReleaseEvent(event);
	}
	else
	{
		m_pRender3DParam->m_isLowRes = false;
		CW3View3D_forMPR::mouseReleaseEvent(event);
	}
}

void CW3View3DMPR::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event_type_ == MPR3DEventType::VRCUT_DRAW && vr_cut_polygon_)
	{
		EndDraw3DCutUI();
		QGraphicsView::mouseDoubleClickEvent(event);
	}
	else
	{
		CW3View2D_forMPR::mouseDoubleClickEvent(event);
	}
}

void CW3View3DMPR::drawBackground(QPainter *painter, const QRectF &rect)
{
	if (!m_bIsOnlyTRDMode)
	{
		CW3View3D_forMPR::drawBackground(painter, rect);

		if (m_vaoDrawCube)
		{
			glEnable(GL_DEPTH_TEST);

			unsigned int program = m_pgVREngine->getPROGanno();

			glUseProgram(program);
			WGLSLprogram::setUniform(program, "MVP", m_mvp);
			WGLSLprogram::setUniform(program, "Color", vec4(0.0f, 1.0f, 0.0f, 1.0f));
			WGLSLprogram::setUniform(program, "PlaneDepth", 1.0f);

			glBindVertexArray(m_vaoDrawCube);

			glEnable(GL_LINE_SMOOTH);
			glLineWidth(1.0f);
			glDrawArrays(GL_LINES, 0, 24);
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDisable(GL_LINE_SMOOTH);

			glUseProgram(0);

			glDisable(GL_DEPTH_TEST);
		}
	}
	else
	{
		QGraphicsView::drawBackground(painter, rect);

		painter->beginNativePainting();

		m_pRender3DParam->m_photo3D->setVisible(true);

		m_vVolRange = glm::vec3(160.0f);
		m_scaleMat = glm::scale(m_vVolRange);
		m_inverseScale = glm::scale(1.0f / m_vVolRange);

		m_modelPhotoForTexture = m_inverseScale *
			m_pgRcontainer->getFacePhoto3D()->getSRtoVol() *
			m_scaleMat;

		m_camFOV = glm::length(m_vVolRange);

		setViewMatrix();
		setProjection();

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

		m_pRender3DParam->m_width = width();
		m_pRender3DParam->m_height = height();

		m_pgVREngine->RenderSurface(m_pRender3DParam, 0, true, m_isReconSwitched);

		painter->endNativePainting();
	}
}

void CW3View3DMPR::wheelEvent(QWheelEvent *event)
{
	if (m_bIsObliqueMode)
	{
		float minZ = -0.5f;
		float maxZ = 0.5f;

		double degrees = event->delta() / 8.0;
		float translate = degrees / 1500.0f;
		m_fObiquePlanePos += translate;
		m_fObiquePlanePos =
			(m_fObiquePlanePos > maxZ)
			? maxZ
			: ((m_fObiquePlanePos < minZ) ? minZ : m_fObiquePlanePos);

		m_pRender3DParam->m_clipPlanes[0].w = m_fObiquePlanePos;
		m_pRender3DParam->m_isLowRes = true;
		wheel_timer_->start();
		Render3DAndUpdateIfVisible();
	}
	else
	{
		wheel_timer_->start();
		CW3View3D_forMPR::wheelEvent(event);
	}
}

void CW3View3DMPR::resizeEvent(QResizeEvent *pEvent)
{
	if (!isVisible()) return;

	CW3View3D_forMPR::resizeEvent(pEvent);

	m_pMPROverlayOnOff->setPos(
		mapToScene(width() - common::ui_define::kViewMarginWidth -
			m_pMPROverlayOnOff->sceneBoundingRect().width(),
			common::ui_define::kViewFilterOffsetY + 20.0f));

	m_pPerspectiveOnOff->setPos(
		mapToScene(width() - common::ui_define::kViewMarginWidth -
			m_pPerspectiveOnOff->sceneBoundingRect().width(),
			common::ui_define::kViewFilterOffsetY + 40.0f));

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));

	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(), scene_size.x() + 1, scene_size.y() + 1));
}

void CW3View3DMPR::slotMPROverlayOnOff(bool isMPROverlay)
{
	if (!isMPROverlay && m_pRender3DParam->m_isClipped && m_bToggledClipMPROverlay)
	{
		m_pMPROverlayOnOff->setCurrentState(true);
		return;
	}

	if (offMIPOverlayWhereOnMIP(m_pRender3DParam->m_isMIP)) return;

	m_bDrawMPROverlay = isMPROverlay;
	m_pRender3DParam->m_pMPROverlay->setShown(isMPROverlay);
	Render3DAndUpdateIfVisible();

	if (isMPROverlay)
	{
		emit sigMPROverlayOn();
	}
}

void CW3View3DMPR::slotPerspectiveOnOff(bool isPerspective)
{
	m_pRender3DParam->m_isPerspective = isPerspective;

	setViewMatrix();
	Render3DAndUpdateIfVisible();
}

void CW3View3DMPR::slotWheelTimeout()
{
	m_pRender3DParam->m_isLowRes = false;
	Render3DAndUpdateIfVisible();
}

void CW3View3DMPR::setMaximize(bool maximize)
{
	if (!maximize)
	{
		event_type_ = MPR3DEventType::NONE;
	}
}

void CW3View3DMPR::SetOblique(bool bToggled)
{
	if (!m_is3Dready)
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR, "CW3View3DMPR::slotOblique: 3d not ready.");
	}

	m_bIsObliqueMode = bToggled;

	if (m_bIsObliqueMode)
	{
		m_saveClipPlanes[LOWER] = m_pRender3DParam->m_clipPlanes[LOWER];
		m_saveClipPlanes[UPPER] = m_pRender3DParam->m_clipPlanes[UPPER];
		m_saveIsClipped = m_pRender3DParam->m_isClipped;
		m_pRender3DParam->m_clipPlanes.clear();
		m_pRender3DParam->m_clipPlanes.push_back(vec4(0.0));
		m_pRender3DParam->m_isClipped = true;
	}
	else
	{
		m_pRender3DParam->m_clipPlanes.clear();
		m_pRender3DParam->m_clipPlanes.push_back(m_saveClipPlanes[LOWER]);
		m_pRender3DParam->m_clipPlanes.push_back(m_saveClipPlanes[UPPER]);
		m_pRender3DParam->m_isClipped = m_saveIsClipped;
	}

	Render3DAndUpdateIfVisible();
}

void CW3View3DMPR::setMPROverlay(const glm::vec4 &equ,
	const glm::vec3 &rightVec)
{
	m_pRender3DParam->m_pMPROverlay->editPlane(
		"MPROverlay", glm::scale(vec3(-1.0f, 1.0f, 1.0f)) * equ);
	m_pRender3DParam->m_pMPROverlay->setPlaneRightVector(
		"MPROverlay", vec3(-1.0f, 1.0f, 1.0f) * rightVec);

	vec3 scale = m_pRender3DParam->m_pgMainVolume[0]->m_voltexScale;
	vec3 scaledNorm = glm::normalize(equ.xyz * scale);

	if (m_bToggledClipMPROverlay)
	{
		if (m_pRender3DParam->flip_clipping_)
		{
			m_pRender3DParam->m_clipPlanes[LOWER] = vec4(scaledNorm * -1.0f, -sqrt(3.0f));
			m_pRender3DParam->m_clipPlanes[UPPER] = vec4(scaledNorm, equ.w);
		}
		else
		{
			m_pRender3DParam->m_clipPlanes[LOWER] = vec4(scaledNorm * -1.0f, -equ.w);
			m_pRender3DParam->m_clipPlanes[UPPER] = vec4(scaledNorm, -sqrt(3.0f));
		}
	}

	//if (m_pRender3DParam->m_pMPROverlay->isShown())
	//{
	//	render3D();
	//	scene()->update();
	//}
}

void CW3View3DMPR::Cut3D(bool bToggled, VRCutTool cut_tool)
{
	cut_tool_ = cut_tool;
	if (bToggled)
	{
		m_pPerspectiveOnOff->setCurrentState(false);
		event_type_ = MPR3DEventType::VRCUT_DRAW;
	}
	else
	{
		event_type_ = MPR3DEventType::NONE;
	}
	DeleteVRCutUI();
}

void CW3View3DMPR::setTranslateMatSecondVolume(glm::mat4 *translate)
{
	m_translateSecond = *translate;
	setModel();
}

void CW3View3DMPR::setRotateMatSecondVolume(glm::mat4 *rotate)
{
	m_rotateSecond = *rotate;
	setModel();
}

void CW3View3DMPR::SetZoom3DVR(const MPRViewType eViewType,
	const glm::vec3 center, const float radius)
{
	m_fRadius = radius;
	zoom_center_ = center;

	render3D();
	scene()->update();
}

void CW3View3DMPR::setZoom3DCube()
{
	if (m_fRadius <= 0.0f) return;

	const float cubeHalf = m_fRadius / sqrt(3.0f);
	m_pGLWidget->makeCurrent();
	glm::vec3 ptVol[8] = { zoom_center_ + vec3(-cubeHalf, cubeHalf, -cubeHalf),
						  zoom_center_ + vec3(-cubeHalf, cubeHalf, cubeHalf),
						  zoom_center_ + vec3(-cubeHalf, -cubeHalf, -cubeHalf),
						  zoom_center_ + vec3(-cubeHalf, -cubeHalf, cubeHalf),
						  zoom_center_ + vec3(cubeHalf, cubeHalf, -cubeHalf),
						  zoom_center_ + vec3(cubeHalf, cubeHalf, cubeHalf),
						  zoom_center_ + vec3(cubeHalf, -cubeHalf, -cubeHalf),
						  zoom_center_ + vec3(cubeHalf, -cubeHalf, cubeHalf) };

	vec3 p1(ptVol[0].x / m_vVolRange.x, ptVol[0].y / m_vVolRange.y,
		ptVol[0].z / m_vVolRange.z);
	vec3 p2(ptVol[1].x / m_vVolRange.x, ptVol[1].y / m_vVolRange.y,
		ptVol[1].z / m_vVolRange.z);
	vec3 p3(ptVol[2].x / m_vVolRange.x, ptVol[2].y / m_vVolRange.y,
		ptVol[2].z / m_vVolRange.z);
	vec3 p4(ptVol[3].x / m_vVolRange.x, ptVol[3].y / m_vVolRange.y,
		ptVol[3].z / m_vVolRange.z);

	vec3 p5(ptVol[4].x / m_vVolRange.x, ptVol[4].y / m_vVolRange.y,
		ptVol[4].z / m_vVolRange.z);
	vec3 p6(ptVol[5].x / m_vVolRange.x, ptVol[5].y / m_vVolRange.y,
		ptVol[5].z / m_vVolRange.z);
	vec3 p7(ptVol[6].x / m_vVolRange.x, ptVol[6].y / m_vVolRange.y,
		ptVol[6].z / m_vVolRange.z);
	vec3 p8(ptVol[7].x / m_vVolRange.x, ptVol[7].y / m_vVolRange.y,
		ptVol[7].z / m_vVolRange.z);

	float tex[24] = { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y,
					 p3.z, p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, p6.x,
					 p6.y, p6.z, p7.x, p7.y, p7.z, p8.x, p8.y, p8.z };

	float vert[24] = { 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  -1.0f,
					  -1.0f, 1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,
					  1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f };

	if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
	{
		glm::vec3 range(1.0f);
		m_scaleMat = glm::scale(range);
		m_inverseScale = glm::scale(range);
		m_camFOV = glm::length(range);

		setModel();
		setViewMatrix();
		setProjection();

		setMVP();

		CW3GLFunctions::initVBO(m_vboCUBE, vert, tex, 24, kZoomCubeIndex, 36);
		CW3GLFunctions::initVAO(&m_pRender3DParam->m_mainVolume_vao[0], m_vboCUBE);
	}
	else if (m_eViewType == common::ViewTypeID::MPR_3D)
	{
		for (int i = 0; i < 24; i++)
		{
			vert[i] = (tex[i] * 2.0f) - 1.0f;

			if (i % 3 == 0) vert[i] *= -1.0f;
		}

		// 1
		m_vertCube[0] = vert[0];
		m_vertCube[1] = vert[1];
		m_vertCube[2] = vert[2];
		m_vertCube[3] = vert[3];
		m_vertCube[4] = vert[4];
		m_vertCube[5] = vert[5];

		m_vertCube[6] = vert[3];
		m_vertCube[7] = vert[4];
		m_vertCube[8] = vert[5];
		m_vertCube[9] = vert[9];
		m_vertCube[10] = vert[10];
		m_vertCube[11] = vert[11];

		m_vertCube[12] = vert[9];
		m_vertCube[13] = vert[10];
		m_vertCube[14] = vert[11];
		m_vertCube[15] = vert[6];
		m_vertCube[16] = vert[7];
		m_vertCube[17] = vert[8];

		m_vertCube[18] = vert[6];
		m_vertCube[19] = vert[7];
		m_vertCube[20] = vert[8];
		m_vertCube[21] = vert[0];
		m_vertCube[22] = vert[1];
		m_vertCube[23] = vert[2];

		// 2
		m_vertCube[24] = vert[12];
		m_vertCube[25] = vert[13];
		m_vertCube[26] = vert[14];
		m_vertCube[27] = vert[15];
		m_vertCube[28] = vert[16];
		m_vertCube[29] = vert[17];

		m_vertCube[30] = vert[15];
		m_vertCube[31] = vert[16];
		m_vertCube[32] = vert[17];
		m_vertCube[33] = vert[21];
		m_vertCube[34] = vert[22];
		m_vertCube[35] = vert[23];

		m_vertCube[36] = vert[21];
		m_vertCube[37] = vert[22];
		m_vertCube[38] = vert[23];
		m_vertCube[39] = vert[18];
		m_vertCube[40] = vert[19];
		m_vertCube[41] = vert[20];

		m_vertCube[42] = vert[18];
		m_vertCube[43] = vert[19];
		m_vertCube[44] = vert[20];
		m_vertCube[45] = vert[12];
		m_vertCube[46] = vert[13];
		m_vertCube[47] = vert[14];

		// 3
		m_vertCube[48] = vert[0];
		m_vertCube[49] = vert[1];
		m_vertCube[50] = vert[2];
		m_vertCube[51] = vert[12];
		m_vertCube[52] = vert[13];
		m_vertCube[53] = vert[14];

		m_vertCube[54] = vert[3];
		m_vertCube[55] = vert[4];
		m_vertCube[56] = vert[5];
		m_vertCube[57] = vert[15];
		m_vertCube[58] = vert[16];
		m_vertCube[59] = vert[17];

		m_vertCube[60] = vert[9];
		m_vertCube[61] = vert[10];
		m_vertCube[62] = vert[11];
		m_vertCube[63] = vert[21];
		m_vertCube[64] = vert[22];
		m_vertCube[65] = vert[23];

		m_vertCube[66] = vert[6];
		m_vertCube[67] = vert[7];
		m_vertCube[68] = vert[8];
		m_vertCube[69] = vert[18];
		m_vertCube[70] = vert[19];
		m_vertCube[71] = vert[20];

		if (m_vboDrawCube)
		{
			glDeleteBuffers(1, &m_vboDrawCube);
			m_vboDrawCube = 0;
		}

		glGenBuffers(1, &m_vboDrawCube);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboDrawCube);
		glBufferData(GL_ARRAY_BUFFER, 24 * 3 * sizeof(float), m_vertCube,
			GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (m_vaoDrawCube)
		{
			glDeleteVertexArrays(1, &m_vaoDrawCube);
			m_vaoDrawCube = 0;
		}

		glGenVertexArrays(1, &m_vaoDrawCube);
		glBindVertexArray(m_vaoDrawCube);

		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboDrawCube);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	m_pGLWidget->doneCurrent();
}

bool CW3View3DMPR::offMIPOverlayWhereOnMIP(bool isMIP)
{
	if (isMIP && m_pMPROverlayOnOff->getCurrentState())
	{
		m_pMPROverlayOnOff->setCurrentState(false);
		return true;
	}
	else
		return false;
}

void CW3View3DMPR::InitMPROverlaySwitch()
{
	if (m_eViewType != common::ViewTypeID::MPR_3D) return;

	QString sectionKey = "MPR/mpr_overlay_on";
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	bool visibleMPROverlay = settings.value(sectionKey).toBool();
	m_pMPROverlayOnOff->setCurrentState(visibleMPROverlay);
}

void CW3View3DMPR::SaveMPROverlaySwitch()
{
	if (m_eViewType != common::ViewTypeID::MPR_3D) return;

	QString sectionKey = "MPR/mpr_overlay_on";
	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
	settings.setValue(sectionKey, m_pMPROverlayOnOff->getCurrentState());
}

bool CW3View3DMPR::IsControllerTextOnMousePress()
{
	// VR cut UI 를 그리는 중에 text의 방해를 받지 않기 위한 코드
	bool bHoveredAlign = false;

	bHoveredAlign = CW3View3D_forMPR::IsControllerTextOnMousePress();

	if (m_pPerspectiveOnOff->isUnderMouse() ||
		m_pMPROverlayOnOff->isUnderMouse() || bHoveredAlign)
	{
		return true;
	}

	return false;
}

bool CW3View3DMPR::IsCurrentDrawingVRCut()
{
	if (event_type_ == MPR3DEventType::VRCUT_DRAW && !vr_cut_polygon_)
		return true;
	return false;
}

void CW3View3DMPR::SelectVRCutAreaInPressNew()
{
	if (!vr_cut_mask_vol_)
		CreateNewVRCutMaskVolume(m_vVolRange.x, m_vVolRange.y, m_vVolRange.z);

	QPolygonF cut_area = vr_cut_polygon_->polygon();
	const int width_view = width();
	const int height_view = height();

	bool *contain = nullptr;
	W3::p_allocate_1D(&contain, width_view * height_view);
	memset(contain, false, sizeof(bool) * width_view * height_view);

	clock_t start_time = clock();

	QPainterPath pp_cut_area;
	pp_cut_area.addPolygon(cut_area);
	const int num_thread = omp_get_max_threads();
	omp_set_num_threads(num_thread);
#pragma omp parallel for
	for (int i = 0; i < height_view; i++)
	{
		for (int j = 0; j < width_view; j++)
		{
			if (is_cut_inside_)
			{
				contain[i * width_view + j] =
					pp_cut_area.contains(mapToScene(j, i));
			}
			else
			{
				contain[i * width_view + j] =
					!pp_cut_area.contains(mapToScene(j, i));
			}
		}
	}

	clock_t end_time = clock();
	float elapsed_time = static_cast<float>(end_time - start_time);
	qDebug() << "2D bool mask elapsed_time :" << elapsed_time;

	glm::mat4 mv = m_view * m_model;
	glm::mat4 p = m_projection;
	QMatrix4x4 q_mv, q_p;
	for (int i = 0; i < 4; ++i)
	{
		q_mv.setColumn(i, QVector4D(mv[i][0], mv[i][1], mv[i][2], mv[i][3]));
		q_p.setColumn(i, QVector4D(p[i][0], p[i][1], p[i][2], p[i][3]));
	}
	QMatrix4x4 q_mvp = q_p * q_mv;

	unsigned short **data = vr_cut_mask_vol_->getData();
	int width_vol = vr_cut_mask_vol_->width();
	int height_vol = vr_cut_mask_vol_->height();
	int depth_vol = vr_cut_mask_vol_->depth();
	float half_width_view = static_cast<float>(width_view) * 0.5f;
	float half_height_view = static_cast<float>(height_view) * 0.5f;

	boolean pop_front_and_shift = false;
	if (cur_vr_cut_history_step_ == 15)
	{
		pop_front_and_shift = true;
		is_cut_shifted_ = true;
	}
	else if (cur_vr_cut_history_step_ < 15)
		cur_vr_cut_history_step_++;

	last_vr_cut_history_step_ = cur_vr_cut_history_step_;

	start_time = clock();

	int prev_step = cur_vr_cut_history_step_ - 1;

	omp_set_num_threads(num_thread);
#pragma omp parallel for
	for (int d = 0; d < depth_vol; ++d)
	{
		int ix, iy;
		QVector4D vh;
		float px, py, pz, window_x, window_y;
		unsigned short *slice = data[d];
		for (int h = 0; h < height_vol; ++h)
		{
			for (int w = 0; w < width_vol; ++w)
			{
				px = static_cast<float>(width_vol - w) /
					static_cast<float>(width_vol - 1) * 2.0f -
					1.0f;
				py = static_cast<float>(h) / static_cast<float>(height_vol - 1) * 2.0f -
					1.0f;
				pz = static_cast<float>(d) / static_cast<float>(depth_vol - 1) * 2.0f -
					1.0f;
				vh = q_mvp.map(QVector4D(px, py, pz, 1.0f));
				window_x = half_width_view * (vh.x() / vh.w() + 1.0f);
				window_y = half_height_view * (vh.y() / vh.w() + 1.0f);
				ix = static_cast<int>(window_x + 0.5f);
				iy = height_view - static_cast<int>(window_y + 0.5f);

				int slice_index = h * width_vol + w;

				if (pop_front_and_shift)
				{
					slice[slice_index] = slice[slice_index] >> 1;
				}

				if (ix < 0 || ix >= width_view || iy < 0 || iy >= height_view)
				{
					if (!is_cut_inside_)
						slice[slice_index] |= 0x0001 << cur_vr_cut_history_step_;
					else if (prev_step > -1 && slice[slice_index] & 0x0001 << prev_step)
						slice[slice_index] |= 0x0001 << cur_vr_cut_history_step_;
					else
						slice[slice_index] &= 0x0000 << cur_vr_cut_history_step_;
					continue;
				}

				if (contain[iy * width_view + ix] == false)
				{
					if (prev_step > -1 && slice[slice_index] & 0x0001 << prev_step)
						slice[slice_index] |= 0x0001 << cur_vr_cut_history_step_;
					else
						slice[slice_index] &= 0x0000 << cur_vr_cut_history_step_;
				}
				else
				{
					slice[slice_index] |= 0x0001 << cur_vr_cut_history_step_;
				}
			}
		}
	}

	end_time = clock();
	elapsed_time = static_cast<float>(end_time - start_time);
	qDebug() << "3D unsigned short mask elapsed_time :" << elapsed_time;

	SAFE_DELETE_ARRAY(contain);

	m_pGLWidget->makeCurrent();
	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
	m_pGLWidget->doneCurrent();

	UpdateVRCutMaskVolume(width_vol, height_vol, depth_vol, data);

	scene()->removeItem(vr_cut_polygon_);
	SAFE_DELETE_OBJECT(vr_cut_polygon_);

	DeleteVRCutPolygonArea();
	event_type_ = MPR3DEventType::VRCUT_DRAW;
}

void CW3View3DMPR::CreateNewVRCutMaskVolume(int width, int height, int depth)
{
	if (vr_cut_mask_vol_) SAFE_DELETE_OBJECT(vr_cut_mask_vol_);
	vr_cut_mask_vol_ = new CW3Image3D((unsigned int)width, (unsigned int)height,
		(unsigned int)depth);
	auto *data = vr_cut_mask_vol_->getData();
	for (int i = 0; i < depth; i++)
	{
		memset(data[i], 0, sizeof(unsigned short) * width * height);
	}

	cur_vr_cut_history_step_ = -1;
	last_vr_cut_history_step_ = -1;
	is_cut_shifted_ = false;
}

void CW3View3DMPR::DeleteVRCutMaskVolume()
{
	if (vr_cut_mask_vol_) SAFE_DELETE_OBJECT(vr_cut_mask_vol_);

	if (tex_handler_vr_cut_mask_vol_)
	{
		glDeleteTextures(1, &tex_handler_vr_cut_mask_vol_);
		tex_handler_vr_cut_mask_vol_ = 0;
	}
}

void CW3View3DMPR::UpdateVRCutMaskVolume(int width, int height, int depth,
	unsigned short **data)
{
	m_pgVREngine->makeCurrent();

	if (tex_handler_vr_cut_mask_vol_)
	{
		glDeleteTextures(1, &tex_handler_vr_cut_mask_vol_);
		tex_handler_vr_cut_mask_vol_ = 0;
	}

	CW3GLFunctions::InitVRCutVol3DTex2Dpointer(tex_handler_vr_cut_mask_vol_,
		width, height, depth, data);

	m_pgVREngine->doneCurrent();
	m_pGLWidget->makeCurrent();

#if 0
	// set vr cut mask vol texture to gl
	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);
	if (tex_handler_vr_cut_mask_vol_ > 0)
	{
		glActiveTexture(gl_tex_num_vr_cut_mask_vol_);
		glBindTexture(GL_TEXTURE_3D, tex_handler_vr_cut_mask_vol_);
		WGLSLprogram::setUniform(PROGRaycasting, "VRCutMaskVolumeTex", tex_num_vr_cut_mask_vol_);
	}
	glUseProgram(0);
	//
#endif

	m_pGLWidget->doneCurrent();
}

void CW3View3DMPR::UpdateVRCutHistoryStep(int step)
{
	m_pGLWidget->makeCurrent();

	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);

	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);

	WGLSLprogram::setUniform(PROGRaycasting, "VRCutMaskBitShift", step);

	glUseProgram(0);

	m_pGLWidget->doneCurrent();
}

void CW3View3DMPR::Render3DAndUpdateIfVisible()
{
	if (isVisible())
	{
		render3D();
		scene()->update();
	}
}

void CW3View3DMPR::CreateVRCutUIInPress(
	const QPointF &curr_scene_pos)
{
	QColor color(Qt::red);
	color.setAlphaF(1.0);

	if (!vr_cut_polygon_)
	{
		if (IsControllerTextOnMousePress()) return;

		QPen pen;
		pen.setColor(color);
		pen.setWidthF(2.0f);
		QBrush brush;
		brush.setColor(color);

		vr_cut_polygon_ = new QGraphicsPolygonItem();
		vr_cut_polygon_->setZValue(0.0f);
		vr_cut_polygon_->setPen(pen);
		vr_cut_polygon_->setBrush(brush);

		scene()->addItem(vr_cut_polygon_);
	}

	switch (cut_tool_)
	{
	case VRCutTool::POLYGON:
	{
		QPolygonF poly = vr_cut_polygon_->polygon();
		if (poly.size() == 0)
		{
			poly.append(curr_scene_pos);
		}
		poly.append(curr_scene_pos);
		vr_cut_polygon_->setPolygon(poly);
	} break;
	case VRCutTool::FREEDRAW:
		bool done = true;
		vr_cut_freedraw_ = new AnnotationFreedraw();
		vr_cut_freedraw_->InputParam(scene(), curr_scene_pos,
			glm::vec3(0.0f), done);
		vr_cut_freedraw_->SetLineColor(color);
	}
}

void CW3View3DMPR::Draw3DCutUI(QPolygonF &poly, const QPointF &curr_scene_pos)
{
	if (poly.size() > 1)
	{
		poly.last().setX(curr_scene_pos.x());
		poly.last().setY(curr_scene_pos.y());
		vr_cut_polygon_->setPolygon(poly);
	}
}

void CW3View3DMPR::Draw3DCutSelectAreaUI(QPolygonF &poly,
	const QPointF &curr_scene_pos)
{
	if (poly.size() < 2)
	{
		return;
	}

	if (poly.size() == 2)
	{
		QLineF line(QPointF(poly.at(0)), QPointF(poly.at(1)));
		vr_cut_polygon_->setPolygon(CreateCutAreaByLine(line));
	}

	QList<QPolygonF> listPoly = GetSubpathPolygons();

	QPolygonF preSelectedPolygon;
	if (vr_cut_selected_area_)
	{
		preSelectedPolygon = vr_cut_selected_area_->polygon();
	}

	DeleteVRCutPolygonArea();

	QColor colorPen(Qt::red);
	colorPen.setAlphaF(0.0f);
	QPen pen;
	pen.setWidthF(0.0f);
	pen.setColor(colorPen);

	QColor colorBrush(Qt::red);
	colorBrush.setAlphaF(0.5f);
	QBrush brush(colorBrush);
	brush.setStyle(Qt::DiagCrossPattern);

	is_cut_inside_ = false;

	QPolygonF remain;
	remain.append(QPointF(0.0f, 0.0f));
	remain.append(QPointF(width(), 0.0f));
	remain.append(QPointF(width(), height()));
	remain.append(QPointF(0.0f, height()));

	for (int i = 0; i < listPoly.size(); i++)
	{
		QPolygonF polygon = listPoly.at(i);
		if (polygon.containsPoint(curr_scene_pos, Qt::WindingFill))
		{
			vr_cut_selected_area_ = scene()->addPolygon(polygon, pen, brush);

			is_cut_inside_ = true;
			break;
		}
		else
		{
			remain = remain.subtracted(polygon);
		}
	}

	if (!is_cut_inside_)
	{
		vr_cut_selected_area_ = scene()->addPolygon(remain, pen, brush);
	}

	if (preSelectedPolygon != vr_cut_selected_area_->polygon())
	{
		std::cout << "Draw3DCutSelectAreaUI : scene update" << std::endl;
		scene()->update();
	}
}

void CW3View3DMPR::DeleteVRCutUI()
{
	if (vr_cut_polygon_)
	{
		scene()->removeItem(vr_cut_polygon_);
		SAFE_DELETE_OBJECT(vr_cut_polygon_);
	}
	if (vr_cut_freedraw_) SAFE_DELETE_OBJECT(vr_cut_freedraw_);

	DeleteVRCutPolygonArea();
}

void CW3View3DMPR::EndDraw3DCutUI()
{
	QPolygonF before = vr_cut_polygon_->polygon();
	vr_cut_polygon_->polygon().removeLast();
	QPolygonF after = vr_cut_polygon_->polygon();
	after.removeLast();
	vr_cut_polygon_->setPolygon(after);
	QPolygonF final = vr_cut_polygon_->polygon();
	if (vr_cut_polygon_->polygon().size() > 1)
	{
		event_type_ = MPR3DEventType::VRCUT_SELECT;
	}
	else
	{
		scene()->removeItem(vr_cut_polygon_);
		SAFE_DELETE_OBJECT(vr_cut_polygon_);

		event_type_ = MPR3DEventType::VRCUT_DRAW;

		scene()->update();
	}
}

QPolygonF CW3View3DMPR::CreateCutAreaByLine(const QLineF& line)
{
	QPolygonF polygon;

	float width = static_cast<float>(this->width());
	float height = static_cast<float>(this->height());

	bool intersect_with_left = false;
	bool intersect_with_right = false;
	bool intersect_with_top = false;
	bool intersect_with_bottom = false;

	float left = 0.0f;
	float right = width + 0.0f;
	float top = 0.0f;
	float bottom = height + 0.0f;

	QPointF left_top(left, top);
	QPointF right_top(right, top);
	QPointF left_bottom(left, bottom);
	QPointF right_bottom(right, bottom);

	QPointF upper_point = line.p1();
	QPointF lower_point = line.p2();
	if (line.p1().y() > line.p2().y())
	{
		upper_point = line.p2();
		lower_point = line.p1();
	}

	QVector2D normalized_dir = QVector2D(lower_point - upper_point).normalized();

	// get upper insersection point
	float upper_distance_to_view_border = std::abs(upper_point.y() - top);
	QPointF upper_intersection_point = QPointF(upper_point.x() - std::abs(upper_distance_to_view_border / normalized_dir.y()) * normalized_dir.x(), top);
	if (upper_intersection_point.x() < left)
	{
		upper_distance_to_view_border = std::abs(upper_point.x() - left);
		upper_intersection_point = QPointF(left, upper_point.y() - std::abs(upper_distance_to_view_border / normalized_dir.x()) * normalized_dir.y());
		intersect_with_left = true;

	}
	else if (upper_intersection_point.x() > right)
	{
		upper_distance_to_view_border = std::abs(upper_point.x() - right);
		upper_intersection_point = QPointF(right, upper_point.y() - std::abs(upper_distance_to_view_border / normalized_dir.x()) * normalized_dir.y());
		intersect_with_right = true;
	}
	else
	{
		intersect_with_top = true;
	}

	if (intersect_with_right)
	{
		polygon.append(left_top);
		polygon.append(right_top);
		intersect_with_right = false;
	}
	else if (intersect_with_top)
	{
		polygon.append(left_top);
	}
	polygon.append(upper_intersection_point);

	// get lower insersection point
	float lower_distance_to_view_border = std::abs(lower_point.y() - bottom);
	QPointF lower_intersection_point = QPointF(lower_point.x() + std::abs(lower_distance_to_view_border / normalized_dir.y()) * normalized_dir.x(), bottom);
	if (lower_intersection_point.x() < left)
	{
		lower_distance_to_view_border = std::abs(lower_point.x() - left);
		lower_intersection_point = QPointF(left, lower_point.y() + std::abs(lower_distance_to_view_border / normalized_dir.x()) * normalized_dir.y());
		intersect_with_left = true;

	}
	else if (lower_intersection_point.x() > right)
	{
		lower_distance_to_view_border = std::abs(lower_point.x() - right);
		lower_intersection_point = QPointF(right, lower_point.y() + std::abs(lower_distance_to_view_border / normalized_dir.x()) * normalized_dir.y());
		intersect_with_right = true;
	}
	else
	{
		intersect_with_bottom = true;
	}

	polygon.append(lower_intersection_point);
	if (intersect_with_right)
	{
		polygon.append(right_bottom);
		polygon.append(left_bottom);
	}
	else if (intersect_with_bottom)
	{
		polygon.append(left_bottom);
	}

#if 0
	polygon.clear();
	polygon.append(QPointF(0.0f, 0.0f));
	polygon.append(upper_intersection_point);
	polygon.append(lower_intersection_point);
	polygon.append(QPointF(0.0f, height));
#endif

	return polygon;
}

void CW3View3DMPR::render3D()
{
	if (!isReadyRender3D()) return;

	if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
	{
		m_model = m_rotate * m_origModel * m_scaleMat;
	}

	if (m_is3Dready)
	{
		if (m_bIsZoom3DMode) setZoom3DCube();

		m_pGLWidget->makeCurrent();
		if (m_pRender3DParam->m_pMPROverlay->isShown())
		{
			setProjection();
			setMVP();
			m_pRender3DParam->m_pMPROverlay->setTransformMat(m_rotate, ARCBALL);
			m_pRender3DParam->m_pMPROverlay->setTransformMat(m_origModel,
				REORIENTATION);
			m_pRender3DParam->m_pMPROverlay->setProjViewMat(m_projection, m_view);

			unsigned int PROGslice = m_pgVREngine->getPROGslice();
			glUseProgram(PROGslice);
			auto &vol = ResourceContainer::GetInstance()->GetMainVolume();
			m_nAdjustWindowLevel = vol.windowCenter();
			m_nAdjustWindowWidth = vol.windowWidth();
			WGLSLprogram::setUniform(PROGslice, "WindowLevel",
				(float)m_nAdjustWindowLevel /
				(float)m_pgVREngine->getVol(0)->getMax());
			WGLSLprogram::setUniform(PROGslice, "WindowWidth",
				(float)m_nAdjustWindowWidth /
				(float)m_pgVREngine->getVol(0)->getMax());
			glUseProgram(0);
		}

#if 1
		// set vr cut mask vol texture to gl
		unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
		glUseProgram(PROGRaycasting);

		glActiveTexture(gl_tex_num_vr_cut_mask_vol_);
		glBindTexture(GL_TEXTURE_3D, tex_handler_vr_cut_mask_vol_);
		WGLSLprogram::setUniform(PROGRaycasting, "VRCutMaskVolumeTex",
			tex_num_vr_cut_mask_vol_);

		if (tex_handler_vr_cut_mask_vol_ > 0)
		{
			WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", true);
			WGLSLprogram::setUniform(PROGRaycasting, "VRCutMaskBitShift",
				cur_vr_cut_history_step_);			
		}
		else
		{
			WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", false);
		}
		glUseProgram(0);
		//
#endif

		m_pGLWidget->doneCurrent();

		if (m_bIsObliqueMode)
		{
			vec4 clipPlane = glm::scale(vec3(-1.0, 1.0, 1.0)) *
				glm::inverse(m_rotate * m_origModel) *
				vec4(0.0, 1.0f, 0.0, 0.0);
			clipPlane.w = m_fObiquePlanePos;

			m_pRender3DParam->m_clipPlanes[0] = clipPlane;
		}
	}

	CW3View3D_forMPR::render3D();
}

void CW3View3DMPR::init()
{
	CW3View3D_forMPR::init();

	if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
		m_origModel = glm::mat4(1.0);

	if (!m_pRender3DParam->m_pMPROverlay)
		m_pRender3DParam->m_pMPROverlay = new CW3SurfacePlaneItem();

	m_pRender3DParam->m_pMPROverlay->setVertexCoord(sqrt(2.0f));
	m_pRender3DParam->m_pMPROverlay->setTransformMat(glm::scale(m_vVolRange),
		SCALE);
	m_pRender3DParam->m_pMPROverlay->addPlane(QString("MPROverlay"),
		vec4(0.0f, 0.0f, 1.0f, 0.0f));

	unsigned int progSurface = m_pgVREngine->getPROGsurface();
	glUseProgram(progSurface);

	float camFOV = glm::length(m_vVolRange);
	WGLSLprogram::setUniform(progSurface, "Light.Intensity", vec3(1.0f));
	vec4 lightPos = glm::scale(m_vVolRange) * vec4(0.0f, -10.0f, 0.0f, 1.0f);
	WGLSLprogram::setUniform(
		progSurface, "Light.Position",
		glm::lookAt(glm::vec3(0.0f, -camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)) *
		lightPos);

	glUseProgram(0);

	if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
	{
		m_pMPROverlayOnOff->setVisible(false);
		m_pPerspectiveOnOff->setVisible(false);
	}
	else
	{
		m_pMPROverlayOnOff->setVisible(true);
		m_pPerspectiveOnOff->setVisible(true);
	}

	InitMPROverlaySwitch();
	border_->setVisible(true);
}

void CW3View3DMPR::clearGL()
{
	if (m_pGLWidget)
	{
		m_pGLWidget->makeCurrent();

		if (m_pRender3DParam->m_pMPROverlay)
			m_pRender3DParam->m_pMPROverlay->clearVAOVBO();

		if (m_vaoDrawCube)
		{
			glDeleteVertexArrays(1, &m_vaoDrawCube);
			m_vaoDrawCube = 0;
		}
		if (m_vboDrawCube)
		{
			glDeleteBuffers(1, &m_vboDrawCube);
			m_vboDrawCube = 0;
		}
		if (m_vboCUBE[0])
		{
			glDeleteBuffers(3, m_vboCUBE);
			m_vboCUBE[0] = 0;
			m_vboCUBE[1] = 0;
			m_vboCUBE[2] = 0;
		}

		m_pGLWidget->doneCurrent();
	}

	CW3View3D_forMPR::clearGL();
}

void CW3View3DMPR::zoom3D(bool bToggled)
{
	if (!isVisible()) return;

	m_bIsZoom3DMode = bToggled;
	m_fRadius = 0.0f;

	if (m_bIsZoom3DMode) return;

	if (m_is3Dready && m_pgVREngine->isVRready())
	{
		clearGL();

		if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
		{
			m_pGLWidget->makeCurrent();
			m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
			m_pGLWidget->doneCurrent();
			ResetView();
		}
		else
		{
			render3D();
		}

		scene()->update();
	}
}

bool CW3View3DMPR::IsDefaultInteractionsIn3D(Qt::MouseButton button)
{
	if ((button == (Qt::LeftButton | Qt::RightButton)) ||
		button == Qt::RightButton)
		return true;
	return false;
}

bool CW3View3DMPR::IsDefaultInteractionsIn3D(Qt::MouseButtons buttons)
{
	if ((buttons == (Qt::LeftButton | Qt::RightButton)) ||
		buttons == Qt::RightButton)
		return true;
	return false;
}

void CW3View3DMPR::ClipEnable(int isEnable)
{
	m_pMPROverlayOnOff->setCurrentState(false);

	if (m_bToggledClipMPROverlay) setEnableClipMPRMode(isEnable);

	CW3View3D_forMPR::ClipEnable(isEnable);
}

void CW3View3DMPR::ClipRangeMove(int lower, int upper)
{
	if (m_pRender3DParam->m_isClipped && m_bToggledClipMPROverlay)
	{
#if 1
		return;
#else
		m_pRender3DParam->m_isClipped = false;

		CW3View3D_forMPR::ClipRangeMove(lower, upper);

		editMPROverlayFromClipPlane();
		m_pRender3DParam->m_isClipped = true;
#endif
		Render3DAndUpdateIfVisible();
	}
	else
	{
		CW3View3D_forMPR::ClipRangeMove(lower, upper);
	}
}

void CW3View3DMPR::ClipPlaneChanged(const MPRClipID &clip_plane)
{
	if (!m_pRender3DParam->m_isClipped) return;

	if (clip_plane == MPRClipID::MPROVERLAY)
	{
		m_bToggledClipMPROverlay = true;
		m_pMPROverlayOnOff->setCurrentState(true);
#if 0
		setClippingValueFromMPROverlay();
		editDistanceMPROverlayFromClipPlane();
		render3D();
		scene()->update();
#endif
	}
	else
	{
		m_bToggledClipMPROverlay = false;
		m_pMPROverlayOnOff->setCurrentState(false);
		CW3View3D_forMPR::ClipPlaneChanged(clip_plane);
	}
}

void CW3View3DMPR::ResetView()
{
	CW3View3D_forMPR::ResetView();

	if (m_bIsOnlyTRDMode) scene()->update();

	if (!isReadyRender3D()) return;

	if (m_pRender3DParam->m_pMPROverlay &&
		m_pRender3DParam->m_clipPlanes.size() > 0)
	{
		emit sigRequestClipStatus();
	}
}

void CW3View3DMPR::FitView()
{
	CW3View3D_forMPR::FitView();

	if (m_bIsOnlyTRDMode) scene()->update();
}

void CW3View3DMPR::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (event->key() == Qt::Key_Escape)
	{
		if (event_type_ == MPR3DEventType::NONE) return;

		DeleteVRCutUI();
		event_type_ = MPR3DEventType::VRCUT_DRAW;
		scene()->update();
	}

	emit sigKeyPressEvent(event);

	CW3View3D_forMPR::keyPressEvent(event);
}

void CW3View3DMPR::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE
	CW3View3D_forMPR::keyReleaseEvent(event);
}

void CW3View3DMPR::setOnlyTRDMode()
{
	m_pRender3DParam->m_isPerspective = true;
	m_bIsOnlyTRDMode = true;
	setInitScale();

	ruler_->setVisible(false);

	HideUI(true);
}

void CW3View3DMPR::setMIP(bool isMIP)
{
	m_pRender3DParam->m_isMIP = isMIP;

	offMIPOverlayWhereOnMIP(isMIP);
	Render3DAndUpdateIfVisible();
}

void CW3View3DMPR::setEnableClipMPRMode(bool isEnable)
{
	if (isEnable) editDistanceMPROverlayFromClipPlane();

	m_pMPROverlayOnOff->setCurrentState(isEnable);
}

glm::vec4 CW3View3DMPR::getClipPlaneNearbyCenter()
{
	glm::vec4 clipPlane;

	if (abs(m_pRender3DParam->m_clipPlanes[UPPER].w) >
		abs(m_pRender3DParam->m_clipPlanes[LOWER].w))
	{
		clipPlane = m_pRender3DParam->m_clipPlanes[LOWER];
	}
	else
	{
		clipPlane = m_pRender3DParam->m_clipPlanes[UPPER];
	}

	if (!m_pRender3DParam->flip_clipping_)
	{
		clipPlane.w *= -1.0f;
	}

	return clipPlane;
}

void CW3View3DMPR::editDistanceMPROverlayFromClipPlane()
{
	glm::vec4 clipPlane = getClipPlaneNearbyCenter();

	vec3 normal(m_pRender3DParam->m_pMPROverlay->getPlaneEquation("MPROverlay"));
	m_pRender3DParam->m_pMPROverlay->editPlane("MPROverlay",
		vec4(normal, clipPlane.w));
}

void CW3View3DMPR::editMPROverlayFromClipPlane()
{
	glm::vec4 clipPlane = getClipPlaneNearbyCenter();

	// MPR Overlay plane
	vec3 invScale = m_pRender3DParam->m_pgMainVolume[0]->m_invVolTexScale;
	vec3 norm =
		glm::normalize(vec3(-1.0f, 1.0f, 1.0f) * clipPlane.xyz * invScale);

	m_pRender3DParam->m_pMPROverlay->editPlane("MPROverlay",
		vec4(norm, clipPlane.w));
}

void CW3View3DMPR::setClippingValueFromMPROverlay()
{
	vec3 normal(m_pRender3DParam->m_pMPROverlay->getPlaneEquation("MPROverlay"));
	vec3 scale = m_pRender3DParam->m_pgMainVolume[0]->m_voltexScale;
	vec3 scaledNorm = glm::normalize(vec3(-1.0f, 1.0f, 1.0f) * normal * scale);

	const int OVERLAY = 0;
	const int CLIPPING = 1;
	glm::vec4 clipPlane[2];

	if (abs(m_pRender3DParam->m_clipPlanes[UPPER].w) >
		abs(m_pRender3DParam->m_clipPlanes[LOWER].w))
	{
		clipPlane[OVERLAY] = m_pRender3DParam->m_clipPlanes[LOWER];
		clipPlane[CLIPPING] = m_pRender3DParam->m_clipPlanes[UPPER];

		m_pRender3DParam->m_clipPlanes[UPPER] =
			vec4(-scaledNorm, clipPlane[CLIPPING].w);
		m_pRender3DParam->m_clipPlanes[LOWER] =
			vec4(scaledNorm, clipPlane[OVERLAY].w);
	}
	else
	{
		clipPlane[OVERLAY] = m_pRender3DParam->m_clipPlanes[UPPER];
		clipPlane[CLIPPING] = m_pRender3DParam->m_clipPlanes[LOWER];

		m_pRender3DParam->m_clipPlanes[LOWER] =
			vec4(-scaledNorm, clipPlane[CLIPPING].w);
		m_pRender3DParam->m_clipPlanes[UPPER] =
			vec4(scaledNorm, clipPlane[OVERLAY].w);
	}
}

void CW3View3DMPR::Reset3DCut()
{
	cur_vr_cut_history_step_ = -1;
	last_vr_cut_history_step_ = -1;
	DeleteVRCutMaskVolume();
	DeleteVRCutUI();

	m_pGLWidget->makeCurrent();
	unsigned int PROGRaycasting = m_pgVREngine->getPROGRayCasting();
	glUseProgram(PROGRaycasting);
	WGLSLprogram::setUniform(PROGRaycasting, "useVRCut", false);
	glActiveTexture(gl_tex_num_vr_cut_mask_vol_);
	glBindTexture(GL_TEXTURE_3D, tex_handler_vr_cut_mask_vol_);
	WGLSLprogram::setUniform(PROGRaycasting, "VRCutMaskVolumeTex",
		tex_num_vr_cut_mask_vol_);
	glUseProgram(0);

	m_pgVREngine->setActiveIndex(&m_pRender3DParam->m_mainVolume_vao[0], 0);
	m_pGLWidget->doneCurrent();

	if (event_type_ == MPR3DEventType::VRCUT_SELECT)
		event_type_ = MPR3DEventType::VRCUT_DRAW;

	Render3DAndUpdateIfVisible();
}

void CW3View3DMPR::Undo3DCut()
{
	if ((cur_vr_cut_history_step_ == 0 && !is_cut_shifted_) ||
		cur_vr_cut_history_step_ > 0)
		cur_vr_cut_history_step_--;

	UpdateVRCutHistoryStep(cur_vr_cut_history_step_);
	render3D();
	qDebug() << "cur_vr_cut_history_step_ :" << cur_vr_cut_history_step_
		<< ", last_vr_cut_history_step_ :" << last_vr_cut_history_step_;
	scene()->update();
}

void CW3View3DMPR::Redo3DCut()
{
	if (cur_vr_cut_history_step_ < 15 &&
		cur_vr_cut_history_step_ < last_vr_cut_history_step_)
		cur_vr_cut_history_step_++;

	UpdateVRCutHistoryStep(cur_vr_cut_history_step_);
	render3D();
	qDebug() << "cur_vr_cut_history_step_ :" << cur_vr_cut_history_step_
		<< ", last_vr_cut_history_step_ :" << last_vr_cut_history_step_;
	scene()->update();
}

QList<QPolygonF> CW3View3DMPR::GetSubpathPolygons()
{
	QPainterPath pp, winding_pp;
	pp.addPolygon(vr_cut_polygon_->polygon());

	pp.setFillRule(Qt::WindingFill);
	winding_pp = pp.simplified();

	pp.setFillRule(Qt::OddEvenFill);
	pp = pp.simplified();

	QList<QPolygonF> polygons = pp.toSubpathPolygons();
	QPolygonF inside_polygon =
		winding_pp.toFillPolygon().subtracted(pp.toFillPolygon());
	if (!inside_polygon.isEmpty()) polygons.append(inside_polygon);

	return polygons;
}

void CW3View3DMPR::SetVisibleItems()
{
	CW3View3D_forMPR::SetVisibleItems();
	bool is_text_visible = !(hide_all_view_ui_);

	border_->setVisible(!hide_all_view_ui_);
	if (m_eViewType == common::ViewTypeID::MPR_ZOOM3D)
	{
		m_pMPROverlayOnOff->setVisible(false);
		m_pPerspectiveOnOff->setVisible(false);
	}
	else
	{
		m_pMPROverlayOnOff->setVisible(is_text_visible);
		m_pPerspectiveOnOff->setVisible(is_text_visible);
	}
}

void CW3View3DMPR::DeleteVRCutPolygonArea()
{
	if (vr_cut_selected_area_)
	{
		scene()->removeItem(vr_cut_selected_area_);
		SAFE_DELETE_OBJECT(vr_cut_selected_area_);
	}
}

void CW3View3DMPR::setInitScale()
{
	if (m_bIsOnlyTRDMode)
	{
		m_initScale = 1.5f;
		m_scale = m_initScale;
	}
	else
	{
		CW3View3D_forMPR::setInitScale();
	}
}

void CW3View3DMPR::VisibleFace(int state)
{
	CW3View3D_forMPR::VisibleFace(state);

	if (state)
	{
		m_pMPROverlayOnOff->setCurrentState(false);
	}
}

void CW3View3DMPR::SetFlipClipping(int state)
{
	m_pRender3DParam->flip_clipping_ = static_cast<bool>(state);

	if (m_bToggledClipMPROverlay)
	{
		glm::vec4 lower_plane = m_pRender3DParam->m_clipPlanes[LOWER];
		glm::vec4 upper_plane = m_pRender3DParam->m_clipPlanes[UPPER];
		if (m_pRender3DParam->flip_clipping_)
		{
			m_pRender3DParam->m_clipPlanes[LOWER] = glm::vec4(glm::vec3(lower_plane), upper_plane.w);
			m_pRender3DParam->m_clipPlanes[UPPER] = glm::vec4(glm::vec3(upper_plane), -lower_plane.w);
		}
		else
		{
			m_pRender3DParam->m_clipPlanes[LOWER] = glm::vec4(glm::vec3(lower_plane), -upper_plane.w);
			m_pRender3DParam->m_clipPlanes[UPPER] = glm::vec4(glm::vec3(upper_plane), lower_plane.w);
		}

	}

	ClipRangeSet();
}
