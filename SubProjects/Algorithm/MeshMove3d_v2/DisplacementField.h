#pragma once
#pragma message("MeshMove3d_v2/DisplacementField.h visited")
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include <Util/KnnWrapper/KnnWrapper.h>
#include "meshmove3d_v2_global.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class		DisplacementField
/// 			
/// @brief		이 클레스는 tetrahedron들로 이루어진 3d model에 대해 각 꼭지점 마다 displacements를 할당해서
/// 			초기화 한다. 그 다음 3d model 내부에 존재하는 queryPoint에 대해서 displacements를 barycentric
/// 			interpolation해서 구해준다.
/// 			
///	@details	내부적으로 queryPoint가 포함된 tetrahedron은 k-nearest neighbor를 사용하여 찾는다. 이게 가능한
///				이유는 tetrahedron이 delaunay tetrahedralization를 통해 만들어졌다고 가정했기 때문이다.
///				참고로 cgal의 "3d Mesh Generation"으로 tetrahedron은 delaunay tetrahedralization을 지원한다.
///				cgal의 "3d Mesh Generation"은 "Tetrahedron Generator"라는 프로젝트로 Wrapping 해뒀다.
///////////////////////////////////////////////////////////////////////////////////////////////////
class MESHMOVE3D_V2_EXPORT DisplacementField{
public: /* members */

	std::vector<glm::vec3> m_fieldPoints; ///< vertexs of tetrahedron.
	std::vector<std::vector<int>> m_fieldTetrahedrons; ///< idxs of tetrahedrons.
	std::vector<glm::vec3> m_fieldTetrahedronCircumcenters; ///< 각 tetrahedron의 외접원의 중심.
	std::vector<glm::vec3> m_fieldDisplacements; ///< 각 vertexs of tetrahedron 마다 할당된 displacement.
	KnnWrapper m_knn; ///< k-nearest neighbor wrapper.
protected:  /* methods */
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void DisplacementField::_init();
	///
	/// @brief	초기화
	///////////////////////////////////////////////////////////////////////////////////////////////////
	void _init();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	virtual bool DisplacementField::_constr( std::vector<glm::vec3> fieldPoints, std::vector<std::vector<int>> fieldTetrahedrons, std::vector<glm::vec3> fieldDisplacements );
	///
	/// @brief	생성자.
	///
	/// @param	fieldPoints		  	The field points.
	/// @param	fieldTetrahedrons 	The field tetrahedrons.
	/// @param	fieldDisplacements	The field displacements.
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	virtual bool _constr(
		std::vector<glm::vec3> fieldPoints,
		std::vector<std::vector<int>> fieldTetrahedrons,
		std::vector<glm::vec3> fieldDisplacements
		);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	glm::vec4 DisplacementField::_getBarycentric( const glm::vec3& p, const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3 ) const;
	///
	/// @brief	p0, p1, p2, p3에 대한 p의 3d barycentric coordinate를 계산함.
	///
	/// @param	p 	barycentric coordinate를 구하려는 점.
	/// @param	p0	tetrahedron 1번째 vertex.
	/// @param	p1	tetrahedron 2번째 vertex.
	/// @param	p2	tetrahedron 3번째 vertex.
	/// @param	p3	tetrahedron 4번째 vertex.
	///
	/// @return	p의 barycentric coordinate.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	glm::vec4 _getBarycentric(
		const glm::vec3& p,
		const glm::vec3& p0,
		const glm::vec3& p1,
		const glm::vec3& p2,
		const glm::vec3& p3
		) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	glm::vec3 DisplacementField::_getCircumcenter( const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3 ) const;
	///
	/// @brief	p0, p1, p2, p3로 이루어진 사면체에 대한 외접원의 중심을 계산함.
	///			http://www.wolframalpha.com/input/?i=circumsphere 참조.
	///			
	/// @param	p0	tetrahedron 1번째 vertex.
	/// @param	p1	tetrahedron 2번째 vertex.
	/// @param	p2	tetrahedron 3번째 vertex.
	/// @param	p3	tetrahedron 4번째 vertex.
	///
	/// @return	p0, p1, p2, p3로 이루어진 사면체에 대한 외접원의 중심.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	glm::vec3 _getCircumcenter(
		const glm::vec3& p0,
		const glm::vec3& p1,
		const glm::vec3& p2,
		const glm::vec3& p3
		) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementField::isInner(const glm::vec3& point) const;
	///
	/// @brief	어떤 점이 tetrahedrons 내부인지 외부인지 확인한다.
	///
	/// @param	point	이 점이 tetrahedrons 내부인지 외부인지 확인한다.
	///
	/// @return	true if inner, false if not.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool isInner(const glm::vec3& point) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementField::calculateKnn();
	///
	/// @brief	tetrahedron의 외접원의 중심으로 k-nearest neighbor 객체를 초기화함.
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool calculateKnn();

public:

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	DisplacementField::DisplacementField();
	///
	/// @brief	생성자.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	DisplacementField();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	DisplacementField::DisplacementField(std::vector<glm::vec3> fieldPoints, std::vector<std::vector<int>> fieldTetrahedrons, std::vector<glm::vec3> fieldDisplacements );
	///
	/// @brief	생성자.
	///
	/// @param	fieldPoints		  	vertexs of tetrahedrons
	/// @param	fieldTetrahedrons 	idxs of tetrahedrons
	/// @param	fieldDisplacements	각 vertex of tetrahedrons 마다 할당된 displacment
	///////////////////////////////////////////////////////////////////////////////////////////////////
	DisplacementField(
		const std::vector<glm::vec3>& fieldPoints,
		const std::vector<std::vector<int>>& fieldTetrahedrons,
		const std::vector<glm::vec3>& fieldDisplacements
		);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementField::execute( std::vector<glm::vec3>& displacementsOut, const std::vector<glm::vec3>& queryPoints );
	///
	/// @brief	queryPoints들에 대해 displacements를 계산함.
	///
	/// @param [in,out]	displacements		각 queryPoints마다 구해진 displacement.
	/// @param	queryPoints					displacements를 구하려는 위치.
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool execute(
		std::vector<glm::vec3>& displacements,			// displacements at input points
		const std::vector<glm::vec3>& queryPoints		// points where you want interpolated displacement
		);
};
