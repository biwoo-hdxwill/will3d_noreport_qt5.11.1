#pragma once
/*=========================================================================

File:			class RigidPW
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last date:		2015-12-18

=========================================================================*/
#include "../../Core/CLPlatform/WCLprogram.h"
#include "rigidpw_global.h"

#define MAX_PARAM_N 16

class CLplatform;
class ReductionCL;
class CW3Image3D;
class CW3TRDsurface;
class CW3SurfaceRenderingCL;
class CW3VREngine;

enum class EPRO
{
	REORIENTATION,
	POINTMATCHING,
	SUPERIMPOSE
};

/**********************************************************************************************//**
 * @class	RigidPW
 *
 * @brief	RigidPW. // TODO smseo : 이 프로젝트는 core에 올라가야 함.
 * 			현재 VR Engine을 물고있어서 어쩔 수 없이 Module에 있음
 * 			추후 VR Engine을 분리 가능한 상태가 되면 core로 올려야 함
 *
 * @author	Hong Jung
 * @date	2015-12-18
 **************************************************************************************************/
class RIGIDPW_EXPORT RigidPW : public WCLprogram
{
public:
	RigidPW(CLplatform* OCL, EPRO problem, int nx, int ny, int nz);
	RigidPW(CLplatform* OCL, EPRO problem);
	~RigidPW();

	//bool setVolume(unsigned short *vol);  smseo : 사용되지 않는 함수라 블록
	//bool setVolume(short *vol);  smseo : 사용되지 않는 함수라 블록
	bool setVolume(CW3Image3D *vol);
	bool setVolume(CW3Image3D *ref, CW3Image3D *moving, int boneIntensity, int downFactor);

	void setVRengine(CW3VREngine* VRengine);
	void setSurface(CW3TRDsurface *surfaceMoving, CW3TRDsurface *surfaceRef);

	void runRegistrationRef(float *Sol);
	bool runRegistration(float *Sol, float &FRET);
	void Transform(float *Sol);

	inline const unsigned int getDownFactor() const { return m_downFactor; }

	inline const EPRO problem() const { return m_problem; }

	void clearResources();

	void initRegiPoints();

	void transformVolume(CW3Image3D *ref, CW3Image3D *moving, float *param);

private:
	void init();
	void initSuperImpose(int boneIntensity);
	void setTM(float *param);
	void TransformError(float *Sol, float &FRET);
	void TransformErrorSUPERIMPOSE(float *Sol, float &FRET);
	void TransformErrorSR(float *Sol, float &FRET, bool isFinal = false);
	bool LINMIN(float *direction, float *tmpSol, float &FRET);
	void MNBRAK(float *direction, float *tmpSol, float &AX, float &BX, float &CX, float &FA, float &FB, float &FC);
	bool BRENT(float *direction, float *tmpSol, float &AX, float &BX, float &CX, float FX, float TOL, float &XMIN, float &FRET);
	void F1DIM(float *direction, float *tmpSol, float alpha, float &FRET);
	void Difference2D(size_t *global, size_t *local);
	void Test2D(size_t *global, size_t *local);

	void checkError(cl_int& status, int nLine, const char* strErrMsg);

private:
	EPRO m_problem;

	ReductionCL*			m_ReductionCL = nullptr;
	CW3SurfaceRenderingCL*	m_pSRCL = nullptr;
	unsigned short*				m_pVolData = nullptr;
	unsigned short*				m_pVolDataMoving = nullptr;

	float m_feasibleRange[MAX_PARAM_N];
	float m_regi_param[MAX_PARAM_N];
	float m_directions[MAX_PARAM_N*MAX_PARAM_N];
	float m_TM[MAX_PARAM_N];

	int m_Nvariable = 0;
	unsigned int m_Nid = 0;
	unsigned int m_downFactor = 1;

	int m_nx, m_ny, m_nz;
	int m_nxM, m_nyM, m_nzM;
	float m_centerx, m_centery, m_centerz;
	float m_centerxM, m_centeryM, m_centerzM;

	int m_nTMele;
	int m_nTMx, m_nTMy;

	const float m_FTOL;
	const int m_ITMAX;

	cl_kernel m_kernel_TransformCL = 0;
	cl_kernel m_kernel_Transform2CL = 0;
	cl_kernel m_kernel_DifferenceXReflectionCL = 0;
	cl_kernel m_kernel_Difference2D = 0;
	cl_kernel m_kernel_test2D = 0;

	cl_mem m_mem_Vol = 0;
	cl_mem m_mem_Ref = 0;
	cl_mem m_mem_fTM = 0;
	cl_mem m_mem_fDifference = 0;
	cl_mem m_mem_bIsIn = 0;
	cl_mem m_mem_IsInCount = 0;
	cl_mem m_mem_refSR = 0;
	cl_mem m_mem_movingSR = 0;
	cl_mem m_mem_id = 0;
	cl_image_format m_image_format;

	size_t m_localWorkSize[2];
	size_t m_localWorkSize1D[1];
};

