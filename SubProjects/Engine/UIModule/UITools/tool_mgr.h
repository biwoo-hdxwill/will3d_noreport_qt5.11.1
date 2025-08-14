#pragma once
/**=================================================================================================

Project: 			UITools
File:				tool_mgr.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-09-12
Last modify:		2018-09-12

 *===============================================================================================**/
#include <vector>
#include <memory>
#include <mutex>

#include "uitools_global.h"

class QLabel;
class QLayout;
class QWidget;
class QVBoxLayout;
class CommonTaskTool;
class OTFTool;
class MPRTaskTool;
class PanoTaskTool;
class ImplantTaskTool;
class TMJTaskTool;
#ifndef WILL3D_LIGHT
class CephTaskTool;
class FaceTaskTool;
class SITaskTool;
class EndoTaskTool;
#endif

class UITOOLS_EXPORT ToolMgr {
public:
	static void SetInstance();
	static ToolMgr* instance();

public:
	typedef std::vector<QWidget*> ToolList;

public:
	void SetCommonTaskTool(const std::weak_ptr<CommonTaskTool>& tool);
	void SetOTFTaskTool(const std::weak_ptr<OTFTool>& tool);
	void SetMPRTaskTool(const std::weak_ptr<MPRTaskTool>& tool);
	void SetPanoTaskTool(const std::weak_ptr<PanoTaskTool>& tool);
	void SetImplantTaskTool(const std::weak_ptr<ImplantTaskTool>& tool);
	void SetTMJTaskTool(const std::weak_ptr<TMJTaskTool>& tool);
#ifndef WILL3D_LIGHT
	void SetCephTaskTool(const std::weak_ptr<CephTaskTool>& tool);
	void SetFaceTaskTool(const std::weak_ptr<FaceTaskTool>& tool);
	void SetSITaskTool(const std::weak_ptr<SITaskTool>& tool);
	void SetEndoTaskTool(const std::weak_ptr<EndoTaskTool>& tool);
#endif

	void RemoveTools(QVBoxLayout* layout);
	void SetVisibleTools(QVBoxLayout* layout, bool visible);

	QLayout* GetCommonTaskToolLayout() const;
	bool GetMPRTools(QVBoxLayout* layout);
	void GetPanoTools(QVBoxLayout* layout);
	void GetImplantTools(QVBoxLayout* layout);
	void GetTMJTools(QVBoxLayout* layout);
#ifndef WILL3D_LIGHT
	void GetCephTools(QVBoxLayout* layout);
	void GetSITools(QVBoxLayout* layout);
	void GetEndoTools(QVBoxLayout* layout);
	void GetFaceTools(QVBoxLayout* layout);
#endif

	void GetTMJ2DTaskLayout(QVBoxLayout* layout);
	void GetTMJ3DTaskLayout(QVBoxLayout* layout);

private:
	ToolMgr();
	ToolMgr(const ToolMgr&) = delete;
	ToolMgr& operator=(const ToolMgr&) = delete;

	QWidget* GetOTFToolBox() const;
	void AddTools(QVBoxLayout* layout, const ToolList& tool_list);

private:
	static std::unique_ptr<ToolMgr> instance_;
	static std::once_flag onceFlag_;

	std::weak_ptr<CommonTaskTool> common_task_tool_;
	std::weak_ptr<OTFTool> otf_task_tool_;
	std::weak_ptr<MPRTaskTool> mpr_task_tool_;
	std::weak_ptr<PanoTaskTool> pano_task_tool_;
	std::weak_ptr<ImplantTaskTool> implant_task_tool_;
	std::weak_ptr<TMJTaskTool> tmj_task_tool_;
#ifndef WILL3D_LIGHT
	std::weak_ptr<CephTaskTool> ceph_task_tool_;
	std::weak_ptr<FaceTaskTool> face_task_tool_;
	std::weak_ptr<SITaskTool> si_task_tool_;
	std::weak_ptr<EndoTaskTool> endo_task_tool_;
#endif

	static QLabel* empty_space_;
};
