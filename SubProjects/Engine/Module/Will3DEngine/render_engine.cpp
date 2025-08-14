#include "render_engine.h"

#include <QDebug>
#include <QElapsedTimer>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_otf.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Resource/Resource/W3TF.h"
#include "../../Resource/Resource/plane_resource.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "gl_resource_mgr.h"
#include "renderer_manager.h"
#include "native_render_func.h"

#include <QOpenGLContext>
#include <QOffscreenSurface>

using namespace Will3DEngine;
using namespace common;

namespace
{
	VolumeRenderer& RendererVolume(const VolType& vol_type)
	{
		return RendererManager::GetInstance().renderer_vol(vol_type);
	}
	SliceRenderer& RendererSlice(const VolType& vol_type)
	{
		return RendererManager::GetInstance().renderer_slice(vol_type);
	}
	ImageRenderer& RendererImage()
	{
		return RendererManager::GetInstance().renderer_image();
	}
	const char* kErrMsgContextInvalid = "The opengl context is invalid.";
	const char* kErrMsgContextNotUsing = "There isn't using context.";
}

/**=======================================================================================================
public functions
=======================================================================================================*/

RenderEngine::RenderEngine()
{
	gl_resource_mgr_.reset(new GLresourceMgr);
	plane_resource_.reset(new PlaneResource);
	ResourceContainer::GetInstance()->SetPlaneResource(plane_resource_);
	InitializeInternal();
}

RenderEngine::~RenderEngine()
{
	if (MakeCurrent())
	{
		gl_resource_mgr_->ClearGL();
		DoneCurrent();
	}
}

bool RenderEngine::MakeCurrent()
{
	if (context_->isValid())
	{
		context_->makeCurrent(surface_.get());
		return true;
	}
	else
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, kErrMsgContextInvalid);
		return false;
	}
}

bool RenderEngine::DoneCurrent()
{
	if (!IsUsingGLContextInGPU())
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, kErrMsgContextNotUsing);
		return false;
	}

	if (context_->isValid())
	{
		context_->doneCurrent();
		return true;
	}
	else
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, kErrMsgContextInvalid);
		return false;
	}
}

bool RenderEngine::IsValid() const
{
	return (initialized_ && context_);
}

bool RenderEngine::IsUsingGLContextInGPU()
{
	return (context_->globalShareContext()->currentContext() != nullptr);
}

void RenderEngine::EnableGlobalShareContext()
{
	context_->setShareContext(QOpenGLContext::globalShareContext());
}

void RenderEngine::SetShareContext(QOpenGLContext * context)
{
	context_->setShareContext(context);
}

void RenderEngine::SetVRreconType(const QString& otf_name)
{
	if (otf_name == common::otf_preset::MIP)
	{
		this->SetEnableXray(false);
		this->SetEnableMIP(true);
	}
	else if (otf_name == common::otf_preset::XRAY)
	{
		this->SetEnableMIP(false);
		this->SetEnableXray(true);
	}
	else
	{
		this->SetEnableMIP(false);
		this->SetEnableXray(false);
	}
}

void RenderEngine::SetEnableMIP(bool is_mip)
{
	for (int i = 0; i < VolType::VOL_TYPE_END; i++)
		RendererVolume((VolType)i).SetEnableMIP(is_mip);
}

void RenderEngine::SetEnableXray(bool is_xray)
{
	for (int i = 0; i < VolType::VOL_TYPE_END; i++)
		RendererVolume((VolType)i).SetEnableXray(is_xray);
}

void RenderEngine::SetVolume(const CW3Image3D & vol, const Will3DEngine::VolType& vol_type)
{
	QElapsedTimer timer;
	timer.start();

	common::Logger::instance()->Print(common::LogType::INF, "start SetVolume");

	if (!this->MakeCurrent())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::SetVolume: fail to makecurrent.");
		return;
	}

	gl_resource_mgr_->ClearVolumeTexture(vol_type);

	unsigned int vol_tex_handler, vol_tex_id;
	int down_factor;
	gl_resource_mgr_->SetVolumeTexture(vol_type, vol, &vol_tex_handler, &vol_tex_id, &down_factor);

	if (vol_tex_handler != 0)
	{
		if (vol_type == VOL_MAIN)
		{
			RendererImage().SettingsImageParamSet(vol);
		}

		RendererVolume(VolType(vol_type)).InitVRparamSet(vol);
		RendererVolume(VolType(vol_type)).SetTexHandler(vol_tex_handler, vol_tex_id);
		RendererVolume(VolType(vol_type)).SetRenderDownFactor(down_factor);
		RendererVolume(VolType(vol_type)).SetEnableShade(is_enable_shade_vol_);

		if (VolType(vol_type) == VolType::VOL_PANORAMA)
		{
			RendererVolume(VolType(vol_type)).SetVolCenterZtoZero();
		}

		RendererSlice(VolType(vol_type)).InitSliceParamSet(vol);
		RendererSlice(VolType(vol_type)).SetTexHandler(vol_tex_handler, vol_tex_id);
	}

	this->DoneCurrent();

	common::Logger::instance()->Print(common::LogType::INF, "end SetVolume : " + QString::number(timer.elapsed()).toStdString() + " ms");
}
void RenderEngine::ClearVolume(const Will3DEngine::VolType& vol_type)
{
	if (!this->MakeCurrent())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::ClearVolume: fail to makecurrent.");
		return;
	}
	gl_resource_mgr_->ClearVolumeTexture(vol_type);
	RendererVolume(vol_type).Clear();

	this->DoneCurrent();
}
void RenderEngine::SetVolumeShade(bool is_shade)
{
	is_enable_shade_vol_ = is_shade;

	for (int i = 0; i < VOL_TYPE_END; i++)
	{
		RendererVolume((VolType)i).SetEnableShade(is_shade);
	}
}
void RenderEngine::UpdateTF(bool changed_min_max)
{
	if (!this->MakeCurrent())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::UpdateTF: fail to makecurrent.");
		return;
	}

	const auto& res_tf = ResourceContainer::GetInstance()->GetTfResource();
	unsigned int tf_handler, tf_tex_id;
	gl_resource_mgr_->SetTfTexture(res_tf, &tf_handler, &tf_tex_id);

	for (int i = 0; i < VOL_TYPE_END; i++)
	{
		RendererVolume((VolType)i).SettingsTFhandler(tf_handler, tf_tex_id);

		if (changed_min_max)
			RendererVolume((VolType)i).SetActiveBlock(res_tf.min_value(), res_tf.max_value());
	}

	this->DoneCurrent();
}
void RenderEngine::ClearTF()
{
	if (!this->MakeCurrent())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::ClearTF: fail to makecurrent.");
		return;
	}
	gl_resource_mgr_->ClearTf();

	this->DoneCurrent();
}
unsigned int RenderEngine::tmpGetVolTexHandler(const Will3DEngine::VolType& vol_type)
{
	return gl_resource_mgr_->tmpGetVolTexHandler(vol_type);
}
unsigned int RenderEngine::tmpGetTfTexHandler()
{
	return gl_resource_mgr_->tmpGetTfTexHandler();
}
int RenderEngine::tmpGetDownFactor(const Will3DEngine::VolType& vol_type)
{
	return RendererVolume(vol_type).GetRenderDownFactor();
}

void RenderEngine::SavePresetVolumeImage(const QString& file_path)
{
	if (!this->MakeCurrent())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::SavePresetVolumeImage: fail to makecurrent.");
		return;
	}

	NativeRenderFunc::SavePresetVolumeImage(RendererVolume(VOL_MAIN), file_path);
	DoneCurrent();
}
/**=======================================================================================================
private functions
=======================================================================================================*/

void RenderEngine::InitializeInternal()
{
	bool res = InitGLcontext();

	initialized_ = false;

	if (!res)
		return;

	if (this->MakeCurrent())
	{
		glewExperimental = GL_TRUE;
		GLenum glewErr = glewInit();
		if (GLEW_OK != glewErr)
		{
			common::Logger::instance()->Print(common::LogType::ERR, "RenderEngine::InitializeInternal: glew initialization failed.");
			this->DoneCurrent();
			return;
		}

		this->SetOpenGLDeviceInfo();
		this->InitRenderers();

		this->DoneCurrent();

		initialized_ = true;
	}
}
bool RenderEngine::InitGLcontext()
{
	context_.reset(new QOpenGLContext());
	surface_.reset(new QOffscreenSurface());

	context_->setShareContext(QOpenGLContext::globalShareContext());

	if (!context_->create())
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, "RenderEngine::InitGLcontext: The opengl context is invalid.");
		return false;
	}

	QSurfaceFormat format;
	format.setSamples(4);
	context_->setFormat(format);
	surface_->setFormat(context_->format()); //sampling �߰�.
	surface_->create();
	return true;
}
void RenderEngine::InitRenderers()
{
	if (!gl_device_info_.is_valid)
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, "RenderEngine::InitRenderers: gl_device_info_ is invalid.");
		return;
	}

	gl_resource_mgr_->Initialize(gl_device_info_);
	if (!gl_resource_mgr_->IsValid())
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR, "RenderEngine::InitRenderers: gl_resource_mgr is invalid.");
		return;
	}

	for (int i = 0; i < VOL_TYPE_END; i++)
		RendererVolume((VolType)i).SettingsTFMaxAxisTexLength(gl_device_info_.max_tex_axis_size);

	VolumeRenderer::ShaderPrograms vol_shader;
	vol_shader.front_face_cube = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_FRONT_FACE_CUBE);
	vol_shader.front_face_final = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_FRONT_FACE_FINAL);
	vol_shader.back_face_cube = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_BACK_FACE_CUBE);
	vol_shader.ray_casting = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RAYCASTING);
	vol_shader.ray_firsthit = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RAY_FIRST_HIT);
	vol_shader.render_surface = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SURFACE);
	vol_shader.render_bone_density = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_BONEDENSITY);
	vol_shader.texturing_surface = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_TEXTURING_SURFACE);
	vol_shader.pick_object = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_PICK_OBJECT);
	vol_shader.render_screen = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_IMAGE);

	SliceRenderer::ShaderPrograms slice_shader;
	slice_shader.render_slice = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SLICE);
	slice_shader.render_screen = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_IMAGE);
	slice_shader.render_slice_nerve = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SLICE_NERVE);
	slice_shader.render_slice_implant = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SLICE_IMPLANT);
	slice_shader.render_slice_implant_wire = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SLICE_IMPLANT_WIRE);
	slice_shader.pick_object = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_PICK_OBJECT);

	for (int i = 0; i < VOL_TYPE_END; i++)
	{
		RendererVolume((VolType)i).set_programs(vol_shader);
		RendererSlice((VolType)i).set_programs(slice_shader);
	}

	ImageRenderer::ShaderPrograms image_shader;

	image_shader.render_image = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_IMAGE_WINDOWING);
	image_shader.render_mask = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_IMAGE);
	image_shader.render_surface = gl_resource_mgr_->GetProgramID(ShaderPack::SHADER_RENDER_SURFACE);

	RendererImage().set_programs(image_shader);
}
void RenderEngine::SetOpenGLDeviceInfo()
{
	common::Logger::instance()->PrintDebugMode("RenderEngine::SetOpenGLDeviceInfo()", "start");

	auto logger = Logger::instance();
#if 0
	const char* ver = (char*)glGetString(GL_VERSION);
	const char* ven = (char*)glGetString(GL_VENDOR);
	const char* ren = (char*)glGetString(GL_RENDERER);
#else
	QString ver = QString::fromStdString(std::string((char*)glGetString(GL_VERSION)));
	QString glsl_ver = QString::fromStdString(std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));
	QString ven = QString::fromStdString(std::string((char*)glGetString(GL_VENDOR)));
	QString ren = QString::fromStdString(std::string((char*)glGetString(GL_RENDERER)));
#endif

	qDebug() << "GPU vendor :" << ven;

	gl_device_info_.vendor = ven;

	logger->Print(LogType::INF, "--------------<OpenGL Info>---------------------");
	logger->Print(LogType::INF, "GL_VERSION : " + ver.toStdString());
	logger->Print(LogType::INF, "GL_SHADING_LANGUAGE_VERSION : " + glsl_ver.toStdString());
	logger->Print(LogType::INF, "GL_VENDOR : " + ven.toStdString());
	logger->Print(LogType::INF, "GL_RENDERER : " + ren.toStdString());

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_device_info_.max_tex_axis_size);  // �ִ� ������ ����
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gl_device_info_.max_3d_tex_size);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_device_info_.max_count_texture);

	if (ven.contains("ATI", Qt::CaseSensitive))
	{
		GLint params[4] = { 0 };
		//param[0] - total memory free in the pool
		//param[1] - largest available free block in the pool
		//param[2] - total auxiliary memory free
		//param[3] - largest auxiliary free block

		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, params);
		logger->Print(LogType::INF, "GL_TEXTURE_FREE_MEMORY_ATI : " + std::to_string(params[0]));
		gl_device_info_.curr_gpu_memory_kb = params[0];

		glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, params);
		logger->Print(LogType::INF, "GL_VBO_FREE_MEMORY_ATI : " + std::to_string(params[0]));

		glGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, params);
		logger->Print(LogType::INF, "GL_RENDERBUFFER_FREE_MEMORY_ATI : " + std::to_string(params[0]));

		glGetError();
	}
	else
	{
		if (ven.contains("Intel", Qt::CaseInsensitive))
		{
#if defined(_WIN32)
			MEMORYSTATUSEX statex;
			statex.dwLength = sizeof(statex);
			GlobalMemoryStatusEx(&statex);
			gl_device_info_.total_gpu_memory_kb = (statex.ullTotalPhys / 2) / 1024;
			gl_device_info_.curr_gpu_memory_kb = (statex.ullAvailPhys / 2) / 1024;
#endif
		}
		else
		{
			glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &gl_device_info_.total_gpu_memory_kb);
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gl_device_info_.curr_gpu_memory_kb);
		}

		logger->Print(LogType::INF, "GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY : " + std::to_string(gl_device_info_.total_gpu_memory_kb));
		logger->Print(LogType::INF, "GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM : " + std::to_string(gl_device_info_.curr_gpu_memory_kb));
	}

	logger->Print(LogType::INF, "GL_MAX_TEXTURE_SIZE : " + std::to_string(gl_device_info_.max_tex_axis_size));
	logger->Print(LogType::INF, "GL_MAX_3D_TEXTURE_SIZE : " + std::to_string(gl_device_info_.max_3d_tex_size));
	logger->Print(LogType::INF, "GL_MAX_TEXTURE_IMAGE_UNITS : " + std::to_string(gl_device_info_.max_count_texture));
	logger->Print(LogType::INF, "------------------------------------------------");

	gl_device_info_.is_valid = true;

	common::Logger::instance()->PrintDebugMode("RenderEngine::SetOpenGLDeviceInfo()", "end");
}
