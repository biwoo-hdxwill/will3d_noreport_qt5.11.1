#include "MarchingCube.h"
/*=========================================================================

File:			class MarchingCube
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-13
Last date:		2016-01-13

=========================================================================*/
#include <set>
#include <map>
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif

#include "../../../Algorithm/Util/Core/Logger.hpp"

MarchingCube::MarchingCube() : m_nIndex(0), m_fIsoVal(0.0f), m_nEdge(0) {}

MarchingCube::FOV::FOV() {
	m_ngx = 0;
	m_ngy = 0;
	m_ngz = 0;
	m_x0 = 0;
	m_x1 = 0;
	m_y0 = 0;
	m_y1 = 0;
	m_z0 = 0;
	m_z1 = 0;
}

MarchingCube::FOV::FOV(
	int ngx, int ngy, int ngz,
	Ein x0, Ein x1, Ein y0, Ein y1, Ein z0, Ein z1)
	: m_ngx(ngx), m_ngy(ngy), m_ngz(ngz), m_x0(x0), m_x1(x1), m_y0(y0), m_y1(y1), m_z0(z0), m_z1(z1) {}

MarchingCube::DownIdx::DownIdx(int nx, int ny, int nz, float down)
	: m_nx(nx), m_ny(ny), m_nz(nz), m_down(down)
{}

MarchingCube::Idx12::Idx12(int npx, int npy, int npz, float down)
	: m_npx(npx), m_npy(npy), m_npz(npz), m_down(down)
{}

MarchingCube::Idx3::Idx3(int npx, int npy, int npz)
	: m_npx(npx), m_npy(npy), m_npz(npz)
{}

glm::vec3 MarchingCube::linearInterp(const glm::vec4& p1, const glm::vec4& p2, float val) {
	glm::vec3 p = glm::vec3(p1) + (glm::vec3(p2) - glm::vec3(p1)) / (p2[3] - p1[3]) * (val - p1[3]);
	p.x = -p.x;

	return p;
}

void MarchingCube::execute(
	std::vector<glm::vec3>& pointsOut,
	std::vector<glm::u32vec3>& trianglesOut,
	float threshold,
	unsigned short** data,
	int nx, int ny, int nz,
	int down,
	std::vector<glm::vec3>* normalsOut)
{
	lg << "MarchingCube #1" << std::endl;
	lg << "threshold = " << threshold << std::endl;
	lg << "down = " << down << std::endl;

	int npx = nx;
	int npy = ny;
	int npz = nz;
	int ngx = nx - 1;
	int ngy = ny - 1;
	int ngz = nz - 1;
	pointsOut.clear();
	trianglesOut.clear();

	std::vector<std::vector<int>> triangles_;
	triangles_.reserve(nx * ny + ny * nz + nz * nx); // approximate size

	DownIdx at(nx, ny, nz, 1);
	Idx12 idx12(npx, npy, npz, down);
	Idx3 idx3(npx, npy, npz);
	FOV fov(ngx, ngy, ngz, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	lg.tic("triangle vector Init");
	//#if defined(_OPENMP)
	//	int nthreads = omp_get_max_threads();
	//#else
	//	int nthreads = 1;
	//#endif
	//	lg << "omp_get_max_threads = " << nthreads << endl;
	//	omp_set_num_threads(nthreads);
	//	//vector<vector<unsigned long long>> omp_triangles(nthreads);
	//#pragma omp parallel
	//	{
	//		int id = omp_get_thread_num();
	//		vector<unsigned long long> triangle;
	//		//vector<unsigned long long>& triangles = omp_triangles[id];
	//		//int curSize = 0;
	//		//int curCap = 3 * 3 * (ngx*ngy + ngy*ngz + ngz*ngx);
	//		//triangles.resize(curCap);
	float vals[8];
	//		int cubeIdx = 0;
	//
	//for (int iz = id; iz < ngz; iz += (down * nthreads)) 
	for (int iz = 0; iz < ngz; iz += down)
	{
		for (int iy = 0; iy < ngy; iy += down)
		{
			for (int ix = 0; ix < ngx; ix += down)
			{
				// step 3
				vals[0] = data[iz + 0][at(ix + 0, iy + 0)];
				vals[1] = data[iz + 0][at(ix + down, iy + 0)];
				vals[2] = data[iz + down][at(ix + down, iy + 0)];
				vals[3] = data[iz + down][at(ix + 0, iy + 0)];
				vals[4] = data[iz + 0][at(ix + 0, iy + down)];
				vals[5] = data[iz + 0][at(ix + down, iy + down)];
				vals[6] = data[iz + down][at(ix + down, iy + down)];
				vals[7] = data[iz + down][at(ix + 0, iy + down)];

				// step 4: get cubeidx
				int cubeIdx = 0;
				for (int n = 0; n < 8; n++)
					if (vals[n] <= threshold)
						cubeIdx |= (1 << n);

				// step 5: complete inside or outsize
				if (!edgeTable[cubeIdx])
					continue;

				// step 7: make triangle by using triTable
				for (int n = 0; triTable[cubeIdx][n] != -1; n += 3)
				{
					/*if (!(curSize < curCap - 2)) {
						curCap *= 2;
						triangles.resize(curCap);
						}

						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 2]);
						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 1]);
						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 0]);*/

					std::vector<int> triangle = {
						idx12(ix, iy, iz, triTable[cubeIdx][n + 0]),
						idx12(ix, iy, iz, triTable[cubeIdx][n + 1]),
						idx12(ix, iy, iz, triTable[cubeIdx][n + 2]) // CCW
					};

					triangles_.push_back(triangle);
				}
			}
		}
	}

	//triangles.resize(curSize);
	//}

	lg.toc();

	lg.tic("eval vertexs and inverse map");

	std::set<unsigned long long> idxSet; // set has no same value, and sorts in ascending order
	for (const auto& triangle_ : triangles_)
		for (auto i : triangle_)
			idxSet.insert(i);

	pointsOut.reserve(idxSet.size());
	trianglesOut.reserve(idxSet.size());

	std::map<unsigned long long, unsigned int> inverseMap;
	unsigned int i = 0;

	for (auto a : idxSet) {
		// make inverse map for triangle reidx
		inverseMap[a] = i;

		// make points
		int ix, iy, iz;
		int e3;
		idx3(ix, iy, iz, e3, a);
		glm::vec4 verta = glm::vec4(fov.xip(ix), fov.yip(iy), fov.zip(iz), data[iz][at(ix, iy)]);
		glm::vec4 vertb;
		switch (e3) {
		case 0:
			vertb = glm::vec4(fov.xip(ix + down), fov.yip(iy), fov.zip(iz), data[iz][at(ix + down, iy)]);
			break;
		case 1:
			vertb = glm::vec4(fov.xip(ix), fov.yip(iy + down), fov.zip(iz), data[iz][at(ix, iy + down)]);
			break;
		case 2:
			vertb = glm::vec4(fov.xip(ix), fov.yip(iy), fov.zip(iz + down), data[iz + down][at(ix, iy)]);
			break;
		}
		pointsOut.push_back(glm::vec3(linearInterp(verta, vertb, threshold)));

		++i;
	}

	lg.toc();

	lg.tic("make result");

	// reidx triangles_
	for (const auto& triangle_ : triangles_) {
		glm::u32vec3 triangle;
		int count = 0;
		for (auto a : triangle_)
		{
			triangle[count] = inverseMap[a];
			++count;
		}
		trianglesOut.push_back(triangle);
	}

	lg.toc();


	if (normalsOut) {

		lg.tic("make normal");

		std::vector<glm::vec3> normals_;

		for (const auto& tri : trianglesOut) {
			glm::vec3 &p0 = pointsOut.at(tri[0]);
			glm::vec3 &p1 = pointsOut.at(tri[1]);
			glm::vec3 &p2 = pointsOut.at(tri[2]);

			glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
			normals_.push_back(n);
		}

		std::vector<std::vector<int>> mapPointsToIndices(pointsOut.size());

		for (int tidx = 0; tidx < trianglesOut.size(); tidx++)
		{
			glm::u32vec3 vidx = trianglesOut.at(tidx);

			mapPointsToIndices[vidx.x].push_back(tidx);
			mapPointsToIndices[vidx.y].push_back(tidx);
			mapPointsToIndices[vidx.z].push_back(tidx);
		}

		normalsOut->clear();

		for (int vidx = 0; vidx < pointsOut.size(); vidx++)
		{
			unsigned int count = 0;
			glm::vec3 n(0.0f);
			for (auto tidx : mapPointsToIndices[vidx]) {
				n += normals_[tidx];
				++count;
			}

			n.x = n.x / count;
			n.y = n.y / count;
			n.z = n.z / count;

			n = glm::normalize(n);

			normalsOut->push_back(n);
		}

		lg.toc();

	}
}

void MarchingCube::execute(
	std::vector<glm::vec3>& pointsOut,
	std::vector<std::vector<int>>& trianglesOut,
	float threshold,
	unsigned short** data,
	int nx, int ny, int nz,
	float down,
	std::vector<glm::vec3>* normalsOut
) {

	lg << "MarchingCube #2" << std::endl;
	lg << "threshold = " << threshold << std::endl;
	lg << "down = " << down << std::endl;


	/* dim of data */
	int npx = nx / down; // # of points in x axis;
	int npy = ny / down; // # of points in y axis;
	int npz = nz / down; // # of points in z axis;
	int ngx = npx - 1; // # of grid in x axis;
	int ngy = npy - 1; // # of grid in y axis;
	int ngz = npz - 1; // # of grid in z axis;

	DownIdx at(nx, ny, nz, down);
	Idx12 idx12(npx, npy, npz, 1);
	Idx3 idx3(npx, npy, npz);
	FOV fov(ngx, ngy, ngz, -1, 1, -1, 1, -1, 1);

#if 1
	/* boundary set 0 for closed surface generation */
	std::vector<unsigned short> px0(npy*npz, 0);
	std::vector<unsigned short> px1(npy*npz, 0);
	std::vector<unsigned short> py0(npz*npx, 0);
	std::vector<unsigned short> py1(npz*npx, 0);
	std::vector<unsigned short> pz0(npx*npy, 0);
	std::vector<unsigned short> pz1(npx*npy, 0);
	{
		int ix, iy, iz;
		ix = 0;
		for (int iz = 0; iz < npz; iz++) {
			for (int iy = 0; iy < npy; iy++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					px0[npy*iz + iy] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
		ix = npx - 1;
		for (int iz = 0; iz < npz; iz++) {
			for (int iy = 0; iy < npy; iy++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					px1[npy*iz + iy] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
		iy = 0;
		for (int iz = 0; iz < npz; iz++) {
			for (int ix = 0; ix < npx; ix++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					py0[npx*iz + ix] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
		iy = npy - 1;
		for (int iz = 0; iz < npz; iz++) {
			for (int ix = 0; ix < npx; ix++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					py1[npx*iz + ix] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
		iz = 0;
		for (int iy = 0; iy < npy; iy++) {
			for (int ix = 0; ix < npx; ix++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					pz0[npx*iy + ix] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
		iz = npz - 1;
		for (int iy = 0; iy < npy; iy++) {
			for (int ix = 0; ix < npx; ix++) {
				if (data[at(iz)][at(ix, iy)] != 0) {
					pz1[npx*iy + ix] = data[at(iz)][at(ix, iy)];
					data[at(iz)][at(ix, iy)] = 0;
				}
			}
		}
	}
#endif

	lg.tic("triangle vector Init");
#if defined(_OPENMP)
	int nthreads = omp_get_max_threads();
#else
	int nthreads = 1;
#endif

	lg << "omp_get_max_threads = " << nthreads << std::endl;

	omp_set_num_threads(nthreads);
	std::vector<std::vector<unsigned long long>> omp_triangles(nthreads);
	// GPU에서 똑같이 짜면됨
#pragma omp parallel
	{
		int id = omp_get_thread_num();
		std::vector<unsigned long long>& triangles = omp_triangles[id];
		int curSize = 0;
		int curCap = 3 * 3 * (ngx*ngy + ngy*ngz + ngz*ngx);
		triangles.resize(curCap);
		float vals[8];
		int cubeIdx;
		for (int iz = id; iz < ngz; iz += nthreads) {
			for (int iy = 0; iy < ngy; iy++) {
				for (int ix = 0; ix < ngx; ix++) {

					// step 3
					vals[0] = data[at(iz + 0)][at(ix + 0, iy + 0)];
					vals[1] = data[at(iz + 0)][at(ix + 1, iy + 0)];
					vals[2] = data[at(iz + 1)][at(ix + 1, iy + 0)];
					vals[3] = data[at(iz + 1)][at(ix + 0, iy + 0)];
					vals[4] = data[at(iz + 0)][at(ix + 0, iy + 1)];
					vals[5] = data[at(iz + 0)][at(ix + 1, iy + 1)];
					vals[6] = data[at(iz + 1)][at(ix + 1, iy + 1)];
					vals[7] = data[at(iz + 1)][at(ix + 0, iy + 1)];

					// step 4: get cubeidx
					cubeIdx = 0;
					for (int n = 0; n < 8; n++) {
						if (vals[n] <= threshold) {
							cubeIdx |= (1 << n);
						}
					}

					// step 5: complete inside or outside
					if (!edgeTable[cubeIdx]) {
						continue;
					}
					// step 7: make triangle by using triTable
					int n;
					for (n = 0; triTable[cubeIdx][n] != -1; n += 3) {
						if (!(curSize < curCap - 2)) {
							curCap *= 2;
							triangles.resize(curCap);
						}
						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 2]);
						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 0]);
						triangles[curSize++] = idx12(ix, iy, iz, triTable[cubeIdx][n + 1]);
					}
				}
			}
		}

		triangles.resize(curSize);
	}

	lg.toc();

	lg.tic("eval vertexs and inverse map");
	int nTotalTriangles = 0;
	for (int id = 0; id < nthreads; id++) {
		nTotalTriangles += omp_triangles[id].size() / 3;
	}
	pointsOut.resize(nTotalTriangles * 3);
	std::map<unsigned long long, int> inverseMap;
	int cnt = 0;
	for (int id = 0; id < nthreads; id++) {
		const auto& triangles = omp_triangles[id];
		for (const auto& a : triangles) {
			auto it = inverseMap.find(a);
			auto end = inverseMap.end();
			if (it != end) {
				continue;
			} else {
				inverseMap[a] = cnt;
				int ix, iy, iz;
				int e3;
				idx3(ix, iy, iz, e3, a);
				glm::vec4 verta = glm::vec4(fov.xip(ix), fov.yip(iy), fov.zip(iz), data[at(iz)][at(ix, iy)]);
				glm::vec4 vertb;
				switch (e3) {
				case 0:
					vertb = glm::vec4(fov.xip(ix + 1), fov.yip(iy + 0), fov.zip(iz + 0), data[at(iz + 0)][at(ix + 1, iy + 0)]);
					break;
				case 1:
					vertb = glm::vec4(fov.xip(ix + 0), fov.yip(iy + 1), fov.zip(iz + 0), data[at(iz + 0)][at(ix + 0, iy + 1)]);
					break;
				case 2:
					vertb = glm::vec4(fov.xip(ix + 0), fov.yip(iy + 0), fov.zip(iz + 1), data[at(iz + 1)][at(ix + 0, iy + 0)]);
					break;
				}
				pointsOut[cnt] = glm::vec3(linearInterp(verta, vertb, threshold));
				++cnt;
			}
		}
	}
	pointsOut.resize(cnt);

	lg.toc();

	lg.tic("make result");
	trianglesOut.resize(nTotalTriangles, std::vector<int>(3));
	for (int id = 0; id < nthreads; id++) {
		const auto& triangles = omp_triangles[id];
		int base = 0;
		for (int i = 0; i < id; i++) {
			base += omp_triangles[i].size() / 3;
		}
		int nTriangles = triangles.size() / 3;
		for (int i = 0; i < nTriangles; i++) {
			trianglesOut[base + i][0] = inverseMap[triangles[3 * i + 0]];
			trianglesOut[base + i][1] = inverseMap[triangles[3 * i + 1]];
			trianglesOut[base + i][2] = inverseMap[triangles[3 * i + 2]];
		}
	}

	lg.toc();


#if 1
	/* recover boundary */
	{
		int ix, iy, iz;
		ix = 0;
		for (int iz = 0; iz < npz; iz++) {
			for (int iy = 0; iy < npy; iy++) {
				if (px0[npy*iz + iy] != 0) {
					data[at(iz)][at(ix, iy)] = px0[npy*iz + iy];
				}
			}
		}
		ix = npx - 1;
		for (int iz = 0; iz < npz; iz++) {
			for (int iy = 0; iy < npy; iy++) {
				if (px1[npy*iz + iy] != 0) {
					data[at(iz)][at(ix, iy)] = px1[npy*iz + iy];
				}
			}
		}
		iy = 0;
		for (int iz = 0; iz < npz; iz++) {
			for (int ix = 0; ix < npx; ix++) {
				if (py0[npx*iz + ix] != 0) {
					data[at(iz)][at(ix, iy)] = py0[npx*iz + ix];
				}
			}
		}
		iy = npy - 1;
		for (int iz = 0; iz < npz; iz++) {
			for (int ix = 0; ix < npx; ix++) {
				if (py1[npx*iz + ix] != 0) {
					data[at(iz)][at(ix, iy)] = py1[npx*iz + ix];
				}
			}
		}
		iz = 0;
		for (int iy = 0; iy < npy; iy++) {
			for (int ix = 0; ix < npx; ix++) {
				if (pz0[npx*iy + ix] != 0) {
					data[at(iz)][at(ix, iy)] = pz0[npx*iy + ix];
				}
			}
		}
		iz = npz - 1;
		for (int iy = 0; iy < npy; iy++) {
			for (int ix = 0; ix < npx; ix++) {
				if (pz1[npx*iy + ix] != 0) {
					data[at(iz)][at(ix, iy)] = pz1[npx*iy + ix];
				}
			}
		}
	}
#endif

	if (normalsOut) {

		lg.tic("make normal");

		std::vector<glm::vec3> normals_;

		for (const auto& tri : trianglesOut) {
			glm::vec3 &p0 = pointsOut.at(tri[0]);
			glm::vec3 &p1 = pointsOut.at(tri[1]);
			glm::vec3 &p2 = pointsOut.at(tri[2]);

			glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
			normals_.push_back(n);
		}

		std::vector<std::vector<int>> mapPointsToIndices(pointsOut.size());

		for (int tidx = 0; tidx < trianglesOut.size(); tidx++)
		{
			std::vector<int> vidx = trianglesOut.at(tidx);

			mapPointsToIndices[vidx[0]].push_back(tidx);
			mapPointsToIndices[vidx[1]].push_back(tidx);
			mapPointsToIndices[vidx[2]].push_back(tidx);
		}

		normalsOut->clear();

		for (int vidx = 0; vidx < pointsOut.size(); vidx++)
		{
			unsigned int count = 0;
			glm::vec3 n(0.0f);
			for (auto tidx : mapPointsToIndices[vidx]) {
				n += normals_[tidx];
				++count;
			}

			n.x = n.x / count;
			n.y = n.y / count;
			n.z = n.z / count;

			n = glm::normalize(n);

			normalsOut->push_back(n);
		}

		lg.toc();

	}
}

void MarchingCube::executeForAirway(
	std::vector<tri_STL> &meshSTL,
	float threshold,
	float** data,
	int nx, int ny, int nz,
	int down
)
{
	lg << "MarchingCube #3" << std::endl;
	lg << "threshold = " << threshold << std::endl;
	lg << "down = " << down << std::endl;

}

// airway
bool MarchingCube::initializeAirway(glm::vec3 &vStart, glm::vec3& vSize, float fIsoVal) {
	if (vSize.x <= 0 || vSize.y <= 0 || vSize.z <= 0)
		return false;

	m_vStart = vStart;
	m_vSize = vSize;
	m_fIsoVal = fIsoVal;

	return true;
}

/*
* Sets the positions of 8 vertices consisting of a current cube.
//	[ Index ]
//		7 _______ 6
//		 /|		/|
//	   3/______/2|
//	    |4/----|-/5
//		|/_____|/
//	   0	    1
*/
void MarchingCube::setCubeVerticesPos(const int& x, const int& y, const int& z)
{
	m_vertex[0].vPos = glm::vec3(m_vSize.x*x, m_vSize.y*y, m_vSize.z*z);
	m_vertex[1].vPos = glm::vec3(m_vSize.x*(x + 1), m_vSize.y*y, m_vSize.z*z);
	m_vertex[2].vPos = glm::vec3(m_vSize.x*(x + 1), m_vSize.y*(y + 1), m_vSize.z*z);
	m_vertex[3].vPos = glm::vec3(m_vSize.x*x, m_vSize.y*(y + 1), m_vSize.z*z);
	m_vertex[4].vPos = glm::vec3(m_vSize.x*x, m_vSize.y*y, m_vSize.z*(z + 1));
	m_vertex[5].vPos = glm::vec3(m_vSize.x*(x + 1), m_vSize.y*y, m_vSize.z*(z + 1));
	m_vertex[6].vPos = glm::vec3(m_vSize.x*(x + 1), m_vSize.y*(y + 1), m_vSize.z*(z + 1));
	m_vertex[7].vPos = glm::vec3(m_vSize.x*x, m_vSize.y*(y + 1), m_vSize.z*(z + 1));
}

int MarchingCube::calculateIndex(void)
{
	//	[ Index ]
	//		7 _______ 6
	//		 /|		/|
	//	   3/______/2|
	//	    |4/----|-/5
	//		|/_____|/
	//	   0	    1
	//
	//		[ Cube numbering ]
	//		Fig.4 in "Marching cubes: a high resolution 3D surface construction algyorithm" SIGGRAPH '87
	//		|	7	|	6	|	5	|	4	|	3	|	2	|	1	|	0	|
	//		|	128	|	64	|	32	|	16	|	8	|	4	|	2	|	1	|

	m_nIndex = 0;
	//int nInsideVerts = 0;

	for (int iVert = 0; iVert < 8; ++iVert)
	{
		if (m_vertex[iVert].val < m_fIsoVal)	// If the current vertex belongs to the outside
			m_nIndex |= bitFlags[iVert];		// 8 bits : Vertex flag indicating that it belongs to the inside or outside
												//else
	}
	m_nEdge = edgeTable[m_nIndex];			// 12 bits : Edge flag indicating that it belongs to the surface
	return m_nIndex;
}

/*
* Find the vertices where the surface inthrsects the cube and add triangles to mesh data
*/
void MarchingCube::polygonize(std::vector<tri_STL>& vMesh, unsigned int areaVal) {
	// Find the vertices where the surface intersects the cube
	glm::vec3 vertList[12];

	for (int iEdge = 0; iEdge < 12; ++iEdge)
	{
		if (m_nEdge & bitFlags[iEdge])
		{
			// 2 vertices sharing the edge
			int v0 = edgeVertex[iEdge][0];
			int v1 = edgeVertex[iEdge][1];

			// The vertex corresponding to the edge index
			vertList[iEdge] = linearInterp(glm::vec4(m_vertex[v0].vPos, m_vertex[v0].val), glm::vec4(m_vertex[v1].vPos, m_vertex[v1].val), m_fIsoVal);
		}
	}

	// Add triangles (vNormal & vVertices & iClass)
	int* table = triTable[m_nIndex];	// A triangle consisting of 3 vertices with front clockwise
	int iE;
	int iV = 0;
	tri_STL triangle;
	for (int i = 0; (iE = table[i]) != -1; ++i)
	{
		switch (iV) {
		case 0:
			triangle.v1.x = vertList[iE].x;
			triangle.v1.y = vertList[iE].y;
			triangle.v1.z = vertList[iE].z;
			break;
		case 1:
			triangle.v2.x = vertList[iE].x;
			triangle.v2.y = vertList[iE].y;
			triangle.v2.z = vertList[iE].z;
			break;
		case 2:
			triangle.v3.x = vertList[iE].x;
			triangle.v3.y = vertList[iE].y;
			triangle.v3.z = vertList[iE].z;
			break;
		}
		++iV;

		if (iV == 3) {
			glm::vec3 v31(triangle.v3.x - triangle.v1.x, triangle.v3.y - triangle.v1.y, triangle.v3.z - triangle.v1.z);
			glm::vec3 v21(triangle.v2.x - triangle.v1.x, triangle.v2.y - triangle.v1.y, triangle.v2.z - triangle.v1.z);
			glm::vec3 vN = glm::normalize(glm::cross(v31, v21));
			//glm::vec3 vN = glm::vec3::CrossProduct(v31, v21).Normalized();
			triangle.normal.x = vN.x;
			triangle.normal.y = vN.y;
			triangle.normal.z = vN.z;
			triangle.cntAttributes = (unsigned short)1;
			triangle.nColorVal = areaVal;
			vMesh.push_back(triangle);
			iV = 0;
		}
	}
}
