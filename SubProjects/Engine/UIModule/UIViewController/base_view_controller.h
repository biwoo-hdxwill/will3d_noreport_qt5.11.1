#pragma once
/**=================================================================================================

Project: 			UIViewController
File:				base_view_controller
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-24
Last modify:		2017-07-24

 *===============================================================================================**/
#include <memory>
#include <QSize>
#include <qobject.h>

#include "uiviewcontroller_global.h"

#include "base_transform.h"

class ViewRenderParam;
class ViewPlaneObjGL;

namespace UIViewController
{
	struct _PACK_VIEW_INFO_PROJ;
}

class UIVIEWCONTROLLER_EXPORT BaseViewController 
{
public:
	explicit BaseViewController();
	virtual ~BaseViewController();

	BaseViewController(const BaseViewController&) = delete;
	BaseViewController& operator=(const BaseViewController&) = delete;

public:
	inline void set_view_param(const std::weak_ptr<ViewRenderParam>& param) { view_param_ = param; }

	void Initialize();

	virtual void ProcessViewEvent(bool *need_render) = 0;
	virtual void ClearGL() = 0;
	virtual void SetProjection() = 0;
	virtual bool IsReady() = 0;

	virtual void ApplyPreferences() = 0;

	virtual void SetFitMode(BaseTransform::FitMode mode) = 0;

protected:
	ViewRenderParam* view_param() const;
	ViewPlaneObjGL* plane_obj() const { return plane_obj_; }
	uint fb_handler() const;
	uint depth_handler(int index) const;
	uint tex_buffer(int index) const;
	uint tex_handler(int index) const;
	uint tex_num(int index) const;
	uint _tex_num(int index) const;

	void ReadyBufferHandles(int depthCount, int texCount);
	void ReadyFrameBuffer();

	UIViewController::_PACK_VIEW_INFO_PROJ GetPackViewProj() const;

	virtual bool SetRenderer() = 0;
	virtual void SetPackTexture() = 0;
	virtual void SetPackTextureHandle() = 0;
	virtual void ReadyBufferHandles() = 0;

	/* view events */
	void ScaleView();
	void ScaleWheelView();
	void PanView();
	void FitView();

	float GetMouseScale() const;
	float GetWheelScale() const;
	QPointF GetMouseTranslate() const;
	QPointF GetMouseDeltaWindowLevel() const;

	inline const bool& initialized() const { return initialized_; }

private:
	void SetBuffers();

private:
	uint fb_handler_ = 0;
	QSize fb_size_ = QSize(0, 0);

	std::vector<uint> depth_handler_;

	std::vector<uint> tex_buffer_;
	std::vector<uint> tex_handler_;
	std::vector<uint> tex_num_;
	std::vector<int> _tex_num_;

	std::weak_ptr<ViewRenderParam> view_param_;
	ViewPlaneObjGL* plane_obj_;

	bool initialized_ = false;
};
