#pragma once
#pragma message("# TetrahedronGenerator/PolyhedronIOWrapperT.h visited")
#include <vector>
#include <string>
#include <functional>
#include <set>
#include <fstream>
#include <sstream>
#include <exception>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include "PolyhedronBuilder.hpp"
#include "TetrahedronGenerator_global.h"

/**********************************************************************************************//**
 * @class	TETRAHEDRONGENERATOR_EXPORT
 *
 * @brief	cgal의 polyhedron과 vertex of triangles, idxs of triangles 사이를 변환시켜주는 클레스.
 *
 * @author	Hosan
 * @date	2016-05-13
 **************************************************************************************************/

class TETRAHEDRONGENERATOR_EXPORT PolyhedronIOWrapperT{
public:

	template<class Polyhedron>
	static void toPolyhedron(
		Polyhedron&								polyhedron,		// out
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	template<class Polyhedron>
	static void toPolyhedron(
		Polyhedron&								polyhedron,		// out 
		std::vector<glm::vec3>&					points_out,		// out valid points
		std::vector<std::vector<int>>&			triangles_out,	// out valid triangles
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	template<class Polyhedron>
	static void toPolyhedron(
		Polyhedron&								polyhedron,
		std::vector<glm::vec3>&					points_out,		// out valid points
		std::vector<std::vector<int>>&			triangles_out,	// out valid triangles
		std::vector<std::vector<int>>&			triangles_invalid,	// out invalid triangles
		const std::vector<glm::vec3>&			points,			// in points
		const std::vector<std::vector<int>>&	triangles		// in triangles
		);

	template<class Polyhedron>
	static void fromPolyhedron(
		std::vector<glm::vec3>&					points,			// out points
		std::vector<std::vector<int>>&			triangles,		// out triangles
		const Polyhedron&						polyhedron		// in
		);

	template<class Polyhedron>
	static bool writeOffFile(const Polyhedron& polyhedron, const std::string& fpath);

	template<class Polyhedron>
	static bool writeOffFile(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::string& fpath);
};




template<class Polyhedron>
void PolyhedronIOWrapperT::toPolyhedron(
	Polyhedron&								polyhedron,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
) {
	PolyhedronBuilder<typename Polyhedron::HalfedgeDS> builder(points, triangles);
	polyhedron.delegate(builder);
}

template<class Polyhedron>
void PolyhedronIOWrapperT::toPolyhedron(
	Polyhedron&								polyhedron,
	std::vector<glm::vec3>&					points_out,
	std::vector<std::vector<int>>&			triangles_out,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
) {
	PolyhedronIOWrapperT::toPolyhedron(
		polyhedron,
		points_out,
		triangles_out,
		std::vector<std::vector<int>>(),
		points,
		triangles
	);
}

template<class Polyhedron>
void PolyhedronIOWrapperT::toPolyhedron(
	Polyhedron&								polyhedron,
	std::vector<glm::vec3>&					points_out,
	std::vector<std::vector<int>>&			triangles_out,
	std::vector<std::vector<int>>&			triangles_invalid,
	const std::vector<glm::vec3>&			points,
	const std::vector<std::vector<int>>&	triangles
) {
	polyhedron.clear();
	PolyhedronBuilder<HalfedgeDS> builder(points, triangles);
	polyhedron.delegate(builder);
	set<int> invalidTriangleIdxs(builder.invalidTriangleIdxs.begin(), builder.invalidTriangleIdxs.end());
	std::function<int(int)> l_remap;
	std::function<bool(int)> l_removeTriangle =
		[invalidTriangleIdxs](int tidx) {
		return invalidTriangleIdxs.find(tidx) != invalidTriangleIdxs.end();
	};
	points_out = points;
	triangles_out = triangles;
	MeshCompacter::executeRemoveMeshByTriangleCriteria(points_out, triangles_out, l_remap, l_removeTriangle);
	triangles_invalid.clear();
	triangles_invalid.reserve(invalidTriangleIdxs.size());
	//wrong //std::copy_if(invalidTriangleIdxs.begin(), invalidTriangleIdxs.end(), std::back_inserter(triangles_invalid), l_removeTriangle);
	for (const auto& tidx : invalidTriangleIdxs) {
		triangles_invalid.push_back(triangles[tidx]);
	}
}


template<class Polyhedron>
void PolyhedronIOWrapperT::fromPolyhedron(
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
		std::vector<int> triangle;
		Halfedge_facet_const_circulator it2 = it->facet_begin();
		do {
			triangle.push_back(idxMap[it2->vertex()]);
		} while (++it2 != it->facet_begin());
		triangles.push_back(triangle);
	}
}

template<class Polyhedron>
bool PolyhedronIOWrapperT::writeOffFile(const Polyhedron& polyhedron, const std::string& fpath) {
	typedef K::Point_3												Point_3;
	typedef Polyhedron::Facet_iterator							    Facet_iterator;
	typedef Polyhedron::Facet_const_iterator						Facet_const_iterator;
	typedef Polyhedron::Halfedge_around_facet_circulator			Halfedge_facet_circulator;
	typedef Polyhedron::Halfedge_around_facet_const_circulator		Halfedge_facet_const_circulator;
	typedef Polyhedron::Point_const_iterator						Point_const_iterator;
	try {
		{
			std::ofstream out(fpath);
			if (!out) {
				std::stringstream ss;
				ss << "file open err (" << fpath << ")";
				throw std::exception(ss.str().c_str());
			}

			out << "OFF" << std::endl << polyhedron.size_of_vertices() << ' '
				<< polyhedron.size_of_facets() << " 0" << std::endl;

			for (Point_const_iterator it = polyhedron.points_begin(); it != polyhedron.points_end(); ++it) {
				out << it->x() << " " << it->y() << " " << it->z() << std::endl;
			}

			int cnt = 0;
			for (Facet_const_iterator it = polyhedron.facets_begin(); it != polyhedron.facets_end(); ++it, cnt++) {
				Halfedge_facet_const_circulator it2 = it->facet_begin();
				CGAL_assertion(CGAL::circulator_size(it2) == 3); // only support triangle in my code
				std::vector<int> triangle;
				triangle.reserve(3);
				do {
					triangle.push_back(std::distance(polyhedron.vertices_begin(), it2->vertex()));
				} while (++it2 != it->facet_begin());
				if (!triangle.empty()) {
					out << triangle.size() << "\t" << triangle[0] << " " << triangle[1] << " " << triangle[2] << std::endl;
				}
			}
			out.close();
		}
		return true;
	}
	catch (std::exception& e) {
		std::cout << "PolyhedronIOWrapperT::writePOlyhedron: " << e.what() << std::endl;
		return false;
	}
}

template<class Polyhedron>
bool PolyhedronIOWrapperT::writeOffFile(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::string& fpath) {
	// Write polyhedron in Object File Format (OFF).
	try {
		std::ofstream out(fpath);
		if (!out) {
			std::stringstream ss;
			ss << "file open err (" << fpath << ")";
			throw std::exception(ss.str().c_str());
		}
		out << "OFF" << std::endl << points.size() << ' '
			<< triangles.size() << " 0" << std::endl;
		for (const auto& point : points) {
			out << point[0] << " " << point[1] << " " << point[2] << std::endl;
		}
		for (const auto& triangle : triangles) {
			out << triangle.size() << "\t" << triangle[0] << " " << triangle[1] << " " << triangle[2] << std::endl;
		}
		out.close();
		return true;
	}
	catch (std::exception& e) {
		std::cout << "PolyhedronIOWrapperT::writePOlyhedron: " << e.what() << std::endl;
		return false;
	}
}