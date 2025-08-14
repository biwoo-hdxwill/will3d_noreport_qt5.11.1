#include "DisplacementSurface2.h"
#include <numeric>
#include <algorithm>

//#include <RenderingCode_v2/drawable.h>
#include <Util/Core/Logger.hpp>
#include <Util/Core/path_global_setting.h>
#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#else
#include <gl/glm/gtx/transform.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#endif

#if defined(__APPLE__)
#include <QOpenGLWidget>
#else
#include <QtWidgets/QOpenGLWidget>
#endif
#include <RenderingCode_v2/MeshFillVertexColor2.h>

using namespace std;
#define debug 0

static float _getDegreeAngle(float dDeltaX, float dDeltaY) {
	float dAngle = atan2(dDeltaY, dDeltaX);
	dAngle *= (180.f / glm::pi<float>());
	if (dAngle < 0.f)
		dAngle += 360.f;

	return dAngle;
}

DisplacementSurface2::DisplacementSurface2(
	QOpenGLWidget* glWidget,
	std::vector<glm::vec3>& surfPoints,
	std::vector<std::vector<int>>& surfTriangles,
	std::vector<glm::vec3>& surfDisplacements) {
	m_fboWidth = 512;
	m_fboHeight = 512;

	if (glWidget == nullptr) {
		cout << " ERR : glWidget==nullptr !" << endl;
	}

	if (glWidget->context() == nullptr) {
		cout << " ERR : glWidget context is not made yet." << endl;
	}

	m_glWidget = glWidget;
	m_surfPoints = &surfPoints;
	m_surfTriangles = &surfTriangles;
	m_surfDisplacements = &surfDisplacements;
	auto surfDisplacementsAsColor = new vector<glm::vec4>(surfDisplacements.size());
	for (int i = 0; i < surfDisplacementsAsColor->size(); i++) {
		surfDisplacementsAsColor->at(i) = glm::vec4(surfDisplacements[i], 1);
	}

	m_surf2 = new MeshFillVertexColor2(
		m_surfPoints,
		m_surfTriangles,
		surfDisplacementsAsColor,
		"MeshFillVertexColor_tex.vert",
		"MeshFillVertexColor_tex.frag"
		//fullfile(SHADER_PATH, "MeshFillVertexColor_tex.vert"),
		//fullfile(SHADER_PATH, "MeshFillVertexColor_tex.frag")
	);

	m_glWidget->makeCurrent();
	{
		m_surf2->initVBOs();
		m_surf2->initVAOs();
		m_surf2->initPROGs();
		m_surf2->initFBOs(m_fboWidth, m_fboHeight);
	}
	m_glWidget->doneCurrent();
}

void DisplacementSurface2::Deinit() {
	if (m_surf2 != nullptr) {
		//printf("\n\n$$$$$$$$$$$$$$$$$$$$$$$$ DisplacementSurface2::_deinit() called !!           \n\n");

		m_glWidget->makeCurrent();
		{
			m_surf2->deinitAllGLObjects();
		}
		m_glWidget->doneCurrent();

		delete m_surf2;
		m_surf2 = nullptr;
	}
}

DisplacementSurface2::~DisplacementSurface2() {
	Deinit();
}

////////////////////////////////////////////////////////////////////////////////////////
bool DisplacementSurface2::execute(
	std::vector<glm::vec3>& displacements,
	std::vector<float>& dists,
	std::vector<char>& valid,
	const std::vector<glm::vec3>& queryPoints,
	float x0, float x1, float y0, float y1, float z0, float z1) // bounding box
{
	if (m_surf2 == nullptr) {
		cout << " ERR : m_surf2==nullptr !" << endl;
	}

	// 코 cut 되는거때문에 y방향으로 좀 키워줘야할듯?...
	//cout << "			bounding box!!! = " << x0 << "," << x1 << "," << y0 << "," << y1 << "," << z0 << "," << z1 << "\n";
	// add extra space
	x0 = x0*1.1;
	x1 = x1*1.1;
	y0 = y0*1.1;
	y1 = y1*1.1;

	/// from y0 to y1
	m_glWidget->makeCurrent();
	{
		int fboWidth = m_fboWidth;
		int fboHeight = m_fboHeight;

		/* setup matrix */
		glm::mat4 preproj = glm::scale(glm::vec3(2 / (x1 - x0), 2 / (y1 - y0), 2 / (z1 - z0))) * glm::translate(glm::vec3(-(x0 + x1) / 2, -(y0 + y1) / 2, -(z0 + z1) / 2));
		glm::mat4 proj = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
		glm::mat4 toIdxCoord = glm::scale(glm::vec3(fboWidth - 1, fboHeight - 1, 1)) * glm::translate(glm::vec3(0.5, 0.5, 0.5)) * glm::scale(glm::vec3(0.5, 0.5, 0.5));

		glm::mat4 viewA = glm::lookAt(glm::vec3(0, -1, 0), glm::vec3(0), glm::vec3(0, 0, 1)); // Face (A) direction
		viewA[3] = glm::vec4(0, 0, 0, 1); // remove translate component
		glm::mat4 viewL = glm::lookAt(glm::vec3(-1, 0, 0), glm::vec3(0), glm::vec3(0, 0, 1)); // Left Face
		viewL[3] = glm::vec4(0, 0, 0, 1); // remove translate component
		glm::mat4 viewR = glm::lookAt(glm::vec3(1, 0, 0), glm::vec3(0), glm::vec3(0, 0, 1)); // Right Face
		viewR[3] = glm::vec4(0, 0, 0, 1); // remove translate component

		glm::mat4 allA = toIdxCoord * proj * viewA * preproj;
		glm::mat4 allL = toIdxCoord * proj * viewL * preproj;
		glm::mat4 allR = toIdxCoord * proj * viewR * preproj;

		// world to idx coord
		auto l_toIdxCoord = [fboWidth, fboHeight](glm::mat4& all, const glm::vec3& p)->glm::ivec3 {
			return glm::ivec3(glm::clamp(glm::vec3(all*glm::vec4(p, 1)), glm::vec3(0), glm::vec3(fboWidth - 1, fboHeight - 1, 1)));
		};

		glm::mat4 viewMatArr[3] = { viewA, viewL, viewR };
		glm::mat4 allMatArr[3] = { allA, allL, allR };
		vector<vector<float>> img_dispVec(3);
		vector<vector<float>> img_vertCoordsVec(3);

		for (int m = 0; m < 3; m++) {
			img_dispVec[m].resize(fboWidth*fboHeight * 3); // 원하는 위치에서 이동량들
			img_vertCoordsVec[m].resize(fboWidth*fboHeight * 4); // 이동량을 원하는 위치와 mesh surface에 맵핑된 점과의 거리들

			m_surf2->draw(preproj, viewMatArr[m], proj);

			/* read texture */
			m_surf2->readTextureDisps(img_dispVec[m].data());
			m_surf2->readTextureVertCoords(img_vertCoordsVec[m].data());
		}

#if 1
		//** extrapolation code **
		// find min iY idx
		for (int m = 0; m < 3; m++) {
			int validMaxY = 0;
			float validMax3D_Z = 0.f;
			for (int y = 0; y < fboHeight; y++) {
				for (int x = 0; x < fboWidth; x++) {
					char curValid = (char)img_vertCoordsVec[m][4 * (fboWidth*y + x) + 3];
					if (curValid != 0) {
						validMaxY = y;
						validMax3D_Z = img_vertCoordsVec[m][4 * (fboWidth*y + x) + 2];
					}
				}
			}
			//cout << "			validMaxY = " << validMaxY << "\n";
			//cout << "			validMax3D_Z = " << validMax3D_Z << "\n";
			//
			int lastY = validMaxY - 2;
			// fill
			for (int y = lastY + 1; y < fboHeight; y++) {
				for (int x = 0; x < fboWidth; x++) {
					img_dispVec[m][3 * (fboWidth*y + x) + 0] = img_dispVec[m][3 * (fboWidth*lastY + x) + 0];
					img_dispVec[m][3 * (fboWidth*y + x) + 1] = img_dispVec[m][3 * (fboWidth*lastY + x) + 1];
					img_dispVec[m][3 * (fboWidth*y + x) + 2] = img_dispVec[m][3 * (fboWidth*lastY + x) + 2];

					img_vertCoordsVec[m][4 * (fboWidth*y + x) + 0] = img_vertCoordsVec[m][4 * (fboWidth*lastY + x) + 0];
					img_vertCoordsVec[m][4 * (fboWidth*y + x) + 1] = img_vertCoordsVec[m][4 * (fboWidth*lastY + x) + 1];
					img_vertCoordsVec[m][4 * (fboWidth*y + x) + 2] = img_vertCoordsVec[m][4 * (fboWidth*lastY + x) + 2];
				}
			}
		}
#endif

		// Fill displacements vector !
		displacements.resize(queryPoints.size());
		for (int i = 0; i < displacements.size(); i++) {
			glm::vec2 vecInXY = glm::vec2(queryPoints[i].x, queryPoints[i].y);
			//vecInXY = glm::normalize(vecInXY);

			float degreeAngle = _getDegreeAngle(vecInXY.x, vecInXY.y);

			int pickIdx = 2; // R
			if (degreeAngle >= 90 && degreeAngle < (90 + 120))
				pickIdx = 1; // L
			else if (degreeAngle >= (90 + 120) && degreeAngle < (90 + 120 + 120))
				pickIdx = 0; // A

			{
				glm::ivec3 idx = l_toIdxCoord(allMatArr[pickIdx], queryPoints[i]);
				int ix = idx[0];
				int iy = idx[1];

				displacements[i] = glm::vec3(
					img_dispVec[pickIdx][3 * (fboWidth*iy + ix) + 0],
					img_dispVec[pickIdx][3 * (fboWidth*iy + ix) + 1],
					img_dispVec[pickIdx][3 * (fboWidth*iy + ix) + 2]
				);
			}

			//cout << "			z1 = " << z1 << "\n";
			//if (queryPoints[i].z > z1 - 0.1) // below neck area
			//{
			//	displacements[i] = glm::vec3(0, 0, 0);

			//	for (int m = 0; m < 3; m++) {
			//		glm::ivec3 idx = l_toIdxCoord(allMatArr[m], queryPoints[i]);
			//		int ix = idx[0];
			//		int iy = idx[1];

			//		glm::vec3 curDisp = glm::vec3(
			//			img_dispVec[m][3 * (fboWidth*iy + ix) + 0],
			//			img_dispVec[m][3 * (fboWidth*iy + ix) + 1],
			//			img_dispVec[m][3 * (fboWidth*iy + ix) + 2]
			//		);

			//		displacements[i] = (1 / 3.f)*curDisp + displacements[i];
			//	}
			//}
		}

#if debug /* debug */

		for (int m = 0; m < 3; m++) {
			vector<float> img_debug_disps_x(fboWidth*fboHeight);
			vector<float> img_debug_disps_y(fboWidth*fboHeight);
			vector<float> img_debug_disps_z(fboWidth*fboHeight);
			vector<float> img_debug_vertCoords_x(fboWidth*fboHeight);
			vector<float> img_debug_vertCoords_y(fboWidth*fboHeight);
			vector<float> img_debug_vertCoords_z(fboWidth*fboHeight);
			vector<char> img_debug_vertCoords_valid(fboWidth*fboHeight);

			for (int i = 0; i < fboHeight; i++) {
				for (int j = 0; j < fboWidth; j++) {
					img_debug_disps_x[fboWidth*i + j] = img_dispVec[m][3 * (fboWidth*i + j) + 0];
					img_debug_disps_y[fboWidth*i + j] = img_dispVec[m][3 * (fboWidth*i + j) + 1];
					img_debug_disps_z[fboWidth*i + j] = img_dispVec[m][3 * (fboWidth*i + j) + 2];
				}
			}

			for (int i = 0; i < fboHeight; i++) {
				for (int j = 0; j < fboWidth; j++) {
					img_debug_vertCoords_x[fboWidth*i + j] = img_vertCoordsVec[m][4 * (fboWidth*i + j) + 0];
					img_debug_vertCoords_y[fboWidth*i + j] = img_vertCoordsVec[m][4 * (fboWidth*i + j) + 1];
					img_debug_vertCoords_z[fboWidth*i + j] = img_vertCoordsVec[m][4 * (fboWidth*i + j) + 2];
					img_debug_vertCoords_valid[fboWidth*i + j] = img_vertCoordsVec[m][4 * (fboWidth*i + j) + 3];
				}
			}

			std::ostringstream strDispX, strDispY, strDispZ;
			strDispX << "img_debug_disps" << m << "_x.raw";
			strDispY << "img_debug_disps" << m << "_y.raw";
			strDispZ << "img_debug_disps" << m << "_z.raw";

			std::ostringstream strVertX, strVertY, strVertZ, strValid;
			strVertX << "tex_vertCoords" << m << "_x.raw";
			strVertY << "tex_vertCoords" << m << "_y.raw";
			strVertZ << "tex_vertCoords" << m << "_z.raw";
			strValid << "tex_vertCoords" << m << "_valid.raw";

			IOParser_v3::writeRaw1d<float, float>(img_debug_disps_x, fullfile(DEBUG_PATH, strDispX.str()));
			IOParser_v3::writeRaw1d<float, float>(img_debug_disps_y, fullfile(DEBUG_PATH, strDispY.str()));
			IOParser_v3::writeRaw1d<float, float>(img_debug_disps_z, fullfile(DEBUG_PATH, strDispZ.str()));
			IOParser_v3::writeRaw1d<float, float>(img_debug_vertCoords_x, fullfile(DEBUG_PATH, strVertX.str()));
			IOParser_v3::writeRaw1d<float, float>(img_debug_vertCoords_y, fullfile(DEBUG_PATH, strVertY.str()));
			IOParser_v3::writeRaw1d<float, float>(img_debug_vertCoords_z, fullfile(DEBUG_PATH, strVertZ.str()));
			IOParser_v3::writeRaw1d<char, float>(img_debug_vertCoords_valid, fullfile(DEBUG_PATH, strValid.str()));
		}

#endif
	}
	m_glWidget->doneCurrent();
	return true;
}

bool DisplacementSurface2::execute(
	std::vector<glm::vec3>& displacements,
	const std::vector<glm::vec3>& queryPoints,
	float x0, float x1, float y0, float y1, float z0, float z1 // bounding box
) {
	vector<float> v1;
	vector<char> v2;
	return execute(displacements, v1, v2, queryPoints, x0, x1, y0, y1, z0, z1);
}
bool DisplacementSurface2::execute(
	std::vector<glm::vec3>& displacements,
	const std::vector<glm::vec3>& queryPoints
) {
	try {
		if (queryPoints.empty()) {
			throw std::runtime_error("queryPoints is empty");
		}
		auto x = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.x < p1.x;
		});
		auto y = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.y < p1.y;
		});
		auto z = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.z < p1.z;
		});

		vector<float> v1;
		vector<char> v2;
		return execute(displacements, v1, v2, queryPoints, x.first->x, x.second->x, y.first->y, y.second->y, z.first->z, z.second->z);
	} catch (std::runtime_error& e) {
		lg << "DisplacementSurface::execute: " << e << endl;
		return false;
	}
}
bool DisplacementSurface2::execute(
	std::vector<glm::vec3>& displacements,
	std::vector<float>& dists,
	std::vector<char>& valid,
	const std::vector<glm::vec3>& queryPoints
) {
	try {
		if (queryPoints.empty()) {
			throw std::runtime_error("queryPoints is empty");
		}
		auto x = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.x < p1.x;
		});
		auto y = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.y < p1.y;
		});
		auto z = minmax_element(queryPoints.begin(), queryPoints.end(), [](const glm::vec3& p0, const glm::vec3& p1)->bool {
			return p0.z < p1.z;
		});
		return execute(displacements, dists, valid, queryPoints, x.first->x, x.second->x, y.first->y, y.second->y, z.first->z, z.second->z);
	} catch (std::runtime_error& e) {
		lg << "DisplacementSurface::execute: " << e << endl;
		return false;
	}
}
