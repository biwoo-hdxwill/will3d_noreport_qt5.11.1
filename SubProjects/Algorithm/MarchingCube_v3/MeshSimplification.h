#pragma once
#include "marchingcube_v3_global.h"
#include <functional>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include "../Util/PointProcessing/Edge.h"

class MARCHINGCUBE_V3_EXPORT MeshSimplification {
public:
	typedef std::pair<int, int> Edge; ///< edge 자료형
public:
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static bool MeshSimplification::findConnectedComponents(std::vector<std::vector<int>>& connectedComponents, const std::vector<Edge>& edges);
	///
	/// @brief	주어진 edges들에서 connected component를 찾음
	///
	/// @param [in,out]	connectedComponents	각 connected components를 이루는 edge의 idx들을 2차원 배열로 저장.
	/// @param	edges					   	connected component를 찾을 edge들
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool findConnectedComponents(std::vector<std::vector<int>>& connectedComponents,
										const std::vector<Edge>& edges);
	static bool findConnectedComponents(std::vector<std::vector<int>>& connectedComponents,
										const std::vector<tora::Edge>& edges);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static bool MeshSimplification::execute(std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, float arTol, float areaTol);
	///
	/// @brief	Executes.
	///
	/// @param [in,out]	points   	vertex of mesh
	/// @param [in,out]	triangles	idx of mesh
	/// @param	arTol			 	Aspect Ratio tolerance. 삼각형이 AR값이 이 값보다 작으면 삭제시킴.
	/// @param	areaTol			 	Area tolerance. 삼각형의 넓이가 이 값보다 작으면 삭제시킴.
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static bool execute(std::vector<glm::vec3>& points,
						std::vector<std::vector<int>>& triangles,
						float arTol, float areaTol);

	static bool executeIsolatedComponentSelectMax(std::vector<glm::vec3>& points,
												  std::vector<std::vector<int>>& triangles);
	static bool executeIsolatedComponent(std::vector<glm::vec3>& points,
										 std::vector<std::vector<int>>& triangles,
										 std::function<int(int)>& remap, int threshold);
	static bool executeIsolatedComponent(std::vector<glm::vec3>& points,
										 std::vector<std::vector<int>>& triangles, int threshold);	

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	static void MeshSimplification::avgAspectRatio(float& avgAR, float& avgAreaAR, const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles);
	///
	/// @brief	Average aspect ratio.
	///
	/// @param [in,out]	avgAR	 	mesh의 Aspect Ratio의 평균.
	/// @param [in,out]	avgAreaAR	mesh의 (Aspect Ratio*삼각형 넓이)의 평균
	/// @param	points			 	vertex of mesh
	/// @param	triangles		 	idx of mesh
	///////////////////////////////////////////////////////////////////////////////////////////////////
	static void avgAspectRatio(float& avgAR, float& avgAreaAR, const std::vector<glm::vec3>& points,
							   const std::vector<std::vector<int>>& triangles);
};
