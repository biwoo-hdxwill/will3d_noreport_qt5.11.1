#pragma once

#include "meshmove3d_v2_global.h"
#include <vector>
#include <memory>
#include <gl/glew.h>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include <QtWidgets/QOpenGLWidget>
class MeshFillVertexColor2;
class QOpenGLWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	DisplacementSurface2
///
/// @brief	이 클레스는 이동량이 저장된 mesh surface를 이용해 mesh surface 근방의 원하는 위치에서의 
/// 		이동량을 구해준다. mesh surafce는 정점(m_surfPoints)과 인덱스(m_surfTriangles)로 이루어져있고,
/// 		각 정점(m_surfPoints[i])마다 대응되는 이동량(m_surfDisplacements[i])가 있다. 이동량을 계산할때
/// 		gl의 렌더링을 이용하는데 CT volume data가 (0,-1,0)에서 (0,0,0)으로 바라볼때 얼굴면이 나온다고 
/// 		가정하고 있다.
/// 		
/// @detail 동작 방식은 (0,-1,0)에서 (0,0,0)으로 바라볼때의 mesh surface를 그린다. 이때 각 정점(m_surfPoints)마다
/// 		m_surfDisplacements를 컬러로 하여 fbo에 렌더링하고 texture를 뽑아낸다. mesh 근방에서 원하는 
/// 		위치들(displacements)에서의 이동량은 이 texture를 읽어서 구한다.
/// 		
///////////////////////////////////////////////////////////////////////////////////////////////////
class MESHMOVE3D_V2_EXPORT DisplacementSurface2{
public: /* members */
	QOpenGLWidget* m_glWidget;///< gl 렌더링을 위한 엔진
	std::vector<glm::vec3>* m_surfPoints; ///< 이동량이 저장된 mesh surface를 이루는 정점
	std::vector<std::vector<int>>* m_surfTriangles; ///< 이동량이 저장된 mesh surface를 이루는 인덱스
	std::vector<glm::vec3>* m_surfDisplacements; ///< 이동량이 저장된 mesh surface의 정점의 이동량
	MeshFillVertexColor2* m_surf2; ///< 이동량이 저장된 mesh surface. 컬러는 이동량이다.

private:
	int m_fboWidth;
	int m_fboHeight;

public: /* methods */
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	DisplacementSurface2::DisplacementSurface2( const std::vector<glm::vec3>& surfPoints, const std::vector<std::vector<int>>& surfTriangles, const std::vector<glm::vec3>& surfDisplacements);
	///
	/// @brief	생성자
	///
	/// @param	surfPoints		 	이동량이 저장된 mesh surface를 이루는 정점
	/// @param	surfTriangles	 	이동량이 저장된 mesh surface를 이루는 인덱스
	/// @param	surfDisplacements	이동량이 저장된 mesh surface의 정점의 이동량
	///////////////////////////////////////////////////////////////////////////////////////////////////
	DisplacementSurface2(
		QOpenGLWidget* glWidget,
		std::vector<glm::vec3>& surfPoints,
		std::vector<std::vector<int>>& surfTriangles,
		std::vector<glm::vec3>& surfDisplacements);

	/**********************************************************************************************//**
	 * @fn	DisplacementSurface2::~DisplacementSurface2();
	 *
	 * @brief	Destructor.
	 *
	 * @author	Jun
	 * @date	2017-07-04
	 **************************************************************************************************/
	~DisplacementSurface2();

	void Deinit();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementSurface2::execute( std::vector<glm::vec3>& displacements, std::vector<float>& dists, const std::vector<glm::vec3>& queryPoints, float x0, float x1, float y0, float y1, float z0, float z1 );
	///
	/// @brief	이동량을 구해주는 메서드
	///
	/// @param [in,out]	displacements	원하는 위치에서 이동량들
	/// @param [in,out]	dists		 	이동량을 원하는 위치와 mesh surface에 맵핑된 점과의 거리들
	/// @param [in,out]	valid			해당 위치에서 tetrahedron surface가 존재하는가를 의미
	/// @param	queryPoints			 	이동량을 원하는 위치들
	/// @param	x0					 	queryPoints를 감싸는 bounding box xmin
	/// @param	x1					 	queryPoints를 감싸는 bounding box xmax
	/// @param	y0					 	queryPoints를 감싸는 bounding box ymin
	/// @param	y1					 	queryPoints를 감싸는 bounding box ymax
	/// @param	z0					 	queryPoints를 감싸는 bounding box zmin
	/// @param	z1					 	queryPoints를 감싸는 bounding box zmax
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool execute(
		std::vector<glm::vec3>& displacements, 
		std::vector<float>& dists,
		std::vector<char>& valid,
		const std::vector<glm::vec3>& queryPoints,
		float x0, float x1, float y0, float y1, float z0, float z1
	);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementSurface2::execute( std::vector<glm::vec3>& displacements, const std::vector<glm::vec3>& queryPoints, float x0, float x1, float y0, float y1, float z0, float z1 );
	///
	/// @brief	이동량을 구해주는 메서드
	///
	/// @param [in,out]	displacements	원하는 위치에서 이동량들
	/// @param	queryPoints			 	이동량을 원하는 위치들
	/// @param	x0					 	queryPoints를 감싸는 bounding box xmin
	/// @param	x1					 	queryPoints를 감싸는 bounding box xmax
	/// @param	y0					 	queryPoints를 감싸는 bounding box ymin
	/// @param	y1					 	queryPoints를 감싸는 bounding box ymax
	/// @param	z0					 	queryPoints를 감싸는 bounding box zmin
	/// @param	z1					 	queryPoints를 감싸는 bounding box zmax
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool execute(
		std::vector<glm::vec3>& displacements,
		const std::vector<glm::vec3>& queryPoints,
		float x0, float x1, float y0, float y1, float z0, float z1
	);
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool DisplacementSurface2::execute( std::vector<glm::vec3>& displacements, const std::vector<glm::vec3>& queryPoints );
	///
	/// @brief	이동량을 구해주는 메서드. queryPoints들을 둘러싸는 bounding box를 메서드 내에서 알아서 구함.
	///
	/// @param [in,out]	displacements	원하는 위치에서 이동량들
	/// @param	queryPoints			 	이동량을 원하는 위치들
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool execute(
		std::vector<glm::vec3>& displacements,
		const std::vector<glm::vec3>& queryPoints
	);
	bool execute(
		std::vector<glm::vec3>& displacements,
		std::vector<float>& dists,
		std::vector<char>& valid,
		const std::vector<glm::vec3>& queryPoints
	);
};
