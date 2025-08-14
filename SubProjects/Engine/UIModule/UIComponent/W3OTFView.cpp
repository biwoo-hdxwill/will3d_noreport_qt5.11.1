#include "W3OTFView.h"
/*=========================================================================

File:		class CW3OTFView
Language:	C++11
Library:	Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-22
Last modify:	2015-12-22

=========================================================================*/
#include <math.h>
#include <QMouseEvent>
#include <qapplication.h>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/define_ui.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_otf.h"
#include "../../UIModule/UIPrimitive/W3TextItem_switch.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "W3OTFScene.h"

CW3OTFView::CW3OTFView(/*CW3OTFScene *scene, */QWidget *parent)
	: QGraphicsView(parent) {
	setMouseTracking(true);

	QSizePolicy otfSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	otfSizePolicy.setHorizontalStretch(0);
	otfSizePolicy.setVerticalStretch(0);
	otfSizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
	this->setSizePolicy(otfSizePolicy);
	this->setMinimumSize(QSize(0, 200));
	this->setMaximumSize(QSize(16777215, 200));	
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	this->setStyleSheet(QString(
		"QScrollBar::handle:horizontal { background: rgb(250, 50, 50); }"
		"QScrollBar:horizontal { background: white; }"));
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_pOTFScene = new CW3OTFScene(this);
	this->setScene(m_pOTFScene);
	//this->setScene(scene);

	m_fFitWidth = 0.0f;
	m_fPreFitWidth = 0.0f;
	this->setDragMode(NoDrag);

	// Export & Preset.
	m_pTextSavePreset = new CW3TextItem();
	m_pTextSavePreset->setDefaultTextColor(Qt::white);
	m_pTextSavePreset->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
	m_pTextSavePreset->setPlainText(lang::LanguagePack::txt_save());
	m_pTextSavePreset->setPos(mapToScene(10, 20));
	m_pTextSavePreset->setZValue(100.0f);

	m_pShadeSwitch = new CW3TextItem_switch(lang::LanguagePack::txt_shading());
	m_pShadeSwitch->setPos(mapToScene(this->width() - 130, 20));

	connect(m_pTextSavePreset, SIGNAL(sigPressed(void)), this, SIGNAL(sigOTFSave()));
	connect(m_pShadeSwitch, SIGNAL(sigState(bool)), this, SIGNAL(sigShadeOnSwitch(bool)));
	connect(this->scene(), SIGNAL(sigShadeOnSwitch(bool)), this, SLOT(slotShadeOnSwitch(bool)));
	connect(this->scene(), SIGNAL(sigChangeTFMove(bool)), this, SIGNAL(sigChangeTFMove(bool)));
	connect(this->scene(), SIGNAL(sigRenderCompleted()), this, SIGNAL(sigRenderCompleted()));	
	connect(this->scene(), SIGNAL(sigSetScrollHandDrag(void)), this, SLOT(slotSetScrollHandDrag(void)));
	this->scene()->addItem(m_pTextSavePreset);
	this->scene()->addItem(m_pShadeSwitch);
}

CW3OTFView::~CW3OTFView(void) {
	this->scene()->removeItem(m_pTextSavePreset);
	SAFE_DELETE_OBJECT(m_pTextSavePreset);

	this->scene()->removeItem(m_pShadeSwitch);
	SAFE_DELETE_OBJECT(m_pShadeSwitch);

	if (m_pOTFScene)
		SAFE_DELETE_OBJECT(m_pOTFScene);
}

void CW3OTFView::setTeethPreset() {
	((CW3OTFScene*)this->scene())->setPreset("teeth");
}
void CW3OTFView::SetOTFpreset(const QString& preset) {
  const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
  if (!&vol) {
	common::Logger::instance()->Print(
	  common::LogType::ERR, "CW3OTFView::SetOTFpreset: volume is empty");
	return;
  }

  const float window_center = vol.windowCenter();
  const float window_width = vol.windowWidth();
  const float intercept = vol.intercept();

  if (preset.compare(common::otf_preset::MIP) == 0) {
	this->setPreset(common::otf_preset::MIP);
  }
  else if (preset.compare(common::otf_preset::XRAY) == 0) {
	  setPreset(common::otf_preset::XRAY);
#if 0
	  setXRayTF(vol.getMax() + intercept, vol.getMin() + intercept);
#else
	  setXRayTF(window_center + window_width * 0.5f + intercept, window_center - window_width * 0.5f + intercept);
#endif
  }
  else if (preset.contains(common::otf_preset::GRAY)) {
	  setPreset(common::otf_preset::GRAY);
	this->setXRayTF(window_center + window_width * 0.5f + intercept,
						 window_center - window_width * 0.5f + intercept);
  }
  else {
	this->setPreset(preset);
  }

  emit sigRenderCompleted();
}
void CW3OTFView::initOTF(int * histogram, int size, float slope, int offset) {
	m_pOTFScene->initOTF(histogram, size, slope, offset);
	is_resize_scene = true;

}

void CW3OTFView::setThreshold(int thdAir, int thdTissue, int thdBone) {
	m_pOTFScene->setThreshold(thdAir, thdTissue, thdBone);
}

bool CW3OTFView::setAdjustOTF(const AdjustOTF & val) {
	return m_pOTFScene->setAdjustOTF(val);
}

void CW3OTFView::Export(const QString & path) {
	m_pOTFScene->Export(path);
}

void CW3OTFView::movePolygonAtMin(float minValue) {
	m_pOTFScene->movePolygonAtMin(minValue);
}

void CW3OTFView::setPreset(const QString & str) {
	m_pOTFScene->setPreset(str);
}

void CW3OTFView::setXRayTF(float max, float min) {
	m_pOTFScene->setXRayTF(max, min);
}

QString CW3OTFView::getCurrentPreset() const {
	return m_pOTFScene->getCurrentPreset();
}

void CW3OTFView::connections() {
	connect(this->scene(), SIGNAL(sigShadeOnSwitch(bool)), this, SLOT(slotShadeOnSwitch(bool)));
	connect(this->scene(), SIGNAL(sigChangeTFMove(CW3TF*, bool)), this, SIGNAL(sigChangeTFMove(CW3TF*, bool)));
}

void CW3OTFView::disconnections() {
	disconnect(this->scene(), SIGNAL(sigShadeOnSwitch(bool)), this, SLOT(slotShadeOnSwitch(bool)));
	disconnect(this->scene(), SIGNAL(sigChangeTFMove(CW3TF*, bool)), this, SIGNAL(sigChangeTFMove(CW3TF*, bool)));
}

void CW3OTFView::slotShadeOnSwitch(bool isEnable) {
	m_pShadeSwitch->setCurrentState(isEnable);
}
void CW3OTFView::leaveEvent(QEvent * event) {
	QGraphicsView::leaveEvent(event);

	m_pOTFScene->deactiveItems();
}
void CW3OTFView::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);
	this->setDragMode(NoDrag);
}

void CW3OTFView::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
	if (event->buttons() & Qt::LeftButton) {
		setItemPos();
	}
}

void CW3OTFView::resizeEvent(QResizeEvent *event) {
	QGraphicsView::resizeEvent(event);
	
	if (!is_resize_scene) {
		if (m_fPreFitWidth == 0)
			m_fPreFitWidth = this->sceneRect().width();

		m_fFitWidth = m_fPreFitWidth;

		fitInView(0.0f, 0.0f, m_fFitWidth, this->sceneRect().height());
		is_resize_scene = true;
	}
	setItemPos();
}
void CW3OTFView::setVisible(bool is_visible) {
	QGraphicsView::setVisible(is_visible);

	if (is_visible) {
		if (m_fPreFitWidth == 0)
			m_fPreFitWidth = this->sceneRect().width();

		m_fFitWidth = m_fPreFitWidth;

		fitInView(0.0f, 0.0f, m_fFitWidth, this->sceneRect().height());
		is_resize_scene = true;
		setItemPos();
	}
}
void CW3OTFView::setItemPos() {
	m_pTextSavePreset->setPos(mapToScene(common::ui_define::kViewMarginWidth,
										 common::ui_define::kViewMarginHeight));
	QPointF posShadeSwitch =
		mapToScene(width() - common::ui_define::kViewMarginWidth - m_pShadeSwitch->sceneBoundingRect().width(),
				   common::ui_define::kViewMarginHeight);
	m_pShadeSwitch->setPos(posShadeSwitch.x(), posShadeSwitch.y());
}

void CW3OTFView::wheelEvent(QWheelEvent *event) // by jdk 151221
{
	QScrollBar *pScrollBar = nullptr;
	int nScrollPos = 0;
	double numDegrees = event->delta() / 8.0;
	double numSteps = numDegrees / 15.0;
	double factor = std::pow(1.125, (numSteps > 0) ? -1 : 1);

	// zoom
	if (m_fPreFitWidth == 0)
		m_fPreFitWidth = this->sceneRect().width();

	m_fFitWidth = m_fPreFitWidth * factor;
	if (m_fFitWidth < (this->width() * 0.2f))
		return;

	if (m_fFitWidth > this->sceneRect().width())
		m_fFitWidth = this->sceneRect().width();

	QPointF ptPreScene = mapToScene(event->pos());

	// keep current pos on view
	fitInView(0.0f, 0.0f, m_fFitWidth, this->sceneRect().height());

	QPointF ptNewScene = mapToScene(event->pos());
	centerOn((mapToScene(this->width() / 2.0f, 0.0f) + (ptPreScene - ptNewScene)).x(), 0.0f);

	m_fPreFitWidth = m_fFitWidth;

	setItemPos();

	scene()->update();

	QGraphicsView::wheelEvent(event);
}

void CW3OTFView::slotSetScrollHandDrag(void) {
	this->setDragMode(ScrollHandDrag);
}

void CW3OTFView::SetSoftTissueMin(const float value)
{
	m_pOTFScene->SetSoftTissueMin(value);
}
