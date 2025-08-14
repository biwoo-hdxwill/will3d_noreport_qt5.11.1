#pragma once
/**=================================================================================================
Project: 			UIViewController
File:				view_controller_slice.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-01
Last modify:		2017-08-01
*===============================================================================================**/

#include "../../Common/GLfunctions/W3GLTypes.h"

#include "uiviewcontroller_global.h"
#include "transform_slice.h"
#include "base_view_controller.h"

class SliceRenderer;
class GLNerveWidget;
class GLImplantWidget;
class ImplantData;
class UIVIEWCONTROLLER_EXPORT ViewControllerSlice : public BaseViewController
{
public:
	ViewControllerSlice();
	~ViewControllerSlice();

	ViewControllerSlice(const ViewControllerSlice&) = delete;
	ViewControllerSlice& operator=(const ViewControllerSlice&) = delete;

public:
	/**=================================================================================================
	Sets a plane.

	Parameters:
	center_pos - 	   	The center position in volume coordinate.
	right_vector - 	   	The right vector. It's width of plane in volume coordinate and it isn't unit vector
	back_vector - 	   	The back vector. It's height of plane in volume coordinate and it isn't unit vector
	thickness_in_vol - 	The thickness in volume.
	 *===============================================================================================**/

	void SetPlane(const glm::vec3& center_pos, const glm::vec3& right_vector, const glm::vec3& back_vector, int thickness_in_vol);
	void SetSharpenLevel(const SharpenLevel & level);

	virtual void ClearGL() override;
	virtual void ProcessViewEvent(bool *need_render) override;
	virtual void SetProjection() override;
	virtual bool IsReady() override;
	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);

	void RenderingSlice();
	void RenderScreen(uint dfbo);

	void ForceRotateMatrix(const glm::mat4& mat);

	void MapSceneToVol(const std::vector<QPointF>& src_scene_points, std::vector<glm::vec3>& dst_vol_points);
	void MapVolToScene(const std::vector<glm::vec3>& src_vol_points, std::vector<QPointF>& dst_scene_points);

	inline glm::vec3 MapSceneToVol(const QPointF& pt_scene);
	inline QPointF MapVolToScene(const glm::vec3& pt_vol);
	inline QPointF MapPlaneToScene(const QPointF & pt_plane);
	inline QPointF MapSceneToPlane(const QPointF & pt_scene);

	void GetSliceRange(float& min, float& max) const;
	float GetLocationNose() const;
	float GetLocationChin() const;
	glm::vec3 GetUpVector() const;
	glm::vec3 GetRightVector() const;
	glm::vec3 GetBackVector() const;
	glm::mat4 GetCameraMatrix()const;
	glm::mat4 GetRotateMatrix() const;
	const glm::mat4& GetViewMatrix() const;
	const bool& GetInvertWindow() const;
	glm::vec4 GetDicomInfoPoint(const QPointF& pt_scene);
	void GetDicomHULine(const QPointF & start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data);
	void GetDicomHURect(const QPointF & start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data);

	void GetWindowParams(float* window_width, float* window_level);
	float GetIntercept() const;
	void RenderAndPickImplant(int* implant_id);

	const std::map<int, bool>& GetImplantVisibility();

	inline void ClearPlane() { is_set_plane_ = false; }
	inline const bool is_implant_wire() { return is_implant_wire_; }
	inline void set_is_implant_wire(bool wire) { is_implant_wire_ = wire; }
	inline TransformSlice* transform() { return transform_.get(); }

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;


	const std::vector<glm::vec3>& GetVolVertex() const;

	bool IsNerveVisible();
	bool IsImplantVisible();
	bool IsDrawSurface();

#ifndef WILL3D_VIEWER
	void InitOffScreenBuff(const int width, const int height);
	bool GetTextureData(unsigned char*& out_data, const int width, const int height, const int filter, const int thickness);
	bool GetTextureData(unsigned short*& out_data, const int width, const int height, const int filter, const int thickness);
	const int GetSharpenLevel();
#endif

private:
	virtual bool SetRenderer() override;
	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

	void SetSurfaceMVP();
	void RenderSlice();
	void DrawSurface();

	void ResetView();
	void LightView();

	SliceRenderer& Renderer() const;
	ViewPlaneObjGL* slice_obj() { return slice_obj_.get(); }

	ImplantData* GetImplantData();
	void DeleteOffScreenBuff();
	bool OffScreenDrawSlice(const int viewport_width, const int viewport_height, const int filter, const int thickness, bool draw_surface);

	void InitVAOs() {}
	void BlendingGL() {}

private:
	std::unique_ptr<TransformSlice> transform_;

	std::unique_ptr<ViewPlaneObjGL> slice_obj_;
	std::unique_ptr<GLNerveWidget> nerve_;
	std::unique_ptr<GLImplantWidget> implant_;

	PackTexture pack_screen_;
	PackTexture pack_pick_implant_;

	SharpenLevel sharpen_level_ = SharpenLevel::SHARPEN_OFF;

	bool is_update_slice_obj_ = false;
	bool is_set_plane_ = false;
	int slice_thickness_ = 1;

	bool is_implant_wire_ = false;

	enum GL_TEXTURE_HANDLE {
		TEX_SCREEN = 0,
		TEX_PICK_IMPLANT,
		TEX_END
	};

	enum GL_DEPTH_HANDLE {
		DEPTH_DEFAULT = 0,
		DEPTH_END
	};

	unsigned int off_fb_ = 0;
	unsigned int off_rb_ = 0;
	PackTexture off_texture_pack_;
};
