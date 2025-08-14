#include "W3View2D.h"

/*=========================================================================

File:			class CW3View2D
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-21
Last date:		2016-06-04

=========================================================================*/
#include <QScrollBar>
#include <QGraphicsProxyWidget>
#include <QApplication>
#include <QMouseEvent>
#include <qgraphicsscene.h>
#include <qopenglwidget.h>

#include <Engine/Common/Common/color_will3d.h>
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/W3Define.h"

#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include "../UIGLObjects/W3GLObject.h"
#include "../UIGLObjects/W3GLNerve.h"
#include <Engine/UIModule/UIPrimitive/image_filter_selector.h>
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/grid_lines.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VRengineTextures.h"

using common::Logger;
using common::LogType;

namespace {
const float kInitScale2D = 1.0f;
// TODO smseo : 볼륨 좌표를 넣기 전 임시 값
const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

CW3View2D::CW3View2D(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					 CW3ResourceContainer *Rcontainer, common::ViewTypeID eType,
					 QWidget *pParent) :
	m_eViewType(eType), m_pgVREngine(VREngine), m_pgMPRengine(MPRengine),
	m_pgRcontainer(Rcontainer), HU_value_(new SimpleTextItem), QGraphicsView(pParent) {
	m_pRender3DParam = new CW3Render3DParam(eType, m_pGLWidget,
											m_pgRcontainer->getCollisionContainer(),
											m_pgRcontainer->getImplantThereContainer());

	this->setScene(new QGraphicsScene(this));
	this->scene()->setSceneRect(-3000, -3000, 6000, 6000);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setRenderHint(QPainter::Antialiasing, true);
	this->horizontalScrollBar()->installEventFilter(this);
	this->setMouseTracking(true);

	setInitScale();

	m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
						 glm::vec3(0.0f, 0.0f, 0.0f),
						 glm::vec3(0.0f, 1.0f, 0.0f));

	measure_tools_ = new MeasureTools(m_eViewType, this->scene());

	if (m_eViewType != common::ViewTypeID::ENDO) {
		ruler_ = new CW3ViewRuler(ColorView::k3D, nullptr);
		this->scene()->addItem(ruler_);
		ruler_->setVisible(false);

		grid_ = new GridLines();
		this->scene()->addItem(grid_);
	}

	this->scene()->addItem(HU_value_);
	HU_value_->setVisible(false);

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	sharpen_filter_text_ = new ImageFilterSelector();
	sharpen_filter_text_->SetFont(font);
	sharpen_filter_text_->setVisible(false);
	scene()->addItem(sharpen_filter_text_);

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	sharpen_level_ = settings.value("SLICE/default_filter", 0).toInt();
	sharpen_filter_text_->SetLevel(sharpen_level_);

	connections();

	this->setStyleSheet(CW3Theme::getInstance()->appQMenuStyleSheet());

	ApplyPreferences();
}

CW3View2D::~CW3View2D() {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(measure_tools_);
	SAFE_DELETE_OBJECT(m_pViewPlane[0]);
	SAFE_DELETE_OBJECT(m_pViewPlane[1]);

	SAFE_DELETE_OBJECT(m_pRender3DParam);
	SAFE_DELETE_OBJECT(m_pGLWidget);
}
#ifndef WILL3D_VIEWER
void CW3View2D::exportProject(ProjectIOView & out) {
	out.SaveViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans, m_nAdjustWindowLevel, m_nAdjustWindowWidth);
}

void CW3View2D::importProject(ProjectIOView& in) {
	in.LoadViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans, m_nAdjustWindowLevel, m_nAdjustWindowWidth);
	import_proj_info_.view_scale = m_scale;
	import_proj_info_.gl_trans_x = m_WglTrans;
	import_proj_info_.gl_trans_y = m_HglTrans;
	import_proj_info_.is_import = true;

	measure_tools_->ImportMeasureResource();
}
#endif
void CW3View2D::reset() {
	m_is2Dready = false;
	m_nAdjustWindowLevel = 0;
	m_nAdjustWindowWidth = 0;

	m_drawVolId = 0;
	m_isDrawBoth = 0;

	SAFE_DELETE_OBJECT(m_pRender3DParam);
	m_pRender3DParam = new CW3Render3DParam(m_eViewType, m_pGLWidget,
											m_pgRcontainer->getCollisionContainer(),
											m_pgRcontainer->getImplantThereContainer());

	m_isItemInitialized = false;
	m_isReconSwitched = false;

	m_isOnlyItemUpdate = false;

	last_scene_pos_.setX(0.0f);
	last_scene_pos_.setY(0.0f);
	last_view_pos_.setX(0.0f);
	last_view_pos_.setY(0.0f);

	recon_type_ = common::ReconTypeID::MPR;

	m_scaleSceneToGL = 1.0f;
	m_scaleVolToGL = 2.0f;
	m_Wglorig = 1.0f;
	m_Hglorig = 1.0f;
	m_Dglorig = 1.0f;
	m_Wgl = 1.0f;
	m_Hgl = 1.0f;
	m_Dgl = 1.0f;
	m_Wglpre = 1.0f;
	m_Hglpre = 1.0f;
	m_Dglpre = 1.0f;
	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;

	m_vBackVec = glm::vec3();

	setInitScale();

	m_model = glm::mat4(1.0f);
	m_modelSecond = glm::mat4(1.0f);
	m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_projection = glm::mat4(1.0f);

	common_tool_type_ = common::CommonToolTypeOnOff::NONE;

	m_scalePre = 1.0f;

	hide_all_view_ui_ = false;

#if 0
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	sharpen_level_ = settings.value("SLICE/default_filter", 0).toInt();
	sharpen_filter_text_->SetLevel(sharpen_level_);
#endif

	m_bLoadProject = false;

	if (m_pSlider)
		m_pSlider->setValue(0);

	ApplyPreferences();
}

void CW3View2D::connections() {
	connect(sharpen_filter_text_, SIGNAL(sigPressed(int)), this, SLOT(slotSelectFilter(int)));

	/*if (m_pSlider)
		connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChangedValueSlider(int)));*/

	connect(measure_tools_, SIGNAL(sigGetProfileData(QPointF, QPointF, std::vector<short>&)),
			this, SLOT(slotSetProfileData(QPointF, QPointF, std::vector<short>&)));
	connect(measure_tools_, SIGNAL(sigGetROIData(QPointF, QPointF, std::vector<short>&)),
			this, SLOT(slotSetROIData(QPointF, QPointF, std::vector<short>&)));
}

void CW3View2D::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) {
	switch (type) {
	case common::CommonToolTypeOnce::V_RESET:
		ResetView();
		break;
	case common::CommonToolTypeOnce::V_FIT:
		FitView();
		break;
	case common::CommonToolTypeOnce::V_INVERT:
		InvertView(on);
		break;
	case common::CommonToolTypeOnce::V_GRID:
		GridOnOff(on);
		break;
	case common::CommonToolTypeOnce::V_HIDE_TXT:
		HideText(on);
		break;
	case common::CommonToolTypeOnce::V_HIDE_UI:
		HideUI(on);
		break;
	case common::CommonToolTypeOnce::M_HIDE_M:
		HideMeasure(on);
		break;
	case common::CommonToolTypeOnce::M_DEL_ALL:
		DeleteAllMeasure();
		break;
	case common::CommonToolTypeOnce::M_DEL_INCOMPLETION:
		measure_tools_->ClearUnfinishedItem();
		break;
	default:
		break;
	}
}

void CW3View2D::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	common_tool_type_ = type;

	if (!isVisible())
		return;

	CW3Cursor::SetViewCursor(common_tool_type_);
	measure_tools_->ClearUnfinishedItem();
	measure_tools_->SetMeasureType(common_tool_type_);
}

void CW3View2D::setWLWW() {
	auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	m_nAdjustWindowLevel = vol.windowCenter();
	m_nAdjustWindowWidth = vol.windowWidth();
	changeWLWW();
}

void CW3View2D::changeWLWW() {
	int windowWidth = m_bInvertWindowWidth ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;
	m_pRender3DParam->set_windowing_min((m_nAdjustWindowLevel - windowWidth * 0.5f) / 65535.0f);
	m_pRender3DParam->set_windowing_norm(windowWidth / 65535.0f);

	Logger::instance()->Print(LogType::INF, "WL / WW : "
							  + std::to_string(m_nAdjustWindowLevel
											   /*+ (int)m_pgVREngine->getVol(0)->intercept()*/)
							  + " / " + std::to_string(m_nAdjustWindowWidth));
}

void CW3View2D::GridOnOff(bool grid_on) {
	grid_visible_ = grid_on;
	if (grid_)
	{
		grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
	}
}

void CW3View2D::drawImageOnTexture(CW3ViewPlane *viewPlane, bool isDifferentSize) {
	if (!isVisible())
		return;

	m_pgVREngine->makeCurrent();

	CW3GLFunctions::update2DTex(
		m_pRender3DParam->m_VRtextures.m_texHandler[0],
		viewPlane->getWidth(), viewPlane->getHeight(),
		viewPlane->getImage2D()->getData(), isDifferentSize);

	if (m_pRender3DParam->m_pNerve && m_pRender3DParam->m_pNerve->isVisible()) {
		CW3GLFunctions::update2DTex(
			m_pRender3DParam->m_VRtextures.m_texHandler[1],
			viewPlane->getWidth(), viewPlane->getHeight(),
			viewPlane->getMask(), isDifferentSize);
	}

	m_pgVREngine->doneCurrent();

	if (import_proj_info_.is_import && viewPlane->getImage2D()) {
		m_scale = import_proj_info_.view_scale;
		m_WglTrans = import_proj_info_.gl_trans_x;
		m_HglTrans = import_proj_info_.gl_trans_y;
		measure_tools_->Initialize(ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize(),
								   m_scaleSceneToGL);
		import_proj_info_.is_import = false;
	}

	if (!m_is2Dready) {
		m_Wglorig = m_Wglpre;
		m_Hglorig = m_Hglpre;
		m_Dglorig = m_Dglpre;
	}

	m_is2Dready = true;

	if (ruler_)
		ruler_->setVisible(!hide_all_view_ui_ && show_rulers_);
}

void CW3View2D::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();
	if (m_is2Dready && isVisible()) {
		if (!m_pRender3DParam->m_plane->getVAO()) {
			m_pRender3DParam->m_plane->clearVAOVBO();

			unsigned int vao = 0;
			m_pgVREngine->initVAOplane(&vao);
			m_pRender3DParam->m_plane->setVAO(vao);
			m_pRender3DParam->m_plane->setNindices(6);
		}

		m_pRender3DParam->m_plane->setMVP(m_model, m_view, m_projection);
		m_pRender3DParam->m_width = this->width();
		m_pRender3DParam->m_height = this->height();

		if (m_drawVolId == 0 || m_isDrawBoth)
			m_pgVREngine->RenderMPR(m_pRender3DParam, 0, m_isDrawBoth, m_isReconSwitched);

		if (m_drawVolId == 1 || m_isDrawBoth) {
			m_pGLWidget->doneCurrent();
			m_pgVREngine->makeCurrent();

			CW3GLFunctions::update2DTex(
				m_pRender3DParam->m_VRtextures.m_texHandler[2],
				m_pViewPlane[1]->getWidth(), m_pViewPlane[1]->getHeight(),
				m_pViewPlane[1]->getImage2D()->getData(), true);

			m_pgVREngine->doneCurrent();
			m_pGLWidget->makeCurrent();
			m_mvpSecond = m_projection * m_view*m_modelSecond;
			m_pRender3DParam->m_plane->setMVP(m_modelSecond, m_view, m_projection);

			m_pgVREngine->RenderMPR(m_pRender3DParam, 2, m_isDrawBoth, m_isReconSwitched);
		}
	} else {
		CW3GLFunctions::clearView(false);

		if (ruler_)
			ruler_->setVisible(false);
	}

	painter->endNativePainting();
}

void CW3View2D::wheelEvent(QWheelEvent *event) {
	event->ignore();
}

void CW3View2D::setProjection() {
	const float viewRatio = m_sceneWinView / m_sceneHinView;
	const float imgRatio = m_Wglpre / m_Hglpre;

	if (viewRatio < imgRatio) {
		proj_left_ = -m_Wglpre / m_scale + m_WglTrans;
		proj_right_ = m_Wglpre / m_scale + m_WglTrans;
		proj_bottom_ = -m_Wglpre / (viewRatio*m_scale) - m_HglTrans;
		proj_top_ = m_Wglpre / (viewRatio*m_scale) - m_HglTrans;

		m_scaleSceneToGL = m_Wglpre / (m_sceneWinView*m_scale);
	} else {
		proj_left_ = -m_Hglpre * viewRatio / m_scale + m_WglTrans;
		proj_right_ = m_Hglpre * viewRatio / m_scale + m_WglTrans;
		proj_bottom_ = -m_Hglpre / m_scale - m_HglTrans;
		proj_top_ = m_Hglpre / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_Hglpre / (m_sceneHinView*m_scale);
	}

	m_projection = glm::ortho(proj_left_, proj_right_, proj_bottom_, proj_top_, 0.0f, 2.0f);
	//measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);

	setViewRulerValue();
	SetGridValue();
}

void CW3View2D::clearGL() {
	m_pRender3DParam->ClearGLObjects();
}

void CW3View2D::resizeEvent(QResizeEvent *pEvent) {
	if (!isVisible())
		return;

	QGraphicsView::resizeEvent(pEvent);

	const int width = this->width();
	const int height = this->height();
	fitInView(0, 0, width, height);

	if (!m_isIntendedResize) {
		QPointF leftTop = mapToScene(QPoint(0, 0));
		QPointF rightBottom = mapToScene(QPoint(width, height));
		m_sceneWinView = (rightBottom.x() - leftTop.x()) * 0.5f;
		m_sceneHinView = (rightBottom.y() - leftTop.y()) * 0.5f;

		rightBottom = mapToScene(QPoint(width - 1, height - 1));

		m_pntCurViewCenterinScene.setX((rightBottom.x() + leftTop.x()) * 0.5f);
		m_pntCurViewCenterinScene.setY((rightBottom.y() + leftTop.y()) * 0.5f);

		m_SceneGLOffset = mapToScene(QPoint(0, 2)) - leftTop;
		m_SceneGLOffset.setY(m_SceneGLOffset.y() - 1.0f);

		m_isIntendedResize = true;

		measure_tools_->SetCenter(m_pntCurViewCenterinScene);
		measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);
	} else {
		resizeScene();
	}

	if (m_pWorldAxisItem) {
		m_pWorldAxisItem->SetSize(100, 100);
		m_pWorldAxisItem->setPos(mapToScene(width - 60, height - 60));
	}

	setViewRulerValue();
	SetGridValue();

	if (direction_text_) {
		QPointF posRightText = mapToScene(((float)width * 0.1f), ((float)height * 0.5f));
		direction_text_->setPos(posRightText);
	}

	HU_value_->setPos(mapToScene(30, height - 70));

	if (m_pProxySlider) {
		m_pProxySlider->resize(10, std::min(height * 0.33f, 159.0f));
		m_pProxySlider->setPos(mapToScene(width - common::ui_define::kViewMarginWidth - m_pProxySlider->sceneBoundingRect().width(),
										  height*0.5f - m_pProxySlider->sceneBoundingRect().height() * 0.5f));
	}

	QPointF posFilterItem = mapToScene(width - common::ui_define::kViewMarginWidth - sharpen_filter_text_->sceneBoundingRect().width(),
									   common::ui_define::kViewFilterOffsetY);
	sharpen_filter_text_->setPos(posFilterItem);
}

void CW3View2D::resizeScene() {
	float preScale = m_scaleSceneToGL;
	QPointF	preViewCenterInScene = m_pntCurViewCenterinScene;

	QPointF leftTop = mapToScene(QPoint(0, 0));
	QPointF rightBottom = mapToScene(QPoint(this->width(), this->height()));
	m_sceneWinView = (rightBottom.x() - leftTop.x())*0.5f;
	m_sceneHinView = (rightBottom.y() - leftTop.y())*0.5f;

	rightBottom = mapToScene(QPoint(this->width() - 1, this->height() - 1));

	m_pntCurViewCenterinScene.setX((rightBottom.x() + leftTop.x())*0.5f);
	m_pntCurViewCenterinScene.setY((rightBottom.y() + leftTop.y())*0.5f);

	m_SceneGLOffset = mapToScene(QPoint(0, 2)) - leftTop;
	m_SceneGLOffset.setY(m_SceneGLOffset.y() - 1.0f);

	setViewProjection();

	float scale = preScale / m_scaleSceneToGL;
	transformItems(preViewCenterInScene, m_pntCurViewCenterinScene, scale);

	measure_tools_->SetCenter(m_pntCurViewCenterinScene);
	measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);
}

void CW3View2D::ScaleView(const QPoint& curr_view_pos) {
	m_scalePre = m_scale;
	m_scale = m_scale + float(last_view_pos_.y() - curr_view_pos.y()) / float(this->height());

	if (m_scale < 0.3f)
		m_scale = 0.3f;

	setViewProjection();

	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_scale / m_scalePre);

	last_view_pos_ = curr_view_pos;

	if (m_scale / m_scalePre > 1.0f)
		QApplication::setOverrideCursor(CW3Cursor::ZoomInCursor());
	else
		QApplication::setOverrideCursor(CW3Cursor::ZoomOutCursor());
}

void CW3View2D::PanningView(const QPointF& curr_scene_pos) {
	float transX = last_scene_pos_.x() - curr_scene_pos.x();
	float transY = last_scene_pos_.y() - curr_scene_pos.y();

	m_WglTrans += transX * m_scaleSceneToGL;
	m_HglTrans += transY * m_scaleSceneToGL;

	setViewProjection();
	transformItems(QPointF(-transX, -transY));
	last_scene_pos_ = curr_scene_pos;
	QApplication::setOverrideCursor(CW3Cursor::ClosedHandCursor());
}

void CW3View2D::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (hide_all_view_ui_ &&
		common_tool_type_ != common::CommonToolTypeOnOff::V_PAN &&
		common_tool_type_ != common::CommonToolTypeOnOff::V_ZOOM &&
		common_tool_type_ != common::CommonToolTypeOnOff::V_LIGHT)
		return;

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
		return;

	last_view_pos_ = event->pos();
	last_scene_pos_ = mapToScene(event->pos());

	Qt::MouseButton button = event->button();
	if (button == Qt::LeftButton) {
		if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_)) {
			measure_tools_->ProcessMousePressed(last_scene_pos_, kTempPos);
		}
	} else if (button == Qt::RightButton) {
		is_measure_delete_event_ = true;
	}
}

void CW3View2D::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event); 
	
	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		is_measure_delete_event_ = false;
	}

	if (hide_all_view_ui_)
		return;

	CW3Cursor::SetViewCursor(common_tool_type_);
	QPointF scene_pos = mapToScene(event->pos());
	if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_)) {
		if (measure_tools_->IsDrawing()) {
			measure_tools_->ProcessMouseMove(scene_pos, kTempPos);
			return;
		}
	}

	Qt::MouseButtons buttons = event->buttons();
	if (buttons == Qt::NoButton) {
		// empty if statement
	} else {
		if (buttons == Qt::RightButton)
			is_measure_delete_event_ = false;

		if (measure_tools_->IsSelected() &&
			measure_tools_->IsMeasureInteractionAvailable(common_tool_type_)) {
			measure_tools_->ProcessMouseMove(scene_pos, kTempPos);
			return;
		}
		switch (common_tool_type_) {
		case common::CommonToolTypeOnOff::V_LIGHT:
			m_nAdjustWindowLevel += (last_scene_pos_.x() - scene_pos.x()) * 2;
			m_nAdjustWindowWidth += (last_scene_pos_.y() - scene_pos.y()) * 2;
			changeWLWW();
			last_scene_pos_ = scene_pos;
			scene()->update();
			break;
		case common::CommonToolTypeOnOff::V_PAN:
		case common::CommonToolTypeOnOff::V_PAN_LR:
			PanningView(scene_pos);
			scene()->update();
			break;
		case common::CommonToolTypeOnOff::V_ZOOM:
		case common::CommonToolTypeOnOff::V_ZOOM_R:
			ScaleView(event->pos());
			scene()->update();
			break;
		default:
			break;
		}
	}
}

void CW3View2D::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	bool is_current_measure_available =
		measure_tools_->IsMeasureInteractionAvailable(common_tool_type_);

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
		return;

	CW3Cursor::SetViewCursor(common_tool_type_);
	Qt::MouseButton button = event->button();

	if ((is_current_measure_available && is_measure_delete_event_) ||
		common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW)
	{
		measure_tools_->ProcessMouseReleased(button, mapToScene(event->pos()), kTempPos);
	}

	is_measure_delete_event_ = false;
}

void CW3View2D::mouseDoubleClickEvent(QMouseEvent *event) {
	QGraphicsView::mouseDoubleClickEvent(event);

	//if (event->buttons() & Qt::LeftButton)
		measure_tools_->ProcessMouseDoubleClick(event->button(), mapToScene(event->pos()), kTempPos);

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (event->button() == Qt::RightButton)
	{
		is_measure_delete_event_ = true;
	}
}

void CW3View2D::ResetView() {
	hide_all_view_ui_ = false;

	setWLWW();
	resetFromViewTool();
}

void CW3View2D::FitView() {
	if (!isVisible())
		return;

	resetFromViewTool();
}

void CW3View2D::InvertView(bool bToggled) {
	m_bInvertWindowWidth = bToggled;

	changeWLWW();

	if (isVisible())
		scene()->update();
}

void CW3View2D::HideText(bool bToggled) {
	// HideUI 로 통일
}

void CW3View2D::HideUI(bool bToggled) {
	hide_all_view_ui_ = bToggled;
	SetVisibleItems();
}

void CW3View2D::HideMeasure(bool toggled) {
	measure_tools_->SetVisibleFromToolbar(!toggled);
}

void CW3View2D::DeleteAllMeasure()
{
	measure_tools_->DeleteAllMeasure();
}

void CW3View2D::slotSetProfileData(const QPointF & start_scene_pos,
								   const QPointF & end_scene_pos,
								   std::vector<short>& data) {
	const int nWidth = m_pViewPlane[0]->getWidth();
	const int nHeight = m_pViewPlane[0]->getHeight();
	const float scale = m_scaleSceneToGL / m_scaleVolToGL;

	const float trans_x = m_WglTrans / m_scaleVolToGL;
	const float trans_y = m_HglTrans / m_scaleVolToGL;
	QPointF posFirst = start_scene_pos - m_pntCurViewCenterinScene;
	posFirst.setX(posFirst.x()*scale + nWidth * 0.5f + trans_x);
	posFirst.setY(posFirst.y()*scale + nHeight * 0.5f + trans_y);

	QPointF posEnd = end_scene_pos - m_pntCurViewCenterinScene;
	posEnd.setX(posEnd.x()*scale + nWidth * 0.5f + trans_x);
	posEnd.setY(posEnd.y()*scale + nHeight * 0.5f + trans_y);

	QPointF size = posEnd - posFirst;
	const int nLength = (int)sqrt((int)(size.rx() * size.rx() + size.ry() * size.ry()));

	const float step_x = size.rx() / nLength;
	const float step_y = size.ry() / nLength;
	float px = posFirst.rx();
	float py = posFirst.ry();

	data.resize(nLength);
	const float intercept = ResourceContainer::GetInstance()->GetMainVolume().intercept();
	unsigned short *img_buffer = m_pViewPlane[0]->getImage2D()->getData();
	for (int i = 0; i < nLength; i++) {
		px += step_x;
		py += step_y;
		if (px > nWidth || py > nHeight || px < 0 || py < 0) {
			data[i] = common::dicom::kInvalidHU;
		} else {
			data[i] = img_buffer[(int)py * nWidth + (int)px] + intercept;
		}
	}
}

void CW3View2D::slotSetROIData(const QPointF& start_scene_pos,
							   const QPointF& end_scene_pos,
							   std::vector<short>& data) {
	const int nWidth = m_pViewPlane[0]->getWidth();
	const int nHeight = m_pViewPlane[0]->getHeight();
	const float scale = m_scaleSceneToGL / m_scaleVolToGL;

	float trans_x = m_WglTrans / m_scaleVolToGL;
	float trans_y = m_HglTrans / m_scaleVolToGL;
	QPointF posFirst = start_scene_pos - m_pntCurViewCenterinScene;
	posFirst.setX(posFirst.x()*scale + nWidth * 0.5f + trans_x);
	posFirst.setY(posFirst.y()*scale + nHeight * 0.5f + trans_y);

	QPointF posEnd = end_scene_pos - m_pntCurViewCenterinScene;
	posEnd.setX(posEnd.x()*scale + nWidth * 0.5f + trans_x);
	posEnd.setY(posEnd.y()*scale + nHeight * 0.5f + trans_y);

	const int start_x = static_cast<int>(posFirst.x());
	const int end_x = static_cast<int>(posEnd.x());
	const int start_y = static_cast<int>(posFirst.y());
	const int end_y = static_cast<int>(posEnd.y());

	const float intercept = ResourceContainer::GetInstance()->GetMainVolume().intercept();
	const short empty_value = intercept - 1;
	unsigned short *img_buffer = m_pViewPlane[0]->getImage2D()->getData();

	data.reserve((end_x - start_x)*(end_y - start_y));
	for (int y = start_y; y < end_y; ++y) {
		for (int x = start_x; x < end_x; ++x) {
			if (x < nWidth && y < nHeight && x >= 0 && y >= 0) {
				data.push_back(img_buffer[y * nWidth + x] + intercept);
			} else {
				data.push_back(intercept);
			}
		}
	}
}

void CW3View2D::leaveEvent(QEvent * event) {
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
	HU_value_->setVisible(false);
}

void CW3View2D::enterEvent(QEvent * event) {
	QGraphicsView::enterEvent(event);
	CW3Cursor::SetViewCursor(common_tool_type_);
	if (recon_type_ == common::ReconTypeID::MPR)
	{
		bool is_visible = !(hide_all_view_ui_);
		HU_value_->setVisible(is_visible);
	}
}

void CW3View2D::setVisible(bool state) {
	if (measure_tools_)
	{
		measure_tools_->SetVisibleFromView(state);
	}

	/////////////////////////////////////////////////////////////////
	//thyoo 170124: QOpenGLWidget memory issue. (offscreen 연산용 GPU로 랜더링 하는 경우에만 해당)
	//setVisible(false)가 되었을 때 QOpenGLWidget이 점유하고있는 GPU 메모리가
	//해제가 안되어서 모든 view가 활성화 되었을 때 GPU 메모리 낭비가 심하다.
	//그래서 메모리 해제를 해주어야 하는 상황인데, QOpenGLWidget의 FBO, Texture를 핸들링 할 수 없기 때문에 강제로 Delete한다.
	//setViewport로 대상을 바꾸어주면 이 view의 child인 QOpenGLWidget은 자동으로 delete 된다.
	/////////////////////////////////////////////////////////////////
	if (state)
		newQOpenGLWidget();

	QGraphicsView::setVisible(state);

	if (state) {
		if (m_is2Dready) {
			drawImageOnTexture(m_pViewPlane[0], false);
		}
	} else {
		deleteQOpenGLWidget();
	}
}

void CW3View2D::SetVisibleItems() {
	bool is_text_visible = !(hide_all_view_ui_);
	bool isNotVR = recon_type_ == common::ReconTypeID::MPR || recon_type_ == common::ReconTypeID::X_RAY;
	
	if (ruler_)
	{
		ruler_->setVisible(!hide_all_view_ui_ && show_rulers_);
	}

	if (grid_)
	{
		grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
	}

	if (m_pProxySlider)
		m_pProxySlider->setVisible(!hide_all_view_ui_ && isNotVR);

	if (direction_text_)
		direction_text_->setVisible(is_text_visible && isNotVR);

	if (m_pWorldAxisItem) {
	  m_pWorldAxisItem->setVisible(!hide_all_view_ui_ && !isNotVR);
	  sharpen_filter_text_->setVisible(is_text_visible && isNotVR);
	}

	HU_value_->setVisible(!hide_all_view_ui_);
}

void CW3View2D::setViewRulerValue() {
	if (!ruler_)
		return;

	QPointF viewSizeInScene = mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f);

	const float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	QPointF viewLength = viewSizeInScene * pixelSpacing * m_scaleSceneToGL / m_scaleVolToGL;

	ruler_->setViewRuler(viewSizeInScene.x(), viewSizeInScene.y(),
						 viewLength.x(), viewLength.y(),
						 m_pntCurViewCenterinScene - m_SceneGLOffset);
}

void CW3View2D::SetGridValue() {
	if (!grid_)
		return;
	QPointF viewSizeInScene = mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f);

	float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	QPointF viewLength = viewSizeInScene * pixelSpacing * m_scaleSceneToGL / m_scaleVolToGL;

	int index = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing;
	int spacing = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing_preset[index];
	grid_->SetGrid(viewSizeInScene.x(), viewSizeInScene.y(),
				   viewLength.x(), viewLength.y(),
				   m_pntCurViewCenterinScene - m_SceneGLOffset, static_cast<float>(spacing));
	grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
}

void CW3View2D::ChangeViewWidthLength(float length) {
	float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	float newViewWidthInScene = length * m_scaleVolToGL / m_scaleSceneToGL / pixelSpacing;
	float oldViewWidthInScene = (mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f)).x();

	if (newViewWidthInScene <= 0.0f)
	{
		return;
	}

	m_scalePre = m_scale;
	m_scale *= oldViewWidthInScene / newViewWidthInScene;

	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_scale / m_scalePre);

	setViewProjection();
	scene()->update();
}

void CW3View2D::ChangeViewHeightLength(float length) {
	float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	float newViewHeightInScene = length * m_scaleVolToGL / m_scaleSceneToGL / pixelSpacing;
	float oldViewHeightInScene = (mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f)).y();

	if (newViewHeightInScene <= 0.0f)
	{
		return;
	}

	m_scalePre = m_scale;
	m_scale *= oldViewHeightInScene / newViewHeightInScene;

	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_scale / m_scalePre);

	setViewProjection();
	scene()->update();
}

void CW3View2D::slotSelectFilter(const int index) {
	sharpen_level_ = index;
}

void CW3View2D::slotChangedValueSlider(int value) {
	if (!m_pSlider)
		return;

	if (m_is2Dready && m_pSlider->pressed()) {
		changedDeltaSlider(value - m_nPreSliderValue);
		m_nPreSliderValue = value;
	}
}

void CW3View2D::CreateDirectionText() {
	QFont font = QApplication::font();
	direction_text_ = new CW3TextItem(font, "R", Qt::white);
	direction_text_->setVisible(false);
	direction_text_->setZValue(0.0f);
	scene()->addItem(direction_text_);
}

void CW3View2D::CreateSlider() {
	m_pSlider = new CW3Slider_2DView();
	m_pSlider->setInvertedAppearance(true);
	m_pProxySlider = scene()->addWidget(m_pSlider);
	m_pProxySlider->setZValue(10.0f);
	m_pProxySlider->setVisible(false);
	connect(m_pSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChangedValueSlider(int)));
}

void CW3View2D::transformItems(const QPointF & translate) {
	QTransform transform;
	transform.translate(translate.x(), translate.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}
void CW3View2D::transformItems(const QPointF& preViewCenterInScene,
							   const QPointF& curViewCenterInScene, float scale) {
	QTransform transform;
	transform.translate(curViewCenterInScene.x(), curViewCenterInScene.y());
	transform.scale(scale, scale);
	transform.translate(-preViewCenterInScene.x(), -preViewCenterInScene.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}
void CW3View2D::transformPositionMemberItems(const QTransform& transform) {
	measure_tools_->TransformItems(transform);
}
void CW3View2D::resetFromViewTool() {
	transformItems(m_sceneTrans);
	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_initScale / m_scale);

	setInitScale();

	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;
	m_sceneTrans = QPointF(0.0f, 0.0f);
	m_Wglpre = m_Wgl;
	m_Hglpre = m_Hgl;

	setViewProjection();
}

void CW3View2D::newQOpenGLWidget() {
	if (m_pGLWidget)
		return;

	m_pGLWidget = new QOpenGLWidget(this);

#if !defined(__APPLE__)
	QSurfaceFormat format;
	format.setSamples(4);
	m_pGLWidget->setFormat(format);
#endif

	setViewport(m_pGLWidget);
	m_pRender3DParam->SetCurGLWidget(m_pGLWidget);
}

void CW3View2D::deleteQOpenGLWidget() {
	if (!m_pGLWidget || !m_pGLWidget->context())
		return;

	this->clearGL();
	setViewport(nullptr);
	m_pGLWidget = nullptr;
	m_pRender3DParam->SetCurGLWidget(nullptr);
}

void CW3View2D::setViewProjection() {
	setProjection(); //virtual function
	m_sceneTrans = QPointF(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL);
}

glm::vec4 CW3View2D::getPlaneEquation() {
	if (m_pViewPlane[0]) {
		unsigned int volWidth = m_pgVREngine->getVol(0)->width();
		unsigned int volHeight = m_pgVREngine->getVol(0)->height();
		unsigned int volDepth = m_pgVREngine->getVol(0)->depth();
		vec3 glDist = (m_pViewPlane[0]->getPlaneCenterInVol() - vec3((volWidth - 1)*0.5f, (volHeight - 1)*0.5f, (volDepth - 1)*0.5f))
			* (vec3(2.0f / (volWidth - 1),
					2.0f / (volHeight - 1),
					2.0f / (volDepth - 1)));

		vec3 upVec = glm::normalize(m_pViewPlane[0]->getUpVec());

		return glm::vec4(upVec, glm::dot(glDist, upVec));
	}
	return glm::vec4();
}

glm::vec3 CW3View2D::getPlaneRightVector() {
	return m_pViewPlane[0] ? m_pViewPlane[0]->getRightVec() : glm::vec3();
}

void CW3View2D::keyPressEvent(QKeyEvent *event) {
	QWidget::keyPressEvent(event);
}

void CW3View2D::setInitScale() {
	m_scale = m_initScale;
}

void CW3View2D::ApplyPreferences() {
	show_rulers_ = GlobalPreferences::GetInstance()->preferences_.general.display.show_rulers;
	if (ruler_)
		ruler_->setVisible(show_rulers_);
	if (measure_tools_)
		measure_tools_->ApplyPreferences();

	if (grid_) {
		int index = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing;
		int spacing = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing_preset[index];
		grid_->SetGridSpacing(spacing);
	}
}

void CW3View2D::DeleteMeasureUI(const unsigned int & measure_id) {
	measure_tools_->SyncDeleteMeasureUI(measure_id);
}
