#include "W3View2D_forMPR.h"

/*=========================================================================

File:			class CW3View2D_forMPR
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-21
Last date:		2016-06-04

=========================================================================*/

#include <qgraphicsscene.h>
#include <qopenglwidget.h>
#include <QApplication>
#include <QGraphicsProxyWidget>
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>
#include <QElapsedTimer>

#include <Engine/Common/Common/color_will3d.h>
#include <Engine/Common/Common/event_handler.h>
#include "../../Common/Common/W3Cursor.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#include "../../Resource/Resource/implant_resource.h"
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include "../UIGLObjects/W3GLNerve.h"
#include "../UIGLObjects/W3GLObject.h"
#include <Engine/UIModule/UIPrimitive/image_filter_selector.h>
#include "../UIPrimitive/W3Slider_2DView.h"
#include "../UIPrimitive/W3TextItem_ImplantID.h"
#include "../UIPrimitive/W3ViewRuler.h"
#include "../UIPrimitive/grid_lines.h"
#include "../UIPrimitive/measure_tools.h"
#include "../UIPrimitive/simple_text_item.h"
#include "../UIViewController/view_navigator_item.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3VRengineTextures.h"

#define ENABLE_ELAPSED_TIMER 0

using common::Logger;
using common::LogType;

namespace
{
	const float kInitScale2D = 1.0f;
	const glm::vec3 kTempPos = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::vec3 kDefaultViewEye = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 kDefaultViewCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 kDefaultViewUp = glm::vec3(0.0f, 1.0f, 0.0f);
}  // namespace

CW3View2D_forMPR::CW3View2D_forMPR(CW3VREngine* VREngine,
	CW3MPREngine* MPRengine,
	CW3ResourceContainer* Rcontainer,
	common::ViewTypeID eType, QWidget* pParent)
	: m_eViewType(eType),
	m_pgVREngine(VREngine),
	m_pgMPRengine(MPRengine),
	m_pgRcontainer(Rcontainer),
	implant_id_text_(new CW3TextItem_ImplantID),
	HU_value_(new SimpleTextItem),
	QGraphicsView(pParent)
{
	this->setScene(new QGraphicsScene(this));
	QGraphicsScene* scene = this->scene();
	scene->setSceneRect(-4000, -4000, 8000, 8000);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setRenderHint(QPainter::Antialiasing, true);
	this->horizontalScrollBar()->installEventFilter(this);
	this->setMouseTracking(true);

	m_pRender3DParam =
		new CW3Render3DParam(m_pGLWidget, m_pgRcontainer->getCollisionContainer(),
			m_pgRcontainer->getImplantThereContainer());
	m_fThickness[0] = 0.0f;

	initVectorsUpBack();
	setInitScale();

	m_view = glm::lookAt(kDefaultViewEye, kDefaultViewCenter, kDefaultViewUp);

	measure_tools_ = new MeasureTools(m_eViewType, scene);

	ruler_ = new CW3ViewRuler(GetViewColor(), nullptr);
	ruler_->setVisible(false);
	scene->addItem(ruler_);

	grid_ = new GridLines();
	scene->addItem(grid_);

	implant_id_text_->addItems(scene);
	implant_id_text_->setVisible(false);

	scene->addItem(HU_value_);
	HU_value_->setVisible(false);

	QFont font = QApplication::font();
	font.setPixelSize(font.pixelSize() - 1);
	sharpen_filter_text_ = new ImageFilterSelector();
	sharpen_filter_text_->SetFont(font);
	sharpen_filter_text_->setVisible(false);
	scene->addItem(sharpen_filter_text_);

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	sharpen_level_ = settings.value("SLICE/default_filter", 0).toInt();
	sharpen_filter_text_->SetLevel(sharpen_level_);

	connections();

	this->setStyleSheet(CW3Theme::getInstance()->appQMenuStyleSheet());

	ApplyPreferences();
}

CW3View2D_forMPR::~CW3View2D_forMPR()
{
	if (m_pGLWidget && m_pGLWidget->context()) this->clearGL();

	SAFE_DELETE_OBJECT(measure_tools_);
	SAFE_DELETE_OBJECT(m_pViewPlane[0]);
	SAFE_DELETE_OBJECT(m_pViewPlane[1]);
	SAFE_DELETE_OBJECT(m_pRender3DParam);
	SAFE_DELETE_OBJECT(m_pGLWidget);
}
#ifndef WILL3D_VIEWER
void CW3View2D_forMPR::exportProject(ProjectIOView& out)
{
#if 1
	if (m_pViewPlane[0])
	{
		out.SaveViewPlane(m_pViewPlane[0]);
	}
#endif
	out.SaveViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans, m_nAdjustWindowLevel, m_nAdjustWindowWidth);
}

void CW3View2D_forMPR::importProject(ProjectIOView& in)
{
	in.LoadViewInfo(m_scale, m_scaleSceneToGL, m_WglTrans, m_HglTrans, m_nAdjustWindowLevel, m_nAdjustWindowWidth);
	import_proj_info_.view_scale = m_scale;
	import_proj_info_.gl_trans_x = m_WglTrans;
	import_proj_info_.gl_trans_y = m_HglTrans;
	import_proj_info_.is_import = true;

#if 1
	if (m_pViewPlane[0])
	{
		in.LoadViewPlane(m_pViewPlane[0]);
		drawImageOnViewPlane(false, 0, false);
	}
#endif

	setViewProjection();
	scene()->update();

	measure_tools_->ImportMeasureResource();
}
#endif
void CW3View2D_forMPR::reset()
{
	m_is2Dready = false;
	m_nAdjustWindowLevel = 0;
	m_nAdjustWindowWidth = 0;

	m_drawVolId = 0;
	m_isDrawBoth = 0;

	SAFE_DELETE_OBJECT(m_pRender3DParam);
	m_pRender3DParam =
		new CW3Render3DParam(m_pGLWidget, m_pgRcontainer->getCollisionContainer(),
			m_pgRcontainer->getImplantThereContainer());
	m_isUpdateSurface = false;
	m_isReconSwitched = false;

	last_view_pos_.setX(0);
	last_view_pos_.setY(0);

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

	initVectorsUpBack();

	m_fThickness[0] = 0.0f;

	setInitScale();

	m_model = glm::mat4(1.0f);
	m_modelSecond = glm::mat4(1.0f);
	m_view = glm::lookAt(kDefaultViewEye, kDefaultViewCenter, kDefaultViewUp);
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
	slider_value_set_ = true;

	if (m_pSlider) m_pSlider->setValue(0);

	ApplyPreferences();
}

void CW3View2D_forMPR::connections()
{
	connect(sharpen_filter_text_, SIGNAL(sigPressed(int)), this, SLOT(slotSelectFilter(int)));

	connect(measure_tools_, &MeasureTools::sigGetProfileData, this,
		&CW3View2D_forMPR::slotSetProfileData);
	connect(measure_tools_, &MeasureTools::sigGetROIData, this,
		&CW3View2D_forMPR::slotSetROIData);
}

void CW3View2D_forMPR::disconnections()
{
	disconnect(sharpen_filter_text_, SIGNAL(sigPressed(int)), this, SLOT(slotSelectFilter(int)));

	disconnect(measure_tools_, &MeasureTools::sigGetProfileData, this,
		&CW3View2D_forMPR::slotSetProfileData);
	disconnect(measure_tools_, &MeasureTools::sigGetROIData, this,
		&CW3View2D_forMPR::slotSetROIData);
}

QColor CW3View2D_forMPR::GetViewColor() { return ColorView::k3D; }

void CW3View2D_forMPR::ToolTypeInteractions2DInMove(
	const QPoint& curr_view_pos)
{
	if (measure_tools_->IsSelected() &&
		measure_tools_->IsMeasureInteractionAvailable(common_tool_type_))
	{
		measure_tools_->ProcessMouseMove(mapToScene(curr_view_pos), kTempPos);
		return;
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_LIGHT)
	{
		WindowingView(mapToScene(curr_view_pos));
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_PAN ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		PanningView(mapToScene(curr_view_pos));
	}
	else if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R)
	{
		ScaleView(curr_view_pos);
	}
	scene()->update();
}

void CW3View2D_forMPR::GridOnOff(bool grid_on)
{
	grid_visible_ = grid_on;
	grid_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_ & grid_visible_);
}

void CW3View2D_forMPR::SetVisibleItems()
{
	SetVisibleItems(!single_view_hide_ui_ && !hide_all_view_ui_);
}

void CW3View2D_forMPR::SetVisibleItems(const bool visible)
{
	bool is_text_visible = !(!visible);
	bool isNotVR = recon_type_ == common::ReconTypeID::MPR ||
		recon_type_ == common::ReconTypeID::X_RAY;
	sharpen_filter_text_->setVisible(is_text_visible && isNotVR);
	ruler_->setVisible(visible && show_rulers_);
	grid_->setVisible(visible & grid_visible_);
	m_pWorldAxisItem->setVisible(visible);

	if (m_pProxySlider)
	{
		m_pProxySlider->setVisible(visible && isNotVR);
	}

	HU_value_->setVisible(visible);
}

void CW3View2D_forMPR::SetCommonToolOnce(const common::CommonToolTypeOnce& type,
	bool on)
{
	switch (type)
	{
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
		DeleteUnfinishedMeasure();
		break;
	default:
		break;
	}
}

void CW3View2D_forMPR::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type)
{
	common_tool_type_ = type;

	if (!isVisible()) return;

	CW3Cursor::SetViewCursor(common_tool_type_);
	if (!is_right_button_clicked_)
	{
		measure_tools_->ClearUnfinishedItem();
	}
	measure_tools_->SetMeasureType(common_tool_type_);
}

void CW3View2D_forMPR::setWLWW()
{
	auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (!&vol)
	{
		return;
	}

	m_nAdjustWindowLevel = vol.windowCenter();
	m_nAdjustWindowWidth = vol.windowWidth();
	changeWLWW();
}

void CW3View2D_forMPR::changeWLWW()
{
	int windowWidth =
		invert_windowing_ ? -m_nAdjustWindowWidth : m_nAdjustWindowWidth;
	m_pRender3DParam->set_windowing_min(
		(m_nAdjustWindowLevel - windowWidth * 0.5f) / 65535.0f);
	m_pRender3DParam->set_windowing_norm(windowWidth / 65535.0f);
}

void CW3View2D_forMPR::updateMask()
{
	if (m_pViewPlane[0])
	{
		m_pgMPRengine->reconNerveMask(
			m_pViewPlane[0], ResourceContainer::GetInstance()->GetNerveResource());

		m_pgVREngine->makeCurrent();
		CW3GLFunctions::update2DTex(m_pRender3DParam->m_VRtextures.m_texHandler[1],
			m_pViewPlane[0]->getWidth(),
			m_pViewPlane[0]->getHeight(),
			m_pViewPlane[0]->getMask(), false);
		m_pgVREngine->doneCurrent();
	}
}

bool CW3View2D_forMPR::reconMPR(int id, bool isCanalShown)
{
#if ENABLE_ELAPSED_TIMER
	QElapsedTimer timer;
	timer.start();
#endif

	bool res = false;
	if (isCanalShown)
		res = m_pgMPRengine->reconMPRimage(
			m_pViewPlane[id], id, m_fThickness[id],
			ResourceContainer::GetInstance()->GetNerveResource());
	else
		res = m_pgMPRengine->reconMPRimage(m_pViewPlane[id], id, m_fThickness[id]);

#if ENABLE_ELAPSED_TIMER
	qDebug() << "reconMPR :" << timer.elapsed();
#endif

	return res;
}

void CW3View2D_forMPR::drawImageOnViewPlane(bool isFitIn, int id, 
	bool isCanalShown)
{
#if ENABLE_ELAPSED_TIMER
	QElapsedTimer timer;
	timer.start();
#endif

	if (!m_pViewPlane[id])
	{
		m_pViewPlane[id] = new CW3ViewPlane();
	}

	bool isDifferentSize = reconMPR(id, isCanalShown);

	m_pgMPRengine->shapenPlane(m_pViewPlane[id], sharpen_level_);

	float main_vol_pixel_size =
		ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	if (id == 0)
	{
		glm::vec3* vol_range = m_pgVREngine->getVolRange(0);
		float max_axis =
			std::max(vol_range->x, std::max(vol_range->y, vol_range->z));
		if (isFitIn)
		{
			if (!import_proj_info_.is_import)
			{
				setInitScale();
				m_WglTrans = 0.0f;
				m_HglTrans = 0.0f;
			}

			m_Wglpre = m_Hglpre = max_axis;
		}

		m_Wgl = m_Hgl = max_axis;

		m_model = glm::scale(glm::vec3(m_pViewPlane[0]->getWidth(),
			m_pViewPlane[0]->getHeight(), m_Dgl));

		setViewProjection();
		drawImageOnTexture(m_pViewPlane[id], isDifferentSize);
	}
	else
	{
		float second_vol_pixel_size =
			ResourceContainer::GetInstance()->GetSecondVolume().GetBasePixelSize();
		float scaleRatio = second_vol_pixel_size / main_vol_pixel_size;
		m_modelSecond = glm::scale(
			glm::vec3((m_pViewPlane[1]->getWidth()) * scaleRatio,
			(m_pViewPlane[1]->getHeight()) * scaleRatio, m_Dgl));
	}

	m_pViewPlane[0]->getImage2D()->setPixelSpacing(main_vol_pixel_size);
	measure_tools_->SetPixelSpacing(main_vol_pixel_size);
	measure_tools_->Update(m_pViewPlane[0]->getPlaneCenterInVol(),
		m_pViewPlane[0]->getUpVec(),
		m_pViewPlane[0]->getBackVec());

#if ENABLE_ELAPSED_TIMER
	qDebug() << "drawImageOnViewPlane :" << timer.elapsed();
#endif
}

void CW3View2D_forMPR::drawImageOnTexture(CW3ViewPlane* viewPlane,
	bool isDifferentSize)
{
	if (!isVisible()) return;

#if ENABLE_ELAPSED_TIMER
	QElapsedTimer timer;
	timer.start();
#endif
	m_pgVREngine->makeCurrent();

	CW3GLFunctions::update2DTex(m_pRender3DParam->m_VRtextures.m_texHandler[0],
		viewPlane->getWidth(), viewPlane->getHeight(),
		viewPlane->getImage2D()->getData(),
		isDifferentSize);

	if (m_pRender3DParam->m_pNerve->isVisible())
	{
		CW3GLFunctions::update2DTex(m_pRender3DParam->m_VRtextures.m_texHandler[1],
			viewPlane->getWidth(), viewPlane->getHeight(),
			viewPlane->getMask(), isDifferentSize);
	}

	m_pgVREngine->doneCurrent();

	if (import_proj_info_.is_import && viewPlane->getImage2D())
	{
		m_scale = import_proj_info_.view_scale;
		m_WglTrans = import_proj_info_.gl_trans_x;
		m_HglTrans = import_proj_info_.gl_trans_y;
		measure_tools_->Initialize(
			ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize(),
			m_scaleSceneToGL);
		import_proj_info_.is_import = false;
	}

	if (!m_is2Dready)
	{
		m_Wglorig = m_Wglpre;
		m_Hglorig = m_Hglpre;
		m_Dglorig = m_Dglpre;
	}

	m_is2Dready = true;

	ruler_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_ && show_rulers_);

#if ENABLE_ELAPSED_TIMER
	qDebug() << "drawImageOnTexture :" << timer.elapsed();
#endif
}

void CW3View2D_forMPR::CreateSlider()
{
	m_pSlider = new CW3Slider_2DView();
	m_pSlider->setInvertedAppearance(true);
	m_pProxySlider = scene()->addWidget(m_pSlider);
	m_pProxySlider->setZValue(30.0f);
	m_pProxySlider->setVisible(false);
	connect(m_pSlider, SIGNAL(valueChanged(int)), this,
		SLOT(slotChangedValueSlider(int)));
}

void CW3View2D_forMPR::drawBackground(QPainter* painter, const QRectF& rect)
{
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();
	if (m_is2Dready && isVisible())
	{
		if (m_isUpdateSurface)
		{
			m_pGLWidget->doneCurrent();
			updateMask();
			m_isUpdateSurface = false;
			m_pGLWidget->makeCurrent();
		}

		if (!m_pRender3DParam->m_plane->getVAO())
		{
			unsigned int vao = 0;
			m_pRender3DParam->m_plane->clearVAOVBO();

			m_pgVREngine->initVAOplane(&vao);
			m_pRender3DParam->m_plane->setVAO(vao);
			m_pRender3DParam->m_plane->setNindices(6);
		}

		m_pRender3DParam->m_plane->setMVP(m_model, m_view, m_projection);
		m_pRender3DParam->m_width = this->width();
		m_pRender3DParam->m_height = this->height();

		if (m_drawVolId == 0 || m_isDrawBoth)
			m_pgVREngine->RenderMPR(m_pRender3DParam, 0, m_isDrawBoth,
				m_isReconSwitched);

		if (m_pRender3DParam->m_isImplantShown)
		{
			for (int i = 0; i < MAX_IMPLANT; i++)
			{
				m_pRender3DParam->g_is_implant_exist_[i] = false;

				const auto& res_implant =
					ResourceContainer::GetInstance()->GetImplantResource();
				const auto& datas = res_implant.data();
				auto data = datas.find(kImplantNumbers[i]);
				if (data == datas.end()) continue;

				if (!m_pRender3DParam->m_pImplant[i]->getVAO())
				{
					setVAOVBOImplant(i);
				}

				setImplantViewProjection();

#if 0
				float left = -m_Wglpre / m_scale + m_WglTrans;
				float right = m_Wglpre / m_scale + m_WglTrans;
				float bottom = -m_Wglpre / m_scale - m_HglTrans;
				float top = m_Wglpre / m_scale - m_HglTrans;
				m_projectionImplant = glm::ortho(left, right, bottom, top, 0.0f, 0.001f);
#endif

				glm::mat4 model =
					data->second->translate_in_vol() * data->second->rotate_in_vol();

				m_pRender3DParam->m_pImplant[i]->setMVP(model, m_viewImplant,
					m_projectionImplant);
				m_pRender3DParam->g_is_implant_exist_[i] = true;
				m_pRender3DParam->g_is_implant_collided_[i] = data->second->is_collide();
			}
		}

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
			}

			setImplantViewProjection();
			m_pRender3DParam->m_photo3D->setMVP(m_modelPhoto, m_viewImplant,
				m_projectionImplant);
		}

		if (m_pRender3DParam->m_isImplantShown ||
			m_pRender3DParam->m_photo3D->isVisible())
		{
#if 0
			if (m_eViewType == common::ViewTypeID::MPR_AXIAL)
			{
				m_pgVREngine->RenderMeshMPR(m_pRender3DParam, 0, m_isReconSwitched);
	}
#else
			m_pgVREngine->RenderMeshMPR(m_pRender3DParam, 0, m_isReconSwitched);
#endif
}

		if (m_drawVolId == 1 || m_isDrawBoth)
		{
			m_pGLWidget->doneCurrent();
			m_pgVREngine->makeCurrent();

			CW3GLFunctions::update2DTex(
				m_pRender3DParam->m_VRtextures.m_texHandler[2],
				m_pViewPlane[1]->getWidth(), m_pViewPlane[1]->getHeight(),
				m_pViewPlane[1]->getImage2D()->getData(), true);

			m_pgVREngine->doneCurrent();
			m_pGLWidget->makeCurrent();
			m_mvpSecond = m_projection * m_view * m_modelSecond;
			m_pRender3DParam->m_plane->setMVP(m_modelSecond, m_view, m_projection);

			m_pgVREngine->RenderMPR(m_pRender3DParam, 2, m_isDrawBoth,
				m_isReconSwitched);
		}
  }
	else
	{
		CW3GLFunctions::clearView(false);
		ruler_->setVisible(false);
	}

	painter->endNativePainting();
}

void CW3View2D_forMPR::setImplantViewProjection()
{
	glm::vec3 eye = m_pViewPlane[0]->getPlaneCenterInVol() -
		*m_pgMPRengine->getMPRrotCenterOrigInVol(0);
	eye.x *= -1.0f;
	eye *= m_scaleVolToGL;

	glm::vec3 dir = m_pViewPlane[0]->getUpVec();
	dir.x *= -1.0f;

	glm::vec3 up = m_pViewPlane[0]->getBackVec();
	up.y *= -1.0f;
	up.z *= -1.0f;

	m_viewImplant = glm::lookAt(eye, eye + dir, up);

	float viewRatio = m_sceneWinView / m_sceneHinView;
	float imgRatio = m_Wglpre / m_Hglpre;
	float left, right, top, bottom;
	if (viewRatio < imgRatio)
	{
		left = -m_Wglpre / m_scale + m_WglTrans;
		right = m_Wglpre / m_scale + m_WglTrans;
		bottom = -m_Wglpre / (viewRatio * m_scale) - m_HglTrans;
		top = m_Wglpre / (viewRatio * m_scale) - m_HglTrans;
	}
	else
	{
		left = -m_Hglpre * viewRatio / m_scale + m_WglTrans;
		right = m_Hglpre * viewRatio / m_scale + m_WglTrans;
		bottom = -m_Hglpre / m_scale - m_HglTrans;
		top = m_Hglpre / m_scale - m_HglTrans;
	}
	m_projectionImplant = glm::ortho(left, right, bottom, top, 0.0f, 0.001f);
}

void CW3View2D_forMPR::wheelEvent(QWheelEvent* event) { event->ignore(); }

void CW3View2D_forMPR::setProjection()
{
	const float viewRatio = m_sceneWinView / m_sceneHinView;
	const float imgRatio = m_Wglpre / m_Hglpre;

	float left, right, top, bottom;
	if (viewRatio < imgRatio)
	{
		left = -m_Wglpre / m_scale + m_WglTrans;
		right = m_Wglpre / m_scale + m_WglTrans;
		bottom = -m_Wglpre / (viewRatio * m_scale) - m_HglTrans;
		top = m_Wglpre / (viewRatio * m_scale) - m_HglTrans;

		m_scaleSceneToGL = m_Wglpre / (m_sceneWinView * m_scale);
	}
	else
	{
		left = -m_Hglpre * viewRatio / m_scale + m_WglTrans;
		right = m_Hglpre * viewRatio / m_scale + m_WglTrans;
		bottom = -m_Hglpre / m_scale - m_HglTrans;
		top = m_Hglpre / m_scale - m_HglTrans;

		m_scaleSceneToGL = m_Hglpre / (m_sceneHinView * m_scale);
	}

	m_projection = glm::ortho(left, right, bottom, top, 0.0f, 2.0f);
	measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);

	setViewRulerValue();
	SetGridValue();
}

void CW3View2D_forMPR::clearGL()
{
	m_pRender3DParam->ClearGLObjects();
	m_isUpdateSurface = true;
}

void CW3View2D_forMPR::resizeEvent(QResizeEvent* pEvent)
{
	if (!isVisible()) return;

	QGraphicsView::resizeEvent(pEvent);

	int width = this->width();
	int height = this->height();

	fitInView(0, 0, width, height);

	if (!m_isIntendedResize)
	{
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
	}
	else
	{
		resizeScene();
	}

	if (m_pWorldAxisItem)
	{
		m_pWorldAxisItem->SetSize(100, 100);
		m_pWorldAxisItem->setPos(
			mapToScene(QPoint(width, height) - QPoint(60, 60)));
	}

	setViewRulerValue();
	SetGridValue();

	HU_value_->setPos(mapToScene(30, height - 70));

	if (m_pProxySlider)
	{
		m_pProxySlider->resize(15, std::min(height * 0.33f, 159.0f));
		m_pProxySlider->setPos(mapToScene(
			width - common::ui_define::kViewMarginWidth -
			m_pProxySlider->sceneBoundingRect().width(),
			height * 0.5f - m_pProxySlider->sceneBoundingRect().height() * 0.5f));
	}

	QPointF posFilterItem =
		mapToScene(width - common::ui_define::kViewMarginWidth -
			sharpen_filter_text_->sceneBoundingRect().width(),
			common::ui_define::kViewFilterOffsetY);
	sharpen_filter_text_->setPos(posFilterItem);
}

void CW3View2D_forMPR::resizeScene()
{
	float preScale = m_scaleSceneToGL;
	QPointF preViewCenterInScene = m_pntCurViewCenterinScene;

	QPointF leftTop = mapToScene(QPoint(0, 0));
	QPointF rightBottom = mapToScene(QPoint(this->width(), this->height()));
	m_sceneWinView = (rightBottom.x() - leftTop.x()) * 0.5f;
	m_sceneHinView = (rightBottom.y() - leftTop.y()) * 0.5f;

	rightBottom = mapToScene(QPoint(this->width() - 1, this->height() - 1));
	m_pntCurViewCenterinScene.setX((rightBottom.x() + leftTop.x()) * 0.5f);
	m_pntCurViewCenterinScene.setY((rightBottom.y() + leftTop.y()) * 0.5f);

	m_SceneGLOffset = mapToScene(QPoint(0, 2)) - leftTop;
	m_SceneGLOffset.setY(m_SceneGLOffset.y() - 1.0f);

	setViewProjection();

	float scale = preScale / m_scaleSceneToGL;
	transformItems(preViewCenterInScene, m_pntCurViewCenterinScene, scale);

	measure_tools_->SetCenter(m_pntCurViewCenterinScene);
	measure_tools_->SetScale(m_scaleSceneToGL / m_scaleVolToGL);
}

void CW3View2D_forMPR::ScaleView(const QPoint& curr_view_pos)
{
	m_scalePre = m_scale;
	m_scale = m_scale + float(last_view_pos_.y() - curr_view_pos.y()) /
		float(this->height());

	if (m_scale < 0.3f) m_scale = 0.3f;

	setViewProjection();

	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene,
		m_scale / m_scalePre);

	last_view_pos_ = curr_view_pos;

	if (m_scale / m_scalePre > 1.0f)
		QApplication::setOverrideCursor(CW3Cursor::ZoomInCursor());
	else
		QApplication::setOverrideCursor(CW3Cursor::ZoomOutCursor());

	emit sigSyncScale(m_scale);
}

void CW3View2D_forMPR::PanningView(const QPointF& curr_scene_pos)
{
	float transX = last_scene_pos_.x() - curr_scene_pos.x();
	float transY = last_scene_pos_.y() - curr_scene_pos.y();

	m_WglTrans += transX * m_scaleSceneToGL;
	m_HglTrans += transY * m_scaleSceneToGL;

	setViewProjection();
	transformItems(QPointF(-transX, -transY));
	QPointF scene_trans(m_WglTrans / m_scaleSceneToGL,
		m_HglTrans / m_scaleSceneToGL);
	measure_tools_->SetSceneTrans(scene_trans);

	last_scene_pos_ = curr_scene_pos;
	QApplication::setOverrideCursor(CW3Cursor::ClosedHandCursor());
}

void CW3View2D_forMPR::WindowingView(const QPointF& curr_scene_pos)
{
	m_nAdjustWindowLevel += (last_scene_pos_.y() - curr_scene_pos.y()) * 2;
	m_nAdjustWindowWidth += (curr_scene_pos.x() - last_scene_pos_.x()) * 2;
	changeWLWW();
	last_scene_pos_ = curr_scene_pos;

	emit sigSyncWindowing(m_nAdjustWindowLevel, m_nAdjustWindowWidth);
}

void CW3View2D_forMPR::mousePressEvent(QMouseEvent* event)
{
	QGraphicsView::mousePressEvent(event);

	if (m_pProxySlider && m_pProxySlider->isUnderMouse()) return;

	last_view_pos_ = event->pos();
	last_scene_pos_ = mapToScene(last_view_pos_);

	Qt::MouseButton button = event->button();
	if (button == Qt::LeftButton)
	{
		if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_))
		{
			measure_tools_->ProcessMousePressed(last_scene_pos_, kTempPos);
		}
	}
	else if (button == Qt::RightButton)
	{
		//is_measure_delete_event_ = true;
		is_right_button_clicked_ = true;
	}

	QMouseEvent ev(QEvent::GraphicsSceneMousePress, event->pos(), event->button(),
		event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);
}

void CW3View2D_forMPR::mouseMoveEvent(QMouseEvent* event)
{
	QGraphicsView::mouseMoveEvent(event);

	if (common_tool_type_ == common::CommonToolTypeOnOff::V_ZOOM_R ||
		common_tool_type_ == common::CommonToolTypeOnOff::V_PAN_LR)
	{
		is_measure_delete_event_ = false;
	}

	if (m_pProxySlider && m_pProxySlider->isUnderMouse()) return;

	//CW3Cursor::SetViewCursor(common_tool_type_);
	QPoint curr_view_pos = event->pos();
	if (measure_tools_->IsMeasureInteractionAvailable(common_tool_type_))
	{
		if (measure_tools_->IsDrawing())
		{
			measure_tools_->ProcessMouseMove(mapToScene(curr_view_pos), kTempPos);
			return;
		}
	}

	Qt::MouseButtons buttons = event->buttons();
	if (buttons == Qt::NoButton)
	{
	}
	else
	{
		if (buttons == Qt::RightButton)
		{
			is_measure_delete_event_ = false;
		}
		ToolTypeInteractions2DInMove(curr_view_pos);
	}
}

void CW3View2D_forMPR::mouseReleaseEvent(QMouseEvent* event)
{
	QGraphicsView::mouseReleaseEvent(event);

	bool is_current_measure_available = measure_tools_->IsMeasureInteractionAvailable(common_tool_type_);

	QMouseEvent ev(QEvent::GraphicsSceneMouseRelease, event->pos(), event->button(), event->buttons(), event->modifiers());
	QApplication::sendEvent(QApplication::instance(), &ev);

	if (m_pProxySlider && m_pProxySlider->isUnderMouse())
	{
		is_right_button_clicked_ = false;
		return;
	}

	CW3Cursor::SetViewCursor(common_tool_type_);
	Qt::MouseButton button = event->button();
	if ((is_current_measure_available && is_measure_delete_event_) ||
		(/*button == Qt::LeftButton && */common_tool_type_ == common::CommonToolTypeOnOff::M_FREEDRAW))
	{
		measure_tools_->ProcessMouseReleased(button, mapToScene(event->pos()), kTempPos);
	}

	is_right_button_clicked_ = false;
	is_measure_delete_event_ = false;
}

void CW3View2D_forMPR::mouseDoubleClickEvent(QMouseEvent* event)
{
	QGraphicsView::mouseDoubleClickEvent(event);

	//if (event->buttons() & Qt::LeftButton)
		measure_tools_->ProcessMouseDoubleClick(event->button(), mapToScene(event->pos()), kTempPos);

	if (event->button() == Qt::RightButton)
	{
		is_measure_delete_event_ = true;
		is_right_button_clicked_ = true;
	}
}

void CW3View2D_forMPR::ResetView()
{
	setWLWW();
	resetFromViewTool();
}

void CW3View2D_forMPR::FitView()
{
	if (!isVisible()) return;

	resetFromViewTool();
	scene()->update();
}

void CW3View2D_forMPR::InvertView(bool bToggled)
{
	invert_windowing_ = bToggled;

	changeWLWW();

	if (isVisible()) scene()->update();
}

void CW3View2D_forMPR::HideText(bool bToggled)
{
	// HideUI 占쏙옙 占쏙옙占쏙옙
}

void CW3View2D_forMPR::HideUI(bool bToggled)
{
	hide_all_view_ui_ = bToggled;
	single_view_hide_ui_ = bToggled;
	SetVisibleItems();
}

void CW3View2D_forMPR::HideMeasure(bool toggled)
{
	measure_tools_->SetVisibleFromToolbar(!toggled);
}

void CW3View2D_forMPR::DeleteAllMeasure()
{
	measure_tools_->DeleteAllMeasure();
}

void CW3View2D_forMPR::SetMeasure3Dmode()
{
	measure_tools_->SetViewRenderMode(common::ReconTypeID::VR);
}

bool CW3View2D_forMPR::IsDefaultInteractionsIn2D(Qt::MouseButton button)
{
	if ((button == (Qt::LeftButton | Qt::RightButton)) ||
		button == Qt::RightButton)
		return true;
	return false;
}

void CW3View2D_forMPR::VisibleObject(const VisibleID& visible_id, int state)
{
	switch (visible_id)
	{
	case VisibleID::NERVE:
		VisibleNerve(state);
		break;
	case VisibleID::IMPLANT:
		VisibleImplant(state);
		break;
#ifndef WILL3D_LIGHT
	case VisibleID::SECONDVOLUME:
		VisibleSecond(state);
		break;
	case VisibleID::AIRWAY:
		VisibleAirway(state);
		break;
	case VisibleID::FACEPHOTO:
		VisibleFace(state);
		break;
#endif
	default:
		break;
	}
}

void CW3View2D_forMPR::VisibleNerve(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_pNerve->setVisible(true);
		drawImageOnTexture(m_pViewPlane[0], false);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_pNerve->setVisible(false);
		break;
	}

	if (isVisible()) scene()->update();
}

void CW3View2D_forMPR::VisibleImplant(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_pRender3DParam->m_isImplantShown = true;
		m_pRender3DParam->g_is_implant_exist_ = m_pgRcontainer->getImplantThereContainer();
		m_pRender3DParam->g_is_implant_collided_ = m_pgRcontainer->getCollisionContainer();

		drawImageOnTexture(m_pViewPlane[0], false);
		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_isImplantShown = false;
		break;
	}

	if (isVisible()) scene()->update();
}

void CW3View2D_forMPR::VisibleFace(int state)
{
	if (!isVisible()) return;

	switch (state)
	{
	case Qt::CheckState::Checked:
		if (!m_pgRcontainer) return;

		if (!m_pgRcontainer->getFacePhoto3D()) return;

		m_modelPhoto = m_pgRcontainer->getFacePhoto3D()->getSRtoVol() *
			glm::scale(*m_pgVREngine->getVolRange(0));
		m_pRender3DParam->m_photo3D->setVisible(true);

		drawImageOnTexture(m_pViewPlane[0], false);

		break;
	case Qt::CheckState::Unchecked:
		m_pRender3DParam->m_photo3D->setVisible(false);
		break;
	}

	if (isVisible()) scene()->update();
}

void CW3View2D_forMPR::VisibleSecond(int state)
{
	switch (state)
	{
	case Qt::CheckState::Checked:
		m_isDrawBoth = true;
		drawImageOnViewPlane(false, 1, false);
		scene()->update();
		break;
	case Qt::CheckState::Unchecked:
		m_isDrawBoth = false;
		scene()->update();
		break;
	}
}

void CW3View2D_forMPR::VisibleVolMain(bool state)
{
	if (state)
	{
		m_drawVolId = 0;
		m_isDrawBoth = false;
		scene()->update();
	}
}

void CW3View2D_forMPR::VisibleVolSecond(bool state)
{
	if (state)
	{
		m_drawVolId = 1;
		m_isDrawBoth = false;
		scene()->update();
	}
}

void CW3View2D_forMPR::VisibleVolBoth(bool state)
{
	if (state)
	{
		m_drawVolId = 0;
		m_isDrawBoth = true;
		scene()->update();
	}
}

void CW3View2D_forMPR::InitFacePhoto3D()
{
	glm::mat4 model_photo_to_mc = m_pgRcontainer->getFacePhoto3D()->getSRtoVol();
	m_modelPhoto = model_photo_to_mc * glm::scale(*m_pgVREngine->getVolRange(0));
	m_modelPhotoForTexture = m_inverseScale * model_photo_to_mc * m_scaleMat;

	if (!m_pRender3DParam->m_photo3D || !isVisible()) return;

	m_pGLWidget->makeCurrent();

	if (m_pRender3DParam->m_photo3D->getVAO())
		m_pRender3DParam->m_photo3D->clearVAOVBO();

	m_pRender3DParam->m_photo3D->setNindices(
		m_pgRcontainer->getFacePhoto3D()->getNindices());

	unsigned int vao = 0;
	CW3GLFunctions::initVAOSR(&vao, m_pgRcontainer->getFacePhoto3D()->getVBO());
	m_pRender3DParam->m_photo3D->setVAO(vao);

	scene()->update();

	m_pGLWidget->doneCurrent();
}

void CW3View2D_forMPR::DeleteMeasureUI(const unsigned int& measure_id)
{
	measure_tools_->SyncDeleteMeasureUI(measure_id);
}

void CW3View2D_forMPR::GetMeasureParams(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	common::measure::VisibilityParams* measure_params)
{
	if (measure_tools_) measure_tools_->GetMeasureParams(view_type, measure_id, measure_params);
}

void CW3View2D_forMPR::leaveEvent(QEvent* event)
{
	QGraphicsView::leaveEvent(event);
	QApplication::setOverrideCursor(CW3Cursor::ArrowCursor());
	HU_value_->setVisible(false);
}

void CW3View2D_forMPR::enterEvent(QEvent* event)
{
	QGraphicsView::enterEvent(event);
	this->setFocus();
	CW3Cursor::SetViewCursor(common_tool_type_);
	bool is_visible = !(hide_all_view_ui_ || single_view_hide_ui_);
	HU_value_->setVisible(is_visible);
}

void CW3View2D_forMPR::setVisible(bool state)
{
	if (measure_tools_)
	{
		measure_tools_->SetVisibleFromView(state);
	}

	/////////////////////////////////////////////////////////////////
	// thyoo 170124: QOpenGLWidget memory issue. (offscreen 占쏙옙占쏙옙占?GPU占쏙옙 占쏙옙占쏙옙占쏙옙
	// 占싹댐옙 占쏙옙荑∽옙占?占쌔댐옙) setVisible(false)占쏙옙 占실억옙占쏙옙 占쏙옙 QOpenGLWidget占쏙옙
	// 占쏙옙占쏙옙占싹곤옙占쌍댐옙 GPU 占쌨모리곤옙 占쏙옙占쏙옙占쏙옙 占싫되어서 占쏙옙占?view占쏙옙 활占쏙옙화 占실억옙占쏙옙 占쏙옙 GPU
	//占쌨몌옙 占쏙옙占쏙옙 占쏙옙占싹댐옙. 占쌓뤄옙占쏙옙 占쌨몌옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쌍억옙占?占싹댐옙 占쏙옙황占싸듸옙,
	//QOpenGLWidget占쏙옙 FBO, Texture占쏙옙 占쌘들링 占쏙옙 占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 Delete占싼댐옙.
	// setViewport占쏙옙 占쏙옙占쏙옙占?占쌕꾸억옙占쌍몌옙 占쏙옙 view占쏙옙 child占쏙옙 QOpenGLWidget占쏙옙 占쌘듸옙占쏙옙占쏙옙
	// delete 占싫댐옙.
	/////////////////////////////////////////////////////////////////
	if (state) newQOpenGLWidget();

	QGraphicsView::setVisible(state);

	if (state)
	{
		if (m_is2Dready)
		{
			drawImageOnTexture(m_pViewPlane[0], false);
		}
	}
	else
	{
		deleteQOpenGLWidget();
	}
}

void CW3View2D_forMPR::setViewRulerValue()
{
	if (!&ResourceContainer::GetInstance()->GetMainVolume())
		return;

	QPointF viewSizeInScene =
		mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f);

	float pixelSpacing =
		ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	QPointF viewLength =
		viewSizeInScene * pixelSpacing * m_scaleSceneToGL / m_scaleVolToGL;

	ruler_->setViewRuler(viewSizeInScene.x(), viewSizeInScene.y(), viewLength.x(),
		viewLength.y(),
		m_pntCurViewCenterinScene - m_SceneGLOffset);
}

void CW3View2D_forMPR::SetGridValue()
{
	if (!&ResourceContainer::GetInstance()->GetMainVolume())
		return;

	QPointF viewSizeInScene =
		mapToScene(this->width(), this->height()) - mapToScene(0.0f, 0.0f);

	float pixelSpacing =
		ResourceContainer::GetInstance()->GetMainVolume().GetBasePixelSize();
	QPointF viewLength =
		viewSizeInScene * pixelSpacing * m_scaleSceneToGL / m_scaleVolToGL;

	int index = GlobalPreferences::GetInstance()
		->preferences_.general.display.grid_spacing;
	int spacing = GlobalPreferences::GetInstance()
		->preferences_.general.display.grid_spacing_preset[index];
	grid_->SetGrid(viewSizeInScene.x(), viewSizeInScene.y(), viewLength.x(),
		viewLength.y(), m_pntCurViewCenterinScene - m_SceneGLOffset,
		static_cast<float>(spacing));
	grid_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_ & grid_visible_);
}

void CW3View2D_forMPR::DeleteUnfinishedMeasure()
{
	measure_tools_->ClearUnfinishedItem();
}

void CW3View2D_forMPR::slotSelectFilter(const int index)
{
	sharpen_level_ = index;

	updateImage();
}

void CW3View2D_forMPR::slotChangedValueSlider(int value)
{
	if (!m_pSlider) return;

	if (m_is2Dready && m_pSlider->pressed())
	{
		slider_value_set_ = false;
		changedDeltaSlider(value - prev_slider_value_);
		prev_slider_value_ = value;
		slider_value_set_ = true;
	}
}
void CW3View2D_forMPR::slotSetProfileData(const QPointF& start_scene_pos,
	const QPointF& end_scene_pos,
	std::vector<short>& data)
{
	const int nWidth = m_pViewPlane[0]->getWidth();
	const int nHeight = m_pViewPlane[0]->getHeight();
	const float scale = m_scaleSceneToGL / m_scaleVolToGL;

	float trans_x = m_WglTrans / m_scaleVolToGL;
	float trans_y = m_HglTrans / m_scaleVolToGL;
	QPointF posFirst = start_scene_pos - m_pntCurViewCenterinScene;
	posFirst.setX(posFirst.x() * scale + nWidth * 0.5f + trans_x);
	posFirst.setY(posFirst.y() * scale + nHeight * 0.5f + trans_y);

	QPointF posEnd = end_scene_pos - m_pntCurViewCenterinScene;
	posEnd.setX(posEnd.x() * scale + nWidth * 0.5f + trans_x);
	posEnd.setY(posEnd.y() * scale + nHeight * 0.5f + trans_y);

	QPointF size = posEnd - posFirst;
	const int nLength =
		(int)sqrt((int)(size.rx() * size.rx() + size.ry() * size.ry()));

	const float step_x = size.rx() / nLength;
	const float step_y = size.ry() / nLength;
	float px = posFirst.rx();
	float py = posFirst.ry();

	data.resize(nLength);
	float intercept =
		ResourceContainer::GetInstance()->GetMainVolume().intercept();
	unsigned short* img_buffer = m_pViewPlane[0]->getImage2D()->getData();
	for (int i = 0; i < nLength; i++)
	{
		px += step_x;
		py += step_y;
		if (px > nWidth || py > nHeight || px < 0 || py < 0)
		{
			data[i] = common::dicom::kInvalidHU;
		}
		else
		{
			data[i] = img_buffer[(int)py * nWidth + (int)px] + intercept;
		}
	}
}

void CW3View2D_forMPR::slotSetROIData(const QPointF& start_scene_pos,
	const QPointF& end_scene_pos,
	std::vector<short>& data)
{
	const int nWidth = m_pViewPlane[0]->getWidth();
	const int nHeight = m_pViewPlane[0]->getHeight();
	const float scale = m_scaleSceneToGL / m_scaleVolToGL;

	const float trans_x = m_WglTrans / m_scaleVolToGL;
	const float trans_y = m_HglTrans / m_scaleVolToGL;
	QPointF posFirst = start_scene_pos - m_pntCurViewCenterinScene;
	posFirst.setX(posFirst.x() * scale + nWidth * 0.5f + trans_x);
	posFirst.setY(posFirst.y() * scale + nHeight * 0.5f + trans_y);

	QPointF posEnd = end_scene_pos - m_pntCurViewCenterinScene;
	posEnd.setX(posEnd.x() * scale + nWidth * 0.5f + trans_x);
	posEnd.setY(posEnd.y() * scale + nHeight * 0.5f + trans_y);

	int start_x = static_cast<int>(posFirst.x());
	int end_x = static_cast<int>(posEnd.x());
	int start_y = static_cast<int>(posFirst.y());
	int end_y = static_cast<int>(posEnd.y());

	float intercept =
		ResourceContainer::GetInstance()->GetMainVolume().intercept();
	unsigned short* img_buffer = m_pViewPlane[0]->getImage2D()->getData();

	data.reserve((end_x - start_x) * (end_y - start_y));
	for (int y = start_y; y < end_y; ++y)
	{
		for (int x = start_x; x < end_x; ++x)
		{
			if (x < nWidth && y < nHeight && x >= 0 && y >= 0)
			{
				data.push_back(img_buffer[y * nWidth + x] + intercept);
			}
			else
			{
				data.push_back(common::dicom::kInvalidHU);
			}
		}
	}
}

void CW3View2D_forMPR::setVAOVBOImplant(int idx)
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	const auto& datas = res_implant.data();

	auto data = datas.find(kImplantNumbers[idx]);

	const std::vector<glm::vec3>& verts = data->second->mesh_vertices();
	const std::vector<glm::vec3>& norms = data->second->mesh_normals();
	const std::vector<uint>& indices = data->second->mesh_indices();

	unsigned int vao = 0;
	std::vector<uint> vbo;
	vbo.resize(3, 0);
	int cnt_indices = indices.size();

	CW3GLFunctions::initVAOVBO(&vao, &vbo[0], verts, norms, indices);

	m_pRender3DParam->m_pImplant[idx]->setVAO(vao);
	m_pRender3DParam->m_pImplant[idx]->setVBO(vbo);
	m_pRender3DParam->m_pImplant[idx]->setNindices(cnt_indices);
}
void CW3View2D_forMPR::drawImplantID(int index, const QPointF& implantPos,
	bool bVisible)
{
	if (!implant_id_text_) return;

	implant_id_text_->setPos(index, implantPos);
	implant_id_text_->setID(index, kImplantNumbers[index]);
	implant_id_text_->setVisible(index, bVisible);
}

void CW3View2D_forMPR::drawImplantID(bool bVisible)
{
	if (!implant_id_text_) return;

	implant_id_text_->setVisible(bVisible);
}

void CW3View2D_forMPR::initVectorsUpBack()
{
	m_vUpVec = glm::vec3();
	m_vBackVec = glm::vec3();
}
void CW3View2D_forMPR::transformItems(const QPointF& translate)
{
	QTransform transform;
	transform.translate(translate.x(), translate.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}
void CW3View2D_forMPR::transformItems(const QPointF& preViewCenterInScene,
	const QPointF& curViewCenterInScene,
	float scale)
{
	QTransform transform;
	transform.translate(curViewCenterInScene.x(), curViewCenterInScene.y());
	transform.scale(scale, scale);
	transform.translate(-preViewCenterInScene.x(), -preViewCenterInScene.y());

	transformPositionMemberItems(transform);
	transformPositionItems(transform);
}
void CW3View2D_forMPR::transformPositionMemberItems(
	const QTransform& transform)
{
	implant_id_text_->transformItems(transform);
	measure_tools_->TransformItems(transform);
}
void CW3View2D_forMPR::resetFromViewTool()
{
	transformItems(m_sceneTrans);
	transformItems(m_pntCurViewCenterinScene, m_pntCurViewCenterinScene,
		m_initScale / m_scale);

	setInitScale();

	m_WglTrans = 0.0f;
	m_HglTrans = 0.0f;
	m_sceneTrans = QPointF(0.0f, 0.0f);
	m_Wglpre = m_Wgl;
	m_Hglpre = m_Hgl;

	setViewProjection();
}

void CW3View2D_forMPR::newQOpenGLWidget()
{
	if (m_pGLWidget) return;

	m_pGLWidget = new QOpenGLWidget(this);

#if !defined(__APPLE__)
	QSurfaceFormat format;
	format.setSamples(4);
	m_pGLWidget->setFormat(format);
#endif

	setViewport(m_pGLWidget);
	m_pRender3DParam->SetCurGLWidget(m_pGLWidget);
}

void CW3View2D_forMPR::deleteQOpenGLWidget()
{
	if (!m_pGLWidget || !m_pGLWidget->context()) return;

	this->clearGL();
	setViewport(nullptr);
	m_pGLWidget = nullptr;
	m_pRender3DParam->SetCurGLWidget(nullptr);
}

void CW3View2D_forMPR::setViewProjection()
{
	setProjection();  // virtual function

	m_sceneTrans =
		QPointF(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL);
}

glm::vec4 CW3View2D_forMPR::getPlaneEquation()
{
	if (m_pViewPlane[0])
	{
		unsigned int volWidth = m_pgVREngine->getVol(0)->width();
		unsigned int volHeight = m_pgVREngine->getVol(0)->height();
		unsigned int volDepth = m_pgVREngine->getVol(0)->depth();
		vec3 glDist = (m_pViewPlane[0]->getPlaneCenterInVol() -
			vec3((volWidth - 1) * 0.5f, (volHeight - 1) * 0.5f,
			(volDepth - 1) * 0.5f)) *
				(vec3(2.0f / (volWidth - 1), 2.0f / (volHeight - 1),
					2.0f / (volDepth - 1)));

		vec3 upVec = glm::normalize(m_pViewPlane[0]->getUpVec());

		return glm::vec4(upVec, glm::dot(glDist, upVec));
	}
	return glm::vec4();
}

glm::vec3 CW3View2D_forMPR::getPlaneRightVector()
{
	if (m_pViewPlane[0])
	{
		return m_pViewPlane[0]->getRightVec();
	}
	return glm::vec3();
}

glm::vec3 CW3View2D_forMPR::GetPlaneUpVector()
{
	if (m_pViewPlane[0])
	{
		return m_pViewPlane[0]->getUpVec();
	}
	return glm::vec3();
}

void CW3View2D_forMPR::keyPressEvent(QKeyEvent* event)
{
	QWidget::keyPressEvent(event);
}

void CW3View2D_forMPR::setInitScale() { m_scale = m_initScale; }

void CW3View2D_forMPR::ApplyPreferences()
{
	bool is_text_visible = !(hide_all_view_ui_ || single_view_hide_ui_);
	show_rulers_ = GlobalPreferences::GetInstance()
		->preferences_.general.display.show_rulers &&
		m_eViewType != common::ViewTypeID::MPR_ZOOM3D;
	ruler_->setVisible(!single_view_hide_ui_ && !hide_all_view_ui_ && show_rulers_);

	measure_tools_->ApplyPreferences();

	if (grid_)
	{
		int index = GlobalPreferences::GetInstance()
			->preferences_.general.display.grid_spacing;
		int spacing = GlobalPreferences::GetInstance()
			->preferences_.general.display.grid_spacing_preset[index];
		grid_->SetGridSpacing(spacing);
	}

	scene()->update();
}

glm::vec3 CW3View2D_forMPR::MapSceneToVol(const QPointF& scene_pos)
{
	float scene_trans_x = m_WglTrans / m_scaleSceneToGL;
	float scene_trans_y = m_HglTrans / m_scaleSceneToGL;

	int idx = scaledSceneToVol(scene_pos.x() + scene_trans_x - m_pntCurViewCenterinScene.x());
	int idy = scaledSceneToVol(scene_pos.y() + scene_trans_y - m_pntCurViewCenterinScene.y());

	glm::vec3 vol_pos = m_pViewPlane[0]->getPlaneCenterInVol() +
		m_pViewPlane[0]->getRightVec() * float(idx) +
		m_pViewPlane[0]->getBackVec() * float(idy);

	return vol_pos;
}

QPointF CW3View2D_forMPR::MapVolToScene(const glm::vec3& vol_pos)
{
	glm::vec3 gl_pos = (vol_pos - m_pViewPlane[0]->getPlaneCenterInVol());

	QPointF scene_pos(scaledVolToScene(glm::dot(gl_pos, m_pViewPlane[0]->getRightVec())), scaledVolToScene(glm::dot(gl_pos, m_pViewPlane[0]->getBackVec())));

	scene_pos += m_pntCurViewCenterinScene;

	float scene_trans_x = m_WglTrans / m_scaleSceneToGL;
	float scene_trans_y = m_HglTrans / m_scaleSceneToGL;

	scene_pos -= QPointF(scene_trans_x, scene_trans_y);

	return scene_pos;
}

glm::mat4 CW3View2D_forMPR::GetProjectionViewMatrix() const
{
	return m_projection * m_view;
}
