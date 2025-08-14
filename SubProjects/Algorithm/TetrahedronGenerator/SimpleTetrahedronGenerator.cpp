#include "SimpleTetrahedronGenerator.h"
using namespace std;

void SimpleTetrahedronGenerator::makeTetrahedronGrid(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& tetrahedrons,
	float size,
	int nGrids
) {
	float sizeX = size;
	float sizeY = size;
	float sizeZ = size;
	int nGridsX = nGrids;
	int nGridsY = nGrids;
	int nGridsZ = nGrids;
	int npXg = nGridsX + 1; // nPointsX in grid
	int npYg = nGridsY + 1;
	int npZg = nGridsZ + 1;
	int npg = npXg*npYg*npZg;
	int npXc = nGridsX;
	int npYc = nGridsY;
	int npZc = nGridsZ;
	float dx = sizeX / nGridsX;
	float dy = sizeY / nGridsY;
	float dz = sizeZ / nGridsZ;
	float startCoordX = -sizeX / 2.0;
	float startCoordY = -sizeY / 2.0;
	float startCoordZ = -sizeZ / 2.0;
	points.clear();
	points.reserve(npXg*npYg*npZg + npXc*npYc*npZc);
	tetrahedrons.clear();
	tetrahedrons.reserve(12 * nGridsX*nGridsY*nGridsZ);

	for (int iz = 0; iz < npZg; iz++) {
		for (int iy = 0; iy < npYg; iy++) {
			for (int ix = 0; ix < npXg; ix++) {
				float x = startCoordX + dx*ix;
				float y = startCoordY + dy*iy;
				float z = startCoordZ + dz*iz;
				points.push_back(glm::vec3(x, y, z));
			}
		}
	}
	for (int iz = 0; iz < npZc; iz++) {
		for (int iy = 0; iy < npYc; iy++) {
			for (int ix = 0; ix < npXc; ix++) {
				float x = startCoordX + dx / 2.0 + dx*ix;
				float y = startCoordY + dy / 2.0 + dy*iy;
				float z = startCoordZ + dz / 2.0 + dz*iz;
				points.push_back(glm::vec3(x, y, z));
			}
		}
	}
	// points[ix + npXg*iy + npXg*npYg*iz] for (ix, iy, iz) in grid
	// points[npg + ix + npXc*iy + npXc*npYc*iz] for (ix, iy, iz) in center
	auto idxg = [npXg, npYg](int ix, int iy, int iz) {
		return ix + npXg*iy + npXg*npYg*iz;
	};
	auto idxc = [npg, npXc, npYc](int ix, int iy, int iz) {
		return npg + ix + npXc*iy + npXc*npYc*iz;
	};
	for (int iz = 0; iz < nGridsZ; iz++) {
		for (int iy = 0; iy < nGridsY; iy++) {
			for (int ix = 0; ix < nGridsX; ix++) {
				vector<vector<int>> tetra12(12, vector<int>(4));
				{
					// down triangle
					// 0: bot
					// 1: top
					// 2: 0'
					// 3: 90'
					// 4: 180'
					// 5: 270'
					tetra12[0][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[0][1] = idxg(ix + 1, iy + 0, iz + 0);
					tetra12[0][2] = idxg(ix + 1, iy + 1, iz + 0);
					tetra12[0][3] = idxc(ix, iy, iz);
					tetra12[1][0] = idxg(ix + 0, iy + 0, iz + 1);
					tetra12[1][1] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[1][2] = idxg(ix + 1, iy + 0, iz + 1);
					tetra12[1][3] = idxc(ix, iy, iz);
					tetra12[2][0] = idxg(ix + 1, iy + 0, iz + 0);
					tetra12[2][1] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[2][2] = idxg(ix + 1, iy + 1, iz + 0);
					tetra12[2][3] = idxc(ix, iy, iz);
					tetra12[3][0] = idxg(ix + 0, iy + 1, iz + 0);
					tetra12[3][1] = idxg(ix + 1, iy + 1, iz + 0);
					tetra12[3][2] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[3][3] = idxc(ix, iy, iz);
					tetra12[4][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[4][1] = idxg(ix + 0, iy + 1, iz + 0);
					tetra12[4][2] = idxg(ix + 0, iy + 1, iz + 1);
					tetra12[4][3] = idxc(ix, iy, iz);
					tetra12[5][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[5][1] = idxg(ix + 1, iy + 0, iz + 1);
					tetra12[5][2] = idxg(ix + 1, iy + 0, iz + 0);
					tetra12[5][3] = idxc(ix, iy, iz);
				}
				{
					// up triangle
					// 6: bot
					// 7: top
					// 8: 0'
					// 9: 90'
					// 10: 180'
					// 11: 270'
					tetra12[6][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[6][1] = idxg(ix + 1, iy + 1, iz + 0);
					tetra12[6][2] = idxg(ix + 0, iy + 1, iz + 0);
					tetra12[6][3] = idxc(ix, iy, iz);
					tetra12[7][0] = idxg(ix + 0, iy + 0, iz + 1);
					tetra12[7][1] = idxg(ix + 0, iy + 1, iz + 1);
					tetra12[7][2] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[7][3] = idxc(ix, iy, iz);
					tetra12[8][0] = idxg(ix + 1, iy + 0, iz + 0);
					tetra12[8][1] = idxg(ix + 1, iy + 0, iz + 1);
					tetra12[8][2] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[8][3] = idxc(ix, iy, iz);
					tetra12[9][0] = idxg(ix + 0, iy + 1, iz + 0);
					tetra12[9][1] = idxg(ix + 1, iy + 1, iz + 1);
					tetra12[9][2] = idxg(ix + 0, iy + 1, iz + 1);
					tetra12[9][3] = idxc(ix, iy, iz);
					tetra12[10][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[10][1] = idxg(ix + 0, iy + 1, iz + 1);
					tetra12[10][2] = idxg(ix + 0, iy + 0, iz + 1);
					tetra12[10][3] = idxc(ix, iy, iz);
					tetra12[11][0] = idxg(ix + 0, iy + 0, iz + 0);
					tetra12[11][1] = idxg(ix + 0, iy + 0, iz + 1);
					tetra12[11][2] = idxg(ix + 1, iy + 0, iz + 1);
					tetra12[11][3] = idxc(ix, iy, iz);
				}
				for (int i = 0; i < 12; i++) {
					tetrahedrons.push_back(tetra12[i]);
				}
			}
		}
	}
}
