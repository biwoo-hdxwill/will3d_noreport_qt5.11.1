#include "gl_resource_mgr.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

#include <QDebug>
#include <QString>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/shader_compiler.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3TF.h"

#include "texture_pack.h"

GLresourceMgr::GLresourceMgr()
{
	will3d_textures_.reset(new TexturePack);
	shader_compiler_.reset(new ShaderCompiler);
}

GLresourceMgr::~GLresourceMgr()
{
}

void GLresourceMgr::Initialize(const Will3DEngine::GLdeviceInfo& device_info)
{
	initialized_ = false;

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		common::Logger* logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "GLresourceMgr::Initialize: Please check the status of opengl.");
		//return;
	}

	gpu_vendor_ = device_info.vendor;
	available_gpu_memory_kb_ = device_info.curr_gpu_memory_kb;
	max_tex_axis_size_ = device_info.max_tex_axis_size;
	this->InstallShaderPrograms();

	initialized_ = true;
}

void GLresourceMgr::ClearGL()
{
	will3d_textures_->ClearGL();
	shader_compiler_->ClearPrograms();
}

void GLresourceMgr::SetVolumeTexture(const Will3DEngine::VolType & vol_type, const CW3Image3D & vol,
	unsigned int * handler, unsigned int * texture_id, int* down_factor)
{
	int vol_width = vol.width();
	int vol_height = vol.height();
	int vol_depth = vol.depth();
	unsigned short** data = vol.getData();

	if (vol_type == Will3DEngine::VolType::VOL_MAIN)
	{
		reserved_gpu_memory_kb_ = (int)((float)(vol_width * vol_height * vol_depth * sizeof(unsigned short)) / 1024);
	}

	*down_factor = this->GetDownSamplingRatio(vol_width, vol_height, vol_depth);

	if (vol_type == Will3DEngine::VolType::VOL_MAIN)
	{
		reserved_gpu_memory_kb_ = 0;
	}

	unsigned int vol_tex_handler = 0;
	if (*down_factor > 1)
	{
		std::vector<unsigned short> resized_vol;
		int finalw = (int)(((float)vol_width / *down_factor) + .5f);
		int finalh = (int)(((float)vol_height / *down_factor) + .5f);
		int finald = (int)(((float)vol_depth / *down_factor) + .5f);

		finald = std::max(1, finald);

		resized_vol.resize(finald*finalh*finalw);

		for (int z = 0; z < finald; z++)
		{
			int iz = z * finalh*finalw;
			for (int y = 0; y < finalh; y++)
			{
				int iy = y * finalw;
				for (int x = 0; x < finalw; x++)
				{
					resized_vol[iz + iy + x] = data[z*(*down_factor)][(x + y * vol_width)*(*down_factor)];
				}
			}
		}

		will3d_textures_->SetVolumeTexture(vol_type, &resized_vol[0], finalw, finalh, finald, handler);
	}
	else
	{
		will3d_textures_->SetVolumeTexture(vol_type, vol.getData(), vol_width, vol_height, vol_depth, handler);
	}

	qDebug() << "GLresourceMgr::SetVolumeTexture 1";

	*texture_id = (unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_VOL;
}

void GLresourceMgr::SetTfTexture(const CW3TF & tf, unsigned int* handler, unsigned int * texture_id)
{
	int tf_width, tf_height;

	tf_width = tf.size();
	tf_height = 1;
	if (tf_width > max_tex_axis_size_)
	{
		tf_height = (int)(tf_width / max_tex_axis_size_);
		tf_width = max_tex_axis_size_;
	}

	will3d_textures_->SetTfTexture(tf.getTF(), tf_width, tf_height, handler);

	*texture_id = (unsigned int)TexturePack::GL_TEXTURE_ID::GL_TEXTURE_TF;
}

void GLresourceMgr::ClearVolumeTexture(const Will3DEngine::VolType & vol_type)
{
	will3d_textures_->ClearVolumeTexture(vol_type);
}
void GLresourceMgr::ClearTf()
{
	will3d_textures_->ClearTf();
}

unsigned int GLresourceMgr::GetProgramID(const ShaderPack::SHADER & shader_id)
{
	return (shader_compiler_->GetProgramID(shader_id));
}

unsigned int GLresourceMgr::tmpGetVolTexHandler(const Will3DEngine::VolType & vol_type)
{
	return	will3d_textures_->tex_vol_handler(vol_type);
}
unsigned int GLresourceMgr::tmpGetTfTexHandler()
{
	return will3d_textures_->tex_tf_handler();
}

void GLresourceMgr::InstallShaderPrograms()
{
	for (int i = 0; i < ShaderPack::SHADER_END; i++)
	{
		QString vert_path, frag_path;
		ShaderPack::GetShaderFilePath((ShaderPack::SHADER)i, &vert_path, &frag_path);
		shader_compiler_->ShaderCompileAndAttachProgram(i, vert_path, frag_path);
	}
}

int GLresourceMgr::GetDownSamplingRatio(const int & width, const int & height, const int & depth)
{
	// resize
	float factor = 1.0f;
	bool graphic_card = false;

	if (gpu_vendor_.contains("nvidia", Qt::CaseInsensitive))
	{
		glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &available_gpu_memory_kb_);
		graphic_card = true;
	}
	else if (gpu_vendor_.contains("ATI", Qt::CaseSensitive))
	{
		GLint params[4] = { 0 };
		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, params);
		available_gpu_memory_kb_ = params[0];
		glGetError();
		graphic_card = true;
	}
	else if (gpu_vendor_.contains("Intel", Qt::CaseInsensitive))
	{
#if defined(_WIN32)
		MEMORYSTATUSEX statex;
		statex.dwLength = sizeof(statex);
		GlobalMemoryStatusEx(&statex);
		available_gpu_memory_kb_ = (statex.ullAvailPhys / 2) / 1024;
#endif
	}

	int alloc_texture_size_kb = (int)((float)(width * height * depth * sizeof(unsigned short)) / 1024);

	std::string log_msg = QString("current gpu mem : %1, volume texture size : %2").arg(available_gpu_memory_kb_).arg(alloc_texture_size_kb).toStdString();
	common::Logger::instance()->Print(common::LogType::INF, log_msg);

	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;

	int minimum_down_factor = 1;
	switch (volume_rendering_quality)
	{
	case GlobalPreferences::Quality2::High:
		minimum_down_factor = 1;
		break;
	case GlobalPreferences::Quality2::Low:
		minimum_down_factor = 2;
		break;
	}

#ifdef _WIN64
	if (!graphic_card)
#endif
	{
		if (minimum_down_factor == 1)
		{
			if (alloc_texture_size_kb >= (1024 * 1024))
			{
				minimum_down_factor = 2;
			}
		}
	}

	if (available_gpu_memory_kb_ == 0)
	{
		return minimum_down_factor;
	}

	if (alloc_texture_size_kb > available_gpu_memory_kb_ - reserved_gpu_memory_kb_)
	{
		common::Logger::instance()->Print(
			common::LogType::INF,
			"Reduce the size of the image so that the program can operate smoothly.");
		CW3MessageBox msg_box("Will3D", lang::LanguagePack::msg_24(), CW3MessageBox::Information);
		msg_box.setDetailedText(lang::LanguagePack::msg_49());
		msg_box.exec();
#if 1
		std::string reserved_gpu_memory_log = QString("Reserved GPU Memory : %1").arg(reserved_gpu_memory_kb_).toStdString();
		common::Logger::instance()->Print(common::LogType::INF, reserved_gpu_memory_log);
		factor = (float)(alloc_texture_size_kb + reserved_gpu_memory_kb_) / (float)available_gpu_memory_kb_;
#else
		factor = (float)alloc_texture_size_kb / (float)available_gpu_memory_kb_;
#endif
	}

#if 0
	int down_factor = 1;
	if (factor > 1.0f)
	{
		while (factor > down_factor*down_factor*down_factor)
		{
			++down_factor;
		}
	}
#else
	int down_factor = ceilf(factor);
#endif

	down_factor = std::max(down_factor, minimum_down_factor);

	std::string strDownFactor = QString("Volume downsize factor : %1").arg(down_factor).toStdString();
	common::Logger::instance()->Print(common::LogType::INF, strDownFactor);

	return down_factor;
}
