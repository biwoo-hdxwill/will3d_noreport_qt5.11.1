#pragma once
#pragma message("# MeshMove3d_v2/Kcoef.h visited")
#include "MeshMove3d_v2_global.h"
#include <map>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <eigen3/Eigen/Dense>
#else
#include <gl/glm/glm.hpp>
#include <Eigen/Dense>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	Kcoef
///
/// @brief		K sparse matrix를 만들기위한 coefficient map임.
/// 			Key는 (int, int) edge이고, value는 glm::mat3이다.
/// 			
/// @details	kcoef.get(glm::mat3& returnValue, int edge.v0, int edge.v1),
/// 			kcoef.set(int edge.v0, int edge.v1, glm::mat3 setValue)와 같이 사용함
///////////////////////////////////////////////////////////////////////////////////////////////////
class MESHMOVE3D_V2_EXPORT Kcoef{
public:
	typedef std::pair<int, int> Key; ///< Key (int, int)는 edge를 나타냄
	typedef glm::mat3 Val; ///< Value는 3x3 matrix
	typedef std::pair<Key, Val> Elem; ///< (Key, Val) pair
protected:
	std::map<Key, Val> m_data; ///< 내부적인 map구조
public:

	Kcoef(); ///< 생성자

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool Kcoef::get(Val& val, int k0, int k1) const;
	///
	/// @brief	edge (k0, k1)에 해당하는 값을 get
	///
	/// @param [in,out]	val	(k0, k1)에 할당된 3x3 matrix
	/// @param	k0		   	edge의 첫번째 번호
	/// @param	k1		   	edge의 두번재 번호
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool get(Val& val, int k0, int k1) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool Kcoef::get(Eigen::Matrix3f& val, int k0, int k1) const;
	///
	/// @brief	edge (k0, k1)에 해당하는 값을 get
	///
	/// @param [in,out]	val	(k0, k1)에 할당된 3x3 matrix
	/// @param	k0		   	edge의 첫번째 번호
	/// @param	k1		   	edge의 두번재 번호
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool get(Eigen::Matrix3f& val, int k0, int k1) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	const Val& Kcoef::set(int k0, int k1, const Val& val);
	///
	/// @brief	Sets.
	///
	/// @param	k0 	edge의 첫번째 번호
	/// @param	k1 	edge의 두번째 번호
	/// @param	val	(k0, k1)에 할당된 3x3 matrix
	///
	/// @return	val과 같음
	///////////////////////////////////////////////////////////////////////////////////////////////////
	const Val& set(int k0, int k1, const Val& val);

	void clear(); ///< m_data.clear()
};
