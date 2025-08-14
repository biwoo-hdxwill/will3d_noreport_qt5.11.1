#pragma once
/*=========================================================================

File:			class CW3Endoscopy
Language:		C++11
Library:		Qt 5.8
Author:			Jung Dae Gun
First date:		2017-03-09
Last date:		2017-03-09

=========================================================================*/
#include <vector>

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/W3GLTypes.h"
#include "endoscopy_global.h"

class CAirwaySeg;

class ENDOSCOPY_EXPORT CW3Endoscopy
{
public:
	CW3Endoscopy();
	~CW3Endoscopy();

	bool runAirwaySeg(
		unsigned short **ppVolData3D, 
		unsigned char **ppMaskData3D,
		int nW, int nH, int nD,
		int nDataMin, float fPixelSpacing, float fIntercept,
		std::vector<glm::vec3> vecCP, 
		std::vector<glm::vec3> vecPath,
		int nDownFactor = 1, int nAirwayMax = -800, bool bClosing = false);

	std::vector<tri_STL> getMeshSTL();

private:
	CAirwaySeg *m_pAirwaySeg;
};
