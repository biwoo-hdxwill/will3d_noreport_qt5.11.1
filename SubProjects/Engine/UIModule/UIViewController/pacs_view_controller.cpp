#include "pacs_view_controller.h"

#include <omp.h>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Logger.h"

#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/gl_helper.h"

#include "../../Core/ImageProcessing/W3ImageProcessing.h"

#include "../../Module/Renderer/slice_renderer.h"
#include "../../Module/Will3DEngine/renderer_manager.h"

#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"

#include "../../UIGLObjects/view_plane_obj_gl.h"
#include "../../UIGLObjects/gl_implant_widget.h"
#include "../../UIGLObjects/gl_nerve_widget.h"

#include "view_render_param.h"
#include "transform_slice.h"

using namespace Will3DEngine;
using namespace UIGLObjects;

namespace
{
	const glm::vec3 kInvAxisX(-1.f, 1.f, 1.f);
	const int kScreenNum = 0;
	const float kInvUshortMax = 1.f / 65535.f;
}

PACSViewController::PACSViewController()
{
	transform_.reset(new TransformSlice());

	slice_obj_.reset(new ViewPlaneObjGL());
	slice_obj_->set_vertex_scale(glm::vec3(sqrt(2.0f)));

	nerve_.reset(new GLNerveWidget(ObjCoordSysType::TYPE_VOLUME));
	implant_.reset(new GLImplantWidget(ObjCoordSysType::TYPE_VOLUME));

	GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;
	sharpen_level_ = static_cast<SharpenLevel>(setting.mpr_filter_level);
}

PACSViewController::~PACSViewController()
{

}

/**=================================================================================================
public functions
*===============================================================================================**/
void PACSViewController::SetPlane(const glm::vec3& center_pos, const glm::vec3& right_vector, const glm::vec3& back_vector, int thickness_in_vol)
{
	const float z_spacing = Renderer().GetSpacingZ();

	glm::vec3 center = Renderer().GetVolCenter();
	glm::vec3 center_position_in_vol_gl = kInvAxisX * GLhelper::MapVolToWorldGL(center_pos, Renderer().GetVolCenter(), z_spacing);
	glm::vec3 rv = kInvAxisX * right_vector;
	glm::vec3 bv = kInvAxisX * back_vector;
	
#if 0
	glm::vec3 model_scale = Renderer().GetModelScale();
	float major_scale = std::max(model_scale.x, model_scale.y);
	major_scale = std::max(major_scale, model_scale.z);

	float right_scale = glm::length(major_scale * glm::normalize(rv));
	float back_scale = glm::length(major_scale * glm::normalize(bv));
	slice_obj_->set_vertex_scale(glm::vec3(glm::length(rv / right_scale), glm::length(bv / back_scale), 1.0f));
#endif
	is_update_slice_obj_ = true;
	slice_thickness_ = thickness_in_vol;
	this->SetProjection();
	transform_->SetPlane(center_position_in_vol_gl, rv, bv);
	is_set_plane_ = true;
}

void PACSViewController::ClearGL()
{
	BaseViewController::ClearGL();

	slice_obj_->ClearVAOVBO();
	nerve_->ClearVAOVBO();
	implant_->ClearVAOVBO();

	DeleteOffScreenBuff();
}

void PACSViewController::SetProjection()
{
	if (!Renderer().IsInitialize())
	{
		return;
	}

	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	float map_scene_to_gl = 1.0f;
	transform_->SetProjection(GetPackViewProj(), map_scene_to_gl);
	view_param->set_map_scene_to_gl(map_scene_to_gl);
}

bool PACSViewController::IsReady()
{
	return (Renderer().IsInitialize() && initialized() && is_set_plane_);
}

void PACSViewController::SetVisibleNerve(bool is_visible)
{
	nerve_->set_is_visible(is_visible);
}

void PACSViewController::SetVisibleImplant(bool is_visible)
{
	implant_->set_is_visible(is_visible);
}

void PACSViewController::RenderingSlice()
{
	if (!initialized())
	{
		Initialize();
		SetProjection();
	}

	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

	CW3GLFunctions::printError(__LINE__, "CW3PACSViewController::RenderSlice Start");
	BaseViewController::ReadyFrameBuffer();

	RenderSlice();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::RenderSlice");

	DrawSurface();
	CW3GLFunctions::printError(__LINE__, "CW3ViewControllerSlice::DrawSurface");
}

void PACSViewController::RenderScreen(uint dfbo)
{
	auto view_param = BaseViewController::view_param();
	if (view_param == nullptr)
	{
		return;
	}

	const QSize& view_size = view_param->view_size();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo);
	glViewport(0, 0, view_size.width(), view_size.height());

	if (is_set_plane_)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		Renderer().DrawScreen(plane_obj(), pack_screen_);
	}

	CW3GLFunctions::printError(__LINE__, "CW3PACSViewController::RenderScreen");
}

glm::mat4 PACSViewController::GetRotateMatrix() const
{
	return transform_->rotate() * transform_->reorien();
}

const glm::mat4& PACSViewController::GetViewMatrix() const
{
	return transform_->view();
}

const std::vector<glm::vec3>& PACSViewController::GetVolVertex() const
{
	return Renderer().GetVolVertex();
}

bool PACSViewController::GetTextureData(unsigned char*& out_data, const int width, const int height)
{
	if (!out_data || OffScreenDrawSlice(width, height, true) == false)
	{
		return false;
	}
		
	const int size = width * height;
	const int w_4 = width * 4;
	const int w_3 = width * 3;

	float* tex_buffer = new float[size * 4]; //RGBA 

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

	float ww, wl;
	Renderer().GetWindowParams(&ww, &wl);

	float window_min = wl - (ww * 0.5f);
	float window_max = wl + (ww * 0.5f);

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		unsigned char* img_buf = out_data + i * w_3;
		float* tex_buf = tex_buffer + (height - i - 1) * w_4;

		for (int j = 0; j < width; ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				float value = *tex_buf * 65535.f;
				if (value < window_min)
				{
					value = window_min;
				}
				else if (value > window_max)
				{
					value = window_max;
				}

				value = (value - window_min) / ww;
				*img_buf++ = static_cast<unsigned char>(value * 255.f);
				tex_buf++;
			}
			tex_buf++;
		}
	}

	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

bool PACSViewController::GetTextureData(unsigned short*& out_data, const int width, const int height)
{
	if (!out_data || OffScreenDrawSlice(width, height, false) == false)
	{
		return false;
	}

	const int size = width * height;
	const int w_4 = width * 4;
	float* tex_buffer = new float[size * 4];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tex_buffer);

	float slope = ResourceContainer::GetInstance()->GetMainVolume().slope();
	int pixel_representation_offset = ResourceContainer::GetInstance()->GetMainVolume().pixel_representation_offset();

#pragma omp parallel for
	for (int i = 0; i < height; ++i)
	{
		ushort* img_buf = out_data + i * width;
		float* tex_buf = tex_buffer + (height - i - 1) * w_4;

		for (int j = 0; j < width; ++j)
		{
			float value = *tex_buf;
			*img_buf++ = static_cast<ushort>((value * 65535.f) / slope - pixel_representation_offset);
			tex_buf += 4;
		}
	}

	delete[] tex_buffer;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void PACSViewController::InitOffScreenBuff(const int width, const int height)
{
	if (off_fb_)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &off_fb_);
		off_fb_ = 0;
	}

	off_texture_pack_.tex_buffer = GL_COLOR_ATTACHMENT1;
	off_texture_pack_.tex_num = GL_TEXTURE1;

	glGenFramebuffers(1, &off_fb_);
	UpdateOffFrameBuffer(width, height);
}

void PACSViewController::UpdateOffFrameBuffer(const int width, const int height)
{
	if (off_texture_pack_.handler)
	{
		glDeleteTextures(1, &off_texture_pack_.handler);
		off_texture_pack_.handler = 0;
	}

	if (off_rb_)
	{
		glDeleteRenderbuffers(1, &off_rb_);
		off_rb_ = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);

	glGenTextures(1, &off_texture_pack_.handler);

	glActiveTexture(off_texture_pack_.tex_num);
	glBindTexture(GL_TEXTURE_2D, off_texture_pack_.handler);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &off_rb_);
	glBindRenderbuffer(GL_RENDERBUFFER, off_rb_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	glFramebufferTexture2D(GL_FRAMEBUFFER, off_texture_pack_.tex_buffer, GL_TEXTURE_2D, off_texture_pack_.handler, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, off_rb_);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**=================================================================================================
private functions: level 0
*===============================================================================================**/
bool PACSViewController::SetRenderer()
{
	if (Renderer().IsInitialize())
	{
		auto view_param = BaseViewController::view_param();
		if (view_param == nullptr)
		{
			return false;
		}

		transform_->Initialize(Renderer().GetModelScale());
		view_param->set_base_pixel_spacing_mm(Renderer().GetBasePixelSpacingMM());
		this->SetProjection();
		//is_set_plane_ = true;
		return true;
	}
	else
	{
		return false;
	}
}

void PACSViewController::SetPackTexture()
{
	int handle_id = kScreenNum;
	int buffer = tex_buffer(handle_id);
	int texture_num = tex_num(handle_id);
	int texture_handle = tex_handler(handle_id);
	int _texture_num = _tex_num(handle_id);

	pack_screen_ = PackTexture(buffer, texture_num, texture_handle, _texture_num);
}

void PACSViewController::SetPackTextureHandle()
{
	pack_screen_.handler = tex_handler(kScreenNum);
}

void PACSViewController::ReadyBufferHandles()
{
	BaseViewController::ReadyBufferHandles(1, 1);
}

void PACSViewController::RenderSlice()
{
	if (!initialized())
	{
		Initialize();
	}

	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

	Renderer().SetVolumeTexture();

	if (is_update_slice_obj_)
	{
		slice_obj_->ClearVAOVBO();
		is_update_slice_obj_ = false;
	}

	Renderer().DrawSlice(slice_obj_.get(), tex_buffer(kScreenNum),
		transform_->mvp(), transform_->vol_tex_transform_mat(),
		transform_->GetUpVector(), slice_thickness_, false);

	if (sharpen_level_ != SHARPEN_OFF)
	{
		Renderer().PostProcessingSharpen(pack_screen_, sharpen_level_);
	}
}

void PACSViewController::DrawSurface()
{
	if (!Renderer().IsInitialize())
	{
		ClearGL();
		return;
	}

	const auto& prog = Renderer().programs();
	if (nerve_->is_visible())
	{
		nerve_->set_projection(transform_->projection());
		nerve_->set_view(transform_->view());
		nerve_->SetTransformMat(transform_->rotate(), ARCBALL);
		nerve_->SetTransformMat(transform_->reorien(), REORIENTATION);

		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glUseProgram(prog.render_slice_nerve);
		if (slice_thickness_ > 1)
		{
			float save_gl_z = transform_->gl_z_position();
			glm::mat4 upper_mvp, lower_mvp;
			transform_->TranslateZ((float)slice_thickness_);
			upper_mvp = transform_->mvp();
			transform_->TranslateZ(-(float)slice_thickness_);
			lower_mvp = transform_->mvp();
			transform_->TranslateZ(save_gl_z);

			vec4 plane_equ = transform_->plane_equation();
			vec3 plane_up_vector = vec3(transform_->plane_equation());

			vec4 slice_equ_front(plane_up_vector, plane_equ.w - slice_thickness_);
			vec4 slice_equ_back(plane_up_vector, plane_equ.w + slice_thickness_);

			nerve_->RenderThicknessSlice(prog.render_slice_nerve, slice_obj(),
				upper_mvp, lower_mvp, slice_equ_front,
				slice_equ_back);
		}
		else
		{
			nerve_->RenderSlice(prog.render_slice_nerve, slice_obj(), transform_->mvp());
		}
		glUseProgram(0);
	}

	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
	if (implant_->is_visible())
	{
		if (!is_implant_wire_)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_DEPTH_TEST);
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);
			glDepthFunc(GL_LESS);
			glUseProgram(prog.render_slice_implant);

			glm::vec4 plane_equ = transform_->plane_equation();
			float z_position = -transform_->gl_z_position();

			implant_->set_projection(transform_->projection());
			implant_->set_view(transform_->view());
			implant_->SetTransformMat(transform_->rotate(), ARCBALL);
			implant_->SetTransformMat(transform_->reorien(), REORIENTATION);

			implant_->RenderSlice(
				prog.render_slice_implant,
				glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position));

			glUseProgram(0);

			glDisable(GL_BLEND);
		}
		else
		{
			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);

			glDisable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);

			glUseProgram(prog.render_slice_implant_wire);

			implant_->set_projection(transform_->projection_for_implant());
			implant_->set_view(transform_->view_for_implant());
			implant_->SetTransformMat(transform_->rotate(), ARCBALL);
			implant_->SetTransformMat(transform_->reorien(), REORIENTATION);

			glm::vec4 plane_equ = transform_->plane_equation();
			float z_position = -transform_->gl_z_position();

			implant_->RenderSliceWire(
				prog.render_slice_implant_wire,
				glm::vec4((glm::vec3)plane_equ, plane_equ.w + z_position)
			);

			glUseProgram(0);
		}
	}
}

SliceRenderer& PACSViewController::Renderer() const
{
	return RendererManager::GetInstance().renderer_slice(VOL_MAIN);
}

/**=================================================================================================
private functions: level 1
*===============================================================================================**/
void PACSViewController::ApplyPreferences()
{
	implant_->ApplyPreferences();
	is_implant_wire_ = (GlobalPreferences::GetInstance()->preferences_.objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Wire);
}

void PACSViewController::DeleteOffScreenBuff()
{
	if (off_texture_pack_.handler)
	{
		glDeleteTextures(1, &off_texture_pack_.handler);
		off_texture_pack_.handler = 0;
	}

	if (off_rb_)
	{
		glDeleteRenderbuffers(1, &off_rb_);
		off_rb_ = 0;
	}

	if (off_fb_)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &off_fb_);
		off_fb_ = 0;
	}
}

bool PACSViewController::OffScreenDrawSlice(const int viewport_width, const int viewport_height, bool draw_surface)
{
	auto view_param = BaseViewController::view_param();
	QSize ori_view_size = view_param->view_size();

	view_param->SetViewPort(viewport_width, viewport_height);
	SetProjection();

	glBindFramebuffer(GL_FRAMEBUFFER, off_fb_);
	glViewport(0, 0, viewport_width, viewport_height);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		common::Logger::instance()->Print(common::LogType::ERR, " PACSViewController::OffScreenDrawSlice: invalid fbo.");
		return false;
	}

	Renderer().SetVolumeTexture();
	Renderer().DrawSlice(slice_obj_.get(), off_texture_pack_.tex_buffer,
		transform_->mvp(), transform_->vol_tex_transform_mat(),
		transform_->GetUpVector(), slice_thickness_, false, true);

	if (sharpen_level_ != SHARPEN_OFF)
	{
		Renderer().PostProcessingSharpen(off_texture_pack_, sharpen_level_);
	}

	if (draw_surface)
	{
		DrawSurface();
	}

	view_param->SetViewPort(ori_view_size.width(), ori_view_size.height());
	SetProjection();

	return true;
}
