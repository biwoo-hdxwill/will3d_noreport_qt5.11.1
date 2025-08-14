#include "W3View3DCeph.h"
/*=========================================================================

File:			class CW3View3DCeph
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-04-14
Last modify:	2016-04-14

=========================================================================*/
#include <qmath.h>
#include <qopenglwidget.h>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QApplication>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

#include "../../../Algorithm/Util/Core/PlyIO.h"

#include "../../Common/Common/color_will3d.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_otf.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/Common/event_handler.h"
#include "../../Common/Common/event_handle_common.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3ImageHeader.h"
#include "../../Resource/Resource/W3TF.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/face_photo_resource.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_ceph.h"
#endif
#include "../UIPrimitive/W3TextItem.h"
#include "../UIPrimitive/W3TextItem_switch.h"
#include "../UIPrimitive/view_border_item.h"
#include "../UIPrimitive/measure_tools.h"

#include "../UIGLObjects/W3SurfaceAxesItem.h"
#include "../UIGLObjects/W3SurfaceTextEllipseItem.h"
#include "../UIGLObjects/W3SurfacePlaneItem.h"
#include "../UIGLObjects/W3SurfaceAngleItem.h"
#include "../UIGLObjects/W3SurfaceDistanceItem.h"
#include "../UIGLObjects/W3SurfaceArchItem.h"
#include "../UIViewController/view_navigator_item.h"

#include "W3VTOSTO.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/VREngine/W3VolumeRenderParam.h"
#include "../../Module/VREngine/W3ActiveBlock.h"

#include "../../../Managers/DBManager/W3CephDM.h"

#include <Engine/UIModule/UIGLObjects/measure_3d_manager.h>

using namespace std;
using namespace surgery;
using namespace UIGLObjects;

namespace {
const int kTexHandlerSG = 6;
const glm::mat4 kIMat = glm::mat4(1.0f);
}

CW3View3DCeph::CW3View3DCeph(CW3VREngine *VREngine, CW3MPREngine *MPREngine,
							 CW3VTOSTO* vtosto, CW3CephDM* DataManager,
							 common::ViewTypeID eType, QWidget *pParent) :
	m_pgVTOSTO(vtosto), m_pgDataManager(DataManager),
	CW3View3D_thyoo(VREngine, MPREngine, eType, false, pParent) {
	this->setFrameShape(QFrame::NoFrame);

	for (int i = 0; i < surgery::CutTypeID::CUT_TYPE_END; i++) {
		m_lstAxesItem[i] = nullptr;
		m_lstSurgeryCutItem[i] = nullptr;
	}

	m_pCurrTracingText = new CW3TextItem(nullptr);
	m_pCurrTracingText->setTextColor("#FF82ABFF");
	m_pCurrTracingText->setBackground("#72303030");
	m_pCurrTracingText->setTextBold(true);
	this->scene()->addItem(m_pCurrTracingText);
	m_pCurrTracingText->setVisible(false);

	m_PROGsurfaceTexture = m_pgVREngine->getPROGsurfaceTexture();

	border_.reset(new ViewBorderItem());
	border_->SetColor(ColorView::k3D);
	scene()->addItem(border_.get());

	render_timer_ = new QTimer(this);
	render_timer_->setInterval(300);
	render_timer_->setSingleShot(true);
	connect(render_timer_, SIGNAL(timeout()), this, SLOT(slotSceneUpdate()));

	connect(m_pgVTOSTO, SIGNAL(sigSetPhoto()), this, SLOT(slotSetPhoto()));
	connect(m_pgVTOSTO, SIGNAL(sigChangeFaceAfterSurface()), this, SLOT(slotChangeFaceAfterSurface()));

	ApplyPreferences();
}

CW3View3DCeph::~CW3View3DCeph(void) {
	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(m_pFace);
	SAFE_DELETE_OBJECT(render_timer_);
	SAFE_DELETE_OBJECT(m_pLandmarkItem);
	SAFE_DELETE_OBJECT(m_pCurrTracingText);
}
#ifndef WILL3D_VIEWER
void CW3View3DCeph::exportProject(ProjectIOCeph& out) {
	using namespace surgery;

	out.SaveIsSurgeryCut(is_surgery_cut_[CutTypeID::MAXILLA],
						 is_surgery_cut_[CutTypeID::MANDIBLE],
						 is_surgery_cut_[CutTypeID::CHIN]);
	out.SaveIsSurgeryAdjust(is_surgery_adjust_[CutTypeID::MAXILLA],
							is_surgery_adjust_[CutTypeID::MANDIBLE],
							is_surgery_adjust_[CutTypeID::CHIN]);
	out.SaveIsSurgeryMove(is_surgery_move_[CutTypeID::MAXILLA],
						  is_surgery_move_[CutTypeID::MANDIBLE],
						  is_surgery_move_[CutTypeID::CHIN]);

	const int kCutItemCnt = CutTypeID::CUT_TYPE_END;
	out.SaveSurgeryCutItemCount(kCutItemCnt);
	for (int cut_item_id = 0; cut_item_id < kCutItemCnt; ++cut_item_id) {
		CW3SurfaceArchItem* cut_item = m_lstSurgeryCutItem[cut_item_id];
		if (!cut_item)
			continue;

		out.SaveSurgeryCutItemPoints(cut_item_id,
									 cut_item->getControlPoints());
		out.SaveSrugeryCutItemMatrix(cut_item_id,
									 cut_item->getTransformMat(UIGLObjects::TransformType::TRANSLATE),
									 cut_item->getTransformMat(UIGLObjects::TransformType::ROTATE),
									 cut_item->getTransformMat(UIGLObjects::TransformType::SCALE),
									 cut_item->getTransformMat(UIGLObjects::TransformType::ARCBALL),
									 cut_item->getTransformMat(UIGLObjects::TransformType::REORIENTATION));
	}

	if (m_pLandmarkItem)
		out.SaveLandmarks(m_pLandmarkItem->getPositions());
}

void CW3View3DCeph::importProject(ProjectIOCeph& in) {
	using namespace surgery;

	m_bLoadProject = true;

	in.LoadIsSurgeryCut(is_surgery_cut_[CutTypeID::MAXILLA],
						is_surgery_cut_[CutTypeID::MANDIBLE],
						is_surgery_cut_[CutTypeID::CHIN]);
	in.LoadIsSurgeryAdjust(is_surgery_adjust_[CutTypeID::MAXILLA],
						   is_surgery_adjust_[CutTypeID::MANDIBLE],
						   is_surgery_adjust_[CutTypeID::CHIN]);
	in.LoadIsSurgeryMove(is_surgery_move_[CutTypeID::MAXILLA],
						 is_surgery_move_[CutTypeID::MANDIBLE],
						 is_surgery_move_[CutTypeID::CHIN]);

	int cut_item_cnt;
	in.LoadSurgeryCutItemCount(cut_item_cnt);
	for (int cut_item_id = 0; cut_item_id < cut_item_cnt; ++cut_item_id) {
		std::vector<glm::vec3> points;
		in.LoadSurgeryCutItemPoints(cut_item_id, points);

		if (points.empty())
			continue;

		m_lstCtrlPoint.push_back(points);

		TransformMat transform;
		in.LoadSrugeryCutItemMatrix(cut_item_id, transform.translate,
									transform.rotate, transform.scale,
									transform.arcball, transform.reorien);
		m_lstTransform.push_back(transform);
	}

	in.LoadLandmarks(m_loadProjectLandmarks);

	if (m_loadProjectLandmarks.size() > 0) {
		m_pgDataManager->InitFromProjectFile(m_loadProjectLandmarks);
	}
}
#endif
void CW3View3DCeph::UpdateVR(bool is_high_quality) {
  m_isChanging = !is_high_quality;
  if (isVisible())
	this->scene()->update();
}
void CW3View3DCeph::setVisible(bool is_visible) {
	CW3View3D_thyoo::setVisible(is_visible);

	if (!m_pWorldAxisItem)
	{
		return;
	}
	
	m_pWorldAxisItem->setVisible(is_visible);

	if (is_visible)
	{
		m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
	}
}

void CW3View3DCeph::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) {
	CW3View3D_thyoo::SetCommonToolOnOff(type);

#if 0
	if (type >= common::CommonToolTypeOnOff::M_RULER && type < common::CommonToolTypeOnOff::M_DEL)
		common_tool_type_ = common::CommonToolTypeOnOff::NONE;
	else
		common_tool_type_ = type;
	measure_tools_->SetMeasureType(common_tool_type_);
#endif
}

void CW3View3DCeph::reset() {
	m_pgVTOSTO->reset();
	CW3View3D_thyoo::reset();

	m_bLoadProject = false;

	if (m_pGLWidget && m_pGLWidget->context())
		this->clearGL();

	SAFE_DELETE_OBJECT(m_pFace);
	SAFE_DELETE_OBJECT(m_pLandmarkItem);
	SAFE_DELETE_OBJECT(m_pReferPlaneItem);

	setRotateMatrix(kIMat);
	m_orienSave = kIMat;

	m_is3Dready = false;
	m_isDrawFinal = false;
	m_isFinishTracing = false;
	m_isFace = false;
	m_isNoEventMode = false;

	m_FBHandlerSG = 0;
	m_depthHandlerSG = 0;

	m_width3DviewSG = 0;
	m_height3DviewSG = 0;

	m_isMidSagittalMod = false;
	m_isSetCoordSys = false;
	m_isSurgery = false;

	for (int i = 0; i < CUT_TYPE_END; i++) {
		is_surgery_cut_[i] = false;
		is_surgery_adjust_[i] = false;
		is_surgery_move_[i] = false;
	}

	emit sigRenderCompleted();
}

void CW3View3DCeph::setVTO() {
	if (!m_is3Dready)
		return;

	if (!m_pgVTOSTO->flag.landmark && !m_pgVTOSTO->flag.cutFace
		&& !m_pgVTOSTO->flag.loadTRD) {
		return;
	} else if (!m_pgVTOSTO->flag.makeTetra &&
			   !m_pgVTOSTO->flag.generateHead &&
			   !m_pgVTOSTO->flag.fixedIsoValueInSurgery) {
	} else if (!m_pgVTOSTO->flag.makeTetra &&
			   !m_pgVTOSTO->flag.generateHead &&
			   m_pgVTOSTO->flag.fixedIsoValueInSurgery) {
		cout << m_pgVTOSTO->flag.makeTetra << " " << m_pgVTOSTO->flag.generateHead << " " << m_pgVTOSTO->flag.fixedIsoValueInSurgery << endl;

		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(m_pgVTOSTO, &CW3VTOSTO::genHeadAndMkTetra);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();

		this->makeMeshMove();
		this->calcDisps();
		this->makeSurf();
		this->makeField();
	} else if (!m_pgVTOSTO->flag.makeTetra && m_pgVTOSTO->flag.generateHead) {
		cout << m_pgVTOSTO->flag.makeTetra << " " << m_pgVTOSTO->flag.generateHead << " " << m_pgVTOSTO->flag.fixedIsoValueInSurgery << endl;

		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(m_pgVTOSTO, &CW3VTOSTO::makeTetra);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();

		this->makeMeshMove();
		this->calcDisps();
		this->makeSurf();
		this->makeField();
	} else if (m_pgVTOSTO->flag.makeTetra && !m_pgVTOSTO->flag.makeMeshMove) {
		this->makeMeshMove();
		this->calcDisps();
		this->makeSurf();
		this->makeField();
	} else if (m_pgVTOSTO->flag.makeMeshMove) {
		this->calcDisps();
		this->makeSurf();
		this->makeField();
	}
}

void CW3View3DCeph::setNoEventMode(bool isEnable) {
	if (!m_is3Dready)
		return;

	if (m_isNoEventMode != isEnable) {
		if (isEnable) {
			m_orienSave = m_reorienMat;
			m_reorienMat = kIMat;
		} else {
			m_reorienMat = m_orienSave;
			//this->setVTO(); // v1.0.2 3d surgery 수행하고 project 저장 후 불러왔을 때 face simulation tab에서 3d ceph tab으로 가면 3d face 초기화 되는 문제 수정
		}

		m_pFace->setTransformMat(m_reorienMat, REORIENTATION);
		m_isNoEventMode = isEnable;
	}
}

void CW3View3DCeph::forceRotateMatrix(const glm::mat4& mat) {
	if (!isVisible() || !m_is3Dready || !m_pGLWidget && !m_pGLWidget->context())
		return;

	if (m_arcMat != mat) {
		m_isChanging = true;
		m_arcMat = mat;
		m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
		this->scene()->update();
	}
}

void CW3View3DCeph::FacePhotoTransparencyChange(int value) {
	if (m_pFace == nullptr)
		return;

	float alpha = pow(float(value) / 100.0f, 0.5f);
	if (alpha > 1.0f)
		alpha = 1.0f;

	m_pFace->setAlpha(alpha);
	this->scene()->update();
}

void CW3View3DCeph::SetClipValues(const MPRClipID & clip_plane, const bool & is_clipping,
								  const int & lower, const int & upper) {
	initClipValues(clip_plane, is_clipping, lower, upper);

	m_isChanging = true;
	render_timer_->start();
	this->scene()->update();
}

void CW3View3DCeph::slotSetPhoto() {
	if (!isVisible() || !m_is3Dready || !m_pGLWidget && !m_pGLWidget->context())
		return;

	m_pGLWidget->makeCurrent();
	initPhotoGL();
	m_pGLWidget->doneCurrent();

	scene()->update();
}

//task는 tracingbar에서 활성화된 tracing.
void CW3View3DCeph::slotActiveTracingTask(const QString& task) {
	m_activeLandmarks.clear();

	m_tracingTask.name = task;
	m_activeTracingTask = task;

	//DB에서 tracing에 관한 landmark 정보들을 가져온다.
	CW3DBM::getInstance()->getTracingtaskRecordFromName(m_tracingTask);

	//tracing 볼륨 회전 설정.
	glm::mat3 rotMat;
	memcpy(&rotMat[0], m_tracingTask.rotateMatrix, sizeof(float) * 9);

	setRotateMatrix(mat4(rotMat));
	m_pWorldAxisItem->SetWorldAxisDirection(m_arcMat, m_view);
	emit sigRenderCompleted();

	//tracing clip parameter 설정.
	if (m_tracingTask.clippingPlane == MPRClipID::SAGITTAL) {
		if (m_isSetCoordSys && m_tracingTask.clippingPosition == 0.5f) {
			m_isMidSagittalMod = true;
		}
	}

	int lower, upper;
	if (m_tracingTask.clippingIsFlip) {
		lower = static_cast<int>(m_tracingTask.clippingPosition*100.0f);
		upper = 100;
	} else {
		lower = 0;
		upper = static_cast<int>(m_tracingTask.clippingPosition*100.0f);
	}

	emit sigSetClipToolValues(
		m_tracingTask.clippingIsOn, 
		false,
		m_tracingTask.clippingPlane, 
		lower, 
		upper
	);

	//활성화 된 landmark들, 찍어야하는 landmark이다.
	m_activeLandmarks = m_tracingTask.landmarks;

	//마우스를 따라다니는 landmark text 설정.
	m_pCurrTracingText->setPlainText(m_activeLandmarks.front());

	//ceph indicatorbar와 관련된 아이템들은 전부 안보이게 하고, 만약 tracing을 마친 다음에 수정하는 상태이면
	//이전에 찍은 랜드마크 아이템만 보이도록 한다.
	m_pLandmarkItem->setVisibleItems(false);
	m_pReferPlaneItem->setVisibleItems(false);

	for (auto& elem : m_pDistanceItem)
		elem.second->SetVisible(false);

	for (auto& elem : m_pAngleItem)
		elem.second->SetVisible(false);

	if (m_isFinishTracing) {
		for (const auto& elem : m_activeLandmarks) {
			m_pLandmarkItem->setVisibleItem(elem, true);
		}
	}

	if (!this->setTF(m_tracingTask.volumePreset))
		this->scene()->update();

	//tracingbar에 전달되어 랜드마크를 어디에 찍어야 하는지 보여주는 가이드 이미지를 띄운다.
	emit sigSetTracingGuideImage(m_activeLandmarks.front());

	m_isChanging = false;
}

void CW3View3DCeph::TracingTasksClear() {
	m_pgDataManager->clear();
	m_activeLandmarks.clear();
	m_pLandmarkItem->clear();
	m_pReferPlaneItem->deleteAllPlanes();
	m_isFinishTracing = false;
	m_pCurrTracingText->setVisible(false);
	m_isSetCoordSys = false;

	setRotateMatrix(kIMat);
	emit sigRenderCompleted();

	setReorientation(kIMat);

	m_tracingTask.clippingIsOn = false;

	emit sigSetClipToolValues(false, false, MPRClipID::AXIAL, 0, 100);
}

void CW3View3DCeph::TracingTasksFinished() {
	try {
		m_landmarkPos = m_pLandmarkItem->getPositions();

		for (const auto& elem : m_landmarkPos) {
			m_pgDataManager->addLandmark(elem.first, elem.second);
		}

		m_pgDataManager->endEditLandmark();

		if (!m_pgVTOSTO->isAvailableFace()) {
			m_pgVTOSTO->setFixedIsoValue(m_pgVREngine->getVol(0)->getAirTissueThreshold());
			m_pgVTOSTO->flag.landmark = true;
		}

		CW3DBM::getInstance()->setStudyLandmark(m_pgVREngine->getVol(0)->getHeader()->getStudyID(), m_landmarkPos);

		QStringList referPlanes;
		CW3DBM::getInstance()->getReferenceName(referPlanes);
		for (const auto& elem : referPlanes) {
			glm::vec4 plane = m_pgDataManager->getReferencePlane(elem);

			if (plane != vec4(0.0f))
				m_pReferPlaneItem->addPlane(elem, plane);
		}

		SetAdjustControlPoints();

		m_isFinishTracing = true;

		setRotateMatrix(kIMat);
		emit sigRenderCompleted();
		emit sigSetClipToolValues(false, false, MPRClipID::AXIAL, 0, 50);

		//ceph indicatorbar의 on/off와 동기화
		emit sigSyncCephIndicatorBar();

		this->setTF(common::otf_preset::BONE);
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::slotFinishedTracingTasks: " << e.what() << endl;
	}
}

void CW3View3DCeph::slotSetCoordSystem(const QStringList & tasks) {
	//좌표계 포인트.
	std::vector<glm::vec3> coordSysPoints;

	//현재까지 저장된 랜드마크 포인트.
	std::map<QString, glm::vec3> currLandmarkPos = m_pLandmarkItem->getPositions();

	//coordSysPoints에 task 순서에 맞게 좌표계 포인트를 넣는다.
	for (int i = 0; i < tasks.size(); i++) {
		TracingTaskInfo tracingTask;
		tracingTask.name = tasks[i];
		CW3DBM::getInstance()->getTracingtaskRecordFromName(tracingTask);
		coordSysPoints.push_back(currLandmarkPos[tracingTask.landmarks[0]]);
	}
	m_pgDataManager->setCoordinateSystem(coordSysPoints);

	//설정한 좌표계로 reorientation한다.
	this->setReorientation(glm::inverse(m_pgDataManager->getCoordSysMatrix()));

	//랜드마크를 다 안찍고 좌표계 랜드마크만 찍은 다음에 설정한 좌표계로 view를 확인하는 경우를 위해서
	//현재까지 찍은 랜드마크를 임시로 설정한다.
	for (const auto& elem : currLandmarkPos) {
		m_pgDataManager->addLandmark(elem.first, elem.second);
	}

	m_pgDataManager->endEditLandmark();

	//reference plane 설정.
	QStringList referPlanes;
	CW3DBM::getInstance()->getReferenceName(referPlanes);
	for (const auto& elem : referPlanes) {
		glm::vec4 plane = m_pgDataManager->getReferencePlane(elem);

		if (plane != vec4(0.0f))
			m_pReferPlaneItem->addPlane(elem, plane);
	}

	//emit sigSetClipToolValues(false, false, CLIP_AXIAL, 0, 50);

	m_isSetCoordSys = true;
	scene()->update();
}

void CW3View3DCeph::slotLandmarkChangeContentSwitch(const QString&, const QString& field, bool isEnable) {
	if (!m_is3Dready)
		return;

	m_pLandmarkItem->setVisibleItem(field, isEnable);
	scene()->update();
}

void CW3View3DCeph::slotMeasurementChangeContentSwitch(const QString&, const QString& field, bool isEnable) {
	if (!m_is3Dready)
		return;
	CW3CephDM::MEASURE_INFO info;
	m_pgDataManager->getMeasurementInfo(field, info);

	m_pGLWidget->makeCurrent();
	if (isEnable) {
		if (info.type == CW3CephDM::CephMeasureType::ANGLE) {
			if (m_pAngleItem.find(field) == m_pAngleItem.end()) {
				m_pAngleItem[field] = new CW3SurfaceAngleItem(this->scene());
				m_pAngleItem[field]->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
				m_pAngleItem[field]->setTransformMat(glm::scale(m_vVolRange), SCALE);
				m_pAngleItem[field]->setTransformMat(m_reorienMat, REORIENTATION);
				m_pAngleItem[field]->set_line_width(0.01f);
			}

			if (info.points.size() >= 3)
			{
				m_pAngleItem[field]->SetLabelPos(CW3SurfaceAngleItem::LabelPos::BACK);
				m_pAngleItem[field]->setProjViewMat(m_projection, m_view);
				m_pAngleItem[field]->setAnglePoints(info.points[0], info.points[1], info.points[2], info.val);
			}

		} else if (info.type == CW3CephDM::CephMeasureType::DISTANCE) {
			if (m_pDistanceItem.find(field) == m_pDistanceItem.end()) {
				m_pDistanceItem[field] = new CW3SurfaceDistanceItem(this->scene());
				m_pDistanceItem[field]->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
				m_pDistanceItem[field]->setTransformMat(glm::scale(m_vVolRange), SCALE);
				m_pDistanceItem[field]->setTransformMat(m_reorienMat, REORIENTATION);
				m_pDistanceItem[field]->set_line_width(0.01f);
			}

			m_pDistanceItem[field]->SetLabelPos(CW3SurfaceDistanceItem::LabelPos::BACK);
			m_pDistanceItem[field]->setProjViewMat(m_projection, m_view);
			m_pDistanceItem[field]->setDistancePoint(info.points, info.val);
		}
	} else {
		if (info.type == CW3CephDM::CephMeasureType::ANGLE) {
			if (m_pAngleItem.find(field) != m_pAngleItem.end()) {
				SAFE_DELETE_OBJECT(m_pAngleItem[field]);
				m_pAngleItem.erase(field);
			}
		} else if (info.type == CW3CephDM::CephMeasureType::DISTANCE) {
			if (m_pDistanceItem.find(field) != m_pDistanceItem.end()) {
				SAFE_DELETE_OBJECT(m_pDistanceItem[field]);
				m_pDistanceItem.erase(field);
			}
		}
	}
	m_pGLWidget->doneCurrent();
	scene()->update();
}

void CW3View3DCeph::slotReferenceChangeContentSwitch(const QString&, const QString& field, bool isEnable) {
	if (!m_is3Dready)
		return;
	m_pReferPlaneItem->setVisibleItem(field, isEnable);
	scene()->update();
}

void CW3View3DCeph::TracingTaskCancel() {
	//현재 활성화 된 landmark들을 비운다.
	m_activeLandmarks.clear();

	if (m_pCurrTracingText->isVisible())
		m_pCurrTracingText->setVisible(false);

	//볼륨 회전행렬을 정면으로
	setRotateMatrix(kIMat);
	emit sigRenderCompleted();

	//clipping 초기화
	emit sigSetClipToolValues(false, false, MPRClipID::AXIAL, 0, 50);

	//TF bone으로
	this->setTF(common::otf_preset::BONE);

	//ceph indicatorbar의 on/off와 동기화
	emit sigSyncCephIndicatorBar();

	QApplication::processEvents();

	scene()->update();
}

void CW3View3DCeph::setIsoValueRunThread(double value) {
	this->makeMeshMove();
	this->calcDisps();

	m_pgVTOSTO->setIsoValue(value);
	m_pgVTOSTO->generateHead();
	m_pgVTOSTO->makeTetra();
}

void CW3View3DCeph::FacePhotoEnable(int isEnable) {
	m_isFace = isEnable;
	this->scene()->update();
}

void CW3View3DCeph::initializeGL() {
	CW3View3D_thyoo::initializeGL();

	m_vVolRange = glm::vec3(m_pgVREngine->getVol(0)->width(),
							m_pgVREngine->getVol(0)->height(),
							m_pgVREngine->getVol(0)->depth());

	this->initItems();
	this->initItemModelScale();

	glUseProgram(m_PROGraycasting);

	m_passSurgeryRayCasting = glGetSubroutineIndex(m_PROGraycasting, GL_FRAGMENT_SHADER, "surgeryRayCasting");
	m_passBasicRayCasting = glGetSubroutineIndex(m_PROGraycasting, GL_FRAGMENT_SHADER, "basicRayCasting");

	if (m_pgDataManager->isTurnOn()) {
		try {
			if (m_pgVTOSTO->flag.doMapping || m_pgVTOSTO->flag.loadTRD) {
				initPhotoGL();
			}

			m_landmarkPos = m_pgDataManager->getVolumeLandmarks();

			for (const auto& elem : m_landmarkPos) {
				m_pLandmarkItem->addItem(elem.first, elem.second);
				m_pLandmarkItem->setVisibleItem(elem.first, false);
			}

			QStringList referPlanes;
			CW3DBM::getInstance()->getReferenceName(referPlanes);
			for (const auto& elem : referPlanes) {
				m_pReferPlaneItem->addPlane(elem, m_pgDataManager->getReferencePlane(elem));
				m_pReferPlaneItem->setVisibleItem(elem, false);
			}

			this->setReorientation(glm::inverse(m_pgDataManager->getCoordSysMatrix()));

			SetAdjustControlPoints();

			if (m_bLoadProject) {
				for (const auto& elem : m_loadProjectLandmarks)
					m_pLandmarkItem->editItem(elem.first, elem.second);

				m_pgDataManager->updateLandmarkPositions(m_loadProjectLandmarks);
			}

			m_isSetCoordSys = true;

			m_isFinishTracing = true;
		} catch (std::runtime_error& e) {
			cout << "CW3View3DCeph::initializeGL: " << e.what() << endl;
		}
	}

	for (int i = 0; i < kTexHandlerSG; i++) {
		m_texBufferSG.push_back(GL_COLOR_ATTACHMENT0 + i);
		m_texHandlerSG.push_back(0);
		m_texNumSG.push_back(GL_TEXTURE10 + TEX_END + i);
		m_texNum_SG.push_back(10 + TEX_END + i);
	}
}

void CW3View3DCeph::initPhotoGL(void) {
	const FacePhotoResource& pResFace = ResourceContainer::GetInstance()->GetFacePhotoResource();
	m_pFace->initSurfaceFillTexture(pResFace.points_after(), pResFace.tex_coords(), pResFace.indices());
}

void CW3View3DCeph::surgeryRayCasting(void) {
	//qDebug() << "start CW3View3D_thyoo::surgeryRayCasting";

	CW3GLFunctions::printError(__LINE__, "1 CW3View3DCeph::surgeryRayCasting");

	//recompile for test
	//m_pgVREngine->recompileRaycasting();
	//m_PROGfrontfaceCUBE = m_pgVREngine->getThyooPROGfrontface();
	//m_PROGfrontfaceFinal = m_pgVREngine->getThyooPROGforntfaceFinal();
	//m_PROGbackfaceCUBE = m_pgVREngine->getThyooPROGbackface();
	//m_PROGraycasting = m_pgVREngine->getThyooPROGRayCasting();
	//recompile for test end

	CW3VolumeRenderParam* pRenderParam = m_pgVREngine->getVRparams(0);

	m_pgVREngine->setVolTextureUniform(m_PROGraycasting, pRenderParam->m_texHandlerVol);
	m_pgVREngine->setTFTextureUniform();

	setMVP(m_rotAngle, m_rotAxis);

	int width = this->width();
	int height = this->height();

	glUseProgram(m_PROGraycasting);
	if (m_isChanging) {
		width *= low_res_frame_buffer_resize_factor_;
		height *= low_res_frame_buffer_resize_factor_;

		WGLSLprogram::setUniform(m_PROGraycasting, "StepSize", m_stepSize * low_res_step_size_factor_);
	} else {
		WGLSLprogram::setUniform(m_PROGraycasting, "StepSize", m_stepSize);
	}

	CW3GLFunctions::printError(__LINE__, "2 CW3View3DCeph::surgeryRayCasting");

	auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
	int window_width, window_level;
	if (m_pgVREngine->IsMIP()) {
		window_level = vol.windowCenter();
		window_width = vol.windowWidth();
	} else if (m_pgVREngine->IsXRAY()) {
#if 1
		window_level = (float)pRenderParam->getTissueBoneThreshold();
		window_width = vol.windowWidth();
#else
		window_level = vol.windowCenter() * 1.5f;
		window_width = vol.windowWidth() * 0.5f;
#endif
	} else {
		window_width = 65535;
		window_level = 32767;
	}

	WGLSLprogram::setUniform(m_PROGraycasting, "WindowLevel", (float)window_level);
	WGLSLprogram::setUniform(m_PROGraycasting, "WindowWidth", (float)window_width);

	CW3GLFunctions::printError(__LINE__, "3 CW3View3DCeph::surgeryRayCasting");

	//if (m_FBHandler == 0 || m_width3Dview != width || m_height3Dview != height) {
		CW3GLFunctions::initFrameBufferMultiTexture(m_FBHandler, m_depthHandler,
													m_texHandler, width, height, m_texNum);

		m_width3Dview = width;
		m_height3Dview = height;
	//}

	CW3GLFunctions::printError(__LINE__, "4 CW3View3DCeph::surgeryRayCasting");
	
	//if (m_FBHandlerSG == 0 || m_width3DviewSG != width || m_height3DviewSG != height) {
		CW3GLFunctions::initFrameBufferMultiTexture(m_FBHandlerSG, m_depthHandlerSG,
													m_texHandlerSG, width, height, m_texNumSG);

		m_width3DviewSG = width;
		m_height3DviewSG = height;
	//}

	CW3GLFunctions::printError(__LINE__, "5 CW3View3DCeph::surgeryRayCasting");
	glUseProgram(0);

	// TF must be set bone!
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//thyoo.
	//Phase 1: Subtracted volume start
	//볼륨에 surgery 영역을 뺀 Subtracted raycasting을 수행한다.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandlerSG);
	glViewport(0, 0, width, height);

	CW3GLFunctions::printError(__LINE__, "6 CW3View3DCeph::surgeryRayCasting");

	//qDebug() << "1 CW3View3D_thyoo::surgeryRayCasting";

#if 1
	////////// Front Face For Surgery Item
	glUseProgram(m_PROGfrontfaceCUBE);
	for (int i = 0; i < CUT_TYPE_END; i++) {
		if (is_surgery_cut_[i]) {
			glDrawBuffer(m_texBufferSG[TEX_MAXILLA_ENTRY_POSITION + i]);
			CW3GLFunctions::clearView(true, GL_BACK);
			m_lstSurgeryCutItem[i]->draw(m_PROGfrontfaceCUBE, GL_BACK, false);
		}
	}
	glUseProgram(0);

	CW3GLFunctions::printError(__LINE__, "7 CW3View3DCeph::surgeryRayCasting");
#endif

	//qDebug() << "2 CW3View3D_thyoo::surgeryRayCasting";

#if 1
	////////// Back Face For Surgery Item
	glUseProgram(m_PROGbackfaceCUBE);
	for (int i = 0; i < CUT_TYPE_END; i++) {
		if (is_surgery_cut_[i]) {
			glDrawBuffer(m_texBufferSG[TEX_MAXILLA_EXIT_POSITION + i]);
			CW3GLFunctions::clearView(true, GL_FRONT);
			m_lstSurgeryCutItem[i]->draw(m_PROGbackfaceCUBE, GL_FRONT, false);
		}
	}
	glUseProgram(0);

	CW3GLFunctions::printError(__LINE__, "8 CW3View3DCeph::surgeryRayCasting");
#endif

	//qDebug() << "3 CW3View3D_thyoo::surgeryRayCasting";

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandler);
	glViewport(0, 0, width, height);

	CW3GLFunctions::printError(__LINE__, "9 CW3View3DCeph::surgeryRayCasting");

	//////// Front Face For Volume
	glDrawBuffer(m_texBuffer[TEX_ENTRY_POSITION]);
	{
		glUseProgram(m_PROGfrontfaceCUBE);

		CW3GLFunctions::clearView(true, GL_BACK);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", m_mvp);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "VolTexTransformMat", kIMat);

#if kRenderActiveCube
		CW3GLFunctions::drawView(m_vaoCUBE, m_pgVREngine->GetActiveIndices(0), GL_BACK);
#else
		CW3GLFunctions::drawView(m_vaoCUBE, 36, GL_FRONT);
#endif
		glUseProgram(0);
	}

	//qDebug() << "4 CW3View3D_thyoo::surgeryRayCasting";

	CW3GLFunctions::printError(__LINE__, "10 CW3View3DCeph::surgeryRayCasting");

	////////// Back Face For Volume
	glDrawBuffer(m_texBuffer[TEX_EXIT_POSITION]);
	{
		glUseProgram(m_PROGbackfaceCUBE);

		CW3GLFunctions::clearView(true, GL_FRONT);
		WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", m_mvp);
		WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", kIMat);

#if kRenderActiveCube
		CW3GLFunctions::drawView(m_vaoCUBE, m_pgVREngine->GetActiveIndices(0), GL_FRONT);
#else
		CW3GLFunctions::drawView(m_vaoCUBE, 36, GL_FRONT);
#endif
		this->drawBackFaceSurface();

		glUseProgram(0);
	}

	//qDebug() << "5 CW3View3D_thyoo::surgeryRayCasting";

	CW3GLFunctions::printError(__LINE__, "11 CW3View3DCeph::surgeryRayCasting");

	//////// Extract Front Face	For Volume
	glDepthFunc(GL_LESS);

	CW3GLFunctions::printError(__LINE__, "12-1 CW3View3DCeph::surgeryRayCasting");
	//qDebug() << "5-1 CW3View3D_thyoo::surgeryRayCasting";

	GLenum textures[2] = { m_texBuffer[TEX_ENTRY_POSITION], m_texBuffer[TEX_EXIT_POSITION] };
	glClearDepth(1.0f);

	CW3GLFunctions::printError(__LINE__, "12-2 CW3View3DCeph::surgeryRayCasting");
	//qDebug() << "5-2 CW3View3D_thyoo::surgeryRayCasting";

	glDrawBuffers(2, textures);
	{
		CW3GLFunctions::printError(__LINE__, "12-3 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-3 CW3View3D_thyoo::surgeryRayCasting";

		glUseProgram(m_PROGfrontfaceFinal);

		CW3GLFunctions::printError(__LINE__, "12-4 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-4 CW3View3D_thyoo::surgeryRayCasting";

		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-5 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-5 CW3View3D_thyoo::surgeryRayCasting";

		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-6 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-6 CW3View3D_thyoo::surgeryRayCasting";

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-7 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-7 CW3View3D_thyoo::surgeryRayCasting";

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-8 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-8 CW3View3D_thyoo::surgeryRayCasting";

		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-9 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-9 CW3View3D_thyoo::surgeryRayCasting";

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);

		CW3GLFunctions::printError(__LINE__, "12-10 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-10 CW3View3D_thyoo::surgeryRayCasting";

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", m_clipParams.isEnable);

		CW3GLFunctions::printError(__LINE__, "12-11 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-11 CW3View3D_thyoo::surgeryRayCasting";

		for (int i = 0; i < m_clipParams.planes.size(); i++)
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), m_clipParams.planes[i]);

		CW3GLFunctions::printError(__LINE__, "12-12 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-12 CW3View3D_thyoo::surgeryRayCasting";

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)m_clipParams.planes.size());

		CW3GLFunctions::printError(__LINE__, "12-13 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-13 CW3View3D_thyoo::surgeryRayCasting";

		//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
		
		CW3GLFunctions::printError(__LINE__, "12-14 CW3View3DCeph::surgeryRayCasting");
		//qDebug() << "5-14 CW3View3D_thyoo::surgeryRayCasting";

		glUseProgram(0);
	}

	CW3GLFunctions::printError(__LINE__, "12-15 CW3View3DCeph::surgeryRayCasting");

	//qDebug() << "6 CW3View3D_thyoo::surgeryRayCasting";

	////////// Ray Casting
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_RAYCASTING]);
	glDrawBuffer(m_texBuffer[TEX_RAYCASTING]);
	{
		glUseProgram(m_PROGraycasting);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_passSurgeryRayCasting);

		int numTextures = 0;
		for (int i = 0; i < CUT_TYPE_END; i++) {
			if (is_surgery_cut_[i]) {
				glActiveTexture(m_texNumSG[TEX_MAXILLA_ENTRY_POSITION + i]);
				glBindTexture(GL_TEXTURE_2D, m_texHandlerSG[TEX_MAXILLA_ENTRY_POSITION + i]);
				WGLSLprogram::setUniform(m_PROGraycasting,
										 QString("SubEntryPositions[%1]").arg(numTextures).toStdString().c_str(),
										 m_texNum_SG[TEX_MAXILLA_ENTRY_POSITION + i]);

				glActiveTexture(m_texNumSG[TEX_MAXILLA_EXIT_POSITION + i]);
				glBindTexture(GL_TEXTURE_2D, m_texHandlerSG[TEX_MAXILLA_EXIT_POSITION + i]);
				WGLSLprogram::setUniform(m_PROGraycasting,
										 QString("SubExitPositions[%1]").arg(numTextures).toStdString().c_str(),
										 m_texNum_SG[TEX_MAXILLA_EXIT_POSITION + i]);

				++numTextures;
			}
		}

		WGLSLprogram::setUniform(m_PROGraycasting, "NumSubTextures", numTextures);

		CW3GLFunctions::clearView(true, GL_BACK);

		WGLSLprogram::setUniform(m_PROGraycasting, "BMVP", m_mvp*glm::inverse(pRenderParam->m_volTexBias));
		WGLSLprogram::setUniform(m_PROGraycasting, "isShade", m_pShadeSwitch->getCurrentState());
		WGLSLprogram::setUniform(m_PROGraycasting, "VolTexelSize", pRenderParam->m_volTexelSize);
		WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", m_isMIP);
		WGLSLprogram::setUniform(m_PROGraycasting, "MaxValue", pRenderParam->m_MaxValueForMIP);
		WGLSLprogram::setUniform(m_PROGraycasting, "InvVolTexScale", pRenderParam->m_invVolTexScale);

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);

		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);

		glUseProgram(0);
	}

	//qDebug() << "7 CW3View3D_thyoo::surgeryRayCasting";

	CW3GLFunctions::printError(__LINE__, "13 CW3View3DCeph::surgeryRayCasting");

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_DEFAULT]);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//thyoo.
	//Phase 2: separated volume start
	//지정한 surgery영역만 raycasting한다. depthbuffer에는
	//Phase1에서 수행한 raycasting의 depth가 저장된다. depth는 alpha color가 1.0이 되는 위치의 depth이며
	//Phase2에서 쓰여지는 depth와 비교하여 더 가까운 것이 framebuffer에 쓰여지게 된다.
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (int i = 0; i < CutTypeID::CUT_TYPE_END; i++) {
		if (!is_surgery_cut_[i])
			continue;

		if (i == MANDIBLE && is_surgery_cut_[CHIN]) {
			this->drawSubtractedMandibleByChin();
			continue;
		}

		////////// Front Face
		glDrawBuffer(m_texBuffer[TEX_ENTRY_POSITION]);
		{
			glUseProgram(m_PROGfrontfaceCUBE);
			CW3GLFunctions::clearView(true, GL_BACK);
			m_lstSurgeryCutItem[i]->draw(m_PROGfrontfaceCUBE, GL_BACK);
			glUseProgram(0);

			//qDebug() << "8 CW3View3D_thyoo::surgeryRayCasting";
		}

		////////// Back Face
		glDrawBuffer(m_texBuffer[TEX_EXIT_POSITION]);
		{
			glUseProgram(m_PROGbackfaceCUBE);
			CW3GLFunctions::clearView(true, GL_FRONT);
			m_lstSurgeryCutItem[i]->draw(m_PROGbackfaceCUBE, GL_FRONT);

			//qDebug() << "9 CW3View3D_thyoo::surgeryRayCasting";

			//this->drawBackFaceSurface();

			glDepthFunc(GL_LESS);

			for (int j = 0; j < CUT_TYPE_END; j++) {
				if (!is_surgery_cut_[j])
					continue;
				if (!is_surgery_adjust_[j])
					continue;

				m_lstSurgeryCutItem[j]->drawOutline(m_PROGbackfaceCUBE);
				m_lstSurgeryCutItem[j]->drawControl(m_PROGbackfaceCUBE, GL_BACK);
			}

			if (m_isFace  && m_pgVTOSTO->isAvailableFace() && !m_pFace->isTransparency()) {
				////////////////////////////////////////////////////////////////////////////////////////////////////////
				////thyoo.
				////separated volume의 backface에 surface를 그릴 때
				////backface는 월드좌표계에서 이동해도 고정된 volume texture를 가지므로
				////surface의 texture에는 이동한 행렬의 역행렬을 곱한다.
				////
				///////////////////////////////////////////////////////////////////////////////////////////////////////
				WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", glm::inverse(m_lstSurgeryCutItem[i]->getVolTexTransformMat()));
				m_pFace->draw(m_PROGbackfaceCUBE);

				//qDebug() << "10 CW3View3D_thyoo::surgeryRayCasting";
			}
			glUseProgram(0);
		}

		//////// Extract Front Face

		GLenum textures[2] = { m_texBuffer[TEX_ENTRY_POSITION], m_texBuffer[TEX_EXIT_POSITION] };
		glDrawBuffers(2, textures);
		{
			glUseProgram(m_PROGfrontfaceFinal);

			glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
			glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

			glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
			glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);

			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", m_clipParams.isEnable);
			for (int i = 0; i < m_clipParams.planes.size(); i++)
				WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), m_clipParams.planes[i]);

			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)m_clipParams.planes.size());

			//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
			CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);

			glUseProgram(0);

			//qDebug() << "11 CW3View3D_thyoo::surgeryRayCasting";
		}
		//glDrawBuffers(0, nullptr);

		////////// Ray Casting
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_RAYCASTING]);
		glDrawBuffer(m_texBuffer[TEX_RAYCASTING]);
		{
			glUseProgram(m_PROGraycasting);
			glDepthFunc(GL_LESS);

			glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_passBasicRayCasting);

			glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
			glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
			WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);
			glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
			glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
			WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

			WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", m_isMIP);
			WGLSLprogram::setUniform(m_PROGraycasting, "MaxValue", pRenderParam->m_MaxValueForMIP);

			WGLSLprogram::setUniform(m_PROGraycasting, "isFixedColor", true);
			WGLSLprogram::setUniform(m_PROGraycasting, "FixedColorIdx", i);

			//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
			CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);

			WGLSLprogram::setUniform(m_PROGraycasting, "isFixedColor", false);

			glUseProgram(0);

			//qDebug() << "12 CW3View3D_thyoo::surgeryRayCasting";
		}
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_DEFAULT]);
	}

	glUseProgram(0);

	CW3GLFunctions::printError(__LINE__, "14 CW3View3DCeph::surgeryRayCasting");

	//qDebug() << "end CW3View3D_thyoo::surgeryRayCasting";
}

void CW3View3DCeph::drawSubtractedMandibleByChin() {
	if (!is_surgery_cut_[CHIN] || !is_surgery_cut_[MANDIBLE])
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandlerSG);

	glm::mat4 tempRot = m_lstSurgeryCutItem[CHIN]->getTransformMat(ROTATE);
	glm::mat4 tempTrl = m_lstSurgeryCutItem[CHIN]->getTransformMat(TRANSLATE);

	m_lstSurgeryCutItem[CHIN]->setTransformMat(m_lstSurgeryCutItem[MANDIBLE]->getTransformMat(ROTATE), ROTATE);
	m_lstSurgeryCutItem[CHIN]->setTransformMat(m_lstSurgeryCutItem[MANDIBLE]->getTransformMat(TRANSLATE), TRANSLATE);

	////////// Front Face For Surgery Item
	glUseProgram(m_PROGfrontfaceCUBE);
	glDrawBuffer(m_texBufferSG[TEX_CHIN_ENTRY_POSITION]);
	CW3GLFunctions::clearView(true, GL_BACK);
	m_lstSurgeryCutItem[CHIN]->draw(m_PROGfrontfaceCUBE, GL_BACK);

	////////// Back Face For Surgery Item
	glUseProgram(m_PROGbackfaceCUBE);
	glDrawBuffer(m_texBufferSG[TEX_CHIN_EXIT_POSITION]);
	CW3GLFunctions::clearView(true, GL_FRONT);
	m_lstSurgeryCutItem[CHIN]->draw(m_PROGbackfaceCUBE, GL_FRONT);

	m_lstSurgeryCutItem[CHIN]->setTransformMat(tempRot, ROTATE);
	m_lstSurgeryCutItem[CHIN]->setTransformMat(tempTrl, TRANSLATE);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandler);

	//////// Front Face For Volume
	glDrawBuffer(m_texBuffer[TEX_ENTRY_POSITION]);
	{
		glUseProgram(m_PROGfrontfaceCUBE);
		CW3GLFunctions::clearView(true, GL_BACK);
		m_lstSurgeryCutItem[MANDIBLE]->draw(m_PROGfrontfaceCUBE, GL_BACK);
	}

	////////// Back Face For Volume
	glDrawBuffer(m_texBuffer[TEX_EXIT_POSITION]);
	{
		glUseProgram(m_PROGbackfaceCUBE);
		CW3GLFunctions::clearView(true, GL_FRONT);
		m_lstSurgeryCutItem[MANDIBLE]->draw(m_PROGbackfaceCUBE, GL_FRONT);

		glDepthFunc(GL_LESS);

		for (int i = 0; i < CUT_TYPE_END; i++) {
			if (!is_surgery_cut_[i] || !is_surgery_adjust_[i])
				continue;

			m_lstSurgeryCutItem[i]->drawOutline(m_PROGbackfaceCUBE);
			m_lstSurgeryCutItem[i]->drawControl(m_PROGbackfaceCUBE, GL_BACK);
		}

		if (m_isFace  && m_pgVTOSTO->isAvailableFace() && !m_pFace->isTransparency()) {
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			////thyoo.
			////separated volume의 backface에 surface를 그릴 때
			////backface는 월드좌표계에서 이동해도 고정된 volume texture를 가지므로
			////surface의 texture에는 이동한 행렬의 역행렬을 곱한다.
			////
			///////////////////////////////////////////////////////////////////////////////////////////////////////
			WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", glm::inverse(m_lstSurgeryCutItem[MANDIBLE]->getVolTexTransformMat()));
			m_pFace->draw(m_PROGbackfaceCUBE);
		}
	}

	//////// Extract Front Face
	GLenum textures[2] = { m_texBuffer[TEX_ENTRY_POSITION], m_texBuffer[TEX_EXIT_POSITION] };
	glDrawBuffers(2, textures);
	{
		glUseProgram(m_PROGfrontfaceFinal);

		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", m_clipParams.isEnable);
		for (int i = 0; i < m_clipParams.planes.size(); i++)
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), m_clipParams.planes[i]);

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)m_clipParams.planes.size());

		//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
	}

	////////// Ray Casting
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_RAYCASTING]);
	glDrawBuffer(m_texBuffer[TEX_RAYCASTING]);
	{
		glUseProgram(m_PROGraycasting);

		int numTextures = 1;
		glActiveTexture(m_texNumSG[TEX_CHIN_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandlerSG[TEX_CHIN_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting,
								 "SubEntryPositions[0]",
								 m_texNum_SG[TEX_CHIN_ENTRY_POSITION]);

		glActiveTexture(m_texNumSG[TEX_CHIN_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandlerSG[TEX_CHIN_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting,
								 "SubExitPositions[0]",
								 m_texNum_SG[TEX_CHIN_EXIT_POSITION]);

		WGLSLprogram::setUniform(m_PROGraycasting, "NumSubTextures", numTextures);

		glDepthFunc(GL_LESS);

		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_passSurgeryRayCasting);

		glActiveTexture(m_texNum[TEX_ENTRY_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_ENTRY_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNum_[TEX_ENTRY_POSITION]);
		glActiveTexture(m_texNum[TEX_EXIT_POSITION]);
		glBindTexture(GL_TEXTURE_2D, m_texHandler[TEX_EXIT_POSITION]);
		WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNum_[TEX_EXIT_POSITION]);

		WGLSLprogram::setUniform(m_PROGraycasting, "isFixedColor", true);
		WGLSLprogram::setUniform(m_PROGraycasting, "FixedColorIdx", MANDIBLE);

		//m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);
		CW3GLFunctions::drawView(m_vaoPlane, m_pgVREngine->vbo_plane_inverse_y()[2], 6, GL_BACK);
		WGLSLprogram::setUniform(m_PROGraycasting, "isFixedColor", false);
	}
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_DEFAULT]);

	//recovery chin Texture..
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandlerSG);
	glUseProgram(m_PROGfrontfaceCUBE);
	glDrawBuffer(m_texBufferSG[TEX_CHIN_ENTRY_POSITION]);
	CW3GLFunctions::clearView(true, GL_BACK);
	m_lstSurgeryCutItem[CHIN]->draw(m_PROGfrontfaceCUBE, GL_BACK, false);

	glUseProgram(m_PROGbackfaceCUBE);
	glDrawBuffer(m_texBufferSG[TEX_CHIN_EXIT_POSITION]);
	CW3GLFunctions::clearView(true, GL_FRONT);
	m_lstSurgeryCutItem[CHIN]->draw(m_PROGbackfaceCUBE, GL_FRONT, false);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandler);
}

void CW3View3DCeph::drawTransparencySurface() {
	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (m_isFace && m_pFace && m_pFace->isTransparency()) {
		if (m_pgVTOSTO->isAvailableFace()) {
			glUseProgram(m_PROGsurfaceTexture);

			glActiveTexture(m_texNumPlane);
			glBindTexture(GL_TEXTURE_2D, m_pgVTOSTO->getFaceTexHandler());
			WGLSLprogram::setUniform(m_PROGsurfaceTexture, "tex1", m_texNumPlane_);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_RAYCASTING]);
			{
				glColorMask(false, false, false, false);
				glDepthMask(false);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_STENCIL_TEST);
				glStencilMask(0xff);

				glDepthFunc(GL_LEQUAL);

				glClearStencil(0.0);
				glClear(GL_STENCIL_BUFFER_BIT);
				glStencilFunc(GL_ALWAYS, 0, 0xff);
				glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

				m_pFace->draw(m_PROGsurfaceTexture, GL_BACK);

				glColorMask(true, true, true, true);
				glDepthMask(true);
				glStencilFunc(GL_NOTEQUAL, 0, 0xff);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				m_pFace->draw(m_PROGsurfaceTexture, GL_BACK);

				glDisable(GL_STENCIL_TEST);
			}
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthHandler[DEPTH_DEFAULT]);
		}
	}

	if (m_isNoEventMode)
		return;
}

void CW3View3DCeph::drawSurface() {
	if (m_isFace && m_pFace && !m_pFace->isTransparency()) {
		if (m_pgVTOSTO->isAvailableFace()) {
			glUseProgram(m_PROGsurfaceTexture);

			glActiveTexture(m_texNumPlane);
			glBindTexture(GL_TEXTURE_2D, m_pgVTOSTO->getFaceTexHandler());
			WGLSLprogram::setUniform(m_PROGsurfaceTexture, "tex1", m_texNumPlane_);
			m_pFace->draw(m_PROGsurfaceTexture, GL_BACK);
			glUseProgram(0);
		}
	}

	if (m_isNoEventMode)
		return;

	for (int i = 0; i < CUT_TYPE_END; i++) {
		if (!is_surgery_cut_[i] || !is_surgery_adjust_[i])
			continue;

		glUseProgram(m_PROGsurface);

		m_lstSurgeryCutItem[i]->draw(m_PROGsurface);

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		m_lstSurgeryCutItem[i]->drawOutline(m_PROGsurface);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_SMOOTH);

		m_lstSurgeryCutItem[i]->drawControl(m_PROGsurface);

		glUseProgram(0);
	}

	glUseProgram(m_PROGsurface);

	m_pLandmarkItem->draw(m_PROGsurface);

	for (const auto& elem : m_pDistanceItem) {
		elem.second->draw(m_PROGsurface);
	}
	for (const auto& elem : m_pAngleItem) {
		elem.second->draw(m_PROGsurface);
	}

	glUseProgram(0);

#if 0
	for (int i = 0; i < CUT_TYPE_END; i++)
	{
		if (is_surgery_cut_[i] && isVisibleAxesItem(i))
		{
			glEnable(GL_DEPTH_TEST);

			//glClearDepth(1.0);
			//glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(m_PROGsurface);
			m_lstAxesItem[i]->draw(m_PROGsurface);
			glUseProgram(0);

			glDisable(GL_DEPTH_TEST);
		}
	}
#endif
}

void CW3View3DCeph::drawBackFaceSurface() {
	if (m_isFace && m_pFace && !m_pFace->isTransparency()) {
		if (m_pgVTOSTO->isAvailableFace()) {
			glUseProgram(m_PROGbackfaceCUBE);
			glDepthFunc(GL_LESS);
			WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "VolTexTransformMat", kIMat);
			m_pFace->draw(m_PROGbackfaceCUBE);
			glUseProgram(0);
		}
	}

	if (m_isNoEventMode)
		return;

	glUseProgram(m_PROGbackfaceCUBE);
	glDepthFunc(GL_LESS);

	for (int i = 0; i < CUT_TYPE_END; i++) {
		if (!is_surgery_cut_[i] || !is_surgery_adjust_[i])
			continue;

		m_lstSurgeryCutItem[i]->drawOutline(m_PROGbackfaceCUBE);
		m_lstSurgeryCutItem[i]->drawControl(m_PROGbackfaceCUBE, GL_BACK);
	}

	m_pLandmarkItem->draw(m_PROGbackfaceCUBE);

	for (const auto& elem : m_pDistanceItem) {
		elem.second->draw(m_PROGbackfaceCUBE);
	}
	for (const auto& elem : m_pAngleItem) {
		elem.second->draw(m_PROGbackfaceCUBE);
	}
	glUseProgram(0);
}

void CW3View3DCeph::renderingGL(void) {
	if (!this->isVisible())
		return;

	if (m_pgVREngine->isVRready()) {
		//qDebug() << "start CW3View3DCeph::renderingGL";

		CW3GLFunctions::printError(__LINE__, "start CW3View3DCeph::renderingGL");

		if (!m_is3Dready) {
			initializeGL();
			m_is3Dready = true;
		}

		if (!m_vaoPlane)
			m_pgVREngine->InitVAOPlaneInverseY(&m_vaoPlane);

#if kRenderActiveCube
		if (!m_vaoCUBE)
			m_pgVREngine->setActiveIndex(&m_vaoCUBE, 0);
#else
		if (!m_vaoCUBE) {
			unsigned int *vbo = m_pgVREngine->getVolVBO();
			CW3GLFunctions::initVAO(&m_vaoCUBE, vbo);
		}
#endif

		if ((m_pgVTOSTO->flag.loadTRD || m_pgVTOSTO->flag.doMapping) && !m_pFace->isReadyVAO()) {
			initPhotoGL();
		}

		if (m_isMidSagittalMod) {
			vec4 plane = m_pgDataManager->getReferencePlane("Mid-Sagittal Plane");
			plane = glm::scale(vec3(-1.0f, 1.0f, 1.0f))*plane;

			if (m_tracingTask.clippingIsFlip)
				m_clipParams.planes[0] = vec4(vec3(plane), plane.w);
			else
				m_clipParams.planes[1] = -vec4(vec3(plane), plane.w);

			m_isMidSagittalMod = false;
		}

		if (m_isDrawFinal) {
			CW3View3D_thyoo::drawFinal();
			m_isDrawFinal = false;
		} else {
			bool bClipRendering = false;

			for (int i = 0; i < CUT_TYPE_END; i++) {
				if (is_surgery_cut_[i]) {
					bClipRendering = true;
					break;
				}
			}

			if (!bClipRendering) {
				CW3View3D_thyoo::basicRayCasting();
			} else {
				this->surgeryRayCasting();
			}

			CW3View3D_thyoo::blendingGL();

			CW3View3D_thyoo::drawFinal();
		}
		
		if (measure_3d_manager_)
		{
			uint surface_program = m_pgVREngine->getPROGsurface();
			glUseProgram(surface_program);
			WGLSLprogram::setUniform(surface_program, "Light.Intensity", vec3(1.0f));
			vec4 lightPos = glm::scale(m_vVolRange)*vec4(0.0f, -10.0f, 0.0f, 1.0f);
			WGLSLprogram::setUniform(surface_program, "Light.Position",
				glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)) * lightPos);
			glUseProgram(0);

			measure_3d_manager_->set_reorientation_matrix(m_reorienMat);
			measure_3d_manager_->set_scale_matrix(m_model);
			measure_3d_manager_->set_rotate_matrix(m_arcMat);
			measure_3d_manager_->set_projection_matrix(m_projection);
			measure_3d_manager_->set_view_matrix(m_view);
			measure_3d_manager_->Draw(surface_program);
		}

		glUseProgram(0);

		CW3GLFunctions::printError(__LINE__, "end CW3View3DCeph::renderingGL");

		m_isChanging = false;
		m_rotAngle = 0.0f;

		//qDebug() << "end CW3View3DCeph::renderingGL";
	} else {
		CW3GLFunctions::clearView(true);
	}
}

void CW3View3DCeph::drawOverrideSurface() {
	//qDebug() << "start CW3View3DCeph::drawOverrideSurface";

	//if (m_pReferPlaneItem->isShown())
	{
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//glClearDepth(1.0);
		//glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(m_PROGsurface);
		m_pReferPlaneItem->draw(m_PROGsurface);
		glUseProgram(0);

		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}

	for (int i = 0; i < CUT_TYPE_END; i++) 
	{
		if (is_surgery_cut_[i] && isVisibleAxesItem(i))
		{
			glEnable(GL_DEPTH_TEST);

			//glClearDepth(1.0);
			//glClear(GL_DEPTH_BUFFER_BIT);

			glUseProgram(m_PROGsurface);
			m_lstAxesItem[i]->draw(m_PROGsurface);
			glUseProgram(0);

			glDisable(GL_DEPTH_TEST);
		}
	}

	//qDebug() << "end CW3View3DCeph::drawOverrideSurface";
}

void CW3View3DCeph::drawBackground(QPainter *painter, const QRectF &rect) {
	QGraphicsView::drawBackground(painter, rect);

	painter->beginNativePainting();
	this->renderingGL();
	painter->endNativePainting();
}

void CW3View3DCeph::resizeScene() {
	CW3View3D_thyoo::resizeScene();
	if (m_pLandmarkItem)
		m_pLandmarkItem->setSceneSizeInView(m_sceneWinView, m_sceneHinView);

	for (const auto& elem : m_pDistanceItem) {
		elem.second->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
	}
	for (const auto& elem : m_pAngleItem) {
		elem.second->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
	}
}

void CW3View3DCeph::setMVP(float rotAngle, glm::vec3 rotAxis) {
	CW3View3D_thyoo::setMVP(rotAngle, rotAxis);

	for (int i = 0; i < CUT_TYPE_END; i++) {
		m_lstAxesItem[i]->setTransformMat(m_arcMat, ARCBALL);
		m_lstSurgeryCutItem[i]->setTransformMat(m_arcMat, ARCBALL);

		m_lstAxesItem[i]->setProjViewMat(m_projection, m_view);
		m_lstSurgeryCutItem[i]->setProjViewMat(m_projection, m_view);
	}

	m_pFace->setTransformMat(m_arcMat, ARCBALL);
	m_pLandmarkItem->setTransformMat(m_arcMat, ARCBALL);
	m_pReferPlaneItem->setTransformMat(m_arcMat, ARCBALL);

	m_pFace->setProjViewMat(m_projection, m_view);
	m_pLandmarkItem->setProjViewMat(m_projection, m_view);
	m_pReferPlaneItem->setProjViewMat(m_projection, m_view);

	for (const auto& elem : m_pDistanceItem) {
		elem.second->setTransformMat(m_arcMat, ARCBALL);
		elem.second->setProjViewMat(m_projection, m_view);
	}
	for (const auto& elem : m_pAngleItem) {
		elem.second->setTransformMat(m_arcMat, ARCBALL);
		elem.second->setProjViewMat(m_projection, m_view);
	}
}

void CW3View3DCeph::setReorientation(const glm::mat4& reorienMat) {
	CW3View3D_thyoo::setReorientation(reorienMat);

	m_pLandmarkItem->setTransformMat(m_reorienMat, REORIENTATION);
	m_pReferPlaneItem->setTransformMat(m_reorienMat, REORIENTATION);
	m_pFace->setTransformMat(m_reorienMat, REORIENTATION);

	for (int i = 0; i < CUT_TYPE_END; i++) {
		m_lstSurgeryCutItem[i]->setTransformMat(m_reorienMat, REORIENTATION);
	}

	for (const auto& elem : m_pDistanceItem) {
		elem.second->setTransformMat(m_reorienMat, REORIENTATION);
	}
	for (const auto& elem : m_pAngleItem) {
		elem.second->setTransformMat(m_reorienMat, REORIENTATION);
	}
}

void CW3View3DCeph::clearGL() {
	if (m_pGLWidget) {
		m_pGLWidget->makeCurrent();
		for (auto elem : m_lstSurgeryCutItem) {
			if (elem)
				elem->clearVAOVBO();
		}

		for (auto elem : m_lstAxesItem) {
			if (elem)
				elem->clearVAOVBO();
		}
		if (m_pLandmarkItem)
			m_pLandmarkItem->clearVAOVBO();

		if (m_pReferPlaneItem)
			m_pReferPlaneItem->clearVAOVBO();

		for (const auto& item : m_pDistanceItem) {
			item.second->clearVAOVBO();
		}
		for (const auto& item : m_pAngleItem) {
			item.second->clearVAOVBO();
		}

		if (m_FBHandlerSG) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &m_FBHandlerSG);
			m_FBHandlerSG = 0;
		}
		if (m_depthHandlerSG) {
			glDeleteRenderbuffers(1, &m_depthHandlerSG);
			m_depthHandlerSG = 0;
		}
		if (m_texHandlerSG.size() > 0) {
			glDeleteTextures(m_texHandlerSG.size(), &m_texHandlerSG[0]);
			m_texHandlerSG.assign(m_texHandlerSG.size(), 0);
		}

		if (m_pFace)
			m_pFace->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	}

	CW3View3D_thyoo::clearGL();
}

void CW3View3DCeph::mousePressEvent(QMouseEvent* event)
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE	

	CW3View3D_thyoo::mousePressEvent(event);
}

void CW3View3DCeph::mouseMoveEvent(QMouseEvent *event) 
{
#ifdef WILL3D_EUROPE
	if (is_control_key_on_)
	{
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::mouseMoveEvent(event);

	if (measure_3d_manager_ && measure_3d_manager_->started())
	{
		m_isDrawFinal = true;
	}

	if (event->buttons() & Qt::LeftButton) {
		bool isUpdate = false;

		for (int i = 0; i < CUT_TYPE_END; i++) {
			if (!is_surgery_cut_[i])
				continue;

			if (isVisibleAxesItem(i)) {
				if (m_lstAxesItem[i]->isSelectTranslate()) {
					QPointF transPos = curr_scene_pos_ - last_scene_pos_;
					transPos -= (QPoint(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL));

					vec3 glTrans(transPos.x()*m_scaleSceneToGL / ((proj_right_ - proj_left_)*0.5f),
								 -transPos.y()*m_scaleSceneToGL / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);
					vec3 vTrans = m_lstAxesItem[i]->translate(glTrans);

					editSpinBoxMoveSurgery(vTrans, static_cast<CutTypeID>(i));

					isUpdate = true;
				} else if (m_lstAxesItem[i]->isSelectRotate()) {
					QPointF curPos = curr_scene_pos_;
					QPointF lastPos = last_scene_pos_;

					vec3 glCur((curPos.x() - m_pntCurViewCenterinScene.x())*m_scaleSceneToGL / ((proj_right_ - proj_left_)*0.5f),
							   -(curPos.y() - m_pntCurViewCenterinScene.y())*m_scaleSceneToGL / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);
					vec3 glLast = vec3((lastPos.x() - m_pntCurViewCenterinScene.x())*m_scaleSceneToGL / ((proj_right_ - proj_left_)*0.5f),
									   -(lastPos.y() - m_pntCurViewCenterinScene.y())*m_scaleSceneToGL / ((proj_top_ - proj_bottom_)*0.5f), 0.0f);

					QPair<float, vec3> pairRot = m_lstAxesItem[i]->rotate(glCur, glLast);
					editSpinBoxRotateSurgery(pairRot, static_cast<CutTypeID>(i));

					isUpdate = true;
				}
			}

			if (is_surgery_adjust_[i]) {
				if (m_lstSurgeryCutItem[i]->isPicking()) {
					QPointF transPos = curr_scene_pos_ - last_scene_pos_;
					transPos -= QPoint(m_WglTrans / m_scaleSceneToGL, m_HglTrans / m_scaleSceneToGL);

					vec3 glTrans(transPos.x()*m_scaleSceneToGL / (proj_right_ - proj_left_)*2.0f,
								 -transPos.y()*m_scaleSceneToGL / (proj_top_ - proj_bottom_)*2.0f, 0.0f);
					m_lstAxesItem[i]->translate(glTrans);
					m_lstSurgeryCutItem[i]->translateControl(glTrans);

					isUpdate = true;
				}
			}
		}

		if (isUpdate) {
			m_isChanging = true;
			this->scene()->update();
		}

		last_scene_pos_ = curr_scene_pos_;
	} else if (!(event->buttons() & Qt::RightButton)) {
		if (m_activeLandmarks.size()) {
			if (!m_pCurrTracingText->isVisible())
				m_pCurrTracingText->setVisible(true);
			QPointF scenePos = curr_scene_pos_;

			scenePos = QPointF(scenePos.x() + 20, scenePos.y());
			m_pCurrTracingText->setPos(scenePos);
			m_isDrawFinal = true;
			this->scene()->update();
		}

		bool isUpdateGL = false;

		for (int i = 0; i < CUT_TYPE_END; i++) {
			if (!is_surgery_cut_[i])
				continue;

			if (isVisibleAxesItem(i)) {
				m_pGLWidget->makeCurrent();
				glUseProgram(m_PROGpick);
				QOpenGLFramebufferObject fbo(this->width(), this->height(), QOpenGLFramebufferObject::Depth);
				fbo.bind();
				CW3GLFunctions::clearView(true, GL_BACK);
				glViewport(0, 0, this->width(), this->height());
				m_lstAxesItem[i]->render_for_pick(m_PROGpick);
				m_lstAxesItem[i]->pick(event->pos(), &isUpdateGL);

				fbo.release();
				glUseProgram(0);
				m_pGLWidget->doneCurrent();
			}

			if (is_surgery_adjust_[i]) {
				m_pGLWidget->makeCurrent();
				glUseProgram(m_PROGpick);
				QOpenGLFramebufferObject fbo(this->width(), this->height(), QOpenGLFramebufferObject::Depth);
				fbo.bind();
				CW3GLFunctions::clearView(true, GL_BACK);
				glViewport(0, 0, this->width(), this->height());
				m_lstSurgeryCutItem[i]->pick(event->pos(), &isUpdateGL, m_PROGpick);

				fbo.release();
				glUseProgram(0);
				m_pGLWidget->doneCurrent();
			}
		}

		if (isUpdateGL) {
			this->scene()->update();
		}
	}
}

void CW3View3DCeph::mouseReleaseEvent(QMouseEvent *event) 
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

	if (event->button() == Qt::LeftButton &&
		common_tool_type_ == common::CommonToolTypeOnOff::NONE) {
		if (m_activeLandmarks.size() != 0) {
			vec3 posVolume;

			if (volumeTracking(event->pos(), posVolume)) {
				QString activeLandmark = m_activeLandmarks.front();

				m_pLandmarkItem->addItem(activeLandmark, posVolume);

				if (m_pCurrTracingText->isVisible())
					m_pCurrTracingText->setVisible(false);

				m_isDrawFinal = false;

				this->scene()->update();
				QApplication::processEvents();

				if (m_activeLandmarks.size() == 1) {
					QThread::msleep(500);
					m_isDrawFinal = false;
					m_activeLandmarks.erase(m_activeLandmarks.begin());
					emit sigDoneTracingTask(m_activeTracingTask);
				} else if (m_activeLandmarks.size() > 1) {
					m_activeLandmarks.erase(m_activeLandmarks.begin());

					m_pCurrTracingText->setPlainText(m_activeLandmarks.front());
					emit sigSetTracingGuideImage(m_activeLandmarks.front());

					m_isDrawFinal = true;
				}
			}
		}
	}

	CW3View3D_thyoo::mouseReleaseEvent(event);

	this->scene()->update();
}

void CW3View3DCeph::wheelEvent(QWheelEvent * event) {
	if (m_activeLandmarks.size() && m_tracingTask.clippingIsOn) {
		float degrees = event->delta() / 12000.0f;

		m_tracingTask.clippingPosition += degrees;

		render_timer_->start();

		if (m_tracingTask.clippingIsFlip)
			emit sigClipLower(static_cast<int>(m_tracingTask.clippingPosition*100.0f));
		else
			emit sigClipUpper(static_cast<int>(m_tracingTask.clippingPosition*100.0f));
	}
	else
	{
		CW3View3D_thyoo::wheelEvent(event);
	}
}

void CW3View3DCeph::keyPressEvent(QKeyEvent * event) 
{
#ifdef WILL3D_EUROPE
	if (event->modifiers().testFlag(Qt::ControlModifier))
	{
		is_control_key_on_ = true;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	if (event->key() == Qt::Key_S) {
		if (event->modifiers() & Qt::ControlModifier) {
			if (m_pgVTOSTO && m_pgVTOSTO->flag.loadTRD) {
				printf("save modified TRD\r\n");

				QFileInfo info(m_pgVTOSTO->m_trdFilePath);
				QString modifiedTRDName = info.completeBaseName();

				QDate date = QDate::currentDate();
				QTime time = QTime::currentTime();
				QString year, month, day, hour, minute, second, msec;
				year.sprintf("%04d", date.year());
				month.sprintf("%02d", date.month());
				day.sprintf("%02d", date.day());
				hour.sprintf("%02d", time.hour());
				minute.sprintf("%02d", time.minute());
				second.sprintf("%02d", time.second());
				msec.sprintf("%03d", time.msec());
				modifiedTRDName =
					modifiedTRDName + "_" +
					year + month + day +
					hour + minute + second + msec;

				QString modifiedTRDPath = info.absolutePath() + "/" + modifiedTRDName;

#if 0
				m_pgVTOSTO->saveTRD(modifiedTRDPath + ".trd");
				m_pgVTOSTO->SavePLY(modifiedTRDPath + ".ply");
#else
				m_pgVTOSTO->saveTRD("./" + modifiedTRDName + ".trd");
				//m_pgVTOSTO->SavePLY("./" + modifiedTRDName + ".ply");
#endif
			}
		}
	}
	CW3View3D_thyoo::keyPressEvent(event);
}

void CW3View3DCeph::keyReleaseEvent(QKeyEvent* event)
{
#ifdef WILL3D_EUROPE
	if (event->key() == Qt::Key_Control)
	{
		is_control_key_on_ = false;
		emit sigSyncControlButton(is_control_key_on_);
		return;
	}
#endif // WILL3D_EUROPE

	CW3View3D_thyoo::keyReleaseEvent(event);
}

void CW3View3DCeph::resizeEvent(QResizeEvent * pEvent) {
	CW3View3D_thyoo::resizeEvent(pEvent);

	QPointF begin_idx = mapToScene(QPoint(0, 0));
	QPointF scene_size = mapToScene(QPoint(width(), height()));
	border_->SetRect(QRectF(begin_idx.x(), begin_idx.y(),
							scene_size.x(), scene_size.y()));
}

////////////////////////////////////////////////////////////////////////////////
//	thyoo.
//	surgery 안쪽 bone 영역에 해당하는 point는 outMovedIndices로 설정되고
//	surgery 바깥쪽 bone 영역은 outFixedIndices로 설정된다.
//	알고리즘은 surgery object의 frontface, backface를 얻고 backface와 frontface
//	사이에 있으면 inside point로, 아니면 outside point를 구분한다.
//	구분된 point 위치에 해당하는 alpha color값이 0이 아닐 때(=bone)
//	output indices로 리턴한다.
////////////////////////////////////////////////////////////////////////////////
void CW3View3DCeph::pointSelection() {
	bool isSurgery = false;

	for (const auto& elem : is_surgery_cut_) {
		if (elem) {
			isSurgery = true;
			break;
		}
	}

	if (!isSurgery || !m_is3Dready)
		return;

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(common::otf_preset::BONE);

	m_pGLWidget->makeCurrent();
	try {
		CW3VolumeRenderParam* pRenderParam = m_pgVREngine->getVRparams(0);

		int width = m_width3DviewSG;
		int height = m_height3DviewSG;

		glBindFramebuffer(GL_FRAMEBUFFER, m_FBHandlerSG);

		float** ffData = SAFE_ALLOC_VOLUME(float, CUT_TYPE_END, width*height * 4);
		float** bbData = SAFE_ALLOC_VOLUME(float, CUT_TYPE_END, width*height * 4);

		for (int i = 0; i < CUT_TYPE_END; i++) {
			memset(ffData[i], 0, sizeof(float)*width*height * 4);
			memset(bbData[i], 0, sizeof(float)*width*height * 4);

			if (is_surgery_cut_[i]) {
				glReadBuffer(m_texBufferSG[TEX_MAXILLA_ENTRY_POSITION + i]);
				glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, ffData[i]);

				glReadBuffer(m_texBufferSG[TEX_MAXILLA_EXIT_POSITION + i]);
				glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, bbData[i]);
			}
		}

		setMVP(m_rotAngle, m_rotAxis);

		std::vector<vec3> points = m_pgVTOSTO->m_modelPoints;

		int tfWidth, tfHeight;
		m_pgVREngine->getVolTFTexSize(&tfWidth, &tfHeight);

		const auto& res_tf = ResourceContainer::GetInstance()->GetTfResource();
		float* tfData = new float[tfWidth*tfHeight * 4];
		memcpy(tfData, res_tf.getTF(), sizeof(float)*tfWidth*tfHeight * 4);

		auto alphaFromTF = [&](const vec3& point)->float {
			unsigned short** volData = pRenderParam->m_pgVol->getData();

			int MaxTexSize = m_pgVREngine->getMaxTexAxisSize();

			vec3 vVolRange(pRenderParam->m_pgVol->width(),
						   pRenderParam->m_pgVol->height(),
						   pRenderParam->m_pgVol->depth());
			vec3 vVolMaxIdx(vVolRange.x - 1, vVolRange.y - 1, vVolRange.z - 1);

			vec3 volCoord = (point * 0.5f + 0.5f)*vVolRange;

			int ix = (volCoord.x < 0.0f) ? 0 : (volCoord.x > vVolMaxIdx.x) ? (int)vVolMaxIdx.x : (int)volCoord.x;
			int iy = (volCoord.y < 0.0f) ? 0 : (volCoord.y > vVolMaxIdx.y) ? (int)vVolMaxIdx.y : (int)volCoord.y;
			int iz = (volCoord.z < 0.0f) ? 0 : (volCoord.z > vVolMaxIdx.z) ? (int)vVolMaxIdx.z : (int)volCoord.z;
			int ixy = ix + iy * vVolRange.x;

			float intensity = volData[iz][ixy];
			vec2 tfIdx = vec2(intensity - (static_cast<int>(intensity / (float)MaxTexSize)*MaxTexSize),
							  static_cast<int>(intensity / MaxTexSize));

			int idx = static_cast<int>(tfIdx.x*4.0) +
				static_cast<int>(tfIdx.y*4.0*(float)tfWidth) + 3;

			return tfData[idx];
		};

		mat4 inverseX = glm::scale(vec3(-1.0f, 1.0f, 1.0f));

		mat4 bias = mat4(
			vec4(0.5f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.5f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.5f, 0.0f),
			vec4(0.5f, 0.5f, 0.5f, 1.0f));

		for (int j = 0; j < points.size(); j++) {
			vec3 point = points[j];

			vec4 glPoint = bias * m_mvp*vec4(point, 1.0);

			int ptX = (int)(glPoint.x*(float)width);
			int ptY = (int)(glPoint.y*(float)height);

			int ptIdx = width * 4 * ptY + ptX * 4;

			bool bFindInside = false;
			for (int i = CUT_TYPE_END - 1; i > -1; i--) {
				if (!is_surgery_cut_[i])
					continue;

				float ffDepth = ffData[i][ptIdx + 3];
				float bbDepth = bbData[i][ptIdx + 3];

				if (ffDepth <= glPoint.z && bbDepth >= glPoint.z) {
					vec3 invPoint = vec3(inverseX*vec4(point, 1.0f));
					if (alphaFromTF(invPoint) >= 0.15f) {
						m_jointMoveGroup[i].push_back(j);
					}

					bFindInside = true;
					break;
				}
			}

			if (!bFindInside) {
				vec3 invPoint = vec3(inverseX*vec4(point, 1.0f));
				if (alphaFromTF(invPoint) >= 0.15f) {
					m_jointFixed.push_back(j);
				}
			}
		}

		SAFE_DELETE_VOLUME(ffData, CUT_TYPE_END);
		SAFE_DELETE_VOLUME(bbData, CUT_TYPE_END);
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::pointSelection: " << e.what() << endl;
	}
	m_pGLWidget->doneCurrent();
}

void CW3View3DCeph::editSpinBoxMoveSurgery(const glm::vec3& translate, const CutTypeID& cutType) {
	CW3Image3D* vol = m_pgVREngine->getVol(0);
	const glm::vec3 spacing(vol->pixelSpacing(), vol->pixelSpacing(), vol->sliceSpacing());
	vec3 realTrans = -0.5f * translate * spacing;
	emit sigSurgeryTrans(cutType, realTrans);
}

void CW3View3DCeph::editSpinBoxRotateSurgery(const QPair<float, glm::vec3>& pairRot, const CutTypeID& cutType) {
	mat4 axes = m_lstAxesItem[cutType]->getTransformMat(ROTATE)*m_lstAxesItem[cutType]->getTransformMat(REORIENTATION);
	surgery::RotateID rotate_id;
	if (abs(glm::dot(vec3(axes[0]), pairRot.second)) >= 0.999f) {
		rotate_id = surgery::RotateID::SAG;
	} else if (abs(glm::dot(vec3(axes[1]), pairRot.second)) >= 0.999f) {
		rotate_id = surgery::RotateID::COR;
	} else if (abs(glm::dot(vec3(axes[2]), pairRot.second)) >= 0.999f) {
		rotate_id = surgery::RotateID::AXI;
	}
	float degree = -glm::degrees(pairRot.first);
	emit sigSurgeryRotate(cutType, rotate_id, degree);
}

void CW3View3DCeph::MeshMove() {
	m_pgVTOSTO->depChainMakeMeshMove();
	this->setVTO();
	this->scene()->update();
}

void CW3View3DCeph::initItems() {
	this->initFaceItem();
	this->initSurgeryItems();
	this->initReferenceItem();
	this->initLandmarkItem();
}

void CW3View3DCeph::setVisibleItems(bool isVisible) {}

void CW3View3DCeph::initFaceItem() {
	try {
		if (m_pFace != nullptr)
			throw std::runtime_error("m_pFace isn't nullptr");

		m_pFace = new CW3SurfaceItem();
		Material mater;
		mater.Ka = vec3(0.3f);
		mater.Kd = vec3(0.3f);
		mater.Ks = vec3(0.3f);
		mater.Shininess = 3.0f;
		m_pFace->setColor(mater);
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::initFaceItem: " << e.what() << endl;
	}
}

void CW3View3DCeph::initReferenceItem() {
	try {
		if (m_pReferPlaneItem != nullptr)
			throw std::runtime_error("m_pReferPlaneItem isn't nullptr");

		m_pReferPlaneItem = new CW3SurfacePlaneItem();
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::initReferenceItem: " << e.what() << endl;
	}
}

void CW3View3DCeph::initSurgeryItems() {
	for (int i = 0; i < CUT_TYPE_END; i++) {
		CW3SurfaceAxesItem* axes = new CW3SurfaceAxesItem();
		axes->setTransformMat(glm::rotate(glm::radians(180.0f), vec3(1.0f, 0.0f, 0.0f)), ROTATE);
		m_lstAxesItem[i] = axes;

		CW3SurfaceArchItem* surgeryCut = new CW3SurfaceArchItem();
		m_lstSurgeryCutItem[i] = surgeryCut;
	}
	//dummy points
	std::vector<glm::vec3> ctrlPoint;

	if (!is_surgery_cut_[MAXILLA]) {
		ctrlPoint.push_back(vec3(0.62f, -0.12f, 0.05f)); //p1
		ctrlPoint.push_back(vec3(0.0f, -0.65f, 0.13f)); //p2
		ctrlPoint.push_back(vec3(-0.62f, -0.12f, 0.05f)); //p3
		ctrlPoint.push_back(vec3(0.62f, -0.12f, 0.27f)); //p4
		ctrlPoint.push_back(vec3(0.0f, -0.65f, 0.45f)); //p5
		ctrlPoint.push_back(vec3(-0.62f, -0.12f, 0.27f)); //p6

		m_lstSurgeryCutItem[MAXILLA]->initializeArch(ctrlPoint);
	} else {
		m_lstSurgeryCutItem[MAXILLA]->initializeArch(m_lstCtrlPoint.at(MAXILLA));
	}

	ctrlPoint.clear();

	if (!is_surgery_cut_[MANDIBLE]) {
		ctrlPoint.push_back(vec3(0.59f, -0.12f, 0.21f)); //p1
		ctrlPoint.push_back(vec3(0.0f, -0.67f, 0.46f)); //p2
		ctrlPoint.push_back(vec3(-0.6f, -0.15f, 0.28f)); //p3
		ctrlPoint.push_back(vec3(0.58f, 0.0f, 0.64f)); //p4
		ctrlPoint.push_back(vec3(0.0f, -0.43f, 0.93f)); //p5
		ctrlPoint.push_back(vec3(-0.58, 0.0f, 0.64f)); //p6

		m_lstSurgeryCutItem[MANDIBLE]->initializeArch(ctrlPoint);
	} else {
		m_lstSurgeryCutItem[MANDIBLE]->initializeArch(m_lstCtrlPoint.at(MANDIBLE));
	}

	//TODO.. CHIN..

	//TODO CHIN END
}

void CW3View3DCeph::initLandmarkItem() {
	try {
		if (m_pLandmarkItem != nullptr)
			throw std::runtime_error("m_pLandmarkItem is nullptr");

		m_pLandmarkItem = new CW3SurfaceTextEllipseItem(this->scene());
		m_pLandmarkItem->setSceneSizeInView(m_sceneWinView, m_sceneHinView);
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::initLandmarkItem: " << e.what() << endl;
	}
}

void CW3View3DCeph::initItemModelScale() {
	for (int i = 0; i < CutTypeID::CUT_TYPE_END; i++) {
		m_lstAxesItem[i]->setTransformMat(glm::scale(m_vVolRange)*glm::scale(vec3(0.8f)), SCALE);
		m_lstAxesItem[i]->pushModelMat();
	}

	for (int i = 0; i < CutTypeID::CUT_TYPE_END; i++) {
		m_lstSurgeryCutItem[i]->setTransformMat(glm::scale(m_vVolRange), SCALE);
		m_lstSurgeryCutItem[i]->pushModelMat();

		if (m_bLoadProject && (CutTypeID::CUT_TYPE_END == m_lstTransform.size())) {
			m_lstSurgeryCutItem[i]->setTransformMat(m_lstTransform[i].translate, TRANSLATE);
			m_lstSurgeryCutItem[i]->setTransformMat(m_lstTransform[i].rotate, ROTATE);
			m_lstSurgeryCutItem[i]->setTransformMat(m_lstTransform[i].arcball, ARCBALL);
			m_lstSurgeryCutItem[i]->setTransformMat(m_lstTransform[i].reorien, REORIENTATION);
		}
	}
	if (m_pLandmarkItem)
		m_pLandmarkItem->setTransformMat(glm::scale(m_vVolRange), SCALE);

	if (m_pReferPlaneItem)
		m_pReferPlaneItem->setTransformMat(glm::scale(m_vVolRange), SCALE);

	if (m_pFace)
		m_pFace->setTransformMat(glm::scale(m_vVolRange), SCALE);
}

bool CW3View3DCeph::setTF(const QString & preset) {
	QString lPreset = preset.toLower();

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(lPreset);

	//if (m_pgVREngine->getOTFScene()->getCurrentPreset() == lPreset)
	//	return false;
	//
	//if (lPreset == "grayscale") {
	//	m_pgVREngine->getOTFScene()->setXRayTF(m_pgVREngine->getVol(0)->windowCenter() + m_pgVREngine->getVol(0)->windowWidth()*0.5f + m_pgVREngine->getVol(0)->intercept(),
	//										   m_pgVREngine->getVol(0)->windowCenter() - m_pgVREngine->getVol(0)->windowWidth()*0.5f + m_pgVREngine->getVol(0)->intercept());
	//} else {
	//	m_pgVREngine->getOTFScene()->setPreset(lPreset);
	//}

	this->scene()->update();

	return true;
}

void CW3View3DCeph::makeMeshMove() {
	m_jointMoveGroup.resize(CUT_TYPE_END);

	for (auto& elem : m_jointMoveGroup)
		elem.clear();

	m_jointFixed.clear();

	this->pointSelection();

	std::vector<int> jointIdxs;

	for (const auto& jointMove : m_jointMoveGroup) {
		for (const auto& elem : jointMove)
			jointIdxs.push_back(elem);
	}

	for (const auto& elem : m_jointFixed)
		jointIdxs.push_back(elem);

	m_pgVTOSTO->makeMeshMove(jointIdxs);
}

void CW3View3DCeph::calcDisps() {
	std::vector<glm::vec3> jointDisp;

	for (int i = 0; i < CUT_TYPE_END; i++) {
		if (is_surgery_cut_[i]) {
			mat4 invScale = glm::inverse(m_lstSurgeryCutItem[i]->getTransformMat(SCALE));
			mat4 trans = m_lstSurgeryCutItem[i]->getTransformMat(TRANSLATE);
			trans[3][0] *= invScale[0][0];
			trans[3][1] *= invScale[1][1];
			trans[3][2] *= invScale[2][2];

			mat4 rot = glm::translate(m_lstSurgeryCutItem[i]->getCentroid())*
				m_lstSurgeryCutItem[i]->getTransformMat(ROTATE)*
				glm::translate((-m_lstSurgeryCutItem[i]->getCentroid()));

			mat4 dispMat = trans * rot;

			for (const int& idx : m_jointMoveGroup[i]) {
				vec3 vDisp = vec3(dispMat*vec4(m_pgVTOSTO->m_modelPoints[idx], 1.0f)) - m_pgVTOSTO->m_modelPoints[idx];
				jointDisp.push_back(vDisp);
			}
		}
	}

	for (const int& elem : m_jointFixed)
		jointDisp.push_back(vec3(0.0));

	m_pgVTOSTO->calcDisp(jointDisp);
}

void CW3View3DCeph::makeSurf() {
	if (m_pgVTOSTO->flag.cutFace || m_pgVTOSTO->flag.loadTRD) {
		m_pgVTOSTO->makeSurf();
		m_pgVTOSTO->executeSurf();
	}
}

void CW3View3DCeph::makeField() {
	if (!m_pgVTOSTO->flag.landmark)
		return;

	m_pgVTOSTO->makeField();

	std::map<QString, vec3> landPoints = m_landmarkPos;
	std::vector<vec3> points;
	std::vector<QString> txtPoints;
	for (const auto& elem : landPoints) {
		points.push_back(elem.second);
		txtPoints.push_back(elem.first);
	}

	if (m_pgVTOSTO->executeField(points)) {
		for (int i = 0; i < points.size(); i++) {
			landPoints[txtPoints[i]] = points[i];
		}

		m_pLandmarkItem->editItems(landPoints);
		m_pgDataManager->updateLandmarkPositions(landPoints);
	}
}

bool CW3View3DCeph::isVisibleAxesItem(int index) {
	if (is_surgery_move_[index] && m_isSurgery && m_eViewType == common::ViewTypeID::CEPH) {
		for (const bool& is_cut : is_surgery_adjust_) {
			if (is_cut)
				return false;
		}
		return true;
	}
	return false;
}

void CW3View3DCeph::SetAdjustControlPoints() {
	try {
		std::map<QString, vec3> landmarks = m_landmarkPos;

		if (landmarks.find("PNS") == landmarks.end())
			throw std::runtime_error("tracing point \"PNS\" was not found.");
		if (landmarks.find("Ag_R") == landmarks.end())
			throw std::runtime_error("tracing point \"Ag_R\" was not found.");
		if (landmarks.find("Ag_L") == landmarks.end())
			throw std::runtime_error("tracing point \"Ag_L\" was not found.");
		if (landmarks.find("Ls") == landmarks.end())
			throw std::runtime_error("tracing point \"Ls\" was not found.");
		if (landmarks.find("Li") == landmarks.end())
			throw std::runtime_error("tracing point \"Li\" was not found.");
		if (landmarks.find("Soft_Pog") == landmarks.end())
			throw std::runtime_error("tracing point \"Soft_Pog\" was not found.");

		vec4 planeOC = m_pReferPlaneItem->getPlaneEquation("Occlusal Plane R");
		if (planeOC == vec4(0.0))
			throw std::runtime_error("refernece plane \"Occlusal Plnae R\" was not found.");

		vec4 planeMX = m_pReferPlaneItem->getPlaneEquation("Maxillary Plane");
		if (planeMX == vec4(0.0))
			throw std::runtime_error("refernece plane \"Maxillary Plane\" was not found.");

		vec4 planeMN = m_pReferPlaneItem->getPlaneEquation("Mandibular Plane");
		if (planeMN == vec4(0.0))
			throw std::runtime_error("refernece plane \"Mandibular Plane\" was not found.");

		vec4 planeMP = m_pReferPlaneItem->getPlaneEquation("B Perp MP");
		if (planeMP == vec4(0.0))
			throw std::runtime_error("refernece plane \"B Perp MP Plane\" was not found.");

		float offset = 1.2f;

		vec3 vAgR = (landmarks["Ag_R"] - landmarks["PNS"])*offset + landmarks["PNS"];
		vec3 vAgL = (landmarks["Ag_L"] - landmarks["PNS"])*offset + landmarks["PNS"];
		planeMN.w = planeMN.w*offset;

		auto proj = [](const glm::vec3& point, const glm::vec4& plane) {
			vec3 norm(plane.x, plane.y, plane.z);
			return point - (glm::dot(point, norm) - plane.w)*norm;
		};

		std::vector<glm::vec3> ctrlPoint;

		ctrlPoint.push_back(proj(proj(vAgR, planeOC), planeMX)); //p1
		ctrlPoint.push_back(proj(landmarks["Ls"], planeMX)); //p2
		ctrlPoint.push_back(proj(proj(vAgL, planeOC), planeMX)); //p3
		ctrlPoint.push_back(proj(vAgR, planeOC)); //p4
		ctrlPoint.push_back(proj(landmarks["Ls"], planeOC)); //p5
		ctrlPoint.push_back(proj(vAgL, planeOC)); //p6
		m_lstSurgeryCutItem[MAXILLA]->initializeArch(ctrlPoint);

		m_lstAxesItem[MAXILLA]->popModelMat();
		m_lstAxesItem[MAXILLA]->setTransformMat(glm::translate(m_vVolRange*((ctrlPoint[1] - ctrlPoint[4])*0.5f + ctrlPoint[4])), TRANSLATE);
		m_lstAxesItem[MAXILLA]->pushModelMat();

		m_lstAxesItem[MAXILLA]->editTransformMat(m_lstSurgeryCutItem[MAXILLA]->getTransformMat(TRANSLATE), TRANSLATE);
		m_lstAxesItem[MAXILLA]->editTransformMat(m_lstSurgeryCutItem[MAXILLA]->getTransformMat(ROTATE), ROTATE);

		ctrlPoint.clear();

		ctrlPoint.push_back(proj(vAgR, planeOC)); //p1
		ctrlPoint.push_back(proj(landmarks["Li"], planeOC)); //p2
		ctrlPoint.push_back(proj(vAgL, planeOC)); //p3
		ctrlPoint.push_back(proj(vAgR, planeMN)); //p4
		ctrlPoint.push_back(proj(landmarks["Soft_Pog"], planeMN)); //p5
		ctrlPoint.push_back(proj(vAgL, planeMN)); //p6
		m_lstSurgeryCutItem[MANDIBLE]->initializeArch(ctrlPoint);

		m_lstAxesItem[MANDIBLE]->popModelMat();
		m_lstAxesItem[MANDIBLE]->setTransformMat(glm::translate(m_vVolRange*((ctrlPoint[1] - ctrlPoint[4])*0.5f + ctrlPoint[4])), TRANSLATE);
		m_lstAxesItem[MANDIBLE]->pushModelMat();

		m_lstAxesItem[MANDIBLE]->editTransformMat(m_lstSurgeryCutItem[MANDIBLE]->getTransformMat(TRANSLATE), TRANSLATE);
		m_lstAxesItem[MANDIBLE]->editTransformMat(m_lstSurgeryCutItem[MANDIBLE]->getTransformMat(ROTATE), ROTATE);

		ctrlPoint[0] = proj((ctrlPoint[0] + ctrlPoint[3])*0.5f, planeMP);
		ctrlPoint[1] = (ctrlPoint[1] + ctrlPoint[4])*0.5f;
		ctrlPoint[2] = proj((ctrlPoint[2] + ctrlPoint[5])*0.5f, planeMP);
		ctrlPoint[3] = proj(ctrlPoint[3], planeMP);
		ctrlPoint[5] = proj(ctrlPoint[5], planeMP);
		m_lstSurgeryCutItem[CHIN]->initializeArch(ctrlPoint);

		m_lstAxesItem[CHIN]->popModelMat();
		m_lstAxesItem[CHIN]->setTransformMat(glm::translate(m_vVolRange*((ctrlPoint[1] - ctrlPoint[4])*0.5f + ctrlPoint[4])), TRANSLATE);
		m_lstAxesItem[CHIN]->pushModelMat();

		m_lstAxesItem[CHIN]->editTransformMat(m_lstSurgeryCutItem[CHIN]->getTransformMat(TRANSLATE), TRANSLATE);
		m_lstAxesItem[CHIN]->editTransformMat(m_lstSurgeryCutItem[CHIN]->getTransformMat(ROTATE), ROTATE);
	} catch (std::runtime_error& e) {
		cout << "CW3View3DCeph::SetAdjustControlPoints: " << e.what() << endl;
	}
}

void CW3View3DCeph::SurgeryEnable(const bool isEnable) {
	m_isSurgery = isEnable;

	if (m_isSurgery) { // v1.0.2 3d surgery를 수행 후 off 하고 face simulation tab으로 가면 after 비활성화 되는 문제를 수정
		m_pgVTOSTO->setFixedIsoValue(m_pgVREngine->getVol(0)->getAirTissueThreshold());
		m_pgVTOSTO->flag.landmark = true;

		if (m_is3Dready) {
			EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(common::otf_preset::BONE);
		}
	}

	this->scene()->update();

	if (!m_is3Dready)
		return;

	if (isEnable)
		this->setVTO();
}

void CW3View3DCeph::slotSurgeryEnableOn(const surgery::CutTypeID& cut_id,
										const bool isEnable) {
	if (!m_is3Dready)
		return;

	is_surgery_cut_[cut_id] = isEnable;

	if (isEnable) {

		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(common::otf_preset::BONE);

		QApplication::processEvents();
	}

	if (m_pgVTOSTO->flag.generateHead) {
		m_pgVTOSTO->depChainMakeTetra();
	}
	this->setVTO();
	this->clearGL();
}

void CW3View3DCeph::slotSurgeryAdjustOn(const surgery::CutTypeID& cut_id,
										const bool isAdjust) {
	if (!m_is3Dready)
		return;

	is_surgery_adjust_[cut_id] = isAdjust;

	if (is_surgery_adjust_[cut_id]) {
		emit sigSetZeroValuesSurgeryBar(cut_id);
		m_lstAxesItem[cut_id]->popModelMat();
		m_lstAxesItem[cut_id]->pushModelMat();

		m_lstSurgeryCutItem[cut_id]->popModelMat();
		m_lstSurgeryCutItem[cut_id]->pushModelMat();
	} else {
		m_pgVTOSTO->depChainMakeTetra();
		this->setVTO();
	}

	this->scene()->update();
}

void CW3View3DCeph::slotSurgeryMoveOn(const surgery::CutTypeID& cut_id,
									  const bool isAdjust) {
	if (!m_is3Dready)
		return;

	is_surgery_move_[cut_id] = isAdjust;
	this->scene()->update();
}

void CW3View3DCeph::slotSurgeryCutTranslate(const surgery::CutTypeID& cut_id,
											const glm::mat4& trans) {
	if (!m_is3Dready)
		return;

	CW3Image3D* vol = m_pgVREngine->getVol(0);
	vec3 spacing(vol->pixelSpacing(), vol->pixelSpacing(), vol->sliceSpacing());

	mat4 trans_mat = glm::translate(
		vec3(trans[3][0], trans[3][1], trans[3][2])*(2.0f / spacing));

	m_lstSurgeryCutItem[cut_id]->editTransformMat(trans_mat, TRANSLATE);

	MeshMove();
}

void CW3View3DCeph::slotSurgeryCutRotate(const surgery::CutTypeID & cut_id,
										 const glm::mat4 & rot) {
	if (!m_is3Dready)
		return;

	m_lstSurgeryCutItem[cut_id]->setTransformMat(rot, ROTATE);

	MeshMove();
}

void CW3View3DCeph::slotSurgeryAxisTranslate(const surgery::CutTypeID& cut_id,
											 const glm::mat4& trans) {
	if (!m_is3Dready)
		return;

	slotSurgeryCutTranslate(cut_id, trans);

	CW3Image3D* vol = m_pgVREngine->getVol(0);
	vec3 spacing(vol->pixelSpacing(), vol->pixelSpacing(), vol->sliceSpacing());
	mat4 trans_mat = glm::translate(vec3(trans[3][0], trans[3][1], trans[3][2])*(2.0f / spacing));
	m_lstAxesItem[cut_id]->editTransformMat(trans_mat, TRANSLATE);

	MeshMove();
}

void CW3View3DCeph::slotSurgeryAxisRotate(const surgery::CutTypeID& cut_id,
										  const glm::mat4& rot) {
	if (!m_is3Dready)
		return;

	slotSurgeryCutRotate(cut_id, rot);
	m_lstAxesItem[cut_id]->setTransformMat(m_lstSurgeryCutItem[cut_id]->getTransformMat(ROTATE)
											*m_lstAxesItem[cut_id]->getSaveTransformMat(ROTATE), ROTATE);

	MeshMove();
}

void CW3View3DCeph::slotChangeFaceAfterSurface() {
	common::Logger::instance()->Print(common::LogType::DBG, "slotChangeFaceAfterSurface() : start");

	if (!m_pGLWidget)
		return;

	if (!m_pgVTOSTO->isAvailableFace()) {
		m_pGLWidget->makeCurrent();
		m_pFace->clearVAOVBO();
		m_pGLWidget->doneCurrent();
	} else {
		m_pGLWidget->makeCurrent();

		const FacePhotoResource& pResFace = ResourceContainer::GetInstance()->GetFacePhotoResource();
		m_pFace->initSurfaceFillTexture(pResFace.points_after(), pResFace.tex_coords(), pResFace.indices());

		m_pGLWidget->doneCurrent();
	}

	this->scene()->update();

	common::Logger::instance()->Print(common::LogType::DBG, "slotChangeFaceAfterSurface() : end");
}

void CW3View3DCeph::slotSceneUpdate() {
	m_isChanging = false;
	this->scene()->update();
}
