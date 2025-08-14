#pragma once
/*=========================================================================

File:			class CW3ENDOViewMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-07

=========================================================================*/
#include <memory>
#include <qwidget.h>

#include "../../Engine/Common/Common/W3Enum.h"
#include "../../Engine/Common/Common/define_view.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"

class CW3VREngine;
class CW3MPREngine;
class CW3JobMgr;
class CW3ResourceContainer;
class CW3View3DEndoSagittal;
class CW3View3DEndoSlice;
class CW3View3DEndo;
class CW3View3DEndoModify;
class CW3Spline3DItem;
class EndoTaskTool;
#ifndef WILL3D_VIEWER
class ProjectIOEndo;
#endif

class CW3ENDOViewMgr : public QWidget {
	Q_OBJECT
public:
	CW3ENDOViewMgr(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
				   CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer,
				   QWidget *parent = 0);

	~CW3ENDOViewMgr(void);

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOEndo& out);
	void importProject(ProjectIOEndo& in);
#endif

	void UpdateVRview(bool is_high_quality);

	void setVisibleViews(bool bVisible);
	void set_task_tool(const std::shared_ptr<EndoTaskTool>& task_tool);

	inline QWidget* getEndoSagittalView() { return (QWidget*)m_pView3DEndoSagital; }
	inline QWidget* getEndoSliceView() { return (QWidget*)m_pView3DEndoSlice; }
	inline QWidget* getEndoVRview() { return (QWidget*)m_pView3DENDO; }
	inline QWidget* getEndoModifyVRview() { return (QWidget*)m_pView3DEndoModify; }

	void reset();

	void ApplyPreferences();
	void DeleteMeasureUI(const common::ViewTypeID & view_type,
						 const unsigned int & measure_id);

	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

signals:
	void sigSelectPath(int);
	void sigDeletePath(int);
	void sigAirwayEnable(bool);
	void sigSetAirwaySize(double);
	void sigSetAirway(std::vector<tri_STL>&);
	void sigSpline3DEnd();

#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

public slots:
	void slotAirwayDisabled(bool isDraw);
	void slotSetAirwaySize(double size);
	void slotReoriUpdate(glm::mat4*);

	void slotSetPathNum(int path);

private slots:
	void slotTFupdated(bool);
	void slotTFupdateCompleted(void);
	void slotSetCameraPos(const int, const bool);
	void slotUpdate_3DENDO(void);
	void slotUpdateEndoPath(CW3Spline3DItem*, const bool);
	void slotSegAirway(std::vector<tri_STL>&);
	void slotVisiblePath(int);
	void slotUpdatePoint(int);

private:
	void connections();

private:
	CW3VREngine * m_pgVRengine = nullptr;
	std::shared_ptr<EndoTaskTool> task_tool_ = nullptr;

	CW3View3DEndoSagittal*	m_pView3DEndoSagital = nullptr;
	CW3View3DEndoSlice*		m_pView3DEndoSlice = nullptr;
	CW3View3DEndo*			m_pView3DENDO = nullptr;
	CW3View3DEndoModify*	m_pView3DEndoModify = nullptr;
};
