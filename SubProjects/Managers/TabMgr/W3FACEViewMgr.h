#pragma once
/*=========================================================================

File:			class CW3FACEViewMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Tae Hoon Yoo
First date:		2016-08-04
Last modify:	2016-08-04

=========================================================================*/
#include <memory>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <qwidget.h>

#include "../../Engine/Common/Common/W3Enum.h"
#include "../../Engine/Common/Common/define_view.h"

class GenerateFaceDlg;
class CW3MPREngine;
class CW3ResourceContainer;
class CW3View3DFaceMesh;
class CW3View3DFacePhoto;
class CW3VREngine;
class CW3VTOSTO;

class ViewFace;
#ifndef WILL3D_VIEWER
class ProjectIOFace;
#endif

class CW3FACEViewMgr : public QWidget {
	Q_OBJECT
public:
	CW3FACEViewMgr(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
		CW3VTOSTO* vtosto, CW3ResourceContainer* Rcontainer,
		QWidget* parent = 0);
	~CW3FACEViewMgr(void);

	void UpdateVRview(bool is_high_quality);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOFace& out);
	void importProject(ProjectIOFace& in);
#endif

	void setView3DFaceAfter(QWidget* viewCeph, bool is_visible_face);

	inline QWidget* getViewFaceBefore() {
		return (QWidget*)view_face_before_.get();
	}
	inline QWidget* getViewFaceAfter() { return (QWidget*)m_pgViewFaceAfter; }
	inline QWidget* getViewFaceMesh() { return (QWidget*)m_pViewFaceMesh; }
	inline QWidget* getViewFacePhoto() { return (QWidget*)m_pViewFacePhoto; }

	void reset();

	bool setTRDFromExternalProgram(const QString& path, const bool onlyTRD);

	bool LoadTRD(const QString& file_name);
	bool GenerateFace();
	void ClearMappingPoints();
	bool FaceMapping();
	void VisibleFace(int);

	void ApplyPreferences();
	void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);

	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

signals:
	void sigLoadPhoto(const QString&);
	void sigLoadTRD(const QString&);

	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();

	void sigSetSoftTissueMin(const float value);

#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

private:
	void connections();

#ifdef WILL3D_EUROPE
	void SetAllSyncControlButton(bool is_on);
#endif // WILL3D_EUROPE

private slots:
	void slotChangeGenFaceDlgValue(double);
	void slotRotateMatFromAfter(const glm::mat4& mat);
	void slotRotateMatFromBefore(const glm::mat4& mat);
	void slotRenderQualityFromAfter();

	void slotFaceChangeFaceTransparency(float);

	void slotThresholdEditingFinished();

private:
	CW3VREngine* m_pgVRengine = nullptr;

	CW3VTOSTO* m_pgVTOSTO = nullptr;

	QWidget* m_pgViewFaceAfter = nullptr;
	CW3View3DFaceMesh* m_pViewFaceMesh = nullptr;
	CW3View3DFacePhoto* m_pViewFacePhoto = nullptr;

	GenerateFaceDlg* m_pGenFaceDlg = nullptr;

	std::unique_ptr<ViewFace> view_face_before_;
};
