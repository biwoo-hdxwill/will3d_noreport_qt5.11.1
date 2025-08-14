#pragma once
/*=========================================================================

File:			class CW3JobMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-18
Last date:		2016-01-25

=========================================================================*/
#include <vector>
#include <QObject>

#include "../../Engine/Common/GLfunctions/WGLHeaders.h"
#include "../../Engine/Common/GLfunctions/W3GLTypes.h"

#include "jobmgr_global.h"

class CW3Image3D;
class CW3VREngine;
class CW3ResourceContainer;

#ifndef WILL3D_LIGHT
class CLplatform;
class CW3Registration;
class CW3Endoscopy;
class CW3Face;
#endif
class CW3TMJ;
class PanoAlgorithm;

class JOBMGR_EXPORT CW3JobMgr : public QObject {
	Q_OBJECT

public:
	CW3JobMgr(CW3VREngine *VREngine, CW3ResourceContainer *Rcontainer);
	~CW3JobMgr();

public:
	void setVolume(CW3Image3D *vol);

	//MPR
	void runReorientation();

	//Panorama
	//void runAutoArch(CW3Image3D* vol, std::vector<glm::vec3>& points);
	void runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume);
	void runAutoCanal(std::vector<glm::vec3> *out, glm::vec3 *coord, float pixelsize, CW3Image3D *pVolume);
	void runAutoCanal(unsigned char **out, glm::vec3 *coord, float pixelsize, CW3Image3D* pVolume, int *startZ, int *endZ);
	void runAutoCanal(std::vector<glm::vec3> *out, float pixelsize, CW3Image3D *pVolume);

	//FaceSimulation
	void runSurfacing(float minValue, int down, bool isNormalNeeded);
	void runMarchingCube(float minValue, int down, bool isNormalNeeded);
	void runRegiPoints2();
	void runRegiPoints2OnlyTRD();

	//SuperImposition
	void runSuperImpose(glm::mat4 &out, CW3Image3D *moving);

	//TMJ
	void ActionforTMJEx2(int w, int h, int d, unsigned short** ppUSVolume, unsigned char** ppOutMask, float pixelsize, unsigned short intercept);
	void ActionforTMJEx2(int w, int h, int d, unsigned short ** ppUSVolume, unsigned char ** ppOutMask, float pixelsize, unsigned short intercept, int thd_tissue_bone, int thd_air_tissue);

	//Endo
	bool runAirwaySeg(
		unsigned short **ppVolData3D, unsigned char **ppMaskData3D,
		int nW, int nH, int nD, int nDataMin, float fPixelSpacing, float fIntercept,
		std::vector<glm::vec3> vecCP, std::vector<glm::vec3> vecPath,
		int nDownFactor = 1, int nAirwayMax = -800, bool bClosing = false);

	std::vector<tri_STL> getMeshSTL();

signals:
	//MPR
	void sigReorientedFromMPRtab(std::vector<float>);

	//FaceSimulation
	void sigPointModelFromFaceTab(glm::mat4*);
	void sigUpdateMPRPhotoFromFaceTab();

	//SuperImposition
	void sigSecondDisabledFromSuperTab(bool, float *);

private:
	CW3ResourceContainer *m_pgRcontainer;

#ifndef WILL3D_LIGHT
	CLplatform *m_pCLplatform;

	CW3Registration*	m_pReorientationMod = nullptr;
	CW3Registration*	m_pSIMod = nullptr;
	CW3Face*			m_pFaceMod = nullptr;
	CW3Endoscopy*		m_pEndoMod = nullptr;
#endif
	CW3TMJ*				m_pTMJMod = nullptr;
	PanoAlgorithm*		m_pPanoMod = nullptr;
};
