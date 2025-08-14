#include "W3ViewMPR.h"
/*=========================================================================

File:			class CW3ViewMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-23
Last modify:	2016-04-21

=========================================================================*/
#include <qgraphicsproxywidget.h>
#include <qmath.h>
#include <QApplication>
#include <QDebug>
#include <QTextCodec>
#include <QTime>
#include <QDirIterator>

#include <Engine/Common/Common/global_preferences.h>
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"

#include <Engine/Resource/Resource/implant_resource.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/lightbox_resource.h>
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_mpr.h"
#include "../../Core/W3ProjectIO/project_io_si.h"
#endif
#include "../UIGLObjects/W3GLNerve.h"
#include <Engine/UIModule/UIPrimitive/image_filter_selector.h>
#include "../UIPrimitive/W3LineItem_MPR.h"
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3TextItem_ImplantID.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/ellipse_mpr.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIPrimitive/zoom_3d_cube.h"
#include <Engine/UIModule/UIPrimitive/implant_handle.h>
#include <Engine/UIModule/UIPrimitive/pano_arch_item.h>
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3VolumeRenderParam.h"

#define ADJUST_IMPLANT 1

namespace
{
	const float kHalfPartAngle = 22.5f;
	const float kRoundVal = 0.499999f;

	glm::vec3 kDefaultAxialUpVec = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 kDefaultAxialBackVec = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 kDefaultSagittalUpVec = glm::vec3(-1.0f, 0.0f, 0.0f);
	glm::vec3 kDefaultSagittalBackVec = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 kDefaultCoronalUpVec = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 kDefaultCoronalBackVec = glm::vec3(0.0f, 0.0f, 1.0f);
}  // end of namespace

CW3ViewMPR::CW3ViewMPR(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3ResourceContainer* Rcontainer,
	const common::ViewTypeID& eType,
	const MPRViewType& mpr_type, QWidget* pParent)
	: m_eMPRType(mpr_type), implant_handle_(new ImplantHandle()),
	CW3View2D_forMPR(VREngine, MPRengine, Rcontainer, eType, pParent)
{
	CreateSlider();

	m_pWorldAxisItem = new ViewNavigatorItem();
	scene()->addItem(m_pWorldAxisItem);

	ruler_->SetItemColor(GetViewColor());

	m_rect = new QGraphicsRectItem();
	m_rect->setZValue(0);
	scene()->addItem(m_rect);

	m_rectShadow = new QGraphicsRectItem();
	m_rectShadow->setZValue(2);
	scene()->addItem(m_rectShadow);

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	slice_number_ = new CW3TextItem(font, "0", Qt::yellow);
	slice_number_->setVisible(false);
	scene()->addItem(slice_number_);

	connect(measure_tools_, &MeasureTools::sigMeasureCreated, this, &CW3ViewMPR::sigMeasureCreated);
	connect(measure_tools_, &MeasureTools::sigMeasureDeleted, this, &CW3ViewMPR::sigMeasureDeleted);

	implant_handle_->Disable();
	CW3View2D_forMPR::drawImplantID(false);
	scene()->addItem(implant_handle_.get());

	connect(implant_handle_.get(), SIGNAL(sigTranslate()), this, SLOT(slotTranslateImplant()));
	connect(implant_handle_.get(), SIGNAL(sigRotate(float)), this, SLOT(slotRotateImplant(float)));

	arch_ = new PanoArchItem();
	arch_->SetDisplayMode(PanoArchItem::DisplayMode::IMPLANT);
	arch_->setZValue(5.0);
	connect(arch_, SIGNAL(sigEndEdit()), this, SLOT(slotArchEndEdit()));
	scene()->addItem(arch_);
}

CW3ViewMPR::~CW3ViewMPR()
{
	SAFE_DELETE_OBJECT(arch_);
}

#ifndef WILL3D_VIEWER
void CW3ViewMPR::exportProject(ProjectIOMPR& out)
{
	CW3View2D_forMPR::exportProject(out.GetViewIO());
	out.SaveThicknessInterval(m_fThickness[0], interval_slider_);
	out.SaveSecondTransformMatrix(m_secondTransform);

#if 1
	QPointF cross_controller_center_pos(m_crossCenter.x() / width(), m_crossCenter.y() / height());
	out.SaveCrossController(cross_controller_center_pos, m_pLine[0]->getAngleDegree());

	qDebug() << m_pntCurViewCenterinScene << m_crossCenter << cross_controller_center_pos << m_pLine[0]->getAngleDegree();
	qDebug() << "save :" << m_pLine[0]->pos() << width() << height() << m_SceneGLOffset;
#else
	QPointF cross_controller_center_pos_delta = m_pntCurViewCenterinScene - m_crossCenter;
	out.SaveCrossControllerDelta(cross_controller_center_pos_delta, m_pLine[0]->getAngleDegree());

	qDebug() << m_pntCurViewCenterinScene << m_crossCenter << cross_controller_center_pos_delta << m_pLine[0]->getAngleDegree();
#endif
}

void CW3ViewMPR::importProject(ProjectIOMPR& in)
{
	CW3View2D_forMPR::importProject(in.GetViewIO());
	float thickness = 0.0f, interval = 0.0f;
	in.LoadThicknessInterval(thickness, interval);
	// Do something with thickness & interval
	in.LoadSecondTransformMatrix(m_secondTransform);

	float cross_controller_rotate_angle = 0.0f;
	if (in.LoadCrossController(m_crossCenter, cross_controller_rotate_angle))
	{
		m_crossCenter.setX(m_crossCenter.x() * width());
		m_crossCenter.setY(m_crossCenter.y() * height());
	}
	else
	{
		m_crossCenter = m_pntCurViewCenterinScene;
		import_proj_info_.is_import = false;
	}

#if 0
	QPointF cross_controller_center_pos_delta;
	in.LoadCrossControllerDelta(cross_controller_center_pos_delta, cross_controller_rotate_angle);
	if (cross_controller_center_pos_delta.x() != 0.0f && cross_controller_center_pos_delta.y() != 0.0f)
	{
		m_crossCenter = m_pntCurViewCenterinScene - cross_controller_center_pos_delta;
	}
	qDebug() << cross_controller_center_pos_delta;
#endif

	float radian = cross_controller_rotate_angle * M_PI / 180.0f;

	qDebug() << m_pntCurViewCenterinScene << m_crossCenter << cross_controller_rotate_angle;

	if (m_pgMPRengine && m_pViewPlane[0])
	{
		glm::vec3 vD = *m_pgMPRengine->getMPRrotCenterInVol(0) - m_pViewPlane[0]->getPlaneCenterInVol();
		QPointF delta(scaledVolToScene(glm::dot(m_pViewPlane[0]->getRightVec(), vD)), scaledVolToScene(glm::dot(m_pViewPlane[0]->getBackVec(), vD)));

		m_crossCenter = m_pntCurViewCenterinScene + delta - m_sceneTrans;
	}

#if 0
	last_scene_pos_ = m_pntCurViewCenterinScene;
	ChangeCircleInMove(cross_controller_center_pos);
#endif

#if 0
	last_scene_pos_ = m_pntCurViewCenterinScene;
	m_crossCenter = cross_controller_center_pos;

	rotateItems(radian);
	setPosItemsUseCrossCenter();
#endif

	m_bLoadProject = true;

	rotateItems(radian);
	setPosItemsUseCrossCenter();
	SetNavigatorDirection();

	qDebug() << "load :" << m_pLine[0]->pos() << width() << height() << m_SceneGLOffset;
}

void CW3ViewMPR::exportProject(ProjectIOSI& out)
{
	CW3View2D_forMPR::exportProject(out.GetViewIO());
	out.SaveSecondTransformMatrix(m_secondTransform);
}

void CW3ViewMPR::importProject(ProjectIOSI& in)
{
	CW3View2D_forMPR::importProject(in.GetViewIO());
	in.LoadSecondTransformMatrix(m_secondTransform);
	m_bLoadProject = true;
}
#endif
//////////////////////////////////////////////////////////////////////////
//	public functions
//////////////////////////////////////////////////////////////////////////
void CW3ViewMPR::reset()
{
	CW3View2D_forMPR::reset();

	m_marginCenterSqr = 0.0f;

	m_crossCenter.setX(0.0f);
	m_crossCenter.setY(0.0f);

	if (implant_id_text_)
	{
		implant_id_text_->ResetHoverIDs();
	}

	if (m_pZoom3D)
	{
		SAFE_DELETE_OBJECT(m_pZoom3D);
	}

	m_bIsZoom3DMode = false;

	for (int i = 0; i < UILineType::LINE_END; ++i)
	{
		if (m_pLine[i])
		{
			m_pLine[i]->setVisibleLines(false);
			m_pLine[i]->setVisibleThickness(false);
		}
	}

	circle_->SetVisible(false);

	m_rect->setVisible(false);
	m_rectShadow->setVisible(false);

	if (slice_number_)
	{
		slice_number_->setVisible(false);
	}

	ApplyPreferences();
}

void CW3ViewMPR::initViewPlane(int id)
{
	if (!m_pViewPlane[id])
	{
		m_pViewPlane[id] = new CW3ViewPlane();
	}

	if (id == 0)
	{
		initVectorsUpBack();
		setWLWW();
	}

	int nMaxAxisSize = m_pgMPRengine->getMaxAxisSize(id);
	m_pViewPlane[id]->createImage2D(nMaxAxisSize, nMaxAxisSize);

	if (m_pgMPRengine->isValid())
	{
		glm::vec3 rotCenter = *m_pgMPRengine->getMPRrotCenterOrigInVol(id);

		if (id == 0)
		{
			m_pViewPlane[0]->setVectors(rotCenter, m_vUpVec, m_vBackVec, *m_pgMPRengine->getVolRange(0), true);
			SetNavigatorDirection();
		}
		else
		{
			m_pViewPlane[1]->setVectors(rotCenter, m_vUpVecSecond, m_vBackVecSecond, *m_pgMPRengine->getVolRange(1), true);
		}
	}
	else
	{
		printf("Error: Volume is not set in MPRengine!!!!!\n");
		exit(1);
	}
}

void CW3ViewMPR::initItems(void)
{
	SAFE_DELETE_OBJECT(m_pLine[UILineType::HORIZONTAL]);
	SAFE_DELETE_OBJECT(m_pLine[UILineType::VERTICAL]);
	SAFE_DELETE_OBJECT(circle_);

	m_pLine[UILineType::HORIZONTAL] = new CW3LineItem_MPR(
		static_cast<UILineType>(UILineType::HORIZONTAL),
		static_cast<MPRViewType>(Line2View[m_eMPRType][UILineType::HORIZONTAL])
	);
	m_pLine[UILineType::VERTICAL] = new CW3LineItem_MPR(
		static_cast<UILineType>(UILineType::VERTICAL),
		static_cast<MPRViewType>(Line2View[m_eMPRType][UILineType::VERTICAL])
	);
	m_pLine[UILineType::HORIZONTAL]->addLineToScene(scene());
	m_pLine[UILineType::VERTICAL]->addLineToScene(scene());

	circle_ = new EllipseMPR(m_crossCenter);
	circle_->AddToScene(scene());

	m_crossCenter = m_pntCurViewCenterinScene;
	QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;

	// 단축의 1/4 영역의 line 위에 mouse 있으면 translate 하도록 정하는 parameter
	float tmp = (m_sceneWinView < m_sceneHinView ? m_sceneWinView : m_sceneHinView) * 0.25f;

	float marginCenter = tmp * 0.5f;
	float marginTranslate = marginCenter + tmp;
	float marginRotate = marginTranslate + tmp * 0.5f;
	for (int i = 0; i < UILineType::LINE_END; ++i)
	{
		m_pLine[i]->setLineRange(
			marginTranslate,
			marginRotate
		);  // rotate 인지 translate 인지 구분하는 영역 정함
		m_pLine[i]->setLine(QPoint(0, 0), marginTranslate, marginRotate);
		m_pLine[i]->initLine(cross_center_offeset);
	}

	m_marginCenterSqr = marginCenter * marginCenter;

#if 0
	int w = width();
	int h = height();
	float diSize = 30.0f;
	float diameter = (mapToScene(w * 0.5f + w / diSize, h * 0.5f + w / diSize) -
		mapToScene(w * 0.5f - w / diSize, h * 0.5f - w / diSize))
		.x();

	circle_->InitItems(cross_center_offeset, diameter);
#else
	circle_->InitItems(cross_center_offeset, 54.0f); // 최초 창 크기에 따라 원 크기가 달라지는 문제 때문에 고정값으로 변경
#endif

	QPen pen;
	pen.setWidthF(2.0f);
	pen.setCosmetic(true);
	pen.setJoinStyle(Qt::RoundJoin);
	switch (m_eMPRType)
	{
	case MPRViewType::AXIAL:
		pen.setColor(ColorView::kAxial);
		break;
	case MPRViewType::SAGITTAL:
		pen.setColor(ColorView::kSagittal);
		break;
	case MPRViewType::CORONAL:
		pen.setColor(ColorView::kCoronal);
		break;
	}

	if (slice_number_)
	{
		slice_number_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_ && show_slice_numbers_);
		slice_number_->setTextColor(pen.color());
	}

	m_rect->setPen(pen);
	m_rect->setRect(-m_sceneWinView + 2, -m_sceneHinView + 2,
		m_sceneWinView * 2.0f - 5, m_sceneHinView * 2.0f - 5);
	m_rect->setPos(m_pntCurViewCenterinScene);
	m_rectShadow->setPen(QPen(QColor(0, 0, 0, 127), 2, Qt::SolidLine,
		Qt::RoundCap, Qt::RoundJoin));
	m_rectShadow->setRect(-m_sceneWinView, -m_sceneHinView, m_sceneWinView * 2.0f,
		m_sceneHinView * 2.0f);
	m_rectShadow->setPos(m_pntCurViewCenterinScene);

	m_pWorldAxisItem->SetSize(100, 100);
	m_pWorldAxisItem->setPos(mapToScene(QPoint(width(), height()) - QPoint(60, 60)));

	sharpen_filter_text_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_);

	if (m_pProxySlider)
	{
		m_pProxySlider->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_);
	}
}

void CW3ViewMPR::initVectorsUpBack()
{
	switch (m_eMPRType)
	{
	case MPRViewType::AXIAL:
		m_vUpVec = kDefaultAxialUpVec;
		m_vBackVec = kDefaultAxialBackVec;
		break;
	case MPRViewType::SAGITTAL:
		m_vUpVec = kDefaultSagittalUpVec;
		if (GlobalPreferences::GetInstance()->preferences_.advanced.mpr.sagittal_direction == GlobalPreferences::Direction::Inverse)
		{
			m_vUpVec *= -1.0f;
		}
		m_vBackVec = kDefaultSagittalBackVec;
		break;
	case MPRViewType::CORONAL:
		m_vUpVec = kDefaultCoronalUpVec;
		m_vBackVec = kDefaultCoronalBackVec;
		break;
	default:
		m_vUpVec = glm::vec3();
		m_vBackVec = glm::vec3();
		break;
	}
}

void CW3ViewMPR::centeringItems()
{
	m_crossCenter = m_pntCurViewCenterinScene;
	QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
	for (int i = 0; i < UILineType::LINE_END; ++i)
	{
		if (m_pLine[i]) m_pLine[i]->centeringLine(cross_center_offeset);
	}

	circle_->SetPos(cross_center_offeset);

	m_pWorldAxisItem->SetSize(100, 100);
	m_pWorldAxisItem->setPos(mapToScene(QPoint(width(), height()) - QPoint(60, 60)));
	SetNavigatorDirection();

	if (m_pZoom3D)
	{
		m_pZoom3D->setCenter(cross_center_offeset);

		if (m_eMPRType == MPRViewType::AXIAL)
		{
			float WsceneTrans = m_WglTrans / m_scaleSceneToGL;
			float HsceneTrans = m_HglTrans / m_scaleSceneToGL;

			int idx = scaledSceneToVol(cross_center_offeset.x() + WsceneTrans -
				m_pntCurViewCenterinScene.x());
			int idy = scaledSceneToVol(cross_center_offeset.y() + HsceneTrans -
				m_pntCurViewCenterinScene.y());

			glm::vec3 ptIn3DVol = m_pViewPlane[0]->getPlaneCenterInVol() +
				m_pViewPlane[0]->getRightVec() * float(idx) +
				m_pViewPlane[0]->getBackVec() * float(idy);

			emit sigSetZoom3DVR(m_eMPRType, ptIn3DVol, scaledSceneToVol(m_pZoom3D->radius()));
		}

		rotateZoom3DCube(m_rotate);
	}
}

void CW3ViewMPR::SetThicknessMaximumRange(const float& range)
{
	m_fThicknessMaxInVol = range;
}

void CW3ViewMPR::InitialDraw()
{
	ApplyPreferences();
	drawImageOnViewPlane(true, 0, m_pRender3DParam->m_pNerve->isVisible());
	initItems();
}

void CW3ViewMPR::SetVisibleMPRUIs(bool visible)
{
	for (int kLine = 0; kLine < UILineType::LINE_END; kLine++)
	{
		if (!m_pLine[kLine])
		{
			continue;
		}

		m_pLine[kLine]->setVisibleLines(!super_imposition_mode_ && visible);
		if (!visible)
		{
			m_pLine[kLine]->setVisibleThickness(visible);
		}
	}

	if (!visible)
	{
		circle_->SetVisible(visible);
	}

	m_pWorldAxisItem->setVisible(visible);
}

void CW3ViewMPR::SetVisibleRects(bool visible)
{
	m_rect->setVisible(visible);
	m_rectShadow->setVisible(visible);
}

void CW3ViewMPR::setDirectionText()
{
	if (!direction_text_)
	{
		CreateDirectionText();
	}

	switch (m_eMPRType)
	{
	case AXIAL:
		direction_text_->setPlainText("R");
		break;
	case SAGITTAL:
		direction_text_->setPlainText("A");
		break;
	case CORONAL:
		direction_text_->setPlainText("R");
		break;
	default:
		break;
	}

	direction_text_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_);

	QPointF posRightText(static_cast<float>(width()) * 0.1f, static_cast<float>(height()) * 0.5f);
	direction_text_->setPos(posRightText);
}

void CW3ViewMPR::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	float viewRatio = m_sceneWinView / m_sceneHinView;
	float imgRatio = m_Wglpre / m_Hglpre;

	if (!m_is2Dready)
	{
		return;
	}

	if (draw_arch_)
	{
		return;
	}

	if (sharpen_filter_text_ && sharpen_filter_text_->isUnderMouse())
	{
		QGraphicsView::mousePressEvent(event);

		QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
			event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);
	}
	else
	{
		CW3View2D_forMPR::mousePressEvent(event);
	}

	Qt::MouseButton button = event->button();
	// tool 이 선택되었을 경우 및 기본 인터랙션이 선택되었을 경우에는
	// 2D View에서 처리가 완료된다
	if (common_tool_type_ != common::CommonToolTypeOnOff::NONE)
	{
		return;
	}

	if (IsDefaultInteractionsIn2D(button))
	{
		event_type_ = MPREventType::VIEW2D_EVENT;
	}
	else if (button == Qt::LeftButton)
	{
		if (adjust_implant_ && implant_handle_->isHovered())
		{
			QPointF implant_scene_pos = implant_handle_->scenePos();
			last_scene_pos_ = m_crossCenter;
			ChangeCircleInMove(implant_scene_pos);
			last_scene_pos_ = implant_scene_pos;
		}
		else
		{
			last_scene_pos_ = mapToScene(event->pos());
			GetMPREventType();
		}
	}
}

#ifdef WILL3D_EUROPE
void CW3ViewMPR::leaveEvent(QEvent* event)
{
	CW3View2D_forMPR::leaveEvent(event);

	//mouse_orver_ = false;
}

void CW3ViewMPR::enterEvent(QEvent* event)
{
	CW3View2D_forMPR::enterEvent(event);

	mouse_orver_ = true;
}
#endif // WILL3D_EUROPE

void CW3ViewMPR::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	if (!m_is2Dready)
	{
		return;
	}

	Qt::MouseButtons buttons = event->buttons();
	QPointF scene_pos = mapToScene(event->pos());
	DisplayDICOMInfo(scene_pos);

	if (m_pSlider && m_pSlider->hovered())
	{
		return QGraphicsView::mouseMoveEvent(event);
	}

	if (draw_arch_)
	{
		arch_->DrawingCurrentPath(scene_pos);
		return;
	}

	CW3View2D_forMPR::mouseMoveEvent(event);

	if (common_tool_type_ != common::CommonToolTypeOnOff::NONE)
	{
		return;
	}

	if (buttons == Qt::NoButton)
	{
		if (adjust_implant_)
		{
			PickImplant(scene_pos);
		}
	}
	if (implant_handle_->isHovered())
	{
		for (int i = 0; i < UILineType::LINE_END; ++i)
		{
			if (m_pLine[i])
			{
				m_pLine[i]->setVisibleThickness(false);
			}
		}

		circle_->SetVisible(false);

		QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());

		return;
	}

	if (single_view_hide_ui_ || hide_all_view_ui_)
	{
		return;
	}

	//DrawImplantID();

	if (super_imposition_mode_ || event_type_ == MPREventType::VIEW2D_EVENT)
	{
		return;
	}

	if (event->buttons() == Qt::LeftButton)
	{
		if (event_type_ == MPREventType::ZOOM3D)
		{
			ChangeZoom3DInMove(scene_pos);
		}
		else if (selected_item_ != MPRItemType::none)
		{
			ChangeMPRInMove(scene_pos);
		}
		return;
	}

	if (MoveInteractionZoom3D(scene_pos))
	{
		return;  // Zoom3D 에서 동작이 이뤄지면 MPR UI는 더이상 변경하지 않음.
	}

	MoveInteractionMPRItems(scene_pos);
}

void CW3ViewMPR::mouseReleaseEvent(QMouseEvent* event)
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

	if (!m_is2Dready)
	{
		return;
	}

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
	{
		// event->ignore();
		QGraphicsView::mouseReleaseEvent(event);

		QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
			event->button(), event->buttons(), event->modifiers());
		QApplication::sendEvent(QApplication::instance(), &ev);
		emit sigMouseRelease();
		return;
	}

	QPointF scene_pos = mapToScene(event->pos());

	if (draw_arch_)
	{
		Qt::MouseButton button = event->button();
		if (button == Qt::LeftButton)
		{
			arch_->AddPoint(scene_pos);
		}
		else if (button == Qt::RightButton)
		{
			arch_->CancelLastPoint();
			if (arch_->GetCtrlPoints().size() < 1)
			{
				draw_arch_ = false;
			}
		}
		return;
	}

	if (m_bIsZoom3DMode)
	{
		DrawZoom3D(scene_pos);
	}

	CW3View2D_forMPR::mouseReleaseEvent(event);
	if (selected_item_ != MPRItemType::none)
	{
		emit sigMouseRelease();
	}

	selected_item_ = MPRItemType::none;
	event_type_ = MPREventType::NONE;

	// event->ignore();

	if (implant_handle_->isVisible())
	{
		emit sigDoneAdjustImplant();
	}
}

void CW3ViewMPR::mouseDoubleClickEvent(QMouseEvent* event)
{
	Qt::MouseButton button = event->button();
	if (draw_arch_ && arch_->IsStartEdit() &&
		button == Qt::LeftButton)
	{
		arch_->EndEdit();
	}
	CW3View2D_forMPR::mouseDoubleClickEvent(event);
}

void CW3ViewMPR::wheelEvent(QWheelEvent* event)
{
	if (!m_is2Dready)
	{
		return;
	}

	float dist = event->delta() / 120.0f;

	if (m_eMPRType == MPRViewType::CORONAL)
	{
		dist = -dist;
	}

	dist *= interval_slider_;

	float fittedDist = translateDraw(dist);

	if (m_pViewPlane[0]->getAvailableDetph() > 0)
	{
		emit sigWheel(m_eMPRType, fittedDist, true);
	}
}

void CW3ViewMPR::keyPressEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (!m_is2Dready)
	{
		return;
	}

	float dist = 1;

	if (m_eMPRType == MPRViewType::CORONAL)
	{
		dist = -dist;
	}

	dist *= interval_slider_;

	if (event->key() == Qt::Key_Up)
	{
		float fittedDist = translateDraw(dist);

		if (m_pViewPlane[0]->getAvailableDetph() > 0)
		{
			emit sigWheel(m_eMPRType, fittedDist, true);
		}
	}
	else if (event->key() == Qt::Key_Down)
	{
		float fittedDist = translateDraw(-dist);

		if (m_pViewPlane[0]->getAvailableDetph() > 0)
		{
			emit sigWheel(m_eMPRType, fittedDist, true);
		}
	}
	else if (event->key() == Qt::Key_Enter ||
		event->key() == Qt::Key_Return)
	{
		if (hide_all_view_ui_)
		{
			return;
		}

		single_view_hide_ui_ = !single_view_hide_ui_;
		SetVisibleItems(!single_view_hide_ui_);
	}
	else if (event->key() == Qt::Key_Escape)
	{
#if 0
		if (arch_)
		{
			arch_->Clear();
		}
		draw_arch_ = false;
#else
		if (arch_)
		{
			arch_->CancelLastPoint();
			if (arch_->GetCtrlPoints().size() < 1)
			{
				draw_arch_ = false;
			}
		}
#endif
	}

	emit sigKeyPressEvent(event);
}

void CW3ViewMPR::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE
	CW3View2D_forMPR::keyReleaseEvent(event);
}

void CW3ViewMPR::PickImplant(const QPointF& scene_pos)
{
#if ADJUST_IMPLANT
	if (!m_pRender3DParam->m_isImplantShown)
	{
		return;
	}

	if (implant_handle_->isVisible())
	{
		if (!implant_handle_->isHovered())
		{
			implant_handle_->Disable();
			CW3View2D_forMPR::drawImplantID(false);
		}
	}

	//implant_handle_->Disable();
	for (int i = 0; i < MAX_IMPLANT; i++)
	{
		const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
		const auto& datas = res_implant.data();
		auto data = datas.find(kImplantNumbers[i]);
		if (data == datas.end())
		{
			continue;
		}

		glm::vec3 vol_pos = MapSceneToVol(scene_pos);
		glm::vec3 implant_bounding_box_min = data->second->position_in_vol() + data->second->bounding_box_min();
		glm::vec3 implant_bounding_box_max = data->second->position_in_vol() + data->second->bounding_box_max();

		if (!implant_handle_->isVisible())
		{
			if ((implant_bounding_box_min.x <= vol_pos.x && vol_pos.x <= implant_bounding_box_max.x) &&
				(implant_bounding_box_min.y <= vol_pos.y && vol_pos.y <= implant_bounding_box_max.y) &&
				(implant_bounding_box_min.z <= vol_pos.z && vol_pos.z <= implant_bounding_box_max.z))
			{
				int id = data->first;
				QPointF implant_pos = MapVolToScene(data->second->position_in_vol());
				implant_handle_->Enable(id, true, implant_pos);
				QPointF text_pos(
					implant_pos.x() - implant_id_text_->sceneBoundingRect(i).width() / 2,
					implant_pos.y() + implant_handle_->sceneBoundingRect().height() / 2
				);
				CW3View2D_forMPR::drawImplantID(i, text_pos, true);

				emit sigSelectImplant(id);
			}
		}
	}
#endif
}

void CW3ViewMPR::setVisible(bool state)
{
	if (state && arch_)
	{
		arch_->Clear();
	}
	draw_arch_ = false;

	if (!state)
	{
		single_view_hide_ui_ = false;
	}

	CW3View2D_forMPR::setVisible(state);
}

QColor CW3ViewMPR::GetViewColor()
{
	switch (m_eMPRType)
	{
	case MPRViewType::AXIAL:
		return ColorView::kAxial;
	case MPRViewType::SAGITTAL:
		return ColorView::kSagittal;
	case MPRViewType::CORONAL:
		return ColorView::kCoronal;
	default:
		return ColorView::k3D;
	}
}

float CW3ViewMPR::getRotationAngle(const QPointF& ptCur)
{
	QVector2D prev = QVector2D(last_scene_pos_ - m_crossCenter).normalized();
	QVector2D next = QVector2D(ptCur - m_crossCenter).normalized();
	last_scene_pos_ = ptCur;

#if 0
	return static_cast<float>(std::asin(prev.x() * next.y() - prev.y() * next.x()));
#else
	float theta = QVector2D::dotProduct(prev, next);
	float radian = 0.0f;

	if (std::fabs(theta - 1.0f) < std::numeric_limits<float>::epsilon())
	{
		radian = 0;
	}
	else if (std::fabs(theta + 1.0f) < std::numeric_limits<float>::epsilon())
	{
		radian = M_PI;
	}
	else
	{
		radian = acosf(theta);
	}

	QVector3D p = QVector3D(prev, 0.f);
	QVector3D n = QVector3D(next, 0.f);
	QVector3D cross = QVector3D::crossProduct(n, p);

	return cross.z() < 0.0f ? radian : -radian;
#endif
}

void CW3ViewMPR::rotate3DDraw(glm::mat4* T, glm::mat4* rotate)
{
	m_pViewPlane[0]->rotate3D(*T, *m_pgMPRengine->getMPRrotCenterInVol(0), *rotate);

	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		setSecondVectors();
		drawImageOnViewPlane(false, 1, false);
	}

	scene()->update();

	glm::vec3 vD = *m_pgMPRengine->getMPRrotCenterInVol(0) -
		m_pViewPlane[0]->getPlaneCenterInVol();
	QPointF delta(scaledVolToScene(glm::dot(m_pViewPlane[0]->getRightVec(), vD)),
		scaledVolToScene(glm::dot(m_pViewPlane[0]->getBackVec(), vD)));

	m_crossCenter = m_pntCurViewCenterinScene + delta - m_sceneTrans;

	setPosItemsUseCrossCenter();
	SetNavigatorDirection();
}

void CW3ViewMPR::rotate3DDrawPassive(const glm::mat4& rotate)
{
	m_pViewPlane[0]->rotate3Dpassive(*m_pgMPRengine->getMPRrotCenterInVol(0), rotate);

	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	scene()->update();

	glm::vec3 vD = *m_pgMPRengine->getMPRrotCenterInVol(0) -
		m_pViewPlane[0]->getPlaneCenterInVol();
	QPointF delta(scaledVolToScene(glm::dot(m_pViewPlane[0]->getRightVec(), vD)),
		scaledVolToScene(glm::dot(m_pViewPlane[0]->getBackVec(), vD)));

	m_crossCenter = m_pntCurViewCenterinScene + delta - m_sceneTrans;

	setPosItemsUseCrossCenter();
	SetNavigatorDirection();
}

void CW3ViewMPR::rotateDraw(const glm::vec3& UpVec, float fAngle)
{
	m_pViewPlane[0]->rotate(*m_pgMPRengine->getMPRrotCenterInVol(0), UpVec, -fAngle);

	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		glm::vec3 UpVec2(m_secondTransform * glm::vec4(UpVec, 0.0f));
		m_pViewPlane[1]->rotateSecond(
			glm::vec3(m_secondTransform *
				glm::vec4(m_pViewPlane[0]->getPlaneCenterInVol(), 1.0f)),
			UpVec2, -fAngle);

		drawImageOnViewPlane(false, 1, false);
	}

	scene()->update();

	glm::vec3 vD = *m_pgMPRengine->getMPRrotCenterInVol(0) -
		m_pViewPlane[0]->getPlaneCenterInVol();
	QPointF delta(scaledVolToScene(glm::dot(m_pViewPlane[0]->getRightVec(), vD)),
		scaledVolToScene(glm::dot(m_pViewPlane[0]->getBackVec(), vD)));

	m_crossCenter = m_pntCurViewCenterinScene + delta - m_sceneTrans;

	measure_tools_->UpdateProjection();

	setPosItemsUseCrossCenter();
	SetNavigatorDirection();
}

float CW3ViewMPR::translateDraw(float fDist, const bool limit_move_range)
{
	float fittedDist = 0.0f;
	if (!super_imposition_mode_)
	{
		fittedDist = m_pViewPlane[0]->translate(
			fDist, *m_pgMPRengine->getMPRrotCenterInVol(0), limit_move_range);
	}
	else
	{
		fittedDist = m_pViewPlane[0]->translate(
			fDist, *m_pgMPRengine->getSIMPRrotCenterInVol(0), limit_move_range);
	}

	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		glm::vec3 planeCenter =
			glm::vec3(m_secondTransform *
				glm::vec4(m_pViewPlane[0]->getPlaneCenterInVol(), 1.0f));
		m_pViewPlane[1]->translateSecond(planeCenter);

		drawImageOnViewPlane(false, 1, false);
	}

	scene()->update();

	measure_tools_->UpdateProjection();

	return fittedDist;
}

void CW3ViewMPR::thickDraw(const float& fThickness)
{
	m_fThickness[0] = fThickness;
	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		// Jung To Do :: vol1 과 vol2 의 resolution 다를 경우 처리 필요
		m_fThickness[1] = fThickness * m_pgMPRengine->getBasePixelSize(0) /
			m_pgMPRengine->getBasePixelSize(1);
		drawImageOnViewPlane(false, 1, false);
	}

	scene()->update();
}

void CW3ViewMPR::SetIntervalOfSlider(float interval)
{
	interval_slider_ = interval;
}

void CW3ViewMPR::thickLineChange(MPRViewType eLineType, float fThickness)
{
	MPRViewType hV =
		static_cast<MPRViewType>(Line2View[m_eMPRType][UILineType::HORIZONTAL]);

	if (hV == eLineType)
	{
		m_pLine[UILineType::HORIZONTAL]->setThickness(scaledVolToScene(fThickness));
		m_pLine[UILineType::HORIZONTAL]->setPos(m_crossCenter.x(), m_crossCenter.y());
	}
	else
	{
		m_pLine[UILineType::VERTICAL]->setThickness(scaledVolToScene(fThickness));
		m_pLine[UILineType::VERTICAL]->setPos(m_crossCenter.x(), m_crossCenter.y());
	}
}

void CW3ViewMPR::translateCrossPt(const UILineType eLineType, float fDist)
{
	if (!m_pLine[eLineType])
	{
		return;
	}

	float dy = scaledVolToScene(fDist);

	QVector2D NormalVec = m_pLine[eLineType]->getNormal();

	m_crossCenter = QPointF(m_crossCenter.x() + dy * NormalVec.x(),
		m_crossCenter.y() + dy * NormalVec.y());

	last_scene_pos_ = QPointF(last_scene_pos_.x() + dy * NormalVec.x(),
		last_scene_pos_.y() + dy * NormalVec.y());

	setPosItemsUseCrossCenter();
}

void CW3ViewMPR::translateCrossPt(const float& fDistx, const float& fDisty)
{
	if (!m_pLine[UILineType::HORIZONTAL] && !m_pLine[UILineType::VERTICAL])
	{
		return;
	}

	float dx = scaledVolToScene(fDistx);
	float dy = scaledVolToScene(fDisty);

	QVector2D NormalVecH = m_pLine[UILineType::HORIZONTAL]->getNormal();
	QVector2D NormalVecV = m_pLine[UILineType::VERTICAL]->getNormal();

	m_crossCenter =
		QPointF(m_crossCenter.x() + dx * NormalVecV.x() + dy * NormalVecH.x(),
			m_crossCenter.y() + dx * NormalVecV.y() + dy * NormalVecH.y());

	last_scene_pos_ =
		QPointF(last_scene_pos_.x() + dx * NormalVecV.x() + dy * NormalVecH.x(),
			last_scene_pos_.y() + dx * NormalVecV.y() + dy * NormalVecH.y());

	setPosItemsUseCrossCenter();
}

void CW3ViewMPR::rotateItems(const float& radian)
{
	for (int i = 0; i < UILineType::LINE_END; ++i)
	{
		if (!m_pLine[i])
		{
			continue;
		}

		m_pLine[i]->rotate(radian * 180.0f / M_PI);
		m_pLine[i]->rotateVector2D(radian);
	}
}

void CW3ViewMPR::setPosItemsUseCrossCenter()
{
	QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
	for (int i = 0; i < UILineType::LINE_END; ++i)
	{
		if (!m_pLine[i])
		{
			continue;
		}

		m_pLine[i]->setPos(cross_center_offeset.x(), cross_center_offeset.y());
	}

	circle_->SetPos(cross_center_offeset);

	if (m_pZoom3D)
	{
		m_pZoom3D->setCenter(cross_center_offeset);
	}
}

void CW3ViewMPR::RotateView(const QPointF& scene_pos)
{
	SetNavigatorDirection();

	last_scene_pos_ = scene_pos;
}

void CW3ViewMPR::ChangeTranslateCursorWithRotateAngle(const float angle)
{
	if ((0.0f <= angle && angle < kHalfPartAngle) ||
		(360.0f - kHalfPartAngle < angle && angle < 360.0f) ||
		(180.0f - kHalfPartAngle < angle && angle < 180.0f + kHalfPartAngle))
	{
		QApplication::setOverrideCursor(CW3Cursor::SizeVerCursor());
	}
	else if ((90.0f - kHalfPartAngle < angle && angle < 90.0f + kHalfPartAngle) ||
		(270.0f - kHalfPartAngle < angle && angle < 270.0f + kHalfPartAngle))
	{
		QApplication::setOverrideCursor(CW3Cursor::SizeHorCursor());
	}
	else if ((45.0f - kHalfPartAngle <= angle &&
		angle <= 45.0f + kHalfPartAngle) ||
		(225.0f - kHalfPartAngle <= angle &&
			angle <= 225.0f + kHalfPartAngle))
	{
		QApplication::setOverrideCursor(CW3Cursor::SizeBDiagCursor());
	}
	else if ((135.0f - kHalfPartAngle <= angle &&
		angle <= 135.0f + kHalfPartAngle) ||
		(315.0f - kHalfPartAngle <= angle &&
			angle <= 315.0f + kHalfPartAngle))
	{
		QApplication::setOverrideCursor(CW3Cursor::SizeFDiagCursor());
	}
}

void CW3ViewMPR::ChangeUIInteractiveArea(UILineType line_type,
	const QPointF& scene_pos)
{
	if (m_pLine[line_type]->isTranslationSelectedBy(scene_pos, m_crossCenter))
	{
		m_pLine[line_type]->setLineWidth(3.0f);
		ChangeTranslateCursorWithRotateAngle(m_pLine[line_type]->getRotateAngle());
	}
	else if (m_pLine[line_type]->isRotationSelectedBy(scene_pos, m_crossCenter))
	{
		m_pLine[line_type]->setLineWidth(3.0f);
		QApplication::setOverrideCursor(CW3Cursor::RotateCursor());
	}
	else
	{  // mouse move, not clicked, thickness area can be selected
		m_pLine[line_type]->setThicknessLineWidth(7.0f);
		QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
	}
}

void CW3ViewMPR::DrawImplantID()
{
#if 0
	if (!m_pRender3DParam->m_isImplantShown)
		return;

	QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
	float idx = scaledSceneToVol(cross_center_offeset.x() + m_sceneTrans.x() -
		m_pntCurViewCenterinScene.x()); float idy =
		scaledSceneToVol(cross_center_offeset.y() + m_sceneTrans.y() -
			m_pntCurViewCenterinScene.y());

	glm::vec3 ptIn3DVol = m_pViewPlane[0]->getPlaneCenterInVol()
		+ m_pViewPlane[0]->getRightVec()*idx
		+ m_pViewPlane[0]->getBackVec()*idy;

	ptIn3DVol = (ptIn3DVol - m_pViewPlane[0]->getVolCenter()) * m_scaleVolToGL;
	ptIn3DVol.x *= -1.0f;

	int hovered_id = /* removed. implant picking code here */;
	int prehovered_id = implant_id_text_->prehovered_id();
	implant_id_text_->set_hovered_id(hovered_id);
	if (prehovered_id != -1)
	{
		CW3View2D_forMPR::drawImplantID(prehovered_id, QPointF(0.0f, 0.0f),
			false); 	implant_id_text_->set_prehovered_id(-1);
	}

	if (hovered_id != -1)
	{
		implant_id_text_->set_prehovered_id(hovered_id);

		CW3Implant** implant_set = m_pgRcontainer->getImplants();
		glm::vec3 implantPosInVol =
			glm::vec3(implant_set[hovered_id]->m_translate[3]); 	implantPosInVol.x
			*= -1.0f;

		implantPosInVol = implantPosInVol / m_scaleVolToGL +
			m_pViewPlane[0]->getVolCenter(); 	glm::vec3 planeCenter =
			m_pViewPlane[0]->getPlaneCenterInVol();

		glm::vec3 temp = implantPosInVol - planeCenter;
		float pos_x = glm::dot(temp, m_pViewPlane[0]->getRightVec());
		float pos_y = glm::dot(temp, m_pViewPlane[0]->getBackVec());

		glm::vec2 implantPosInScene = glm::vec2(pos_x, pos_y) *
			scaledVolToScene(); 	QPointF implantPos(implantPosInScene.x -
				m_sceneTrans.x(), implantPosInScene.y - m_sceneTrans.y()); 	implantPos
			= implantPos + m_pntCurViewCenterinScene;
		CW3View2D_forMPR::drawImplantID(hovered_id, implantPos, true);
	}
#endif
}

void CW3ViewMPR::DrawZoom3D(const QPointF& scene_pos)
{
	if (!m_pZoom3D)
	{
		m_pZoom3D = new Zoom3DCube(last_scene_pos_, m_pViewPlane[0]->getRightVec(), m_pViewPlane[0]->getBackVec());
		m_pZoom3D->AddToScene(scene());
		m_pZoom3D->setVisible(true);
		m_pZoom3D->drawStart();
		m_pZoom3D->resize();
	}
	else
	{
		QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
		QVector2D vTrans(m_pZoom3D->center() - cross_center_offeset);

		QVector2D vD(scaledSceneToVol(QVector2D::dotProduct(
			m_pLine[UILineType::VERTICAL]->getNormal(), vTrans)),
			scaledSceneToVol(QVector2D::dotProduct(
				m_pLine[UILineType::HORIZONTAL]->getNormal(), vTrans)));

		bool updateVR = false;

		if (m_pZoom3D->isDrawMode())
		{
			m_pZoom3D->drawEnd();

			setMPRInteraction(true);

			switch (m_eMPRType)
			{
			case AXIAL:
				m_pLine[UILineType::VERTICAL]->setVisibleLines(true);
				m_pLine[UILineType::HORIZONTAL]->setVisibleLines(false);
				break;
			case SAGITTAL:
				m_pLine[UILineType::HORIZONTAL]->setVisibleLines(true);
				m_pLine[UILineType::VERTICAL]->setVisibleLines(false);
				break;
			default:
				break;
			}
			circle_->SetVisible(true);

			emit sigTranslateCross(m_eMPRType, vD);

			updateVR = true;
		}
		else
		{
			if (selected_item_ == MPRItemType::circle)
			{
				updateVR = true;
			}
			else if (selected_item_ == MPRItemType::hLine)
			{
				if (event_type_ == MPREventType::TRANSLATE) updateVR = true;
				emit sigSelectedMPR(
					(MPRViewType)Line2View[m_eMPRType][UILineType::HORIZONTAL], true);
			}
			else if (selected_item_ == MPRItemType::vLine)
			{
				if (event_type_ == MPREventType::TRANSLATE) updateVR = true;
				emit sigSelectedMPR(
					(MPRViewType)Line2View[m_eMPRType][UILineType::VERTICAL], true);
			}
			else
			{
				if (m_pZoom3D->selectedElement(scene_pos) ==
					Zoom3DCube::ZOOM3D_ELEMENT::SPHERE)
				{
					updateVR = true;
				}
			}
		}
		circle_->SetVisibleCircle(false);

		float WsceneTrans = m_WglTrans / m_scaleSceneToGL;
		float HsceneTrans = m_HglTrans / m_scaleSceneToGL;

		cross_center_offeset = m_crossCenter - m_SceneGLOffset;
#ifdef ID_FLOAT
		float idx = scaledSceneToVol(cross_center_offeset.x() + WsceneTrans -
			view_center_.x());
		float idy = scaledSceneToVol(cross_center_offeset.y() + HsceneTrans -
			view_center_.y());
#else
		int idx = scaledSceneToVol(cross_center_offeset.x() + WsceneTrans -
			m_pntCurViewCenterinScene.x());
		int idy = scaledSceneToVol(cross_center_offeset.y() + HsceneTrans -
			m_pntCurViewCenterinScene.y());
#endif

		glm::vec3 ptIn3DVol = m_pViewPlane[0]->getPlaneCenterInVol() +
			m_pViewPlane[0]->getRightVec() * float(idx) +
			m_pViewPlane[0]->getBackVec() * float(idy);

		emit sigSetZoom3DMPR(m_eMPRType, m_pZoom3D->center(), m_pZoom3D->radius(), updateVR);
		if (m_eMPRType == MPRViewType::AXIAL && updateVR)
		{
			emit sigSetZoom3DVR(m_eMPRType, ptIn3DVol,
				scaledSceneToVol(m_pZoom3D->radius()));
		}
	}
}

float CW3ViewMPR::GetCurrentCenterMargin()
{
	float center_margin = m_marginCenterSqr;
	if (m_bIsZoom3DMode && m_pZoom3D)
	{
		m_pZoom3D->GetMargin(center_margin);
	}

	return center_margin;
}

void CW3ViewMPR::GetMPREventType()
{
	QPointF view_dist_cross = m_crossCenter - last_scene_pos_;
	float fLengthSqr = view_dist_cross.x() * view_dist_cross.x() +
		view_dist_cross.y() * view_dist_cross.y();

	if (fLengthSqr < GetCurrentCenterMargin())
	{
		selected_item_ = MPRItemType::circle;
		event_type_ = MPREventType::CROSS;
		emit sigSelectedMPR(m_eMPRType, true);
	}
	else if (m_bIsZoom3DMode && m_pZoom3D &&
		m_pZoom3D->selectedElement(last_scene_pos_) ==
		Zoom3DCube::ZOOM3D_ELEMENT::SPHERE)
	{
		event_type_ = MPREventType::ZOOM3D;
	}
	else
	{
		for (int kLine = 0; kLine < UILineType::LINE_END; kLine++)
		{
			if (!m_pLine[kLine]->isSelectedBy(last_scene_pos_, m_crossCenter))
			{
				continue;
			}

			UILineType line_type = static_cast<UILineType>(kLine);
			selected_item_ = (kLine == UILineType::HORIZONTAL) ? MPRItemType::hLine
				: MPRItemType::vLine;
			if (m_pLine[line_type]->isTranslationSelectedBy(last_scene_pos_,
				m_crossCenter))
			{
				event_type_ = MPREventType::TRANSLATE;
			}
			else if (m_pLine[line_type]->isRotationSelectedBy(last_scene_pos_,
				m_crossCenter))
			{
				event_type_ = MPREventType::ROTATE;
			}
			else
			{
				event_type_ = MPREventType::THICKNESS;
			}

			emit sigSelectedMPR((MPRViewType)Line2View[m_eMPRType][line_type], true);
		}
	}
}

void CW3ViewMPR::ChangeMPRInMove(const QPointF& scene_pos)
{
	if (selected_item_ == MPRItemType::circle)
	{
		ChangeCircleInMove(scene_pos);
	}
	else
	{
		UILineType eLineType = (selected_item_ == MPRItemType::hLine)
			? UILineType::HORIZONTAL
			: UILineType::VERTICAL;
		if (event_type_ == MPREventType::ROTATE)
		{
			emit sigRotate(eLineType, m_eMPRType, m_pViewPlane[0]->getUpVec(),
				getRotationAngle(scene_pos));
		}
		else if (event_type_ == MPREventType::TRANSLATE)
		{
			QVector2D vTrans = QVector2D(scene_pos - last_scene_pos_);
			QVector2D lineNormal = m_pLine[eLineType]->getNormal();
			float fDist = scaledSceneToVol(QVector2D::dotProduct(lineNormal, vTrans));

			emit sigTranslate(m_eMPRType,
				(MPRViewType)Line2View[m_eMPRType][eLineType], fDist);
		}
		else if (event_type_ == MPREventType::THICKNESS)
		{
			ChangeThicknessInMove(eLineType, scene_pos);
		}
	}
}

void CW3ViewMPR::ChangeThicknessInMove(UILineType line_type,
	const QPointF& scene_pos)
{
	QVector2D trans(scene_pos - last_scene_pos_);
	QVector2D line_normal = m_pLine[line_type]->getNormal();
	float dist_scene = QVector2D::dotProduct(trans, line_normal);
	float thickness_in_scene = m_pLine[line_type]->thickness() - dist_scene;
	float thickness_max_in_scene = scaledVolToScene(m_fThicknessMaxInVol);

	if (thickness_in_scene < 0.0f)
	{
		thickness_in_scene = 0.0f;
	}
	else if (thickness_in_scene > thickness_max_in_scene)
	{
		thickness_in_scene = thickness_max_in_scene;
	}

	m_pLine[line_type]->setThickness(thickness_in_scene);
	m_pLine[line_type]->setPos(m_crossCenter.x(), m_crossCenter.y());
	float thickness_in_vol = scaledSceneToVol(thickness_in_scene);

	UILineType eTheOtherLine = (line_type == UILineType::HORIZONTAL)
		? UILineType::VERTICAL
		: UILineType::HORIZONTAL;

	emit sigThickChange((MPRViewType)Line2View[m_eMPRType][line_type],
		(MPRViewType)Line2View[m_eMPRType][eTheOtherLine],
		thickness_in_vol);

	last_scene_pos_ = scene_pos;
}

void CW3ViewMPR::ChangeCircleInMove(const QPointF& scene_pos)
{
	QVector2D vTrans(scene_pos - last_scene_pos_);
	QVector2D vD(
		scaledSceneToVol(QVector2D::dotProduct(m_pLine[UILineType::VERTICAL]->getNormal(), vTrans)),
		scaledSceneToVol(QVector2D::dotProduct(m_pLine[UILineType::HORIZONTAL]->getNormal(), vTrans))
	);

	circle_->SetVisibleCircle(true);
	circle_->SetVisibleCenter(true);
	emit sigTranslateCross(m_eMPRType, vD);
}

void CW3ViewMPR::ChangeZoom3DInMove(const QPointF& scene_pos)
{
	float fDist = sqrt(pow((scene_pos.x() - m_pZoom3D->center().x()), 2) +
		pow((scene_pos.y() - m_pZoom3D->center().y()), 2));

	if (m_pZoom3D->isDrawMode())
	{
		m_pZoom3D->resize(fDist);
	}
	else if (selected_item_ == MPRItemType::none)
	{
		m_pZoom3D->resize(scene_pos, fDist);
	}
}

bool CW3ViewMPR::MoveInteractionZoom3D(const QPointF& scene_pos)
{
	if (m_bIsZoom3DMode)
	{
		if (!m_pZoom3D)
		{
			return true;
		}

		if (m_pZoom3D->isDrawMode())
		{
			float fDist = sqrt(pow((scene_pos.x() - m_pZoom3D->center().x()), 2) +
				pow((scene_pos.y() - m_pZoom3D->center().y()), 2));
			m_pZoom3D->resize(fDist);
			return true;
		}

		if (m_pZoom3D->selectedElement(scene_pos) ==
			Zoom3DCube::ZOOM3D_ELEMENT::SPHERE)
		{
			QApplication::setOverrideCursor(CW3Cursor::OpenHandCursor());
			//라인 컬러 초기화
			m_pLine[UILineType::HORIZONTAL]->initLineWidthColor();
			m_pLine[UILineType::VERTICAL]->initLineWidthColor();
			return true;
		}
		else
		{
			QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
		}
	}
	return false;
}

void CW3ViewMPR::MoveInteractionMPRItems(const QPointF& scene_pos)
{
	//라인 컬러 초기화
	m_pLine[UILineType::HORIZONTAL]->initLineWidthColor();
	m_pLine[UILineType::VERTICAL]->initLineWidthColor();

	QPointF viewDistCross = m_crossCenter - scene_pos;
	float fLengthSqr = viewDistCross.x() * viewDistCross.x() +
		viewDistCross.y() * viewDistCross.y();
	// 원 안에 있으면 원을 보이게 한다.
	if (fLengthSqr < GetCurrentCenterMargin())
	{
		circle_->SetVisible(true);
		QApplication::setOverrideCursor(CW3Cursor::CrossCursor());
	}
	else
	{
		bool on_line = false;

		circle_->SetVisible(false);
		for (int kLine = 0; kLine < UILineType::LINE_END; kLine++)
		{
			//마우스가 라인 위면 마우스 커서와 라인의 색깔 크기를 설정한다.
			if (m_pLine[(UILineType)kLine]->isSelectedBy(scene_pos, m_crossCenter))
			{
				// thickness 라인과 가운데 작은 원을 보이게 한다.
				m_pLine[kLine]->setVisibleThickness(true);
				circle_->SetVisibleCenter(true);
				ChangeUIInteractiveArea((UILineType)kLine, scene_pos);

				on_line = true;
			}
			else
			{
				circle_->SetVisibleCenter(false);
			}
		}

		if (!on_line)
		{
			QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
		}
	}
}

void CW3ViewMPR::DisplayDICOMInfo(const QPointF& scene_pos)
{
	const int width = m_pViewPlane[0]->getWidth();
	const int height = m_pViewPlane[0]->getHeight();
	const float trans_x = m_WglTrans / m_scaleVolToGL;
	const float trans_y = m_HglTrans / m_scaleVolToGL;

	auto center = scene_pos - m_pntCurViewCenterinScene;

#if 0
	float idx = scaledSceneToVol(center.x() + m_Wglorig / m_scaleSceneToGL);
	float idy = scaledSceneToVol(center.y() + m_Hglorig / m_scaleSceneToGL);
#else
	float idx = scaledSceneToVol(center.x() + width / m_scaleSceneToGL) + trans_x;
	float idy =
		scaledSceneToVol(center.y() + height / m_scaleSceneToGL) + trans_y;
#endif

	glm::vec4 volume_info =
		m_pgMPRengine->GetVolumeInfo(m_pViewPlane[0], idx, idy);

	if (volume_info.w != common::dicom::kInvalidHU)
	{
		HU_value_->SetText(
			QString("WL %1\nWW %2\n(%3, %4, %5), %6")
			.arg(m_nAdjustWindowLevel +
				ResourceContainer::GetInstance()->GetMainVolume().intercept())
			.arg(m_nAdjustWindowWidth)
			.arg(volume_info.x)
			.arg(volume_info.y)
			.arg(volume_info.z)
			.arg(volume_info.w));
	}
	else
	{
		HU_value_->SetText(
			QString("WL %1\nWW %2\n(-, -, -)")
			.arg(m_nAdjustWindowLevel +
				ResourceContainer::GetInstance()->GetMainVolume().intercept())
			.arg(m_nAdjustWindowWidth));
	}
}

void CW3ViewMPR::CreateDirectionText()
{
	QFont font = QApplication::font();
	direction_text_ = new CW3TextItem(font, "R", Qt::white);
	direction_text_->setVisible(false);
	direction_text_->setZValue(0.0f);
	scene()->addItem(direction_text_);
}

void CW3ViewMPR::SetVisibleItems()
{
#if 0
	bool visible = true;
	if (single_view_hide_ui_ || hide_all_view_ui_)
	{
		visible = false;
	}
	else
	{
		visible = !single_view_hide_ui_;
	}
	SetVisibleItems(visible);
#else
	SetVisibleItems(!single_view_hide_ui_ && !hide_all_view_ui_);
#endif
}

void CW3ViewMPR::SetVisibleItems(const bool visible)
{
	CW3View2D_forMPR::SetVisibleItems(visible);

	bool is_text_visible = !(!visible);
	if (super_imposition_mode_ && direction_text_)
	{
		direction_text_->setVisible(is_text_visible);
	}

	if (slice_number_)
	{
		slice_number_->setVisible(is_text_visible && show_slice_numbers_);
	}

	SetVisibleMPRUIs(super_imposition_mode_ ? false : visible);
	SetVisibleRects(visible);
	if (m_pZoom3D)
	{
		m_pZoom3D->setVisible(visible);
	}
}

void CW3ViewMPR::resizeEvent(QResizeEvent* pEvent)
{
	if (!isVisible())
	{
		return;
	}

	CW3View2D_forMPR::resizeEvent(pEvent);

	SetSliceNumberPosition();

	if (direction_text_)
	{
		direction_text_->setPos(mapToScene(width() * 0.1f, height() * 0.5f));
	}
}

void CW3ViewMPR::resizeScene()
{
	if (!isVisible())
	{
		return;
	}

	CW3View2D_forMPR::resizeScene();

	m_rect->setRect(-m_sceneWinView + 2, -m_sceneHinView + 2,
		m_sceneWinView * 2.0f - 5, m_sceneHinView * 2.0f - 5);
	m_rect->setPos(m_pntCurViewCenterinScene);
	m_rectShadow->setRect(-m_sceneWinView, -m_sceneHinView, m_sceneWinView * 2.0f,
		m_sceneHinView * 2.0f);
	m_rectShadow->setPos(m_pntCurViewCenterinScene);
}

void CW3ViewMPR::SetNavigatorDirection()
{
	if (!m_pViewPlane[0])
	{
		return;
	}

	glm::vec3 init_up_vector = m_vUpVec;
	glm::vec3 init_back_vector = m_vBackVec;
	glm::vec3 up_vector = m_pViewPlane[0]->getUpVec();
	glm::vec3 back_vector = m_pViewPlane[0]->getBackVec();

	glm::mat4 view;
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	switch (m_eMPRType)
	{
	case MPRViewType::AXIAL:
		eye = glm::vec3(0.0f, 0.0f, 1.0f);
		center = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, -1.0f, 0.0f);

		init_up_vector *= glm::vec3(-1.0f, 1.0f, 1.0f);
		init_back_vector *= glm::vec3(-1.0f, 1.0f, 1.0f);
		up_vector *= glm::vec3(-1.0f, 1.0f, 1.0f);
		back_vector *= glm::vec3(-1.0f, 1.0f, 1.0f);
		break;
	case MPRViewType::SAGITTAL:
		eye = glm::vec3(-1.0f, 0.0f, 0.0f);
		center = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 0.0f, -1.0f);

		if (GlobalPreferences::GetInstance()->preferences_.advanced.mpr.sagittal_direction == GlobalPreferences::Direction::Inverse)
		{
			eye *= -1.0f;
		}

		init_up_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		init_back_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		up_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		back_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		break;
	case MPRViewType::CORONAL:
		eye = glm::vec3(0.0f, -1.0f, 0.0f);
		center = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 0.0f, -1.0f);

		init_up_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		init_back_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		up_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		back_vector *= glm::vec3(1.0f, -1.0f, -1.0f);
		break;
	}

	view = glm::lookAt(eye, center, up);

	glm::mat4 plane_rotate = GLTransformFunctions::GetRotMatrixVecToVec(init_up_vector, up_vector);
	glm::mat4 plane_rotate_right = GLTransformFunctions::GetRightRotateMatrix(plane_rotate, glm::cross(up_vector, back_vector), glm::cross(init_up_vector, init_back_vector));

	if (isVisible())
	{
		orientation_matrix_ = glm::inverse(plane_rotate_right * plane_rotate);
		m_pWorldAxisItem->SetWorldAxisDirection(orientation_matrix_, view);
	}
}

void CW3ViewMPR::slotReoriupdate(glm::mat4* T)
{
	m_reoriMat = *T;

	glm::vec3 rotCenter = *m_pgMPRengine->getMPRrotCenterOrigInVol(0);
	m_pgMPRengine->setMPRrotCenterInVol(rotCenter, 0);

	initVectorsUpBack();

	m_vUpVec = glm::vec3(m_reoriMat * glm::vec4(m_vUpVec, 0.0f));
	m_vBackVec = glm::vec3(m_reoriMat * glm::vec4(m_vBackVec, 0.0f));

	SetNavigatorDirection();
	m_fThickness[0] = 0.0f;

	if (m_is2Dready && m_pViewPlane[0])
	{
		m_pViewPlane[0]->setVectors(rotCenter, m_vUpVec, m_vBackVec,
			*m_pgMPRengine->getVolRange(0), true);
		if (isVisible())
		{
			drawImageOnViewPlane(true, 0, m_pRender3DParam->m_pNerve->isVisible());
			scene()->update();
		}
		else
		{
			m_bReoriupdated = true;
		}

		if (!super_imposition_mode_)
		{
			centeringItems();
		}
	}
}

void CW3ViewMPR::SecondUpdate(glm::mat4* T)
{
	if (m_bLoadProject)
	{
		m_bLoadProject = false;
	}
	else
	{
		m_secondTransform = *T;
	}

	initViewPlane(1);
	setSecondVectors();

	drawImageOnViewPlane(false, 1, false);
	scene()->update();
}

void CW3ViewMPR::setSecondVectors()
{
	m_vUpVecSecond =
		(m_secondTransform * glm::vec4(m_pViewPlane[0]->getUpVec(), 0.0f)).xyz;
	m_vBackVecSecond =
		(m_secondTransform * glm::vec4(m_pViewPlane[0]->getBackVec(), 0.0f)).xyz;

	m_pViewPlane[1]->setVectors(
		glm::vec3(m_secondTransform *
			glm::vec4(m_pViewPlane[0]->getPlaneCenterInVol(), 1.0f)),
		m_vUpVecSecond, m_vBackVecSecond, *m_pgMPRengine->getVolRange(1), false);
	m_pViewPlane[1]->setVolCenter(glm::vec3(
		m_secondTransform * glm::vec4(m_pViewPlane[0]->getVolCenter(), 1.0f)));
	// Jung To Do : vol1 과 vol2 resolution 다를 경우 처리 필요
	m_fThickness[1] = m_fThickness[0] * m_pgMPRengine->getBasePixelSize(0) /
		m_pgMPRengine->getBasePixelSize(1);
}

void CW3ViewMPR::VisibleImplant(int state)
{
	if (state == Qt::CheckState::Unchecked)
	{
		implant_handle_->Disable();
		CW3View2D_forMPR::drawImplantID(false);
	}
	CW3View2D_forMPR::VisibleImplant(state);
}

void CW3ViewMPR::VisibleSecond(int state)
{
	if (!m_pViewPlane[1])
	{
		initViewPlane(1);
	}

	CW3View2D_forMPR::VisibleSecond(state);
	if (state == Qt::CheckState::Checked)
	{
		setSecondVectors();
	}
}

void CW3ViewMPR::ResetView()
{
	CW3View2D_forMPR::ResetView();
	slotReoriupdate(&m_reoriMat);

	emit sigThickChange(
		(MPRViewType)Line2View[m_eMPRType][UILineType::HORIZONTAL],
		(MPRViewType)Line2View[m_eMPRType][UILineType::VERTICAL], 0.0f);
	emit sigThickChange(
		(MPRViewType)Line2View[m_eMPRType][UILineType::VERTICAL],
		(MPRViewType)Line2View[m_eMPRType][UILineType::HORIZONTAL], 0.0f);
}

void CW3ViewMPR::FitView()
{
	if (isVisible())
	{
		m_crossCenter = m_crossCenter + m_sceneTrans;
		QPointF preDir = m_crossCenter - m_pntCurViewCenterinScene;
		float preScaleSceneToGL = m_scaleSceneToGL;

		CW3View2D_forMPR::FitView();

		m_crossCenter = m_pntCurViewCenterinScene +
			preDir * preScaleSceneToGL / m_scaleSceneToGL;
	}
}

void CW3ViewMPR::updateImage()
{
	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		drawImageOnViewPlane(false, 1, false);
	}
}

void CW3ViewMPR::UpdateSliderValue()
{
	int adepth = m_pViewPlane[0]->getAvailableDetph();

	int sliderValue = (int)glm::dot(
		m_pViewPlane[0]->getVolCenter() - m_pViewPlane[0]->getPlaneCenterInVol(),
		-m_pViewPlane[0]->getUpVec());

	if (m_eMPRType != MPRViewType::CORONAL)
	{
		sliderValue = -sliderValue;
	}

	int slider_value =
		sliderValue + static_cast<int>(floorf(adepth * 0.5f + kRoundVal)) - 1;

	prev_slider_value_ = slider_value;

	if (adepth != 0 && m_pSlider)
	{
		int depth_index = adepth - 1;
		m_pSlider->setRange(0, depth_index);
		m_pSlider->setValue(slider_value);
	}
}

void CW3ViewMPR::changedDeltaSlider(int delta)
{
	if (m_eMPRType != MPRViewType::CORONAL)
	{
		delta = -delta;
	}

	emit sigWheel(m_eMPRType, translateDraw(delta), false);
}

void CW3ViewMPR::WheelView(MPRViewType type, float dist)
{
	if (!m_pLine[UILineType::HORIZONTAL] || !m_pLine[UILineType::VERTICAL])
	{
		return;
	}

	float distScene = scaledVolToScene(dist);

	if (type == MPRViewType::AXIAL)
	{
		QPointF lineNormal(
			m_pLine[UILineType::HORIZONTAL]->getNormal().x() * distScene,
			m_pLine[UILineType::HORIZONTAL]->getNormal().y() * distScene);
		m_crossCenter = m_crossCenter - lineNormal;
		setPosItemsUseCrossCenter();
	}
	else if (type == MPRViewType::SAGITTAL)
	{
		QPointF lineNormal(
			m_pLine[UILineType::VERTICAL]->getNormal().x() * distScene,
			m_pLine[UILineType::VERTICAL]->getNormal().y() * distScene);
		m_crossCenter = m_crossCenter - lineNormal;
		setPosItemsUseCrossCenter();
	}
	else if (type == MPRViewType::CORONAL)
	{
		QPointF lineNormal;
		if (m_eMPRType == MPRViewType::AXIAL)
			lineNormal =
			QPointF(m_pLine[UILineType::HORIZONTAL]->getNormal().x() * distScene,
				m_pLine[UILineType::HORIZONTAL]->getNormal().y() * distScene);
		else
			lineNormal =
			QPointF(m_pLine[UILineType::VERTICAL]->getNormal().x() * distScene,
				m_pLine[UILineType::VERTICAL]->getNormal().y() * distScene);

		m_crossCenter = m_crossCenter + lineNormal;
		setPosItemsUseCrossCenter();
	}

	if (m_pZoom3D)
	{
		QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
#ifdef ID_FLOAT
		float idx = scaledSceneToVol(cross_center_offeset.x() + m_sceneTrans.x() -
			view_center_.x());
		float idy = scaledSceneToVol(cross_center_offeset.y() + m_sceneTrans.y() -
			view_center_.y());
#else
		int idx = scaledSceneToVol(cross_center_offeset.x() + m_sceneTrans.x() -
			m_pntCurViewCenterinScene.x());
		int idy = scaledSceneToVol(cross_center_offeset.y() + m_sceneTrans.y() -
			m_pntCurViewCenterinScene.y());
#endif

		glm::vec3 ptIn3DVol = m_pViewPlane[0]->getPlaneCenterInVol() +
			m_pViewPlane[0]->getRightVec() * float(idx) +
			m_pViewPlane[0]->getBackVec() * float(idy);

		emit sigSetZoom3DMPR(m_eMPRType, m_pZoom3D->center(), m_pZoom3D->radius(), true);
		if (m_eMPRType == MPRViewType::AXIAL)
		{
			emit sigSetZoom3DVR(m_eMPRType, ptIn3DVol,
				scaledSceneToVol(m_pZoom3D->radius()));
		}
	}

	//emit sigSelectedMPR(type);
}

void CW3ViewMPR::GetViewPlaneParams(
	lightbox_resource::PlaneParams& plane_param)
{
	plane_param.up = m_pViewPlane[0]->getUpVec();
	plane_param.back = m_pViewPlane[0]->getBackVec();
	plane_param.right = m_pViewPlane[0]->getRightVec();
	plane_param.width = m_pViewPlane[0]->getImage2D()->width();
	plane_param.height = m_pViewPlane[0]->getImage2D()->height();
	plane_param.dist_from_vol_center = m_pViewPlane[0]->getDistFromVolCenter();
	plane_param.available_depth = m_pViewPlane[0]->getAvailableDetph();
}

glm::vec3 CW3ViewMPR::GetViewPlaneCenter()
{
	return m_pViewPlane[0]->getPlaneCenterInVol();
}

glm::mat4 CW3ViewMPR::GetPlaneRotateMatrix() const
{
	glm::mat4 rotate_matrix(1.0f);
	rotate_matrix[0] = glm::vec4(m_pViewPlane[0]->getRightVec(), 0.0f);
	rotate_matrix[1] = glm::vec4(m_pViewPlane[0]->getBackVec(), 0.0f);
	rotate_matrix[2] = glm::vec4(m_pViewPlane[0]->getUpVec(), 0.0f);

	return rotate_matrix;
}

const float & CW3ViewMPR::GetAngleDegree() const
{
	return m_pLine[0]->getAngleDegree();
}

void CW3ViewMPR::zoom3D(bool bToggled)
{
	m_bIsZoom3DMode = bToggled;

	if (m_bIsZoom3DMode)
	{
		if (m_pZoom3D)
		{
			m_pZoom3D->setVisible(m_bIsZoom3DMode);
			switch (m_eMPRType)
			{
			case AXIAL:
				m_pLine[UILineType::VERTICAL]->setVisibleLines(true);
				m_pLine[UILineType::HORIZONTAL]->setVisibleLines(false);
				m_pLine[UILineType::HORIZONTAL]->setVisibleThickness(false);
				break;
			case SAGITTAL:
				m_pLine[UILineType::HORIZONTAL]->setVisibleLines(true);
				m_pLine[UILineType::VERTICAL]->setVisibleLines(false);
				m_pLine[UILineType::VERTICAL]->setVisibleThickness(false);
				break;
			default:
				break;
			}
			circle_->SetVisible(true);
		}
		else
		{
			setMPRInteraction(false);
			circle_->SetVisible(false);

			m_pLine[UILineType::VERTICAL]->setVisibleThickness(false);
			m_pLine[UILineType::HORIZONTAL]->setVisibleThickness(false);
		}
	}
	else
	{
		if (m_pZoom3D)
		{
			m_pZoom3D->RemoveFromScene(scene());
			SAFE_DELETE_OBJECT(m_pZoom3D);
		}

		setMPRInteraction(true);
		circle_->SetVisible(false);
	}
}

void CW3ViewMPR::setMPRInteraction(bool bInteraction)
{
	m_pLine[UILineType::HORIZONTAL]->setVisibleLines(bInteraction);
	m_pLine[UILineType::VERTICAL]->setVisibleLines(bInteraction);
}

void CW3ViewMPR::setZoom3DMPR(const float& radius, const bool updateVR)
{
	setMPRInteraction(true);

	QPointF cross_center_offeset = m_crossCenter - m_SceneGLOffset;
	if (!m_pZoom3D)
	{
		m_pZoom3D =
			new Zoom3DCube(cross_center_offeset, m_pViewPlane[0]->getRightVec(),
				m_pViewPlane[0]->getBackVec());
		m_pZoom3D->AddToScene(scene());
		m_pZoom3D->setVisible(m_bIsZoom3DMode);
		m_pZoom3D->drawStart();
		m_pZoom3D->resize(radius);
		m_pZoom3D->drawEnd();
	}
	else
	{
		m_pZoom3D->drawStart();
		m_pZoom3D->resize(cross_center_offeset, radius);
		m_pZoom3D->drawEnd();
	}

	if (m_eMPRType == MPRViewType::AXIAL && updateVR)
	{
		float WsceneTrans = m_WglTrans / m_scaleSceneToGL;
		float HsceneTrans = m_HglTrans / m_scaleSceneToGL;

#ifdef ID_FLOAT
		float idx = scaledSceneToVol(cross_center_offeset.x() + WsceneTrans -
			view_center_.x());
		float idy = scaledSceneToVol(cross_center_offeset.y() + HsceneTrans -
			view_center_.y());
#else
		int idx = scaledSceneToVol(cross_center_offeset.x() + WsceneTrans -
			m_pntCurViewCenterinScene.x());
		int idy = scaledSceneToVol(cross_center_offeset.y() + HsceneTrans -
			m_pntCurViewCenterinScene.y());
#endif
		glm::vec3 ptIn3DVol = m_pViewPlane[0]->getPlaneCenterInVol() +
			m_pViewPlane[0]->getRightVec() * float(idx) +
			m_pViewPlane[0]->getBackVec() * float(idy);

		emit sigSetZoom3DVR(m_eMPRType, ptIn3DVol,
			scaledSceneToVol(m_pZoom3D->radius()));
	}

	switch (m_eMPRType)
	{
	case AXIAL:
		m_pLine[UILineType::VERTICAL]->setVisibleLines(true);
		m_pLine[UILineType::HORIZONTAL]->setVisibleLines(false);
		break;
	case SAGITTAL:
		m_pLine[UILineType::HORIZONTAL]->setVisibleLines(true);
		m_pLine[UILineType::VERTICAL]->setVisibleLines(false);
		break;
	default:
		break;
	}
	circle_->SetVisibleCircle(false);
	circle_->SetVisibleCenter(true);
}

void CW3ViewMPR::rotateZoom3DCube(const glm::mat4& rotMat)
{
	if (!m_pZoom3D)
	{
		return;
	}

	m_pZoom3D->setCenter(m_crossCenter - m_SceneGLOffset);
	m_pZoom3D->rotate(rotMat, m_pViewPlane[0]->getRightVec(),
		m_pViewPlane[0]->getBackVec());
}

void CW3ViewMPR::drawBackground(QPainter* painter, const QRectF& rect)
{
	if (m_bReoriupdated)
	{
		m_pGLWidget->doneCurrent();
		drawImageOnViewPlane(true, 0, m_pRender3DParam->m_pNerve->isVisible());
		m_pGLWidget->makeCurrent();

		m_bReoriupdated = false;
	}

	CW3View2D_forMPR::drawBackground(painter, rect);
}

bool CW3ViewMPR::event(QEvent* event)
{
	if (!m_is2Dready)
	{
		return QGraphicsView::event(event);
	}

	if (event->type() == QEvent::Leave)
	{
		for (int i = 0; i < UILineType::LINE_END; ++i)
		{
			// m_pLine[i]->setVisibleTxt(false);
			m_pLine[i]->setVisibleThickness(false);
			m_pLine[i]->initLineWidthColor();
		}
		circle_->SetVisible(false);

		implant_handle_->Disable();
		CW3View2D_forMPR::drawImplantID(false);
	}

	return QGraphicsView::event(event);
}

void CW3ViewMPR::transformPositionItems(const QTransform& transform)
{
	m_crossCenter = transform.map(m_crossCenter);

	for (int i = 0; i < UILineType::LINE_END; i++)
	{
		if (m_pLine[i]) m_pLine[i]->transformItems(transform);
	}

	if (circle_)
	{
		circle_->SetPos(transform.map(circle_->Pos()));
	}

	if (m_pZoom3D)
	{
		m_pZoom3D->transformItems(transform);
	}

	m_rect->setRect(-m_sceneWinView + 2, -m_sceneHinView + 2,
		m_sceneWinView * 2.0f - 5, m_sceneHinView * 2.0f - 5);
	m_rect->setPos(m_pntCurViewCenterinScene);
	m_rectShadow->setRect(-m_sceneWinView, -m_sceneHinView, m_sceneWinView * 2.0f,
		m_sceneHinView * 2.0f);
	m_rectShadow->setPos(m_pntCurViewCenterinScene);
}

void CW3ViewMPR::updateImageOnViewPlane()
{
	drawImageOnViewPlane(false, 0, m_pRender3DParam->m_pNerve->isVisible());

	if (m_drawVolId == 1 || m_isDrawBoth)
	{
		drawImageOnViewPlane(false, 1, false);
	}

	scene()->update();
}

void CW3ViewMPR::updateMPRPhoto()
{
	m_isFacePhotoUpdated = true;

	m_modelPhoto = m_pgRcontainer->getFacePhoto3D()->getSRtoVol() *
		glm::scale(*m_pgVREngine->getVolRange(0));
}

void CW3ViewMPR::drawImageOnViewPlane(bool isFitIn, int id, bool isCanalShown)
{
	CW3View2D_forMPR::drawImageOnViewPlane(isFitIn, id, isCanalShown);

	if (slice_number_ && m_pSlider)
	{
		if (slider_value_set_)
		{
			UpdateSliderValue();
		}

		int sliceNum = m_pSlider->value() + 1;
		slice_number_->setPlainText(QString::number(sliceNum));
		SetSliceNumberPosition();
	}
}

void CW3ViewMPR::ApplyPreferences()
{
	CW3View2D_forMPR::ApplyPreferences();

	bool is_text_visible = !(hide_all_view_ui_ || single_view_hide_ui_);
	show_slice_numbers_ = GlobalPreferences::GetInstance()->preferences_.general.display.show_slice_numbers;
	if (slice_number_)
	{
		slice_number_->setVisible(is_text_visible && show_slice_numbers_);
	}
}

void CW3ViewMPR::slotSyncScale(float scale)
{
	m_scalePre = m_scale;
	m_scale = scale;

	if (m_scale < 0.3f)
	{
		m_scale = 0.3f;
	}

	setViewProjection();

	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_scale / m_scalePre);
	scene()->update();
}

void CW3ViewMPR::slotSyncWindowing(int level, int width)
{
	m_nAdjustWindowLevel = level;
	m_nAdjustWindowWidth = width;
	changeWLWW();
	scene()->update();
}

void CW3ViewMPR::SetSliceNumberPosition()
{
	if (!slice_number_ || !m_pProxySlider)
	{
		return;
	}

	slice_number_->setPos(mapToScene(
		m_pProxySlider->sceneBoundingRect().right() -
		slice_number_->sceneBoundingRect().width(),
		(height() + m_pProxySlider->sceneBoundingRect().height()) * 0.5f));
}

void CW3ViewMPR::slotTranslateImplant()
{
	int implant_id = implant_handle_->selected_id();
	QPointF implant_scene_pos = implant_handle_->scenePos();
	SetImplantIDTextPosUnderImplantHandle();
	ChangeCircleInMove(implant_scene_pos);
	emit sigTranslateImplant(implant_id, MapSceneToVol(implant_scene_pos));
	//VisibleImplant(Qt::CheckState::Checked);
}

void CW3ViewMPR::slotRotateImplant(float degree_angle)
{
	glm::vec3 up_vector = m_pViewPlane[0]->getUpVec();
	up_vector.x = up_vector.x * -1.0f;

	SetImplantIDTextPosUnderImplantHandle();
	emit sigRotateImplant(
		implant_handle_->selected_id(),
		up_vector,
		-degree_angle
	);
	int implant_id = implant_handle_->selected_id();
	//VisibleImplant(Qt::CheckState::Checked);
}

void CW3ViewMPR::AdjustImplant(bool checked)
{
	adjust_implant_ = checked;
	if (!adjust_implant_)
	{
		implant_handle_->Disable();
		CW3View2D_forMPR::drawImplantID(false);
	}
}

void CW3ViewMPR::SetImplantIDTextPosUnderImplantHandle()
{
	if (!implant_handle_ || !implant_id_text_ ||
		implant_handle_->selected_id() < 0)
	{
		return;
	}

	int id = implant_handle_->selected_id();
	QPointF implant_pos = implant_handle_->scenePos();
	QPointF text_pos(
		implant_pos.x() - implant_id_text_->sceneBoundingRect(id).width() / 2,
		implant_pos.y() + implant_handle_->sceneBoundingRect().height() / 2
	);
	implant_id_text_->setPos(text_pos);
}

void CW3ViewMPR::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	if (type != common::CommonToolTypeOnOff::NONE)
	{
		implant_handle_->Disable();
	}

	CW3View2D_forMPR::SetCommonToolOnOff(type);
}

const QVector2D& CW3ViewMPR::GetLineNormal(UILineType line)
{
	return m_pLine[line]->getNormal();
}

void CW3ViewMPR::DrawArch(ArchTypeID arch_type)
{
	if (m_eMPRType != MPRViewType::AXIAL)
	{
		return;
	}

	arch_->Clear();
	draw_arch_ = true;
	arch_type_ = arch_type;
}

void CW3ViewMPR::slotArchEndEdit()
{
	int slice_number = m_pSlider->value();

	const unsigned int depth = ResourceContainer::GetInstance()->GetMainVolume().depth();
	int pano_slice_number = (depth / 2) + (slice_number - ((m_pSlider->maximum() + m_pSlider->minimum()) / 2));

	std::vector<QPointF> scene_points = arch_->GetCtrlPoints();
	std::vector<glm::vec3> volume_points;
	for (int i = 0; i < scene_points.size(); ++i)
	{
		glm::vec3 volume_point = MapSceneToVol(scene_points.at(i));
		//volume_point.z = pano_slice_number;
		volume_points.push_back(volume_point);
	}

#if 0
	QApplication::processEvents();
#endif

	qDebug() << "pano_slice_number :" << pano_slice_number;

	emit sigUpdateArch(arch_type_, volume_points, orientation_matrix_, pano_slice_number);
};
