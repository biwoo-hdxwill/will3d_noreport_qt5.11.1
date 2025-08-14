#include "view_controller_pacs_3d.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"

#include "../../Module/Will3DEngine/renderer_manager.h"
#include "../../UIModule/UIGLObjects/view_plane_obj_gl.h"

#include "view_render_param.h"
#include "surface_items.h"

using namespace Will3DEngine;
using namespace UIGLObjects;

namespace
{
	const int kOffFront = 0;
	const int kOffBack = 1;
	const int kOffRaycasting = 2;
	const int kOffDepth = 3;
}

ViewControllerPACS3D::ViewControllerPACS3D()
{
	transform_.reset(new TransformBasicVR());
	surface_items()->InitItem(SurfaceItems::NERVE, ObjCoordSysType::TYPE_VOLUME);
	surface_items()->InitItem(SurfaceItems::IMPLANT, ObjCoordSysType::TYPE_VOLUME);
}

ViewControllerPACS3D::~ViewControllerPACS3D()
{

}

/*=================================================================================================
public functions
=================================================================================================*/
void ViewControllerPACS3D::SetTFupdated(bool is_min_max_changed)
{
	if (!is_min_max_changed)
	{
		return;
	}

	set_indices_vol(Renderer().GetActiveIndices());

	uint vao = vao_vol();
	Renderer().UpdateActiveBlockVAO(&vao);
	set_vao_vol(vao);
}

void ViewControllerPACS3D::ClearGL()
{
	BaseViewController3D::ClearGL();
}

void ViewControllerPACS3D::SetFitMode(BaseTransform::FitMode mode)
{
	transform_->set_fit_mode(mode);
}

bool ViewControllerPACS3D::GetTextureData(unsigned char*& out_data, const int width, const int height, const glm::mat4& rot_mat)
{
#if 1
	if (off_fb_ == 0)
	{
		off_depth_handler_.resize(1, 0);
		off_tex_handler_.resize(3, 0);

		GLenum color_attach_id = GL_COLOR_ATTACHMENT4;
		for (int i = 0; i < 3; ++i)
		{
			off_tex_buffer_.push_back(color_attach_id + i);
			off_tex_num_.push_back(GL_TEXTURE9 + i);
		}

		for (int i = 0; i < off_tex_handler_.size(); ++i)
		{
			glGenTextures(1, &off_tex_handler_[i]);

			glActiveTexture(off_tex_num_[i]);
			glBindTexture(GL_TEXTURE_2D, (off_tex_handler_[i]));
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		for (int i = 0; i < off_depth_handler_.size(); ++i)
		{
			glGenRenderbuffers(1, &off_depth_handler_[i]);
			glBindRenderbuffer(GL_RENDERBUFFER, off_depth_handler_[i]);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		}

		glGenFramebuffers(1, &off_fb_);
		glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);


		for (int i = 0; i < off_tex_handler_.size(); ++i)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, color_attach_id + i, GL_TEXTURE_2D, off_tex_handler_[i], 0);
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, off_depth_handler_[0]);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);
	glViewport(0, 0, width, height);

	this->SetRayStepSize();

	Renderer().SetVolumeTexture();
	Renderer().SetTFtexture();
	
	PackTexture pack_front(off_tex_buffer_[kOffFront], off_tex_num_[kOffFront], off_tex_handler_[kOffFront], 9);
	PackTexture pack_back(off_tex_buffer_[kOffBack], off_tex_num_[kOffBack], off_tex_handler_[kOffBack], 10);
	PackTexture pack_raycasting(off_tex_buffer_[kOffRaycasting], off_tex_num_[kOffRaycasting], off_tex_handler_[kOffRaycasting], 11);

	float fw = static_cast<float>(width) * 0.5f;
	float fh = static_cast<float>(height) * 0.5f;
	glm::mat4 m = rot_mat * transform().model();
	glm::mat4 v = transform().view();
	glm::mat4 p = glm::ortho(-fw, fw, -fh, fh, 0.f, transform().cam_fov() * 2.f);

	surface_items()->SetSurfaceMVP(transform().model(), v * rot_mat, p);

	glm::mat4 mvp = p * v * m;	

	Renderer().DrawFrontFace(vao_vol(), indices_vol(), pack_front.tex_buffer, mvp);
	Renderer().DrawBackFace(vao_vol(), indices_vol(), pack_back.tex_buffer, mvp);

	glDepthFunc(GL_LESS);
	this->DrawBackFaceSurface();

	PackClipping temp_clipping;
	Renderer().DrawFinalFrontFace(plane_obj(), pack_front, pack_back, temp_clipping);

	CW3GLFunctions::setDepthStencilAttarch(off_depth_handler_[0]);

	glDrawBuffer(pack_raycasting.tex_buffer);
	{
		CW3GLFunctions::clearView(true, GL_BACK);

		this->DrawSurface();

		Renderer().DrawRaycasting(plane_obj(), pack_raycasting.tex_buffer, mvp, pack_front, pack_back);
	}

	glActiveTexture(pack_raycasting.tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_raycasting.handler);
#else 
	glActiveTexture(pack_raycasting().tex_num);
	glBindTexture(GL_TEXTURE_2D, pack_raycasting().handler);

#endif
	const int size = width * height;
	const int w_4 = width * 4;
	const int w_3 = width * 3;

	float* tex_buffer = new float[size * 4]; //RGBA 
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		unsigned char* img_buf = out_data + i * w_3;
		float* tex_buf = tex_buffer + (height - i - 1) * w_4;

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

/*=================================================================================================
private functions
=================================================================================================*/
void ViewControllerPACS3D::InitVAOs()
{
	BaseViewController3D::InitVAOs();
}

VolumeRenderer& ViewControllerPACS3D::Renderer() const
{
	return RendererManager::GetInstance().renderer_vol(VOL_MAIN);
}

BaseTransform& ViewControllerPACS3D::transform() const
{
	return *(dynamic_cast<BaseTransform*>(transform_.get()));
}
