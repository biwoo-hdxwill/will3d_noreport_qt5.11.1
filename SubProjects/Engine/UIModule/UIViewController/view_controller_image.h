#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				view_controller_image.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-23
Last modify:		2017-08-23

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "uiviewcontroller_global.h"


#include "base_view_controller.h"

#include "../../Common/GLfunctions/W3GLTypes.h"
#include "../../Resource/Resource/image_2d.h"
#include "transform_image.h"

class ImageRenderer;
class GLImplantWidget;

class UIVIEWCONTROLLER_EXPORT ViewControllerImage : public BaseViewController
{
public:
	explicit ViewControllerImage();
	~ViewControllerImage();

	ViewControllerImage(const ViewControllerImage&) = delete;
	ViewControllerImage& operator=(const ViewControllerImage&) = delete;

public:
	virtual void ClearGL() override;
	virtual void ProcessViewEvent(bool *need_render) override;
	virtual void SetProjection() override;
	virtual bool IsReady() override;

	void SetSharpenLevel(const SharpenLevel& level);
	void SetImage(const std::weak_ptr<resource::Image2D>& image);
	void SetNerveMask(const std::weak_ptr<resource::Image2D>& mask_image);
	void SetImplantMask(const std::weak_ptr<resource::Image2D>& mask_image);
	void RenderScreen(uint dfbo);

	void ResetView();
	void LightView();

	void MapSceneToImage(const std::vector<QPointF>& src_scene_points, std::vector<QPointF>& dst_image_points) const;
	QPointF MapSceneToImage(const QPointF& pt_scene) const;
	void MapImageToScene(const std::vector<QPointF>& src_image_points, std::vector<QPointF>& dst_scene_points) const;
	QPointF MapImageToScene(const QPointF & pt_image) const;

	void GetWindowParams(float* window_width, float* window_level) const;

	float GetIntercept() const;

	bool IsCursorInImage(const QPointF& pt_scene) const;
	const bool& GetInvertWindow() const;
	inline const int& GetImageWidth() const noexcept { return tex_info_img_.width; }
	inline const int& GetImageHeight() const noexcept { return tex_info_img_.height; }
	inline const bool& IsImageSizeChanged() const noexcept { return tex_info_img_.is_diff_size; }

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

protected:
	enum GL_TEXTURE_HANDLE {
		TEX_IMAGE = 0,
		TEX_NERVE_MASK,
		TEX_IMPLANT_MASK,
		TEX_END
	};

	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

private:
	struct TextureInfo {
		int width = 0;
		int height = 0;
		bool is_updated = false;
		bool is_diff_size = false;
		PackTexture pack_;
	};

	void SetImageTexture();

	virtual bool SetRenderer() override;

	ImageRenderer& Renderer() const;

	void SetMaskTexture(resource::Image2D* mask, TextureInfo* texture_info);

	bool IsValidImage(const resource::Image2D& img) const;
	bool IsValidMask(const resource::Image2D& mask) const;
	bool IsDifferenceTextureSize(const TextureInfo& texture_info, int width, int height) const;
	void DeleteTexture(TextureInfo* texture_info);


private:
	std::weak_ptr<resource::Image2D> img_;
	std::weak_ptr<resource::Image2D> mask_nerve_;
	std::weak_ptr<resource::Image2D> mask_implant_;

	std::unique_ptr<TransformImage> transform_;
	
	TextureInfo tex_info_img_;
	TextureInfo	mask_nerve_tex_info_;
	TextureInfo	mask_implant_tex_info_;

	SharpenLevel sharpen_level_ = SharpenLevel::SHARPEN_OFF;
};
