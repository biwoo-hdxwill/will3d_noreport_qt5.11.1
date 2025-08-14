#include "thyoo_W3View2D.h"

/*=========================================================================

File:			class CW3View2D
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-12-11

=========================================================================*/
#include <qgraphicsscene.h>
#include <QSurfaceFormat>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <qopenglwidget.h>

#include <Engine/Common/Common/color_will3d.h>
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/language_pack.h"

#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/ResContainer/resource_container.h"
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/grid_lines.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/VREngine/W3VREngine.h"

CW3View2D_thyoo::CW3View2D_thyoo(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
					 common::ViewTypeID eType, bool is2Dused, QWidget *pParent) :
	m_eViewType(eType), m_pgVREngine(VREngine), m_pgMPRengine(MPRengine),
	m_is2Dused(is2Dused), QGraphicsView(pParent) {
	this->setFrameShape(QFrame::NoFrame);

	m_pGLWidget = new QOpenGLWidget(this);
	setViewport(m_pGLWidget);

#if !defined(__APPLE__)
	QSurfaceFormat format;
	format.setSamples(4);
	m_pGLWidget->setFormat(format);
#endif

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setRenderHint(QPainter::Antialiasing, true);

	QGraphicsScene* main_scene = new QGraphicsScene(this);
	this->setScene(main_scene);
	this->scene()->setSceneRect(-20000, -20000, 40000, 40000);

	m_pntCurViewCenterinScene = QPointF(0.0f, 0.0f);

	this->setMouseTracking(true);

	if (m_is2Dused) {
		m_pViewPlane = new CW3ViewPlane();

		m_model = glm::mat4(1.0f);
		m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
							 glm::vec3(0.0f, 0.0f, 0.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		m_projection = glm::mat4(1.0f);

		m_scale = 1.0f;
	}
	
	measure_tools_ = new MeasureTools(m_eViewType, this->scene());

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() + 3);
	font.setWeight(QFont::Bold);

	grid_ = new GridLines();
	this->scene()->addItem(grid_);

	if (m_eViewType == common::ViewTypeID::CEPH || m_eViewType == common::ViewTypeID::FACE_AFTER) {
		ruler_ = new CW3ViewRuler(ColorView::k3D);
		ruler_->setVisible(true);
		this->scene()->addItem(ruler_);
	}

	if (m_eViewType == common::ViewTypeID::CEPH ||
		m_eViewType == common::ViewTypeID::FACE_AFTER) {
		m_pWorldAxisItem = new ViewNavigatorItem();
		scene()->addItem(m_pWorldAxisItem);
		m_pWorldAxisItem->setVisible(false);
	}

	ApplyPreferences();
}

CW3View2D_thyoo::~CW3View2D_thyoo() {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(measure_tools_);
	SAFE_DELETE_OBJECT(m_pViewPlane);
	SAFE_DELETE_OBJECT(m_pGLWidget);
}
#ifndef WILL3D_VIEWER
void CW3View2D_thyoo::exportProject(ProjectIOView & out) {
	out.SaveViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans);
}

void CW3View2D_thyoo::importProject(ProjectIOView & in) {
	in.LoadViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans);
}
#endif
void CW3View2D_thyoo::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) {
	switch (type) {
	case common::CommonToolTypeOnce::V_RESET:
		ResetView();
		break;
	case common::CommonToolTypeOnce::V_FIT:
		FitView();
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
		DeleteUnfinishedMeasure();
		break;
	default:
		break;
	}
}
void CW3View2D_thyoo::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	common_tool_type_ = type;

	if (!isVisible())
		return;

	CW3Cursor::SetViewCursor(common_tool_type_);
	measure_tools_->ClearUnfinishedItem();
	measure_tools_->SetMeasureType(common_tool_type_);
}

void CW3View2D_thyoo::reset() {
	m_is2Dready = false;
	m_isIntendedResize = false;
	m_scaleSceneToGL = 1.0f;
	m_Wgl = 1.0f;
	m_Hgl = 1.0f;
	m_Dgl = 1.0f;
	m_Wglorig = 1.0f;
	m_Hglorig = 1.0f;
	m_Wglpre = 1.0f;
	m_Hglpre = 1.0f;
	m_Dglpre = 1.0f;
	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;

	m_texNumPlane = GL_TEXTURE0;
	m_texNumPlane_ = 0;
	m_texHandlerPlane = 0;

	if (m_is2Dused) {
		m_model = glm::mat4(1.0f);
		m_view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
							 glm::vec3(0.0f, 0.0f, 0.0f),
							 glm::vec3(0.0f, 1.0f, 0.0f));
		m_projection = glm::mat4(1.0f);

		m_scale = 1.0f;
	}

	common_tool_type_ = common::CommonToolTypeOnOff::NONE;

	m_scalePre = 1.0f;

	hide_all_view_ui_ = false;

	// serialization
	m_bLoadProject = false;

	ApplyPreferences();
}

void CW3View2D_thyoo::resizeEvent(QResizeEvent *pEvent) {
	if (!isVisible())
		return;

	QGraphicsView::resizeEvent(pEvent);

	fitInView(0, 0, this->width(), this->height());

	if (!m_isIntendedResize) {
		QPointF leftTop = mapToScene(QPoint(0, 0));
		QPointF rightBottom = mapToScene(QPoint(this->width(), this->height()));
		m_sceneWinView = (rightBottom.x() - leftTop.x())*0.5f;
		m_sceneHinView = (rightBottom.y() - leftTop.y())*0.5f;

		rightBottom = mapToScene(QPoint(this->width() - 1, this->height() - 1));

		m_pntCurViewCenterinScene.setX((rightBottom.x() + leftTop.x())*0.5f);
		m_pntCurViewCenterinScene.setY((rightBottom.y() + leftTop.y())*0.5f);

		m_isIntendedResize = true;

		if (measure_tools_)
			measure_tools_->SetCenter(m_pntCurViewCenterinScene);
	} else
		resizeScene();

	if (m_pWorldAxisItem) {
		m_pWorldAxisItem->SetSize(100, 100);
		m_pWorldAxisItem->setPos(mapToScene(this->width() - 60, this->height() - 60));
	}

	setViewRulerValue();
	SetGridValue();

	scene()->update();
}

void CW3View2D_thyoo::wheelEvent(QWheelEvent *event) {
	event->ignore();
}

void CW3View2D_thyoo::drawBackground(QPainter *painter, const QRectF &rect) {
}

void CW3View2D_thyoo::setProjection() {
	float width = this->width();
	float height = this->height();

	float viewRatio = width / height;
	float imgRatio = m_Wglpre / m_Hglpre;

	if (viewRatio < imgRatio) {
		m_projection = glm::ortho(
			-m_Wglpre / m_scale + m_WglTrans,
			m_Wglpre / m_scale + m_WglTrans,
			-m_Wglpre / (viewRatio*m_scale) - m_HglTrans,
			m_Wglpre / (viewRatio*m_scale) - m_HglTrans, 0.0f, 2.0f);

		m_scaleSceneToGL = m_Wglpre / (m_sceneWinView*m_scale);
	} else {
		m_projection = glm::ortho(
			-m_Hglpre*viewRatio / m_scale + m_WglTrans,
			m_Hglpre*viewRatio / m_scale + m_WglTrans,
			-m_Hglpre / m_scale - m_HglTrans,
			m_Hglpre / m_scale - m_HglTrans,
			0.0f, 2.0f);

		m_scaleSceneToGL = m_Hglpre / (m_sceneHinView*m_scale);
	}

	measure_tools_->SetScale(m_scaleSceneToGL / 2.0f);
	measure_tools_->UpdateProjection();
}

void CW3View2D_thyoo::setVisible(bool state) {
	if (measure_tools_)
	{
		measure_tools_->SetVisibleFromView(state);
	}

	if (!state && m_pGLWidget && m_pGLWidget->context()) {
		clearGL();
		setViewport(nullptr);
		m_pGLWidget = nullptr;
	} else {
		if (!m_pGLWidget) {
			m_pGLWidget = new QOpenGLWidget(this);

#if !defined(__APPLE__)
			QSurfaceFormat format;
			format.setSamples(4);
			m_pGLWidget->setFormat(format);
#endif

			setViewport(m_pGLWidget);
		}
	}

	QGraphicsView::setVisible(state);
}

void CW3View2D_thyoo::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		{
			if (m_texHandlerPlane) {
				glDeleteTextures(1, &m_texHandlerPlane);
				m_texHandlerPlane = 0;
			}
		}
		m_pGLWidget->doneCurrent();
	}
}

void CW3View2D_thyoo::resizeScene() {
	if (!isVisible())
		return;

	float preScale = m_scaleSceneToGL;

	QPointF	preViewCenterInScene = m_pntCurViewCenterinScene;

	QPointF leftTop = mapToScene(QPoint(0, 0));
	QPointF rightBottom = mapToScene(QPoint(this->width(), this->height()));
	m_sceneWinView = (rightBottom.x() - leftTop.x())*0.5f;
	m_sceneHinView = (rightBottom.y() - leftTop.y())*0.5f;

	rightBottom = mapToScene(QPoint(this->width() - 1, this->height() - 1));

	m_pntCurViewCenterinScene.setX((rightBottom.x() + leftTop.x())*0.5f);
	m_pntCurViewCenterinScene.setY((rightBottom.y() + leftTop.y())*0.5f);

	setViewProjection();

	float scale = preScale / m_scaleSceneToGL;

	transformItems(preViewCenterInScene, m_pntCurViewCenterinScene, scale);

	if (measure_tools_)
		measure_tools_->SetCenter(m_pntCurViewCenterinScene);
}

void CW3View2D_thyoo::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);

	if (event->buttons() & Qt::LeftButton) {
		switch (common_tool_type_) {
		case common::CommonToolTypeOnOff::V_PAN:
		case common::CommonToolTypeOnOff::V_PAN_LR:
			curr_scene_pos_ = mapToScene(event->pos());
			mousePanningEvent();

			QApplication::setOverrideCursor(CW3Cursor::OpenHandCursor());

			last_scene_pos_ = curr_scene_pos_;
			scene()->update();
			break;
		case common::CommonToolTypeOnOff::V_ZOOM:
		case common::CommonToolTypeOnOff::V_ZOOM_R:

			curr_view_pos_ = event->pos();

			mouseScaleEvent();

			if (m_scale / m_scalePre > 1.0f)
				QApplication::setOverrideCursor(CW3Cursor::ZoomInCursor());
			else
				QApplication::setOverrideCursor(CW3Cursor::ZoomOutCursor());

			last_view_pos_ = curr_view_pos_;
			scene()->update();
			break;
		default:
			break;
		}
	} else {
		QGraphicsView::mouseMoveEvent(event);
	}
}

void CW3View2D_thyoo::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);

	last_scene_pos_ = curr_scene_pos_ = mapToScene(event->pos());
	last_view_pos_ = curr_view_pos_ = event->pos();

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void CW3View2D_thyoo::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	curr_scene_pos_ = mapToScene(event->pos());
	curr_view_pos_ = event->pos();

	if (common_tool_type_ != common::CommonToolTypeOnOff::NONE) {
		CW3Cursor::SetViewCursor(common_tool_type_);
	}

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
	//event->ignore();
}

void CW3View2D_thyoo::mouseDoubleClickEvent(QMouseEvent *event) {
	QGraphicsView::mouseDoubleClickEvent(event);

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(),
				   event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void CW3View2D_thyoo::mousePanningEvent() {
	float transX = last_scene_pos_.x() - curr_scene_pos_.x();
	float transY = last_scene_pos_.y() - curr_scene_pos_.y();

	m_WglTrans += transX * m_scaleSceneToGL;
	m_HglTrans += transY * m_scaleSceneToGL;

	m_sceneTrans = QPointF(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL);

	setViewProjection();

	transformItems(QPointF(-transX, -transY));
}

void CW3View2D_thyoo::mouseScaleEvent() {
	m_scalePre = m_scale;
	m_scale = m_scale + float(last_view_pos_.y() - curr_view_pos_.y()) / float(this->height());

	if (m_scale < 0.3f) {
		m_scale = 0.3f;
	}

	setViewProjection();
	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, m_scale / m_scalePre);
}

void CW3View2D_thyoo::HideText(bool bToggled) {
	
}

void CW3View2D_thyoo::HideUI(bool bToggled) {
	hide_all_view_ui_ = bToggled;

	hide_all_view_ui_ = bToggled;
	SetVisibleItems();
}
void CW3View2D_thyoo::HideMeasure(bool bToggled) {
	measure_tools_->SetVisibleFromToolbar(!bToggled);
}

void CW3View2D_thyoo::DeleteAllMeasure() {
	measure_tools_->DeleteAllMeasure();
}

void CW3View2D_thyoo::DeleteUnfinishedMeasure() {
	measure_tools_->ClearUnfinishedItem();
}

void CW3View2D_thyoo::setViewRulerValue() {
	if (ruler_) {
		int width = this->width();
		int height = this->height();

		QPointF viewSizeInScene = mapToScene(width, height) - mapToScene(0.0f, 0.0f);

		float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
		QPointF viewLength = viewSizeInScene * pixelSpacing * m_scaleSceneToGL / 2.0f;
		QPointF leftTop = mapToScene(QPoint(0, 0));
		QPointF offset = mapToScene(QPoint(0, 2));
		QPointF sceneGLOffset = offset - leftTop;
		sceneGLOffset.setY(sceneGLOffset.y() - 1.0f);

		ruler_->setViewRuler(viewSizeInScene.x(), viewSizeInScene.y(),
								   viewLength.x(), viewLength.y(),
								   m_pntCurViewCenterinScene - sceneGLOffset);
	}
}

void CW3View2D_thyoo::SetGridValue() {
	QPointF viewSizeInScene = mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f);

	float pixelSpacing = ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	QPointF viewLength = viewSizeInScene * pixelSpacing * m_scaleSceneToGL / 2.0f;
	QPointF leftTop = mapToScene(QPoint(0, 0));
	QPointF offset = mapToScene(QPoint(0, 2));
	QPointF sceneGLOffset = offset - leftTop;
	sceneGLOffset.setY(sceneGLOffset.y() - 1.0f);

	int index = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing;
	int spacing = GlobalPreferences::GetInstance()->preferences_.general.display.grid_spacing_preset[index];
	grid_->SetGrid(viewSizeInScene.x(), viewSizeInScene.y(),
				   viewLength.x(), viewLength.y(),
				   m_pntCurViewCenterinScene - sceneGLOffset, static_cast<float>(spacing));
	grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
}

void CW3View2D_thyoo::SetVisibleItems() {
	bool isNotVR = m_eReconType == common::ReconTypeID::MPR || m_eReconType == common::ReconTypeID::X_RAY;
	ruler_->setVisible(!hide_all_view_ui_ && show_rulers_);
	grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
	m_pWorldAxisItem->setVisible(!hide_all_view_ui_);
}

void CW3View2D_thyoo::GridOnOff(bool grid_on) {
	grid_visible_ = grid_on;
	grid_->setVisible(!hide_all_view_ui_ & grid_visible_);
}

void CW3View2D_thyoo::ResetView() {
	if (!isVisible())
		return;

	resetFromViewTool();

	scene()->update();
}

void CW3View2D_thyoo::FitView() {
	if (!isVisible())
		return;

	resetFromViewTool();

	scene()->update();
}

void CW3View2D_thyoo::resetFromViewTool() {
	transformItems(m_sceneTrans);
	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene, 1.0f / m_scale);

	m_scale = 1.0f;
	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;
	m_sceneTrans = QPointF(0.0f, 0.0f);
	m_Wglpre = m_Wgl;
	m_Hglpre = m_Hgl;

	setViewProjection();
}

void CW3View2D_thyoo::setViewProjection() {
	setProjection();
	m_sceneTrans = QPointF(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL);
}

void CW3View2D_thyoo::transformItems(const QPointF & translate) {
	QTransform transform;
	transform.translate(translate.x(), translate.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}

void CW3View2D_thyoo::transformItems(const QPointF& preViewCenterInScene, const QPointF& curViewCenterInScene, float scale) {
	QTransform transform;
	transform.translate(curViewCenterInScene.x(), curViewCenterInScene.y());
	transform.scale(scale, scale);
	transform.translate(-preViewCenterInScene.x(), -preViewCenterInScene.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}
void CW3View2D_thyoo::transformPositionMemberItems(const QTransform& transform) {
	measure_tools_->TransformItems(transform);
}

void CW3View2D_thyoo::ApplyPreferences() {
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

void CW3View2D_thyoo::DeleteMeasureUI(const unsigned int & measure_id) {
	measure_tools_->SyncDeleteMeasureUI(measure_id);
}

void CW3View2D_thyoo::keyPressEvent(QKeyEvent* event)
{
	QWidget::keyPressEvent(event);
}

void CW3View2D_thyoo::leaveEvent(QEvent* event)
{
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
}

void CW3View2D_thyoo::enterEvent(QEvent* event)
{
	QGraphicsView::enterEvent(event);
	this->setFocus();
	CW3Cursor::SetViewCursor(common_tool_type_);
}
