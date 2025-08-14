#pragma once
/**=================================================================================================
File:				pacs_view_controller.h
Language:			C++11
Library:			Qt 5.9.9
author:				lim Tae Kuin
First date:			2021-06-05
Last modify:		2021-07-01
 *===============================================================================================**/

#include "../../Common/GLfunctions/W3GLTypes.h"

#include "base_view_controller.h"

class SliceRenderer;
class TransformSlice;
class GLNerveWidget;
class GLImplantWidget;
class UIVIEWCONTROLLER_EXPORT PACSViewController : public BaseViewController
{
public:
	PACSViewController();
	~PACSViewController();

	PACSViewController(const PACSViewController&) = delete;
	PACSViewController& operator=(const PACSViewController&) = delete;

	/**=================================================================================================
	Sets a plane.

	Parameters:
	center_pos - 	   	The center position in volume coordinate.
	right_vector - 	   	The right vector. It's width of plane in volume coordinate and it isn't unit vector
	back_vector - 	   	The back vector. It's height of plane in volume coordinate and it isn't unit vector
	thickness_in_vol - 	The thickness in volume.
	 *===============================================================================================**/

	void SetPlane(const glm::vec3 & center_pos, const glm::vec3 & right_vector, const glm::vec3 & back_vector, int thickness_in_vol);

	virtual void ClearGL() override;
	virtual void SetProjection() override;
	virtual bool IsReady() override;

	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);

	void RenderingSlice();
	void RenderScreen(uint dfbo);

	glm::mat4 GetRotateMatrix() const;
	const glm::mat4& GetViewMatrix() const;

	inline TransformSlice* transform() { return transform_.get(); }
	inline void ClearPlane() { is_set_plane_ = false; }

	inline const SharpenLevel& sharpen_level() { return sharpen_level_; }
	inline void set_sharpen_level(const SharpenLevel& level) { sharpen_level_ = level; }

	inline const bool is_implant_wire() { return is_implant_wire_; }
	inline void set_is_implant_wire(bool wire) { is_implant_wire_ = wire; }

	const std::vector<glm::vec3>& GetVolVertex() const;
	bool GetTextureData(unsigned char*& out_data, const int width, const int height);
	bool GetTextureData(unsigned short*& out_data, const int width, const int height);
	void InitOffScreenBuff(const int width, const int height);
	void UpdateOffFrameBuffer(const int width, const int height);

private:
	virtual void ProcessViewEvent(bool *need_render) override {};
	virtual void SetFitMode(BaseTransform::FitMode mode) override {};

	virtual bool SetRenderer() override;
	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

	void RenderSlice();
	void DrawSurface();

	SliceRenderer& Renderer() const;
	inline ViewPlaneObjGL* slice_obj() { return slice_obj_.get(); }
	virtual void ApplyPreferences() override;
	void DeleteOffScreenBuff();
	bool OffScreenDrawSlice(const int viewport_width, const int viewport_height, bool draw_surface);

private:
	std::unique_ptr<TransformSlice> transform_;
	std::unique_ptr<ViewPlaneObjGL> slice_obj_;
	std::unique_ptr<GLNerveWidget> nerve_;
	std::unique_ptr<GLImplantWidget> implant_;

	bool is_implant_wire_ = false;
	PackTexture pack_screen_;

	SharpenLevel sharpen_level_ = SharpenLevel::SHARPEN_OFF;

	bool is_update_slice_obj_ = false;
	bool is_set_plane_ = false;
	int slice_thickness_ = 1;

	unsigned int off_fb_ = 0;
	unsigned int off_rb_ = 0;
	PackTexture off_texture_pack_;
};
