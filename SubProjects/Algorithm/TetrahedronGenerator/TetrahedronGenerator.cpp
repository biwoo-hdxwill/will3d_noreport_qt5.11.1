#pragma message("## TetrahedronGenerator/TetrahedronGenerator.cpp visited")
#include "TetrahedronGenerator.h"

//#include <Util/MatlabEngine/MatlabEngine.h>
//#include <Util/MatlabEngine/mxArrayWrapper.h>
#include <Util/Core/Logger.hpp>
#if defined(_WIN32)
#include <Util/Core/IOParser_v3.h>
#endif
#include <C3t3Maker.h>
#include "PolyhedronIOWrapper.h"
#include "C3t3Wrapper.h"
#include <set>
#include <string>
using namespace std;

void TetrahedronGenerator::_init() {
	this->facetAngle = 25;
	this->facetSize = 0.025;
	this->facetDistance = 0.025;
	this->cellRadiusEdgeRatio = 2;
	this->cellSize = 0.025;
}
TetrahedronGenerator::TetrahedronGenerator() {
	_init();
}

bool TetrahedronGenerator::generate_c3t3(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& tetrahedrons,
	std::vector<std::vector<int>>& triangles,
	const std::vector<glm::vec3>& surfacePoints,
	const std::vector<std::vector<int>>& surfaceTriangles
) const {
	// make polyhedron
	Polyhedron polyhedron;
	PolyhedronIOWrapper::toPolyhedron(polyhedron, surfacePoints, surfaceTriangles);

	// set param for c3t3Maker
	C3t3Maker cm;
	cm.facetAngle = this->facetAngle;
	cm.facetSize = this->facetSize;
	cm.facetDistance = this->facetDistance;
	cm.cellRadiusEdgeRatio = this->cellRadiusEdgeRatio;
	cm.cellSize = this->cellSize;

	C3t3_polyhedral c3t3;

	// execute gen tetrahedronc c3t3 object

	lg.tic("makeFromPolyhedralDomain");

	bool res = cm.makeFromPolyhedralDomain(c3t3, polyhedron);

	lg.toc();
	lg.tic("convert");

	C3t3Wrapper::convert(points, tetrahedrons, triangles, c3t3);

	lg.toc();

	return res;
}
