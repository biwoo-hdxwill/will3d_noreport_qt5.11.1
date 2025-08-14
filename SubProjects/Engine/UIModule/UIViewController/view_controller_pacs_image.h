#pragma once
/*=========================================================================
File:			view_controller_pacs_image.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-08-11
Last modify:	2021-08-13
Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_view_controller.h"

#include "../../Common/GLfunctions/W3GLTypes.h"
#include "../../Resource/Resource/image_2d.h"
#include "transform_image.h"

namespace
{
	struct TextureInfo
	{
		int width = 0;
		int height = 0;
		bool is_updated = false;
		bool is_diff_size = false;

		image::ImageFormat format = image::ImageFormat::UNKNOWN;
		unsigned char *data = nullptr;
		PackTexture pack_;
	};	
}

enum GL_TEXTURE_HANDLE
{
	TEX_IMAGE = 0,
	TEX_NERVE_MASK,
	TEX_IMPLANT_MASK,
	TEX_OFFSCREEN,
	TEX_END
};

class ImageRenderer;
class UIVIEWCONTROLLER_EXPORT ViewControllerPacsImage : public BaseViewController
{
public:
	ViewControllerPacsImage();
	~ViewControllerPacsImage();

	ViewControllerPacsImage(const ViewControllerPacsImage&) = delete;
	ViewControllerPacsImage& operator=(const ViewControllerPacsImage&) = delete;

	void SetSharpenLevel(const SharpenLevel level);
	void SetImage(const std::weak_ptr<resource::Image2D>& image, GL_TEXTURE_HANDLE id);
	void ImageUpdate();
	void RenderScreen(uint dfbo);
	void RenderBackScreen(bool nerve, bool implant);

	bool GetTextureData(unsigned char*& out_data, int& width, int& height);
	bool GetTextureData(unsigned short*& out_data, int& width, int& height);

	virtual void ClearGL() override;
	virtual void SetProjection() override;
	virtual void SetFitMode(BaseTransform::FitMode mode) override;

	inline SharpenLevel sharpen_level() { return sharpen_level_; }

private:
	virtual bool SetRenderer() override;
	ImageRenderer& Renderer() const;

	void SetImageTexture(TextureInfo* texture_info);

	bool IsValidImage(const resource::Image2D& img) const;
	bool IsDifferenceTextureSize(const TextureInfo& texture_info, int width, int height) const;
	void DeleteTexture(TextureInfo* texture_info);

	void InitOffScreenBuff(const int width, const int height);
	void UpdateOffFrameBuffer(const int width, const int height);
	void CopyImageData(const GL_TEXTURE_HANDLE id, const resource::Image2D* img);
	void DeleteImageData(const GL_TEXTURE_HANDLE id);

	virtual void SetPackTexture() override {}
	virtual void SetPackTextureHandle() override {}
	virtual void ReadyBufferHandles() override {}
	virtual void ApplyPreferences() override {}
	virtual void ProcessViewEvent(bool *need_render) override {}
	virtual bool IsReady() override { return true; }

private:
	TextureInfo offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_END];
	std::unique_ptr<TransformImage> transform_;

	unsigned int off_fb_ = 0;
	unsigned int off_rb_ = 0;
	bool init_off_ = false;

	SharpenLevel sharpen_level_ = SharpenLevel::SHARPEN_OFF;
};
