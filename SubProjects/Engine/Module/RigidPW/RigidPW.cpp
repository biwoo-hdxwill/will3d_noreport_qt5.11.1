/*=========================================================================

File:			class RigidPW
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last modify:	2015-12-18

=========================================================================*/
#include "RigidPW.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <float.h>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#endif

#include <QString>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Math.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Core/CLPlatform/WCLplatform.h"
#include "../../Module/VREngine/W3VREngine.h"

#include "ReductionCL.h"
#include "W3SurfaceRenderingCL.h"

using common::Logger;
using common::LogType;

namespace {
// Sign return value 왜 double 인지?
float Sign(float a, float b) { return b >= 0.0f ? a : -a; }
//template <typename T> double Sign(T a, T b) { return b >= 0 ? a : -a; }
template <typename T> T Sqr(T a) { return a * a; }
};

RigidPW::RigidPW(CLplatform* OCL, EPRO problem, int nx, int ny, int nz)
	: WCLprogram(OCL), m_problem(problem), m_nx(nx), m_ny(ny), m_nz(nz), m_FTOL(1e-5), m_ITMAX(200) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

#ifdef _DEBUG
	buildProgramFromFile("../../SubProjects/Managers/JobMgr/cl/Reorientation_POWELL.cl");
#else
	buildProgramFromFile(QString(":/cl/Reorientation_POWELL.cl"));
#endif
	m_ReductionCL = new ReductionCL(OCL);

	memset(m_feasibleRange, 0, MAX_PARAM_N * sizeof(float));
	memset(m_regi_param, 0, MAX_PARAM_N * sizeof(float));
	memset(m_directions, 0, MAX_PARAM_N*MAX_PARAM_N * sizeof(float));
	memset(m_TM, 0, MAX_PARAM_N * sizeof(float));

	switch (m_problem) {
	case EPRO::REORIENTATION:
		m_Nvariable = 3;
		m_feasibleRange[0] = M_PI / 6.0f; // rotZ
		m_feasibleRange[1] = nx / 10.0f; // tranX
		m_feasibleRange[2] = M_PI / 6.0f; // rotY
		m_nTMele = 9;
		m_nTMx = 3;
		m_nTMy = 3;
		break;
	}

	for (int i = 0; i < m_Nvariable; i++) {
		m_directions[i*m_Nvariable + i] = 1.0f;
	}

	m_centerx = m_nx / 2.0f - 0.5f;
	m_centery = m_ny / 2.0f - 0.5f;
	m_centerz = m_nz / 2.0f - 0.5f;

	m_localWorkSize[0] = 16;
	m_localWorkSize[1] = 16;

	cl_int status;
	if (!m_mem_fTM) {
		m_mem_fTM = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY,
			sizeof(cl_float)*m_nTMele,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_fDifference) {
		m_mem_fDifference = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx / 2 * m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_bIsIn) {
		m_mem_bIsIn = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx*m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}
}

RigidPW::RigidPW(CLplatform* OCL, EPRO problem)
	: WCLprogram(OCL), m_problem(problem), m_FTOL(1e-5), m_ITMAX(200) {
	m_nx = 0;
	m_ny = 0;
	m_nz = 0;

	m_localWorkSize[0] = 16;
	m_localWorkSize[1] = 16;

	m_localWorkSize1D[0] = 256;

	if (!m_pOCL->is_valid())
	{
		return;
	}

	buildProgramFromFile(QString(":/cl/Reorientation_POWELL.cl"));

	m_ReductionCL = new ReductionCL(OCL);

	if (m_problem == EPRO::POINTMATCHING)
		m_pSRCL = new CW3SurfaceRenderingCL(m_pOCL);
}

RigidPW::~RigidPW() {
	SAFE_DELETE_OBJECT(m_ReductionCL);
	SAFE_DELETE_OBJECT(m_pSRCL);

	clearResources();
}

void RigidPW::clearResources() {
	if (m_mem_refSR) {
		clReleaseMemObject(m_mem_refSR);
		m_mem_refSR = 0;
	}

	if (m_mem_movingSR) {
		clReleaseMemObject(m_mem_movingSR);
		m_mem_movingSR = 0;
	}

	if (m_mem_id) {
		clReleaseMemObject(m_mem_id);
		m_mem_id = 0;
	}

	if (m_mem_Vol) {
		clReleaseMemObject(m_mem_Vol);
		m_mem_Vol = 0;
	}
	if (m_mem_Ref) {
		clReleaseMemObject(m_mem_Ref);
		m_mem_Ref = 0;
	}
	if (m_mem_fTM) {
		clReleaseMemObject(m_mem_fTM);
		m_mem_fTM = 0;
	}
	if (m_mem_fDifference) {
		clReleaseMemObject(m_mem_fDifference);
		m_mem_fDifference = 0;
	}
	if (m_mem_bIsIn) {
		clReleaseMemObject(m_mem_bIsIn);
		m_mem_bIsIn = 0;
	}
	if (m_mem_IsInCount) {
		clReleaseMemObject(m_mem_IsInCount);
		m_mem_IsInCount = 0;
	}
	if (m_mem_id) {
		clReleaseMemObject(m_mem_id);
		m_mem_id = 0;
	}
	if (m_kernel_TransformCL) {
		clReleaseKernel(m_kernel_TransformCL);
		m_kernel_TransformCL = 0;
	}
	if (m_kernel_Transform2CL) {
		clReleaseKernel(m_kernel_Transform2CL);
		m_kernel_Transform2CL = 0;
	}
	if (m_kernel_DifferenceXReflectionCL) {
		clReleaseKernel(m_kernel_DifferenceXReflectionCL);
		m_kernel_DifferenceXReflectionCL = 0;
	}
	if (m_kernel_Difference2D) {
		clReleaseKernel(m_kernel_Difference2D);
		m_kernel_Difference2D = 0;
	}

	if (m_pSRCL)
		m_pSRCL->clearBuffers();

	SAFE_DELETE_OBJECT(m_pVolData);
	SAFE_DELETE_OBJECT(m_pVolDataMoving);
}

void RigidPW::init() {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	memset(m_feasibleRange, 0, MAX_PARAM_N * sizeof(float));
	memset(m_regi_param, 0, MAX_PARAM_N * sizeof(float));
	memset(m_directions, 0, MAX_PARAM_N*MAX_PARAM_N * sizeof(float));
	memset(m_TM, 0, MAX_PARAM_N * sizeof(float));

	switch (m_problem) {
	case EPRO::REORIENTATION:
		m_Nvariable = 3;
		m_feasibleRange[0] = M_PI / 6.0f; // rotZ
		m_feasibleRange[1] = m_nx / 10.0f; // tranX
		m_feasibleRange[2] = M_PI / 6.0f; // rotY
		m_nTMele = 9;
		m_nTMx = 3;
		m_nTMy = 3;
		break;
	}

	for (int i = 0; i < m_Nvariable; i++) {
		m_directions[i*m_Nvariable + i] = 1.0f;
	}

	m_centerx = m_nx / 2.0f - 0.5f;
	m_centery = m_ny / 2.0f - 0.5f;
	m_centerz = m_nz / 2.0f - 0.5f;

	cl_int status;

	if (!m_mem_fTM) {
		m_mem_fTM = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY,
			sizeof(cl_float)*m_nTMele,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_fDifference) {
		m_mem_fDifference = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx / 2 * m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_bIsIn) {
		m_mem_bIsIn = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx*m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_IsInCount) {
		m_mem_IsInCount = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx / 2 * m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}
}

void RigidPW::initSuperImpose(int boneIntensity) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	memset(m_feasibleRange, 0, MAX_PARAM_N * sizeof(float));
	memset(m_regi_param, 0, MAX_PARAM_N * sizeof(float));
	memset(m_directions, 0, MAX_PARAM_N*MAX_PARAM_N * sizeof(float));
	memset(m_TM, 0, MAX_PARAM_N * sizeof(float));

	m_Nvariable = 6;
	m_feasibleRange[0] = M_PI; // rotZ
	m_feasibleRange[1] = m_nxM / 4.0f; // tranX
	m_feasibleRange[2] = m_nyM / 4.0f; // tranY
	m_feasibleRange[3] = M_PI; // rotX
	m_feasibleRange[4] = m_nzM / 4.0f; // tranZ
	m_feasibleRange[5] = M_PI; // rotY
	m_nTMele = 9;
	m_nTMx = 3;
	m_nTMy = 3;

	for (int i = 0; i < m_Nvariable; i++) {
		m_directions[i*m_Nvariable + i] = 1.0f;
	}

	m_centerx = m_nx / 2.0f - 0.5f;
	m_centery = m_ny / 2.0f - 0.5f;
	m_centerz = m_nz / 2.0f - 0.5f;

	m_centerxM = m_nxM / 2.0f - 0.5f;
	m_centeryM = m_nyM / 2.0f - 0.5f;
	m_centerzM = m_nzM / 2.0f - 0.5f;

	unsigned int *boneId = nullptr;
	W3::p_allocate_1D(&boneId, m_nx*m_ny*m_nz);

	m_Nid = 0;
	unsigned int idxBone = 0;
	for (int i = 0; i < m_nz; ++i) {
		for (int j = 0; j < m_ny; ++j) {
			for (int k = 0; k < m_nx; ++k) {
				idxBone = i * m_nx*m_ny + j * m_nx + k;
				if (m_pVolData[idxBone] > boneIntensity) {
					boneId[m_Nid++] = idxBone;
				}
			}
		}
	}

	cl_int status;

	if (!m_mem_id) {
		m_mem_id = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			sizeof(cl_uint)*m_Nid,
			boneId,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	SAFE_DELETE_ARRAY(boneId);

	if (!m_mem_fTM) {
		m_mem_fTM = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY,
			sizeof(cl_float)*m_nTMele,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_fDifference) {
		m_mem_fDifference = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_Nid,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_IsInCount) {
		m_mem_IsInCount = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_Nid,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}
}

void RigidPW::setVRengine(CW3VREngine* VRengine) {
	if (m_problem == EPRO::POINTMATCHING)
		m_pSRCL->setVRengine(VRengine);
}

void RigidPW::setSurface(CW3TRDsurface *surfaceMoving, CW3TRDsurface *surfaceRef) {
	if (m_problem == EPRO::POINTMATCHING)
		m_pSRCL->setSurface(surfaceMoving, surfaceRef);
}

void RigidPW::initRegiPoints() {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	m_nx = 70;
	m_ny = 70;

	memset(m_feasibleRange, 0, MAX_PARAM_N * sizeof(float));
	memset(m_regi_param, 0, MAX_PARAM_N * sizeof(float));
	memset(m_directions, 0, MAX_PARAM_N*MAX_PARAM_N * sizeof(float));
	memset(m_TM, 0, MAX_PARAM_N * sizeof(float));

	QVector3D *VolRange = m_pSRCL->getVolRangeGL();

	m_Nvariable = 6;
#if 0
	m_feasibleRange[0] = VolRange->z(); // tranZ
	m_feasibleRange[1] = VolRange->x(); // tranX
	m_feasibleRange[2] = VolRange->y(); // tranY
	m_feasibleRange[3] = M_PI / 6.0f; // rotZ
	m_feasibleRange[4] = M_PI / 6.0f; // rotX
	m_feasibleRange[5] = M_PI / 6.0f; // rotY
#else
	m_feasibleRange[0] = VolRange->z(); // tranZ
	m_feasibleRange[1] = VolRange->y(); // tranY
	m_feasibleRange[2] = VolRange->x(); // tranX
	m_feasibleRange[3] = M_PI / 6.0f; // rotX
	m_feasibleRange[4] = M_PI / 6.0f; // rotY
	m_feasibleRange[5] = M_PI / 6.0f; // rotZ
#endif
	m_nTMele = 9;
	m_nTMx = 3;
	m_nTMy = 3;

	for (int i = 0; i < m_Nvariable; i++) {
		m_directions[i*m_Nvariable + i] = 1.0f;
	}

	cl_int status;
	if (!m_kernel_Difference2D) {
		m_kernel_Difference2D = clCreateKernel(m_program, "Difference2DCL", &status);
		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_kernel_test2D) {
		m_kernel_test2D = clCreateKernel(m_program, "Test2DCL", &status);
		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_fDifference) {
		m_mem_fDifference = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx*m_ny,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_IsInCount) {
		m_mem_IsInCount = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx*m_ny,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}
}

bool RigidPW::setVolume(CW3Image3D *ref, CW3Image3D *moving, int boneIntensity, int downFactor) {
	if (!m_pOCL->is_valid())
	{
		return false;
	}

	m_nx = ref->width();
	m_ny = ref->height();
	m_nz = ref->depth();

	m_nxM = moving->width();
	m_nyM = moving->height();
	m_nzM = moving->depth();

	float pixelSpacingR = ref->pixelSpacing();
	float sliceSpacingR = ref->sliceSpacing();

	float pixelSpacingM = moving->pixelSpacing();
	float sliceSpacingM = moving->sliceSpacing();

	m_downFactor = downFactor;

#if 1
	float scaleM = pixelSpacingM / pixelSpacingR;
	common::Logger::instance()->PrintDebugMode("RigidPW::setVolume", "scale : " + std::to_string(scaleM));

	if (scaleM != 1.0f) {
		// Bilinear image scaling
		int newNxM = m_nxM * scaleM;
		int newNyM = m_nyM * scaleM;
		int newNzM = m_nzM * scaleM;

		common::Logger::instance()->PrintDebugMode("RigidPW::setVolume",
												   "scale : " + std::to_string(scaleM) +
												   ", w : " + std::to_string(newNxM) +
												   ", h : " + std::to_string(newNyM) +
												   ", d : " + std::to_string(newNzM));

		unsigned short **tempDataMoving = SAFE_ALLOC_VOLUME(unsigned short, newNzM, newNxM * newNyM);

		for (int i = 0; i < newNzM; i++)
			memset(*tempDataMoving, 0, sizeof(unsigned short) * newNxM * newNyM);

		unsigned short A, B, C, D, E, F, G, H, gray;
		int x, y, z, index;
		float x_ratio = (float)(m_nxM - 1) / newNxM;
		float y_ratio = (float)(m_nyM - 1) / newNyM;
		float z_ratio = (float)(m_nzM - 1) / newNzM;
		float x_diff, y_diff, z_diff;
		int offset = 0;

		for (int i = 0; i < newNzM; i++) {
			//int srcZ = i / scaleM;
			for (int j = 0; j < newNyM; j++) {
				//int srcY = j / scaleM;
				for (int k = 0; k < newNxM; k++) {
					//int srcX = k / scaleM;
					//tempDataMoving[i][(j * newNxM) + k] = moving->getData()[srcZ][(srcY * m_nxM) + srcX];

					x = (int)(x_ratio * k);
					y = (int)(y_ratio * j);
					z = (int)(z_ratio * i);

					x_diff = (x_ratio * k) - x;
					y_diff = (y_ratio * j) - y;
					z_diff = (z_ratio * i) - z;

					index = y * m_nxM + x;

					A = moving->getData()[z][index];
					B = moving->getData()[z][index + 1];
					C = moving->getData()[z][index + m_nxM];
					D = moving->getData()[z][index + m_nxM + 1];
					E = moving->getData()[z + 1][index];
					F = moving->getData()[z + 1][index + 1];
					G = moving->getData()[z + 1][index + m_nxM];
					H = moving->getData()[z + 1][index + m_nxM + 1];

					gray = (unsigned short)(
						A * (1 - x_diff) * (1 - y_diff) * (1 - z_diff) +
						B * (x_diff)* (1 - y_diff) * (1 - z_diff) +
						C * (1 - x_diff) * (y_diff)* (1 - z_diff) +
						D * (x_diff)* (y_diff)* (1 - z_diff) +
						E * (1 - x_diff) * (1 - y_diff) * (z_diff)+
						F * (x_diff)* (1 - y_diff) * (z_diff)+
						G * (1 - x_diff) * (y_diff)* (z_diff)+
						H * (x_diff)* (y_diff)* (z_diff)
						);

					tempDataMoving[i][offset++] = gray;
				}
			}

			offset = 0;

#if 0
			FILE *FWRITE1;
			char name1[MAX_PATH];
			sprintf_s(name1, "D:\\tempraw\\new_%04d.raw", i);
			fopen_s(&FWRITE1, name1, "wb");
			fwrite(tempDataMoving[i], sizeof(unsigned short), newNxM * newNyM, FWRITE1);
			fclose(FWRITE1);

			FILE *FWRITE2;
			char name2[MAX_PATH];
			sprintf_s(name2, "D:\\tempraw\\org_%04d.raw", i);
			fopen_s(&FWRITE2, name2, "wb");
			fwrite(moving->getData()[srcZ], sizeof(unsigned short), moving->width() * moving->height(), FWRITE2);
			fclose(FWRITE2);
#endif
		}

		m_nxM = newNxM;
		m_nyM = newNyM;
		m_nzM = newNzM;

		W3::volumeDown(&m_pVolData, ref->getData(), m_downFactor, m_nx, m_ny, m_nz);
		W3::volumeDown(&m_pVolDataMoving, tempDataMoving, m_downFactor, m_nxM, m_nyM, m_nzM);

		SAFE_DELETE_VOLUME(tempDataMoving, newNzM);
	} else {
		W3::volumeDown(&m_pVolData, ref->getData(), m_downFactor, m_nx, m_ny, m_nz);
		W3::volumeDown(&m_pVolDataMoving, moving->getData(), m_downFactor, m_nxM, m_nyM, m_nzM);
	}
#else
	W3::volumeDown(&m_pVolData, ref->getData(), m_downFactor, m_nx, m_ny, m_nz);
	W3::volumeDown(&m_pVolDataMoving, moving->getData(), m_downFactor, m_nxM, m_nyM, m_nzM);
#endif
	initSuperImpose(boneIntensity);

	//for (int i = 0; i < m_nx*m_ny*m_nz; i++)
	//{
	//	if (m_pVolData[i] > boneIntensity)
	//	{
	//		m_pVolData[i] = 1.0f;
	//	}
	//	else
	//	{
	//		m_pVolData[i] = 0.0f;
	//	}
	//	if (m_pVolDataMoving[i] > boneIntensity)
	//	{
	//		m_pVolDataMoving[i] = 1.0f;
	//	}
	//	else
	//	{
	//		m_pVolDataMoving[i] = 0.0f;
	//	}
	//}

	m_image_format.image_channel_order = CL_A;
	m_image_format.image_channel_data_type = CL_SIGNED_INT16;

	cl_int status;
	if (!m_mem_Vol) {
		m_mem_Vol = clCreateImage3D(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			&m_image_format,
			m_nxM,
			m_nyM,
			m_nzM,
			0,
			0,
			m_pVolDataMoving,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_Ref) {
		m_mem_Ref = clCreateImage3D(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
			&m_image_format,
			m_nx,
			m_ny,
			m_nz,
			0,
			0,
			m_pVolData,
			&status);

		checkError(status, __LINE__, "clCreateImage3D failed.");
	}

	if (!m_kernel_TransformCL) {
		m_kernel_TransformCL = clCreateKernel(m_program, "sTransformedDifferenceCL", &status);
		checkError(status, __LINE__, "clCreateKernel failed.");
	}

	SAFE_DELETE_ARRAY(m_pVolData);
	SAFE_DELETE_ARRAY(m_pVolDataMoving);

	return true;
}

void RigidPW::transformVolume(CW3Image3D *vol, CW3Image3D *moving, float *param) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	m_nx = vol->width();
	m_ny = vol->height();
	m_nz = vol->depth();

	m_downFactor = 1;

	W3::volumeDown(&m_pVolData, vol->getData(), m_downFactor, m_nx, m_ny, m_nz);

	memset(m_feasibleRange, 0, MAX_PARAM_N * sizeof(float));
	memset(m_regi_param, 0, MAX_PARAM_N * sizeof(float));
	memset(m_directions, 0, MAX_PARAM_N*MAX_PARAM_N * sizeof(float));
	memset(m_TM, 0, MAX_PARAM_N * sizeof(float));

	m_centerx = m_nx / 2.0f - 0.5f;
	m_centery = m_ny / 2.0f - 0.5f;
	m_centerz = m_nz / 2.0f - 0.5f;

	m_nTMele = 9;

	cl_int status;

	if (!m_mem_fTM) {
		m_mem_fTM = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_ONLY,
			sizeof(cl_float)*m_nTMele,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_mem_IsInCount) {
		m_mem_IsInCount = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(cl_float)*m_nx*m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	m_image_format.image_channel_order = CL_A;
	m_image_format.image_channel_data_type = CL_SIGNED_INT16;

	m_mem_Vol = clCreateImage3D(
		m_pOCL->getContext(),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&m_image_format,
		m_nx,
		m_ny,
		m_nz,
		0,
		0,
		m_pVolData,
		&status);

	checkError(status, __LINE__, "clCreateImage3D failed.");

	if (!m_mem_Ref) {
		m_mem_Ref = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(short)*m_nx*m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_kernel_TransformCL) {
		m_kernel_TransformCL = clCreateKernel(m_program, "sTransform3DCL", &status);
		checkError(status, __LINE__, "clCreateKernel failed.");
	}

	setTM(param);

	//cl_int status;

	status = clEnqueueWriteBuffer(
		m_pOCL->getCommandQueue(),
		m_mem_fTM,
		CL_TRUE,
		0,
		sizeof(cl_float)*m_nTMele,
		m_TM,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueWriteBuffer failed.");

	// set input parameter
	status = clSetKernelArg(m_kernel_TransformCL, 0, sizeof(cl_mem), &m_mem_Ref);
	status |= clSetKernelArg(m_kernel_TransformCL, 1, sizeof(cl_mem), &m_mem_IsInCount);
	status |= clSetKernelArg(m_kernel_TransformCL, 2, sizeof(cl_mem), &m_mem_fTM);
	status |= clSetKernelArg(m_kernel_TransformCL, 3, sizeof(cl_float), &param[1]);
	status |= clSetKernelArg(m_kernel_TransformCL, 4, sizeof(cl_float), &param[2]);
	status |= clSetKernelArg(m_kernel_TransformCL, 5, sizeof(cl_float), &param[4]);
	status |= clSetKernelArg(m_kernel_TransformCL, 6, sizeof(cl_uint), &m_nx);
	status |= clSetKernelArg(m_kernel_TransformCL, 7, sizeof(cl_uint), &m_ny);
	status |= clSetKernelArg(m_kernel_TransformCL, 8, sizeof(cl_uint), &m_nz);
	status |= clSetKernelArg(m_kernel_TransformCL, 9, sizeof(cl_float), &m_centerx);
	status |= clSetKernelArg(m_kernel_TransformCL, 10, sizeof(cl_float), &m_centery);
	status |= clSetKernelArg(m_kernel_TransformCL, 11, sizeof(cl_float), &m_centerz);
	status |= clSetKernelArg(m_kernel_TransformCL, 12, sizeof(cl_mem), &m_mem_Vol);
	status |= clSetKernelArg(m_kernel_TransformCL, 13, sizeof(cl_float)*m_nTMele, nullptr);
	status |= clSetKernelArg(m_kernel_TransformCL, 14, sizeof(cl_uint), &m_nTMele);
	checkError(status, __LINE__, "clSetKernelArg failed.");

	int sizex = (m_nx + m_localWorkSize[0] - 1) / m_localWorkSize[0];
	int sizey = (m_ny*m_nz + m_localWorkSize[1] - 1) / m_localWorkSize[1];

	size_t globalWorkSize[2] = { sizex*m_localWorkSize[0], sizey*m_localWorkSize[1] };

	status = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_TransformCL,
		2,
		nullptr,
		globalWorkSize,
		m_localWorkSize,
		0, nullptr, nullptr);
	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), m_mem_Ref, CL_TRUE,
								 0, m_nx*m_ny*m_nz * sizeof(short), m_pVolData,
								 0, NULL, NULL);
	if (status != CL_SUCCESS) {
		m_pOCL->printError(status);
	}

	for (int i = 0; i < m_nz; i++) {
		for (int j = 0; j < m_nx*m_ny; j++) {
			moving->getData()[i][j] = m_pVolData[i*m_nx*m_ny + j];
		}
	}

	SAFE_DELETE_ARRAY(m_pVolData);
}

bool RigidPW::setVolume(CW3Image3D *vol) {
	if (!m_pOCL->is_valid())
	{
		return false;
	}

	m_nx = vol->width();
	m_ny = vol->height();
	m_nz = vol->depth();

	m_downFactor = 4;

	W3::volumeDown(&m_pVolData, vol->getData(), m_downFactor, m_nx, m_ny, m_nz);

	init();

	//SAFE_DELETE_ARRAY(m_pVolData);
	//W3::p_allocate_1D(&m_pVolData, m_nx*m_ny*m_nz);
	//for(int k = 0; k < m_nz; k++)
	//{
	//	for(int i = 0; i < m_nx*m_ny; i++)
	//	{
	//		m_pVolData[k*m_nx*m_ny + i] = vol->getData()[k][i];
	//	}
	//}
	//WriteRawData(m_pVolData, "TestVol", m_nx, m_ny, m_nz, 0, 1);

	m_image_format.image_channel_order = CL_A;
	m_image_format.image_channel_data_type = CL_SIGNED_INT16;

	cl_int status;
	m_mem_Vol = clCreateImage3D(
		m_pOCL->getContext(),
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		&m_image_format,
		m_nx,
		m_ny,
		m_nz,
		0,
		0,
		m_pVolData,
		&status);

	checkError(status, __LINE__, "clCreateImage3D failed.");

	if (!m_mem_Ref) {
		m_mem_Ref = clCreateBuffer(
			m_pOCL->getContext(),
			CL_MEM_READ_WRITE,
			sizeof(short)*m_nx*m_ny*m_nz,
			nullptr,
			&status);

		checkError(status, __LINE__, "clCreateBuffer failed.");
	}

	if (!m_kernel_TransformCL) {
		m_kernel_TransformCL = clCreateKernel(m_program, "sTransformCL", &status);
		checkError(status, __LINE__, "clCreateKernel failed.");
	}

	if (!m_kernel_DifferenceXReflectionCL) {
		m_kernel_DifferenceXReflectionCL = clCreateKernel(m_program, "sDifferenceXReflectionCL", &status);
		checkError(status, __LINE__, "clCreateKernel failed.");
	}

	SAFE_DELETE_ARRAY(m_pVolData);

	return true;
}

void RigidPW::setTM(float *param) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	switch (m_problem) {
	case EPRO::REORIENTATION:
		// param: rotZ, tranX, rotY
		// cos(rotY), 0, -sin(rotY)          cos(rotZ) , sin(rotZ), 0
		// 0,         1, 0				*    -sin(rotZ), cos(rotZ), 0
		// sin(rotY), 0, cos(rotY)				0	   , 0		  , 1
		// same as
		// glm::mat4 test = glm::rotate(param[2], glm::vec3(0.0f, -1.0f, 0.0f))*glm::rotate(param[0], glm::vec3(0.0f, 0.0f, -1.0f));
		// vertex 를 움직일 때는 위의 역변환 필요! (왼손 좌표계-> 오른손 좌표계 변환도 고려해야 함)
		m_TM[0] = cos(param[0])*cos(param[2]);
		m_TM[1] = sin(param[0])*cos(param[2]);
		m_TM[2] = -sin(param[2]);
		m_TM[3] = -sin(param[0]);
		m_TM[4] = cos(param[0]);
		m_TM[5] = 0.0f;
		m_TM[6] = cos(param[0])*sin(param[2]);
		m_TM[7] = sin(param[0])*sin(param[2]);
		m_TM[8] = cos(param[2]);
		break;

	case EPRO::POINTMATCHING:
		break;

	case EPRO::SUPERIMPOSE:
		// param: rotZ, tranX, tranY, rotX, tranZ, rotY
		//	1,  0,			0				cos(rotY), 0, -sin(rotY)          cos(rotZ) , sin(rotZ), 0
		//	0,  cos(rotX), sin(rotX)   *	0,         1, 0				*    -sin(rotZ), cos(rotZ), 0
		//	0, -sin(rotX), cos(rotX)		sin(rotY), 0, cos(rotY)				0	   , 0		  , 1
		// same as
		// glm::mat4 test = glm::rotate(param[3], glm::vec3(-1.0f, 0.0f, 0.0f))*glm::rotate(param[5], glm::vec3(0.0f, -1.0f, 0.0f))*glm::rotate(param[0], glm::vec3(0.0f, 0.0f, -1.0f));
		// vertex 를 움직일 때는 위의 역변환 필요! (왼손 좌표계-> 오른손 좌표계 변환도 고려해야 함)
		m_TM[0] = cos(param[0])*cos(param[5]);
		m_TM[1] = sin(param[0])*cos(param[5]);
		m_TM[2] = -sin(param[5]);
		m_TM[3] = -cos(param[3])*sin(param[0]) + sin(param[3])*cos(param[0])*sin(param[5]);
		m_TM[4] = cos(param[3])*cos(param[0]) + sin(param[3])*sin(param[0])*sin(param[5]);
		m_TM[5] = sin(param[3])*cos(param[5]);
		m_TM[6] = sin(param[3])*sin(param[0]) + cos(param[3])*cos(param[0])*sin(param[5]);
		m_TM[7] = -sin(param[3])*cos(param[0]) + cos(param[3])*sin(param[0])*sin(param[5]);
		m_TM[8] = cos(param[3])*cos(param[5]);

		//m_TM[0] = cos(param[0])*cos(param[4]);
		//m_TM[1] = sin(param[0])*cos(param[4]);
		//m_TM[2] = -sin(param[4]);
		////TM[3] = param[1];
		//m_TM[3] = -sin(param[0])*cos(param[3]) + cos(param[0])*sin(param[4])*sin(param[3]);
		//m_TM[4] = cos(param[0])*cos(param[3]) + sin(param[0])*sin(param[4])*sin(param[3]);
		//m_TM[5] = cos(param[4])*sin(param[3]);
		////m_TM[7] = param[2];
		//m_TM[6] = sin(param[0])*sin(param[3]) + cos(param[0])*sin(param[4])*cos(param[3]);
		//m_TM[8] = -cos(param[0])*sin(param[3]) + sin(param[0])*sin(param[4])*cos(param[3]);
		//m_TM[8] = cos(param[4])*cos(param[3]);
		//m_TM[11] = param[5];
		break;
	}
}

void RigidPW::TransformErrorSR(float *Sol, float &FRET, bool isFinal) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	m_pSRCL->rendering(&m_mem_movingSR, m_nx, m_ny, Sol, false);

	int sizex = (m_nx + m_localWorkSize[0] - 1) / m_localWorkSize[0];
	int sizey = (m_ny + m_localWorkSize[1] - 1) / m_localWorkSize[1];

	size_t globalWorkSize[2] = { sizex*m_localWorkSize[0], sizey*m_localWorkSize[1] };

	cl_int status;

	Difference2D(globalWorkSize, m_localWorkSize);

	int N = m_nx * m_ny;

	float sumValue = m_ReductionCL->runReduction(&m_mem_fDifference, N);
	float inCount = m_ReductionCL->runReduction(&m_mem_IsInCount, N);

	if (inCount < (float)N * 0.1f) {
		printf("inCount(%f) < %f\r\n", inCount, (float)N * 0.1f);
		FRET = FLT_MAX;
	} else if (inCount == 0.0f) {
		FRET = FLT_MAX;

		Test2D(globalWorkSize, m_localWorkSize);

		float *data = nullptr;

		W3::p_allocate_1D(&data, m_nx*m_ny);
		status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), m_mem_fDifference, CL_TRUE,
									 0, m_nx*m_ny * sizeof(float), data,
									 0, NULL, NULL);
		if (status != CL_SUCCESS) {
			m_pOCL->printError(status);
		}

#if 0
		FILE *FWRITE = nullptr;
		fopen_s(&FWRITE, "Moving7070false.raw", "wb");

		if (FWRITE) {
			fwrite(data, sizeof(float), m_nx*m_ny, FWRITE);
			fclose(FWRITE);
		}
#endif

		SAFE_DELETE_OBJECT(data);
	} else {
		FRET = sumValue / inCount;
		//printf("Error: %f\n", FRET);
	}

	if (isFinal) {
		//float *data = nullptr;
		//W3::p_allocate_1D(&data, m_nx*m_ny);
		//status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), m_mem_fDifference, CL_TRUE,
		//                               0, m_nx*m_ny*sizeof(float), data,
		//                               0, NULL, NULL);
		//if(status != CL_SUCCESS)
		//{
		//	m_pOCL->printError(status);
		//}
		//FILE *FWRITE;
		//fopen_s(&FWRITE, "Diff7070final.raw", "wb");
		//fwrite(data, sizeof(float), m_nx*m_ny, FWRITE);
		//fclose(FWRITE);
		//SAFE_DELETE_OBJECT(data);

		Test2D(globalWorkSize, m_localWorkSize);

		float *data = nullptr;

		W3::p_allocate_1D(&data, m_nx*m_ny);
		status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), m_mem_fDifference, CL_TRUE,
									 0, m_nx*m_ny * sizeof(float), data,
									 0, NULL, NULL);
		if (status != CL_SUCCESS) {
			m_pOCL->printError(status);
		}

#if 0
		FILE *FWRITE = nullptr;
		fopen_s(&FWRITE, "Moving7070final.raw", "wb");

		if (FWRITE) {
			fwrite(data, sizeof(float), m_nx*m_ny, FWRITE);
			fclose(FWRITE);
		}
#endif

		SAFE_DELETE_OBJECT(data);
	}
}

void RigidPW::Difference2D(size_t *global, size_t *local) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	cl_int err = 0;
	err |= clSetKernelArg(m_kernel_Difference2D, 0, sizeof(cl_mem), &m_mem_fDifference);
	err |= clSetKernelArg(m_kernel_Difference2D, 1, sizeof(cl_mem), &m_mem_IsInCount);
	err |= clSetKernelArg(m_kernel_Difference2D, 2, sizeof(cl_mem), &m_mem_refSR);
	err |= clSetKernelArg(m_kernel_Difference2D, 3, sizeof(cl_mem), &m_mem_movingSR);
	err |= clSetKernelArg(m_kernel_Difference2D, 4, sizeof(cl_uint), &m_nx);
	err |= clSetKernelArg(m_kernel_Difference2D, 5, sizeof(cl_uint), &m_ny);
	checkError(err, __LINE__, "set arg in Difference2D failed.");

	err = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_Difference2D,
		2,
		nullptr,
		global,
		local,
		0, nullptr, nullptr);

	checkError(err, __LINE__, "update Difference2D failed.");
}

void RigidPW::Test2D(size_t *global, size_t *local) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	cl_int err = 0;
	err |= clSetKernelArg(m_kernel_test2D, 0, sizeof(cl_mem), &m_mem_fDifference);
	err |= clSetKernelArg(m_kernel_test2D, 1, sizeof(cl_mem), &m_mem_IsInCount);
	err |= clSetKernelArg(m_kernel_test2D, 2, sizeof(cl_mem), &m_mem_refSR);
	err |= clSetKernelArg(m_kernel_test2D, 3, sizeof(cl_mem), &m_mem_movingSR);
	err |= clSetKernelArg(m_kernel_test2D, 4, sizeof(cl_uint), &m_nx);
	err |= clSetKernelArg(m_kernel_test2D, 5, sizeof(cl_uint), &m_ny);
	checkError(err, __LINE__, "set arg in m_kernel_test2D failed.");

	err = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_test2D,
		2,
		nullptr,
		global,
		local,
		0, nullptr, nullptr);

	checkError(err, __LINE__, "update m_kernel_test2D failed.");
}

void RigidPW::TransformError(float *Sol, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	setTM(Sol);

	cl_int status;

	status = clEnqueueWriteBuffer(
		m_pOCL->getCommandQueue(),
		m_mem_fTM,
		CL_TRUE,
		0,
		sizeof(cl_float)*m_nTMele,
		m_TM,
		0, nullptr, nullptr);
	checkError(status, __LINE__, "clEnqueueWriteBuffer failed.");

	// set input parameter
	status = clSetKernelArg(m_kernel_TransformCL, 0, sizeof(cl_mem), &m_mem_Ref);
	status |= clSetKernelArg(m_kernel_TransformCL, 1, sizeof(cl_mem), &m_mem_bIsIn);
	status |= clSetKernelArg(m_kernel_TransformCL, 2, sizeof(cl_mem), &m_mem_fTM);
	status |= clSetKernelArg(m_kernel_TransformCL, 3, sizeof(cl_float), &Sol[1]);
	status |= clSetKernelArg(m_kernel_TransformCL, 4, sizeof(cl_uint), &m_nx);
	status |= clSetKernelArg(m_kernel_TransformCL, 5, sizeof(cl_uint), &m_ny);
	status |= clSetKernelArg(m_kernel_TransformCL, 6, sizeof(cl_uint), &m_nz);
	status |= clSetKernelArg(m_kernel_TransformCL, 7, sizeof(cl_float), &m_centerx);
	status |= clSetKernelArg(m_kernel_TransformCL, 8, sizeof(cl_float), &m_centery);
	status |= clSetKernelArg(m_kernel_TransformCL, 9, sizeof(cl_float), &m_centerz);
	status |= clSetKernelArg(m_kernel_TransformCL, 10, sizeof(cl_mem), &m_mem_Vol);
	status |= clSetKernelArg(m_kernel_TransformCL, 11, sizeof(cl_float)*m_nTMele, nullptr);
	status |= clSetKernelArg(m_kernel_TransformCL, 12, sizeof(cl_uint), &m_nTMele);
	checkError(status, __LINE__, "clSetKernelArg failed.");

	int sizex = (m_nx + m_localWorkSize[0] - 1) / m_localWorkSize[0];
	int sizey = (m_ny*m_nz + m_localWorkSize[1] - 1) / m_localWorkSize[1];

	size_t globalWorkSize[2] = { sizex*m_localWorkSize[0], sizey*m_localWorkSize[1] };

	status = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_TransformCL,
		2,
		nullptr,
		globalWorkSize,
		m_localWorkSize,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	cl_uint nxhalf = m_nx / 2;

	status = clSetKernelArg(m_kernel_DifferenceXReflectionCL, 0, sizeof(cl_mem), &m_mem_fDifference);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 1, sizeof(cl_mem), &m_mem_IsInCount);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 2, sizeof(cl_mem), &m_mem_Ref);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 3, sizeof(cl_mem), &m_mem_bIsIn);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 4, sizeof(cl_uint), &nxhalf);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 5, sizeof(cl_uint), &m_nx);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 6, sizeof(cl_uint), &m_ny);
	status |= clSetKernelArg(m_kernel_DifferenceXReflectionCL, 7, sizeof(cl_uint), &m_nz);

	checkError(status, __LINE__, "clSetKernelArg failed.");

	sizex = (nxhalf + m_localWorkSize[0] - 1) / m_localWorkSize[0];
	globalWorkSize[0] = sizex * m_localWorkSize[0];

	status = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_DifferenceXReflectionCL,
		2,
		nullptr,
		globalWorkSize,
		m_localWorkSize,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	int N = nxhalf * m_ny*m_nz;
	float sumValue = m_ReductionCL->runReduction(&m_mem_fDifference, N);
	float inCount = m_ReductionCL->runReduction(&m_mem_IsInCount, N);

	FRET = sumValue / inCount;
}

void RigidPW::TransformErrorSUPERIMPOSE(float *Sol, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	setTM(Sol);

	cl_int status;

	status = clEnqueueWriteBuffer(
		m_pOCL->getCommandQueue(),
		m_mem_fTM,
		CL_TRUE,
		0,
		sizeof(cl_float)*m_nTMele,
		m_TM,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueWriteBuffer failed.");

	// set input parameter
	status = clSetKernelArg(m_kernel_TransformCL, 0, sizeof(cl_mem), &m_mem_fDifference);
	status |= clSetKernelArg(m_kernel_TransformCL, 1, sizeof(cl_mem), &m_mem_id);
	status |= clSetKernelArg(m_kernel_TransformCL, 2, sizeof(cl_mem), &m_mem_IsInCount);
	status |= clSetKernelArg(m_kernel_TransformCL, 3, sizeof(cl_mem), &m_mem_fTM);
	status |= clSetKernelArg(m_kernel_TransformCL, 4, sizeof(cl_float), &Sol[1]);
	status |= clSetKernelArg(m_kernel_TransformCL, 5, sizeof(cl_float), &Sol[2]);
	status |= clSetKernelArg(m_kernel_TransformCL, 6, sizeof(cl_float), &Sol[4]);
	status |= clSetKernelArg(m_kernel_TransformCL, 7, sizeof(cl_uint), &m_nxM);
	status |= clSetKernelArg(m_kernel_TransformCL, 8, sizeof(cl_uint), &m_nyM);
	status |= clSetKernelArg(m_kernel_TransformCL, 9, sizeof(cl_uint), &m_nzM);
	status |= clSetKernelArg(m_kernel_TransformCL, 10, sizeof(cl_uint), &m_Nid);
	status |= clSetKernelArg(m_kernel_TransformCL, 11, sizeof(cl_float), &m_centerx);
	status |= clSetKernelArg(m_kernel_TransformCL, 12, sizeof(cl_float), &m_centery);
	status |= clSetKernelArg(m_kernel_TransformCL, 13, sizeof(cl_float), &m_centerz);
	status |= clSetKernelArg(m_kernel_TransformCL, 14, sizeof(cl_float), &m_centerxM);
	status |= clSetKernelArg(m_kernel_TransformCL, 15, sizeof(cl_float), &m_centeryM);
	status |= clSetKernelArg(m_kernel_TransformCL, 16, sizeof(cl_float), &m_centerzM);
	status |= clSetKernelArg(m_kernel_TransformCL, 17, sizeof(cl_mem), &m_mem_Ref);
	status |= clSetKernelArg(m_kernel_TransformCL, 18, sizeof(cl_mem), &m_mem_Vol);
	status |= clSetKernelArg(m_kernel_TransformCL, 19, sizeof(cl_float)*m_nTMele, nullptr);
	status |= clSetKernelArg(m_kernel_TransformCL, 20, sizeof(cl_uint), &m_nTMele);

	checkError(status, __LINE__, "clSetKernelArg failed.");

	int sizex = (m_Nid + m_localWorkSize1D[0] - 1) / m_localWorkSize1D[0];

	size_t globalWorkSize[1] = { sizex*m_localWorkSize1D[0] };

	status = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_TransformCL,
		1,
		nullptr,
		globalWorkSize,
		m_localWorkSize1D,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	float sumValue = m_ReductionCL->runReduction(&m_mem_fDifference, m_Nid);
	float inCount = m_ReductionCL->runReduction(&m_mem_IsInCount, m_Nid);

	if (inCount > m_Nid*0.7f)
		FRET = sumValue / inCount;
	else
		FRET = FLT_MAX;
}

void RigidPW::Transform(float *Sol) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	setTM(Sol);

	cl_int status;

	status = clEnqueueWriteBuffer(
		m_pOCL->getCommandQueue(),
		m_mem_fTM,
		CL_TRUE,
		0,
		sizeof(cl_float)*m_nTMele,
		m_TM,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueWriteBuffer failed.");

	// set input parameter
	status = clSetKernelArg(m_kernel_Transform2CL, 0, sizeof(cl_mem), &m_mem_Ref);
	status |= clSetKernelArg(m_kernel_Transform2CL, 1, sizeof(cl_mem), &m_mem_fTM);
	status |= clSetKernelArg(m_kernel_Transform2CL, 2, sizeof(cl_float), &Sol[1]);
	status |= clSetKernelArg(m_kernel_Transform2CL, 3, sizeof(cl_uint), &m_nx);
	status |= clSetKernelArg(m_kernel_Transform2CL, 4, sizeof(cl_uint), &m_ny);
	status |= clSetKernelArg(m_kernel_Transform2CL, 5, sizeof(cl_uint), &m_nz);
	status |= clSetKernelArg(m_kernel_Transform2CL, 6, sizeof(cl_float), &m_centerx);
	status |= clSetKernelArg(m_kernel_Transform2CL, 7, sizeof(cl_float), &m_centery);
	status |= clSetKernelArg(m_kernel_Transform2CL, 8, sizeof(cl_float), &m_centerz);
	status |= clSetKernelArg(m_kernel_Transform2CL, 9, sizeof(cl_mem), &m_mem_Vol);
	status |= clSetKernelArg(m_kernel_Transform2CL, 10, sizeof(cl_float)*m_nTMele, nullptr);
	status |= clSetKernelArg(m_kernel_Transform2CL, 11, sizeof(cl_uint), &m_nTMele);

	checkError(status, __LINE__, "clSetKernelArg failed.");

	int sizex = (m_nx + m_localWorkSize[0] - 1) / m_localWorkSize[0];
	int sizey = (m_ny*m_nz + m_localWorkSize[1] - 1) / m_localWorkSize[1];

	size_t globalWorkSize[2] = { sizex*m_localWorkSize[0], sizey*m_localWorkSize[1] };

	status = clEnqueueNDRangeKernel(
		m_pOCL->getCommandQueue(),
		m_kernel_Transform2CL,
		2,
		nullptr,
		globalWorkSize,
		m_localWorkSize,
		0, nullptr, nullptr);

	checkError(status, __LINE__, "clEnqueueNDRangeKernel failed.");

	unsigned short *test = new unsigned short[m_nx*m_ny*m_nz];
	status = clEnqueueReadBuffer(m_pOCL->getCommandQueue(), m_mem_Ref, CL_TRUE,
								 0, m_nx*m_ny*m_nz * sizeof(unsigned short), test,
								 0, NULL, NULL);

	checkError(status, __LINE__, "clEnqueueReadBuffer failed.");

	//myViewer->imshow(test, m_nx, m_ny, m_nz);

	delete[] test;
}

bool RigidPW::LINMIN(float *direction, float *tmpSol, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return false;
	}

	float AX, BX, XX, FA, FB, FX, TOL, XMIN;

	TOL = 1e-5;

	AX = 0.0f;
	XX = 1.0f;
	BX = 2.0f;

	// ???
	//if(direction[3] > 0.5f)
	//{
	//	int a = 0;
	//}

	MNBRAK(direction, tmpSol, AX, XX, BX, FA, FX, FB);

	float isSucceed = BRENT(direction, tmpSol, AX, XX, BX, FX, TOL, XMIN, FRET);

	int j;
	for (j = 0; j < m_Nvariable; j++) {
		m_regi_param[j] += direction[j] * XMIN;
	}

	return isSucceed;
}

void RigidPW::MNBRAK(float *direction, float *tmpSol, float &AX, float &BX, float &CX, float &FA, float &FB, float &FC) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	const float GOLD = 1.618034f, GLIMIT = 100.0f, TINY = 1e-5f;
	float DUM, FU, Q, R, U, ULIM;

	F1DIM(direction, tmpSol, AX, FA);
	F1DIM(direction, tmpSol, BX, FB);

	if (FB > FA) {
		DUM = AX;
		AX = BX;
		BX = DUM;
		DUM = FB;
		FB = FA;
		FA = DUM;
	}  // returned FA is always bigger than or equal to FB

	CX = BX + GOLD * (BX - AX);

	F1DIM(direction, tmpSol, CX, FC);

	for (int counter = 0; counter < 1000 && FB >= FC; counter++) {
		R = (BX - AX)*(FB - FC);
		Q = (BX - CX)*(FB - FA);
		//U = BX - ((BX - CX)*Q - (BX - AX)*R) / (2.0*Sign(std::max(fabs(Q - R), TINY), Q - R));
		U = BX - ((BX - CX)*Q - (BX - AX)*R) / (2.0f*Sign(std::max(fabs(Q - R), TINY), Q - R));

		ULIM = BX + GLIMIT * (CX - BX);

		if ((BX - U)*(U - CX) > 0.0f) {
			F1DIM(direction, tmpSol, U, FU);

			if (FU < FC) {
				AX = BX;
				FA = FB;
				BX = U;
				FB = FU;
			} else if (FU > FB) {
				CX = U;
				FC = FU;
			} else {
				U = CX + GOLD * (CX - BX);

				F1DIM(direction, tmpSol, U, FU);

				AX = BX;
				BX = CX;
				CX = U;
				FA = FB;
				FB = FC;
				FC = FU;
			}
		} else if ((CX - U)*(U - ULIM) > 0.0f) {
			F1DIM(direction, tmpSol, U, FU);

			if (FU < FC) {
				BX = CX;
				CX = U;
				U = CX + GOLD * (CX - BX);
				FB = FC;
				FC = FU;
				F1DIM(direction, tmpSol, U, FU);
			}

			AX = BX;
			BX = CX;
			CX = U;
			FA = FB;
			FB = FC;
			FC = FU;
		} else if ((U - ULIM)*(ULIM - CX) >= 0.0f) {
			U = ULIM;
			F1DIM(direction, tmpSol, U, FU);

			AX = BX;
			BX = CX;
			CX = U;
			FA = FB;
			FB = FC;
			FC = FU;
		} else // 2차함수의 꼭지점이 위를 향할 경우 해당
		{
			U = CX + GOLD * (CX - BX);
			F1DIM(direction, tmpSol, U, FU);

			AX = BX;
			BX = CX;
			CX = U;
			FA = FB;
			FB = FC;
			FC = FU;
		}
	}
}

bool RigidPW::BRENT(float *direction, float *tmpSol, float &AX, float &BX, float &CX, float FX, float TOL, float &XMIN, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return false;
	}

	const float CGOLD = 0.3819660, ZEPS = 1e-5;
	float A, B, D, E, ETEMP, FU, FV, FW, P, Q, R, TOL1, TOL2, U, V, W, X, XM;

	A = std::min(AX, CX);
	B = std::max(AX, CX);
	V = BX;
	W = V;
	X = V;
	E = 0.0f;
	FV = FX;
	FW = FX;

	int ITER;
	for (ITER = 0; ITER < m_ITMAX; ITER++) //main loop
	{
		XM = 0.5f*(A + B);
		TOL1 = TOL * fabs(X) + ZEPS;
		TOL2 = 2.0f*TOL1;

		if (fabs(X - XM) <= (TOL2 - 0.5*(B - A))) {
			XMIN = X;   //exit section
			FRET = FX;
			return true;
		}

		if (fabs(E) > TOL1) {               //Construct a trial parabolic fit
			R = (X - W)*(FX - FV);
			Q = (X - V)*(FX - FW);
			P = (X - V)*Q - (X - W)*R;
			Q = 2.0f*(Q - R);
			if (Q > 0.0f)
				P = -P;

			Q = fabs(Q);
			ETEMP = E;
			E = D;
			if (fabs(P) >= fabs(0.5*Q*ETEMP) || P <= Q * (A - X) || P >= Q * (B - X))
				goto e1;

			//  The above conditions determine the acceptability of the
			//  parabolic fit. Here it is o.k.
			D = P / Q;
			U = X + D;
			if (U - A < TOL2 || B - U < TOL2)
				D = Sign(TOL1, XM - X);

			goto e2;
		}
	e1:		if (X >= XM) // golden section search
		E = A - X;
			else
		E = B - X;

			D = CGOLD * E;

		e2:		if (fabs(D) >= TOL1) // goto e2 로 바로 왔다면 parabolic fitting search
			U = X + D;
				else
			U = X + Sign(TOL1, D);

				F1DIM(direction, tmpSol, U, FU);

				//This the one function evaluation per iteration

				if (FU <= FX) {
					if (U >= X)
						A = X;
					else
						B = X;

					V = W;
					FV = FW;
					W = X;
					FW = FX;
					X = U;
					FX = FU;
				} else {
					if (U < X)
						A = U;
					else
						B = U;

					if (FU <= FW || W == X) {
						V = W;
						FV = FW;
						W = U;
						FW = FU;
					} else if (FU <= FV || V == X || V == W) {
						V = U;
						FV = FU;
					}
				}
	}
	printf("\n Brent exceed maximum iterations.\n\n");
	return false;
}

void RigidPW::F1DIM(float *direction, float *tmpSol, float alpha, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	bool flagok = false;

	for (int i = 0; i < m_Nvariable; i++) {
		tmpSol[i] = m_regi_param[i] + alpha * direction[i];

		if (tmpSol[i] >= -m_feasibleRange[i] && tmpSol[i] <= m_feasibleRange[i]) {
			flagok = true;
		} else {
			flagok = false;
			break;
		}
	}

	if (flagok) {
		switch (m_problem) {
		case EPRO::POINTMATCHING:
			TransformErrorSR(tmpSol, FRET);
			break;
		case EPRO::REORIENTATION:
			TransformError(tmpSol, FRET);
			break;
		case EPRO::SUPERIMPOSE:
			TransformErrorSUPERIMPOSE(tmpSol, FRET);
			break;
		}
	} else {
		FRET = FLT_MAX;
	}
}

void RigidPW::runRegistrationRef(float *Sol) {
	if (!m_pOCL->is_valid())
	{
		return;
	}

	m_pSRCL->rendering(&m_mem_refSR, 70, 70, Sol, true);
}

bool RigidPW::runRegistration(float *Sol, float &FRET) {
	if (!m_pOCL->is_valid())
	{
		return false;
	}

	//float FRET;
	float prevSol[MAX_PARAM_N];
	float tmpDirection[MAX_PARAM_N];
	float candiSol[MAX_PARAM_N];
	float tmpSol[MAX_PARAM_N];

	memcpy(m_regi_param, Sol, m_Nvariable * sizeof(float));

	switch (m_problem) {
	case EPRO::POINTMATCHING:
		TransformErrorSR(m_regi_param, FRET);
		break;
	case EPRO::REORIENTATION:
		TransformError(m_regi_param, FRET);
		break;
	case EPRO::SUPERIMPOSE:
		TransformErrorSUPERIMPOSE(m_regi_param, FRET);
		break;
	}

	int i;
	if (FRET > 0.0f) {
		memcpy(prevSol, m_regi_param, m_Nvariable * sizeof(float));

		int ITER;
		for (ITER = 0; ITER < m_ITMAX; ITER++) {
			float FP = FRET;
			printf("Error: %f\n", FP);
			float maxDecrease = 0.0f;
			float tmpF;
			float tmpDecrease = 0.0f;
			float T = 0.0f;

			int maxIndex = -1;

			for (i = 0; i < m_Nvariable; i++) {
				tmpF = FRET;

				if (!LINMIN(m_directions + i * m_Nvariable, tmpSol, FRET))
					return false;

				tmpDecrease = fabs(tmpF - FRET);

				if (tmpDecrease > maxDecrease) {
					maxDecrease = tmpDecrease;
					maxIndex = i;
				}
			}

			if (2.0f*fabs(FP - FRET) <= m_FTOL * (fabs(FP)*fabs(FRET))) {
				break;
			}

			for (i = 0; i < m_Nvariable; i++) {
				candiSol[i] = 2.0f*m_regi_param[i] - prevSol[i];
				tmpDirection[i] = m_regi_param[i] - prevSol[i];
				prevSol[i] = m_regi_param[i];
			}

			switch (m_problem) {
			case EPRO::POINTMATCHING:
				TransformErrorSR(candiSol, tmpF);
				break;
			case EPRO::REORIENTATION:
				TransformError(candiSol, tmpF);
				break;
			case EPRO::SUPERIMPOSE:
				TransformErrorSUPERIMPOSE(candiSol, tmpF);
				break;
			}

			if (tmpF < FP) {
				T = 2.0f*(FP - 2.0f*FRET + tmpF)*Sqr(FP - FRET - maxDecrease) - maxDecrease * Sqr(FP - tmpF);
				if (T < 0.0f) {
					if (!LINMIN(tmpDirection, tmpSol, FRET))
						return false;

					for (i = 0; i < m_Nvariable; i++) {
						m_directions[maxIndex*m_Nvariable + i] = tmpDirection[i];
					}
				}
			}
		}

		memcpy(Sol, m_regi_param, sizeof(float)*m_Nvariable);
	}

	return true;
}

void RigidPW::checkError(cl_int& status, int nLine, const char* strErrMsg) {
	if (status != CL_SUCCESS) {
		std::string errMsg =
			std::string("RidigPW [line: ")
			+ std::to_string(nLine)
			+ std::string(" ] ")
			+ strErrMsg;
		Logger::instance()->Print(LogType::ERR, errMsg);
		CLplatform::printError(status);
	}
}

// smseo : unused member functions
//bool RigidPW::setVolume(unsigned short *vol)
//{
//	m_image_format.image_channel_order = CL_A;
//	m_image_format.image_channel_data_type = CL_SIGNED_INT16;
//
//	cl_int status;
//	if(!m_mem_Vol)
//	{
//		m_mem_Vol = clCreateImage3D(
//			m_pOCL->getContext(),
//			CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
//			&m_image_format,
//			m_nx,
//			m_ny,
//			m_nz,
//			0,
//			0,
//			vol,
//			&status);
//		checkError(status, __LINE__, "clCreateImage3D failed.");
//	}
//
//	if(!m_mem_Ref)
//	{
//		m_mem_Ref = clCreateBuffer(
//			m_pOCL->getContext(),
//			CL_MEM_READ_WRITE,
//			sizeof(unsigned short)*m_nx*m_ny*m_nz,
//			nullptr,
//			&status);
//
//		checkError(status, __LINE__, "clCreateBuffer failed.");
//	}
//
//
//	cl_int statusAll = 0;
//
//	if(!m_kernel_TransformCL)
//		m_kernel_TransformCL = clCreateKernel(m_program, "sTransformCL", &status);
//
//	statusAll |= status;
//
//	if(!m_kernel_DifferenceXReflectionCL)
//		m_kernel_DifferenceXReflectionCL = clCreateKernel(m_program, "sDifferenceXReflectionCL", &status);
//
//	statusAll |= status;
//
//
//	if(!m_kernel_Transform2CL)
//		m_kernel_Transform2CL = clCreateKernel(m_program, "sTransform2CL", &status);
//
//	statusAll |= status;
//	checkError(statusAll, __LINE__, "clCreateKernel failed.");
//
//	return true;
//}
//bool RigidPW::setVolume(short *vol)
//{
//	m_image_format.image_channel_order = CL_A;
//	m_image_format.image_channel_data_type = CL_SIGNED_INT16;
//
//	cl_int status;
//	m_mem_Vol = clCreateImage3D(
//		m_pOCL->getContext(),
//		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
//		&m_image_format,
//		m_nx,
//		m_ny,
//		m_nz,
//		0,
//		0,
//		vol,
//		&status);
//
//	checkError(status, __LINE__, "clCreateImage3D failed.");
//
//	if(!m_mem_Ref)
//	{
//		m_mem_Ref = clCreateBuffer(
//			m_pOCL->getContext(),
//			CL_MEM_READ_WRITE,
//			sizeof(short)*m_nx*m_ny*m_nz,
//			nullptr,
//			&status);
//
//		checkError(status, __LINE__, "clCreateBuffer failed.");
//	}
//
//	if(!m_kernel_TransformCL)
//		m_kernel_TransformCL = clCreateKernel(m_program, "sTransformCL", &status);
//
//	checkError(status, __LINE__, "clCreateKernel failed.");
//
//	if (!m_kernel_DifferenceXReflectionCL){
//		m_kernel_DifferenceXReflectionCL = clCreateKernel(m_program, "sDifferenceXReflectionCL", &status);
//		checkError(status, __LINE__, "clCreateKernel failed.");
//	}
//
//	return true;
//}
