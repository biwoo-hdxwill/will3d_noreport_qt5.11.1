#include "W3ORTHOtab.h"
/*=========================================================================

File:			class CW3ORTHOtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-02

=========================================================================*/
#include <qtoolbutton.h>
#include <QApplication>

#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Common/Common/W3Memory.h"
#include "../../Engine/UIModule/UIMenus/W3TabMenus.h"
#include "../../Engine/UIModule/UIFrame/W3Window.h"
#include "../../Engine/UIModule/UIFrame/W3WindowVROrtho.h"
#include "../../Engine/UIModule/UIFrame/W3ViewLayout.h"
#include "../../Engine/Module/VREngine/W3VREngine.h"

#include "W3ORTHOViewMgr.h"

CW3ORTHOtab::CW3ORTHOtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
						 CW3ResourceContainer *Rcontainer, CW3ViewLayout *viewLayout,
						 CW3TabMenus *OrthoMenus) :
	CW3BASEtab(VREngine, viewLayout),
	m_pgOrthoMenus(OrthoMenus),
	m_pgVREngine(VREngine),
	m_pgMPRengine(MPRengine),
	m_pgRcontainer(Rcontainer) {}

CW3ORTHOtab::~CW3ORTHOtab(void) {
	SAFE_DELETE_OBJECT(m_pORTHOViewMgr);
	SAFE_DELETE_OBJECT(m_pWindowOrtho);
	SAFE_DELETE_OBJECT(m_pWindowOrthoMax);
	SAFE_DELETE_OBJECT(m_pLayout);
}

void CW3ORTHOtab::exportProject(QDataStream& out) {
	m_pORTHOViewMgr->exportProject(out);
}
void CW3ORTHOtab::importProject(QDataStream& in, const int& version) {
	m_pORTHOViewMgr->importProject(in, version);
}

void CW3ORTHOtab::reset() {
	CW3BASEtab::reset();

	setLayout(is_otf_on_);

	m_pWindowOrtho->reset();
	m_pORTHOViewMgr->reset();
}

void CW3ORTHOtab::Initialize() {
	if (CW3BASEtab::initialized()) {
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}

	m_pORTHOViewMgr = new CW3ORTHOViewMgr(m_pgVREngine, m_pgMPRengine, m_pgRcontainer, common_menus());

	m_pWindowOrtho = new CW3WindowVROrtho(&m_pORTHOViewMgr->getListViewOrtho());
	m_pWindowOrthoMax = new CW3Window(m_pORTHOViewMgr->getViewOrthoMax(), EVIEW_TYPE::VR_MPR, true);
	m_pWindowOrthoMax->setMaximize(true);

	setLayout(is_otf_on_);

	connections();

	CW3BASEtab::set_initialized(true);
}

void CW3ORTHOtab::connections() {
	connect(m_pgVRengine, SIGNAL(sigTFupdated(bool)), m_pORTHOViewMgr, SLOT(slotTFUpdated(bool)));
	connect(m_pgVRengine, SIGNAL(sigTFupdateCompleted()), m_pORTHOViewMgr, SLOT(slotTFUpdateCompleted()));
	connect(m_pgVRengine, SIGNAL(sigReoriupdate(glm::mat4*)), m_pORTHOViewMgr, SLOT(slotReoriupdate(glm::mat4*)));

	connect(m_pgOrthoMenus->m_btnOTFonoff, SIGNAL(clicked()), this, SLOT(slotOTFonoff()));

	connect(m_pORTHOViewMgr, SIGNAL(sigAddOrthoCurve()), m_pWindowOrtho, SLOT(slotAddOrthoCurve()));
	connect(m_pORTHOViewMgr, SIGNAL(sigDeleteOrthoCurve(int)), this, SLOT(slotDeleteOrthoCurve(int)));
	connect(m_pORTHOViewMgr, SIGNAL(sigPrevOrtho()), this, SLOT(slotPrevOrtho()));
	connect(m_pORTHOViewMgr, SIGNAL(sigNextOrtho()), this, SLOT(slotNextOrtho()));

	connect(m_pWindowOrtho, SIGNAL(sigSetCurveOpacity(int, int)), m_pORTHOViewMgr, SLOT(slotVisibleOrthoCurveUI(int, int)));
	connect(m_pWindowOrtho, SIGNAL(sigMaximizeonoff(EVIEW_TYPE, bool)), this, SLOT(slotMaximizeonoff(EVIEW_TYPE, bool)));

	connect(m_pWindowOrthoMax, SIGNAL(sigMaximizeonoff(EVIEW_TYPE, bool)), this, SLOT(slotMaximizeoff(EVIEW_TYPE)));

	connect(this, SIGNAL(sigSetMaximizeView(EVIEW_TYPE)), m_pORTHOViewMgr, SLOT(slotMaximizeSliceView(EVIEW_TYPE)));
}

void CW3ORTHOtab::slotDeleteOrthoCurve(const int id) {
	if (m_pWindowOrtho->getCurveCnt() > 0) {
		/*for (int i = m_pWindowOrtho->getCurveCnt() - 1; i < m_pORTHOViewMgr->getListViewOrtho()->size(); i++)
			m_pORTHOViewMgr->getListViewOrtho()->at(i)->deleteOrthoCurve();*/

		m_pORTHOViewMgr->deleteOrthoCurve(id);
		m_pWindowOrtho->slotDeleteOrthoCurve(id);
	}
}

void CW3ORTHOtab::slotPrevOrtho() {
	EVIEW_TYPE curView = m_pORTHOViewMgr->getMaximizedViewType();
	if (curView > EVIEW_TYPE::VR_ORTHODONTIC_1) {
		slotMaximizeonoff((EVIEW_TYPE)((int)curView - 1), true);

		QApplication::processEvents();
	}
}

void CW3ORTHOtab::slotNextOrtho() {
	EVIEW_TYPE curView = m_pORTHOViewMgr->getMaximizedViewType();
	if (curView < EVIEW_TYPE::VR_ORTHODONTIC_10) {
		int nextView = (int)curView + 1;
		if (nextView - (int)EVIEW_TYPE::VR_ORTHODONTIC_1 < m_pWindowOrtho->getCurveCnt()) {
			slotMaximizeonoff((EVIEW_TYPE)nextView, true);

			QApplication::processEvents();
		}
	}
}

void CW3ORTHOtab::setLayout(bool isOTF) {
	SAFE_DELETE_OBJECT(m_pLayout);

	QVBoxLayout* left_layout = new QVBoxLayout();
	QVBoxLayout* right_layout = new QVBoxLayout();
	QHBoxLayout* up_layout = new QHBoxLayout();
	m_pLayout = new QVBoxLayout();

	if (m_eMaximizedViewType >= EVIEW_TYPE::VR_ORTHODONTIC_1 && m_eMaximizedViewType <= EVIEW_TYPE::VR_ORTHODONTIC_10) {
		left_layout->setSpacing(kLayoutSpacing);
		left_layout->addWidget(m_pWindowOrthoMax);
		left_layout->setStretch(0, 1);

		right_layout->setSpacing(kLayoutSpacing);
		right_layout->addWidget(m_pORTHOViewMgr->getViewSlice());
		right_layout->setStretch(0, 1);
		right_layout->addWidget(m_pWindowOrtho);
		right_layout->setStretch(1, 2);

		up_layout->setSpacing(kLayoutSpacing);
		up_layout->addLayout(left_layout);
		up_layout->setStretch(0, 2);
		up_layout->addLayout(right_layout);
		up_layout->setStretch(1, 1);

		m_pLayout->setSpacing(kLayoutSpacing);
		m_pLayout->addLayout(up_layout);
		m_pLayout->setStretch(0, 1);
	} else {
		left_layout->setSpacing(kLayoutSpacing);
		left_layout->addWidget(m_pWindowOrtho);
		left_layout->setStretch(0, 1);

		right_layout->setSpacing(kLayoutSpacing);
		right_layout->addWidget(m_pORTHOViewMgr->getViewSlice());
		right_layout->setStretch(0, 1);
		right_layout->addWidget(m_pORTHOViewMgr->getViewVR());
		right_layout->setStretch(1, 1);

		up_layout->setSpacing(kLayoutSpacing);
		up_layout->addLayout(left_layout);
		up_layout->setStretch(0, 1);
		up_layout->addLayout(right_layout);
		up_layout->setStretch(1, 1);

		m_pLayout->setSpacing(kLayoutSpacing);
		m_pLayout->addLayout(up_layout);
		m_pLayout->setStretch(0, 1);

		//m_pWindowOrthoMax->setVisible(false);
		//m_pORTHOViewMgr->getViewOrthoMax()->setVisible(false);
	}

	if (isOTF) {
		m_pLayout->setSpacing(kLayoutSpacing);
		otf_view_->setFixedHeight(200);
		m_pLayout->addWidget(otf_view_);
		m_pLayout->setStretch(1, 1);
	}

	tab_layout_ = m_pLayout;
}

void CW3ORTHOtab::setVisibleWindows(bool isVisible) {
	if (!initialized())
		return;

	m_bIsActiveTab = isVisible;

	m_pWindowOrtho->setVisible(isVisible);

	if (m_eMaximizedViewType != EVIEW_TYPE::UNKNOWN)	// by jdk 160906
	{
		m_pWindowOrthoMax->setVisible(isVisible);
		m_pORTHOViewMgr->setVisibleMax(isVisible);
	} else {
		m_pORTHOViewMgr->setVisibleVR(isVisible);
	}

	m_pORTHOViewMgr->setVisibleSlice(isVisible);

	if (otf_view_)
		otf_view_->setVisible(is_otf_on_ && isVisible);
}

void CW3ORTHOtab::slotMaximizeonoff(EVIEW_TYPE type, bool max) {
	m_pWindowOrtho->setVisible(false);
	m_pWindowOrtho->setMaximize(max);

	if (m_eMaximizedViewType != type) {
		m_eMaximizedViewType = type;

		m_pWindowOrthoMax->setVisible(false);

		m_pORTHOViewMgr->setVisibleVR(false);
		m_pORTHOViewMgr->setVisibleSlice(false);
		m_pORTHOViewMgr->setVisibleMax(false);

		setLayout(is_otf_on_);
		if (m_pgViewLayout)
			m_pgViewLayout->setViewLayout(tab_layout_);

		if (m_eMaximizedViewType == EVIEW_TYPE::UNKNOWN) {
			m_pORTHOViewMgr->setVisibleVR(true);
			m_pORTHOViewMgr->setVisibleSlice(true);
		} else {
			m_pWindowOrthoMax->setVisible(true);
			m_pORTHOViewMgr->setVisibleMax(true);

			m_pORTHOViewMgr->setVisibleSlice(true);
		}

		emit sigSetMaximizeView(m_eMaximizedViewType);
		if (m_pORTHOViewMgr->getViewOrthoMax())
			m_pORTHOViewMgr->setVisibleMax(m_eMaximizedViewType);
	}

	m_pWindowOrtho->setVisible(true);
}

void CW3ORTHOtab::slotMaximizeoff(EVIEW_TYPE type) {
	//m_eMaximizedViewType = EVIEW_TYPE::UNKNOWN;
	m_pWindowOrthoMax->setMaximize(true);
	slotMaximizeonoff(EVIEW_TYPE::UNKNOWN, false);

	/*setLayout(is_otf_on_);
	if (m_pgViewLayout)
		m_pgViewLayout->setViewLayout2(m_pgLayout);

	emit sigSetMaximizeView(m_eMaximizedViewType);*/
}

void CW3ORTHOtab::setMIP(bool isMIP) {
	m_pORTHOViewMgr->setMIP(isMIP);
}

QStringList CW3ORTHOtab::GetViewList() {
	return QStringList{
		"Orthodontic",
		"Axial",
		"3D"
	};
}

QImage CW3ORTHOtab::GetScreenshot(int view_type) {
	QWidget* source = nullptr;

	switch (view_type) {
	case 1:
		source = (m_pWindowOrthoMax->isVisible()) ? m_pWindowOrthoMax : m_pWindowOrtho;
		break;
	case 2:
		source = m_pORTHOViewMgr->getViewSlice();
		break;
	case 3:
		source = m_pORTHOViewMgr->getViewVR();
		break;
	}

	return CW3BASEtab::GetScreenshot(source);
}

void CW3ORTHOtab::ApplyPreferences() {
	if (m_pORTHOViewMgr)
		m_pORTHOViewMgr->ApplyPreferences();
}
