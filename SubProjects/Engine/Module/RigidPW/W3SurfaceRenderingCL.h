#pragma once

/*=========================================================================

File:			class CW3SurfaceRenderingCL
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-28
Last date:		2016-01-28

=========================================================================*/
#if defined(__APPLE__)
#include <OpenCL/cl.h>
#include <glm/glm.hpp>
#else
#include <CL/cl.h>
#include <gl/glm/glm.hpp>
#endif

#include <qvector3d.h>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Core/CLPlatform/WCLprogram.h"

#include "rigidpw_global.h"

class CLplatform;
class CW3VREngine;
class CW3TRDsurface;
class QOpenGLWidget;

class RIGIDPW_EXPORT CW3SurfaceRenderingCL : public WCLprogram
{
public:
	CW3SurfaceRenderingCL(CLplatform *OCL);
	~CW3SurfaceRenderingCL(void);

	// smseo : 사용되지 않아 막아둠
	//inline void setParams(GLuint prog, GLuint vaoMC, GLuint vaoPhoto, int Nmc, int Nphoto)
	//{
	//	m_PROGSR = prog;
	//	m_vaoSRmc = vaoMC;
	//	m_vaoSRphoto = vaoPhoto;
	//	m_Nmc = Nmc;
	//	m_Nphoto = m_Nphoto;
	//}

	void setVRengine(CW3VREngine *VREngine);
	QVector3D* getVolRangeGL(void);

	void rendering(cl_mem *mem, int nx, int ny, float *Sol, bool isRef);

	void clearBuffers();

	void setSurface(CW3TRDsurface *surfaceMoving, CW3TRDsurface *surfaceRef);

private:
	void ShareTexture(cl_mem *mem, cl_mem_flags flags, GLuint textureHandle);
	void setMVP(float *sol);

	void checkError(cl_int& status, int nLine, const char* strErrMsg);

private:
	CW3TRDsurface *m_pgSurfaceMoving = nullptr;
	CW3TRDsurface *m_pgSurfaceRef = nullptr;

	CW3VREngine *m_pgVRengine = nullptr;
	QOpenGLWidget *m_pgGLWidget = nullptr;

	GLuint m_vaoSRmc = 0;
	GLuint m_vaoSRphoto = 0;

	GLuint m_FBhandlerMC = 0;
	GLuint m_DepthHandlerMC = 0;
	GLuint m_TexHandlerMC = 0;

	GLuint m_FBhandlerPhoto = 0;
	GLuint m_DepthHandlerPhoto = 0;
	GLuint m_TexHandlerPhoto = 0;

	int m_Nmc = 0;
	int m_Nphoto = 0;

	glm::mat4 m_model;
	glm::mat4 m_model2;
	glm::mat4 m_view;
	glm::mat4 m_projection;
	glm::mat4 m_mvp;

	GLuint m_vboSRref[3];
};
