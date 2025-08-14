#include "W3Face.h"
#include <ctime>
#include <QtConcurrent/QtConcurrent>

#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"

#include "../../Core/Surfacing/MarchingCube.h"

#include "../../Module/VREngine/W3VREngine.h"
#include "../../Module/Registration/W3Registration.h"

CW3Face::CW3Face(CW3VREngine *VREngine, CLplatform* clPlatformGL, CW3ResourceContainer *Rcontainer)
	: m_pgVREngine(VREngine),
	m_pgRcontainer(Rcontainer)
{
	m_pRegiMod = new CW3Registration(clPlatformGL, EPRO::POINTMATCHING);
	m_RegiPointsModel = glm::mat4(1.0f);
}

CW3Face::~CW3Face()
{
	SAFE_DELETE_OBJECT(m_pMCsurface);
	SAFE_DELETE_OBJECT(m_pRegiMod);
}

void CW3Face::setVolume(CW3Image3D * vol)
{
	m_pgVol = vol;
	m_pRegiMod->setVolume(vol);
}

void CW3Face::runSurfacing(float minValue, int down, bool isNormalNeeded)
{
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

	QFutureWatcher<void> watcher;
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

	QFuture<void> future;
	future = QtConcurrent::run(this, &CW3Face::runMarchingCube, minValue, down, isNormalNeeded);

	watcher.setFuture(future);
	progress->exec();

	watcher.waitForFinished();

	if (m_pMCsurface)
		SAFE_DELETE_OBJECT(m_pMCsurface);

	m_pMCsurface = new CW3TRDsurface(m_pgVREngine->getGLWidget());
	m_pMCsurface->setPoints(&m_MCpoints[0]);
	m_pMCsurface->setIndices((unsigned int *)(&m_MCindices[0]));
	m_pMCsurface->setNpoints(m_MCpoints.size());
	m_pMCsurface->setNindices(m_MCindices.size() * 3);
	m_pMCsurface->setVBO();

	m_pgRcontainer->setFaceMC3D(m_pMCsurface);

	printf("MARCHING CUBE DONE %f\n", minValue);
}

void CW3Face::runMarchingCube(float minValue, int down, bool isNormalNeeded)
{
	printf("start runMarchingCube\r\n");

	m_MCpoints.clear();
	m_MCindices.clear();
	m_MCnormals.clear();

	std::vector<std::vector<int>> MCindices;

	if (isNormalNeeded){
		MarchingCube::execute(m_MCpoints, MCindices, minValue,
			m_pgVol->getData(), m_pgVol->width(), m_pgVol->height(), m_pgVol->depth(),
			down, &m_MCnormals);
	}
	else{
		MarchingCube::execute(m_MCpoints, MCindices, minValue,
			m_pgVol->getData(), m_pgVol->width(), m_pgVol->height(), m_pgVol->depth(),
			down, nullptr);
	}

	for (int i = 0; i < MCindices.size(); i++)
	{
		glm::u32vec3 triangle;
		triangle.x = MCindices.at(i)[0];
		triangle.y = MCindices.at(i)[1];
		triangle.z = MCindices.at(i)[2];

		m_MCindices.push_back(triangle);
	}

	printf("end runMarchingCube : points %d, indices %d\r\n", (int)m_MCpoints.size(), (int)m_MCindices.size());
}

glm::mat4* CW3Face::runRegiPoints2()
{
	printf("run Registration\r\n");
	clock_t start = clock();
	float* regiSolPts = m_pRegiMod->runPointMatching(
		m_pgVREngine,
		m_pgRcontainer->getFacePhoto3D(),
		m_pgRcontainer->getFaceMC3D());

	clock_t end = clock();
	long elapsedTime = end - start;
	printf("run Registration : %d ms\r\n", (int)elapsedTime);

	m_RegiPointsModel =
		glm::rotate(regiSolPts[3], glm::vec3(1.0f, 0.0f, 0.0f)) *
		glm::rotate(regiSolPts[4], glm::vec3(0.0f, 1.0f, 0.0f)) *
		glm::rotate(regiSolPts[5], glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::translate(glm::vec3(regiSolPts[2], regiSolPts[1], regiSolPts[0]));

	return &m_RegiPointsModel;
}

glm::mat4* CW3Face::runRegiPoints2OnlyTRD()
{
	float* regisolPts = m_pRegiMod->solutionPoints();
	m_RegiPointsModel =
		glm::rotate(regisolPts[5], glm::vec3(0.0f, 1.0f, 0.0f))*
		glm::rotate(regisolPts[4], glm::vec3(1.0f, 0.0f, 0.0f))*
		glm::rotate(regisolPts[3], glm::vec3(0.0f, 0.0f, 1.0f))*
		glm::translate(glm::vec3(regisolPts[1], regisolPts[2], regisolPts[0]));

	return &m_RegiPointsModel;
}
