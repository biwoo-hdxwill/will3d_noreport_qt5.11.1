#include "W3VTOSTO.h"

#include <vector>
#include <math.h>
#include <string>

#include <QtConcurrent/QtConcurrent>
#include <QApplication>

#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/W3ProgressDialog.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Resource/Resource/W3TRDsurface.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/ResContainer/W3ResourceContainer.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/face_photo_resource.h"
#include "../../Resource/Resource/W3TF.h"

#include <Util/MeshGenerator/MeshCompacter.h>
#include <Util/Core/Logger.hpp>
#if defined(_WIN32)
//#include <MarchingCube_v3/MarchingCube.h>
#endif
#include <MarchingCube_v3/MeshSimplification.h>
#include <MarchingCube_v3/MeshSimplificationCGAL.h>
#include <TetrahedronGenerator/TetrahedronGenerator.h>
#include <TextureMapper_v2/TextureMapperLevy.h>

#include <MeshMove3d_v2/MeshMove3d.h>
#include <MeshMove3d_v2/DisplacementSurface2.h>
#include <MeshMove3d_v2/DisplacementField.h>

#include "../../Core/Surfacing/MarchingCube.h"
#include "../../Core/Surfacing/W3MeshSimplifier.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_vtosto.h"
#include "../../Core/W3ProjectIO/datatypes.h"
#endif
#include "../../Module/VREngine/W3VREngine.h"
#include "../../../Managers/JobMgr/W3Jobmgr.h"

using std::runtime_error;
using std::cout;
using std::endl;

CW3VTOSTO::CW3VTOSTO(CW3VREngine* VREngine, CW3JobMgr *FACEJobMgr, CW3ResourceContainer* Rcontainer)
	: m_pgVREngine(VREngine), m_pgJobMgr(FACEJobMgr), m_pgRContainer(Rcontainer) {
	connect(m_pgJobMgr, SIGNAL(sigPointModelFromFaceTab(glm::mat4*)),
			this, SLOT(slotTransformedPhotoPoints(glm::mat4*)));
	m_pResFace.reset(new FacePhotoResource());
	ResourceContainer::GetInstance()->SetFacePhotoResource(m_pResFace);
}

CW3VTOSTO::~CW3VTOSTO() {
	SAFE_DELETE_OBJECT(m_pFacePhoto3D);
}
#ifndef WILL3D_VIEWER
void CW3VTOSTO::exportProject(ProjectIOVTOSTO& out) {
	out.InitVTOSTO();
	out.SaveIsoValue(m_isoValue);

	if (!m_headPoints.empty()) {
		out.SaveHeadPoints(m_headPoints);
		out.SaveHeadTriIndices(m_headTriangles);
	}

	if (!m_pResFace->points().empty()) {
		out.SaveFacePoints(m_pResFace->points());
		out.SaveFacePointsAfter(m_pResFace->points_after());
		out.SaveFaceIndices(m_pResFace->indices());
		out.SaveFaceTexCoords(m_pResFace->tex_coords());
	}

	if (!m_modelPoints.empty()) {
		out.SaveModelPoints(m_modelPoints);
		out.SaveModelTetraIndices(m_modelTetras);
		out.SaveModelTriIndices(m_modelTriangles);
		out.SaveModelTetraMoveResult(m_tetraMoveResult);
	}
	out.SaveModelPhotoToSurface(m_modelPhotoToSurface);

	// TRD save
	if (m_trdFilePath.length() > 0) {
		std::string trd_path = m_trdFilePath.toLocal8Bit().toStdString();
		out.SaveTRDPath(trd_path);
		FILE *FREAD;
		if (fopen_s(&FREAD, trd_path.c_str(), "rb")) {
			common::Logger::instance()->Print(common::LogType::ERR,
											  "3D Photo File open is failed");
			return;
		}

		unsigned int Npoints, Nindices, nsTex, ntTex;
		fread(&Npoints, sizeof(unsigned int), 1, FREAD);
		fread(&Nindices, sizeof(unsigned int), 1, FREAD);
		fread(&nsTex, sizeof(unsigned int), 1, FREAD);
		fread(&ntTex, sizeof(unsigned int), 1, FREAD);

		glm::vec3 *pPoints = new glm::vec3[Npoints];
		glm::vec3 *pNormals = new glm::vec3[Npoints];
		glm::vec2 *pTexCoord = new glm::vec2[Npoints];
		unsigned int *pIndices = new unsigned int[Nindices];
		unsigned char *pTexImage = new unsigned char[nsTex * ntTex * 3];

		fread(pPoints, sizeof(glm::vec3), Npoints, FREAD);
		fread(pNormals, sizeof(glm::vec3), Npoints, FREAD);
		fread(pTexCoord, sizeof(glm::vec2), Npoints, FREAD);
		fread(pIndices, sizeof(unsigned int), Nindices, FREAD);
		fread(pTexImage, sizeof(unsigned char), nsTex * ntTex * 3, FREAD);

		out.SaveTRDPoints(pPoints, Npoints);
		out.SaveTRDNormals(pNormals, Npoints);
		out.SaveTRDTexCoords(pTexCoord, Npoints);
		out.SaveTRDIndices(pIndices, Nindices);
		out.SaveTRDTexImage(pTexImage, nsTex, ntTex);

		delete[] pPoints;
		delete[] pNormals;
		delete[] pTexCoord;
		delete[] pIndices;
		delete[] pTexImage;
		fclose(FREAD);
	}

	project::VTOSTOFlags vtosto_flags;
	vtosto_flags.is_set_isovalue = flag.setIsoValue;
	vtosto_flags.is_generate_head = flag.generateHead;
	vtosto_flags.is_make_tetra = flag.makeTetra;
	vtosto_flags.is_fixed_isovalue_in_surgery = flag.fixedIsoValueInSurgery;
	vtosto_flags.is_landmark = flag.landmark;
	vtosto_flags.is_cut_face = flag.cutFace;
	vtosto_flags.is_do_mapping = flag.doMapping;
	vtosto_flags.is_calc_disp = flag.calcDisp;
	vtosto_flags.is_make_surf = flag.makeSurf;
	vtosto_flags.is_make_field = flag.makeField;
	vtosto_flags.is_load_TRD = flag.loadTRD;
	out.SaveVTOSTOFlags(vtosto_flags);
}

void CW3VTOSTO::importProject(ProjectIOVTOSTO& in) {
	if (!in.IsInit())
		return;

	in.LoadIsoValue(m_isoValue);
	setFixedIsoValue(m_isoValue);

	in.LoadHeadPoints(m_headPoints);
	in.LoadHeadTriIndices(m_headTriangles);

	std::vector<glm::vec3> face_points;
	in.LoadFacePoints(face_points);
	m_pResFace->set_points(face_points);

	face_points.clear();
	in.LoadFacePointsAfter(face_points);
	m_pResFace->set_points_after(face_points);

	std::vector<unsigned int> face_indices;
	in.LoadFaceIndices(face_indices);
	m_pResFace->set_indices(face_indices);

	std::vector<glm::vec2> tex_coords;
	in.LoadFaceTexCoords(tex_coords);
	m_pResFace->set_tex_coords(tex_coords);

	in.LoadModelPoints(m_modelPoints);
	in.LoadModelTetraIndices(m_modelTetras);
	in.LoadModelTriIndices(m_modelTriangles);
	in.LoadModelTetraMoveResult(m_tetraMoveResult);
	in.LoadModelPhotoToSurface(m_modelPhotoToSurface);

	std::string trd_path;
	in.LoadTRDPath(trd_path);
	if (trd_path.size() > 0) {
		m_trdFilePath = QString(trd_path.c_str());
		m_bLoadFaceProject = true;

		QFileInfo fileInfo(m_trdFilePath);
		QDir dir = fileInfo.absoluteDir();
		if (!dir.exists())
			dir.mkpath(dir.absolutePath());

		std::vector<glm::vec3> points;
		in.LoadTRDPoints(points);

		std::vector<glm::vec3> normals;
		in.LoadTRDNormals(normals);

		std::vector<glm::vec2> tex_coords;
		in.LoadTRDTexCoords(tex_coords);

		std::vector<unsigned int> indices;
		in.LoadTRDIndices(indices);

		std::vector<unsigned char> image;
		unsigned int tex_w, tex_h;
		in.LoadTRDTexImage(image, tex_w, tex_h);
		
		saveTRD(m_trdFilePath, points, normals, tex_coords,
				indices, &image[0], tex_w, tex_h);
		slotLoadTRD(m_trdFilePath);
	}

	project::VTOSTOFlags vtosto_flags;
	in.LoadVTOSTOFlags(vtosto_flags);
	flag.setIsoValue = vtosto_flags.is_set_isovalue;
	flag.generateHead = vtosto_flags.is_generate_head;
	flag.makeTetra = vtosto_flags.is_make_tetra;
	flag.fixedIsoValueInSurgery = vtosto_flags.is_fixed_isovalue_in_surgery;
	flag.landmark = vtosto_flags.is_landmark;
	flag.cutFace = vtosto_flags.is_cut_face;
	flag.doMapping = vtosto_flags.is_do_mapping;
	flag.calcDisp = vtosto_flags.is_calc_disp;
	flag.makeSurf = vtosto_flags.is_make_surf;
	flag.makeField = vtosto_flags.is_make_field;	
	flag.loadTRD = vtosto_flags.is_load_TRD;
}
#endif
void CW3VTOSTO::reset() {
	m_headPoints.clear();
	m_headTriangles.clear();
	m_modelPoints.clear();
	m_modelTetras.clear();
	m_modelTriangles.clear();

#if 0
	MeshMove3d *meshMove = m_meshMove.get();
	SAFE_DELETE_OBJECT(meshMove);
	m_meshMove = nullptr;
	DisplacementSurface *dispSurf = m_dispSurf.get();
	SAFE_DELETE_OBJECT(dispSurf);
	m_dispSurf = nullptr;
	DisplacementField *dispField = m_dispField.get();
	SAFE_DELETE_OBJECT(dispField);
	m_dispField = nullptr;
#else
	m_meshMove.reset();
	m_meshMove = nullptr;
	m_dispSurf.reset();
	m_dispSurf = nullptr;
	m_dispField.reset();
	m_dispField = nullptr;
#endif

	m_tetraMoveResult.clear();
	m_photoFilePath = "";
	m_trdFilePath = "";

	flag.setIsoValue = 0;
	flag.fixedIsoValueInSurgery = 0;
	flag.generateHead = 0;
	flag.makeTetra = 0;
	flag.makeMeshMove = 0;
	flag.landmark = 0;
	flag.cutFace = 0;
	flag.doMapping = 0;
	flag.calcDisp = 0;
	flag.makeSurf = 0;
	flag.makeField = 0;
	flag.loadTRD = 0;

	m_isoValue = -1;
	SAFE_DELETE_OBJECT(m_pFacePhoto3D);
}

void CW3VTOSTO::slotLoadTRD(const QString& file) {
	try {
		m_trdFilePath = file;

		SAFE_DELETE_OBJECT(m_pFacePhoto3D);
		m_pFacePhoto3D = new CW3TRDsurface(m_pgVREngine->getGLWidget());

		if (m_pgVREngine->getVol(0) != nullptr) {
			CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

			QFutureWatcher<void> watcher;
			connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

			auto future = QtConcurrent::run(m_pFacePhoto3D, &CW3TRDsurface::trdLoad, &file, m_pgVREngine->getVol(0));
			watcher.setFuture(future);
			progress->exec();
			watcher.waitForFinished();

			m_pFacePhoto3D->setVBO();

			if (!flag.setIsoValue)
				throw std::runtime_error("isovalue is empty");

			m_pgRContainer->setFacePhoto3D(m_pFacePhoto3D);
			m_pResFace->set_tex_handler(m_pFacePhoto3D->getTexHandler());

			common::Logger::instance()->PrintDebugMode("CW3VTOSTO::slotLoadTRD",
													   "runSurfaceing value = " + std::to_string(m_isoValue));

			if (!m_bLoadFaceProject) {
				int nx = m_pgVREngine->getVol(0)->width();
				int ny = m_pgVREngine->getVol(0)->height();
				int nz = m_pgVREngine->getVol(0)->depth();
				float dsf = std::max(float(std::max(nx, ny)) / 160.f, 1.0f);
#ifdef MC
				((CW3JobMgr *)m_pgFACEJobMgr)->runSurfacing(m_isoValue, dsf, true);
				((CW3JobMgr *)m_pgFACEJobMgr)->runRegiPoints2();
#else
				CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_21(), CW3MessageBox::ActionRole);
				msgBox.show();
				msgBox.raise();
				QApplication::processEvents();

				m_pgJobMgr->runRegiPoints2();

				msgBox.hide();

				/*auto future = QtConcurrent::run((CW3FACEJobMgr *)m_pgFACEJobMgr, &CW3FACEJobMgr::runRegiPoints2);

				watcher.setFuture(future);
				progress->exec();

				watcher.waitForFinished();*/
#endif
			} else {
				m_bLoadFaceProject = false;
				depChainFixedIsoValueInSurgery();

				flag.loadTRD = true;

				emit sigSetPhoto();
				emit m_pgRContainer->sigInitFacePhoto3D();

				emit sigPointModelLoadProject(&m_modelPhotoToSurface);
				emit sigUpdateMPRPhotoLoadProject();

				//emit sigFaceDisabled(false);
			}
			//return;
		}
	} catch (std::runtime_error& e) {
		cout << "CW3VTOSTO::slotLoadTRD: " << e.what() << endl;
	}
}

void CW3VTOSTO::slotLoadOnlyTRD(const QString& file) {
	try {
		m_trdFilePath = file;

		SAFE_DELETE_OBJECT(m_pFacePhoto3D);
		m_pFacePhoto3D = new CW3TRDsurface(m_pgVREngine->getGLWidget());

		CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);

		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(m_pFacePhoto3D, &CW3TRDsurface::trdLoad, &file, nullptr);
		watcher.setFuture(future);
		progress->exec();

		watcher.waitForFinished();

		m_pFacePhoto3D->setVBO();

		m_pgRContainer->setFacePhoto3D(m_pFacePhoto3D);

		m_pgJobMgr->runRegiPoints2OnlyTRD();
	} catch (std::runtime_error& e) {
		cout << "CW3VTOSTO::slotLoadTRD: " << e.what() << endl;
	}
}

void CW3VTOSTO::slotTransformedPhotoPoints(glm::mat4* regiMat) {
	if (!m_pgVREngine)
		return;

	if (!m_pgVREngine->getVol(0))
		return;

	m_modelPhotoToSurface = *regiMat;

	vec3 volScale = glm::vec3(
		m_pgVREngine->getVol(0)->width(),
		m_pgVREngine->getVol(0)->height(),
		m_pgVREngine->getVol(0)->depth());

	glm::mat4 scale = glm::scale(volScale);
	glm::mat4 rMat = glm::inverse(scale)*(m_modelPhotoToSurface)*scale;
	const auto& facePoints = m_pFacePhoto3D->getPoints();

	for (auto & elem : facePoints) {
		m_pResFace->AddPoint(vec3(rMat*vec4(elem, 1.0f)));
	}

	m_pResFace->set_points_after(m_pResFace->points());

	m_pResFace->set_indices(m_pFacePhoto3D->getIndices());
	m_pResFace->set_tex_coords(m_pFacePhoto3D->getTexCoord());
	m_pResFace->set_tex_handler(m_pFacePhoto3D->getTexHandler());

	depChainFixedIsoValueInSurgery();

	flag.loadTRD = true;

	// log-out m_modelPhotoToSurface value
	std::string regiMatOutLog = "TransformedPhotoPoints succeeded : ";
	for (int columnIdx = 0; columnIdx < 4; columnIdx++) {
		glm::vec4 curColVec = glm::vec4(m_modelPhotoToSurface[columnIdx]);
		regiMatOutLog += std::to_string(curColVec.x) + "," + std::to_string(curColVec.y) + "," + std::to_string(curColVec.z) + "," + std::to_string(curColVec.w) + ",";
	}
	common::Logger::instance()->Print(common::LogType::INF, regiMatOutLog);

	cout << "CW3VTOSTO::slotTransformedPhotoPoints done" << endl;
	cout << "===================================" << endl;

	emit sigSetPhoto();
	emit m_pgRContainer->sigInitFacePhoto3D();
}

//isoValue 강제로 0.5f 더하게 했음. 나중에 수정하기.
bool CW3VTOSTO::setIsoValue(float isoValue) {
	m_isoValue = static_cast<int>(isoValue) + 0.5f;
	//m_isoValue = isoValue;
	depChainSetIsoValue();
	return true;
}

//isoValue 강제로 0.5f 더하게 했음. 나중에 수정하기.
bool CW3VTOSTO::setFixedIsoValue(float isoValue) {
	m_isoValue = static_cast<int>(isoValue) + 0.5f;
	//m_isoValue = isoValue;
	depChainFixedIsoValueInSurgery();
	emit sigSetFixedIsoValue(isoValue);
	return true;
}

bool CW3VTOSTO::genHeadAndMkTetra() {
	return (generateHead() && makeTetra()) ? true : false;
}

bool CW3VTOSTO::generateHead() {
	try {
		lg << "generateHead: isoValue=" << m_isoValue << endl;
		if (!flag.setIsoValue) {
			throw std::runtime_error("flag.setIsoValue = false");
		}

		/* get uint16 volume */
		int nx = m_pgVREngine->getVol(0)->width();
		int ny = m_pgVREngine->getVol(0)->height();
		int nz = m_pgVREngine->getVol(0)->depth();

		float dsf = std::max(float(std::max(nx, ny)) / 160.f, 1.0f);

		//std::vector<glm::u32vec3> headTriangle;
		MarchingCube::execute(
			m_headPoints, m_headTriangles, m_isoValue,
			m_pgVREngine->getVol(0)->getData(),
			nx, ny, nz, dsf, nullptr);

		// Select max ccl element
		//CW3MeshSimplifier::execute_IsolatedComponentSelectMax(m_headPoints, m_headTriangles);

		depChainGenerateHead();

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::generateHead: " << e << endl;
		return false;
	}
}

bool CW3VTOSTO::cutFace(const glm::vec4& plane) {
	try {
		lg << "cutFace" << endl;
		if (!flag.generateHead) {
			throw std::runtime_error("flag.generateHead = false");
		}

		///* cut face */

		std::vector<glm::vec3> headPoints = m_headPoints;
		std::vector<std::vector<int>> headTriangles = m_headTriangles;
		MeshCompacter::executeRemoveMeshByVertexCriteria(
			headPoints, headTriangles,
			std::function<int(int)>(),
			[this, &plane, headPoints](int i)->bool {
			const auto& p = headPoints[i];
			/* only use ax + by + cz + d >= 0*/
			if (p.x * plane.x + p.y * plane.y + p.z * plane.z + plane.w < 0) {
				return true;
			} else
				return false;
		}
		);

		/* simplification */
		if (!flag.cutFace) {
			CW3MeshSimplifier::execute_CGAL(headPoints, headTriangles, 0.2f);
		}

		m_pResFace->set_points(headPoints);
		m_pResFace->set_points_after(headPoints);

		std::vector<unsigned int> indices;
		for (int i = 0; i < headTriangles.size(); i++) {
			indices.push_back(static_cast<unsigned int>(headTriangles[i][0]));
			indices.push_back(static_cast<unsigned int>(headTriangles[i][1]));
			indices.push_back(static_cast<unsigned int>(headTriangles[i][2]));
		}

		m_pResFace->set_indices(indices);

		depChainCutFace();
		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::cutFace: " << e << endl;
		return false;
	}
}

bool CW3VTOSTO::doMappingRunThread(
	//std::vector<glm::vec2>& tex2dCoordsResult,
	const std::vector<glm::vec3>& meshCtrlPoints,
	const std::vector<int>& meshCtrlTriangles,
	const std::vector<glm::vec2>& texCtrlPoints) {
	try {
		if (!flag.cutFace) {
			throw std::runtime_error("flag.cutFace = false");
		}

		const auto& facePoints = m_pResFace->points();
		const auto& indices = m_pResFace->indices();
		std::vector<std::vector<int>> faceTriangles;
		std::vector<int> tri(3);
		for (int i = 0; i < indices.size() / 3; i++) {
			tri[0] = indices[3 * i + 0];
			tri[1] = indices[3 * i + 1];
			tri[2] = indices[3 * i + 2];
			faceTriangles.push_back(tri);
		}

		std::vector<glm::vec2> tex2dCoordsResult;

		bool changed = m_pResFace->points().size() != m_pResFace->tex_coords().size();
		if (changed) {
			TextureMapperLevy tm(facePoints, faceTriangles,
								 meshCtrlPoints, meshCtrlTriangles, texCtrlPoints);

			tm.solve2002(tex2dCoordsResult, 10.f, 1e-6, 4000);
		} else {
			TextureMapperLevy tm(facePoints, faceTriangles,
								 meshCtrlPoints, meshCtrlTriangles, texCtrlPoints);

			tm.solve2002(tex2dCoordsResult, 10.f, 1e-6, 4000);
		}

		m_pResFace->set_tex_coords(tex2dCoordsResult);

		depChainDoMapping();

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::doMapping: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::doMappingFinal(const QImage& photoImage) {
	if (photoImage.isNull())
		return false;
	try {
		const auto& facePoints = m_pResFace->points();
		const auto& faceIndices = m_pResFace->indices();
		const auto& texCoords = m_pResFace->tex_coords();

		std::vector<glm::vec3> normals;
		CW3ElementGenerator::GenerateSmoothNormals(facePoints, faceIndices, normals);

		SAFE_DELETE_OBJECT(m_pFacePhoto3D);
		m_pFacePhoto3D = new CW3TRDsurface(m_pgVREngine->getGLWidget());

		if (m_pFacePhoto3D->loadPassiveSurface(facePoints, normals, faceIndices, texCoords, photoImage)) {
			m_pgRContainer->setFacePhoto3D(m_pFacePhoto3D);
			m_pResFace->set_tex_handler(m_pFacePhoto3D->getTexHandler());
			emit m_pgRContainer->sigInitFacePhoto3D();
			emit sigSetPhoto();
		} else {
			throw std::runtime_error("loadPassiveSurface. return false.");
		}

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::doMapping: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::makeTetra() {
	try {
		lg << "makeTetra" << endl;

		if (!flag.generateHead) {
			throw std::runtime_error("flag.generateHead = false");
		}
		/* tetra gen */
		TetrahedronGenerator tg;
		tg.facetAngle = 25;
		tg.facetSize = 0.1;
		tg.facetDistance = 0.1;
		tg.cellSize = 0.1;
		tg.cellRadiusEdgeRatio = 2;
		tg.generate_c3t3(m_modelPoints, m_modelTetras, m_modelTriangles, m_headPoints, m_headTriangles);
		depChainMakeTetra();
		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::makeTetra: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::makeMeshMove(const std::vector<int>& jointIdxs, float E, float v) {
	try {
		lg << "makeMeshMove" << endl;

		if (!flag.makeTetra) {
			throw std::runtime_error("flag.makeTetra = false");
		}
		m_meshMove.reset(new MeshMove3d(m_modelPoints, jointIdxs, m_modelTetras, E, v));
		depChainMakeMeshMove();

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::makeMeshMove: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::calcDisp(const std::vector<glm::vec3>& jointDisps) {
	try {
		lg << "calcDisp" << endl;

		if (!flag.makeMeshMove)
			throw std::runtime_error("flag.makeMeshMove = false");

		if (flag.calcDisp) {
			m_meshMove->execute(m_tetraMoveResult, jointDisps, m_tetraMoveResult);
		} else {
			m_meshMove->execute(m_tetraMoveResult, jointDisps, std::vector<glm::vec3>(m_modelPoints.size(), glm::vec3(0, 0, 0)));
		}
		depChainCalcDisp();
		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::calcDisp: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::makeSurf() {
	// Drawable 안쓰는 DisplacementSurface2 버젼임 : 머징시 지우지 않도록 주의할것.
	//

	lg << "makeSurf" << endl;

	if (!flag.calcDisp) {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "CW3VTOSTO::makeSurf: flag.calcDisp = false");
		return false;
	}
	m_dispSurf.reset(new DisplacementSurface2(m_pgVREngine->getGLWidget(), m_modelPoints,
											  m_modelTriangles, m_tetraMoveResult));

	depChainMakeSurf();

	common::Logger::instance()->Print(common::LogType::DBG, "CW3VTOSTO::makeSurf()");

	return true;
}

bool CW3VTOSTO::makeField() {
	try {
		lg << "makeField" << endl;

		if (!flag.calcDisp)
			throw std::runtime_error("flag.calcDisp = false");

		lg.pushColor(rlutil::LIGHTGREEN);  lg << "makeField" << endl; lg.popColor();

		m_dispField.reset(new DisplacementField(m_modelPoints, m_modelTetras, m_tetraMoveResult));
		depChainMakeField();
		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::makeField: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::executeSurf() {
	try {
		lg << "executeSurf" << endl;

		if (!flag.makeSurf)
			throw std::runtime_error("flag.makeSurf = false");

		std::vector<glm::vec3> disps;
		const auto& facePoints = m_pResFace->points();
		m_dispSurf->execute(disps, facePoints);

		std::vector<glm::vec3> faceAfter(facePoints.size());

		for (int i = 0; i < facePoints.size(); i++) {
			faceAfter[i] = disps[i] + facePoints[i];
		}

		m_pResFace->set_points_after(faceAfter);

		emit sigChangeFaceAfterSurface();

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::executeSurf: " << e << endl;

		return false;
	}
}

bool CW3VTOSTO::executeField(std::vector<glm::vec3>& queryPoints) {
	try {
		lg << "executeField" << endl;

		if (!flag.makeField)
			throw std::runtime_error("flag.makeField = false");

		std::vector<glm::vec3> disps;
		m_dispField->execute(disps, queryPoints);
		for (int i = 0; i < queryPoints.size(); i++)
			queryPoints[i] = disps[i] + queryPoints[i];

		return true;
	} catch (std::runtime_error& e) {
		lg << "CW3VTOSTO::executeField: " << e << endl;

		return false;
	}
}

//std::vector<glm::vec3> CW3VTOSTO::executeField(const std::vector<glm::vec3>& queryPoints) {
//	try {
//		lg << "executeField" << endl;
//		if (!flag.makeField) {
//			throw std::runtime_error("flag.makeField = false");
//		}
//		vector<glm::vec3> disps;
//		m_dispField->execute(disps, queryPoints);
//		return disps;
//	}
//	catch (std::runtime_error& e) {
//		lg << "CW3VTOSTO::executeField: " << e << endl;
//		return vector<glm::vec3>();
//	}
//}

void CW3VTOSTO::depChainSetIsoValue() {
	lg << "depChainSetIsoValue" << endl;

	flag.setIsoValue = true;
	flag.generateHead = false;
	flag.makeTetra = false;
	flag.makeMeshMove = false;
	flag.calcDisp = false;
	flag.makeSurf = false;
	flag.makeField = false;
	flag.cutFace = false;
	flag.doMapping = false;
}

void CW3VTOSTO::depChainFixedIsoValueInSurgery() {
	lg << "depChainSetIsoValue" << endl;

	flag.setIsoValue = true;
	flag.fixedIsoValueInSurgery = true;
	flag.generateHead = false;
	flag.makeTetra = false;
	flag.makeMeshMove = false;
	flag.calcDisp = false;
	flag.makeSurf = false;
	flag.makeField = false;
	flag.cutFace = false;
	flag.doMapping = false;
}

void CW3VTOSTO::depChainGenerateHead() {
	lg << "depChainGenerateHead" << endl;

	flag.generateHead = true;
	flag.makeTetra = false;
	flag.makeMeshMove = false;
	flag.calcDisp = false;
	flag.makeSurf = false;
	flag.makeField = false;
	flag.cutFace = false;
	flag.doMapping = false;
}

void CW3VTOSTO::depChainMakeTetra() {
	lg << "depChainMakeTetra" << endl;

	flag.makeTetra = true;
	flag.makeMeshMove = false;
	flag.calcDisp = false;
	flag.makeSurf = false;
	flag.makeField = false;
}

void CW3VTOSTO::depChainMakeMeshMove() {
	lg << "depChainMakeMeshMove" << endl;

	flag.makeMeshMove = true;
	flag.calcDisp = false;
	flag.makeSurf = false;
	flag.makeField = false;
}

void CW3VTOSTO::depChainCalcDisp() {
	lg << "depChainCalcDisp" << endl;

	flag.calcDisp = true;
	flag.makeSurf = false;
	flag.makeField = false;
}

void CW3VTOSTO::depChainMakeSurf() {
	lg << "depChainMakeSurf" << endl;

	flag.makeSurf = true;
}

void CW3VTOSTO::depChainMakeField() {
	lg << "depChainMakeField" << endl;

	flag.makeField = true;
}

void CW3VTOSTO::depChainCutFace() {
	lg << "depChainCutFace" << endl;

	flag.cutFace = true;
	flag.doMapping = false;
}

void CW3VTOSTO::depChainDoMapping() {
	lg << "depChainDoMapping" << endl;

	flag.doMapping = true;
}

int CW3VTOSTO::getFacetexWidth() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getTexWidth() : -1;
}

int CW3VTOSTO::getFacetexHeight() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getTexHeight() : -1;
}

unsigned char* CW3VTOSTO::getFacetexData() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getTexData() : nullptr;
}

unsigned int CW3VTOSTO::getFaceNindices() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getNindices() : -1;
}

unsigned int *CW3VTOSTO::getFaceVBO() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getVBO() : nullptr;
}

unsigned int CW3VTOSTO::getFaceTexHandler() {
	return m_pFacePhoto3D ? m_pFacePhoto3D->getTexHandler() : -1;
}

void CW3VTOSTO::saveTRD(const QString &path) {
	if (!m_pFacePhoto3D)
		return;

	std::vector<glm::vec3> points = m_pResFace->points_after();
	std::vector<glm::vec3> normals = m_pFacePhoto3D->getNormals();
	std::vector<glm::vec2> texCoord = m_pFacePhoto3D->getTexCoord();
	std::vector<unsigned int> indices = m_pFacePhoto3D->getIndices();
	unsigned char *texImage = m_pFacePhoto3D->getTexData();
	unsigned int nsTex = m_pFacePhoto3D->getTexWidth();
	unsigned int ntTex = m_pFacePhoto3D->getTexHeight();

	CW3Image3D *vol = m_pgVREngine->getVol(0);
	float xlength = (vol->width()) * 0.5f * vol->pixelSpacing();
	float ylength = (vol->height()) * 0.5f * vol->pixelSpacing();
	float zlength = (vol->depth()) * 0.5f * vol->sliceSpacing();
	for (int i = 0; i < points.size(); i++) {
		points[i].x *= xlength;
		points[i].y *= ylength;
		points[i].z *= zlength;
	}

	saveTRD(path, points, normals, texCoord, indices, texImage, nsTex, ntTex);
}

void CW3VTOSTO::saveTRD(const QString &path, std::vector<glm::vec3> &points,
						std::vector<glm::vec3> &normals, std::vector<glm::vec2> &texCoord,
						std::vector<unsigned int> &indices, unsigned char *texImage,
						const int &nsTex, const int &ntTex) {
	std::string strLocal8Bit = path.toLocal8Bit().toStdString();

	FILE *f = nullptr;
	bool result = false;
#if defined(__APPLE__)
	f = fopen(strLocal8Bit.c_str(), "wb");
	result = (f) ? true : false;
#else
	result = !fopen_s(&f, strLocal8Bit.c_str(), "wb");
#endif
	if (!result)
		std::cerr << "Modified 3D Photo File open is failed" << std::endl;

	// modified data
#if 0
	std::vector<glm::vec3> points = m_facePointsAfter;
	std::vector<glm::vec3> normals = m_pFacePhoto3D->getNormals();
	std::vector<glm::vec2> texCoord = m_pFacePhoto3D->getTexCoord();
	std::vector<unsigned int> indices = m_pFacePhoto3D->getIndices();
	unsigned char *texImage = m_pFacePhoto3D->getTexData();
#endif

	unsigned int Npoints = points.size();
	unsigned int Nindices = indices.size();

	fwrite(&Npoints, sizeof(unsigned int), 1, f);
	fwrite(&Nindices, sizeof(unsigned int), 1, f);
	fwrite(&nsTex, sizeof(unsigned int), 1, f);
	fwrite(&ntTex, sizeof(unsigned int), 1, f);

	fwrite(&points[0], sizeof(glm::vec3), Npoints, f);
	fwrite(&normals[0], sizeof(glm::vec3), Npoints, f);
	fwrite(&texCoord[0], sizeof(glm::vec2), Npoints, f);
	fwrite(&indices[0], sizeof(unsigned int), Nindices, f);
	fwrite(texImage, sizeof(unsigned char), nsTex * ntTex * 3, f);
	//

	if (!m_bLoadFaceProject) {
		// flag
		int flag = 1;
		fwrite(&flag, sizeof(int), 1, f);
		//

		// original data
		std::vector<glm::vec3> pointsOrg = m_pResFace->points();
		//std::vector<glm::vec3> normalsOrg = m_pFacePhoto3D->getNormals();
		//std::vector<glm::vec2> texCoordOrg = m_pFacePhoto3D->getTexCoord();
		std::vector<unsigned int> indicesOrg = m_pFacePhoto3D->getIndices();

		unsigned int NpointsOrg = pointsOrg.size();
		unsigned int NindicesOrg = indicesOrg.size();

		CW3Image3D *vol = m_pgVREngine->getVol(0);
		float xlength = (vol->width()) * 0.5f * vol->pixelSpacing();
		float ylength = (vol->height()) * 0.5f * vol->pixelSpacing();
		float zlength = (vol->depth()) * 0.5f * vol->sliceSpacing();

		for (int i = 0; i < NpointsOrg; i++) {
			pointsOrg[i].x *= xlength;
			pointsOrg[i].y *= ylength;
			pointsOrg[i].z *= zlength;
		}

		fwrite(&NpointsOrg, sizeof(unsigned int), 1, f);
		fwrite(&NindicesOrg, sizeof(unsigned int), 1, f);

		fwrite(&pointsOrg[0], sizeof(glm::vec3), Npoints, f);
		//fwrite(&normalsOrg[0], sizeof(glm::vec3), Npoints, f);
		//fwrite(&texCoordOrg[0], sizeof(glm::vec2), Npoints, f);
		fwrite(&indicesOrg[0], sizeof(unsigned int), Nindices, f);
		//
	}

	fclose(f);
}

bool CW3VTOSTO::SavePLY(const QString& path)
{
	if (!m_pFacePhoto3D)
	{
		return false;
	}

	std::vector<glm::vec3> points = m_pResFace->points_after();
	std::vector<glm::vec3> normals = m_pFacePhoto3D->getNormals();
	std::vector<glm::vec2> tex_coords = m_pFacePhoto3D->getTexCoord();
	std::vector<unsigned int> indices = m_pFacePhoto3D->getIndices();
	unsigned char* texture= m_pFacePhoto3D->getTexData();
	unsigned int texture_ns = m_pFacePhoto3D->getTexWidth();
	unsigned int texture_nt = m_pFacePhoto3D->getTexHeight();

	CW3Image3D* volume = m_pgVREngine->getVol(0);
	float x_length = volume->width() * volume->pixelSpacing();
	float y_length = volume->height() * volume->pixelSpacing();
	float z_length = volume->depth() * volume->sliceSpacing();
	for (int i = 0; i < points.size(); i++)
	{
		points[i].x = (1.0f - (points[i].x + 1.0f) * 0.5f) * x_length;
		points[i].y = ((points[i].y + 1.0f) * 0.5f) * y_length;
		points[i].z = (1.0f - (points[i].z + 1.0f) * 0.5f) * z_length;
	}

	return SavePLY(path, points, normals, tex_coords, indices, texture, texture_ns, texture_nt);
}

bool CW3VTOSTO::SavePLY(const QString& path, std::vector<glm::vec3>& points,
	std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
	std::vector<unsigned int>& indices, unsigned char* texture,
	const int& texture_ns, const int& texture_nt)
{
	QImage img(texture_ns, texture_nt, QImage::Format_RGB888);
	for (int i = 0; i < texture_nt; i++)
	{
		for (int j = 0; j < texture_ns; j++)
		{
			int index = ((i * texture_ns) + j) * 3;
			unsigned char b = texture[index];
			unsigned char g = texture[index + 1];
			unsigned char r = texture[index + 2];
			QRgb color = qRgb(texture[index], texture[index + 1], texture[index + 2]);
			img.setPixel(j, i, color);
		}
	}

	QFile ply(path);
	if (!ply.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		return false;
	}

	int number_of_points = points.size();
	int number_of_triangles = indices.size() / 3;

	QTextStream out(&ply);
	out << "ply\n"
		<< "format ascii 1.0\n"
		<< "comment author: HDXWILL\n"
		<< "comment object: face\n"
		<< "element vertex " << number_of_points << "\n"
		<< "property float x\n"
		<< "property float y\n"
		<< "property float z\n"
		<< "property uchar red\n"
		<< "property uchar green\n"
		<< "property uchar blue\n"
		<< "element face " << number_of_triangles << "\n"
		<< "property list uchar int vertex_index\n"
		<< "end_header\n";

	for (int i = 0; i < number_of_points; i++)
	{
		QRgb rgb = img.pixel(tex_coords[i].x * texture_ns, tex_coords[i].y * texture_nt);
		out << points[i].x << " " << points[i].y << " " << points[i].z << " " << qRed(rgb) << " " << qGreen(rgb) << " " << qBlue(rgb) << "\n";
	}

	for (int i = 0; i < number_of_triangles; i++)
	{
		out << "3 " << indices[i * 3] << " " << indices[i * 3 + 1] << " " << indices[i * 3 + 2] << "\n";
	}

	ply.close();

	return true;
}

bool CW3VTOSTO::SaveOBJ(const QString& path)
{
	if (!m_pFacePhoto3D)
	{
		return false;
	}

	std::vector<glm::vec3> points = m_pResFace->points_after();
	std::vector<glm::vec3> normals = m_pFacePhoto3D->getNormals();
	std::vector<glm::vec2> tex_coords = m_pFacePhoto3D->getTexCoord();
	std::vector<unsigned int> indices = m_pFacePhoto3D->getIndices();
	unsigned char* texture = m_pFacePhoto3D->getTexData();
	unsigned int texture_ns = m_pFacePhoto3D->getTexWidth();
	unsigned int texture_nt = m_pFacePhoto3D->getTexHeight();

	CW3Image3D* volume = m_pgVREngine->getVol(0);
	float x_length = volume->width() * volume->pixelSpacing();
	float y_length = volume->height() * volume->pixelSpacing();
	float z_length = volume->depth() * volume->sliceSpacing();
	for (int i = 0; i < points.size(); i++)
	{
		points[i].x = (1.0f - (points[i].x + 1.0f) * 0.5f) * x_length;
		points[i].y = ((points[i].y + 1.0f) * 0.5f) * y_length;
		points[i].z = (1.0f - (points[i].z + 1.0f) * 0.5f) * z_length;
	}

	QFileInfo info(path);
	QString base_path = path;
	if (!info.suffix().isEmpty())
	{
		base_path = path.left(base_path.length() - (info.suffix().length() + 1));
	}

	return SaveOBJ(base_path, points, normals, tex_coords, indices, texture, texture_ns, texture_nt);
}

bool CW3VTOSTO::SaveOBJ(const QString& base_path, std::vector<glm::vec3>& points,
	std::vector<glm::vec3>& normals, std::vector<glm::vec2>& tex_coords,
	std::vector<unsigned int>& indices, unsigned char* texture,
	const int& texture_ns, const int& texture_nt)
{
	int number_of_points = points.size();
	int number_of_triangles = indices.size() / 3;

	bool include_texture = true;

	QString obj_path = base_path + ".obj";
	QString texture_path = base_path + ".png";
	QString mtl_path = base_path + ".mtl";
	QString mtl_name = "Face";

	// make .obj
	QFile obj(obj_path);
	if (!obj.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		return false;
	}

	QTextStream obj_out(&obj);
	obj_out << "# HDXWILL Will3D v1.2.0\n";
	obj_out << "mtllib " << mtl_path << "\n";

	for (int i = 0; i < number_of_points; i++)
	{
		obj_out << "v " <<
			points[i].x << " " <<
			points[i].y << " " <<
			points[i].z << "\n";
	}
	obj_out << "\n";

	if (include_texture)
	{
		for (int i = 0; i < number_of_points; i++)
		{
			obj_out << "vt " <<
				tex_coords[i].x << " " <<
				-tex_coords[i].y << "\n";
		}
		obj_out << "\n";
	}

	for (int i = 0; i < number_of_points; i++)
	{
		glm::vec3 normalized_normal = glm::normalize(normals[i]);
		obj_out << "vn " <<
			normalized_normal.x << " " <<
			normalized_normal.y << " " <<
			normalized_normal.z << "\n";
	}
	obj_out << "\n";

	obj_out << "usemtl " << mtl_name << "\n";
	obj_out << "s off\n";

	for (int i = 0; i < number_of_triangles; ++i)
	{
		int triangle_point_1 = indices[i * 3] + 1;
		int triangle_point_2 = indices[i * 3 + 1] + 1;
		int triangle_point_3 = indices[i * 3 + 2] + 1;

		if (include_texture)
		{
			obj_out << "f " <<
				triangle_point_1 << "/" << triangle_point_1 << "/" << triangle_point_1 << " " <<
				triangle_point_2 << "/" << triangle_point_2 << "/" << triangle_point_2 << " " <<
				triangle_point_3 << "/" << triangle_point_3 << "/" << triangle_point_3 << "\n";
		}
		else
		{
			obj_out << "f " <<
				triangle_point_1 << "/" << "/" << triangle_point_1 << " " <<
				triangle_point_2 << "/" << "/" << triangle_point_2 << " " <<
				triangle_point_3 << "/" << "/" << triangle_point_3 << "\n";
		}
	}

	obj.close();

	// make .png(texture)
	QImage texture_image(texture_ns, texture_nt, QImage::Format_RGB888);
	for (int i = 0; i < texture_nt; i++)
	{
		for (int j = 0; j < texture_ns; j++)
		{
			int index = ((i * texture_ns) + j) * 3;
			unsigned char b = texture[index];
			unsigned char g = texture[index + 1];
			unsigned char r = texture[index + 2];
			QRgb color = qRgb(texture[index], texture[index + 1], texture[index + 2]);
			texture_image.setPixel(j, i, color);
		}
	}
	if (!texture_image.save(texture_path, "PNG"))
	{
		return false;
	}

	// make .mtl
	QFile mtl(mtl_path);
	if (!mtl.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
	{
		return false;
	}

	QTextStream mtl_out(&mtl);
	mtl_out << "# HDXWILL Will3D v1.2.0\n";

	mtl_out << "newmtl " << mtl_name << "\n";
	mtl_out << "Ns 100.0" << "\n";
	mtl_out << "Ni 1.0" << "\n";
	mtl_out << "Ka 1.0 1.0 1.0" << "\n";
	mtl_out << "Kd 1.0 1.0 1.0" << "\n";
	mtl_out << "Ks 0.5 0.5 0.5" << "\n";
	mtl_out << "d 1.0" << "\n";
	mtl_out << "illum 2" << "\n";
	mtl_out << "map_Kd " << texture_path << "\n";

	mtl.close();

	return true;
}
