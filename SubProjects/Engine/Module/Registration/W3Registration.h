#pragma once

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <QObject>

#include "../../Module/RigidPW/RigidPW.h"
#include "registration_global.h"

class CLplatform;
class CW3Image3D;
class CW3VREngine;
class CW3ResourceContainer;

/**********************************************************************************************//**
 * @class	CW3Registration
 *
 * @brief	Registration class : 3가지 모드의 registration을 동작한다.
 * 			
 *
 * @author	Seo Seok Man
 * @date	2017-06-28
 **************************************************************************************************/

class REGISTRATION_EXPORT CW3Registration : public QObject
{
public:
	CW3Registration(CLplatform* clPlatform, EPRO eProblem);
	~CW3Registration();

	void setVolume(CW3Image3D* pVol);
	inline float* solutionPoints(void) { return m_RegiPointsSol; }

	/**********************************************************************************************//**
	 * @fn	std::vector<float> Registration::runReorientation();
	 *
	 * @brief	Executes the reorientation operation.
	 * 			problem mode : REORIENTATION
	 *
	 * @author	Seo Seok Man
	 * @date	2017-06-28
	 *
	 * @return	A std::vector<float> : m_ReOriSol[3]
	 **************************************************************************************************/
	std::vector<float> runReorientation();

	/**********************************************************************************************//**
	 * @fn	float* Registration::runSuperImpose(glm::mat4 &out, CW3Image3D *moving);
	 *
	 * @brief	Executes the super impose operation.
	 * 			problem mode : SUPERIMPOSE
	 *
	 * @author	Seo Seok Man
	 * @date	2017-06-28
	 *
	 * @param [out]	out   	transform matrix.
	 * @param [in]	moving	second volume.
	 *
	 * @return	m_RegiPointsSol[6]
	 **************************************************************************************************/
	float* runSuperImpose(glm::mat4 &out, CW3Image3D *moving);

	/**********************************************************************************************//**
	 * @fn	glm::mat4* Registration::runPointMatching(CW3VREngine* pVR, CW3TRDsurface *surfaceMoving, CW3TRDsurface *surfaceRef);
	 *
	 * @brief	Executes the point matching operation.
	 * 			problem mode : POINTMATCHING
	 *
	 * @author	Seo Seok Man
	 * @date	2017-06-28
	 *
	 * @param [in]	pVR			 	VR Engine.
	 * @param [in]	surfaceMoving	moving surface.
	 * @param [in]	surfaceRef   	reference surface.
	 *
	 * @return	m_RegiPointsSol[6]
	 **************************************************************************************************/
	float* runPointMatching(
		CW3VREngine* pVR,
		CW3TRDsurface *surfaceMoving, 
		CW3TRDsurface *surfaceRef);

private:
	void runRegistration();
	bool isRunReorientation();

	void initSolPoints();
	void initSolRegiPoints();

	void printRegiPoints(const char* strFuncName);

private:
	CW3Image3D* m_pgVol = nullptr;
	RigidPW* m_pRigidPW = nullptr;

	float m_ReOriSol[3];
	float m_RegiPointsSol[6];
};
