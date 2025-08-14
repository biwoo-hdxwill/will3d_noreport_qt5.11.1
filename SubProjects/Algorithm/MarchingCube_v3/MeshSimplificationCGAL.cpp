#include "MeshSimplificationCGAL.h"

#pragma message("# MeshSimplificationCGAL.cpp visited")
#include <CGAL/Polyhedron_3.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
// Simplification function
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
// Stop-condition policy
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <TetrahedronGenerator/PolyhedronTypedef.h>
#include <TetrahedronGenerator/PolyhedronIOWrapper.h>
using namespace std;
namespace SMS = CGAL::Surface_mesh_simplification;

bool MeshSimplificationCGAL::execute(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& triangles,
	float fraction
) {
	try {
		Polyhedron surface_mesh;
		PolyhedronIOWrapper::toPolyhedron(surface_mesh, points, triangles);

		//std::ifstream is(argv[1]); is >> surface_mesh;
		// This is a stop predicate (defines when the algorithm terminates).
		// In this example, the simplification stops when the number of undirected edges
		// left in the surface mesh drops below the specified number (1000)
		SMS::Count_stop_predicate<Polyhedron> stop(triangles.size() * 3.f / 2 * fraction);

		// This the actual call to the simplification algorithm.
		// The surface mesh and stop conditions are mandatory arguments.
		// The index maps are needed because the vertices and edges
		// of this surface mesh lack an "id()" field.
		int r = SMS::edge_collapse
		(surface_mesh
			, stop
			, CGAL::parameters::vertex_index_map(get(CGAL::vertex_external_index, surface_mesh))
			.halfedge_index_map(get(CGAL::halfedge_external_index, surface_mesh))
			.get_cost(SMS::Edge_length_cost <Polyhedron>())
			.get_placement(SMS::Midpoint_placement<Polyhedron>())
		);
		PolyhedronIOWrapper::fromPolyhedron(points, triangles, surface_mesh);
		return true;
	}
	catch (runtime_error& e) {
		cout << "MeshSimplificationCGAL::execute: " << e.what() << endl;
		return false;
	}

}
