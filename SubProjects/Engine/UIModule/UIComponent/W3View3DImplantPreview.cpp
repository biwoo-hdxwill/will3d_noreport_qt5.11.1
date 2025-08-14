#include "W3View3DImplantPreview.h"
/*=========================================================================

File:			class CW3View3DImplantPreview
Language:		C++11
Library:		Qt 5.2.0
Author:			SANG KEUN PARK
First date:		2016-05-31

comment:
Implant Dialog에서 임플란트 미리보기용
=========================================================================*/
#include <qfileinfo.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Resource/Resource/W3Implant.h"
#include "../../Resource/Resource/W3Image3D.h"
#include <Engine/Resource/ResContainer/resource_container.h>

#include "../UIGLObjects/W3GLObject.h"
#include "../UIPrimitive/W3ViewRuler.h"

#include "../../Module/MPREngine/W3MPREngine.h"
#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3Render3DParam.h"

#include "../UIViewController/view_navigator_item.h"

CW3View3DImplantPreview::CW3View3DImplantPreview(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
												 CW3ResourceContainer *Rcontainer,
												 common::ViewTypeID eType, QWidget *pParent)
	: CW3View3D(VREngine, MPRengine, Rcontainer, eType) {
	recon_type_ = common::ReconTypeID::VR;
	m_scale = 1.0f;
}

CW3View3DImplantPreview::~CW3View3DImplantPreview() {
	SAFE_DELETE_OBJECT(m_pImplant);
}

void CW3View3DImplantPreview::init() {
	m_pRender3DParam->m_pgMainVolume[0] = m_pgVREngine->getVRparams(0);
	m_pRender3DParam->m_pgMainVolume[0]->m_isShown = false;

	m_vVolRange = *m_pgMPRengine->getVolRange(0);

	m_model = *m_pgVREngine->getReorientedModel();
	m_inverseScale = glm::scale(1.0f / m_vVolRange);

	m_rotate = m_model;
	m_rotate[3][0] = 0.0f;
	m_rotate[3][0] = 0.0f;
	m_rotate[3][0] = 0.0f;

	m_origModel = m_model;

	m_camFOV = glm::length(m_vVolRange);

	setViewMatrix();

	setProjection();
	if (m_pImplant)
		setInitScale();

	ruler_->Disable();
	m_is3Dready = true;

	m_pWorldAxisItem->setVisible(false);
}

void CW3View3DImplantPreview::setImplant(const QString& manufacturerName,
										 const QString& productName,
										 const QString& implantPath,
										 float diameter, float length) {
	if (implantPath.size() == 0) {
		SAFE_DELETE_OBJECT(m_pImplant);
		printf("ERROR: Implant load is failed\n");
		if (isVisible())
			this->scene()->update();
		return;
	}

	if (!m_pgVREngine->isVRready()) {
		SAFE_DELETE_OBJECT(m_pImplant);
		printf("ERROR: vr is not ready\n");
		if (isVisible())
			this->scene()->update();
		return;
	}

	SAFE_DELETE_OBJECT(m_pImplant);
	m_pImplant = new CW3Implant(m_pgVREngine->getGLWidget(), MAX_IMPLANT, diameter, length, false);

	const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
	m_NormFactor = glm::vec3(vol.width()*vol.pixelSpacing(),
							 vol.height()*vol.pixelSpacing(),
							 vol.depth()*vol.sliceSpacing()) / (*m_pgVREngine->getVolRange(0));

	bool implantLoad = false;
	QFileInfo info(implantPath);
	if (info.suffix().compare("stl", Qt::CaseInsensitive) == 0)
		implantLoad = m_pImplant->implantLoad(manufacturerName, productName, implantPath);
	else if (info.suffix().compare("wim", Qt::CaseInsensitive) == 0)
		implantLoad = m_pImplant->implantLoadforPreviewWIM(manufacturerName, productName, implantPath, m_NormFactor);

	if (implantLoad) {
		m_pImplant->setVBO();

		if (isVisible())
			setInitScale();
	} else {
		SAFE_DELETE_OBJECT(m_pImplant);
		printf("ERROR: Implant load is failed\n");
	}

	if (isVisible())
		this->scene()->update();
}

// 갱신시 화면 출력
void CW3View3DImplantPreview::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	if (m_pgVREngine->isVRready()) {
		if (!m_is3Dready) {
			init();
		}

		if (m_pImplant) {
			unsigned int implant_index = NW3Render3DParam::kImplantIDPreview;
			m_pImplant->setModelToTextureMatrix(m_vVolRange);

			m_pRender3DParam->g_is_implant_exist_[implant_index] = true;

			setMVP();

			if (m_pRender3DParam->m_pImplant[implant_index]->getVAO())
				m_pRender3DParam->m_pImplant[implant_index]->clearVAOVBO();

			unsigned int vao = 0;
			CW3GLFunctions::initVAOImplant(&vao, m_pImplant->getVBO());
			m_pRender3DParam->m_pImplant[implant_index]->setVAO(vao);
			m_pRender3DParam->m_pImplant[implant_index]->setNindices(m_pImplant->getNindices());

			m_pImplant->m_mvp = m_mvp * m_pImplant->m_rotate * m_pImplant->m_scaleMatrix;

			m_pRender3DParam->m_pImplant[implant_index]->setMVP(m_model*m_pImplant->m_rotate * m_pImplant->m_scaleMatrix, m_view, m_projection);
			m_pRender3DParam->m_pImplant[implant_index]->setVisible(true);

			m_pRender3DParam->m_isImplantShown = true;

			m_pRender3DParam->m_width = this->width();
			m_pRender3DParam->m_height = this->height();

			m_pgVREngine->RenderImplant3D(m_pRender3DParam, implant_index);
		} else {
			CW3GLFunctions::clearView(false);
		}
	} else {
		CW3GLFunctions::clearView(false);
	}

	m_rotAngle = 0.0f;
}

void CW3View3DImplantPreview::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);

	if (mouseMoveEventW3(event)) {
		scene()->update();
		//emit sigRotate(&m_rotate);
	}
}

void CW3View3DImplantPreview::setInitScale() {
	float implantSize = m_pImplant->getMaxAxisLength() * static_cast<float>(sqrt(2));

	if (width() > height())
		ChangeViewHeightLength(implantSize);
	else
		ChangeViewWidthLength(implantSize);
}
