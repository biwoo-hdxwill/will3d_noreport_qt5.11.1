#pragma once
/*=========================================================================

File:			class CW3FACEtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-12
Last date:		2016-01-12

=========================================================================*/
#include <memory>

#include "../../Engine/Common/Common/W3Enum.h"
#include "base_tab.h"

class QToolButton;
class CW3MPREngine;
class CW3VREngine;
class FaceTaskTool;
class CW3ResourceContainer;
class CW3VTOSTO;
class TabSlotLayout;
class CW3FACEViewMgr;
class WindowPlane;
#ifndef WILL3D_VIEWER
class ProjectIOFace;
#endif

class CW3FACEtab : public BaseTab
{
	Q_OBJECT
public:
	CW3FACEtab(CW3VREngine* VREngine, CW3MPREngine* MPRengine, CW3VTOSTO* vtosto,
		CW3ResourceContainer* Rcontainer);

	virtual ~CW3FACEtab(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOFace& out);
	void importProject(ProjectIOFace& in);
#endif

	void UpdateVRview(bool is_high_quality);

	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type,
		bool on) override;
	virtual void SetCommonToolOnOff(
		const common::CommonToolTypeOnOff& type) override;

	virtual void DeleteMeasureUI(const common::ViewTypeID& view_type,
		const unsigned int& measure_id) override;

signals:
	void sigChangeTab(TabType tabType);
	void sigCephNoEventMode();
	void sigRequestViewFaceAfter();

	void sigSave3DFaceToPLYFile();
	void sigSave3DFaceToOBJFile();

	void sigSetSoftTissueMin(const float value);

public:
	void ReleaseSurgeryViewFromLayout();
	virtual void SetLayout() override;
	void setViewFaceAfter(QWidget* widget);
	virtual void SetVisibleWindows(bool isVisible) override;
	void setTRDFromExternalProgram(const QString path, const bool onlyTRD);

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;

	void ApplyPreferences();
#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

private:
	virtual void Initialize() override;
	void connections();

	void TaskLoadPhoto();
	void TaskGenerateFace();
	void TaskClearMappingPoints();
	void TaskFaceMapping();
	void TaskBeforeAndAfter();

	void setVisibleViews(bool isVisible);
	bool setFaceLayout(QWidget* v1, QWidget* v2, QWidget* v3, QWidget* v4);
	bool setFaceLayout(QWidget* v1, QWidget* v2);

	void CreateGoToCephTabWidget();

private slots:
	void slotGoToCephTab();

	void slotFaceTask(const FaceTaskID& task_id);
	void slotVisibleFace(const VisibleID&, int);

private:
	CW3VREngine* m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3VTOSTO* m_pgVTOSTO = nullptr;
	CW3FACEViewMgr* m_pFACEViewMgr = nullptr;
	std::shared_ptr<FaceTaskTool> task_tool_ = nullptr;

	std::unique_ptr<WindowPlane> window_before_ = nullptr;
	std::unique_ptr<WindowPlane> window_after_ = nullptr;
	std::unique_ptr<WindowPlane> window_face_mesh_ = nullptr;
	std::unique_ptr<WindowPlane> window_face_photo_ = nullptr;

	QToolButton* go_to_ceph_tab_widget_ = nullptr;

	bool m_switchBeforeAfter = false;
};
