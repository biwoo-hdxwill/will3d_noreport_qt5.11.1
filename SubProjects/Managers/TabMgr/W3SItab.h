#pragma once
/*=========================================================================

File:			class CW3SItab
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2016-04-22
Last modify:	2016-04-22

=========================================================================*/
#include <memory>
#include <qboxlayout.h>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "base_tab.h"

#include "../../Engine/Common/Common/W3Enum.h"
class CW3SIViewMgr;
class SITaskTool;
class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;
class WindowPlane;
#ifndef WILL3D_VIEWER
class ProjectIOSI;
#endif

class CW3SItab : public BaseTab {
	Q_OBJECT
public:
	CW3SItab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
			 CW3ResourceContainer *Rcontainer);

	virtual ~CW3SItab();

	// serialization
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOSI& out);
	void importProject(ProjectIOSI& in);
#endif

	void UpdateVRview(bool is_high_quality);
	virtual void SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	inline glm::mat4 getSecondToFirst() const { return m_secondToFirst; };

	void setSecVolume(const glm::mat4& secondToFirst);
	void setSecVolume();

	virtual void SetVisibleWindows(const bool isVisible) override;
	void activate();

	QStringList GetViewList() override;
	QImage GetScreenshot(int view_type) override;
	QWidget* GetScreenshotSource(int view_type) override;

	virtual void DeleteMeasureUI(const common::ViewTypeID & view_type,
								 const unsigned int & measure_id) override;
	void ApplyPreferences();
#ifdef WILL3D_EUROPE
	virtual void SyncControlButtonOut() override;
#endif // WILL3D_EUROPE

signals:
	void sigLoadSecondVolume();
	void sigSetTranslateMatSecondVolume(glm::mat4 *);
	void sigSetRotateMatSecondVolume(glm::mat4 *);

private:
	virtual void SetLayout() override;
	virtual void Initialize() override;
	void connections();

private slots:
	void slotSITask(const SITaskID& task_id);
	void slotSIVisible(const SIVisibleID& visible_id, bool on);

private:
	CW3VREngine * m_pgVREngine = nullptr;
	CW3MPREngine* m_pgMPRengine = nullptr;
	CW3ResourceContainer* m_pgRcontainer = nullptr;
	CW3SIViewMgr*	m_pSIViewMgr = nullptr;
	std::shared_ptr<SITaskTool>	task_tool_ = nullptr;

	std::unique_ptr<WindowPlane> window_axial_ = nullptr;
	std::unique_ptr<WindowPlane> window_sagittal_ = nullptr;
	std::unique_ptr<WindowPlane> window_coronal_ = nullptr;
	std::unique_ptr<WindowPlane> window_vr_ = nullptr;

	QVBoxLayout*	m_pLayout = nullptr;

	glm::mat4 m_secondToFirst = glm::mat4();
};
