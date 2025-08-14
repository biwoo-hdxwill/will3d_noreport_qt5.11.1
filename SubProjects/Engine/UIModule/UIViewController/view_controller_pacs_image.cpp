#include "view_controller_pacs_image.h"

#include <GL/glew.h>

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"
#include "../../Common/Common/W3Logger.h"

#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../Module/Renderer/image_renderer.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/image_2d.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../UIGLObjects/view_plane_obj_gl.h"
#include "../UIGLObjects/gl_implant_widget.h"

#include "view_render_param.h"

using namespace UIViewController;
using namespace resource;

ViewControllerPacsImage::ViewControllerPacsImage()
{
	transform_.reset(new TransformImage);
}

ViewControllerPacsImage::~ViewControllerPacsImage()
{
	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		DeleteImageData(static_cast<GL_TEXTURE_HANDLE>(i));
	}
}

/**======================================================================================
Public Functions
*====================================================================================**/
void ViewControllerPacsImage::SetSharpenLevel(const SharpenLevel level)
{
	offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_IMAGE].is_updated = true;
	sharpen_level_ = level;
}

void ViewControllerPacsImage::SetImage(const std::weak_ptr<Image2D>& image, GL_TEXTURE_HANDLE id)
{
	if (id < GL_TEXTURE_HANDLE::TEX_IMAGE || id >= GL_TEXTURE_HANDLE::TEX_END)
	{
		return;
	}

	Image2D* img = image.lock().get();
	if (!IsValidImage(*img))
	{
		return;
	}

	bool is_main_img = id == GL_TEXTURE_HANDLE::TEX_IMAGE ? true : false;
	int width = img->Width();
	int height = img->Height();

	offscreen_tex_list_[id].is_diff_size = IsDifferenceTextureSize(offscreen_tex_list_[id], width, height);

	if (offscreen_tex_list_[id].is_updated || offscreen_tex_list_[id].is_diff_size)
	{
		CopyImageData(id, img);
	}

	if (offscreen_tex_list_[id].is_diff_size)
	{
		offscreen_tex_list_[id].is_updated = true;

		if (is_main_img)
		{
			transform_->Initialize(width, height);
			SetProjection();
		}
	}
}

void ViewControllerPacsImage::ImageUpdate()
{
	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		offscreen_tex_list_[i].is_updated = true;
	}
}

void ViewControllerPacsImage::RenderScreen(uint dfbo)
{
	auto view_param = BaseViewController::view_param();
	const QSize& view_size = view_param->view_size();
	if (view_param == nullptr)
	{
		return;
	}

	int index = GL_TEXTURE_HANDLE::TEX_OFFSCREEN;
	if (offscreen_tex_list_[index].pack_.handler == 0)
	{
		return;
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
	glViewport(0, 0, view_size.width(), view_size.height());
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	Renderer().BlendImage(plane_obj(), transform_->mvp(), offscreen_tex_list_[index].pack_);
	//Renderer().BlendImage(plane_obj(), glm::mat4(), offscreen_tex_list_[index].pack_);
	CW3GLFunctions::printError(__LINE__, "BaseViewController3D::RenderScreen");
}

void ViewControllerPacsImage::RenderBackScreen(bool nerve, bool implant)
{
	int id = GL_TEXTURE_HANDLE::TEX_IMAGE;
	int width = offscreen_tex_list_[id].width;
	int height = offscreen_tex_list_[id].height;

	if (init_off_ == false)
	{
		InitOffScreenBuff(width, height);
	}
	else
	{
		if (offscreen_tex_list_[id].is_diff_size)
		{
			UpdateOffFrameBuffer(width, height);
		}
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, off_fb_);
	glViewport(0, 0, width, height);
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_OFFSCREEN; ++i)
	{
		if (offscreen_tex_list_[i].is_updated)
		{
			SetImageTexture(&offscreen_tex_list_[i]);

			if (i == TEX_IMAGE)
			{
				if (sharpen_level_ != SHARPEN_OFF)
				{
					Renderer().PostProcessingSharpen(offscreen_tex_list_[i].pack_, sharpen_level_);
				}
			}
			offscreen_tex_list_[i].is_diff_size = false;
			offscreen_tex_list_[i].is_updated = false;

			CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::offscreen_tex_list");
		}
	}

	glDrawBuffer(offscreen_tex_list_[TEX_OFFSCREEN].pack_.tex_buffer);
	if (offscreen_tex_list_[TEX_IMAGE].pack_.handler)
	{
		//Renderer().DrawImage(plane_obj(), transform_->mvp(), offscreen_tex_list_[TEX_IMAGE].pack_);
		Renderer().DrawImage(plane_obj(), glm::mat4(), offscreen_tex_list_[TEX_IMAGE].pack_);
		CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::DrawImage");
	}

	if (nerve)
	{
		if (offscreen_tex_list_[TEX_NERVE_MASK].pack_.handler)
		{
			//Renderer().BlendImage(plane_obj(), transform_->mvp(), offscreen_tex_list_[TEX_NERVE_MASK].pack_);
			Renderer().BlendImage(plane_obj(), glm::mat4(), offscreen_tex_list_[TEX_NERVE_MASK].pack_);
			CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::BlendImage");
		}
	}

	if (implant)
	{
		if (offscreen_tex_list_[TEX_IMPLANT_MASK].pack_.handler)
		{
			//Renderer().BlendImage(plane_obj(), transform_->mvp(), offscreen_tex_list_[TEX_IMPLANT_MASK].pack_);
			Renderer().BlendImage(plane_obj(), glm::mat4(), offscreen_tex_list_[TEX_IMPLANT_MASK].pack_);
			CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::BlendImage");
		}
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

bool ViewControllerPacsImage::GetTextureData(unsigned char*& out_data, int& width, int& height)
{
	int id = GL_TEXTURE_HANDLE::TEX_IMAGE;
	int img_width = offscreen_tex_list_[id].width;
	int img_height = offscreen_tex_list_[id].height;
	if (img_width == 0 || img_height == 0)
	{
		return false;
	}

	width = img_width;
	height = img_height;

	int size = width * height;
	out_data = new unsigned char[size * 3];

	const int w_4 = width * 4;
	const int w_3 = width * 3;

	float* tex_buffer = new float[size * 4]; //RGBA 

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, off_fb_);
	glActiveTexture(offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_OFFSCREEN].pack_.tex_num);
	glBindTexture(GL_TEXTURE_2D, offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_OFFSCREEN].pack_.handler);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		unsigned char* img_buf = out_data + i * w_3;
		float* tex_buf = tex_buffer + i * w_4;

		for (int j = 0; j < width; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				float val = *tex_buf * 255.f;
				if (val < 0.f)
				{
					val = 0.f;
				}
				else if (val > 255.f)
				{
					val = 255.f;
				}
				*img_buf++ = static_cast<unsigned char>(val);
				tex_buf++;
			}
			tex_buf++;
		}
	}
	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

bool ViewControllerPacsImage::GetTextureData(unsigned short*& out_data, int& width, int& height)
{
	int id = GL_TEXTURE_HANDLE::TEX_IMAGE;
	int img_width = offscreen_tex_list_[id].width;
	int img_height = offscreen_tex_list_[id].height;
	if (img_width == 0 || img_height == 0)
	{
		return false;
	}

	width = img_width;
	height = img_height;

	const int size = width * height;
	const int w_4 = width * 4;

	out_data = new unsigned short[size];
	float* tex_buffer = new float[size * 4];

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, off_fb_);
	glActiveTexture(offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_OFFSCREEN].pack_.tex_num);
	glBindTexture(GL_TEXTURE_2D, offscreen_tex_list_[GL_TEXTURE_HANDLE::TEX_OFFSCREEN].pack_.handler);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

	float ww, wl;
	Renderer().GetWindowParams(&ww, &wl);	
	float window_min = wl - (ww * 0.5f);
	float slope = ResourceContainer::GetInstance()->GetMainVolume().slope();
	int pixel_representation_offset = ResourceContainer::GetInstance()->GetMainVolume().pixel_representation_offset();

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		ushort* img_buf = out_data + i * width;
		float* tex_buf = tex_buffer + i * w_4;

		for (int j = 0; j < width; ++j)
		{
			float val = (*tex_buf * ww + window_min - pixel_representation_offset) / slope;
			*img_buf++ = static_cast<ushort>(val);
			tex_buf += 4;
		}
	}
	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void ViewControllerPacsImage::ClearGL()
{
	BaseViewController::ClearGL();
	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		DeleteTexture(&offscreen_tex_list_[i]);
	}

	if (off_rb_)
	{
		glDeleteRenderbuffers(1, &off_rb_);
		off_rb_ = 0;
	}

	if (off_fb_)
	{
		glDeleteFramebuffers(1, &off_fb_);
		off_fb_ = 0;
	}
}

void ViewControllerPacsImage::SetProjection()
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	PackViewProj arg = GetPackViewProj();
	float map_scene_to_gl;

	float width = GLhelper::ScaleVolToGL(static_cast<float>(offscreen_tex_list_[TEX_IMAGE].width));
	float height = GLhelper::ScaleVolToGL(static_cast<float>(offscreen_tex_list_[TEX_IMAGE].height));

	transform_->SetProjectionFitIn(arg, width, height, map_scene_to_gl);

	view_param->set_map_scene_to_gl(map_scene_to_gl);
	view_param->set_is_valid_map_scene_to_gl(true);
}

void ViewControllerPacsImage::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}

/**=================================================================================================
Private Functions
 *===============================================================================================**/
bool ViewControllerPacsImage::SetRenderer()
{
	if (Renderer().IsInitialize())
	{
		auto view_param = BaseViewController::view_param();
		if (view_param == nullptr)
		{
			return false;
		}

		view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());

		SetProjection();
		return true;
	}

	return false;
}

ImageRenderer& ViewControllerPacsImage::Renderer() const
{
	return RendererManager::GetInstance().renderer_image();
}

void ViewControllerPacsImage::SetImageTexture(TextureInfo* texture_info)
{
	CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::SetMask Start");

	if (texture_info->data == nullptr)
	{
		return;
	}

	image::ImageFormat mask_format = texture_info->format;
	glActiveTexture(texture_info->pack_.tex_num);

	switch (mask_format)
	{
	case image::ImageFormat::RGBA32:
		CW3GLFunctions::Update2DTexRGBA32UI(texture_info->pack_.handler,
			texture_info->width, texture_info->height, texture_info->data, texture_info->is_diff_size);
		break;
	case image::ImageFormat::GRAY8:
		CW3GLFunctions::Update2DTex8(texture_info->pack_.handler,
			texture_info->width, texture_info->height, texture_info->data, texture_info->is_diff_size);
		break;
	case image::ImageFormat::GRAY16UI:
		CW3GLFunctions::Update2DTex16UI(texture_info->pack_.handler,
			texture_info->width, texture_info->height, reinterpret_cast<ushort*>(texture_info->data), texture_info->is_diff_size);
		break;
	case image::ImageFormat::GRAY16:
		CW3GLFunctions::Update2DTex16(texture_info->pack_.handler,
			texture_info->width, texture_info->height, reinterpret_cast<short*>(texture_info->data), texture_info->is_diff_size);
		break;
	case image::ImageFormat::UNKNOWN:
		return;
	}
	CW3GLFunctions::printError(__LINE__, "ViewControllerPacsImage::SetMask End");
}

bool ViewControllerPacsImage::IsValidImage(const Image2D& img) const
{
	if (&img == nullptr || img.Width() == 0 || img.Height() == 0)
	{
		return false;
	}

	return true;
}

bool ViewControllerPacsImage::IsDifferenceTextureSize(const TextureInfo& texture_info, int width, int height) const
{
	if (texture_info.width != width || texture_info.height != height)
	{
		return true;
	}

	return false;
}

void ViewControllerPacsImage::DeleteTexture(TextureInfo* texture_info)
{
	if (texture_info->pack_.handler)
	{
		glDeleteTextures(1, &texture_info->pack_.handler);
		texture_info->pack_.handler = 0;
	}
}

void ViewControllerPacsImage::InitOffScreenBuff(const int width, const int height)
{
	if (init_off_)
	{
		return;
	}

	if (off_fb_)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &off_fb_);
		off_fb_ = 0;
	}

	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		offscreen_tex_list_[i].pack_.tex_buffer = GL_COLOR_ATTACHMENT0 + i;
		offscreen_tex_list_[i].pack_.tex_num = GL_TEXTURE0 + i;
		offscreen_tex_list_[i].pack_._tex_num = i;
	}

	glGenFramebuffers(1, &off_fb_);
	UpdateOffFrameBuffer(width, height);

	init_off_ = true;
}

void ViewControllerPacsImage::UpdateOffFrameBuffer(const int width, const int height)
{
	if (off_rb_)
	{
		glDeleteRenderbuffers(1, &off_rb_);
		off_rb_ = 0;
	}

	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		if (offscreen_tex_list_[i].pack_.handler)
		{
			glDeleteTextures(1, &offscreen_tex_list_[i].pack_.handler);
			offscreen_tex_list_[i].pack_.handler = 0;
		}

		glGenTextures(1, &offscreen_tex_list_[i].pack_.handler);
		glActiveTexture(offscreen_tex_list_[i].pack_.tex_num);
		glBindTexture(GL_TEXTURE_2D, offscreen_tex_list_[i].pack_.handler);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glGenRenderbuffers(1, &off_rb_);
	glBindRenderbuffer(GL_RENDERBUFFER, off_rb_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);
	for (int i = 0; i < GL_TEXTURE_HANDLE::TEX_END; ++i)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, offscreen_tex_list_[i].pack_.tex_buffer, GL_TEXTURE_2D, offscreen_tex_list_[i].pack_.handler, 0);
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, off_rb_);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewControllerPacsImage::CopyImageData(const GL_TEXTURE_HANDLE id, const resource::Image2D* img)
{
	DeleteImageData(id);

	image::ImageFormat format = img->Format();
	offscreen_tex_list_[id].format = format;

	int width = img->Width();
	int height = img->Height();

	offscreen_tex_list_[id].width = width;
	offscreen_tex_list_[id].height = height;

	int size = width * height;
	switch (format)
	{
	case image::ImageFormat::RGBA32:
		size *= sizeof(image::RGBA32);
		break;
	case image::ImageFormat::GRAY16:
	case image::ImageFormat::GRAY16UI:
		size *= sizeof(ushort);
		break;
	case image::ImageFormat::GRAY8:
		size *= sizeof(uchar);
		break;
	}

	offscreen_tex_list_[id].data = new uchar[size];
	memcpy(offscreen_tex_list_[id].data, img->Data(), sizeof(uchar) * size);
}

void ViewControllerPacsImage::DeleteImageData(const GL_TEXTURE_HANDLE id)
{
	if (offscreen_tex_list_[id].data)
	{
		delete[] offscreen_tex_list_[id].data;
		offscreen_tex_list_[id].data = nullptr;
	}
}
