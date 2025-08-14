#pragma once
#include <memory>
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
#include <Util/TetrahedronProcessing/EdgeTetraMapper.h>
#include <Util/TetrahedronProcessing/VertEdgeMapper.h>
#include <Util/TetrahedronProcessing/VertTetraMapper.h>
#include "Kcoef.h"
#include "DisplacementField.h"
#include <Solver/cgsolver_typedef.h>

#define USE_CUDA 0 ///< CUDA CG Solver.
#define USE_CL 0 ///< OpenCL CG Solver. USE_CL == 1 이더라도 USE_CUDA == 1이면 CUDA를 사용함. 둘다 0이면 EIGEN CG 사용
#if USE_CUDA /* CUDA CG Solver */
#include <Solver/CGSolverGPU.h>
#elif USE_CL /* OpenCL CG Solver */
#include <Solver/CGSolverCL.h>
#endif

#include "MeshMove3d_v2_global.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @class		MeshMove3d
///
/// @brief		face mesh가 움직일 양을 물리적으로 계산해주는 클레스.
/// 			
/// @details	points, tetrahedron idxs로 구성된 tetrahedron 모델과 이들 중 뼈에 해당하는 점들을 나타내는 joint Idxs,
/// 			뼈가 움직일 양을 나타내는 joint points, Young modulus E, Poisson's ratio v가 주어지면 tetrahedron 모델
/// 			내에 점들이 움직일 양을 계산해주는 DisplacementField를 만들어준다.
/// 			
/// @note		not Assignable because of cg object
///////////////////////////////////////////////////////////////////////////////////////////////////
class MESHMOVE3D_V2_EXPORT MeshMove3d{
protected: /* members */
	std::vector<glm::vec3> m_points; ///< vertexs of tetarhedron.
	std::vector<int> m_jointIdxs; ///< joint idx set. tetrahedron의 점들 중 어떤 점을 고정 이동시킬지 선택한다. 이 점들이 뼈에 해당된다
	std::vector<glm::vec3> m_jointDisplacements; ///< joint displacements. 뼈에 해당하는 점이 움직일 양을 나타낸다.
	std::vector<std::vector<int>> m_tetrahedrons; ///< idxs set of tetrahedron.
	/// @brief	edge를 key로 가지고 tetrahedron을 value로 가지는 map.
	/// 		주어진 edge를 포함하는 tetrahedron을 구해준다.
	EdgeTetraMapper m_etm;
	/// @brief	vertex idx를 키로 가지고 edge를 value로 가지는 map.
	/// 		주어진 vertex를 포함하는 tetrahedron내의 모든 edge를 구해준다.
	/// 		주어진 vertex의 neighbor를 모두 구해준다.
	VertEdgeMapper m_vem;
	/// @brief	vertex idx를 키로 가지고 tetrahedron을 value로 가지는 map.
	/// 		주어진 vertex를 포함하는 tetrahedron을 구해준다.
	VertTetraMapper m_vtm;
	float m_lambda; ///< Lame's first parameter
	float m_mu; ///< Shear modulus
#if 1 /* follow cg solver real type */
	typedef cgsolver_real real;
	typedef Eigen::Matrix<real, Eigen::Dynamic, Eigen::Dynamic> MatrixXr;
	typedef Eigen::Matrix<real, Eigen::Dynamic, 1> VectorXr;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SparseCoefr;
#elif 0 /* double */
	typedef double real;
	typedef Eigen::MatrixXd MatrixXr;
	typedef Eigen::VectorXd VectorXr;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SparseCoefr;
#else /* float */
	typedef float real;
	typedef Eigen::MatrixXf MatrixXr;
	typedef Eigen::VectorXf VectorXr;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SparseCoefr;
#endif
protected: /* shared members for sub-routine */
	
	/// @brief	tetrahedron을 이루는 전체 점들 중에서 joint idxs를 제외한 점들. 티슈에 해당한다.
	///			{0, ..., nPoints} -{joint idx set}에 해당한다.
	std::vector<int> freeIdxs;
	Kcoef kcoef; ///< sparse matrix K를 만들기위한 정보를 저장하는 객체.
	SparseMatrixr K; ///< sparse matrix K. "Defining problem and solving"에서 "Problem setup" 참조.
	SparseMatrixr Z; ///< Z sparse matrix. "Defining problem and solving"에서 "Problem setup" 참조.
	VectorXr b; ///< joint displacements에서 만들 b벡터. "Defining problem and solving"에서 "Problem setup" 참조
	SparseMatrixr C; ///< sparse matrix C. Z^T에 해당한다. "Defining problem and solving"에서 "Problem setup" 참조.
	/// @brief	Amat = C*K*Z.
	/// 		(cg로 Amat*x = bvec를 푼다.)
	/// 		"Defining problem and solving"에서 "Problem setup" 참조. */
	SparseMatrixr Amat;
	/// @brief	bvec = -C*K*b.
	/// 		(cg로 solve Amat*x = bvec를 푼다.)
	/// 		"Defining problem and solving"에서 "Problem setup" 참조. */
	VectorXr bvec;
#if USE_CUDA
	CGSolverGPU cg; ///< CUDA GPU CG Solver
#elif USE_CL
	CGSolverCL cg; ///< OpenCL GPU CG Solver
#else
	Eigen::ConjugateGradient<SparseMatrixr> cg; ///< EIGEN CG Solver
#endif
	/// @brief	free displacements (R^(3nFrees * 1) ).
	/// 		C*K*(Z*x+b)=-C*K*b에서 x에 해당. 
	/// 		"Defining problem and solving"에서 "Problem setup" 참조. */
	VectorXr u_sol_free;
	/// @brief	all displacements (R^(3nPoints * 1)). 
	/// 		free displacements and joint displacements.
	/// 		C*K*(Z*x+b)=-C*K*b에서 (Z*x+b)에 해당.
	/// 		"Defining problem and solving"에서 "Problem setup" 참조. */
	VectorXr u_sol;

protected: /* sub-routines */
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	glm::mat3 MeshMove3d::getKst(int s, int t) const;
	///
	/// @brief	[K]_st를 구해준다. "힘 행렬2"에 해당한다.
	///
	/// @param	s	K_st에서 인덱스 s
	/// @param	t	K_st에서 인덱스 t
	///
	/// @return	[K]_st
	///////////////////////////////////////////////////////////////////////////////////////////////////
	glm::mat3 getKst(int s, int t) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	glm::mat3 MeshMove3d::getKist(const std::vector<int>& tetrahedron, int s, int t) const;
	///
	/// @brief	[K_i]_st를 구해준다. "힘 행렬"에 해당한다.
	///
	/// @param	tetrahedron	The tetrahedron.
	/// @param	s		   	K_st에서 인덱스 s
	/// @param	t		   	K_st에서 인덱스 t
	///
	/// @return	[K_i]_st
	///////////////////////////////////////////////////////////////////////////////////////////////////
	glm::mat3 getKist(const std::vector<int>& tetrahedron, int s, int t) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	glm::vec3 MeshMove3d::getMvector(const std::vector<int>& tetrahedron, const int pointIdx) const;
	///
	/// @brief	M_{i,T_i(k)}를 구해준다. "Lemmas for Barycentric interpolation in Tetrahedron" 참조.
	///			i는 i번째 tetrahedron을 의미하며, T_i(k)는 i번째 tetrahedron의 k번째 vertex의 인덱스를 의미한다.
	///
	/// @param	tetrahedron	M_{i,T_i(k)}에서 i에 해당한다.
	/// @param	pointIdx   	M_{i,T_i(k)}에서 T_i(k)에 해당한다.
	///
	/// @return	M_{i,T_i(k)}
	///////////////////////////////////////////////////////////////////////////////////////////////////
	glm::vec3 getMvector(const std::vector<int>& tetrahedron, const int pointIdx) const;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	int MeshMove3d::getInnerIdx(const std::vector<int>& tetrahedron, const int pointIdx) const;
	///
	/// @brief	vertex 인덱스가 주어진 Tetrahedron의 몇번째 원소인지 구해준다.
	///
	/// @param	tetrahedron	pointIdx를 포함하는 tetrahedorn
	/// @param	pointIdx   	vertex의 인덱스
	///
	/// @return	k s.t tetrahedron[k] = pointIdx
	///////////////////////////////////////////////////////////////////////////////////////////////////
	int getInnerIdx(const std::vector<int>& tetrahedron, const int pointIdx) const;

	void constructStructure(); ///< m_etm, m_vem, m_vtm을 초기화

	void initFreeIdxs(); ///< 전체 점들에서 join idx set을 빼서 free idx set을 계산한다.

	void initKcoef(); ///< sparse matrix K를 만들기위해 coefficient들을 구해준다. "힘 행렬", "힘 행렬2" 참조.

	void initK(); ///< sparse matrix K를 만든다. "Defining problem and solving"에서 "Problem setup" 참조.

	void initZ(); ///< sparse matrix Z를 만든다. "Defining problem and solving"에서 "Problem setup" 참조.

	void initb(); ///< b를 만든다. "Defining problem and solving"에서 "Problem setup" 참조.

	void initC(); ///< sparse matrix C (Z^T)를 만든다. "Defining problem and solving"에서 "Problem setup" 참조.

	void setupCG(); ///< setup Eigen cg solver

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	void MeshMove3d::solveCG(const std::vector<cgsolver_real>& guess);
	///
	/// @brief	solve cg with Eigen cg solver
	///
	/// @param	guess	CG를 풀때 초기값
	///////////////////////////////////////////////////////////////////////////////////////////////////
	void solveCG(const std::vector<cgsolver_real>& guess);

	void chkForce(); ///< free point에 걸리는 힘을 계산함. 디버깅용임.


public: /* public methods */
	void init(); ///< 초기화
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool MeshMove3d::constr( const std::vector<glm::vec3>& points, const std::vector<int>& jointIdxs, const std::vector<std::vector<int>>& tetrahedrons, const float E, const float v );
	///
	/// @brief	생성자.
	///
	/// @param	points			vertex of tetrahedron
	/// @param	jointIdxs   	joint idx set. tetrahedron의 점들 중 어떤 점을 고정 이동시킬지 선택한다.
	/// 						이 점들이 뼈에 해당된다.
	/// @param	tetrahedrons	idxs of tetrahedron
	/// @param	E				Young's modulus
	/// @param	v				Poisson's ratio
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool constr(
		const std::vector<glm::vec3>& points,
		const std::vector<int>& jointIdxs,
		const std::vector<std::vector<int>>& tetrahedrons,
		const float E,
		const float v
	);

	~MeshMove3d(); ///< 소멸자. GPU CG Solver를 사용할시 release 해줌

	MeshMove3d(); ///< 생성자

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	MeshMove3d::MeshMove3d( const std::vector<glm::vec3>& points, const std::vector<int>& jointIdxs, const std::vector<std::vector<int>>& tetrahedrons, const float E, const float v );
	///
	/// @brief	생성자.
	///
	/// @param	points			vertex of tetrahedron
	/// @param	jointIdxs   	joint idx set. tetrahedron의 점들 중 어떤 점을 강제 이동시킬지 선택한다.
	/// 						이 점들이 뼈에 해당된다.
	/// @param	tetrahedrons	idxs of tetrahedron
	/// @param	E				Young's modulus
	/// @param	v				Poisson's ratio
	///////////////////////////////////////////////////////////////////////////////////////////////////
	MeshMove3d(
		const std::vector<glm::vec3>& points,
		const std::vector<int>& jointIdxs,
		const std::vector<std::vector<int>>& tetrahedrons,
		const float E,
		const float v
		);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @fn	bool MeshMove3d::execute( std::vector<glm::vec3>& queryDisplacements, const std::vector<glm::vec3>& jointDisplacements, const std::vector<glm::vec3>& queryDisplacements0 );
	///
	/// @brief	Executes.
	///
	/// @param [in,out]	queryDisplacements	계산된 모든 점에서의 이동량.
	/// @param	jointDisplacements		  	jointIdxs에 해당하는 점들에 가할 이동량. jointIdxs와 사이즈가 같아야함.
	/// @param	queryDisplacements0		  	CG를 풀때 queryDisplacements의 초기값. 
	/// 									이전에 MeshMove3d::execute로 얻은 queryDisplacements를 사용하면된다.
	///
	/// @return	true if it succeeds, false if it fails.
	///////////////////////////////////////////////////////////////////////////////////////////////////
	bool execute(
		std::vector<glm::vec3>& queryDisplacements,
		const std::vector<glm::vec3>& jointDisplacements,
		const std::vector<glm::vec3>& queryDisplacements0
	);

};
