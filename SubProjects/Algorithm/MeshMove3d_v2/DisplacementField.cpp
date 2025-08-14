#pragma message("MeshMove3d_v2/DisplacementField.cpp visited")
#include "meshmove3d_v2_global.h"
#include "DisplacementField.h"
#include <iostream>
#include <exception>
using namespace std;

void DisplacementField::_init(){
	m_fieldPoints.clear();
	m_fieldTetrahedrons.clear();
	m_fieldTetrahedronCircumcenters.clear();
	m_fieldDisplacements.clear();
}
bool DisplacementField::_constr(
	std::vector<glm::vec3> fieldPoints,
	std::vector<std::vector<int>> fieldTetrahedrons,
	std::vector<glm::vec3> fieldDisplacements
	){
	try{
		m_fieldPoints = fieldPoints;
		m_fieldTetrahedrons = fieldTetrahedrons;
		m_fieldDisplacements = fieldDisplacements;
		m_fieldTetrahedronCircumcenters.resize(m_fieldTetrahedrons.size());
		for (int i = 0; i < m_fieldTetrahedrons.size(); i++){
			const auto& tetra = m_fieldTetrahedrons[i];
			if (tetra.size() != 4){
				throw runtime_error("there is tetrahedron's size != 4");
			}
			const glm::vec3& p0 = m_fieldPoints[tetra[0]];
			const glm::vec3& p1 = m_fieldPoints[tetra[1]];
			const glm::vec3& p2 = m_fieldPoints[tetra[2]];
			const glm::vec3& p3 = m_fieldPoints[tetra[3]];
			m_fieldTetrahedronCircumcenters[i] = _getCircumcenter(p0, p1, p2, p3);
		}
		calculateKnn();
		return true;
	}
	catch (runtime_error& e){
		cout << "DisplacementField::_constr: " << e.what() << endl;
		return false;
	}
}
glm::vec4 DisplacementField::_getBarycentric(
	const glm::vec3& p,
	const glm::vec3& p0,
	const glm::vec3& p1,
	const glm::vec3& p2,
	const glm::vec3& p3
	) const{
	glm::mat3 V(p1 - p0, p2 - p0, p3 - p0);
	glm::vec3 temp = glm::inverse(V) * (p - p0);
	return glm::vec4(1.0 - temp[0] - temp[1] - temp[2], temp[0], temp[1], temp[2]);
}
glm::vec3 DisplacementField::_getCircumcenter(
	const glm::vec3& p0,
	const glm::vec3& p1,
	const glm::vec3& p2,
	const glm::vec3& p3
	) const{
	// formula for circumcenter of tetrahedron
	// http://www.wolframalpha.com/input/?i=circumsphere
	float r2 = glm::determinant(
		glm::mat4(
		glm::vec4(p0.x, p1.x, p2.x, p3.x),
		glm::vec4(p0.y, p1.y, p2.y, p3.y),
		glm::vec4(p0.z, p1.z, p2.z, p3.z),
		glm::vec4(1.f, 1.f, 1.f, 1.f)
		));
	float x = glm::determinant(
		glm::mat4(
		glm::vec4(
		glm::length(p0)*glm::length(p0),
		glm::length(p1)*glm::length(p1),
		glm::length(p2)*glm::length(p2),
		glm::length(p3)*glm::length(p3)),
		glm::vec4(p0.y, p1.y, p2.y, p3.y),
		glm::vec4(p0.z, p1.z, p2.z, p3.z),
		glm::vec4(1.f, 1.f, 1.f, 1.f)
		));
	float y = glm::determinant(
		glm::mat4(
		glm::vec4(
		glm::length(p0)*glm::length(p0),
		glm::length(p1)*glm::length(p1),
		glm::length(p2)*glm::length(p2),
		glm::length(p3)*glm::length(p3)),
		glm::vec4(p0.x, p1.x, p2.x, p3.x),
		glm::vec4(p0.z, p1.z, p2.z, p3.z),
		glm::vec4(1.f, 1.f, 1.f, 1.f)
		));
	float z = glm::determinant(
		glm::mat4(
		glm::vec4(
		glm::length(p0)*glm::length(p0),
		glm::length(p1)*glm::length(p1),
		glm::length(p2)*glm::length(p2),
		glm::length(p3)*glm::length(p3)),
		glm::vec4(p0.x, p1.x, p2.x, p3.x),
		glm::vec4(p0.y, p1.y, p2.y, p3.y),
		glm::vec4(1.f, 1.f, 1.f, 1.f)
		));
	return glm::vec3(x / (2.f*r2), -y / (2.f*r2), z / (2.f*r2));
}
bool DisplacementField::isInner(const glm::vec3& point) const{
	float ep = 5e-1;
	int tidx = m_knn.knnSearch(point);
	const auto& tetra = m_fieldTetrahedrons[tidx];
	glm::vec4 baryCoord = _getBarycentric(
		point, 
		m_fieldPoints[tetra[0]],
		m_fieldPoints[tetra[1]],
		m_fieldPoints[tetra[2]],
		m_fieldPoints[tetra[3]]);
	return
		(0 - ep <= baryCoord[0] && baryCoord[0] <= 1 + ep) &&
		(0 - ep <= baryCoord[1] && baryCoord[1] <= 1 + ep) &&
		(0 - ep <= baryCoord[2] && baryCoord[2] <= 1 + ep) &&
		(0 - ep <= baryCoord[3] && baryCoord[3] <= 1 + ep);
}
DisplacementField::DisplacementField(){
	_init();
}
DisplacementField::DisplacementField(
	const std::vector<glm::vec3>& fieldPoints,
	const std::vector<std::vector<int>>& fieldTetrahedrons,
	const std::vector<glm::vec3>& fieldDisplacements
	){
	_constr(fieldPoints, fieldTetrahedrons, fieldDisplacements);
}
bool DisplacementField::calculateKnn(){
	m_knn = KnnWrapper(m_fieldTetrahedronCircumcenters);
	return true;
}
bool DisplacementField::execute(
	std::vector<glm::vec3>& displacements,		// displacements at input points
	const std::vector<glm::vec3>& queryPoints		// points where you want interpolated displacement
	){
	try{
		int nQuerys = queryPoints.size();
		displacements.resize(nQuerys, glm::vec3(0.f, 0.f, 0.f));
		vector<int> tidxs;
		if (!m_knn.knnSearch(tidxs, queryPoints)){
			throw runtime_error("there is knnSearch err");
		}
		for (int i = 0; i < nQuerys; i++){
			int tidx = tidxs[i];
			const auto& tetra = m_fieldTetrahedrons[tidx];
			glm::vec4 baryCoord = _getBarycentric(
				queryPoints[i],
				m_fieldPoints[tetra[0]],
				m_fieldPoints[tetra[1]],
				m_fieldPoints[tetra[2]],
				m_fieldPoints[tetra[3]]
				);
			glm::vec3 displacement = glm::vec3(0.f, 0.f, 0.f);
			for (int j = 0; j < 4; j++){
				displacement += baryCoord[j] * m_fieldDisplacements[tetra[j]];
			}
			displacements[i] = displacement;
		}
		return true;
	}
	catch (runtime_error& e){
		cout << "DisplacementField::getDisplacements: " << e.what() << endl;
		return false;
	}
}
