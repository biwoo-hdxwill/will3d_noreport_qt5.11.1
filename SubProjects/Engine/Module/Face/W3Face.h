#pragma once

/*=========================================================================

File:			class CW3Face
Language:		C++11
Library:		Qt 5.8
Author:			Jung Dae Gun
First date:		2017-03-09
Last date:		2017-03-09

=========================================================================*/
#include <QObject>

#include "../../Common/GLfunctions/WGLHeaders.h"

#include "face_global.h"

class CW3VREngine;
class CW3ResourceContainer;
class CW3TRDsurface;
class CW3Registration;
class CLplatform;
class CW3Image3D;

class FACE_EXPORT CW3Face : public QObject
{
	Q_OBJECT

public:
	CW3Face(CW3VREngine *VREngine, CLplatform* clPlatformGL, CW3ResourceContainer *Rcontainer);
	~CW3Face();

	void setVolume(CW3Image3D* vol);

	void runSurfacing(float minValue, int down, bool isNormalNeeded);
	void runMarchingCube(float minValue, int down, bool isNormalNeeded);
	glm::mat4* runRegiPoints2();
	glm::mat4* runRegiPoints2OnlyTRD();

private:
	CW3TRDsurface*		m_pMCsurface = nullptr;
	CW3Registration*	m_pRegiMod = nullptr;

	CW3VREngine*			m_pgVREngine;
	CW3ResourceContainer*	m_pgRcontainer;
	CW3Image3D*				m_pgVol;

	std::vector<glm::vec3>		m_MCpoints;
	std::vector<glm::u32vec3>	m_MCindices;
	std::vector<glm::vec3>		m_MCnormals;

	glm::mat4 m_RegiPointsModel;
};
