#include "W3SurfaceRenderingCL.h"
/*=========================================================================

File:			class CW3SurfaceRenderingCL
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-28
Last date:		2016-02-12

=========================================================================*/
#include <ctime>
#include <qopenglwidget.h>

#include "../../../Common/Common/W3Logger.h"
#include "../../../Common/GLfunctions/WGLSLprogram.h"
#include "../../../Common/GLfunctions/W3GLFunctions.h"
#include "../../../Resource/Resource/W3TRDsurface.h"

#include "../../../Core/CLPlatform/WCLplatform.h"

#include "../../../UIModule/UIGLObjects/W3GLObject.h"

#include "../../../Module/VREngine/W3VREngine.h"
#include "../../../Module/VREngine/W3VRengineTextures.h"
#include "../../../Module/VREngine/W3Render3DParam.h"

using common::Logger;
using common::LogType;

CW3SurfaceRenderingCL::CW3SurfaceRenderingCL(CLplatform *OCL)
	: WCLprogram(OCL) {
	m_model = glm::mat4(1.0f);
	m_model2 = glm::mat4(1.0f);
	m_view = glm::mat4(1.0f);
	m_projection = glm::mat4(1.0f);
	m_mvp = glm::mat4(1.0f);

	m_vboSRref[0] = 0;
	m_vboSRref[1] = 0;
	m_vboSRref[2] = 0;
}

CW3SurfaceRenderingCL::~CW3SurfaceRenderingCL(void) {
	clearBuffers();
}

void CW3SurfaceRenderingCL::ShareTexture(cl_mem *mem, cl_mem_flags flags, GLuint textureHandle) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	cl_int err = 0;

	*mem = clCreateFromGLTexture2D(m_pOCL->getContext(), flags, GL_TEXTURE_2D, 0, textureHandle, &err);
	if (err != CL_SUCCESS) {
		std::cerr << "W3SurfaceRenderingCL [line: " << __LINE__ << "] " << "ShareTexture failed." << std::endl;
		m_pOCL->printError(err);
	}
}

void CW3SurfaceRenderingCL::setVRengine(CW3VREngine *VREngine) {
	m_pgVRengine = VREngine;
	m_pgVRengine->getMVP(m_model, m_view, m_projection, m_mvp);
	m_pgGLWidget = m_pgVRengine->getGLWidget();

	//m_pgVRengine->makeCurrent();
	//m_pgVRengine->getVAOSRs(m_vaoSRphoto, m_Nphoto, m_vaoSRmc, m_Nmc);
	//m_pgVRengine->doneCurrent();
}

QVector3D* CW3SurfaceRenderingCL::getVolRangeGL(void) {
	return m_pgVRengine->getVolRangeGL();
}

void CW3SurfaceRenderingCL::setMVP(float *sol) {
#if 0
	m_model2 = glm::rotate(sol[5], glm::vec3(0.0f, 1.0f, 0.0f))*
		glm::rotate(sol[4], glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::rotate(sol[3], glm::vec3(0.0f, 0.0f, 1.0f))*glm::translate(glm::vec3(sol[1], sol[2], sol[0]))*m_model;
#else
	m_model2 =
		glm::rotate(sol[3], glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(sol[4], glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(sol[5], glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::translate(glm::vec3(sol[2], sol[1], sol[0])) *
		m_model;
#endif
	m_mvp = m_projection * m_view*m_model2;
}

void CW3SurfaceRenderingCL::rendering(cl_mem *mem, int nx, int ny, float *Sol, bool isRef) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	setMVP(Sol);

	cl_int status = 0;

	m_pgVRengine->makeCurrent();

	if (isRef) {
		if (m_FBhandlerMC == 0) {
			CW3GLFunctions::initFrameBuffer(m_FBhandlerMC, m_DepthHandlerMC, m_TexHandlerMC, nx, ny);
			ShareTexture(mem, CL_MEM_READ_ONLY, m_TexHandlerMC);
		} else {
			status = clEnqueueReleaseGLObjects(m_pOCL->getCommandQueue(), 1, mem, 0, nullptr, nullptr);
			checkError(status, __LINE__, "CW3SurfaceRenderingCL::rendering");
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBhandlerMC);
	} else {
		if (m_FBhandlerPhoto == 0) {
			CW3GLFunctions::initFrameBuffer(m_FBhandlerPhoto, m_DepthHandlerPhoto, m_TexHandlerPhoto, nx, ny);
			ShareTexture(mem, CL_MEM_READ_ONLY, m_TexHandlerPhoto);
		} else {
			status = clEnqueueReleaseGLObjects(m_pOCL->getCommandQueue(), 1, mem, 0, nullptr, nullptr);
			checkError(status, __LINE__, "CW3SurfaceRenderingCL::rendering");
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBhandlerPhoto);
	}
	glViewport(0, 0, nx, ny);

	GLuint prog = m_pgVRengine->getPROGSRcoord();
	glUseProgram(prog);
	WGLSLprogram::setUniform(prog, "MVP", m_mvp);
	WGLSLprogram::setUniform(prog, "MODEL", m_model2);

	CW3GLFunctions::clearView(true, GL_BACK);
	if (isRef) {
		if (m_pgSurfaceRef) {
			CW3GLFunctions::drawViewTriangles(m_vaoSRmc, m_Nmc, GL_BACK);
		} else {
			printf("make firsthit surface\r\n");
			clock_t start = clock();

			CW3Render3DParam param(m_pgVRengine->getGLWidget());
			param.m_plane = new CW3GLObject(GL_TRIANGLES);

			param.m_pgMainVolume[0] = m_pgVRengine->getVRparams(0);
			param.m_pgMainVolume[0]->m_isShown = true;

			param.m_width = nx;
			param.m_height = ny;

			param.m_pgMainVolume[0]->m_VolScaleIso = *m_pgVRengine->getVolRange(0);
			param.m_pgMainVolume[0]->m_mvp = &m_mvp;

			m_pgVRengine->setActiveIndex(&param.m_mainVolume_vao[0], 0);

			if (param.m_plane->getVAO())
				param.m_plane->clearVAOVBO();

			unsigned int vao = 0;
			m_pgVRengine->initVAOplane(&vao);
			param.m_plane->setVAO(vao);
			param.m_plane->setNindices(6);

			glm::mat4 viewForFinal = glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f),
												 glm::vec3(0.0f, 0.0f, 0.0f),
												 glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 projForFinal = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
			param.m_plane->setMVP(glm::mat4(1.0f), viewForFinal, projForFinal);

			m_pgVRengine->RenderVolumeFirstHit(&param, &m_FBhandlerMC);

			clock_t end = clock();
			long elapsedTime = end - start;
			printf("make firsthit surface : %d ms\r\n", elapsedTime);

			GLuint PROGfinal = m_pgVRengine->getPROGfinal();

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBhandlerMC);

			glViewport(0, 0, param.m_width, param.m_height);

			GLenum rc = 0;
			int rc_ = 0;
			m_pgVRengine->getRCTexParam(rc, rc_);

			glUseProgram(PROGfinal);
			glActiveTexture(rc);
			glBindTexture(GL_TEXTURE_2D, param.m_texHandler[0]);

			WGLSLprogram::setUniform(PROGfinal, "FinalImage", rc_);
			WGLSLprogram::setUniform(PROGfinal, "MVP", (param.m_plane->getMVP()));

			CW3GLFunctions::clearView(false);
			CW3GLFunctions::drawView(param.m_plane->getVAO(), param.m_plane->getNindices(), GL_BACK);

			glUseProgram(0);
		}

		//#if 0
		//		QString name;
		//		if (m_pgSurfaceRef)
		//			name = "ref_mc.jpg";
		//		else
		//			name = "ref_firsthit.jpg";
		//
		//		float *data = nullptr;
		//		W3::p_allocate_1D(&data, nx * ny * 4);
		//		CW3GLFunctions::readTexture2D(m_TexHandlerMC, data);
		//
		//		byte *b = nullptr;
		//		W3::p_allocate_1D(&b, nx * ny * 4);
		//		for (int i = 0; i < nx * ny * 4; i++)
		//		{
		//			b[i] = (byte)(data[i]);
		//		}
		//
		//		QImage image(b, nx, ny, QImage::Format_RGBA8888);
		//
		//		QImageWriter imgWriter(name, "jpg");
		//		imgWriter.write(image);
		//
		//		/*FILE *FWRITE;
		//		fopen_s(&FWRITE, "ref.raw", "wb");
		//		fwrite(b, sizeof(byte), nx*ny * 4, FWRITE);
		//		fclose(FWRITE);*/
		//
		//		//printf("********** ref : %d, %d\r\n", nx, ny);
		//
		//		SAFE_DELETE_ARRAY(b);
		//		SAFE_DELETE_ARRAY(data);
		//#endif
	} else {
		CW3GLFunctions::drawViewTriangles(m_vaoSRphoto, m_Nphoto, GL_BACK);
		glUseProgram(0);
		//#if 0
		//		QFile moving("moving.jpg");
		//		//if (!moving.exists())
		//		{
		//			float *data = nullptr;
		//			W3::p_allocate_1D(&data, nx * ny * 4);
		//			CW3GLFunctions::readTexture2D(m_TexHandlerPhoto, data);
		//
		//			byte *b = nullptr;
		//			W3::p_allocate_1D(&b, nx * ny * 4);
		//			for (int i = 0; i < nx * ny * 4; i++)
		//			{
		//				b[i] = (byte)(data[i]);
		//			}
		//
		//			QImage image(b, nx, ny, QImage::Format_RGBA8888);
		//
		//			QImageWriter imgWriter("moving.jpg", "jpg");
		//			imgWriter.write(image);
		//
		//			/*FILE *FWRITE;
		//			fopen_s(&FWRITE, "moving.raw", "wb");
		//			fwrite(data, sizeof(float), nx*ny * 4, FWRITE);
		//			fclose(FWRITE);*/
		//
		//			//printf("********** moving : %d, %d\r\n", nx, ny);
		//
		//			SAFE_DELETE_ARRAY(b);
		//			SAFE_DELETE_ARRAY(data);
		//		}
		//#endif
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//glFinish();

	status = clEnqueueAcquireGLObjects(m_pOCL->getCommandQueue(), 1, mem, 0, nullptr, nullptr);
	checkError(status, __LINE__, "CW3SurfaceRenderingCL::rendering");

	m_pgVRengine->doneCurrent();
}

void CW3SurfaceRenderingCL::clearBuffers() {
	if (!m_pgGLWidget)
		return;

	m_pgGLWidget->makeCurrent();
	if (m_FBhandlerMC) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &m_FBhandlerMC);
		m_FBhandlerMC = 0;
	}

	if (m_FBhandlerPhoto) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &m_FBhandlerPhoto);
		m_FBhandlerPhoto = 0;
	}

	if (m_DepthHandlerMC) {
		glDeleteRenderbuffers(1, &m_DepthHandlerMC);
		m_DepthHandlerMC = 0;
	}

	if (m_DepthHandlerPhoto) {
		glDeleteRenderbuffers(1, &m_DepthHandlerPhoto);
		m_DepthHandlerPhoto = 0;
	}

	if (m_TexHandlerMC) {
		glDeleteTextures(1, &m_TexHandlerMC);
		m_TexHandlerMC = 0;
	}

	if (m_TexHandlerPhoto) {
		glDeleteTextures(1, &m_TexHandlerPhoto);
		m_TexHandlerPhoto = 0;
	}
	m_pgGLWidget->doneCurrent();
}

void CW3SurfaceRenderingCL::setSurface(CW3TRDsurface *surfaceMoving, CW3TRDsurface *surfaceRef) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	m_pgSurfaceMoving = surfaceMoving;
	m_pgSurfaceRef = surfaceRef;

	m_pgVRengine->makeCurrent();
	//m_pgVRengine->getVAOSRs(m_vaoSRphoto, m_Nphoto, m_vaoSRmc, m_Nmc);

	if (m_vaoSRphoto) {
		glDeleteVertexArrays(1, &m_vaoSRphoto);
		m_vaoSRphoto = 0;
	}

	if (m_vaoSRmc) {
		glDeleteVertexArrays(1, &m_vaoSRmc);
		m_vaoSRmc = 0;
	}

	//CW3GLFunctions::initVAOSR(&m_vaoSRphoto, m_pgSurfaceMoving->getVBO());
	CW3GLFunctions::initVAOSR(&m_vaoSRphoto, m_pgSurfaceMoving->getVBOOrg());
	//CW3GLFunctions::initVAOpointOnly(&m_vaoSRphoto, m_pgSurfaceMoving->getVBOOrg());
	m_Nphoto = m_pgSurfaceMoving->getNindices();

	if (m_pgSurfaceRef) {
		//CW3GLFunctions::initVAO(&m_vaoSRmc, m_pgSurfaceRef->getVBO());
		CW3GLFunctions::initVAOpointOnly(&m_vaoSRmc, m_pgSurfaceRef->getVBO());
		m_Nmc = m_pgSurfaceRef->getNindices();
	} else {
		float vertPlaneCoord[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f
		};
		float texPlaneCoord[] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 0.0f
		};
		unsigned int	IndexPlane[] = { 0, 1, 2, 3 };

		if (m_vboSRref[0]) {
			glDeleteBuffers(3, m_vboSRref);
			m_vboSRref[0] = 0;
			m_vboSRref[1] = 0;
			m_vboSRref[2] = 0;
		}
		glGenBuffers(3, m_vboSRref);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboSRref[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), vertPlaneCoord, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_vboSRref[1]);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), texPlaneCoord, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboSRref[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned int), IndexPlane, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		CW3GLFunctions::initVAO(&m_vaoSRmc, m_vboSRref);
		m_Nmc = 4;
	}

	//Nphoto = m_sSRdata.s_Nindex;
	//Nmc = m_pgMCindices->size() * 3;

	m_pgVRengine->doneCurrent();
}

void CW3SurfaceRenderingCL::checkError(cl_int& status, int nLine, const char* strErrMsg) {
	if (status != CL_SUCCESS) {
		std::string errMsg =
			std::string("[line: ")
			+ std::to_string(nLine)
			+ std::string(" ] ")
			+ strErrMsg;
		Logger::instance()->Print(LogType::ERR, errMsg);
		m_pOCL->printError(status);
	}
}
