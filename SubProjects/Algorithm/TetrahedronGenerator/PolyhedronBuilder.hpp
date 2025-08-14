#pragma once
#include <vector>
#include <sstream>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
//#include "PolyhedronTypedef.h"
#include <Util/Core/coutOverloading.h>

/**********************************************************************************************//**
 * @class	PolyhedronBuilder
 *
 * @brief	원하는 vertexs of triangles, idxs of triangles로 cgal의 Polyhedron객체를 만드는 빌더.
 *
 * @author	Hosan
 * @date	2016-05-13
 *
 * @tparam	HDS	Type of the hds.
 **************************************************************************************************/

template <class HDS>
class PolyhedronBuilder : public CGAL::Modifier_base<HDS> {
public:
	typedef typename HDS::Halfedge_handle   Halfedge_handle;
	typedef CGAL::Polyhedron_incremental_builder_3<HDS> Builder;
public:
	std::vector<glm::vec3> points;
	std::vector<std::vector<int>> triangles;
	std::vector<int> invalidTriangleIdxs;
	PolyhedronBuilder(const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles)
		: points(points), triangles(triangles) {
	}
	void operator()(HDS& hds) {
		using namespace std;
		typedef typename HDS::Vertex	Vertex;
		typedef typename Vertex::Point	Point;

		try {
			// create a cgal incremental builder
			Builder builder(hds, true);

			//auto l_vert = [](HalfedgeDS& hdss){
			//	auto it = hdss.faces_end();
			//	it--;
			//	auto it2 = it->facet_begin();
			//	do{
			//		cout << std::distance(hdss.vertices_begin(), it2->vertex()) << " ";
			//	} while (++it2 != it->facet_begin());
			//	cout << endl;
			//};

			builder.begin_surface(points.size(), triangles.size(), 0, Builder::ABSOLUTE_INDEXING);
			for (const auto& p : points) {
				builder.add_vertex(Point(p[0], p[1], p[2]));
			}
			int triIdx = -1;
			for (int i = 0; i < triangles.size(); i++) {
				const auto& triangle = triangles[i];
				triIdx++;
				if (!builder.test_facet(triangle.begin(), triangle.end())) {
					invalidTriangleIdxs.push_back(i);
					continue;
				}
				builder.begin_facet();
				builder.add_vertex_to_facet(triangle[0]);
				builder.add_vertex_to_facet(triangle[1]);
				builder.add_vertex_to_facet(triangle[2]);
				auto halfedge = builder.end_facet();
				//@ this code may be unreachable
				if (halfedge == Halfedge_handle()) {
					stringstream ss;
					ss << "add_vertex_to_facet runtime err at" << __FILE__ << "(" << __LINE__ << ")";
					throw std::runtime_error(ss.str().c_str());
				}
			}
			builder.end_surface();
		} catch (std::runtime_error& e) {
			cout << "PolyhedronBuilder::operator(): " << e.what() << endl;
		}
	}
};