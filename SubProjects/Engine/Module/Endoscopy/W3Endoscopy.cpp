#include "W3Endoscopy.h"

#include "../../../Engine/Common/Common/W3Memory.h"
#include "AirwaySeg/AirwaySeg.h"

CW3Endoscopy::CW3Endoscopy()
{
	m_pAirwaySeg = new CAirwaySeg();
}

CW3Endoscopy::~CW3Endoscopy()
{
	SAFE_DELETE_OBJECT(m_pAirwaySeg);
}

bool CW3Endoscopy::runAirwaySeg(
	unsigned short** ppVolData3D, 
	unsigned char** ppMaskData3D,
	int nW, int nH, int nD,
	int nDataMin, float fPixelSpacing, float fIntercept,
	std::vector<glm::vec3> vecCP, 
	std::vector<glm::vec3> vecPath,
	int nDownFactor, int nAirwayMax, bool bClosing)
{
	return m_pAirwaySeg->doRun(ppVolData3D, ppMaskData3D,
		nW, nH, nD, nDataMin, fPixelSpacing, fIntercept,
		vecCP, vecPath, nDownFactor, nAirwayMax, bClosing);
}

std::vector<tri_STL> CW3Endoscopy::getMeshSTL()
{
	return m_pAirwaySeg->getMeshSTL();
}
