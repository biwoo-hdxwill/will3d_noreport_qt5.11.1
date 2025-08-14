#pragma once
/*=========================================================================

File:			class CW3SIViewMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-22
Last modify:	2016-04-22

=========================================================================*/
#include <qwidget.h>

#if defined(__APPLE__)
#include <glm/mat4x4.hpp>
#else
#include <gl/glm/mat4x4.hpp>
#endif

#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_view.h>

class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;
class CW3View3DSI;
class CW3ViewMPR;
#ifndef WILL3D_VIEWER
class ProjectIOSI;
#endif

class CW3SIViewMgr : public QWidget {
	Q_OBJECT
public:
	CW3SIViewMgr(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
				 CW3ResourceContainer *Rcontainer, QWidget *parent = 0);

	~CW3SIViewMgr();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOSI& out);
	void importProject(ProjectIOSI& in);
#endif

	void UpdateVRview(bool is_high_quality);

	void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on);
	void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type);

	inline QWidget* getView(const int i) { return (QWidget*)m_pViewMPR[i]; }
	inline QWidget* getViewSI() { return (QWidget*)m_pViewSI; }

	void reset();
	void activate();

	void drawSecond(const glm::mat4& mat);
	void setVisible(bool bVisible);

	void ApplyPreferences();
	void ResetMatrixToAuto();

	void DeleteMeasureUI(const common::ViewTypeID & view_type,
						 const unsigned int & measure_id);

	void VisibleMain(bool b);
	void VisibleSecond(bool b);
	void VisibleBoth(bool b);

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

signals:
	void sigSetTranslateMatSecondVolume(glm::mat4*);
	void sigSetRotateMatSecondVolume(glm::mat4*);
#ifdef WILL3D_EUROPE
	void sigShowButtonListDialog(const QPoint& global_pos, const int mpr_type = -1);
#endif // WILL3D_EUROPE

private slots:
	void slotReoriUpdate(glm::mat4*);
	void slotSecondUpdate(glm::mat4*);
	void slotWheel(MPRViewType, float);

private:
	void connections();

private:
	CW3ViewMPR*		m_pViewMPR[MPRViewType::MPR_END];
	CW3View3DSI*	m_pViewSI = nullptr;

	CW3MPREngine*	m_pgMPRengine = nullptr;
	CW3VREngine*	m_pgVRengine = nullptr;

	bool			m_isInitialized = false;
};
