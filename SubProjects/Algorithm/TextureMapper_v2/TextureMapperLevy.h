#pragma once
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Sparse>
#else
#include <gl/glm/glm.hpp>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#endif
#include "texturemapper_v2_global.h"

/**********************************************************************************************//**
 * @class	TextureMapperLevy
 *
 * @brief	3d face mesh에 2d texture를 입히는 기능을 함. 
 * 			"2001, Levy, Constrained Texture Mapping for Polygonal Meshes", 
 * 			"2002, Levy, Least squares conformal maps for automatic texture atlas generation"를 구현함.
 *
 * @author	Hosan
 * @date	2016-05-12
 **************************************************************************************************/

class TEXTUREMAPPER_V2_EXPORT TextureMapperLevy{
public: /* types */
#if 0 /* double */
	typedef double real;
	typedef Eigen::VectorXd VectorXr;
	typedef Eigen::Vector3d Vector3r;
	typedef Eigen::Vector2d Vector2r;
	typedef Eigen::Matrix2d Matrix2r;
	typedef Eigen::MatrixXd MatrixXr;
	typedef Eigen::Matrix<real, 2, 3> Matrix23r;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SparseCoefr;
#else /* float */
	typedef float real;
	typedef Eigen::VectorXf VectorXr;
	typedef Eigen::Vector3f Vector3r;
	typedef Eigen::Vector2f Vector2r;
	typedef Eigen::Matrix2f Matrix2r;
	typedef Eigen::MatrixXf MatrixXr;
	typedef Eigen::Matrix<real, 2, 3> Matrix23r;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SparseCoefr;
#endif

public: /* members */
	/** @brief	face mesh vertexs. */
	std::vector<Vector3r> m_points;
	/** @brief	face mesh idxs. */
	std::vector<std::vector<int>> m_triangles;
	/** @brief	face mesh ctrl point. */
	std::vector<Vector3r> m_meshCtrlPoints;
	/** @brief	face mesh ctrl point가 포함되어있는 triangle의 idx. 
		 i번째 ctrl point, ctrl point를 포함한 triangle은
		 m_meshCtrlPoints[i], m_triangles[m_meshCtrlTriangles[i]] 이다. */
	std::vector<int> m_meshCtrlTriangles;
	/** @brief	texture ctrl point. */
	std::vector<Vector2r> m_texCtrlPoints;

protected: /* methods */
	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::_init();
	 *
	 * @brief	생성자를 보조하는 함수
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 **************************************************************************************************/
	void _init();

	/**********************************************************************************************//**
	 * @fn	virtual bool TextureMapperLevy::_constr();
	 *
	 * @brief	생성자를 보조하는 함수
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	virtual bool _constr();

	/**********************************************************************************************//**
	 * @fn	virtual bool TextureMapperLevy::_constr( const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::vector<glm::vec3> meshCtrlPoints, const std::vector<int> meshCtrlTriangles, const std::vector<glm::vec2> texCtrlPoints);
	 *
	 * @brief	생성자
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param	points			 	face mesh points.
	 * @param	triangles		 	face mesh idxs.
	 * @param	meshCtrlPoints   	face mesh ctrl points. 얼굴에 찍은 점들을 저장한 변수.
	 * @param	meshCtrlTriangles	face mesh ctrl points를 포함하는 triangle의 idx들 얼굴에 찍은 점들이 포함된 삼각형들을 저장한 변수.
	 * @param	texCtrlPoints	 	texture ctrl points. 사진에 찍은 점들을 저장한 변수. 이 점들은 [0,1]^2 범위에 있다고 가정함.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	virtual bool _constr(
		const std::vector<glm::vec3>& points,
		const std::vector<std::vector<int>>& triangles,
		const std::vector<glm::vec3> meshCtrlPoints,
		const std::vector<int> meshCtrlTriangles,
		const std::vector<glm::vec2> texCtrlPoints);

public: /* methods */
	/**********************************************************************************************//**
	 * @fn	TextureMapperLevy::TextureMapperLevy( const std::vector<glm::vec3>& points, const std::vector<std::vector<int>>& triangles, const std::vector<glm::vec3> meshCtrlPoints, const std::vector<int> meshCtrlTriangles, const std::vector<glm::vec2> texCtrlPoints);
	 *
	 * @brief	생성자
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param	points			 	face mesh points.
	 * @param	triangles		 	face mesh idxs.
	 * @param	meshCtrlPoints   	face mesh ctrl points. 얼굴에 찍은 점들을 저장한 변수.
	 * @param	meshCtrlTriangles	face mesh ctrl points를 포함하는 triangle의 idx들 얼굴에 찍은 점들이 포함된 삼각형들을 저장한 변수.
	 * @param	texCtrlPoints	 	texture ctrl points. 사진에 찍은 점들을 저장한 변수. 이 점들은 [0,1]^2 범위에 있다고 가정함.
	 **************************************************************************************************/
	TextureMapperLevy(
		const std::vector<glm::vec3>& points,
		const std::vector<std::vector<int>>& triangles,
		const std::vector<glm::vec3> meshCtrlPoints,
		const std::vector<int> meshCtrlTriangles,
		const std::vector<glm::vec2> texCtrlPoints);

	/**********************************************************************************************//**
	 * @fn	bool TextureMapperLevy::solve2001( std::vector<glm::vec2>& texCoords, real regWeight, real cgEp, int maxIter, const std::vector<glm::vec2>& initTexCoords);
	 *
	 * @brief	"2001, Levy, Constrained Texture Mapping for Polygonal Meshes"를 구현함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	texCoords			face mesh vertexs마다 사용할 texture coordinate. 
	 * 									i번째 face mesh vertexs에 할당될 texture coordinate는 texCoords[i]이다.
	 * @param [in]	regWeight		 	"defining problem and solving"의 "Levy 2001"에서 \epsilon에 해당.
	 * @param [in]	cgEp			 	conjugate gradient에서 tolerance에 해당.
	 * @param [in]	maxIter			 	conjugate gradient에서 최대 iteration에 해당.
	 * @param [in]	initTexCoords	 	conjugate gradient에서 초기값.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool solve2001(
		std::vector<glm::vec2>& texCoords,
		real regWeight,
		real cgEp,
		int maxIter,
		const std::vector<glm::vec2>& initTexCoords);

	/**********************************************************************************************//**
	 * @fn	bool TextureMapperLevy::solve2001( std::vector<glm::vec2>& texCoords, real regWeight, real cgEp, int maxIter, const std::vector<glm::vec2>& initTexCoords);
	 *
	 * @brief	"2001, Levy, Constrained Texture Mapping for Polygonal Meshes"를 구현함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	texCoords			face mesh vertexs마다 사용할 texture coordinate. 
	 * 									i번째 face mesh vertexs에 할당될 texture coordinate는 texCoords[i]이다.
	 * @param [in]	regWeight		 	"defining problem and solving"의 "Levy 2001"에서 \epsilon에 해당.
	 * @param [in]	cgEp			 	conjugate gradient에서 tolerance에 해당.
	 * @param [in]	maxIter			 	conjugate gradient에서 최대 iteration에 해당.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool solve2001(
		std::vector<glm::vec2>& texCoords,
		real regWeight,
		real cgEp,
		int maxIter);

	/**********************************************************************************************//**
	 * @fn	bool TextureMapperLevy::solve2002( std::vector<glm::vec2>& texCoords, real confWeight, real cgEp, int maxIter, const std::vector<glm::vec2>& initTexCoords);
	 *
	 * @brief	"2002, Levy, Least squares conformal maps for automatic texture atlas generation"에서 conformal mapping구현함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	texCoords			face mesh vertexs마다 사용할 texture coordinate. 
	 * 									i번째 face mesh vertexs에 할당될 texture coordinate는 texCoords[i]이다.
	 * @param [in]	confWeight		 	"defining problem and solving.
	 * @param [in]	cgEp			 	conjugate gradient에서 tolerance에 해당.
	 * @param [in]	maxIter			 	conjugate gradient에서 최대 iteration에 해당.
	 * @param [in]	initTexCoords	 	conjugate gradient에서 초기값.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool solve2002(
		std::vector<glm::vec2>& texCoords,
		real confWeight,
		real cgEp,
		int maxIter,
		const std::vector<glm::vec2>& initTexCoords);

	/**********************************************************************************************//**
	 * @fn	bool TextureMapperLevy::solve2002( std::vector<glm::vec2>& texCoords, real confWeight, real cgEp, int maxIter, const std::vector<glm::vec2>& initTexCoords);
	 *
	 * @brief	"2002, Levy, Least squares conformal maps for automatic texture atlas generation"에서 conformal mapping를 구현함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	texCoords			face mesh vertexs마다 사용할 texture coordinate. 
	 * 									i번째 face mesh vertexs에 할당될 texture coordinate는 texCoords[i]이다.
	 * @param [in]	confWeight		 	"defining problem and solving.
	 * @param [in]	cgEp			 	conjugate gradient에서 tolerance에 해당.
	 * @param [in]	maxIter			 	conjugate gradient에서 최대 iteration에 해당.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/
	bool solve2002(
		std::vector<glm::vec2>& texCoords,
		real confWeight,
		real cgEp,
		int maxIter);

	bool solveHybrid(
		std::vector<glm::vec2>& texCoords,
		real regWeight,
		real confWeight,
		real cgEp,
		int maxIter,
		const std::vector<glm::vec2>& initTexCoords);

	bool solveHybrid(
		std::vector<glm::vec2>& texCoords,
		real regWeight,
		real confWeight,
		real cgEp,
		int maxIter);

protected: /* sub methods */
	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getBary(Vector3r& baryCoord, const Vector3r& p, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3);
	 *
	 * @brief	barycentric coordinate를 구함.
	 * 			p1, p2, p3는 삼각형의 세 꼭지점 좌표를 의미함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	baryCoord			barycentric coordinate.
	 * @param [in]	p				 	barycentric coordinate를 구하려는 점.
	 * @param [in]	p1				 	사용한 3 꼭지점.
	 * @param [in]	p2				 	사용한 3 꼭지점.
	 * @param [in]	p3				 	사용한 3 꼭지점.
	 **************************************************************************************************/
	void getBary(Vector3r& baryCoord, const Vector3r& p, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3);

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getGrad(Matrix23r& G, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3);
	 *
	 * @brief	"derivative of barycentric coordinate" 참조.
	 * 			barycentric coordinate의 미분한 값을 구해줌.
	 * 			p1, p2, p3는 삼각형의 세 꼭지점 좌표를 의미함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	G			Gradient Matrix.
	 * @param [in]	p1		 	사용한 3 꼭지점.
	 * @param [in]	p2		 	사용한 3 꼭지점.
	 * @param [in]	p3		 	사용한 3 꼭지점.
	 **************************************************************************************************/
	void getGrad(Matrix23r& G, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3); //2001

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getGradWithArea(Matrix23r& G, int i);
	 *
	 * @brief	"derivative of barycentric coordinate" 참조.
	 * 			"conformal mapping cost" 참조.
	 * 			barycentric coordinate의 미분한 값 * i번째 삼각형의 넓이 * 2.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	G		Gradient Matrix * 삼각형 넓이 * 2.
	 * @param [in]	i		triangle idx. i번째 triangle의 Gradient Matrix * 삼각형 넓이 * 2.
	 **************************************************************************************************/
	void getGradWithArea(Matrix23r& G, int i);

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getFitMat(SparseMatrixr& mat);
	 *
	 * @brief	fitting cost에 사용되는 행렬을 구해줌.
	 * 			"fitting cost"에서 "행렬 형태" 참조.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	mat		A_fit에 해당함.
	 **************************************************************************************************/
	void getFitMat(SparseMatrixr& mat);

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getRegMat(SparseMatrixr& mat);
	 *
	 * @brief	regularization cost에 사용되는 행렬을 구해줌.
	 * 			"regularization cost"에서 "행렬 형태" 참조.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	mat		A_reg에 해당함.
	 **************************************************************************************************/
	void getRegMat(SparseMatrixr& mat);

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getConfMat(SparseMatrixr& mat);
	 *
	 * @brief	conformal mapping cost에 사용되는 행렬을 구해줌.
	 * 			"conformal mapping cost"에서 "행렬 형태" 참조.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	mat		A_conf에 해당함.
	 **************************************************************************************************/
	void getConfMat(SparseMatrixr& mat);

	/**********************************************************************************************//**
	 * @fn	void TextureMapperLevy::getBvec(VectorXr& bvec);
	 *
	 * @brief	fitting cost에 사용되는 벡터를 구해줌.
	 * 			"fitting cost"에서 "행렬 형태" 참조.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	bvec	b_fit에 해당함.
	 **************************************************************************************************/
	void getBvec(VectorXr& bvec);
};
