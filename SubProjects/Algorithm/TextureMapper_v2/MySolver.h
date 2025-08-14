#pragma once
#if defined(__APPLE__)
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>
#include <eigen3/Eigen/SparseCore>
#else
#include <Eigen/Dense>
#include <Eigen/Sparse>
#endif

/**********************************************************************************************//**
 * @class	MySolver
 *
 * @brief	CG solver
 *
 * @author	Hosan
 * @date	2016-05-12
 **************************************************************************************************/
class MySolver{
public:
#if 0 /* double */
	typedef double real;
	typedef Eigen::VectorXd VectorXr;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SpCoef;
#else /* float */
	typedef float real;
	typedef Eigen::VectorXf VectorXr;
	typedef Eigen::SparseMatrix<real> SparseMatrixr;
	typedef Eigen::Triplet<real> SpCoef;
#endif
public:

	/** @brief	계산후 몇번째에서 수렴했는지를 저장 */
	int curIter;

	/** @brief	계산전 tolerance를 지정하는데 사용 */
	int tol;

	/** @brief	계산전 최대 iteration를 지정하는데 사용 */
	int maxIter;

	/**********************************************************************************************//**
	 * @fn	MySolver::MySolver();
	 *
	 * @brief	생성자
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 **************************************************************************************************/
	MySolver();

	/**********************************************************************************************//**
	 * @fn	void MySolver::solve(VectorXr& x, const SparseMatrixr& A, const VectorXr& b, const VectorXr& x0);
	 *
	 * @brief	Ax=b를 풀어줌. 초기값은 x0로 시작
	 * 			https://en.wikipedia.org/wiki/Conjugate_gradient_method 에서 "Example code in MATLAB / GNU Octave" 참조
	 * 			성능향상을 위해서 incomplete cholesky decomposition 정도는 구현해주면 좋을 것으로 보임. 다만 GPU에서도 효율적으로 구현 가능한가를 고려해야함.
	 *
	 * @author	Hosan
	 * @date	2016-05-12
	 *
	 * @param [out]	x		solution
	 * @param [in]	A		A
	 * @param [in]	b		b
	 * @param [in]	x0		x 초기값
	 **************************************************************************************************/
	void solve(VectorXr& x, const SparseMatrixr& A, const VectorXr& b, const VectorXr& x0);
};
