#include "PolyhedronIOWrapper.h"
#include <functional>
#include <set>
#include <fstream>
#include <sstream>
#include <exception>

#include "PolyhedronBuilder.hpp"
#include <Util/MeshGenerator/MeshCompacter.h>

using namespace std;
void PolyhedronIOWrapper::toPolyhedron(
	Polyhedron&								polyhedron,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
){
	PolyhedronBuilder<HalfedgeDS> builder(points, triangles);
	polyhedron.delegate(builder);
}

void PolyhedronIOWrapper::toPolyhedron(
	Polyhedron&								polyhedron,
	std::vector<glm::vec3>&					points_out,
	std::vector<std::vector<int>>&			triangles_out,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
){
	vector<vector<int>> v;
	PolyhedronIOWrapper::toPolyhedron(
		polyhedron,
		points_out,
		triangles_out,
		v,
		points,
		triangles
		);
}

void PolyhedronIOWrapper::toPolyhedron(
	Polyhedron&								polyhedron,
	std::vector<glm::vec3>&					points_out,
	std::vector<std::vector<int>>&			triangles_out,
	std::vector<std::vector<int>>&			triangles_invalid,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
){
	polyhedron.clear();
	PolyhedronBuilder<HalfedgeDS> builder(points, triangles);
	polyhedron.delegate(builder);
	set<int> invalidTriangleIdxs(builder.invalidTriangleIdxs.begin(), builder.invalidTriangleIdxs.end());
	function<int(int)> l_remap;
	function<bool(int)> l_removeTriangle =
	[invalidTriangleIdxs](int tidx){
		return invalidTriangleIdxs.find(tidx) != invalidTriangleIdxs.end();
	};
	points_out = points;
	triangles_out = triangles;
	MeshCompacter::executeRemoveMeshByTriangleCriteria(points_out, triangles_out, l_remap, l_removeTriangle);
	triangles_invalid.clear();
	triangles_invalid.reserve(invalidTriangleIdxs.size());
	//wrong //std::copy_if(invalidTriangleIdxs.begin(), invalidTriangleIdxs.end(), std::back_inserter(triangles_invalid), l_removeTriangle);
	for (const auto& tidx : invalidTriangleIdxs){
		triangles_invalid.push_back(triangles[tidx]);
	}
}


#if 0
void PolyhedronIOWrapper::fromPolyhedron(
	std::vector<glm::vec3>&					points,
	std::vector<std::vector<int>>&			triangles,
	const Polyhedron&						polyhedron
){
	typedef Polyhedron::Facet_iterator							    Facet_iterator;
	typedef Polyhedron::Facet_const_iterator						Facet_const_iterator;
	typedef Polyhedron::Halfedge_around_facet_circulator			Halfedge_facet_circulator;
	typedef Polyhedron::Halfedge_around_facet_const_circulator		Halfedge_facet_const_circulator;
	typedef Polyhedron::Point_const_iterator						Point_const_iterator;
	points.clear();
	points.reserve(polyhedron.size_of_vertices());
	for (Point_const_iterator it = polyhedron.points_begin(); it != polyhedron.points_end(); ++it){
		points.push_back(glm::vec3(it->x(), it->y(), it->z()));
	}
	triangles.clear();
	triangles.reserve(polyhedron.size_of_facets());
	for (Facet_const_iterator it = polyhedron.facets_begin(); it != polyhedron.facets_end(); ++it){
		Halfedge_facet_const_circulator it2 = it->facet_begin();
		CGAL_assertion(CGAL::circulator_size(it2) == 3); // only support triangle in my code
		vector<int> triangle;
		triangle.reserve(3);
		do {
			triangle.push_back(std::distance(polyhedron.vertices_begin(), it2->vertex()));
			
		} while (++it2 != it->facet_begin());
		triangles.push_back(triangle);
	}
}
#else
#include <CGAL/IO/print_wavefront.h>
void PolyhedronIOWrapper::fromPolyhedron(
	std::vector<glm::vec3>&					points,
	std::vector<std::vector<int>>&			triangles,
	const Polyhedron&						polyhedron
) {
	typedef Polyhedron::Facet_iterator							    Facet_iterator;
	typedef Polyhedron::Facet_const_iterator						Facet_const_iterator;
	typedef Polyhedron::Halfedge_around_facet_circulator			Halfedge_facet_circulator;
	typedef Polyhedron::Halfedge_around_facet_const_circulator		Halfedge_facet_const_circulator;
	typedef Polyhedron::Point_const_iterator						Point_const_iterator;
	points.clear();
	points.reserve(polyhedron.size_of_vertices());
	map < Polyhedron::Vertex_const_iterator, int> idxMap;
	int idx = 0;
	for (Polyhedron::Vertex_const_iterator it = polyhedron.vertices_begin(); it != polyhedron.vertices_end(); ++it) {
		const auto& p = it->point();
		points.push_back(glm::vec3(p.x(), p.y(), p.z()));
		idxMap[it] = idx++;
	}
	triangles.clear();
	triangles.reserve(polyhedron.size_of_facets());
	for (Facet_const_iterator it = polyhedron.facets_begin(); it != polyhedron.facets_end(); ++it) {
		vector<int> triangle;
		Halfedge_facet_const_circulator it2 = it->facet_begin();
		do {
			triangle.push_back(idxMap[it2->vertex()]);
		} while (++it2 != it->facet_begin());
		triangles.push_back(triangle);
	}
}
#endif


bool PolyhedronIOWrapper::writeOffFile(const Polyhedron& polyhedron, const std::string& fpath){
	typedef K::Point_3												Point_3;
	typedef Polyhedron::Facet_iterator							    Facet_iterator;
	typedef Polyhedron::Facet_const_iterator						Facet_const_iterator;
	typedef Polyhedron::Halfedge_around_facet_circulator			Halfedge_facet_circulator;
	typedef Polyhedron::Halfedge_around_facet_const_circulator		Halfedge_facet_const_circulator;
	typedef Polyhedron::Point_const_iterator						Point_const_iterator;
	try{
		{
			ofstream out(fpath);
			if (!out){
				stringstream ss;
				ss << "file open err (" << fpath << ")";
				throw runtime_error(ss.str().c_str());
			}

			out << "OFF" << std::endl << polyhedron.size_of_vertices() << ' '
				<< polyhedron.size_of_facets() << " 0" << std::endl;

			for (Point_const_iterator it = polyhedron.points_begin(); it != polyhedron.points_end(); ++it){
				out << it->x() << " " << it->y() << " " << it->z() << endl;
			}

			int cnt = 0;
			for (Facet_const_iterator it = polyhedron.facets_begin(); it != polyhedron.facets_end(); ++it, cnt++){
				Halfedge_facet_const_circulator it2 = it->facet_begin();
				CGAL_assertion(CGAL::circulator_size(it2) == 3); // only support triangle in my code
				vector<int> triangle;
				triangle.reserve(3);
				do {
					triangle.push_back(std::distance(polyhedron.vertices_begin(), it2->vertex()));
				} while (++it2 != it->facet_begin());
				if (!triangle.empty()){
					out << triangle.size() << "\t" << triangle[0] << " " << triangle[1] << " " << triangle[2] << endl;
				}
			}
			out.close();
		}
		return true;
	}
	catch (runtime_error& e){
		cout << "PolyhedronIOWrapper::writePOlyhedron: " << e.what() << endl;
		return false;
	}
}

bool PolyhedronIOWrapper::writeOffFile(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::string& fpath){
	// Write polyhedron in Object File Format (OFF).
	try{
		ofstream out(fpath);
		if (!out){
			stringstream ss;
			ss << "file open err (" << fpath << ")";
			throw runtime_error(ss.str().c_str());
		}
		out << "OFF" << std::endl << points.size() << ' '
			<< triangles.size() << " 0" << std::endl;
		for (const auto& point : points){
			out << point[0] << " " << point[1] << " " << point[2] << endl;
		}
		for (const auto& triangle : triangles){
			out << triangle.size() << "\t" << triangle[0] << " " << triangle[1] << " " << triangle[2] << endl;
		}
		out.close();
		return true;
	}
	catch (runtime_error& e){
		cout << "PolyhedronIOWrapper::writePOlyhedron: " << e.what() << endl;
		return false;
	}
}
