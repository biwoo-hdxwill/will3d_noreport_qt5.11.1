#include "C3t3Wrapper.h"
#include <iostream>
#include <map>

// cf) template 명시적 인스턴스화 https://wikidocs.net/436
template bool TETRAHEDRONGENERATOR_EXPORT C3t3Wrapper::convert<C3t3_implicit>(
    std::vector<glm::vec3>& points,
    std::vector<std::vector<int>>& cells,
    std::vector<std::vector<int>>& facets,
    const C3t3_implicit& c3t3
    );
template bool TETRAHEDRONGENERATOR_EXPORT C3t3Wrapper::convert<C3t3_polyhedral>(
    std::vector<glm::vec3>& points,
    std::vector<std::vector<int>>& cells,
    std::vector<std::vector<int>>& facets,
    const C3t3_polyhedral& c3t3
    );

template<class C3t3>
bool C3t3Wrapper::convert(
	std::vector<glm::vec3>& points,
	std::vector<std::vector<int>>& cells,
	std::vector<std::vector<int>>& facets,
	const C3t3& c3t3
){
	using namespace std;
	typedef typename C3t3::Triangulation Tr;
	typedef typename C3t3::Facets_in_complex_iterator Facet_iterator;
	typedef typename C3t3::Cells_in_complex_iterator Cell_iterator;
	typedef typename Tr::Finite_vertices_iterator Finite_vertices_iterator;
	typedef typename Tr::Vertex_handle Vertex_handle;
	typedef typename Tr::Point Point_3;

	std::map<Vertex_handle, int> V;

	const Tr& tr = c3t3.triangulation();
	points.clear();
	points.reserve(tr.number_of_vertices());
	//cout << "tr.number_of_vertices=" << tr.number_of_vertices() << endl;
	int inum = 0;
	for (Finite_vertices_iterator it = tr.finite_vertices_begin(); it != tr.finite_vertices_end(); ++it){
		const Point_3& p = it->point();
		points.push_back(glm::vec3(p.x(), p.y(), p.z()));
		V[it] = inum++;
	}

	facets.clear();
	facets.reserve(c3t3.number_of_facets_in_complex() * 2);
	//cout << "c3t3.number_of_facets_in_complex=" << c3t3.number_of_facets_in_complex() << endl;
	for (Facet_iterator it = c3t3.facets_in_complex_begin(); it != c3t3.facets_in_complex_end(); ++it){

		vector<int> facet;
		facets.reserve(3);

		for (int i = 0; i < 4; i++){
			if (i != it->second){
				const Vertex_handle& vh = it->first->vertex(i);
				facet.push_back(V[vh]);
			}
		}

		facets.push_back(facet);
	}

	cells.clear();
	cells.reserve(c3t3.number_of_cells_in_complex());
	//cout << "c3t3.number_of_cells_in_complex=" << c3t3.number_of_cells_in_complex() << endl;
	for (Cell_iterator it = c3t3.cells_in_complex_begin(); it != c3t3.cells_in_complex_end(); ++it){
		vector<int> cell;
		cell.reserve(4);
		for (int i = 0; i < 4; i++){
			cell.push_back(V[it->vertex(i)]);
		}
		cells.push_back(cell);
	}

	return true;
}
