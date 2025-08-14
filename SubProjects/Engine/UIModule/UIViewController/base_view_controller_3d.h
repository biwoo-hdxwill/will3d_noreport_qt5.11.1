#pragma once
/**=================================================================================================

Project: 			UIViewController
File:				base_view_controller_3d.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

 *===============================================================================================**/
#if defined(__APPLE__)
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec4.hpp>
#include <GL/glm/vec3.hpp>
#endif

#include "../../Common/GLfunctions/W3GLTypes.h"
#include "base_view_controller.h"

class VolumeRenderer;
class BaseTransform;
class SurfaceItems;

class UIVIEWCONTROLLER_EXPORT BaseViewController3D : public BaseViewController {
public:

	explicit BaseViewController3D();
	virtual ~BaseViewController3D();

	BaseViewController3D(const BaseViewController3D&) = delete;
	BaseViewController3D& operator=(const BaseViewController3D&) = delete;

public:
	virtual void SetCliping(const std::vector<glm::vec4>& planes, bool is_enable);

	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);

	glm::vec3 MapSceneToVol(const QPointF & pt_scene) const;
	void MapSceneToVol(const QPointF& src_scene_point, glm::vec3* dst_vol_point) const;
	inline QPointF MapVolToScene(const glm::vec3& src_vol_point) const;

	virtual void ClearGL() override;

	virtual void ProcessViewEvent(bool *need_render) override;
	virtual void SetProjection() override;
	virtual bool IsReady() override;

	void RenderingVolume();
	void RenderScreen(uint dfbo);

	virtual void RotateArcBall();

	void ForceRotateMatrix(const glm::mat4& mat);
	void SetReorienMatrix(const glm::mat4 & mat);

	glm::mat4 GetRotateMatrix();
	glm::mat4 GetCameraMatrix();
	const glm::mat4& GetViewMatrix();
	const glm::mat4& GetProjectionMatrix();
	const bool& GetInvertWindow() const;

	void GetWindowParams(float* window_width, float* window_level) const;
	float GetIntercept() const;
	/**=================================================================================================
	Volume tracking. Please call qopengl makecurrent.

	@param	pt_mouse	   	The view point mouse.
	@param	threshold_alpha	(Optional) The threshold alpha.

	@return	The volume position.
	 *===============================================================================================**/
	glm::vec3 VolumeTracking(const QPointF & pt_mouse, const float& threshold_alpha = 0.7f, bool is_print_log = true) const;

	virtual BaseTransform& transform() const = 0;

	virtual void ApplyPreferences() override;

protected:

	enum GL_TEXTURE_HANDLE {
		TEX_ENTRY_POSITION = 0,
		TEX_EXIT_POSITION,
		TEX_RAYCASTING,
		TEX_SCREEN,
		TEX_END
	};

	enum GL_DEPTH_HANDLE {
		DEPTH_DEFAULT = 0,
		DEPTH_RAYCASTING,
		DEPTH_END
	};

	virtual void InitVAOs();
	virtual VolumeRenderer& Renderer() const = 0;

	virtual void SetSurfaceMVP();
	virtual void DrawBackFaceSurface();
	virtual void DrawSurface();
	virtual void DrawTransparencySurface() { }
	virtual void DrawOverwriteSurface() { }

	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

	virtual void SetProjectionFitIn(float world_fit_width, float world_fit_height);
	virtual void RayCasting();

	void SetRayStepSize();

	void set_vao_vol(uint vao) { vao_vol_ = vao; }
	void set_indices_vol(uint indices) { indices_vol_ = indices; }

	uint vao_vol() const { return vao_vol_; }
	uint indices_vol() const { return indices_vol_; }
	void set_pack_clipping(const PackClipping& pack_clipping) { pack_clipping_ = pack_clipping; }
	const PackClipping& pack_clipping() const { return pack_clipping_; }

	inline SurfaceItems* surface_items() { return  surface_items_.get(); }

	const PackTexture& pack_front() const { return pack_front_; }
	const PackTexture& pack_back() const { return pack_back_; }
	const PackTexture& pack_raycasting() const { return pack_raycasting_; }
	const PackTexture& pack_screen() const { return pack_screen_; }

private:
	/**=================================================================================================
	level 0
	*===============================================================================================**/
	virtual bool SetRenderer() override;

	void BlendingGL();
	void ResetView();
	void LightView();

	/**=================================================================================================
	level 1
	*===============================================================================================**/
	glm::vec3 GetMouseVector(const QPointF& pt_scene);
	virtual glm::vec3 GetArcBallVector(const QPointF& pt_norm);

private:
	PackTexture pack_front_;
	PackTexture pack_back_;
	PackTexture pack_raycasting_;
	PackTexture pack_screen_;

	PackClipping pack_clipping_;

	std::unique_ptr<SurfaceItems> surface_items_;

	//thyoo 170802. TODO. CW3ObjectGL을 상속한 클래스를 만들것.
	uint vao_vol_ = 0;
	uint indices_vol_;

	bool is_light_invert_ = false;
};
