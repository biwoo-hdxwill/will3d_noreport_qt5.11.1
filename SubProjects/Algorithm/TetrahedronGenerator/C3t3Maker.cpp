#include "C3t3Maker.h"
#include <iostream>
#include <Util/Core/Logger.hpp>

using namespace std;
// To avoid verbose function and named parameters call
using namespace CGAL::parameters;
void C3t3Maker::_init() {
	this->facetAngle = 25;
	this->facetSize = 0.025;
	this->facetDistance = 0.025;
	this->cellRadiusEdgeRatio = 2;
	this->cellSize = 0.025;
}
C3t3Maker::C3t3Maker() {
	_init();
}
bool C3t3Maker::makeFromPolyhedralDomain(C3t3_polyhedral& c3t3, const Polyhedron& polyhedron) {
	typedef C3t3_polyhedral C3t3;
	typedef CGAL::Mesh_criteria_3<Tr_polyhedral> Mesh_criteria; // Criteria
	typedef Mesh_domain_polyhedral Mesh_domain;
	try {
		if (polyhedron.empty()) {
			throw runtime_error("polyhedron is empty");
		}
		if (!polyhedron.is_closed()) {
			throw runtime_error("polyhedron is not closed surface");
		}
		// Create domain

		lg << "create domain" << endl;

		Mesh_domain domain(polyhedron);


		lg << "criteria" << endl;

		Mesh_criteria criteria(
			facet_angle				= this->facetAngle, 
			facet_size				= this->facetSize,
			facet_distance			= this->facetDistance,
			cell_radius_edge_ratio	= this->cellRadiusEdgeRatio,
			cell_size				= this->cellSize
			);

		// Mesh generation

		lg << "make_mesh_3" << endl;

		CGAL::default_random = CGAL::Random(0); // for deterministic result. http://cgal-discuss.949826.n4.nabble.com/3D-mesh-generation-random-result-td4655452.html
		c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);

		lg << "done" << endl;

		return true;
	} catch (runtime_error& e) {
		cout << "C3t3Maker::makeFromPolyhedralDomain: " << e.what() << endl;
		return false;
	}
}

FT sphere_function(const Point& p) {
	return CGAL::squared_distance(p, Point(CGAL::ORIGIN)) - 1;
}

bool C3t3Maker::makeFromImplicit(C3t3_implicit& c3t3) {
	typedef C3t3_implicit C3t3;
	typedef CGAL::Mesh_criteria_3<Tr_implicit> Mesh_criteria; // Criteria

	// Domain (Warning: Sphere_3 constructor uses squared radius !)
	Mesh_domain_implicit domain(sphere_function,
		K::Sphere_3(CGAL::ORIGIN, 2.));
	// Mesh criteria
	CGAL::default_random = CGAL::Random(0); // for deterministic result. http://cgal-discuss.949826.n4.nabble.com/3D-mesh-generation-random-result-td4655452.html
	Mesh_criteria criteria(facet_angle = 30, facet_size = 0.1, facet_distance = 0.025,
		cell_radius_edge_ratio = 2, cell_size = 0.1);

	// Mesh generation
	c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);
	return true;
}
