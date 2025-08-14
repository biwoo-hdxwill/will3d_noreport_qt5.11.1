#pragma once
#pragma message("# Util/EigenUtil/EigenUtil.h visited")
#include "../Core/util_global.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <vector>
#include <string>
class UTIL_EXPORT EigenUtil{
public:
	typedef float real;
	typedef Eigen::Matrix<real, Eigen::Dynamic, Eigen::Dynamic> MatrixXr;
	typedef Eigen::Matrix<real, 3, 3> Matrix3r;
	typedef Eigen::Triplet<real> SparseCoefr;
	typedef Eigen::SparseMatrix<real> SparseXr;
	static void getSpCoef(
		std::vector<SparseCoefr>& spCoef,
		const SparseXr& mat,
		int rOffset,
		int cOffset
		);
};
