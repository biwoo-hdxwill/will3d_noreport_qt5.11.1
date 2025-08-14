#include "W3FACEViewMgr.h"
/*=========================================================================

File:			class CW3FACEViewMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-08-04
Last modify:	2016-08-04

=========================================================================*/
#include <QApplication>
#include <QtConcurrent/QtConCurrent>
#include <QDebug>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/W3ProgressDialog.h>
#include <Engine/Common/Common/define_otf.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/language_pack.h>

#include <Engine/Resource/ResContainer/W3ResourceContainer.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/W3TF.h>
#include <Engine/Resource/Resource/face_photo_resource.h>
#include <Engine/Resource/Resource/W3ImageHeader.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_face.h>
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include <Engine/UIModule/UIComponent/W3VTOSTO.h>
#include <Engine/UIModule/UIComponent/W3View3DCeph.h>
#include <Engine/UIModule/UIComponent/W3View3DFaceMesh.h>
#include <Engine/UIModule/UIComponent/W3View3DFacePhoto.h>
#include <Engine/UIModule/UIFrame/generate_face_dialog.h>

#include <Engine/Module/VREngine/W3VREngine.h>

#include <Engine/UIModule/UIComponent/view_face.h>
#include <Engine/UIModule/UIViewController/base_view_controller_3d.h>

using std::exception;
using namespace common;

CW3FACEViewMgr::CW3FACEViewMgr(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3VTOSTO* vtosto,
	CW3ResourceContainer* Rcontainer,
	QWidget* parent)
	: m_pgVRengine(VREngine), m_pgVTOSTO(vtosto) {
	m_pViewFaceMesh = new CW3View3DFaceMesh(m_pgVRengine, MPRengine, m_pgVTOSTO,
		common::ViewTypeID::FACE_SURFACE);
	m_pViewFacePhoto = new CW3View3DFacePhoto(m_pgVRengine, MPRengine, m_pgVTOSTO,
		common::ViewTypeID::FACE_PHOTO);
	view_face_before_.reset(new ViewFace());

	m_pGenFaceDlg = new GenerateFaceDlg();
	connections();
}

CW3FACEViewMgr::~CW3FACEViewMgr(void) {
	SAFE_DELETE_OBJECT(m_pViewFaceMesh);
	SAFE_DELETE_OBJECT(m_pViewFacePhoto);
}

void CW3FACEViewMgr::UpdateVRview(bool is_high_quality) {
	view_face_before_->UpdateVRview(is_high_quality);
}
#ifndef WILL3D_VIEWER
void CW3FACEViewMgr::exportProject(ProjectIOFace& out) {
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	QPointF trans_gl;

	out.InitFaceTab();
	view_face_before_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOFace::ViewType::FACE_BEFORE);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	out.InitializeView(ProjectIOFace::ViewType::PHOTO);
	m_pViewFacePhoto->exportProject(out.GetViewIO());
	// 저장할 게 아직 없음
	// out.Initialize(ProjectIOFace::ViewType::FACE_MESH);
	// m_pViewFaceMesh->exportProject(out.GetViewIO());
}

void CW3FACEViewMgr::importProject(ProjectIOFace& in) {
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	float trans_gl_x = 0.0f, trans_gl_y = 0.0f;

	in.InitializeView(ProjectIOFace::ViewType::FACE_BEFORE);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_face_before_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y));

	in.InitializeView(ProjectIOFace::ViewType::PHOTO);
	m_pViewFacePhoto->importProject(in.GetViewIO());

	// 저장할 게 아직 없음
	// in.Initialize(ProjectIOFace::ViewType::FACE_MESH);
	// m_pViewFaceMesh->importProject(in.GetViewIO());
}
#endif
void CW3FACEViewMgr::reset() {
	m_pViewFaceMesh->reset();
	m_pViewFacePhoto->reset();
}

void CW3FACEViewMgr::setView3DFaceAfter(QWidget* viewCeph, bool is_visible_face)
{
	if (m_pgViewFaceAfter == nullptr)
	{
		connect((CW3View3DCeph*)viewCeph, SIGNAL(sigRotateMat(glm::mat4)), this, SLOT(slotRotateMatFromAfter(glm::mat4)));
		connect((CW3View3DCeph*)viewCeph, SIGNAL(sigRenderCompleted()), this, SLOT(slotRenderQualityFromAfter()));
		connect(view_face_before_.get(), SIGNAL(sigRenderQuality()), (CW3View3DCeph*)viewCeph, SLOT(slotRenderCompleted()));
		((CW3View3DCeph*)viewCeph)->FacePhotoEnable(is_visible_face);
	}

	m_pgViewFaceAfter = viewCeph;
#ifdef WILL3D_EUROPE
	connect((CW3View3DCeph*)viewCeph, &CW3View3DCeph::sigShowButtonListDialog, this, &CW3FACEViewMgr::sigShowButtonListDialog);
	connect((CW3View3DCeph*)viewCeph, &CW3View3DCeph::sigSyncControlButton, [=](bool is_on) { SetAllSyncControlButton(is_on); });
#endif // WILL3D_EUROPE
}

void CW3FACEViewMgr::connections()
{
	connect(m_pGenFaceDlg, SIGNAL(sigChangeValue(double)), this, SLOT(slotChangeGenFaceDlgValue(double)));
	connect(m_pGenFaceDlg, SIGNAL(sigThresholdEditingFinished()), this, SLOT(slotThresholdEditingFinished()));
	connect(view_face_before_.get(), SIGNAL(sigRotateMat(glm::mat4)), this, SLOT(slotRotateMatFromBefore(glm::mat4)));
	connect(m_pgVRengine, SIGNAL(sigShadeOn(bool)), m_pViewFaceMesh, SLOT(slotShadeOnFromOTF(bool)));
	connect(view_face_before_.get(), SIGNAL(sigSave3DFaceToPLYFile()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(view_face_before_.get(), SIGNAL(sigSave3DFaceToOBJFile()), this, SIGNAL(sigSave3DFaceToOBJFile()));

#ifdef WILL3D_EUROPE
	connect(view_face_before_.get(), &ViewFace::sigShowButtonListDialog, this, &CW3FACEViewMgr::sigShowButtonListDialog);
	connect(m_pViewFacePhoto, &CW3View3DFacePhoto::sigShowButtonListDialog, this, &CW3FACEViewMgr::sigShowButtonListDialog);
	connect(m_pViewFaceMesh, &CW3View3DFaceMesh::sigShowButtonListDialog, this, &CW3FACEViewMgr::sigShowButtonListDialog);

	connect(view_face_before_.get(), &ViewFace::sigSyncControlButton, [=](bool is_on) { SetAllSyncControlButton(is_on); });
	connect(m_pViewFacePhoto, &CW3View3DFacePhoto::sigSyncControlButton, [=](bool is_on) { SetAllSyncControlButton(is_on); });
	connect(m_pViewFaceMesh, &CW3View3DFaceMesh::sigSyncControlButton, [=](bool is_on) { SetAllSyncControlButton(is_on); });
#endif // WILL3D_EUROPE
}

#ifdef WILL3D_EUROPE
void CW3FACEViewMgr::SetAllSyncControlButton(bool is_on)
{
	m_pViewFaceMesh->SetSyncControlButton(is_on);
	m_pViewFacePhoto->SetSyncControlButton(is_on);

	if (m_pgViewFaceAfter)
	{
		((CW3View3DCeph*)m_pgViewFaceAfter)->SetSyncControlButton(is_on);
	}
	view_face_before_.get()->SetSyncControlButton(is_on);
}
#endif // WILL3D_EUROPE

void CW3FACEViewMgr::VisibleFace(int state)
{
	if (m_pgVTOSTO->isAvailableFace())
	{
		view_face_before_->SetVisibleFacePhoto(state);

		if (m_pgViewFaceAfter)
		{
			((CW3View3DCeph*)m_pgViewFaceAfter)->FacePhotoEnable(state);
		}
	}
}


bool CW3FACEViewMgr::FaceMapping() {
	auto mapMeshCtrlPoints = m_pViewFaceMesh->getMeshCtrlPoints();
	auto mapMeshTriIdxs = m_pViewFaceMesh->getMeshTriangleIdxs();
	auto mapTexCtrlPoints = m_pViewFacePhoto->getTexCtrlPoints();

	std::vector<glm::vec3> meshCtrlPoints;
	std::vector<int> meshTriIdxs;
	std::vector<glm::vec2> texCtrlPoints;

	if ((mapMeshCtrlPoints.size() != mapTexCtrlPoints.size()) ||
		mapMeshCtrlPoints.empty() || mapTexCtrlPoints.empty())
	{
		CW3MessageBox msg_box(QString("Will3D"), lang::LanguagePack::msg_91(), CW3MessageBox::Critical);
		msg_box.exec();
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"CW3FACEViewMgr::slotSetFaceMapping: A "
			"picking point isn't matching."
		);
		return false;
	}

	for (const auto& elem : mapMeshCtrlPoints) {
		if (mapTexCtrlPoints.find(elem.first) == mapTexCtrlPoints.end()) {
			CW3MessageBox msgBox(QString("Will3D"), lang::LanguagePack::msg_91(), CW3MessageBox::Critical);
			msgBox.exec();
			common::Logger::instance()->Print(common::LogType::ERR,
				"CW3FACEViewMgr::slotSetFaceMapping: A "
				"picking point isn't matching.");
			return false;
		}
		else {
			meshCtrlPoints.push_back(mapMeshCtrlPoints[elem.first]);
			meshTriIdxs.push_back(mapMeshTriIdxs[elem.first]);
			texCtrlPoints.push_back(mapTexCtrlPoints[elem.first]);
		}
	}

	for (const auto& elem : mapTexCtrlPoints) {
		if (mapMeshCtrlPoints.find(elem.first) == mapMeshCtrlPoints.end()) {
			CW3MessageBox msgBox(QString("Will3D"), lang::LanguagePack::msg_91(), CW3MessageBox::Critical);
			msgBox.exec();
			common::Logger::instance()->Print(common::LogType::ERR,
				"CW3FACEViewMgr::slotSetFaceMapping: A "
				"picking point isn't matching.");
			return false;
		}
	}

	if (meshCtrlPoints.size() <= 0) {
		common::Logger::instance()->Print(
			common::LogType::ERR, "CW3FACEViewMgr::slotSetFaceMapping: zero.");
		return false;
	}

	CW3ProgressDialog* progress =
		CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	auto future = QtConcurrent::run(m_pgVTOSTO, &CW3VTOSTO::doMappingRunThread,
		meshCtrlPoints, meshTriIdxs, texCtrlPoints);

	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	// m_pgVTOSTO->doMappingRunThread(meshCtrlPoints, meshTriIdxs, texCtrlPoints);
	return m_pgVTOSTO->doMappingFinal(m_pViewFacePhoto->getPhotoImage());
}

bool CW3FACEViewMgr::LoadTRD(const QString& file_name) {
	FacePhotoResource* face_resource = ResourceContainer::GetInstance()->res_face();
	face_resource->clear();

	bool load_success = false;
	view_face_before_->SetVisibleFacePhoto(false);
	view_face_before_->LoadFace3D();

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
		common::otf_preset::SOFTTISSUE2);
	QApplication::processEvents();
	view_face_before_->UpdateVRview(true);

	int tf_min_value =
		ResourceContainer::GetInstance()->GetTfResource().min_value();
	m_pGenFaceDlg->SetThreshold((double)tf_min_value);

	if (m_pGenFaceDlg->exec()) {
		emit sigSetSoftTissueMin(m_pGenFaceDlg->threshold());
		m_pgVTOSTO->setFixedIsoValue(m_pGenFaceDlg->threshold());
		m_pgVTOSTO->slotLoadTRD(file_name);

		m_pViewFaceMesh->clearFace();
		m_pViewFaceMesh->clearPoints();
		m_pViewFacePhoto->clearPhoto();
		m_pViewFacePhoto->clearPoints();
		view_face_before_->SetVisibleFacePhoto(true);

		load_success = true;
	}

	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
		common::otf_preset::BONE);
	tf_min_value = ResourceContainer::GetInstance()->GetTfResource().min_value();
	m_pGenFaceDlg->SetThreshold((double)tf_min_value);
	QApplication::processEvents();

	view_face_before_->UpdateVRview(true);
	emit m_pgVRengine->sigTFupdateCompleted();
	return load_success;
}
void CW3FACEViewMgr::slotRenderQualityFromAfter() {
	view_face_before_->UpdateVRview(true);
}
void CW3FACEViewMgr::slotChangeGenFaceDlgValue(double value) {
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigMoveTFpolygon(
		value);
}

void CW3FACEViewMgr::slotRotateMatFromAfter(const glm::mat4& mat) {
	view_face_before_->ForceRotateMatrix(mat);
}

void CW3FACEViewMgr::slotRotateMatFromBefore(const glm::mat4& mat) {
	if (m_pgViewFaceAfter != nullptr)
		((CW3View3DCeph*)m_pgViewFaceAfter)->forceRotateMatrix(mat);
}

bool CW3FACEViewMgr::GenerateFace() {
	bool generate_success = false;
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
		common::otf_preset::SOFTTISSUE2);

	QApplication::processEvents();
	view_face_before_->UpdateVRview(true);

	int tf_min_value =
		ResourceContainer::GetInstance()->GetTfResource().min_value();
	m_pGenFaceDlg->SetThreshold((double)tf_min_value);

	if (m_pGenFaceDlg->exec()) {
		emit sigSetSoftTissueMin(m_pGenFaceDlg->threshold());
		m_pViewFaceMesh->generateFaceMesh(m_pGenFaceDlg->threshold());

		m_pViewFaceMesh->clearPoints();
		m_pViewFacePhoto->clearPoints();
		m_pgVTOSTO->flag.loadTRD = false;
		generate_success = true;
	}

	return generate_success;
}

void CW3FACEViewMgr::ClearMappingPoints() {
	m_pViewFaceMesh->clearPoints();
	m_pViewFacePhoto->clearPoints();
}

// load trd from willmaster
bool CW3FACEViewMgr::setTRDFromExternalProgram(const QString& path,
	const bool onlyTRD) {
	bool mesh_set = false;
	auto trdLoad = [&](bool& mesh_set) {
		m_pgVTOSTO->m_trdFilePath = path;
		if (m_pgVTOSTO->m_trdFilePath.length() > 0) {
			if (onlyTRD)
				m_pgVTOSTO->slotLoadOnlyTRD(m_pgVTOSTO->m_trdFilePath);
			else
				m_pgVTOSTO->slotLoadTRD(m_pgVTOSTO->m_trdFilePath);

			mesh_set = true;
		}
	};

	if (onlyTRD) {
		m_pgVTOSTO->setFixedIsoValue(100.0f);
		trdLoad(mesh_set);
	}
	else {
		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
			common::otf_preset::SOFTTISSUE2);
		QApplication::processEvents();
		view_face_before_->UpdateVRview(true);

		int tf_min_value =
			ResourceContainer::GetInstance()->GetTfResource().min_value();
		m_pGenFaceDlg->SetThreshold((double)tf_min_value);

		if (m_pGenFaceDlg->exec()) {
			emit sigSetSoftTissueMin(m_pGenFaceDlg->threshold());
			m_pgVTOSTO->setFixedIsoValue(m_pGenFaceDlg->threshold());
			trdLoad(mesh_set);
		}

		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigSetTFpreset(
			common::otf_preset::BONE);
		QApplication::processEvents();

		tf_min_value =
			ResourceContainer::GetInstance()->GetTfResource().min_value();
		m_pGenFaceDlg->SetThreshold((double)tf_min_value);

		view_face_before_->UpdateVRview(true);
		emit m_pgVRengine->sigTFupdateCompleted();
	}

	return mesh_set;
}

void CW3FACEViewMgr::slotFaceChangeFaceTransparency(float alpha) {
	view_face_before_->SetTransparencyFacePhoto(alpha);
	if (m_pgViewFaceAfter)
		((CW3View3DCeph*)m_pgViewFaceAfter)->FacePhotoTransparencyChange(alpha*100.0f);
}

void CW3FACEViewMgr::ApplyPreferences() {
	if (m_pgViewFaceAfter)
		((CW3View3DCeph*)m_pgViewFaceAfter)->ApplyPreferences();
	m_pViewFaceMesh->ApplyPreferences();
	m_pViewFacePhoto->ApplyPreferences();
	view_face_before_.get()->ApplyPreferences();
}

void CW3FACEViewMgr::DeleteMeasureUI(const common::ViewTypeID& view_type,
	const unsigned int& measure_id) {
	switch (view_type) {
	case common::ViewTypeID::FACE_SURFACE:
		m_pViewFaceMesh->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::FACE_PHOTO:
		m_pViewFacePhoto->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::FACE_BEFORE:
		view_face_before_->SyncDeleteMeasureUI(measure_id);
		break;
	}
}

void CW3FACEViewMgr::slotThresholdEditingFinished()
{
	view_face_before_->SetRenderModeQuality(true);
}

void CW3FACEViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on)
{
	m_pViewFaceMesh->SetCommonToolOnce(type, on);
	m_pViewFacePhoto->SetCommonToolOnce(type, on);
	view_face_before_->SetCommonToolOnce(type, on);
}

void CW3FACEViewMgr::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	m_pViewFaceMesh->SetCommonToolOnOff(type);
	m_pViewFacePhoto->SetCommonToolOnOff(type);
	view_face_before_->SetCommonToolOnOff(type);
}

#ifdef WILL3D_EUROPE
void CW3FACEViewMgr::SetSyncControlButtonOut()
{
	m_pViewFaceMesh->SetSyncControlButton(false);
	m_pViewFacePhoto->SetSyncControlButton(false);
	view_face_before_.get()->SetSyncControlButton(false);
	if (m_pgViewFaceAfter)
	{
		((CW3View3DCeph*)m_pgViewFaceAfter)->SetSyncControlButton(false);
	}
}
#endif // WILL3D_EUROPE
