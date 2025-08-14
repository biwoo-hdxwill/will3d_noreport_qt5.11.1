#include "W3Registration.h"

#include <QtConcurrent/QtConcurrent>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Module/VREngine/W3VREngine.h"

CW3Registration::CW3Registration(CLplatform* clPlatform, EPRO eProblem)
{
	m_pRigidPW = new RigidPW(clPlatform, eProblem);
	initSolPoints();
}

CW3Registration::~CW3Registration()
{
	SAFE_DELETE_OBJECT(m_pRigidPW);
}

void CW3Registration::setVolume(CW3Image3D *vol)
{
	m_pgVol = vol;
	initSolPoints();
}

std::vector<float> CW3Registration::runReorientation()
{
	if (!m_pRigidPW->IsValid())
	{
		common::Logger* logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "OpenCL is not valid.");
	}

	if (!isRunReorientation() && m_pRigidPW->IsValid())
	{
		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		QFuture<void> future;

		future = QtConcurrent::run(this, &CW3Registration::runRegistration);

		watcher.setFuture(future);
		progress->exec();

		watcher.waitForFinished();

		m_pRigidPW->clearResources();
	}

	std::vector<float> out = { m_ReOriSol[0], m_ReOriSol[1], m_ReOriSol[2] };
	return out;
}

float* CW3Registration::runSuperImpose(glm::mat4 &out, CW3Image3D *moving)
{
	if (!m_pRigidPW->IsValid())
	{
		common::Logger* logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "OpenCL is not valid.");
		return m_RegiPointsSol;
	}

	//m_RegiPointsSol[0] = 10.0f / 180.0f*CL_M_PI;			// zrot
	//m_RegiPointsSol[1] = 10.0f;							// xtrans
	//m_RegiPointsSol[2] = 10.0f;							// ytrans
	//m_RegiPointsSol[3] = 10.0f / 180.0f*CL_M_PI;			// xrot
	//m_RegiPointsSol[4] = 10.0f;							// ztrans
	//m_RegiPointsSol[5] = 10.0f / 180.0f*CL_M_PI;			// yrot
	//m_pRigidPW->transformVolume(m_pgVol, moving, m_RegiPointsSol);
	//m_pRigidPW->clearResources();
	//printf("Lowest Resolution SuperImpose\n");
	//printf("Xtrans: %f, Ytrans: %f, Ztrans: %f, Xrot: %f, Yrot: %f, Zrot: %f\n",
	//  -m_RegiPointsSol[1], -m_RegiPointsSol[2], -m_RegiPointsSol[4],
	//	-m_RegiPointsSol[3], -m_RegiPointsSol[5], -m_RegiPointsSol[0]);

	// boneIntensity가 현재 TFMin로 되어있는데 진짜 bone intensity로 변경해야함
	//m_pRigidPW->setVolume(m_pgVol, moving, m_pgRcontainer->getTF()->getMinValue(), 4);

	float FRET = 0.0f;
	m_pRigidPW->setVolume(m_pgVol, moving, m_pgVol->getTissueBoneThreshold(), 4);
	m_pRigidPW->runRegistration(m_RegiPointsSol, FRET);
	m_pRigidPW->clearResources();

	////m_RegiPointsSol[0] += -float(5.0f / 180.0f*CL_M_PI);
	////m_RegiPointsSol[3] += -float(5.0f / 180.0f*CL_M_PI);
	m_RegiPointsSol[1] *= m_pRigidPW->getDownFactor() / 2;
	m_RegiPointsSol[2] *= m_pRigidPW->getDownFactor() / 2;
	m_RegiPointsSol[4] *= m_pRigidPW->getDownFactor() / 2;

	////rotZ, tranX, tranY, rotX, rotY, tranZ
	////m_RegiPointsSol[1] *= m_pRigidPW->getDownFactor();
	////m_RegiPointsSol[2] *= m_pRigidPW->getDownFactor();
	////m_RegiPointsSol[5] *= m_pRigidPW->getDownFactor();
	
	common::Logger* logger = common::Logger::instance();
	logger->PrintDebugMode("CW3Registration::runSuperImpose", "Lowest Resolution SuperImpose");
	printRegiPoints("CW3Registration::runSuperImpose");

	m_pRigidPW->setVolume(m_pgVol, moving, m_pgVol->getTissueBoneThreshold(), 2);
	m_pRigidPW->runRegistration(m_RegiPointsSol, FRET);
	m_pRigidPW->clearResources();

	m_RegiPointsSol[1] *= m_pRigidPW->getDownFactor();
	m_RegiPointsSol[2] *= m_pRigidPW->getDownFactor();
	m_RegiPointsSol[4] *= m_pRigidPW->getDownFactor();

	logger->PrintDebugMode("CW3Registration::runSuperImpose", "Second Resolution SuperImpose");
	printRegiPoints("CW3Registration::runSuperImpose");

	m_pRigidPW->setVolume(m_pgVol, moving, m_pgVol->getTissueBoneThreshold(), 1);
	m_pRigidPW->runRegistration(m_RegiPointsSol, FRET);
	m_pRigidPW->clearResources();

	logger->PrintDebugMode("CW3Registration::runSuperImpose", "Done SuperImpose");
	printRegiPoints("CW3Registration::runSuperImpose");

	//out =
	//  glm::translate(glm::vec3(-m_RegiPointsSol[1] * 2.0f, m_RegiPointsSol[2] *
	//   2.0f, m_RegiPointsSol[4] * 2.0f))*
	//  glm::rotate(m_RegiPointsSol[3], glm::vec3(-1.0f, 0.0f, 0.0f))*
	//	glm::rotate(m_RegiPointsSol[5], glm::vec3(0.0f, 1.0f, 0.0f))*
	//	glm::rotate(m_RegiPointsSol[0], glm::vec3(0.0f, 0.0f, 1.0f));
	//out = 
	//  glm::rotate(m_RegiPointsSol[0], glm::vec3(0.0f, 0.0f, -1.0f))*
	//	glm::rotate(m_RegiPointsSol[4], glm::vec3(0.0f, -1.0f, 0.0f))*
	//	glm::rotate(m_RegiPointsSol[3], glm::vec3(1.0f, 0.0f, 1.0f))*
	//	glm::translate(glm::vec3(m_RegiPointsSol[1] * 2.0f, -m_RegiPointsSol[2] * 2.0f, -m_RegiPointsSol[5] * 2.0f));

	out =
		glm::rotate(m_RegiPointsSol[0], glm::vec3(0.0f, 0.0f, -1.0f)) *
		glm::rotate(m_RegiPointsSol[5], glm::vec3(0.0f, -1.0f, 0.0f)) *
		glm::rotate(m_RegiPointsSol[3], glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::translate(glm::vec3(m_RegiPointsSol[1] * 2.0f, -m_RegiPointsSol[2] * 2.0f, -m_RegiPointsSol[4] * 2.0f));
	
	return m_RegiPointsSol;
}

float* CW3Registration::runPointMatching(
	CW3VREngine* pVR,
	CW3TRDsurface *surfaceMoving, 
	CW3TRDsurface *surfaceRef)
{
	if (!m_pRigidPW->IsValid())
	{
		common::Logger* logger = common::Logger::instance();
		logger->Print(common::LogType::ERR, "OpenCL is not valid.");
		return m_RegiPointsSol;
	}

	pVR->setProjectionEvn();

	m_pRigidPW->setVRengine(pVR);
	m_pRigidPW->setSurface(surfaceMoving, surfaceRef);
	m_pRigidPW->initRegiPoints();

	initSolRegiPoints();

	float tempRegiPointsSol[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	float FRET = 0.0f;
	float minFRET = std::numeric_limits<float>::max();
	m_pRigidPW->runRegistrationRef(m_RegiPointsSol);
	m_pRigidPW->runRegistration(tempRegiPointsSol, FRET);
	if (FRET < minFRET)
	{
		minFRET = FRET;
		m_RegiPointsSol[0] = tempRegiPointsSol[0];
		m_RegiPointsSol[1] = tempRegiPointsSol[1];
		m_RegiPointsSol[2] = tempRegiPointsSol[2];
		m_RegiPointsSol[3] = tempRegiPointsSol[3];
		m_RegiPointsSol[4] = tempRegiPointsSol[4];
		m_RegiPointsSol[5] = tempRegiPointsSol[5];
	}

	common::Logger* logger = common::Logger::instance();
	logger->PrintDebugMode("CW3Registration::runRegiPoints2", 
		"========== first FRET : " + std::to_string(FRET));

	for (int i = 1; i <= 10; i++)
	{
		if (FRET <= 10.0f)
			break;

		float zPos = i * 10.0f;
		if (zPos > (m_pgVol->depth() * 0.25f))
			break;

		FRET = 0.0f;

		m_pRigidPW->initRegiPoints();

		tempRegiPointsSol[0] = zPos;
		tempRegiPointsSol[1] = 0;
		tempRegiPointsSol[2] = 0;
		tempRegiPointsSol[3] = 0;
		tempRegiPointsSol[4] = 0;
		tempRegiPointsSol[5] = 0;
		m_pRigidPW->runRegistration(tempRegiPointsSol, FRET);
		if (FRET < minFRET)
		{
			minFRET = FRET;
			m_RegiPointsSol[0] = tempRegiPointsSol[0];
			m_RegiPointsSol[1] = tempRegiPointsSol[1];
			m_RegiPointsSol[2] = tempRegiPointsSol[2];
			m_RegiPointsSol[3] = tempRegiPointsSol[3];
			m_RegiPointsSol[4] = tempRegiPointsSol[4];
			m_RegiPointsSol[5] = tempRegiPointsSol[5];
		}
		logger->PrintDebugMode("CW3Registration::runRegiPoints2",
			"========== bottom-" + std::to_string(i) + 
			" initZ : " + std::to_string(zPos) + 
			", FRET : " + std::to_string(FRET));
	}

	for (int i = 1; i <= 10; i++)
	{
		if (FRET <= 10.0f)
			break;

		float zPos = i * -10.0f;
		if (zPos < -(m_pgVol->depth() * 0.25f))
			break;

		FRET = 0.0f;

		m_pRigidPW->initRegiPoints();

		tempRegiPointsSol[0] = zPos;
		tempRegiPointsSol[1] = 0;
		tempRegiPointsSol[2] = 0;
		tempRegiPointsSol[3] = 0;
		tempRegiPointsSol[4] = 0;
		tempRegiPointsSol[5] = 0;
		m_pRigidPW->runRegistration(tempRegiPointsSol, FRET);
		if (FRET < minFRET)
		{
			minFRET = FRET;
			m_RegiPointsSol[0] = tempRegiPointsSol[0];
			m_RegiPointsSol[1] = tempRegiPointsSol[1];
			m_RegiPointsSol[2] = tempRegiPointsSol[2];
			m_RegiPointsSol[3] = tempRegiPointsSol[3];
			m_RegiPointsSol[4] = tempRegiPointsSol[4];
			m_RegiPointsSol[5] = tempRegiPointsSol[5];
		}
		logger->PrintDebugMode("CW3Registration::runRegiPoints2",
			"========== top-" + std::to_string(i) +
			" initZ : " + std::to_string(zPos) +
			", FRET : " + std::to_string(FRET));
	}

	m_pRigidPW->clearResources();

	printf("Done RegiPoints\n");
	logger->PrintDebugMode("CW3Registration::runRegiPoints2", "Done RegiPoints");
	logger->PrintDebugMode("CW3Registration::runRegiPoints2",
		"========== min FRET : " + std::to_string(minFRET));
	printRegiPoints("CW3Registration::runRegiPoints2");

	return m_RegiPointsSol;
}

/**=================================================================================================
Private Functions
*===============================================================================================**/

void CW3Registration::runRegistration()
{
	m_pRigidPW->setVolume(m_pgVol);

	float FRET = 0.0f;
	if (m_pRigidPW->runRegistration(m_ReOriSol, FRET))
	{
		m_ReOriSol[1] *= m_pRigidPW->getDownFactor();
	}
}

bool CW3Registration::isRunReorientation()
{
	if (m_ReOriSol[0] == 0.0f && m_ReOriSol[1] == 0.0f && m_ReOriSol[2] == 0.0f)
		return false;
	else
		return true;
}

void CW3Registration::initSolPoints()
{
	initSolRegiPoints();

	m_ReOriSol[0] = 0.0f;
	m_ReOriSol[1] = 0.0f;
	m_ReOriSol[2] = 0.0f;
}

void CW3Registration::initSolRegiPoints()
{
	m_RegiPointsSol[0] = 0.0f;
	m_RegiPointsSol[1] = 0.0f;
	m_RegiPointsSol[2] = 0.0f;
	m_RegiPointsSol[3] = 0.0f;
	m_RegiPointsSol[4] = 0.0f;
	m_RegiPointsSol[5] = 0.0f;
}

void CW3Registration::printRegiPoints(const char* strFuncName)
{
	common::Logger::instance()->PrintDebugMode(strFuncName,
		"Xtrans: " + std::to_string(m_RegiPointsSol[1]) +
		"Ytrans: " + std::to_string(m_RegiPointsSol[2]) +
		"Ztrans: " + std::to_string(m_RegiPointsSol[4]) +
		"Xrot: " + std::to_string(m_RegiPointsSol[3]) +
		"Yrot: " + std::to_string(m_RegiPointsSol[5]) +
		"Zrot: " + std::to_string(m_RegiPointsSol[0]));
}
