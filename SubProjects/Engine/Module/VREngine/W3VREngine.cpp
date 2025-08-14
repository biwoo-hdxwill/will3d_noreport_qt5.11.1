#include "W3VREngine.h"
/*=========================================================================

File:			class CW3VREngine
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-12-01
Last modify:	2016-04-21

=========================================================================*/
#include "../../Common/GLfunctions/WGLHeaders.h"
#include <qopenglwidget.h>

#include <QApplication>
#include <QImageWriter>
#include <QTextCodec>
#include <QDebug>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_otf.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/GLfunctions/W3GLFunctions.h"
#include "../../Common/GLfunctions/WGLSLprogram.h"

#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3Image2D.h"
#include "../../Resource/Resource/W3TF.h"
#include "../../Resource/Resource/W3ViewPlane.h"
#include "../../Resource/ResContainer/resource_container.h"

#include "../../UIModule/UIGLObjects/W3SurfacePlaneItem.h"
#include "../../UIModule/UIGLObjects/W3GLObject.h"
#include "../../UIModule/UIGLObjects/W3GLNerve.h"

#include "../MPREngine/W3MPREngine.h"
#include "../Will3DEngine/renderer_manager.h"

#include "W3Collide.h"
#include "W3ActiveBlock.h"
#include "W3VRengineTextures.h"
#include "W3Render3DParam.h"

#if defined(__APPLE__)
#include <OpenGL.h>
#endif

//#include "W3VRCPU.h"

#ifdef _DEBUG
#define ___DEBUG 1
#else
#define ___DEBUG 0
#endif
#define GPU_MEMORY_FREE 300 * 1024

namespace
{
	const glm::mat4 kIdentityMat = glm::mat4(1.0f);
	const int kActiveBlockSize = 16;
	const char* kRenderBothFailMessage = "Render3Dboth failed.";
}

using namespace VREngine;

CW3VREngine* CW3VREngine::instance_ = nullptr;

CW3VREngine::CW3VREngine(QOpenGLWidget *GLwidget)
	: m_pgGLWidget(GLwidget)
{
	common::Logger::instance()->PrintDebugMode("CW3VREngine", "start");

	Q_INIT_RESOURCE(shader);

	m_pMainVRparams[0] = nullptr;	// for First Volume 3D
	m_pMainVRparams[1] = nullptr;	// for Second Volume 3D
	m_pMainVRparams[2] = nullptr;	// for Panorama 3D
	m_pMainVRparams[3] = nullptr;	// for Left TMJ 3D
	m_pMainVRparams[4] = nullptr;	// for Right TMJ 3D

	m_vboPlane[0] = 0;
	m_vboPlane[1] = 0;
	m_vboPlane[2] = 0;

	vbo_plane_inverse_y_[0] = 0;
	vbo_plane_inverse_y_[1] = 0;
	vbo_plane_inverse_y_[2] = 0;

	m_vboCUBE[0] = 0;
	m_vboCUBE[1] = 0;
	m_vboCUBE[2] = 0;

	m_vboCUBEdivided[0] = 0;
	m_vboCUBEdivided[1] = 0;
	m_vboCUBEdivided[2] = 0;

	m_SimpleSR_draw_buffers[0] = GL_COLOR_ATTACHMENT0;
	m_SimpleSR_draw_buffers[1] = GL_COLOR_ATTACHMENT1;
	m_SimpleSR_draw_buffers[2] = GL_COLOR_ATTACHMENT2;
	m_SimpleSR_draw_buffers[3] = GL_COLOR_ATTACHMENT3;
	m_SimpleSR_draw_buffers[4] = GL_COLOR_ATTACHMENT4;
	m_SimpleSR_draw_buffers[5] = GL_COLOR_ATTACHMENT5;
	m_SimpleSR_draw_buffers[6] = GL_COLOR_ATTACHMENT6;

	m_texNumPlane = GL_TEXTURE0; m_texNumPlane_ = 0;
	m_texNumVol3D = GL_TEXTURE1; m_texNumVol3D_ = 1;
	m_texNumTF2D = GL_TEXTURE2; m_texNumTF2D_ = 2;

	m_texNumRC = GL_TEXTURE5; m_texNumRC_ = 5;

	m_texNumFACE = GL_TEXTURE8; m_texNumFACE_ = 8;
	m_texNumBspline = GL_TEXTURE9; m_texNumBspline_ = 9;
	m_texNumBsplinePrime = GL_TEXTURE10; m_texNumBsplinePrime_ = 10;
	m_texNumMaskPlane = GL_TEXTURE11; m_texNumMaskPlane_ = 11;
	m_texNumTF2DTeeth = GL_TEXTURE12; m_texNumTF2DTeeth_ = 12;
	m_texNumSegTmjMask = GL_TEXTURE13; m_texNumSegTmjMask_ = 13;

	m_texNumSimpleSR[0] = GL_TEXTURE20; m_texNumSimpleSR_[0] = 20;
	m_texNumSimpleSR[1] = GL_TEXTURE21; m_texNumSimpleSR_[1] = 21;
	m_texNumSimpleSR[2] = GL_TEXTURE22; m_texNumSimpleSR_[2] = 22;
	m_texNumSimpleSR[3] = GL_TEXTURE23; m_texNumSimpleSR_[3] = 23;
	m_texNumSimpleSR[4] = GL_TEXTURE24; m_texNumSimpleSR_[4] = 24;
	m_texNumSimpleSR[5] = GL_TEXTURE25; m_texNumSimpleSR_[5] = 25;
	m_texNumSimpleSR[6] = GL_TEXTURE26; m_texNumSimpleSR_[6] = 26;

	//initOTFview();

	m_camFOV = 0.0f;
	m_modelReori = kIdentityMat;
	m_model = kIdentityMat;
	m_view = kIdentityMat;
	m_projection = kIdentityMat;
	m_mvp = kIdentityMat;

	m_VolRangeGL.setX(0.0f);
	m_VolRangeGL.setY(0.0f);
	m_VolRangeGL.setZ(0.0f);

	// by jdk 161122 tmj
	m_texSegTmjMask = 0;

	ApplyPreferences();

	common::Logger::instance()->PrintDebugMode("CW3VREngine", "end");
}

CW3VREngine::~CW3VREngine()
{
	SAFE_DELETE_OBJECT(m_pMainVRparams[0]);
	SAFE_DELETE_OBJECT(m_pMainVRparams[1]);

	deleteVBOs();
	deleteTextures();

	if (m_pgGLWidget->context())
	{
		m_pgGLWidget->makeCurrent();

		glDeleteProgram(m_PROGplaneDisplay); m_PROGplaneDisplay = 0;
		glDeleteProgram(m_PROGfrontfaceCUBE); m_PROGfrontfaceCUBE = 0;
		glDeleteProgram(m_PROGbackfaceCUBE); m_PROGbackfaceCUBE = 0;
		glDeleteProgram(m_PROGfrontfaceFinal); m_PROGfrontfaceFinal = 0;
		glDeleteProgram(m_PROGvolumeFirstHitforFaceSim); m_PROGvolumeFirstHitforFaceSim = 0;
		glDeleteProgram(m_PROGraycasting); m_PROGraycasting = 0;
		glDeleteProgram(m_PROGendoraycasting); m_PROGendoraycasting = 0;
		glDeleteProgram(m_PROGfinal); m_PROGfinal = 0;
		glDeleteProgram(m_PROGslice); m_PROGslice = 0;
		glDeleteProgram(m_PROGsliceCanal); m_PROGsliceCanal = 0;
		glDeleteProgram(m_PROGSR); m_PROGSR = 0;
		glDeleteProgram(m_PROGSRdepth); m_PROGSRdepth = 0;
		glDeleteProgram(m_PROGsimpleSR); m_PROGsimpleSR = 0;
		glDeleteProgram(m_PROGanno); m_PROGanno = 0;
		glDeleteProgram(m_PROGimplant); m_PROGimplant = 0;
		glDeleteProgram(m_PROGsurface); m_PROGsurface = 0;
		glDeleteProgram(m_PROGpick); m_PROGpick = 0;
		glDeleteProgram(m_PROGpickWithCoord); m_PROGpickWithCoord = 0;
		glDeleteProgram(m_PROGendoPlane); m_PROGendoPlane = 0;
		glDeleteProgram(m_PROGdepthSetting); m_PROGdepthSetting = 0;
		glDeleteProgram(m_PROGdepthSetting2); m_PROGdepthSetting2 = 0;
		//thyoo
		glDeleteProgram(m_thyShd.m_PROGbackfaceCUBE); m_thyShd.m_PROGbackfaceCUBE = 0;
		glDeleteProgram(m_thyShd.m_PROGfrontfaceCUBE); m_thyShd.m_PROGfrontfaceCUBE = 0;
		glDeleteProgram(m_thyShd.m_PROGfrontfaceFinal); m_thyShd.m_PROGfrontfaceFinal = 0;
		glDeleteProgram(m_thyShd.m_PROGraycasting); m_thyShd.m_PROGraycasting = 0;
		glDeleteProgram(m_PROGsurfaceTexture); m_PROGsurfaceTexture = 0;
		glDeleteProgram(m_PROGsurfaceEndo); m_PROGsurfaceEndo = 0;
		//thyoo end
		glDeleteProgram(m_PROGdisplacementSurface); m_PROGdisplacementSurface = 0;

		m_pgGLWidget->doneCurrent();
	}

	//SAFE_DELETE_OBJECT(m_pgOTFScene);
	//SAFE_DELETE_OBJECT(m_pVRCPU);

	///占쏙옙占쌘듸옙
}

void CW3VREngine::reset()
{
	m_isTFinitialized = false;
	m_bIsMIP = false;
	m_modelReori = kIdentityMat;
	m_camFOV = 0.0f;
	m_model = kIdentityMat;
	m_view = kIdentityMat;
	m_projection = kIdentityMat;
	m_mvp = kIdentityMat;

	m_VolRangeGL.setX(0.0f);
	m_VolRangeGL.setY(0.0f);
	m_VolRangeGL.setZ(0.0f);

	SAFE_DELETE_OBJECT(m_pMainVRparams[0]);
	SAFE_DELETE_OBJECT(m_pMainVRparams[1]);

	//deleteVBOs();
	deleteTextures();
}

void CW3VREngine::deleteVRparams(int id)
{
	SAFE_DELETE_OBJECT(m_pMainVRparams[id]);
}

void CW3VREngine::deleteVBOs()
{
	if (m_pgGLWidget == nullptr)
	{
		return;
	}

	if (m_pgGLWidget->context())
	{
		m_pgGLWidget->makeCurrent();

		if (m_vboPlane[0])
		{
			glDeleteBuffers(3, m_vboPlane);
			m_vboPlane[0] = 0;
			m_vboPlane[1] = 0;
			m_vboPlane[2] = 0;
		}

		if (vbo_plane_inverse_y_[0])
		{
			glDeleteBuffers(3, vbo_plane_inverse_y_);
			vbo_plane_inverse_y_[0] = 0;
			vbo_plane_inverse_y_[1] = 0;
			vbo_plane_inverse_y_[2] = 0;
		}

		if (m_vboCUBE[0])
		{
			glDeleteBuffers(3, m_vboCUBE);
			m_vboCUBE[0] = 0;
			m_vboCUBE[1] = 0;
			m_vboCUBE[2] = 0;
		}

		if (m_vboCUBEdivided[0])
		{
			glDeleteBuffers(3, m_vboCUBEdivided);
			m_vboCUBEdivided[0] = 0;
			m_vboCUBEdivided[1] = 0;
			m_vboCUBEdivided[2] = 0;
		}

		m_pgGLWidget->doneCurrent();
	}
}

void CW3VREngine::deleteTextures()
{
	if (m_pgGLWidget->context())
	{
		m_pgGLWidget->makeCurrent();

		if (m_texTF2DHandler)
		{
			glDeleteTextures(1, &m_texTF2DHandler);
			m_texTF2DHandler = 0;
		}

		if (m_texTF2DTeethHandler)
		{
			glDeleteTextures(1, &m_texTF2DTeethHandler);
			m_texTF2DTeethHandler = 0;
		}

		//if (m_texVol3DHandler)
		//{
		//	glDeleteTextures(1, &m_texVol3DHandler);
		//	m_texVol3DHandler = 0;
		//}

		if (m_texBsplineHandler)
		{
			glDeleteTextures(1, &m_texBsplineHandler);
			m_texBsplineHandler = 0;
		}

		if (m_texBsplinePrimeHandler)
		{
			glDeleteTextures(1, &m_texBsplinePrimeHandler);
			m_texBsplinePrimeHandler = 0;
		}

		m_pgGLWidget->doneCurrent();
	}
}

void CW3VREngine::SetVRreconType(const QString& otf_name)
{
	if (otf_name == common::otf_preset::MIP)
	{
		this->setXRAY(false);
		this->setMIP(true);
	}
	else if (otf_name == common::otf_preset::XRAY)
	{
		this->setMIP(false);
		this->setXRAY(true);
	}
	else
	{
		this->setMIP(false);
		this->setXRAY(false);
	}
}

void CW3VREngine::initGLSL()
{
	using common::Logger;
	using common::LogType;
	auto logger = Logger::instance();

	logger->Print(LogType::INF, "start initGLSL");

#if defined(__APPLE__)
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewErr));
		logger->Print(LogType::ERR, "glewInit failed");
	}

	int curGPUTexMemKb = 0;

	GLint glRendId = 0;
	CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &glRendId);

	CGLRendererInfoObj rendObj = NULL;
	//    CGOpenGLDisplayMask dispMask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
	GLint rendNb = 0;
	CGLQueryRendererInfo(0xFFFFFFFF, &rendObj, &rendNb);
	for (GLint rendIter = 0; rendIter < rendNb; ++rendIter)
	{
		GLint rendId = 0;
		if (CGLDescribeRenderer(rendObj, rendIter, kCGLRPRendererID, &rendId) != kCGLNoError
			|| rendId != glRendId)
		{
			continue;
		}

#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
		if (CGLDescribeRenderer(rendObj, rendIter, kCGLRPVideoMemoryMegabytes, &m_nCurGPUMemKb) == kCGLNoError)
		{
			m_nCurGPUMemKb *= 1024;
		}
		if (CGLDescribeRenderer(rendObj, rendIter, kCGLRPTextureMemoryMegabytes, &curGPUTexMemKb) == kCGLNoError)
		{
			curGPUTexMemKb *= 1024;
		}
#else
		if (CGLDescribeRenderer(rendObj, rendIter, kCGLRPVideoMemory, &m_nCurGPUMemKb) == kCGLNoError)
		{
			m_nCurGPUMemKb /= 1024;
		}
		if (CGLDescribeRenderer(rendObj, rendIter, kCGLRPTextureMemory, &curGPUTexMemKb) == kCGLNoError)
		{
			curGPUTexMemKb /= 1024;
		}
#endif
	}

	if (m_nCurGPUMemKb <= 0)
	{
		logger->Print(LogType::ERR, "current gpu memory <= 0");
	}

	std::string ver = (const char*)glGetString(GL_VERSION);
	std::string ven = (const char*)glGetString(GL_VENDOR);
	std::string ren = (const char*)glGetString(GL_RENDERER);

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_nMaxTexAxisSize);  // 占쌍댐옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_nMax3DTexSize);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_nMaxNTexture);

	logger->Print(LogType::INF, "--------------<OpenGL Info>---------------------");
	logger->Print(LogType::INF, "GL_VERSION : " + std::string(ver));
	logger->Print(LogType::INF, "GL_VENDOR : " + std::string(ven));
	logger->Print(LogType::INF, "GL_RENDERER : " + std::string(ren));
	logger->Print(LogType::INF, "GPU_MEMORY : " + std::to_string(m_nCurGPUMemKb));
	logger->Print(LogType::INF, "GPU_TEXTURE_MEMORY : " + std::to_string(curGPUTexMemKb));
	logger->Print(LogType::INF, "GL_MAX_TEXTURE_SIZE : " + std::to_string(m_nMaxTexAxisSize));
	logger->Print(LogType::INF, "GL_MAX_3D_TEXTURE_SIZE : " + std::to_string(m_nMax3DTexSize));
	logger->Print(LogType::INF, "GL_MAX_TEXTURE_IMAGE_UNITS : " + std::to_string(m_nMaxNTexture));
	logger->Print(LogType::INF, "------------------------------------------------");
#else
	logger->Print(LogType::INF, "start glewInit");

	// glew init
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		logger->Print(LogType::ERR, "glewInit failed");
	}

	logger->Print(LogType::INF, "end glewInit");

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

	logger->Print(LogType::INF, "--------------<OpenGL Info>---------------------");
	logger->Print(LogType::INF, "GL_VERSION : " + ver.toStdString());
	logger->Print(LogType::INF, "GL_SHADING_LANGUAGE_VERSION : " + glsl_ver.toStdString());
	logger->Print(LogType::INF, "GL_VENDOR : " + ven.toStdString());
	logger->Print(LogType::INF, "GL_RENDERER : " + ren.toStdString());

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_nMaxTexAxisSize);  // 占쌍댐옙 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_nMax3DTexSize);
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_nMaxNTexture);

	if (ven.contains("ATI", Qt::CaseSensitive))
	{
		GLint params[4] = { 0 };
		//param[0] - total memory free in the pool
		//param[1] - largest available free block in the pool
		//param[2] - total auxiliary memory free
		//param[3] - largest auxiliary free block

		glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, params);
		logger->Print(LogType::INF, "GL_TEXTURE_FREE_MEMORY_ATI : " + std::to_string(params[0]));
		m_nCurGPUMemKb = params[0];

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
			m_nTotalGPUMemKb = (statex.ullTotalPhys / 2) / 1024;
			m_nCurGPUMemKb = (statex.ullAvailPhys / 2) / 1024;
#endif
		}
		else
		{
			glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &m_nTotalGPUMemKb);
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &m_nCurGPUMemKb);
		}

		logger->Print(LogType::INF, "GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY : " + std::to_string(m_nTotalGPUMemKb));
		logger->Print(LogType::INF, "GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM : " + std::to_string(m_nCurGPUMemKb));
	}

	logger->Print(LogType::INF, "GL_MAX_TEXTURE_SIZE : " + std::to_string(m_nMaxTexAxisSize));
	logger->Print(LogType::INF, "GL_MAX_3D_TEXTURE_SIZE : " + std::to_string(m_nMax3DTexSize));
	logger->Print(LogType::INF, "GL_MAX_TEXTURE_IMAGE_UNITS : " + std::to_string(m_nMaxNTexture));
	logger->Print(LogType::INF, "------------------------------------------------");
#endif
}

void CW3VREngine::initGLContextValues()
{
	glLineWidth(1);
}

void CW3VREngine::initPlaneVertex()
{
	//// plane vertex ////
	//// 3     2      ////
	//// 0     1      ////
	//////////////////////

	m_vertPlaneCoord[0] = -1.0f, m_vertPlaneCoord[1] = -1.0f, m_vertPlaneCoord[2] = 0.0f;
	m_vertPlaneCoord[3] = 1.0f, m_vertPlaneCoord[4] = -1.0f, m_vertPlaneCoord[5] = 0.0f;
	m_vertPlaneCoord[6] = 1.0f, m_vertPlaneCoord[7] = 1.0f, m_vertPlaneCoord[8] = 0.0f;
	m_vertPlaneCoord[9] = -1.0f, m_vertPlaneCoord[10] = 1.0f, m_vertPlaneCoord[11] = 0.0f;

	m_texPlaneCoord[0] = 0.0f, m_texPlaneCoord[1] = 1.0f;
	m_texPlaneCoord[2] = 1.0f, m_texPlaneCoord[3] = 1.0f;
	m_texPlaneCoord[4] = 1.0f, m_texPlaneCoord[5] = 0.0f;
	m_texPlaneCoord[6] = 0.0f, m_texPlaneCoord[7] = 0.0f;

	m_tex_inverseY_PlaneCoord[0] = 0.0f, m_tex_inverseY_PlaneCoord[1] = 0.0f;
	m_tex_inverseY_PlaneCoord[2] = 1.0f, m_tex_inverseY_PlaneCoord[3] = 0.0f;
	m_tex_inverseY_PlaneCoord[4] = 1.0f, m_tex_inverseY_PlaneCoord[5] = 1.0f;
	m_tex_inverseY_PlaneCoord[6] = 0.0f, m_tex_inverseY_PlaneCoord[7] = 1.0f;

	m_IndexPlane[0] = 0;
	m_IndexPlane[1] = 1;
	m_IndexPlane[2] = 3;
	m_IndexPlane[3] = 3;
	m_IndexPlane[4] = 1;
	m_IndexPlane[5] = 2;

	initVBOplane();
	InitVBOPlaneInverseY();
}

void CW3VREngine::initCUBEVertex()
{
	m_vertCUBECoord[0] = 1.0f, m_vertCUBECoord[1] = 1.0f, m_vertCUBECoord[2] = -1.0f;
	m_vertCUBECoord[3] = 1.0f, m_vertCUBECoord[4] = 1.0f, m_vertCUBECoord[5] = 1.0f;
	m_vertCUBECoord[6] = 1.0f, m_vertCUBECoord[7] = -1.0f, m_vertCUBECoord[8] = -1.0f;
	m_vertCUBECoord[9] = 1.0f, m_vertCUBECoord[10] = -1.0f, m_vertCUBECoord[11] = 1.0f;

	m_vertCUBECoord[12] = -1.0f, m_vertCUBECoord[13] = 1.0f, m_vertCUBECoord[14] = -1.0f;
	m_vertCUBECoord[15] = -1.0f, m_vertCUBECoord[16] = 1.0f, m_vertCUBECoord[17] = 1.0f;
	m_vertCUBECoord[18] = -1.0f, m_vertCUBECoord[19] = -1.0f, m_vertCUBECoord[20] = -1.0f;
	m_vertCUBECoord[21] = -1.0f, m_vertCUBECoord[22] = -1.0f, m_vertCUBECoord[23] = 1.0f;

	//m_texCUBECoord[0] = 1.0f, m_texCUBECoord[1] = 1.0f,  m_texCUBECoord[2] = 0.0f;
	//m_texCUBECoord[3] = 1.0f, m_texCUBECoord[4] = 1.0f,  m_texCUBECoord[5] = 1.0f;
	//m_texCUBECoord[6] = 1.0f, m_texCUBECoord[7] = 0.0f,  m_texCUBECoord[8] = 0.0f;
	//m_texCUBECoord[9] = 1.0f, m_texCUBECoord[10] = 0.0f, m_texCUBECoord[11] = 1.0f;
	//m_texCUBECoord[12] = 0.0f, m_texCUBECoord[13] = 1.0f, m_texCUBECoord[14] = 0.0f;
	//m_texCUBECoord[15] = 0.0f, m_texCUBECoord[16] = 1.0f, m_texCUBECoord[17] = 1.0f;
	//m_texCUBECoord[18] = 0.0f, m_texCUBECoord[19] = 0.0f, m_texCUBECoord[20] = 0.0f;
	//m_texCUBECoord[21] = 0.0f, m_texCUBECoord[22] = 0.0f, m_texCUBECoord[23] = 1.0f;
	//m_texCUBECoord[0] = 1.0f, m_texCUBECoord[1] = 0.0f,  m_texCUBECoord[2] = 0.0f;
	//m_texCUBECoord[3] = 1.0f, m_texCUBECoord[4] = 0.0f,  m_texCUBECoord[5] = 1.0f;
	//m_texCUBECoord[6] = 1.0f, m_texCUBECoord[7] = 1.0f,  m_texCUBECoord[8] = 0.0f;
	//m_texCUBECoord[9] = 1.0f, m_texCUBECoord[10] = 1.0f, m_texCUBECoord[11] = 1.0f;
	//m_texCUBECoord[12] = 0.0f, m_texCUBECoord[13] = 0.0f, m_texCUBECoord[14] = 0.0f;
	//m_texCUBECoord[15] = 0.0f, m_texCUBECoord[16] = 0.0f, m_texCUBECoord[17] = 1.0f;
	//m_texCUBECoord[18] = 0.0f, m_texCUBECoord[19] = 1.0f, m_texCUBECoord[20] = 0.0f;
	//m_texCUBECoord[21] = 0.0f, m_texCUBECoord[22] = 1.0f, m_texCUBECoord[23] = 1.0f;

	// 占쏙옙占쏙옙
	m_texCUBECoord[0] = 1.0f, m_texCUBECoord[1] = 1.0f, m_texCUBECoord[2] = 0.0f;
	m_texCUBECoord[3] = 1.0f, m_texCUBECoord[4] = 1.0f, m_texCUBECoord[5] = 1.0f;
	m_texCUBECoord[6] = 1.0f, m_texCUBECoord[7] = 0.0f, m_texCUBECoord[8] = 0.0f;
	m_texCUBECoord[9] = 1.0f, m_texCUBECoord[10] = 0.0f, m_texCUBECoord[11] = 1.0f;

	m_texCUBECoord[12] = 0.0f, m_texCUBECoord[13] = 1.0f, m_texCUBECoord[14] = 0.0f;
	m_texCUBECoord[15] = 0.0f, m_texCUBECoord[16] = 1.0f, m_texCUBECoord[17] = 1.0f;
	m_texCUBECoord[18] = 0.0f, m_texCUBECoord[19] = 0.0f, m_texCUBECoord[20] = 0.0f;
	m_texCUBECoord[21] = 0.0f, m_texCUBECoord[22] = 0.0f, m_texCUBECoord[23] = 1.0f;

	// draw it counter-clockwise
	// front: 1 5 7 3
	// back: 0 2 6 4
	// left占쏙옙0 1 3 2
	// right:7 5 4 6
	// up: 2 3 7 6
	// down: 1 0 4 5

	m_IndexCUBE[0] = 1, m_IndexCUBE[1] = 5, m_IndexCUBE[2] = 3;
	m_IndexCUBE[3] = 3, m_IndexCUBE[4] = 5, m_IndexCUBE[5] = 7;

	m_IndexCUBE[6] = 0, m_IndexCUBE[7] = 2, m_IndexCUBE[8] = 4;
	m_IndexCUBE[9] = 4, m_IndexCUBE[10] = 2, m_IndexCUBE[11] = 6;

	m_IndexCUBE[12] = 0, m_IndexCUBE[13] = 1, m_IndexCUBE[14] = 2;
	m_IndexCUBE[15] = 2, m_IndexCUBE[16] = 1, m_IndexCUBE[17] = 3;

	m_IndexCUBE[18] = 7, m_IndexCUBE[19] = 5, m_IndexCUBE[20] = 6;
	m_IndexCUBE[21] = 6, m_IndexCUBE[22] = 5, m_IndexCUBE[23] = 4;

	m_IndexCUBE[24] = 2, m_IndexCUBE[25] = 3, m_IndexCUBE[26] = 6;
	m_IndexCUBE[27] = 6, m_IndexCUBE[28] = 3, m_IndexCUBE[29] = 7;

	m_IndexCUBE[30] = 1, m_IndexCUBE[31] = 0, m_IndexCUBE[32] = 5;
	m_IndexCUBE[33] = 5, m_IndexCUBE[34] = 0, m_IndexCUBE[35] = 4;

	CW3GLFunctions::initVBO(m_vboCUBE, m_vertCUBECoord, m_texCUBECoord, 24, m_IndexCUBE, 36);
}

// 占쏙옙占쏙옙六占?
void CW3VREngine::recompileRaycasting(void)
{
	glDeleteProgram(m_PROGbackfaceCUBE);
	glDeleteProgram(m_PROGfrontfaceCUBE);
	glDeleteProgram(m_PROGfrontfaceFinal);
	glDeleteProgram(m_PROGvolumeFirstHitforFaceSim);
	glDeleteProgram(m_PROGraycasting);
	glDeleteProgram(m_thyShd.m_PROGbackfaceCUBE);
	glDeleteProgram(m_thyShd.m_PROGfrontfaceCUBE);
	glDeleteProgram(m_thyShd.m_PROGfrontfaceFinal);
	glDeleteProgram(m_thyShd.m_PROGraycasting);
	glDeleteProgram(m_PROGimplant);
	glDeleteProgram(m_PROGSR);
	glDeleteProgram(m_PROGSRdepth);
	glDeleteProgram(m_PROGsurface);

#if defined(__APPLE__)
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.frag"), m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_final.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_final.frag"), m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backface.frag"), m_PROGbackfaceCUBE);
#ifndef WILL3D_LIGHT
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_integ.frag"), m_PROGraycasting);
#else
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_integ_light.frag"), m_PROGraycasting);
#endif
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface.frag"), m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface_final.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface_final.frag"), m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/backface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/backface.frag"), m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/raycasting.frag"), m_thyShd.m_PROGraycasting);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceFirstHit.frag"), m_PROGfrontfacefirsthitCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_firsthit.frag"), m_PROGrayfirsthit);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceforFaceSim.frag"), m_PROGfrontfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backfaceforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backfaceforFaceSim.frag"), m_PROGbackfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/volumefirsthitforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/volumefirsthitforFaceSim.frag"), m_PROGvolumeFirstHitforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_finalforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_finalforFaceSim.frag"), m_PROGfrontfaceFinalforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/implant.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/implant.frag"), m_PROGimplant);
#else
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface.vert"), QString("../../Will3D/shader/Test/frontface.frag"), m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface_final.vert"), QString("../../Will3D/shader/Test/frontface_final.frag"), m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/backface.vert"), QString("../../Will3D/shader/Test/backface.frag"), m_PROGbackfaceCUBE);
#ifndef WILL3D_LIGHT
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/raycasting.vert"), QString("../../Will3D/shader/Test/raycasting_integ.frag"), m_PROGraycasting);
#else
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/raycasting.vert"), QString("../../Will3D/shader/Test/raycasting_integ_light.frag"), m_PROGraycasting);
#endif
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/frontface.vert"), QString("../../Will3D/shader/thyoo/frontface.frag"), m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/frontface_final.vert"), QString("../../Will3D/shader/thyoo/frontface_final.frag"), m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/backface.vert"), QString("../../Will3D/shader/thyoo/backface.frag"), m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/raycasting.vert"), QString("../../Will3D/shader/thyoo/raycasting.frag"), m_thyShd.m_PROGraycasting);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/volumefirsthitforFaceSim.vert"), QString("../../Will3D/shader/Test/volumefirsthitforFaceSim.frag"), m_PROGvolumeFirstHitforFaceSim);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/implant.vert"), QString("../../Will3D/shader/Test/implant.frag"), m_PROGimplant);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/SR.vert"), QString("../../Will3D/shader/Test/SR.frag"), m_PROGSR);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/SRdepth.vert"), QString("../../Will3D/shader/Test/SRdepth.frag"), m_PROGSRdepth);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/surface.vert"), QString("../../Will3D/shader/Test/surface.frag"), m_PROGsurface);
#endif
	mat4 mvpForFinal = kIdentityMat;
	mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
	mvpForFinal = projection * view;

	glUseProgram(m_PROGraycasting);
	WGLSLprogram::setUniform(m_PROGraycasting, "TFxincrease", m_TFxincreasetest);
	WGLSLprogram::setUniform(m_PROGraycasting, "TFyincrease", m_TFyincreasetest);
	WGLSLprogram::setUniform(m_PROGraycasting, "MaxTexSize", m_nMaxTexAxisSize);

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TFxincrease", m_TFxincreasetest);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TFyincrease", m_TFyincreasetest);

	glUseProgram(m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_thyShd.m_PROGfrontfaceFinal, "MVP", mvpForFinal);
	//WGLSLprogram::setUniform(m_thyShd.m_PROGfrontfaceFinal, "MaxTexSize", m_nMaxTexAxisSize);

	glUseProgram(m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::setUniform(m_thyShd.m_PROGfrontfaceCUBE, "VolTexBias", m_pMainVRparams[0]->m_volTexBias);
	WGLSLprogram::setUniform(m_thyShd.m_PROGfrontfaceCUBE, "VolTexTransformMat", mat4(1.0));

	glUseProgram(m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::setUniform(m_thyShd.m_PROGbackfaceCUBE, "VolTexBias", m_pMainVRparams[0]->m_volTexBias);
	WGLSLprogram::setUniform(m_thyShd.m_PROGbackfaceCUBE, "VolTexTransformMat", mat4(1.0));

	glUseProgram(m_thyShd.m_PROGraycasting);
	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "VolTexelSize", m_pMainVRparams[0]->m_volTexelSize);
	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "MaxTexSize", m_nMaxTexAxisSize);

	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, m_pMainVRparams[0]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "TransferFunc", m_texNumTF2D_);

	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "MVP", mvpForFinal);
}

void CW3VREngine::initPrograms()
{
#if __DEBUG

	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/planeDisplay.vert"), QString("../../Will3D/shader/planeDisplay.frag"), m_PROGplaneDisplay);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface.vert"), QString("../../Will3D/shader/Test/frontface.frag"), m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface_final.vert"), QString("../../Will3D/shader/Test/frontface_final.frag"), m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface.vert"), QString("../../Will3D/shader/Test/frontfaceFirstHit.frag"), m_PROGfrontfacefirsthitCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/backface.vert"), QString("../../Will3D/shader/Test/backface.frag"), m_PROGbackfaceCUBE);
#ifndef WILL3D_LIGHT
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/raycasting.vert"), QString("../../Will3D/shader/Test/raycasting_integ.frag"), m_PROGraycasting);
#else
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/raycasting.vert"), QString("../../Will3D/shader/Test/raycasting_integ_light.frag"), m_PROGraycasting);
#endif
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/raycasting.vert"), QString("../../Will3D/shader/Test/raycasting_firsthit.frag"), m_PROGrayfirsthit);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/Final.vert"), QString("../../Will3D/shader/Test/Final.frag"), m_PROGfinal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/slice.vert"), QString("../../Will3D/shader/slice.frag"), m_PROGslice);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/sliceCanal.vert"), QString("../../Will3D/shader/sliceCanal.frag"), m_PROGsliceCanal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/SR.vert"), QString("../../Will3D/shader/Test/SR.frag"), m_PROGSR);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/SRdepth.vert"), QString("../../Will3D/shader/Test/SRdepth.frag"), m_PROGSRdepth);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/SRcoord.vert"), QString("../../Will3D/shader/Test/SRcoord.frag"), m_PROGSRcoord);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/PR.vert"), QString("../../Will3D/shader/Test/PR.frag"), m_PROGPR);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/simpleSR.vert"), QString("../../Will3D/shader/Test/simpleSR.frag"), m_PROGsimpleSR);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/anno.vert"), QString("../../Will3D/shader/Test/anno.frag"), m_PROGanno);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/implant.vert"), QString("../../Will3D/shader/Test/implant.frag"), m_PROGimplant);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/surface.vert"), QString("../../Will3D/shader/Test/surface.frag"), m_PROGsurface);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/picking.vert"), QString("../../Will3D/shader/Test/picking.frag"), m_PROGpick);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/pickWithCoord.vert"), QString("../../Will3D/shader/Test/pickWithCoord.frag"), m_PROGpickWithCoord);
	// by jdk 160407 endo
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/endoPlane.vert"), QString("../../Will3D/shader/Test/endoPlane.frag"), m_PROGendoPlane);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/DepthSetting.vert"), QString("../../Will3D/shader/Test/DepthSetting.frag"), m_PROGdepthSetting);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/depthSetting2.vert"), QString("../../Will3D/shader/Test/depthSetting2.frag"), m_PROGdepthSetting2);
	//thyoo
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/frontface.vert"), QString("../../Will3D/shader/thyoo/frontface.frag"), m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/frontface_final.vert"), QString("../../Will3D/shader/thyoo/frontface_final.frag"), m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/backface.vert"), QString("../../Will3D/shader/thyoo/backface.frag"), m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/thyoo/raycasting.vert"), QString("../../Will3D/shader/thyoo/raycasting.frag"), m_thyShd.m_PROGraycasting);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/surface_texture.vert"), QString("../../Will3D/shader/surface_texture.frag"), m_PROGsurfaceTexture);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/surface_endo.vert"), QString("../../Will3D/shader/Test/surface_endo.frag"), m_PROGsurfaceEndo);
	//thyoo end
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontfaceforFaceSim.vert"), QString("../../Will3D/shader/Test/frontfaceforFaceSim.frag"), m_PROGfrontfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/backfaceforFaceSim.vert"), QString("../../Will3D/shader/Test/backfaceforFaceSim.frag"), m_PROGbackfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/volumefirsthitforFaceSim.vert"), QString("../../Will3D/shader/Test/volumefirsthitforFaceSim.frag"), m_PROGvolumeFirstHitforFaceSim);
	WGLSLprogram::createShaderProgram(QString("../../Will3D/shader/Test/frontface_finalforFaceSim.vert"), QString("../../Will3D/shader/Test/frontface_finalforFaceSim.frag"), m_PROGfrontfaceFinalforFaceSim);
	// jhyoon
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/MeshFillVertexColor_tex.vert"), QString(":/shader/Test/MeshFillVertexColor_tex.frag"), m_PROGdisplacementSurface);
	return;
#endif

#if defined(__APPLE__)
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/planeDisplay.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/planeDisplay.frag"), m_PROGplaneDisplay);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.frag"), m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_final.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_final.frag"), m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceFirstHit.frag"), m_PROGfrontfacefirsthitCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backface.frag"), m_PROGbackfaceCUBE);
#ifndef WILL3D_LIGHT
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_integ.frag"), m_PROGraycasting);
#else
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_integ_light.frag"), m_PROGraycasting);
#endif
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/raycasting_firsthit.frag"), m_PROGrayfirsthit);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/Final.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/Final.frag"), m_PROGfinal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/slice.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/slice.frag"), m_PROGslice);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/sliceCanal.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/sliceCanal.frag"), m_PROGsliceCanal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SR.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SR.frag"), m_PROGSR);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SRdepth.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SRdepth.frag"), m_PROGSRdepth);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SRcoord.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/SRcoord.frag"), m_PROGSRcoord);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/PR.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/PR.frag"), m_PROGPR);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/simpleSR.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/simpleSR.frag"), m_PROGsimpleSR);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/anno.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/anno.frag"), m_PROGanno);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/implant.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/implant.frag"), m_PROGimplant);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/surface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/surface.frag"), m_PROGsurface);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/picking.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/picking.frag"), m_PROGpick);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/pickWithCoord.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/pickWithCoord.frag"), m_PROGpickWithCoord);
	// by jdk 160407 endo
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/endoPlane.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/endoPlane.frag"), m_PROGendoPlane);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/DepthSetting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/DepthSetting.frag"), m_PROGdepthSetting);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/depthSetting2.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/depthSetting2.frag"), m_PROGdepthSetting2);
	//thyoo
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface.frag"), m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface_final.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/frontface_final.frag"), m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/backface.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/backface.frag"), m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/raycasting.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/thyoo/raycasting.frag"), m_thyShd.m_PROGraycasting);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/surface_texture.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/surface_texture.frag"), m_PROGsurfaceTexture);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/surface_endo.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/surface_endo.frag"), m_PROGsurfaceEndo);
	//thyoo end
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontfaceforFaceSim.frag"), m_PROGfrontfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backfaceforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/backfaceforFaceSim.frag"), m_PROGbackfaceCUBEforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/volumefirsthitforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/volumefirsthitforFaceSim.frag"), m_PROGvolumeFirstHitforFaceSim);
	WGLSLprogram::createShaderProgram(QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_finalforFaceSim.vert"), QString("/Users/software/Desktop/Will3D/Will3D_mac/Will3D/shader/Test/frontface_finalforFaceSim.frag"), m_PROGfrontfaceFinalforFaceSim);
#else
	WGLSLprogram::createShaderProgram(QString(":/shader/planeDisplay.vert"), QString(":/shader/planeDisplay.frag"), m_PROGplaneDisplay);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/frontface.vert"), QString(":/shader/Test/frontface.frag"), m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/frontface_final.vert"), QString(":/shader/Test/frontface_final.frag"), m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/backface.vert"), QString(":/shader/Test/backface.frag"), m_PROGbackfaceCUBE);
#ifndef WILL3D_LIGHT
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/raycasting.vert"), QString(":/shader/Test/raycasting_integ.frag"), m_PROGraycasting);
#else
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/raycasting.vert"), QString(":/shader/Test/raycasting_integ_light.frag"), m_PROGraycasting);
#endif
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/endo_raycasting.vert"), QString(":/shader/Test/endo_raycasting.frag"), m_PROGendoraycasting);

	WGLSLprogram::createShaderProgram(QString(":/shader/Test/Final.vert"), QString(":/shader/Test/Final.frag"), m_PROGfinal);
	WGLSLprogram::createShaderProgram(QString(":/shader/slice.vert"), QString(":/shader/slice.frag"), m_PROGslice);
	WGLSLprogram::createShaderProgram(QString(":/shader/sliceCanal.vert"), QString(":/shader/sliceCanal.frag"), m_PROGsliceCanal);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/SR.vert"), QString(":/shader/Test/SR.frag"), m_PROGSR);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/SRdepth.vert"), QString(":/shader/Test/SRdepth.frag"), m_PROGSRdepth);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/SRcoord.vert"), QString(":/shader/Test/SRcoord.frag"), m_PROGSRcoord);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/simpleSR.vert"), QString(":/shader/Test/simpleSR.frag"), m_PROGsimpleSR);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/anno.vert"), QString(":/shader/Test/anno.frag"), m_PROGanno);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/implant.vert"), QString(":/shader/Test/implant.frag"), m_PROGimplant);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/surface.vert"), QString(":/shader/Test/surface.frag"), m_PROGsurface);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/picking.vert"), QString(":/shader/Test/picking.frag"), m_PROGpick);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/pickWithCoord.vert"), QString(":/shader/Test/pickWithCoord.frag"), m_PROGpickWithCoord);
	// by jdk 160407 endo
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/endoPlane.vert"), QString(":/shader/Test/endoPlane.frag"), m_PROGendoPlane);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/DepthSetting.vert"), QString(":/shader/Test/DepthSetting.frag"), m_PROGdepthSetting);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/depthSetting2.vert"), QString(":/shader/Test/depthSetting2.frag"), m_PROGdepthSetting2);
	//thyoo
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/frontface.vert"), QString(":/shader/thyoo/frontface.frag"), m_thyShd.m_PROGfrontfaceCUBE);
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/frontface_final.vert"), QString(":/shader/thyoo/frontface_final.frag"), m_thyShd.m_PROGfrontfaceFinal);
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/backface.vert"), QString(":/shader/thyoo/backface.frag"), m_thyShd.m_PROGbackfaceCUBE);
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/raycasting.vert"), QString(":/shader/thyoo/raycasting.frag"), m_thyShd.m_PROGraycasting);
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/slice.vert"), QString(":/shader/thyoo/slice.frag"), m_thyShd.m_PROGslice);
	WGLSLprogram::createShaderProgram(QString(":/shader/thyoo/firsthit_ray.vert"), QString(":/shader/thyoo/firsthit_ray.frag"), m_thyShd.m_PROGfirsthit);

	WGLSLprogram::createShaderProgram(QString(":/shader/surface_texture.vert"), QString(":/shader/surface_texture.frag"), m_PROGsurfaceTexture);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/surface_endo.vert"), QString(":/shader/Test/surface_endo.frag"), m_PROGsurfaceEndo);
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/volumefirsthitforFaceSim.vert"), QString(":/shader/Test/volumefirsthitforFaceSim.frag"), m_PROGvolumeFirstHitforFaceSim);
	WGLSLprogram::createShaderProgram(QString(":/shader/image_texture.vert"), QString(":/shader/image_texture.frag"), prog_image_texture_);
	WGLSLprogram::createShaderProgram(QString(":/shader/mask_texture.vert"), QString(":/shader/mask_texture.frag"), prog_mask_texture_);

	unsigned int prog_surface_mask = 0;
	unsigned int prog_slice_implant = 0;
	unsigned int prog_bone_density = 0;

	WGLSLprogram::createShaderProgram(QString(":/shader/Test/surface_mask.vert"), QString(":/shader/Test/surface_mask.frag"), prog_surface_mask);
	WGLSLprogram::createShaderProgram(QString(":/shader/slice_implant.vert"), QString(":/shader/slice_implant.frag"), prog_slice_implant);
	WGLSLprogram::createShaderProgram(QString(":/shader/bone_density.vert"), QString(":/shader/bone_density.frag"), prog_bone_density);
	// jhyoon
	WGLSLprogram::createShaderProgram(QString(":/shader/Test/MeshFillVertexColor_tex.vert"), QString(":/shader/Test/MeshFillVertexColor_tex.frag"), m_PROGdisplacementSurface);
#endif
}

void CW3VREngine::init()
{
	initGLSL();
	initGLContextValues();

	initCUBEVertex();
	initPlaneVertex();
	initPrograms();
	initLookupTextures();

	common::Logger::instance()->PrintDebugMode("CW3VREngine::Init", "OpenGL initialized");
}

int CW3VREngine::GetActiveIndices(int vol_id) const
{
	return GetVolumeRenderer(vol_id)->GetActiveIndices();
}

void CW3VREngine::initLookupTextures()
{
	glm::vec3 *BsplineData = nullptr;
	glm::vec3 *BsplineDataPrime = nullptr;

	int Nsamples = 128;
	W3::p_allocate_1D(&BsplineData, Nsamples);
	W3::p_allocate_1D(&BsplineDataPrime, Nsamples);

	glm::vec4 coef0, coef1, coef2, coef3;
	coef0.x = -1.0f / 6.0f, coef0.y = 0.5f, coef0.z = -0.5f, coef0.w = 1.0f / 6.0f;
	coef1.x = 0.5f, coef1.y = -1.0f, coef1.z = 0.0f, coef1.w = 2.0f / 3.0f;
	coef2.x = -0.5f, coef2.y = 0.5f, coef2.z = 0.5f, coef2.w = 1.0f / 6.0f;
	coef3.x = 1.0f / 6.0f, coef3.y = 0.0f, coef3.z = 0.0f, coef3.w = 0.0f;

	glm::vec4 coef0p, coef1p, coef2p, coef3p;
	coef0p.x = 0.0f, coef0p.y = -0.5f, coef0p.z = 1.0f, coef0p.w = -0.5f;
	coef1p.x = 0.0f, coef1p.y = 1.5f, coef1p.z = -2.0f, coef1p.w = 0.0f;
	coef2p.x = 0.0f, coef2p.y = -1.5f, coef2p.z = 1.0f, coef2p.w = 0.5f;
	coef3p.x = 0.0f, coef3p.y = 0.5f, coef3p.z = 0.0f, coef3p.w = 0.0f;

	glm::vec4 u;
	u.w = 1.0f;

	for (int i = 0; i < Nsamples; i++)
	{
		u.z = float(i) / float(Nsamples);
		u.y = u.z*u.z;
		u.x = u.y*u.z;

		float w0 = glm::dot(u, coef0);
		float w1 = glm::dot(u, coef1);
		float w2 = glm::dot(u, coef2);
		float w3 = glm::dot(u, coef3);

		BsplineData[i].z = w0 + w1;
		BsplineData[i].x = 1.0f - w1 / BsplineData[i].z + u.z;
		BsplineData[i].y = 1.0f + w3 / (w2 + w3) - u.z;

		w0 = glm::dot(u, coef0p);
		w1 = glm::dot(u, coef1p);
		w2 = glm::dot(u, coef2p);
		w3 = glm::dot(u, coef3p);

		BsplineDataPrime[i].z = w0 + w1;
		BsplineDataPrime[i].x = 1.0f - w1 / BsplineDataPrime[i].z + u.z;
		BsplineDataPrime[i].y = 1.0f + w3 / (w2 + w3) - u.z;
	}

	if (!m_texBsplineHandler)
	{
		glGenTextures(1, &m_texBsplineHandler);
		glBindTexture(GL_TEXTURE_1D, m_texBsplineHandler);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGB32F, Nsamples);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, Nsamples, GL_RGB, GL_FLOAT, BsplineData);
	}
	else
	{
		printf("WARNING: 1D texHandler buffer is alreadly generated!\n");
	}

	if (!m_texBsplinePrimeHandler)
	{
		glGenTextures(1, &m_texBsplinePrimeHandler);
		glBindTexture(GL_TEXTURE_1D, m_texBsplinePrimeHandler);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGB32F, Nsamples);
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, Nsamples, GL_RGB, GL_FLOAT, BsplineDataPrime);
	}
	else
	{
		printf("WARNING: 1D texHandler buffer is alreadly generated!\n");
	}

	SAFE_DELETE_ARRAY(BsplineData);
	SAFE_DELETE_ARRAY(BsplineDataPrime);

	/// Catmull-Rom spline
	//coef0.x = -0.5f, coef0.y = 1.0f, coef0.z = -0.5f, coef0.w = 0.0f;
	//coef1.x = 1.5f, coef1.y = -2.5f, coef1.z = 0.0f, coef1.w = 1.0;
	//coef2.x = -1.5f, coef2.y = 2.0f, coef2.z = 0.5f, coef2.w = 0.0f;
	//coef3.x = 0.5f, coef3.y = -0.5f, coef3.z = 0.0f, coef3.w = 0.0f;

	//glm::vec4 u1, u2, u3;

	//u1.w = 1.0f, u2.w = 1.0f, u3.w = 1.0f;
	//u1.z = 0.25f, u2.z = 0.5f, u3.z = 0.75f;
	//u1.y = u1.z*u1.z, u2.y = u2.z*u2.z, u3.y = u3.z*u3.z;
	//u1.x = u1.y*u1.z, u2.x = u2.y*u2.z, u3.x = u3.y*u3.z;

	//m_Catmull_Rom1.x = glm::dot(coef0, u1);
	//m_Catmull_Rom1.y = glm::dot(coef1, u1);
	//m_Catmull_Rom1.z = glm::dot(coef2, u1);
	//m_Catmull_Rom1.w = glm::dot(coef3, u1);

	//m_Catmull_Rom2.x = glm::dot(coef0, u2);
	//m_Catmull_Rom2.y = glm::dot(coef1, u2);
	//m_Catmull_Rom2.z = glm::dot(coef2, u2);
	//m_Catmull_Rom2.w = glm::dot(coef3, u2);

	//m_Catmull_Rom3.x = glm::dot(coef0, u3);
	//m_Catmull_Rom3.y = glm::dot(coef1, u3);
	//m_Catmull_Rom3.z = glm::dot(coef2, u3);
	//m_Catmull_Rom3.w = glm::dot(coef3, u3);
}

void CW3VREngine::initVBOplane()
{
	int vertSize = 12;
	int texSize = 8;
	int indexSize = 6;

	glGenBuffers(3, m_vboPlane);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboPlane[0]);
	glBufferData(GL_ARRAY_BUFFER, vertSize * sizeof(float), m_vertPlaneCoord, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_vboPlane[1]);
	glBufferData(GL_ARRAY_BUFFER, texSize * sizeof(float), m_texPlaneCoord, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboPlane[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), m_IndexPlane, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3VREngine::InitVBOPlaneInverseY()
{
	int vertSize = 12;
	int texSize = 8;
	int indexSize = 6;

	glGenBuffers(3, vbo_plane_inverse_y_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_plane_inverse_y_[0]);
	glBufferData(GL_ARRAY_BUFFER, vertSize * sizeof(float), m_vertPlaneCoord, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_plane_inverse_y_[1]);
	glBufferData(GL_ARRAY_BUFFER, texSize * sizeof(float), m_tex_inverseY_PlaneCoord, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_plane_inverse_y_[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(unsigned int), m_IndexPlane, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3VREngine::initVAOplane(unsigned int *vao)
{
	if (*vao)
	{
		glDeleteVertexArrays(1, vao);
		*vao = 0;
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glEnableVertexAttribArray(0); // for vertexloc
	glEnableVertexAttribArray(1); // for vertexcol

	// the vertex location is the same as the vertex color
	glBindBuffer(GL_ARRAY_BUFFER, m_vboPlane[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboPlane[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboPlane[2]);

	glBindVertexArray(0); // by jdk 151110
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3VREngine::InitVAOPlaneInverseY(unsigned int *vao)
{
	if (*vao)
	{
		glDeleteVertexArrays(1, vao);
		*vao = 0;
	}
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);

	glEnableVertexAttribArray(0); // for vertexloc
	glEnableVertexAttribArray(1); // for vertexcol

								  // the vertex location is the same as the vertex color
	glBindBuffer(GL_ARRAY_BUFFER, vbo_plane_inverse_y_[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_plane_inverse_y_[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_plane_inverse_y_[2]);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CW3VREngine::setActiveIndex(unsigned int *vao, int id)
{
	auto renderer = GetVolumeRenderer(id);
	renderer->UpdateActiveBlockVAO(vao);

	m_pMainVRparams[id]->m_Nindices = renderer->GetActiveIndices();
}
void CW3VREngine::tmpSetVolTexHandler(unsigned int handler, int id)
{
	if (!m_pMainVRparams[id])
	{
		return;
	}

	m_pMainVRparams[id]->m_texHandlerVol = handler;
}
void CW3VREngine::tmpSetTfTexHandler(unsigned int handler)
{
	m_texTF2DHandler = handler;
}
void CW3VREngine::setVolume(CW3Image3D *volume, int id)
{
	int nVolWidth = volume->width();
	int nVolHeight = volume->height();
	int nVolDepth = volume->depth();

	///////////////// About OTF
	if (id == 0)
	{
		m_pMainVRparams[id]->setThresholds(volume->getAirTissueThreshold(),
			volume->getTissueBoneThreshold(),
			volume->getBoneTeethThreshold());
	}
	else
	{
		m_pMainVRparams[id]->m_MinValue = m_pMainVRparams[0]->m_pgVol->getMin();
		m_pMainVRparams[id]->m_MaxIntensity = m_pMainVRparams[0]->m_pgVol->getMax()
			- m_pMainVRparams[id]->m_MinValue;
	}
}

void CW3VREngine::setDownFactor(VREngine::VolType id, const int& down_factor)
{
	m_pMainVRparams[id]->set_down_factor((float)down_factor);
	m_pMainVRparams[id]->updateStepSize();
}
void CW3VREngine::setVolume(CW3Image3D* volume, VREngine::VolType id)
{
	int nVolWidth = volume->width();
	int nVolHeight = volume->height();
	int nVolDepth = volume->depth();

	SAFE_DELETE_OBJECT(m_pMainVRparams[id]);
	m_pMainVRparams[id] = new CW3VolumeRenderParam(volume, m_pgGLWidget);
}

void CW3VREngine::setRCuniforms(unsigned int prog)
{
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MinValue", m_MinValuetest);
	WGLSLprogram::setUniform(prog, "AttenDistance", m_AttenDistancetest);
	WGLSLprogram::setUniform(prog, "texelSize", m_texelSizetest);
	WGLSLprogram::setUniform(prog, "MaxTexSize", m_nMaxTexAxisSize);
	WGLSLprogram::setUniform(prog, "MaxValue", m_maxValuetest);
	WGLSLprogram::setUniform(prog, "TFxincrease", m_TFxincreasetest);
	WGLSLprogram::setUniform(prog, "TFyincrease", m_TFyincreasetest);

	mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	mat4 projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
	mat4 mvpForFinal = projection * view;

	WGLSLprogram::setUniform(prog, "MVP", mvpForFinal);
}

void CW3VREngine::makeCurrent() { m_pgGLWidget->makeCurrent(); }
void CW3VREngine::doneCurrent() { m_pgGLWidget->doneCurrent(); }

void CW3VREngine::volumeDown(short **in, unsigned int downFactor, unsigned int &w, unsigned int &h, unsigned int &d)
{
	int w2 = (float)w / downFactor;
	int h2 = (float)h / downFactor;
	int d2 = (float)d / downFactor;

	size_t sResize = w2 * h2*d2;
	short *tmp_down = nullptr;
	W3::p_allocate_1D(&tmp_down, sResize);

	short *tmp_in = *in;
	int x2, y2, z2;
	for (int i = 0; i < d2; ++i)
	{
		z2 = i * downFactor;
		for (int j = 0; j < w2 * h2; ++j)
		{
			y2 = j / w2 * downFactor;
			x2 = j % w2*downFactor;
			tmp_down[i * (w2 * h2) + j] = tmp_in[z2*w*h + (y2 * w) + x2];
		}
	}

	w = w2;
	h = h2;
	d = d2;

	SAFE_DELETE_ARRAY(*in);
	*in = tmp_down;
}

void CW3VREngine::setTFtoVolume(int volid)
{
	if (volid < 2)
		GetVolumeRenderer(volid)->SetActiveBlock(tf_min_, tf_max_);

	if (m_pMainVRparams[volid])
	{
		m_pMainVRparams[volid]->m_MaxValueForMIP = m_pMainVRparams[volid]->m_pgVol->getMax()
			- m_pMainVRparams[volid]->m_pgVol->getMin() + 0.5f;
	}
}
void CW3VREngine::initTF(int tf_size)
{
	if (m_isTFinitialized)
	{
		return;
	}

	m_nTFtexwidth = tf_size;
	m_nTFtexheight = 1;

	// by jdk 151105
	if (m_nTFtexwidth > m_nMaxTexAxisSize)
	{
		m_nTFtexheight = m_nTFtexwidth / m_nMaxTexAxisSize;
		m_nTFtexwidth = m_nMaxTexAxisSize;
	}

	makeCurrent();
	float TFxincrease = 1.0f / m_nTFtexwidth;
	float TFyincrease = 1.0f / m_nTFtexheight;

	glUseProgram(m_PROGraycasting);
	WGLSLprogram::setUniform(m_PROGraycasting, "TFxincrease", TFxincrease);
	WGLSLprogram::setUniform(m_PROGraycasting, "TFyincrease", TFyincrease);
	WGLSLprogram::setUniform(m_PROGraycasting, "MaxTexSize", m_nMax3DTexSize);

	glUseProgram(m_PROGendoraycasting);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "TFxincrease", TFxincrease);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "TFyincrease", TFyincrease);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "MaxTexSize", m_nMax3DTexSize);

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TFxincrease", TFxincrease);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TFyincrease", TFyincrease);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MaxTexSize", m_nMax3DTexSize);

	m_isTFinitialized = true;
	m_TFxincreasetest = TFxincrease;
	m_TFyincreasetest = TFyincrease;
	doneCurrent();
}
void CW3VREngine::slotUpdateTF(int tf_min, int tf_max, bool isMinMaxChanged)
{
	tf_min_ = tf_min;
	tf_max_ = tf_max;

	if (isMinMaxChanged)
	{
		for (int i = 0; i < 5; i++)
		{
			if (m_pMainVRparams[i])
			{
				m_pMainVRparams[i]->m_MaxValueForMIP = m_pMainVRparams[i]->m_pgVol->getMax()
					- m_pMainVRparams[i]->m_pgVol->getMin() + 0.5f;
			}
		}
	}

	emit sigTFupdated(isMinMaxChanged);
}

void CW3VREngine::initFrameBufferSimpleSR(unsigned int &FBhandler, unsigned int &DepthHandler, unsigned int *texHandler, unsigned int nx, unsigned int ny, int texHandlerCount)
{
	if (FBhandler)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FBhandler);
		FBhandler = 0;
	}

	if (texHandler[0])
	{
		glDeleteTextures(texHandlerCount, texHandler);

		for (int i = 0; i < texHandlerCount; i++)
		{
			texHandler[i] = 0;
		}
	}

	if (DepthHandler)
	{
		glDeleteRenderbuffers(1, &DepthHandler);
		DepthHandler = 0;
	}

	glGenTextures(texHandlerCount, texHandler);

	glGenRenderbuffers(1, &DepthHandler);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

	glGenFramebuffers(1, &FBhandler);
	glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

	for (int i = 0; i < texHandlerCount; i++)
	{
		if (i != 3)
		{
			glBindTexture(GL_TEXTURE_2D, texHandler[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[i], GL_TEXTURE_2D, texHandler[i], 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, texHandler[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, nx, ny);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[i], GL_TEXTURE_2D, texHandler[i], 0);
		}
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthHandler);

	CW3GLFunctions::checkFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CW3VREngine::initFrameBufferFirstHit(unsigned int &FBhandler, unsigned int &DepthHandler, unsigned int *texHandler, unsigned int nx, unsigned int ny)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (FBhandler)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FBhandler);
		FBhandler = 0;
	}

	if (texHandler[0])
	{
		glDeleteTextures(3, texHandler);

		for (int i = 0; i < 3; i++)
		{
			texHandler[i] = 0;
		}
	}

	if (DepthHandler)
	{
		glDeleteRenderbuffers(1, &DepthHandler);
		DepthHandler = 0;
	}

	glGenTextures(3, texHandler);

	glGenRenderbuffers(1, &DepthHandler);
	glBindRenderbuffer(GL_RENDERBUFFER, DepthHandler);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, nx, ny);

	glGenFramebuffers(1, &FBhandler);
	glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

	glBindTexture(GL_TEXTURE_2D, texHandler[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[0], GL_TEXTURE_2D, texHandler[0], 0);

	for (int i = 1; i < 3; i++)
	{
		glBindTexture(GL_TEXTURE_2D, texHandler[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[i], GL_TEXTURE_2D, texHandler[i], 0);
	}

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthHandler);

	CW3GLFunctions::checkFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CW3VREngine::initFrameBufferSlice(unsigned int &FBhandler, unsigned int *texHandler, unsigned int nx, unsigned int ny, int texHandlerCount)
{
	if (FBhandler)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &FBhandler);
		FBhandler = 0;
	}

	if (texHandler[0])
	{
		glDeleteTextures(texHandlerCount, texHandler);

		for (int i = 0; i < texHandlerCount; i++)
		{
			texHandler[i] = 0;
		}
	}

	glGenTextures(texHandlerCount, texHandler);

	glGenFramebuffers(1, &FBhandler);
	glBindFramebuffer(GL_FRAMEBUFFER, FBhandler);

	glBindTexture(GL_TEXTURE_2D, texHandler[0]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[0], GL_TEXTURE_2D, texHandler[0], 0);

	glBindTexture(GL_TEXTURE_2D, texHandler[1]);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, nx, ny);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, m_SimpleSR_draw_buffers[1], GL_TEXTURE_2D, texHandler[1], 0);

	CW3GLFunctions::checkFramebufferStatus();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CW3GLFunctions::printError(__LINE__, "initFrameBufferSlice failed.");
}

void CW3VREngine::setVolTextureUniform(unsigned int prog, unsigned int texHandler)
{
	glUseProgram(prog);
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, texHandler);
	WGLSLprogram::setUniform(prog, "VolumeTex", m_texNumVol3D_);
}

void CW3VREngine::setTFTextureUniform()
{
	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);

	glUseProgram(m_PROGraycasting);
	WGLSLprogram::setUniform(m_PROGraycasting, "TransferFunc", m_texNumTF2D_);

	glUseProgram(m_PROGendoraycasting);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "TransferFunc", m_texNumTF2D_);

	//thyoo
	glUseProgram(m_thyShd.m_PROGraycasting);
	WGLSLprogram::setUniform(m_thyShd.m_PROGraycasting, "TransferFunc", m_texNumTF2D_);
	//thyoo end
}

void CW3VREngine::getFrontMiddleSliceCoord(float *vert, unsigned int *index)
{
	vert[0] = m_vertCUBECoord[3], vert[1] = m_vertCUBECoord[4], vert[2] = 0.0f;
	vert[3] = m_vertCUBECoord[15], vert[4] = m_vertCUBECoord[16], vert[5] = 0.0f;
	vert[6] = m_vertCUBECoord[21], vert[7] = m_vertCUBECoord[22], vert[8] = 0.0f;
	vert[9] = m_vertCUBECoord[9], vert[10] = m_vertCUBECoord[10], vert[11] = 0.0f;

	index[0] = 0;
	index[1] = 1;
	index[2] = 2;
	index[3] = 3;
}

void CW3VREngine::getCUBEvertexCoord(glm::vec4* vert)
{
	vert[0] = glm::vec4(m_vertCUBECoord[0], m_vertCUBECoord[1], m_vertCUBECoord[2], 1.0f);
	vert[1] = glm::vec4(m_vertCUBECoord[3], m_vertCUBECoord[4], m_vertCUBECoord[5], 1.0f);
	vert[2] = glm::vec4(m_vertCUBECoord[6], m_vertCUBECoord[7], m_vertCUBECoord[8], 1.0f);
	vert[3] = glm::vec4(m_vertCUBECoord[9], m_vertCUBECoord[10], m_vertCUBECoord[11], 1.0f);

	vert[4] = glm::vec4(m_vertCUBECoord[12], m_vertCUBECoord[13], m_vertCUBECoord[14], 1.0f);
	vert[5] = glm::vec4(m_vertCUBECoord[15], m_vertCUBECoord[16], m_vertCUBECoord[17], 1.0f);
	vert[6] = glm::vec4(m_vertCUBECoord[18], m_vertCUBECoord[19], m_vertCUBECoord[20], 1.0f);
	vert[7] = glm::vec4(m_vertCUBECoord[21], m_vertCUBECoord[22], m_vertCUBECoord[23], 1.0f);
}

void CW3VREngine::setUniformImplant(CW3Render3DParam * param, const bool wire)
{
	glUseProgram(m_PROGimplant);

	glActiveTexture(m_texNumVol3D);
	//if (param->m_isDerivedVolume)
	//	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[1]->m_texHandlerVol);
	//else
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[0]->m_texHandlerVol);

	WGLSLprogram::setUniform(m_PROGimplant, "VolumeTex", m_texNumVol3D_);

#if 0
	//set color
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Ka", vec3(0.3f));
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Ks", vec3(0.3f));
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Kd", vec3(0.3f));
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Shininess", 3.0f);
	WGLSLprogram::setUniform(m_PROGimplant, "alpha", 1.0f);
#endif

	WGLSLprogram::setUniform(m_PROGimplant, "Light.Position", param->m_lightInfo.Position);
	WGLSLprogram::setUniform(m_PROGimplant, "Light.Intensity", param->m_lightInfo.Intensity);

	//set bonedensity parameters
	WGLSLprogram::setUniform(m_PROGimplant, "isBoneDensity", false);
	WGLSLprogram::setUniform(m_PROGimplant, "texelSize", param->m_pgMainVolume[0]->m_volTexelSize);
	WGLSLprogram::setUniform(m_PROGimplant, "tfOffset", (int)param->m_pgMainVolume[0]->m_pgVol->intercept());

	WGLSLprogram::setUniform(m_PROGimplant, "isWire", wire);
}

void CW3VREngine::setUniformSingleImplant(CW3Render3DParam* param, int implantID, const bool wire)
{
	glUseProgram(m_PROGimplant);

	CW3GLObject* implant = param->m_pImplant[implantID];
	glm::mat4 mv = implant->getMV();

	//set single implant info
	WGLSLprogram::setUniform(m_PROGimplant, "id", (implantID + 1) / 255.0f);

	QColor default_color_volume = GlobalPreferences::GetInstance()->preferences_.objects.implant.default_color_volume;
	QColor default_color_wire = GlobalPreferences::GetInstance()->preferences_.objects.implant.default_color_wire;
	QColor collided_color_volume = GlobalPreferences::GetInstance()->preferences_.objects.implant.collided_color_volume;
	QColor collided_color_wire = GlobalPreferences::GetInstance()->preferences_.objects.implant.collided_color_wire;

	float alpha = GlobalPreferences::GetInstance()->preferences_.objects.implant.alpha;

	QColor color_volume;
	QColor color_wire;
	if (param->g_is_implant_collided_[implantID])
	{
		color_volume = collided_color_volume;
		color_wire = collided_color_wire;
	}
	else
	{
		color_volume = default_color_volume;
		color_wire = default_color_wire;
	}

	WGLSLprogram::setUniform(m_PROGimplant, "Material.Ka", vec3(0.1f));
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Ks", glm::vec3(color_volume.redF(), color_volume.greenF(), color_volume.blueF()) * 0.2f);
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Kd", glm::vec3(color_volume.redF(), color_volume.greenF(), color_volume.blueF()));
	WGLSLprogram::setUniform(m_PROGimplant, "Material.Shininess", 1.0f);
	WGLSLprogram::setUniform(m_PROGimplant, "alpha", alpha);

	WGLSLprogram::setUniform(m_PROGimplant, "meshColor", glm::vec3(color_wire.redF(), color_wire.greenF(), color_wire.blueF()));
	WGLSLprogram::setUniform(m_PROGimplant, "isWire", wire);

	//set matrix
	WGLSLprogram::setUniform(m_PROGimplant, "PositionMatrix", mv);

	WGLSLprogram::setUniform(m_PROGimplant, "NormalMatrix",
		glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));

	WGLSLprogram::setUniform(m_PROGimplant, "MVP", implant->getMVP());
}

void CW3VREngine::slotReoriented(std::vector<float> param)
{
	//m_modelReori = glm::translate(glm::vec3(param[1]*2.0f, 0.0f, 0.0f))*glm::rotate(param[2], glm::vec3(0.0f, -1.0f, 0.0f))*glm::rotate(param[0], glm::vec3(0.0f, 0.0f, -1.0f));
	// Jung To Do : pixelspacing, slicespacing 占쏙옙 占쏙옙占쏙옙 translate 占쏙옙占쏙옙占쌔억옙 占쏙옙
	m_modelReori = glm::rotate(param[0], glm::vec3(0.0f, 0.0f, -1.0f))*glm::rotate(param[2], glm::vec3(0.0f, -1.0f, 0.0f))*glm::translate(glm::vec3(param[1] * 2.0f, 0.0f, 0.0f));

	emit sigReoriupdate(&m_modelReori);
}

void CW3VREngine::applyReorientation(glm::mat4 &mat)
{
	m_modelReori = mat;

	emit sigReoriupdate(&m_modelReori);
}

void CW3VREngine::readyForSetVolume(CW3Image3D* volume, int id)
{
	if (m_pMainVRparams[id])
		SAFE_DELETE_OBJECT(m_pMainVRparams[id]);
	m_pMainVRparams[id] = new CW3VolumeRenderParam(volume, m_pgGLWidget);
}

glm::vec3* CW3VREngine::getVolRange(int id)
{
	return m_pgMPRengine->getVolRange(id);
}

void CW3VREngine::setProjectionEvn()
{
	glm::vec3 vVolRange = *m_pgMPRengine->getVolRange(0);
	m_VolRangeGL.setX(vVolRange.x);
	m_VolRangeGL.setY(vVolRange.y);
	m_VolRangeGL.setZ(vVolRange.z);

	m_model = glm::scale(glm::vec3(m_VolRangeGL.x(), m_VolRangeGL.y(), m_VolRangeGL.z()));
	m_camFOV = glm::length(vVolRange);
	m_view = glm::lookAt(glm::vec3(0.0f, -m_camFOV, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	m_projection = glm::ortho(-m_camFOV, m_camFOV, -m_camFOV, m_camFOV, 0.0f, m_camFOV*2.0f);

	m_mvp = m_projection * m_view*m_model;
}
void CW3VREngine::getVolumeIntensity(const unsigned int idVol, const glm::vec3& position, unsigned short& intensity)
{
	vec3 vVolRange = vec3(m_pMainVRparams[idVol]->m_pgVol->width(),
		m_pMainVRparams[idVol]->m_pgVol->height(),
		m_pMainVRparams[idVol]->m_pgVol->depth());

	vec3 vVolMaxIdx(vVolRange.x - 1, vVolRange.y - 1, vVolRange.z - 1);

	vec3 volCoord = (vec3(-1.0f, 1.0f, 1.0f)*position * 0.5f + 0.5f)*vVolRange;

	int ix = (volCoord.x < 0.0) ? 0 : (volCoord.x > vVolMaxIdx.x) ? (int)vVolMaxIdx.x : (int)volCoord.x;
	int iy = (volCoord.y < 0.0) ? 0 : (volCoord.y > vVolMaxIdx.y) ? (int)vVolMaxIdx.y : (int)volCoord.y;
	int iz = (volCoord.z < 0.0) ? 0 : (volCoord.z > vVolMaxIdx.z) ? (int)vVolMaxIdx.z : (int)volCoord.z;
	int ixy = ix + iy * vVolRange.x;

	unsigned short** volData = m_pMainVRparams[idVol]->m_pgVol->getData();
	intensity = volData[iz][ixy];
}

void CW3VREngine::Render3Dboth(CW3Render3DParam *param, int VolId, bool &isReconSwitched, bool isSecondVolume)
{
	////must be debug.
#if ___DEBUG
	this->recompileRaycasting();
#endif
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3Dboth start");
	////recompile for test end
	int width, height;
	if (param->m_isLowRes)
	{
		width = param->m_width*low_res_frame_buffer_resize_factor_;
		height = param->m_height*low_res_frame_buffer_resize_factor_;

		param->m_pgMainVolume[0]->setLowRes(true);
	}
	else
	{
		width = param->m_width;
		height = param->m_height;

		param->m_pgMainVolume[0]->setLowRes(false);
	}

	if (isReconSwitched || !param->m_fbo || param->m_widthPre != width || param->m_heightPre != height)
	{
		initFrameBufferSimpleSR(param->m_fbo, param->m_depthMap, param->m_texHandler,
			width, height, NW3Render3DParam::kNumTexHandle);
		param->m_widthPre = width;
		param->m_heightPre = height;

		isReconSwitched = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);
	glViewport(0, 0, width, height);
	glDrawBuffers(NW3Render3DParam::kNumTexHandle, m_SimpleSR_draw_buffers);
	CW3GLFunctions::clearView(true);

	////////////// For Volume Cube

	//////// Front Face
	glUseProgram(m_PROGfrontfaceCUBE);
	if (isSecondVolume)
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[5]);
		CW3GLFunctions::clearView(true, GL_BACK);
		glUseProgram(m_PROGfrontfaceCUBE);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", *(param->m_pgMainVolume[1]->m_mvp));
		CW3GLFunctions::drawView(param->m_mainVolume_vao[1], param->m_pgMainVolume[1]->m_Nindices, GL_BACK);
	}

	glDrawBuffer(m_SimpleSR_draw_buffers[1]);
	CW3GLFunctions::clearView(true, GL_BACK);
	WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", *(param->m_pgMainVolume[VolId]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_BACK);

	//////////// Back Face
	glUseProgram(m_PROGbackfaceCUBE);
	if (isSecondVolume)
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[4]);
		CW3GLFunctions::clearView(true, GL_FRONT);
		WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", *(param->m_pgMainVolume[1]->m_mvp));
		CW3GLFunctions::drawView(param->m_mainVolume_vao[1], param->m_pgMainVolume[1]->m_Nindices, GL_FRONT);
	}
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	CW3GLFunctions::clearView(true, GL_FRONT);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", *(param->m_pgMainVolume[VolId]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_FRONT);

	glDepthFunc(GL_LESS);

	if (VolId == 0 && !m_bIsMIP && !m_bIsXRAY)
	{
		////////////// For Surface
		if (param->m_isImplantShown ||
			(param->m_photo3D && param->m_photo3D->isVisible()) ||
			(param->m_pNerve && param->m_pNerve->isVisible()) ||
			(param->m_pAirway && param->m_pAirway->isVisible()))
		{
			//glDrawBuffers(NtexHandler, m_SimpleSR_draw_buffers);
			glUseProgram(m_PROGsimpleSR);

			WGLSLprogram::setUniform(m_PROGsimpleSR, "isYinvert", false);
			WGLSLprogram::setUniform(m_PROGsimpleSR, "isXinvert", !param->m_isDerivedVolume); // false if derived volume
		}

		if (param->m_isImplantShown)
		{
			for (int implantID = 0; implantID < MAX_IMPLANT; implantID++)
			{
				if (param->g_is_implant_exist_[implantID])
				{
					WGLSLprogram::setUniform(m_PROGsimpleSR, "isForBackFace", true);
					WGLSLprogram::setUniform(m_PROGsimpleSR, "isScaled", true);
					WGLSLprogram::setUniform(m_PROGsimpleSR, "InverseScale", param->m_pImplant[implantID]->getInvModel());
					WGLSLprogram::setUniform(m_PROGsimpleSR, "MVP", param->m_pImplant[implantID]->getMVP());

					CW3GLFunctions::drawViewTriangles(param->m_pImplant[implantID]->getVAO(), param->m_pImplant[implantID]->getNindices(), GL_BACK);
				}
			}
		}

		if (param->m_pNerve && param->m_pNerve->isVisible())
		{
			if (!param->m_pNerve->isTransparent())
			{
				WGLSLprogram::setUniform(m_PROGsimpleSR, "isForBackFace", true);
				WGLSLprogram::setUniform(m_PROGsimpleSR, "isScaled", true);
				WGLSLprogram::setUniform(m_PROGsimpleSR, "InverseScale", param->m_pNerve->getInvModel());
				WGLSLprogram::setUniform(m_PROGsimpleSR, "MVP", param->m_pNerve->getMVP());
				CW3GLFunctions::drawView(param->m_pNerve->getVAO(), param->m_pNerve->getNindices(), GL_BACK);
			}
		}

		if (param->m_photo3D && param->m_photo3D->isVisible())
		{
			if (!param->m_photo3D->isTransparent())
			{
				WGLSLprogram::setUniform(m_PROGsimpleSR, "isForBackFace", true);
				WGLSLprogram::setUniform(m_PROGsimpleSR, "isScaled", true);
				WGLSLprogram::setUniform(m_PROGsimpleSR, "InverseScale", param->m_photo3D->getInvModel());
				WGLSLprogram::setUniform(m_PROGsimpleSR, "MVP", param->m_photo3D->getMVP());
				CW3GLFunctions::drawViewTriangles(param->m_photo3D->getVAO(), param->m_photo3D->getNindices(), GL_BACK);
			}
		}

		if (param->m_pAirway && param->m_pAirway->isVisible())
		{
			WGLSLprogram::setUniform(m_PROGsimpleSR, "isForBackFace", true);
			WGLSLprogram::setUniform(m_PROGsimpleSR, "isScaled", false);
			WGLSLprogram::setUniform(m_PROGsimpleSR, "InverseScale", param->m_pAirway->getInvModel());
			WGLSLprogram::setUniform(m_PROGsimpleSR, "MVP", param->m_pAirway->getMVP());
			CW3GLFunctions::drawViewTriangles(param->m_pAirway->getVAO(), param->m_pAirway->getNindices(), GL_BACK);
		}
	}

	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	////// Extract Front Face
	glDrawBuffer(m_SimpleSR_draw_buffers[2]);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum buffers[2] = { m_SimpleSR_draw_buffers[1], m_SimpleSR_draw_buffers[2] };
	glDrawBuffers(2, buffers);

	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", (param->m_isClipped));
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)param->m_clipPlanes.size());
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPerspective", param->m_isPerspective);

	for (int i = 0; i < (int)param->m_clipPlanes.size(); i++)
	{
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), (param->m_clipPlanes[i]));
	}

	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", (param->m_plane->getMVP()));
	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[0]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNumSimpleSR_[0]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isDepthFirstHit", !param->m_isDerivedVolume); // false if derived volume

	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isThisSecond", false);
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TransferFunc", m_texNumTF2D_);

	param->m_pgMainVolume[0]->setRayFirstHitFrontFaceParams(m_PROGfrontfaceFinal);

	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	////// cube front face
	if (isSecondVolume)
	{
		//// cube front face
		glDepthFunc(GL_LESS);

		glDrawBuffer(m_SimpleSR_draw_buffers[6]);
		glClear(GL_COLOR_BUFFER_BIT);
		// front face for second volume
		buffers[0] = m_SimpleSR_draw_buffers[5];
		buffers[1] = m_SimpleSR_draw_buffers[6];
		glDrawBuffers(2, buffers);

		glUseProgram(m_PROGfrontfaceFinal);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", param->m_isClipped);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)param->m_clipPlanes.size());
		for (int i = 0; i < (int)param->m_clipPlanes.size(); i++)
		{
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), (param->m_clipPlanes[i]));
		}

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", (param->m_plane->getMVP()));
		glActiveTexture(m_texNumSimpleSR[4]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[4]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNumSimpleSR_[4]);

		glActiveTexture(m_texNumSimpleSR[5]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[5]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNumSimpleSR_[5]);

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isDepthFirstHit", true);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isThisSecond", true);
		glActiveTexture(m_texNumVol3D);
		glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[1]->m_texHandlerVol);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "VolumeTex", m_texNumVol3D_);

		glActiveTexture(m_texNumTF2D);
		glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TransferFunc", m_texNumTF2D_);
		param->m_pgMainVolume[1]->setRayFirstHitFrontFaceParams(m_PROGfrontfaceFinal);

		CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

		CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);
	}

	/////////// Mesh drawing
	buffers[0] = m_SimpleSR_draw_buffers[0];
	buffers[1] = m_SimpleSR_draw_buffers[3];
	glDrawBuffers(2, buffers);

	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	bool isThereMesh = false;
	bool isPhotoSeparated = false;

	CW3GLFunctions::clearView(true, GL_BACK);

	glFrontFace(GL_CW);
	if (VolId == 0 && !m_bIsMIP && !m_bIsXRAY)
	{
		////////////// Surface Rendering
		if (param->m_isImplantShown ||
			(param->m_photo3D && param->m_photo3D->isVisible()) ||
			(param->m_pNerve && param->m_pNerve->isVisible()) ||
			(param->m_pAirway && param->m_pAirway->isVisible()))
		{
			isThereMesh = true;
			if (param->m_isImplantShown)
			{
				glUseProgram(m_PROGimplant);
				WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", true);
				setUniformImplant(param);

				for (int implantID = 0; implantID < MAX_IMPLANT; implantID++)
				{
					if (param->g_is_implant_exist_[implantID])
					{
						//WGLSLprogram::setUniform(m_PROGimplant, "isWire", false);
						//param->m_pImplant[implantID]->setMeshColor(glm::vec3(1.0f, 1.0f, 1.0f));
						setUniformSingleImplant(param, implantID, false);
						CW3GLFunctions::drawViewTriangles(param->m_pImplant[implantID]->getVAO(), param->m_pImplant[implantID]->getNindices(), GL_BACK);
					}
				}
				CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
			}

			if (param->m_pNerve && param->m_pNerve->isVisible())
			{
				if (!param->m_pNerve->isTransparent())
				{
					glUseProgram(m_PROGsurface);
					mat4 invertY = glm::scale(vec3(1.0f, -1.0f, 1.0f));
					WGLSLprogram::setUniform(m_PROGsurface, "MVP", invertY*(param->m_pNerve->getMVP()));

					mat4 mv = param->m_pNerve->getMV();
					WGLSLprogram::setUniform(m_PROGsurface, "ModelViewMatrix", (mv));
					WGLSLprogram::setUniform(m_PROGsurface, "NormalMatrix", glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
					glm::mat4 inv_nerve_view = glm::inverse(param->m_pNerve->getView());

					WGLSLprogram::setUniform(m_PROGsurface, "Light.Position", glm::vec4(inv_nerve_view[3]));
					WGLSLprogram::setUniform(m_PROGsurface, "Light.Intensity", glm::vec3(1.0f));

					param->m_pNerve->setUniformColor(m_PROGsurface);
					param->m_pNerve->render();
				}
				CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
			}

			if (param->m_pAirway && param->m_pAirway->isVisible())
			{
				glUseProgram(m_PROGsurfaceEndo);
				WGLSLprogram::setUniform(m_PROGsurfaceEndo, "MVP", glm::scale(vec3(1.0, -1.0, 1.0))*((param->m_pAirway->getMVP())));
				glFrontFace(GL_CCW); // by jdk 170203 for airway x占쏙옙 占쏙옙占쏙옙
				CW3GLFunctions::drawViewTriangles(param->m_pAirway->getVAO(), param->m_pAirway->getNindices(), GL_BACK);
				glFrontFace(GL_CW); // by jdk 170203 for airway x占쏙옙 占쏙옙占쏙옙

				glUseProgram(0);
			}

			if (param->m_photo3D && param->m_photo3D->isVisible())
			{
				if (!param->m_photo3D->isTransparent())
				{
					isThereMesh = true;

					glUseProgram(m_PROGSR);
					WGLSLprogram::setUniform(m_PROGSR, "isYinvert", true);
					WGLSLprogram::setUniform(m_PROGSR, "MVP", (param->m_photo3D->getMVP()));
					WGLSLprogram::setUniform(m_PROGSR, "isTexture", true);

					glActiveTexture(m_texNumFACE);
					glBindTexture(GL_TEXTURE_2D, param->m_photo3D->getTexHandler());
					WGLSLprogram::setUniform(m_PROGSR, "FACEtexture", m_texNumFACE_);
					WGLSLprogram::setUniform(m_PROGSR, "Alpha", param->m_photo3D->getAlpha());

					//glFrontFace(GL_CW);
					CW3GLFunctions::drawViewTriangles(param->m_photo3D->getVAO(), param->m_photo3D->getNindices(), GL_BACK);
					//glFrontFace(GL_CCW);
				}
				else
				{
					isPhotoSeparated = true;
				}

				CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
			}
		}
	}

	glFrontFace(GL_CCW);
	////////////// Ray Casting
	glUseProgram(m_PROGraycasting);

	if (isSecondVolume)
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[4]);
		CW3GLFunctions::clearView(false);
		glActiveTexture(m_texNumVol3D);
		glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[1]->m_texHandlerVol);
		WGLSLprogram::setUniform(m_PROGraycasting, "VolumeTex", m_texNumVol3D_);

		glActiveTexture(m_texNumSimpleSR[5]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[5]);
		WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNumSimpleSR_[5]);
		glActiveTexture(m_texNumSimpleSR[6]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[6]);
		WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNumSimpleSR_[6]);

		WGLSLprogram::setUniform(m_PROGraycasting, "ScreenSize", vec2((float)width, (float)height));
		WGLSLprogram::setUniform(m_PROGraycasting, "isShading", vol_shade_);
		WGLSLprogram::setUniform(m_PROGraycasting, "texelSize", param->m_pgMainVolume[1]->m_volTexelSize);
		WGLSLprogram::setUniform(m_PROGraycasting, "MVP", (param->m_plane->getMVP()));
		WGLSLprogram::setUniform(m_PROGraycasting, "isThereSecondVolume", false);
		WGLSLprogram::setUniform(m_PROGraycasting, "isThisSecondVolume", true);
		param->m_pgMainVolume[1]->setRayCastingParams(m_PROGraycasting);

		WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", m_bIsMIP);
		WGLSLprogram::setUniform(m_PROGraycasting, "isXRAY", m_bIsXRAY);
		WGLSLprogram::setUniform(m_PROGraycasting, "isDepthFirstHit", false);
		WGLSLprogram::setUniform(m_PROGraycasting, "isMeshSeparated", false);
		WGLSLprogram::setUniform(m_PROGraycasting, "isFrontAndBack", false);
		CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

		CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
	}

	if (isPhotoSeparated)
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[5]);
		CW3GLFunctions::clearView(true, GL_BACK);
		glUseProgram(m_PROGSRdepth);

		WGLSLprogram::setUniform(m_PROGSRdepth, "isYinvert", false);
		WGLSLprogram::setUniform(m_PROGSRdepth, "MVP", (param->m_photo3D->getMVP()));
		WGLSLprogram::setUniform(m_PROGSRdepth, "isTexture", true);

		glActiveTexture(m_texNumFACE);
		glBindTexture(GL_TEXTURE_2D, param->m_photo3D->getTexHandler());
		WGLSLprogram::setUniform(m_PROGSRdepth, "FACEtexture", m_texNumFACE_);
		WGLSLprogram::setUniform(m_PROGSRdepth, "Alpha", param->m_photo3D->getAlpha());

		CW3GLFunctions::drawViewTriangles(param->m_photo3D->getVAO(), param->m_photo3D->getNindices(), GL_BACK);
	}

	bool isOverlaySeparated = param->m_pMPROverlay->isShown() ? true : false;
	if (isOverlaySeparated)
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[5]);
		CW3GLFunctions::clearView(true, GL_BACK);
		glUseProgram(m_PROGslice);
		glActiveTexture(m_texNumVol3D);
		glDepthFunc(GL_LEQUAL);

		glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
		WGLSLprogram::setUniform(m_PROGslice, "VolumeTex", m_texNumVol3D_);

		m_pMainVRparams[0]->setSliceParams(m_PROGslice);

		param->m_pMPROverlay->drawSliceTexture(m_PROGslice);

		glUseProgram(m_PROGSRdepth);
		param->m_pMPROverlay->drawOutline(m_PROGSRdepth);

		CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
	}

	//m_PROGraycasting = m_PROGvolumeFirstHit;
	glUseProgram(m_PROGraycasting);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	glDisable(GL_DEPTH_TEST);
	if (isThereMesh)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
		CW3GLFunctions::clearView(false);

	///////////////////////////////////////////////////////////////////////////////////////
	/////////////////// isMeshSeparated: isPhotoSeparated
	/////////////////// MeshDrawn: Separated mesh 占쏙옙 占쏙옙占쏙옙 texture
	/////////////////// opacity: param->m_photo3D->m_alpha
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGraycasting, "VolumeTex", m_texNumVol3D_);
	WGLSLprogram::setUniform(m_PROGraycasting, "MaxTexSize", m_nMaxTexAxisSize);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_PROGraycasting, "TransferFunc", m_texNumTF2D_);

	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[2]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[2]);
	WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNumSimpleSR_[2]);

	WGLSLprogram::setUniform(m_PROGraycasting, "ScreenSize", vec2((float)width, (float)height));
	WGLSLprogram::setUniform(m_PROGraycasting, "isShading", vol_shade_);
	WGLSLprogram::setUniform(m_PROGraycasting, "texelSize", param->m_pgMainVolume[VolId]->m_volTexelSize);
	WGLSLprogram::setUniform(m_PROGraycasting, "BMVP", (*param->m_pgMainVolume[VolId]->m_mvp)*(glm::inverse(param->m_pgMainVolume[VolId]->m_volTexBias)));
	WGLSLprogram::setUniform(m_PROGraycasting, "MVP", (param->m_plane->getMVP()));
	WGLSLprogram::setUniform(m_PROGraycasting, "invVoltexScale", param->m_pgMainVolume[VolId]->m_invVolTexScale);

	bool isMeshSeparated = false;
	if (isPhotoSeparated || isOverlaySeparated)
		isMeshSeparated = true;

	WGLSLprogram::setUniform(m_PROGraycasting, "isMeshSeparated", isMeshSeparated);

	glActiveTexture(m_texNumSimpleSR[5]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[5]);
	WGLSLprogram::setUniform(m_PROGraycasting, "MeshDrawn", m_texNumSimpleSR_[5]);

	if (isPhotoSeparated)
	{		
		WGLSLprogram::setUniform(m_PROGraycasting, "opacity", param->m_photo3D->getAlpha());
	}
	if (isOverlaySeparated)
	{
		WGLSLprogram::setUniform(m_PROGraycasting, "opacity", 0.85f);
	}

	glActiveTexture(m_texNumSimpleSR[4]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[4]);
	WGLSLprogram::setUniform(m_PROGraycasting, "RayCastingForSecond", m_texNumSimpleSR_[4]);
	if (isSecondVolume)
	{
		WGLSLprogram::setUniform(m_PROGraycasting, "isThereSecondVolume", true);		
	}
	else
	{
		WGLSLprogram::setUniform(m_PROGraycasting, "isThereSecondVolume", false);
	}

	if (VolId == 0)
	{
		WGLSLprogram::setUniform(m_PROGraycasting, "isThisSecondVolume", false);
	}
	else
	{
		WGLSLprogram::setUniform(m_PROGraycasting, "isThisSecondVolume", true);
	}

	WGLSLprogram::setUniform(m_PROGraycasting, "isFrontAndBack", false);

	param->m_pgMainVolume[VolId]->setRayCastingParams(m_PROGraycasting);

	WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", m_bIsMIP);
	WGLSLprogram::setUniform(m_PROGraycasting, "isXRAY", m_bIsXRAY);

	if (m_bIsMIP || m_bIsXRAY)
	{
		float window_min = param->windowing_min();
		float window_width = param->windowing_norm();
		if (m_bIsXRAY)
		{
			window_min *= 1.5f;
			window_width *= 0.5f;
		}
		WGLSLprogram::setUniform(m_PROGraycasting, "WindowMin", window_min);
		WGLSLprogram::setUniform(m_PROGraycasting, "WindowWidth", window_width);
		WGLSLprogram::setUniform(m_PROGraycasting, "BoneThreshold", (float)param->m_pgMainVolume[VolId]->getTissueBoneThreshold() / 65535.0f);
	}
	WGLSLprogram::setUniform(m_PROGraycasting, "isDepthFirstHit", param->m_isDepthFirstHit);

	if (param->m_isDepthFirstHit)
	{
		glEnable(GL_DEPTH_TEST);
		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
		WGLSLprogram::setUniform(m_PROGraycasting, "matFirstHitToDepth", *(param->m_pgMainVolume[VolId]->m_matFirstHitToDepth));
	}
	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	glActiveTexture(m_texNumSegTmjMask);
	glBindTexture(GL_TEXTURE_3D, m_texSegTmjMask);
	WGLSLprogram::setUniform(m_PROGraycasting, "SegTmjMaskTex", m_texNumSegTmjMask_);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);

	glUseProgram(0);

	if (m_bIsXRAY)
		PostProcessingXray(param, width, height);

	if (isThereMesh)
		glDisable(GL_BLEND);

	param->m_isLowResDrawn = param->m_isLowRes;
	param->m_isLowRes = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	CW3GLFunctions::printError(__LINE__, kRenderBothFailMessage);
}

void CW3VREngine::PostProcessingXray(CW3Render3DParam* param, int view_width, int view_height)
{
	int size = view_width * view_height;

	float* buffer = new float[size * 4];
	memset(buffer, 0, sizeof(float)*size * 4);

	CW3ViewPlane view_plane;
	view_plane.createImage2D(view_width, view_height);

	ushort* view_buffer = view_plane.getImage2D()->getData();

	glActiveTexture(m_texNumSimpleSR[0]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, buffer);

	float max = std::numeric_limits<float>::min();
	float min = std::numeric_limits<float>::max();

	for (int i = 0; i < size * 4; i++)
	{
		max = (max > buffer[i]) ? max : buffer[i];
		min = (min < buffer[i]) ? min : buffer[i];
	}

	auto normUSHORT = [](float max, float min, float data)->ushort
	{
		const float kGammma = 1.8f;
		float val = ((data - min) / (max - min));
		return static_cast<ushort>(pow(val, 1.0f / kGammma)*65535.0f);
	};

	for (int i = 0; i < view_height; i++)
	{
		for (int j = 0; j < view_width; j++)
		{
			int img_idx = ((i*view_width) + j);
			int buf_idx = ((view_height - i - 1)*view_width * 4) + j * 4;

			ushort val = normUSHORT(max, min, buffer[buf_idx]);
			view_buffer[img_idx] = val;
		}
	}

	m_pgMPRengine->shapenPlane(&view_plane, 3);

	view_buffer = view_plane.getImage2D()->getData(); //?

	for (int i = 0; i < view_height; i++)
	{
		for (int j = 0; j < view_width; j++)
		{
			int img_idx = ((i*view_width) + j);
			int buf_idx = ((view_height - i - 1)*view_width * 4) + j * 4;

			ushort val = view_buffer[img_idx];
			float f_val = (float)val / 65535.0f;
			f_val = (f_val > 1.0f) ? 1.0f : (f_val < 0.0f) ? 0.0f : f_val;

			buffer[buf_idx] = f_val;
			buffer[buf_idx + 1] = f_val;
			buffer[buf_idx + 2] = f_val;
			buffer[buf_idx + 3] = 1.0f;
		}
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view_width, view_height, GL_RGBA, GL_FLOAT, buffer);

	delete[] buffer;
}

void CW3VREngine::SetVolShade(bool is_shade)
{
	vol_shade_ = is_shade;
	emit sigShadeOn(is_shade);
}

void CW3VREngine::Render3Dfinal(CW3Render3DParam *param, bool clear, bool isSecondVolume)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3Dfinal start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif

	glUseProgram(m_PROGfinal);
	glActiveTexture(m_texNumRC);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);

	WGLSLprogram::setUniform(m_PROGfinal, "FinalImage", m_texNumRC_);
	WGLSLprogram::setUniform(m_PROGfinal, "MVP", (param->m_plane->getMVP()));
	//glActiveTexture(m_texNumBFCUBE);
	//glBindTexture(GL_TEXTURE_2D, m_texHandlerBFCUBE);
	//WGLSLprogram::setUniform(m_PROGfinal, "FinalImage", m_texNumBFCUBE_);

	if (!isSecondVolume && clear)
	{
		CW3GLFunctions::clearView(false);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	}

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	glUseProgram(0);

	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3Dfinal done.");
}

void CW3VREngine::RenderSlice(CW3Render3DParam *param, int VolId, const bool &isXray)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSlice start.");
	int width = param->m_width;
	int height = param->m_height;
	glUseProgram(m_PROGslice);
	WGLSLprogram::setUniform(m_PROGslice, "isXray", isXray);
	WGLSLprogram::setUniform(m_PROGslice, "MVP", *(param->m_pgMainVolume[VolId]->m_mvp));
	WGLSLprogram::setUniform(m_PROGslice, "invModel", *(param->m_pgMainVolume[VolId]->m_invModel));

	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGslice, "VolumeTex", m_texNumVol3D_);

	param->m_pgMainVolume[VolId]->setSliceParams(m_PROGslice);

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());
#if defined(__APPLE__)
	glViewport(0, 0, width * 2, height * 2);
#else
	glViewport(0, 0, width, height);
#endif
	CW3GLFunctions::clearView(false);
	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSlice failed.");
}

void CW3VREngine::Render3DEndo(CW3Render3DParam *param, int VolId, bool &isReconSwitched)
{
	int width, height;
	if (param->m_isLowRes)
	{
		width = param->m_width * 0.5f;
		height = param->m_height * 0.5f;
	}
	else
	{
		width = param->m_width;
		height = param->m_height;
	}

	int NtexHandler = 3;
	if (isReconSwitched || !param->m_fbo || param->m_widthPre != width || param->m_heightPre != height)
	{
		initFrameBufferSimpleSR(param->m_fbo, param->m_depthMap, param->m_texHandler, width, height, NtexHandler);
		param->m_widthPre = width;
		param->m_heightPre = height;
		isReconSwitched = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);
	glViewport(0, 0, width, height);
	glDrawBuffers(NtexHandler, m_SimpleSR_draw_buffers);
	////////// Front Face
	if (param->m_pAirway->isVisible())
	{
		glDrawBuffer(m_SimpleSR_draw_buffers[1]);
		CW3GLFunctions::clearView(true, GL_BACK);
		WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", *(param->m_pgMainVolume[VolId]->m_mvp));
		CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_BACK);
	}
	////////// Back Face
	glDrawBuffer(m_SimpleSR_draw_buffers[2]);
	CW3GLFunctions::clearView(true, GL_FRONT);

	glUseProgram(m_PROGbackfaceCUBE);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", *(param->m_pgMainVolume[VolId]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_FRONT);

	glDepthFunc(GL_LESS);

	if (param->m_pAirway->isVisible())
	{
		glUseProgram(m_PROGsimpleSR);
		WGLSLprogram::setUniform(m_PROGsimpleSR, "isYinvert", false);
		WGLSLprogram::setUniform(m_PROGsimpleSR, "isXinvert", !param->m_isDerivedVolume); // false if derived volume
		WGLSLprogram::setUniform(m_PROGsimpleSR, "isForBackFace", true);
		WGLSLprogram::setUniform(m_PROGsimpleSR, "isScaled", false);
		WGLSLprogram::setUniform(m_PROGsimpleSR, "InverseScale", (param->m_pAirway->getInvModel()));
		WGLSLprogram::setUniform(m_PROGsimpleSR, "MVP", (param->m_pAirway->getMVP()));
		CW3GLFunctions::drawViewTriangles(param->m_pAirway->getVAO(), param->m_pAirway->getNindices(), GL_BACK);

		glUseProgram(0);
	}

	////////// Front Face
	glDrawBuffer(m_SimpleSR_draw_buffers[1]);
	if (!param->m_pAirway->isVisible())
	{
		glClear(GL_COLOR_BUFFER_BIT);

		if (param->m_isNearClipping)
		{
			////////// 1. color buffer占쏙옙占쏙옙 카占쌨띰옙 占쏙옙치占쏙옙 near clipping plane rendering
			glDepthMask(GL_FALSE);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
			glDisable(GL_DEPTH_TEST);

			glUseProgram(m_PROGendoPlane);
			param->m_pgMainVolume[VolId]->setEndoPlaneParams(m_PROGendoPlane);
			WGLSLprogram::setUniform(m_PROGendoPlane, "isForFront", true);

			CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);
		}
		glUseProgram(m_PROGbackfaceCUBE);
		glEnable(GL_DEPTH_TEST);

		////////// 2. depth buffer占쏙옙占쏙옙 nearest back face rendering
		glDepthMask(GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
		glDepthFunc(GL_LESS);

		CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_FRONT);

		////////// 3. color buffer(1占쏙옙占쏙옙 占쌓뤄옙占쏙옙 占쏙옙占쏙옙)占쏙옙 2占쏙옙占쏙옙占쏙옙 depth占쏙옙 占쏙옙占쏙옙 nearest front face rendering
		// nearest front face 占쏙옙 占쏙옙占쏙옙 占쏙옙占?1 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 texture 占쏙옙占쏙옙 front face 占쏙옙 占쏙옙
		// nearest front face 占쏙옙 占쏙옙占쏙옙 占쏙옙占?clipping plane 占쏙옙占쏙옙 占쌘울옙 front face 占쏙옙 占쏙옙占쏙옙占싹댐옙 占쏙옙李?占쏙옙 (占쌍놂옙占싹몌옙 projectino matrix 占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 clipping plane position 占싱므뤄옙)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthFunc(GL_LESS);

		CW3GLFunctions::drawView(param->m_mainVolume_vao[VolId], param->m_pgMainVolume[VolId]->m_Nindices, GL_BACK);
	}
	else
	{
		glUseProgram(m_PROGfrontfaceFinal);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", param->m_isClipped);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)param->m_clipPlanes.size());
		for (int i = 0; i < (int)param->m_clipPlanes.size(); i++)
		{
			WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), (param->m_clipPlanes[i]));
		}

		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", param->m_plane->getMVP());
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isDepthFirstHit", true);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isThisSecond", false);

		glActiveTexture(m_texNumSimpleSR[1]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNumSimpleSR_[1]);

		glActiveTexture(m_texNumSimpleSR[2]);
		glBindTexture(GL_TEXTURE_2D, param->m_texHandler[2]);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNumSimpleSR_[2]);

		glActiveTexture(m_texNumVol3D);
		glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "VolumeTex", m_texNumVol3D_);

		glActiveTexture(m_texNumTF2D);
		glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TransferFunc", m_texNumTF2D_);

		param->m_pgMainVolume[0]->setRayFirstHitFrontFaceParams(m_PROGfrontfaceFinal);

		CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);
	}

	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	CW3GLFunctions::clearView(true, GL_BACK);

	//glFrontFace(GL_CW); // by jdk 170203 for airway x占쏙옙 占쏙옙占쏙옙
	bool isThereMesh = false;
	if (param->m_pAirway->isVisible())
	{
		isThereMesh = true;

		glUseProgram(m_PROGsurfaceEndo);
		WGLSLprogram::setUniform(m_PROGsurfaceEndo, "MVP", glm::scale(vec3(1.0, -1.0, 1.0))*((param->m_pAirway->getMVP())));
		CW3GLFunctions::drawViewTriangles(param->m_pAirway->getVAO(), param->m_pAirway->getNindices(), GL_BACK);
		glUseProgram(0);
	}
	else
	{
		isThereMesh = false;
	}
	glUseProgram(0);

	//glFrontFace(GL_CCW); // by jdk 170203 for airway x占쏙옙 占쏙옙占쏙옙

	////////// Ray Casting
	glUseProgram(m_PROGendoraycasting);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	glDisable(GL_DEPTH_TEST);
	if (isThereMesh)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		CW3GLFunctions::clearView(false);
	}

	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "TransferFunc", m_texNumTF2D_);

	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "EntryPositions", m_texNumSimpleSR_[1]);

	glActiveTexture(m_texNumSimpleSR[2]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[2]);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "ExitPositions", m_texNumSimpleSR_[2]);

	WGLSLprogram::setUniform(m_PROGendoraycasting, "ScreenSize", vec2((float)width, (float)height));
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isShading", vol_shade_);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "texelSize", param->m_pgMainVolume[VolId]->m_volTexelSize);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isMIP", m_bIsMIP);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isXRAY", m_bIsXRAY);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isDepthFirstHit", param->m_isDepthFirstHit);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isThereSecondVolume", false);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isThisSecondVolume", false);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "invVoltexScale", param->m_pgMainVolume[VolId]->m_invVolTexScale);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "BMVP", (*param->m_pgMainVolume[VolId]->m_mvp)*(glm::inverse(param->m_pgMainVolume[VolId]->m_volTexBias)));
	WGLSLprogram::setUniform(m_PROGendoraycasting, "MVP", (param->m_plane->getMVP()));
	if (m_bIsMIP || m_bIsXRAY)
	{
		float window_min = param->windowing_min();
		float window_width = param->windowing_norm();
		if (m_bIsXRAY)
		{
			window_min *= 1.5f;
			window_width *= 0.5f;
		}
		WGLSLprogram::setUniform(m_PROGendoraycasting, "WindowMin", window_min);
		WGLSLprogram::setUniform(m_PROGendoraycasting, "WindowWidth", window_width);
		WGLSLprogram::setUniform(m_PROGendoraycasting, "BoneThreshold", param->m_pgMainVolume[VolId]->getTissueBoneThreshold() / 65535.0f);
	}

	if (param->m_isDepthFirstHit)
	{
		WGLSLprogram::setUniform(m_PROGendoraycasting, "matFirstHitToDepth", *(param->m_pgMainVolume[VolId]->m_matFirstHitToDepth));
	}
	//WGLSLprogram::setUniform(m_PROGendoraycasting, "texelSize", param->m_volTexelSize);
	param->m_pgMainVolume[VolId]->setRayCastingParams(m_PROGendoraycasting);

	WGLSLprogram::setUniform(m_PROGendoraycasting, "isMeshSeparated", false);
	WGLSLprogram::setUniform(m_PROGendoraycasting, "isFrontAndBack", true);

	glDepthMask(GL_TRUE);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	if (isThereMesh)
		glDisable(GL_BLEND);

	param->m_isLowResDrawn = param->m_isLowRes;
	param->m_isLowRes = false;
	////////// Final
	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

	glViewport(0, 0, param->m_width, param->m_height);

	glUseProgram(m_PROGfinal);
	glActiveTexture(m_texNumRC);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);
	WGLSLprogram::setUniform(m_PROGfinal, "FinalImage", m_texNumRC_);

	CW3GLFunctions::clearView(false);
	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3DEndo failed.");
}

void CW3VREngine::RenderSliceEndo(CW3Render3DParam *param, int VolId, bool &isReconSwitched)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());
	CW3GLFunctions::clearView(false);

#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif

	glUseProgram(m_PROGendoPlane);

	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[VolId]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGendoPlane, "VolumeTex", m_texNumVol3D_);

	WGLSLprogram::setUniform(m_PROGendoPlane, "isForFront", false);
	param->m_pgMainVolume[VolId]->setEndoPlaneParams(m_PROGendoPlane);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSliceEndo failed.");
}

void CW3VREngine::RenderSurface(CW3Render3DParam *param, int VolId, bool isTexture, bool &isReconSwitched)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSurface start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

	glUseProgram(m_PROGSR);

	WGLSLprogram::setUniform(m_PROGSR, "isYinvert", false);
	if (isTexture)
	{
		WGLSLprogram::setUniform(m_PROGSR, "isTexture", true);

		glActiveTexture(m_texNumFACE);
		glBindTexture(GL_TEXTURE_2D, param->m_photo3D->getTexHandler());
		WGLSLprogram::setUniform(m_PROGSR, "FACEtexture", m_texNumFACE_);
	}
	else
	{
		WGLSLprogram::setUniform(m_PROGSR, "isTexture", false);
	}

	WGLSLprogram::setUniform(m_PROGSR, "Alpha", param->m_photo3D->getAlpha());
	WGLSLprogram::setUniform(m_PROGSR, "MVP", (param->m_photo3D->getMVP()));

#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif

	CW3GLFunctions::clearView(true, GL_BACK);
	CW3GLFunctions::drawViewTriangles(param->m_photo3D->getVAO(), param->m_photo3D->getNindices(), GL_BACK);

	glUseProgram(0);
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderSurface done.");
}

void CW3VREngine::RenderMPR(CW3Render3DParam *param, int VolId, bool isDrawBoth, bool &isReconSwitched)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMPR start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif

	glUseProgram(m_PROGplaneDisplay);
	glActiveTexture(m_texNumPlane);
	glBindTexture(GL_TEXTURE_2D, param->m_VRtextures.m_texHandler[VolId]);
	WGLSLprogram::setUniform(m_PROGplaneDisplay, "ImageSlice", m_texNumPlane_);

	WGLSLprogram::setUniform(m_PROGplaneDisplay, "minVal", param->windowing_min());
	WGLSLprogram::setUniform(m_PROGplaneDisplay, "normVal", param->windowing_norm());
	WGLSLprogram::setUniform(m_PROGplaneDisplay, "MVP", (param->m_plane->getMVP()));
	WGLSLprogram::setUniform(m_PROGplaneDisplay, "isNormalized", false);

	if (!VolId)
	{
		//		if (param->m_pNerve->isVisible())
		{
			glActiveTexture(m_texNumMaskPlane);
			glBindTexture(GL_TEXTURE_2D, param->m_VRtextures.m_texHandler[1]);
			WGLSLprogram::setUniform(m_PROGplaneDisplay, "MaskSlice", m_texNumMaskPlane_);
		}

		WGLSLprogram::setUniform(m_PROGplaneDisplay, "isCanalShown", (param->m_pNerve && param->m_pNerve->isVisible()));
		WGLSLprogram::setUniform(m_PROGplaneDisplay, "isSecond", false);

		CW3GLFunctions::clearView(false);
	}
	else
	{
		WGLSLprogram::setUniform(m_PROGplaneDisplay, "isCanalShown", false);
		WGLSLprogram::setUniform(m_PROGplaneDisplay, "isSecond", true);

		if (isDrawBoth)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
		{
			CW3GLFunctions::clearView(false);
		}
	}

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	isReconSwitched = false;
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMPR failed.");
}

void CW3VREngine::RenderImplantMPR(CW3Render3DParam *param, int VolId, bool &isReconSwitched)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplantMPR start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());
#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_PROGimplant);
	if (param->m_isImplantShown)
	{
		for (int i = 0; i < MAX_IMPLANT; i++)
		{
			if (param->g_is_implant_exist_[i])
			{
				WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", false);
				//WGLSLprogram::setUniform(m_PROGimplant, "isWire", true);
				//param->m_pImplant[i]->setMeshColor(glm::vec3(0.0f, 1.0f, 0.0f));
				setUniformSingleImplant(param, i, true);
				CW3GLFunctions::drawWire(param->m_pImplant[i]->getVAO(), param->m_pImplant[i]->getNindices());
			}
		}
	}

	isReconSwitched = false;
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplantMPR failed.");
}

void CW3VREngine::RenderImplantMPRonlySelected(CW3Render3DParam *param, int VolId, int implantID)
{
	if (param->m_isImplantShown && param->g_is_implant_exist_[implantID])
	{
		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplantMPRonlySelected start.");

		glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

#if defined(__APPLE__)
		glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
		glViewport(0, 0, param->m_width, param->m_height);
#endif
		glUseProgram(m_PROGimplant);
		WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", false);
		//WGLSLprogram::setUniform(m_PROGimplant, "isWire", true);

		//param->m_pImplant[implantID]->setMeshColor(glm::vec3(0.0f, 1.0f, 0.0f));
		setUniformSingleImplant(param, implantID, true);
		CW3GLFunctions::drawWire(param->m_pImplant[implantID]->getVAO(),
			param->m_pImplant[implantID]->getNindices());
		glDisable(GL_DEPTH_TEST);

		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplantMPRonlySelected failed.");
	}
}

void CW3VREngine::RenderImplant3D(CW3Render3DParam *param, int implantID)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplant3D start.");

	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());
#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif
	CW3GLFunctions::clearView(true, GL_BACK);

	if (param->g_is_implant_exist_[implantID])
	{
		glUseProgram(m_PROGimplant);
		WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", false);
		//WGLSLprogram::setUniform(m_PROGimplant, "isWire", false);

		//param->m_pImplant[implantID]->setMeshColor(glm::vec3(1.0f, 1.0f, 1.0f));

		setUniformImplant(param);
		setUniformSingleImplant(param, implantID, false);

		CW3GLFunctions::drawViewTriangles(param->m_pImplant[implantID]->getVAO(),
			param->m_pImplant[implantID]->getNindices(), GL_BACK);
	}

	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderImplant3D failed.");
}

void CW3VREngine::RenderMeshMPR(CW3Render3DParam *param, int VolId, bool &isReconSwitched)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMeshMPR start.");

#if 0
	int width, height;
	width = 800;
	height = 800;

	if (isReconSwitched || !param->m_fbo || param->m_widthPre != width || param->m_heightPre != height)
	{
		initFrameBufferSimpleSR(
			param->m_fbo, 
			param->m_depthMap, 
			param->m_texHandler,
			width, 
			height, 
			1
		);
		param->m_widthPre = width;
		param->m_heightPre = height;

		isReconSwitched = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, param->defaultFBO());

#if defined(__APPLE__)
	glViewport(0, 0, param->m_width * 2, param->m_height * 2);
#else
	glViewport(0, 0, param->m_width, param->m_height);
#endif
#endif

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (param->m_isImplantShown)
	{
		for (int i = 0; i < MAX_IMPLANT; i++)
		{
			if (param->g_is_implant_exist_[i])
			{
				glUseProgram(m_PROGimplant);
				WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", false);
				//WGLSLprogram::setUniform(m_PROGimplant, "isWire", true);

				//param->m_pImplant[i]->setMeshColor(glm::vec3(0.0f, 1.0f, 0.0f));

				setUniformSingleImplant(param, i, true);

				CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMeshMPR failed.");
				CW3GLFunctions::drawWire(param->m_pImplant[i]->getVAO(), param->m_pImplant[i]->getNindices());

				glUseProgram(0);
			}
		}
	}
	if (param->m_photo3D->isVisible())
	{
		glUseProgram(m_PROGimplant);
		WGLSLprogram::setUniform(m_PROGimplant, "MVP", (param->m_photo3D->getMVP()));
		WGLSLprogram::setUniform(m_PROGimplant, "isYinvert", false);
		WGLSLprogram::setUniform(m_PROGimplant, "isBoneDensity", false);
		WGLSLprogram::setUniform(m_PROGimplant, "isWire", true);
		WGLSLprogram::setUniform(m_PROGimplant, "meshColor", glm::vec3(0.0f, 1.0f, 0.0f));

		CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMeshMPR failed.");
		CW3GLFunctions::drawWire(param->m_photo3D->getVAO(), param->m_photo3D->getNindices());
		glUseProgram(0);
	}

	isReconSwitched = false;
	glDisable(GL_DEPTH_TEST);

#if 0
	CW3GLFunctions::writeFileBMPTexture2D(param->m_texHandler[0], width, height, "implant_mpr_wire.bmp");
#endif

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderMeshMPR failed.");
}

void CW3VREngine::Render3DOTFPreset(CW3Render3DParam *param, QString filePath)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3DOTFPreset start.");

	bool b = false;
	Render3Dboth(param, 0, b, b);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);

	int width = 0, height = 0;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	GLubyte *texture = nullptr;
	W3::p_allocate_1D(&texture, width * height * 4);
	memset(texture, 0, width * height * sizeof(GLubyte) * 4);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

	int newWidth = sqrt(pow(height, 2) / 2);
	int newHeight = newWidth;
	if (width < height)
	{
		newWidth = sqrt(pow(width, 2) / 2);
		newHeight = newWidth;
	}
	int newX = (width * 0.5f) - (newWidth * 0.5f);
	int newY = (height * 0.5f) - (newHeight * 0.5f);

	QImage img(texture, width, height, QImage::Format_RGBA8888);
	img = img.copy(newX, newY, newWidth, newHeight);
	bool res = img.save(filePath, "BMP");

	SAFE_DELETE_ARRAY(texture);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3DOTFPreset failed.");
}
void CW3VREngine::Render3DTeeth(CW3Render3DParam *param)
{
	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3DTeeth start.");
	////must be debug.
	//this->recompileRaycasting();
	////recompile for test end

	int width, height;
	if (param->m_isLowRes)
	{
		width = param->m_width*low_res_frame_buffer_resize_factor_;
		height = param->m_height*low_res_frame_buffer_resize_factor_;

		param->m_pgMainVolume[0]->setLowRes(true);
	}
	else
	{
		width = param->m_width;
		height = param->m_height;

		param->m_pgMainVolume[0]->setLowRes(false);
	}

	if (!param->m_fbo || param->m_widthPre != width || param->m_heightPre != height)
	{
		initFrameBufferSimpleSR(param->m_fbo, param->m_depthMap, param->m_texHandler,
			width, height, NW3Render3DParam::kNumTexHandle);
		param->m_widthPre = width;
		param->m_heightPre = height;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);
	glViewport(0, 0, width, height);

	////////////// For Volume Cube

	//////// Front Face
	glUseProgram(m_PROGfrontfaceCUBE);
	glDrawBuffer(m_SimpleSR_draw_buffers[1]);
	CW3GLFunctions::clearView(true, GL_BACK);
	WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", *(param->m_pgMainVolume[0]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[0], param->m_pgMainVolume[0]->m_Nindices, GL_BACK);

	//////////// Back Face
	glUseProgram(m_PROGbackfaceCUBE);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	CW3GLFunctions::clearView(true, GL_FRONT);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", *(param->m_pgMainVolume[0]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[0], param->m_pgMainVolume[0]->m_Nindices, GL_FRONT);

	////// Extract Front Face
	glDrawBuffer(m_SimpleSR_draw_buffers[2]);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum buffers[2] = { m_SimpleSR_draw_buffers[1], m_SimpleSR_draw_buffers[2] };
	glDrawBuffers(2, buffers);

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", (param->m_isClipped));
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "numClipPlanes", (int)param->m_clipPlanes.size());
	for (int i = 0; i < (int)param->m_clipPlanes.size(); i++)
	{
		WGLSLprogram::setUniform(m_PROGfrontfaceFinal, QString("clipPlanes[%1]").arg(i).toStdString().c_str(), (param->m_clipPlanes[i]));
	}

	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", (param->m_plane->getMVP()));
	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[0]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNumSimpleSR_[0]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isDepthFirstHit", false);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isThisSecond", false);
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[0]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2DTeeth);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DTeethHandler);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TransferFunc", m_texNumTF2DTeeth_);
	param->m_pgMainVolume[0]->setRayFirstHitFrontFaceParams(m_PROGfrontfaceFinal);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	////////////// Ray Casting
	glUseProgram(m_PROGraycasting);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	glDisable(GL_DEPTH_TEST);
	CW3GLFunctions::clearView(false);

	///////////////////////////////////////////////////////////////////////////////////////
	/////////////////// isMeshSeparated: isPhotoSeparated
	/////////////////// MeshDrawn: Separated mesh 占쏙옙 占쏙옙占쏙옙 texture
	/////////////////// opacity: param->m_photo3D->m_alpha
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[0]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGraycasting, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2DTeeth);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DTeethHandler);
	WGLSLprogram::setUniform(m_PROGraycasting, "TransferFunc", m_texNumTF2DTeeth_);

	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGraycasting, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[2]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[2]);
	WGLSLprogram::setUniform(m_PROGraycasting, "ExitPositions", m_texNumSimpleSR_[2]);

	WGLSLprogram::setUniform(m_PROGraycasting, "ScreenSize", vec2((float)width, (float)height));
	WGLSLprogram::setUniform(m_PROGraycasting, "isShading", vol_shade_);
	WGLSLprogram::setUniform(m_PROGraycasting, "texelSize", param->m_pgMainVolume[0]->m_volTexelSize);
	WGLSLprogram::setUniform(m_PROGraycasting, "MVP", (param->m_plane->getMVP()));
	WGLSLprogram::setUniform(m_PROGraycasting, "invVoltexScale", param->m_pgMainVolume[0]->m_invVolTexScale);

	WGLSLprogram::setUniform(m_PROGraycasting, "isMeshSeparated", false);
	WGLSLprogram::setUniform(m_PROGraycasting, "isThereSecondVolume", false);
	WGLSLprogram::setUniform(m_PROGraycasting, "isThisSecondVolume", false);
	WGLSLprogram::setUniform(m_PROGraycasting, "isFrontAndBack", false);

	param->m_pgMainVolume[0]->setRayCastingParams(m_PROGraycasting);

	WGLSLprogram::setUniform(m_PROGraycasting, "isMIP", false);
	WGLSLprogram::setUniform(m_PROGraycasting, "isDepthFirstHit", false);

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	param->m_isLowResDrawn = param->m_isLowRes;
	param->m_isLowRes = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::Render3DTeeth failed.");
}

void CW3VREngine::setSegTmjTexture(unsigned char **data, const int &width, const int &height, const int &depth)
{
	if (!getVol(0))
		return;

	if (!data)
		return;

	/*int width = getVol(0)->width() / 2;
	int height = getVol(0)->height() / 2;
	int depth = getVol(0)->depth() / 2;*/

	makeCurrent();

	glGenTextures(1, &m_texSegTmjMask);
	//glActiveTexture(m_texNumSegTmjMask);
	glBindTexture(GL_TEXTURE_3D, m_texSegTmjMask);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// pixel transfer happens here from client to OpenGL server
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8_SNORM, width, height, depth);
	for (int i = 0; i < depth; i++)
	{
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, i, width, height, 1, GL_RED, GL_UNSIGNED_BYTE, data[i]);
	}

	doneCurrent();
}

void CW3VREngine::RenderVolumeFirstHit(CW3Render3DParam *param, GLuint *frameBuffer)
{
#if ___DEBUG
	this->recompileRaycasting();
#endif

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderVolumeFirstHit failed.");

	//printf("start RenderVolumeFirstHit\r\n");
	int width, height;

	width = param->m_width;
	height = param->m_height;

	if (!param->m_fbo || param->m_widthPre != width || param->m_heightPre != height)
	{
		initFrameBufferSimpleSR(param->m_fbo, param->m_depthMap, param->m_texHandler,
			width, height, NW3Render3DParam::kNumTexHandle);
		param->m_widthPre = width;
		param->m_heightPre = height;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);
	glViewport(0, 0, width, height);
	glDrawBuffers(NW3Render3DParam::kNumTexHandle, m_SimpleSR_draw_buffers);
	CW3GLFunctions::clearView(true);

	////////////// For Volume Cube

	//////////// Front Face
	glUseProgram(m_PROGfrontfaceCUBE);
	glDrawBuffer(m_SimpleSR_draw_buffers[1]);
	CW3GLFunctions::clearView(true, GL_BACK);
	WGLSLprogram::setUniform(m_PROGfrontfaceCUBE, "MVP", *(param->m_pgMainVolume[0]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[0], param->m_pgMainVolume[0]->m_Nindices, GL_BACK);

	//////////// Back Face
	glUseProgram(m_PROGbackfaceCUBE);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	CW3GLFunctions::clearView(true, GL_FRONT);
	WGLSLprogram::setUniform(m_PROGbackfaceCUBE, "MVP", *(param->m_pgMainVolume[0]->m_mvp));
	CW3GLFunctions::drawView(param->m_mainVolume_vao[0], param->m_pgMainVolume[0]->m_Nindices, GL_FRONT);

	//////////// Extract Front Face
	glDrawBuffer(m_SimpleSR_draw_buffers[2]);
	glClearDepth(1.0f);
	glDepthFunc(GL_LESS);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLenum buffers[2] = { m_SimpleSR_draw_buffers[1], m_SimpleSR_draw_buffers[2] };
	glDrawBuffers(2, buffers);

	glUseProgram(m_PROGfrontfaceFinal);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isPlaneClipped", false);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "MVP", (param->m_plane->getMVP()));
	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[0]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[0]);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "ExitPositions", m_texNumSimpleSR_[0]);

	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isDepthFirstHit", true);

	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "isThisSecond", false);
	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[0]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_PROGfrontfaceFinal, "TransferFunc", m_texNumTF2D_);

	param->m_pgMainVolume[0]->setRayFirstHitFrontFaceParams(m_PROGfrontfaceFinal);

	//WGLSLprogram::setUniform(m_PROGfrontfaceFinalforFaceSim, "MinValue", param->m_pgMainVolume[0]->getAirTissueThreshold());

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	//////////// First Hit
	glUseProgram(m_PROGvolumeFirstHitforFaceSim);
	glDrawBuffer(m_SimpleSR_draw_buffers[0]);
	//glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *frameBuffer);
	glDisable(GL_DEPTH_TEST);

	CW3GLFunctions::clearView(false);

	glActiveTexture(m_texNumVol3D);
	glBindTexture(GL_TEXTURE_3D, param->m_pgMainVolume[0]->m_texHandlerVol);
	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "VolumeTex", m_texNumVol3D_);

	glActiveTexture(m_texNumTF2D);
	glBindTexture(GL_TEXTURE_2D, m_texTF2DHandler);
	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "TransferFunc", m_texNumTF2D_);

	glActiveTexture(m_texNumSimpleSR[1]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[1]);
	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "EntryPositions", m_texNumSimpleSR_[1]);
	glActiveTexture(m_texNumSimpleSR[2]);
	glBindTexture(GL_TEXTURE_2D, param->m_texHandler[2]);
	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "ExitPositions", m_texNumSimpleSR_[2]);

	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "VolRange", param->m_pgMainVolume[0]->m_VolScaleIso);
	WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "MVP", param->m_plane->getMVP());

	param->m_pgMainVolume[0]->setRayCastingParams(m_PROGvolumeFirstHitforFaceSim);

	//WGLSLprogram::setUniform(m_PROGvolumeFirstHitforFaceSim, "MinValue", param->m_pgMainVolume[0]->getAirTissueThreshold());

	CW3GLFunctions::drawView(param->m_plane->getVAO(), param->m_plane->getNindices(), GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CW3GLFunctions::printError(__LINE__, "CW3VREngine::RenderVolumeFirstHit failed.");
}

///占쏙옙占쌘듸옙

void CW3VREngine::ApplyPreferences()
{
	GlobalPreferences::Quality2 volume_rendering_quality = GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality;
	switch (volume_rendering_quality)
	{
	case GlobalPreferences::Quality2::High:
		low_res_frame_buffer_resize_factor_ = 0.5f;
		break;
	case GlobalPreferences::Quality2::Low:
		low_res_frame_buffer_resize_factor_ = 0.25f;
		break;
	}

	for (int i = 0; i < 5; i++)
		if (m_pMainVRparams[i])
			m_pMainVRparams[i]->ApplyPreferences();
}

VolumeRenderer* CW3VREngine::GetVolumeRenderer(int id) const
{
	if (id == 0)
		return &RendererManager::GetInstance().renderer_vol(Will3DEngine::VolType::VOL_MAIN);
	else if (id == 1)
		return &RendererManager::GetInstance().renderer_vol(Will3DEngine::VolType::VOL_SECOND);
	else
	{
		using common::Logger;
		using common::LogType;
		auto logger = Logger::instance();
		logger->PrintAndAssert(LogType::ERR, "invalid volume type");
		return nullptr;
	}
}

bool CW3VREngine::VolumeTracking(CW3Render3DParam* param, const QPointF& mouse_pos, glm::vec3& picked_volume_pos)
{
	try
	{
		int width = param->m_width;
		int height = param->m_height;

		if (mouse_pos.x() > width || mouse_pos.y() > height)
		{
			throw std::runtime_error("Mouse position is not in range.");
		}

#if 0
		CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/tex_handler_2.png", param->m_texHandler[2], GL_RGBA, GL_UNSIGNED_BYTE);
		CW3GLFunctions::SaveTexture2D("c:/users/jdk/desktop/tex_handler_1.png", param->m_texHandler[1], GL_RGBA, GL_UNSIGNED_BYTE);
#endif

		int pos_x = mouse_pos.x();
		int pos_y = height - mouse_pos.y();

		glBindFramebuffer(GL_FRAMEBUFFER, param->m_fbo);

		vec4 dir_len;
		vec4 first_hit;

		glReadBuffer(m_SimpleSR_draw_buffers[2]);
		glReadPixels(pos_x, pos_y, 1, 1, GL_RGBA, GL_FLOAT, &dir_len);

		float len = dir_len[3];

		CW3GLFunctions::printError(__LINE__, __FUNCTION__);

		if (len == 0.0f || std::numeric_limits<float>::infinity() == len)
		{
			//throw std::runtime_error("position not found");
			return false;
		}

		glReadBuffer(m_SimpleSR_draw_buffers[1]);
		glReadPixels(pos_x, pos_y, 1, 1, GL_RGBA, GL_FLOAT, &first_hit);

		CW3VolumeRenderParam* volume_param = getVRparams(0);

		mat4 inv_tex_bias = glm::inverse(volume_param->m_volTexBias);

		picked_volume_pos = vec3(inv_tex_bias * vec4(vec3(first_hit), 1.0f));

		return true;
	}
	catch (std::exception& e)
	{
		std::string err_msg = e.what();
		common::Logger::instance()->Print(common::LogType::ERR, __FUNCTION__ + std::string(" : ") + err_msg);
		return false;
	}

	return true;
}
