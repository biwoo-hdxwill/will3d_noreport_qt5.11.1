#include "W3Jobmgr.h"
/*=========================================================================

File:			class CW3JobMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last date:		2016-01-25

=========================================================================*/
#include "../../../Engine/Common/Common/W3Memory.h"

#ifndef WILL3D_LIGHT
#include "../../../Engine/Core/CLPlatform/WCLplatform.h"

#include "../../Engine/Module/Registration/W3Registration.h"
#include "../../Engine/Module/Endoscopy/W3Endoscopy.h"
#include "../../Engine/Module/Face/W3Face.h"
#endif
#include "../../Engine/Module/Panorama/pano_algorithm.h"
#include "../../Engine/Module/TMJ/W3TMJ.h"

CW3JobMgr::CW3JobMgr(CW3VREngine *VREngine, CW3ResourceContainer *Rcontainer)
: m_pgRcontainer(Rcontainer)
{
#ifndef WILL3D_LIGHT
	m_pCLplatform = new CLplatform(true);
	m_pReorientationMod = new CW3Registration(m_pCLplatform, EPRO::REORIENTATION);
	m_pSIMod = new CW3Registration(m_pCLplatform, EPRO::SUPERIMPOSE);
	m_pFaceMod = new CW3Face(VREngine, m_pCLplatform, m_pgRcontainer);
	m_pEndoMod = new CW3Endoscopy();
#endif
	m_pTMJMod = new CW3TMJ();
	m_pPanoMod = new PanoAlgorithm();
}

CW3JobMgr::~CW3JobMgr()
{
#ifndef WILL3D_LIGHT
	SAFE_DELETE_OBJECT(m_pCLplatform);
	SAFE_DELETE_OBJECT(m_pReorientationMod);
	SAFE_DELETE_OBJECT(m_pSIMod);
	SAFE_DELETE_OBJECT(m_pFaceMod);
	SAFE_DELETE_OBJECT(m_pEndoMod);
#endif
	SAFE_DELETE_OBJECT(m_pTMJMod);
	SAFE_DELETE_OBJECT(m_pPanoMod);
}

void CW3JobMgr::setVolume(CW3Image3D *vol)
{
#ifndef WILL3D_LIGHT
	m_pReorientationMod->setVolume(vol);
	m_pSIMod->setVolume(vol);
	m_pFaceMod->setVolume(vol);
#endif
}

#ifndef WILL3D_LIGHT
/*=========================================================================
MPR
=========================================================================*/
void CW3JobMgr::runReorientation()
{
	std::vector<float> vTrans = m_pReorientationMod->runReorientation();
	emit sigReorientedFromMPRtab(vTrans);
	printf("rotZ: %f, tranX: %f, rotY: %f\n", vTrans[0], vTrans[1], vTrans[2]);
}
#endif
/*=========================================================================
Panorama
=========================================================================*/

//void CW3JobMgr::runAutoArch(CW3Image3D * vol, std::vector<glm::vec3>& points)
//{
//	m_pPanoMod->RunMandibleAutoArch(vol, points);
//}

void CW3JobMgr::runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume)
{
	m_pPanoMod->runAutoCanal(out, coord, pixelsize, pVolume);
}

void CW3JobMgr::runAutoCanal(std::vector<glm::vec3>* out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume)
{
	m_pPanoMod->runAutoCanal(out, coord, pixelsize, pVolume);
}

void CW3JobMgr::runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume, int *startZ, int *endZ)
{
	m_pPanoMod->runAutoCanal(out, coord, pixelsize, pVolume, startZ, endZ);
}

void CW3JobMgr::runAutoCanal(std::vector<glm::vec3>* out, float pixelsize, CW3Image3D* pVolume)
{
	m_pPanoMod->runAutoCanal(out, pixelsize, pVolume);
}

#ifndef WILL3D_LIGHT
/*=========================================================================
FaceSimulation
=========================================================================*/
void CW3JobMgr::runSurfacing(float minValue, int down, bool isNormalNeeded)
{
	m_pFaceMod->runSurfacing(minValue, down, isNormalNeeded);
}

void CW3JobMgr::runMarchingCube(float minValue, int down, bool isNormalNeeded)
{
	m_pFaceMod->runMarchingCube(minValue, down, isNormalNeeded);
}
void CW3JobMgr::runRegiPoints2()
{
	glm::mat4* reorienMat = m_pFaceMod->runRegiPoints2();

	emit sigUpdateMPRPhotoFromFaceTab();
	emit sigPointModelFromFaceTab(reorienMat);

}
void CW3JobMgr::runRegiPoints2OnlyTRD()
{
	glm::mat4* reorienMat = m_pFaceMod->runRegiPoints2OnlyTRD();

	emit sigPointModelFromFaceTab(reorienMat);
}

/*=========================================================================
SuperImposition
=========================================================================*/

void CW3JobMgr::runSuperImpose(glm::mat4 & out, CW3Image3D * moving)
{
	float* trans = m_pSIMod->runSuperImpose(out, moving);
	emit sigSecondDisabledFromSuperTab(false, trans);
}
#endif

/*=========================================================================
TMJ
=========================================================================*/
void CW3JobMgr::ActionforTMJEx2(int w, int h, int d, unsigned short ** ppUSVolume, unsigned char ** ppOutMask, float pixelsize, unsigned short intercept)
{
	m_pTMJMod->ActionforTMJEx2(w, h, d, ppUSVolume, ppOutMask, pixelsize, intercept);
}

void CW3JobMgr::ActionforTMJEx2(int w, int h, int d, unsigned short ** ppUSVolume, unsigned char ** ppOutMask, float pixelsize, unsigned short intercept, int thd_tissue_bone, int thd_air_tissue) {
	m_pTMJMod->ActionforTMJEx2(w, h, d, ppUSVolume, ppOutMask, pixelsize, intercept, thd_tissue_bone, thd_air_tissue);
}

#ifndef WILL3D_LIGHT
/*=========================================================================
Endo
=========================================================================*/
bool CW3JobMgr::runAirwaySeg(unsigned short ** ppVolData3D, unsigned char ** ppMaskData3D, int nW, int nH, int nD, int nDataMin, float fPixelSpacing, float fIntercept, std::vector<glm::vec3> vecCP, std::vector<glm::vec3> vecPath, int nDownFactor, int nAirwayMax, bool bClosing)
{
	return m_pEndoMod->runAirwaySeg(ppVolData3D, ppMaskData3D, nW, nH, nD, nDataMin, fPixelSpacing, fIntercept, vecCP, vecPath, nDownFactor, nAirwayMax, bClosing);
}

std::vector<tri_STL> CW3JobMgr::getMeshSTL()
{
	return m_pEndoMod->getMeshSTL();
}

#endif
