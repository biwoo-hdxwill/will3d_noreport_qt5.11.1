#include "TextureMapperLevy.h"
#include <iostream>
#include <Util/PointProcessing/EdgeToTriIdxMapper.h>
#include <Util/PointProcessing/Edge.h>

#include <util/Core/Logger.hpp>
#include "MySolver.h"

using namespace std;
#define NUM_DEBUG 1
#define MATLAB_DEBUG 0
#define USE_CUDA 0
#if _WIN64
#define USE_CL 1
#else
#define USE_CL 0
#endif

#if USE_CUDA
#include <Solver/CGSolverGPU.h>
//#include "SparseMatrixCSR.h"
#elif USE_CL
#include <Solver/CGSolverCL.h>
#endif

void TextureMapperLevy::_init() {
}
bool TextureMapperLevy::_constr() {
	_init();
	return true;
}
bool TextureMapperLevy::_constr(
	const std::vector<glm::vec3>& points,
	const std::vector<std::vector<int>>& triangles,
	const std::vector<glm::vec3> meshCtrlPoints,
	const std::vector<int> meshCtrlTriangles,
	const std::vector<glm::vec2> texCtrlPoints) {
	try {
		/* 얼굴에 찍은 점 갯수와 텍스쳐에 찍은 점 갯수 일치여부 확인 */
		if (meshCtrlPoints.size() != meshCtrlTriangles.size() ||
			meshCtrlPoints.size() != texCtrlPoints.size()
			) {
			throw std::runtime_error("ctrl points dimension mismatch");
		}
		/* glm::vec3를 Eigen::Vector3r로 변환 */
		m_points.resize(points.size());
		for (int i = 0; i < points.size(); i++) {
			const auto& p = points[i];
			m_points[i] = Vector3r(p.x, p.y, p.z);
		}
		m_triangles = triangles;
		/* glm::vec3를 Eigen::Vector3r로 변환 */
		m_meshCtrlPoints.resize(meshCtrlPoints.size());
		for (int i = 0; i < meshCtrlPoints.size(); i++) {
			const auto& p = meshCtrlPoints[i];
			m_meshCtrlPoints[i] = Vector3r(p.x, p.y, p.z);
		}
		m_meshCtrlTriangles = meshCtrlTriangles;
		/* glm::vec2를 Eigen::Vector2r로 변환 */
		m_texCtrlPoints.resize(texCtrlPoints.size());
		for (int i = 0; i < texCtrlPoints.size(); i++) {
			const auto& p = texCtrlPoints[i];
			m_texCtrlPoints[i] = Vector2r(p.x, p.y);
		}
		return true;
	} catch (std::runtime_error& e) {

		lg << "TextureMapperLevy::TextureMapperLevy: " << e << endl;

		return false;
	}
}
TextureMapperLevy::TextureMapperLevy(
	const std::vector<glm::vec3>& points,
	const std::vector<std::vector<int>>& triangles,
	const std::vector<glm::vec3> meshCtrlPoints,
	const std::vector<int> meshCtrlTriangles,
	const std::vector<glm::vec2> texCtrlPoints) {
	_constr(points, triangles, meshCtrlPoints, meshCtrlTriangles, texCtrlPoints);
}
bool TextureMapperLevy::solve2001(
	std::vector<glm::vec2>& texCoords,
	real regWeight,
	real cgEp,
	int maxIter,
	const std::vector<glm::vec2>& initTexCoords) {
	try {
		int nPoint = m_points.size();
		if (nPoint != initTexCoords.size()) {
			throw std::runtime_error("initTexCoords dim mismatch");
		}

		/* A_fit, A_reg, b_fit을 만듬 */
		SparseMatrixr A_fit;
		SparseMatrixr A_reg;
		VectorXr bvec;

		lg << "fit" << endl;

		getFitMat(A_fit);

		lg << "reg" << endl;

		getRegMat(A_reg);

		lg << "bvec" << endl;

		getBvec(bvec);

		/* Mx=n을 푸는 CG에 사용할 행렬과 벡터를 만듬 */
		SparseMatrixr M = A_fit.transpose()*A_fit + regWeight * regWeight * A_reg.transpose()*A_reg;
		VectorXr n = A_fit.transpose()*bvec;

		/* Mx=n을 CG로 풀때 초기값 x0를 만듬 */
		VectorXr xInit(2 * nPoint);
		for (int i = 0; i < nPoint; i++) {
			xInit(i) = initTexCoords[i][0];
			xInit(i + nPoint) = initTexCoords[i][1];
		}

#if MATLAB_DEBUG
		/* debug */
		MatlabEngine eng;
		eng.setPersistent(true);
		mxArrayWrapper mat_M;
		mxArrayWrapper mat_n;
		mxArrayWrapper mat_b;
		mxArrayWrapper mat_Areg;
		mxArrayWrapper mat_Aregh;
		mxArrayWrapper mat_A1;
		mxArrayWrapper mat_A1h;
		SparseMatrixr A_regh = A_reg.transpose() * A_reg;
		SparseMatrixr A_fith = A_fit.transpose() * A_fit;
		EigenUtil::convert(mat_M, M, "M2");
		EigenUtil::convert(mat_n, n, "n2");
		EigenUtil::convert(mat_b, bvec, "b2");
		EigenUtil::convert(mat_Areg, A_reg, "Areg2");
		EigenUtil::convert(mat_Aregh, A_regh, "Aregh2");
		EigenUtil::convert(mat_A1, A_fit, "Afit2");
		EigenUtil::convert(mat_A1h, A_fith, "Afith2");
		eng.putVariable(mat_M);
		eng.putVariable(mat_n);
		eng.putVariable(mat_b);
		eng.putVariable(mat_Areg);
		eng.putVariable(mat_A1);
		eng.putVariable(mat_Aregh);
		eng.putVariable(mat_A1h);
#endif

#if 0 /* Eigen 내장 CG사용 */
		/* solve cg */
		Eigen::ConjugateGradient<SparseMatrixr> cg;
		cg.compute(M);
		cg.setMaxIterations(maxIter);
		cg.setTolerance(n.norm()*cgEp);
		VectorXr sol = cg.solveWithGuess(n, xInit);
		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */

		lg << "Eigen CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		lg << "CG iter=" << cg.iterations() << "/" << maxIter << endl;
		lg << "CG esti err=" << cg.error() << endl;
		lg << "absErr=" << (M*sol - n).norm() << endl;
		lg << "relErr=" << (M*sol - n).norm() / n.norm() << endl;

#elif USE_CUDA /* cusparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverGPU cg;
			cg.m_absTol = cgEp * n.norm();
			cg.m_relTol = cgEp;
			cg.m_maxIter = maxIter;
			cg.m_x0 = x0;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}

			cout << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;
			cout << "CG iter=" << cg.mr_iter << "/" << maxIter << endl;
			cout << "absErr=" << cg.mr_absErr << endl;
			cout << "relErr=" << cg.mr_relErr << endl;
		}
#elif USE_CL /* clsparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverCL cg;
			cg.m_maxIter = maxIter;
			cg.m_absTol = (cgEp * n.norm());
			cg.m_relTol = cgEp;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}

			cout << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		}
#else /* CPU cg */
		int dim = M.rows();
		VectorXr sol(dim);

		MySolver cg;
		cg.tol = cgEp*n.norm();
		cg.maxIter = maxIter;
		cg.solve(sol, M, n, VectorXr::Zero(dim));

		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */
		cout << "wiki CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		cout << "CG iter=" << cg.curIter << "/" << maxIter << endl;
		cout << "absErr=" << (M*sol - n).norm() << endl;
		cout << "relErr=" << (M*sol - n).norm() / n.norm() << endl;
#endif

		return true;
			} catch (std::runtime_error& e) {
				cout << "TextureMapperLevy::solve: " << e.what() << endl;
				return false;
			}
		}
bool TextureMapperLevy::solve2001(
	std::vector<glm::vec2>& texCoords,
	real regWeight,
	real cgEp,
	int maxIter) {
	return solve2001(texCoords, regWeight, cgEp, maxIter, vector<glm::vec2>(m_points.size(), glm::vec2(0, 0)));
}
bool TextureMapperLevy::solve2002(
	std::vector<glm::vec2>& texCoords,
	real confWeight,
	real cgEp,
	int maxIter,
	const std::vector<glm::vec2>& initTexCoords) {
	try {
		int nPoint = m_points.size();
		if (nPoint != initTexCoords.size()) {
			throw std::runtime_error("initTexCoords dim mismatch");
		}

		/* A_fit, A_conf, b_fit을 만듬 */
		SparseMatrixr A_fit;
		SparseMatrixr A_conf;
		VectorXr bvec;
		cout << "fit" << endl;
		getFitMat(A_fit);
		cout << "conf" << endl;
		getConfMat(A_conf);
		cout << "bvec" << endl;
		getBvec(bvec);

		/* Mx=n을 푸는 CG에 사용할 행렬과 벡터를 만듬 */
		SparseMatrixr M = A_fit.transpose()*A_fit + confWeight * confWeight * A_conf.transpose()*A_conf;
		VectorXr n = A_fit.transpose()*bvec;

#if MATLAB_DEBUG
		/* debug */
		MatlabEngine eng;
		eng.setPersistent(true);
		mxArrayWrapper mat_M;
		mxArrayWrapper mat_n;
		mxArrayWrapper mat_b;
		mxArrayWrapper mat_Aconf;
		mxArrayWrapper mat_Aconfh;
		mxArrayWrapper mat_A1;
		mxArrayWrapper mat_A1h;
		SparseMatrixr A_confh = A_conf.transpose() * A_conf;
		SparseMatrixr A_fith = A_fit.transpose() * A_fit;
		EigenUtil::convert(mat_M, M, "M2");
		EigenUtil::convert(mat_n, n, "n2");
		EigenUtil::convert(mat_b, bvec, "b2");
		EigenUtil::convert(mat_Aconf, A_conf, "Aconf2");
		EigenUtil::convert(mat_Aconfh, A_confh, "Aconfh2");
		EigenUtil::convert(mat_A1, A_fit, "Afit2");
		EigenUtil::convert(mat_A1h, A_fith, "Afith2");
		eng.putVariable(mat_M);
		eng.putVariable(mat_n);
		eng.putVariable(mat_b);
		eng.putVariable(mat_Aconf);
		eng.putVariable(mat_A1);
		eng.putVariable(mat_Aconfh);
		eng.putVariable(mat_A1h);
#endif

		/* Mx=n을 CG로 풀때 초기값 x0를 만듬 */
		VectorXr xInit(2 * nPoint);
		for (int i = 0; i < nPoint; i++) {
			xInit(i) = initTexCoords[i][0];
			xInit(i + nPoint) = initTexCoords[i][1];
		}

#if 0 /* Eigen 내장 CG사용 */
		/* solve cg */
		Eigen::ConjugateGradient<SparseMatrixr> cg;
		cg.compute(M);
		cg.setMaxIterations(maxIter);
		cg.setTolerance(n.norm()*cgEp);
		VectorXr sol = cg.solveWithGuess(n, xInit);
		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */
		cout << "Eigen CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		cout << "CG iter=" << cg.iterations() << "/" << maxIter << endl;
		cout << "CG esti err=" << cg.error() << endl;
		cout << "absErr=" << (M*sol - n).norm() << endl;
		cout << "relErr=" << (M*sol - n).norm() / n.norm() << endl;
#elif USE_CUDA /* cusparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverGPU cg;
			cg.m_absTol = cgEp * n.norm();
			cg.m_relTol = cgEp;
			cg.m_maxIter = maxIter;
			cg.m_x0 = x0;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}

			cout << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;
			cout << "CG iter=" << cg.mr_iter << "/" << maxIter << endl;
			cout << "absErr=" << cg.mr_absErr << endl;
			cout << "relErr=" << cg.mr_relErr << endl;
	}
#elif USE_CL /* clsparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverCL cg;
			cg.m_maxIter = maxIter;
			cg.m_absTol = (cgEp * n.norm());
			cg.m_relTol = cgEp;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}

			//lgc << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		}
#else /* CPU cg */
		int dim = M.rows();
		VectorXr sol(dim);

		MySolver cg;
		cg.tol = cgEp*n.norm();
		cg.maxIter = maxIter;

		lg.tic("cpu solve only");

		cg.solve(sol, M, n, VectorXr::Zero(dim));

		lg.toc();


		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */

		lg << "wiki CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		lg << "CG iter=" << cg.curIter << "/" << maxIter << endl;
		lg << "absErr=" << (M*sol - n).norm() << endl;
		lg << "relErr=" << (M*sol - n).norm() / n.norm() << endl;

#endif

		return true;
		} catch (std::runtime_error& e) {

			lg << "TextureMapperLevy::solve: " << e << endl;

			return false;
		}
}
bool TextureMapperLevy::solve2002(
	std::vector<glm::vec2>& texCoords,
	real confWeight,
	real cgEp,
	int maxIter) {
	return solve2002(texCoords, confWeight, cgEp, maxIter, vector<glm::vec2>(m_points.size(), glm::vec2(0, 0)));
}
bool TextureMapperLevy::solveHybrid(
	std::vector<glm::vec2>& texCoords,
	real regWeight,
	real confWeight,
	real cgEp,
	int maxIter,
	const std::vector<glm::vec2>& initTexCoords) {
	try {
		int nPoint = m_points.size();
		if (nPoint != initTexCoords.size()) {
			throw std::runtime_error("initTexCoords dim mismatch");
		}

		/* A들과 bvec만듬 */
		SparseMatrixr A_fit;
		SparseMatrixr A_reg;
		SparseMatrixr A_conf;
		VectorXr bvec;

		lg << "bvec" << endl;

		getBvec(bvec);

		/* Mx=n을 푸는 CG에 사용할 행렬과 벡터를 만듬 */

		lg << "fit" << endl;

		getFitMat(A_fit);
		SparseMatrixr M = A_fit.transpose()*A_fit;
		if (regWeight != 0) {

			lg << "conf" << endl;

			getRegMat(A_reg);
			M += regWeight * regWeight * A_reg.transpose()*A_reg;
		}
		if (confWeight != 0) {

			lg << "reg" << endl;

			getConfMat(A_conf);
			M += confWeight * confWeight * A_conf.transpose()*A_conf;
		}

		VectorXr n = A_fit.transpose()*bvec;

		/* Mx=n을 CG로 풀때 초기값 x0를 만듬 */
		VectorXr xInit(2 * nPoint);
		for (int i = 0; i < nPoint; i++) {
			xInit(i) = initTexCoords[i][0];
			xInit(i + nPoint) = initTexCoords[i][1];
		}

#if 0 /* Eigen 내장 CG사용 */
		/* solve cg */
		Eigen::ConjugateGradient<SparseMatrixr> cg;
		cg.compute(M);
		cg.setMaxIterations(maxIter);
		cg.setTolerance(n.norm()*cgEp);
		VectorXr sol = cg.solveWithGuess(n, xInit);
		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */

		lg << "Eigen CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		lg << "CG iter=" << cg.iterations() << "/" << maxIter << endl;
		lg << "CG esti err=" << cg.error() << endl;
		lg << "absErr=" << (M*sol - n).norm() << endl;
		lg << "relErr=" << (M*sol - n).norm() / n.norm() << endl;

#elif USE_CUDA /* cusparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverGPU cg;
			cg.m_absTol = cgEp * n.norm();
			cg.m_relTol = cgEp;
			cg.m_maxIter = maxIter;
			cg.m_x0 = x0;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}


			lg << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;
			lg << "CG iter=" << cg.mr_iter << "/" << maxIter << endl;
			lg << "absErr=" << cg.mr_absErr << endl;
			lg << "relErr=" << cg.mr_relErr << endl;

	}
#elif USE_CL /* clsparse로 구현한 cg */
		{
			/* get elements */
			int dim = M.rows();
			typedef cgsolver_real real;
			vector<Coef> coefs;
			coefs.reserve(M.nonZeros());
			for (int k = 0; k < M.outerSize(); k++) {
				for (SparseMatrixr::InnerIterator it(M, k); it; ++it) {
					coefs.push_back(Coef(it.row(), it.col(), it.value()));
				}
			}
			vector<real> x0(xInit.size());
			vector<real> b(n.size());
			for (int i = 0; i < x0.size(); i++) {
				x0[i] = xInit[i];
			}
			for (int i = 0; i < b.size(); i++) {
				b[i] = n[i];
			}
			CGSolverCL cg;
			cg.m_maxIter = maxIter;
			cg.m_absTol = (cgEp * n.norm());
			cg.m_relTol = cgEp;
			vector<real> xSolved;
			cg.compute(dim, dim, coefs);

			lg.tic("gpu solve only");

			cg.solve(xSolved, b);

			lg.toc();


			texCoords.resize(nPoint);
			for (int i = 0; i < nPoint; i++) {
				texCoords[i] = glm::vec2(xSolved[i], xSolved[i + nPoint]);
			}


			lg << "gpu CG(" << M.rows() << "," << M.cols() << "):: " << endl;

		}
#else /* CPU cg */
		int dim = M.rows();
		VectorXr sol(dim);

		MySolver cg;
		cg.tol = cgEp*n.norm();
		cg.maxIter = maxIter;
		cg.solve(sol, M, n, VectorXr::Zero(dim));

		/* result out */
		texCoords.resize(nPoint);
		for (int i = 0; i < nPoint; i++) {
			texCoords[i] = glm::vec2(sol(i), sol(i + nPoint));
		}

		/* print */

		lg << "wiki CG(" << M.rows() << "," << M.cols() << "):: " << endl;
		lg << "CG iter=" << cg.curIter << "/" << maxIter << endl;
		lg << "absErr=" << (M*sol - n).norm() << endl;
		lg << "relErr=" << (M*sol - n).norm() / n.norm() << endl;

#endif

		return true;
			} catch (std::runtime_error& e) {

				lg << "TextureMapperLevy::solve: " << e << endl;

				return false;
			}
		}
bool TextureMapperLevy::solveHybrid(
	std::vector<glm::vec2>& texCoords,
	real regWeight,
	real confWeight,
	real cgEp,
	int maxIter) {
	return solveHybrid(texCoords, regWeight, confWeight, cgEp, maxIter, vector<glm::vec2>(m_points.size(), glm::vec2(0, 0)));
}
void TextureMapperLevy::getBary(Vector3r& baryCoord, const Vector3r& p, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3) {
	// local coordinate system X, Y를 만듬
	// "local coordinate system axis" 참조
	Vector3r X = (p2 - p1) / (p2 - p1).norm();
	Vector3r Y = X.cross(p3 - p1).cross(X);
	Y = Y / Y.norm();
	// barycentric coordinate를 계산함
	// "barycentric coordinate" 참조
	real x = X.dot(p - p1);
	real y = Y.dot(p - p1);
	real x1 = (p1 - p1).dot(X); // 0
	real x2 = (p2 - p1).dot(X);
	real x3 = (p3 - p1).dot(X);
	real y1 = (p1 - p1).dot(Y); // 0
	real y2 = (p2 - p1).dot(Y); // 0
	real y3 = (p3 - p1).dot(Y);
	Vector2r q(x, y);
	Vector2r q1(x1, y1);
	real det = (x2 - x1)*(y3 - y1) - (y2 - y1)*(x3 - x1);
#if NUM_DEBUG // 넓이 0인 삼각형인지 체크
	if (det == 0) {

		lg << "TextureMapperLevy::getBary: det is 0" << endl;

	}
#endif
	Matrix2r invV;
	invV(0, 0) = y3 - y1;	invV(0, 1) = x1 - x3;
	invV(1, 0) = y1 - y2;	invV(1, 1) = x2 - x1;
	invV /= det;
	Vector2r temp = invV * (q - q1);
	baryCoord(0) = 1 - temp(0) - temp(1);
	baryCoord(1) = temp(0);
	baryCoord(2) = temp(1);
}
void TextureMapperLevy::getGrad(Matrix23r& G, const Vector3r& p1, const Vector3r& p2, const Vector3r& p3) {
	// local coordinate system X, Y를 만듬
	// "local coordinate system axis" 참조
	Vector3r X;
	Vector3r Y;
#if NUM_DEBUG // 넓이 0인 삼각형인지 체크
	try {
		bool xDegen = false;
		bool yDegen = false;
		if (!(p2 - p1).norm()) {
			xDegen = true;
		}
		if (!(p3 - p1).norm()) {
			yDegen = true;
		}
		if (!xDegen && !yDegen) {
			X = (p2 - p1);
			X = X / X.norm();
			Y = X.cross(p3 - p1).cross(X);
			if (!Y.norm()) {
				Y = X.cross(Vector3r(0, 1, 0));
			}
			if (!Y.norm()) {
				Y = X.cross(Vector3r(1, 0, 0));
			}
			Y = Y / Y.norm();
		} else if (xDegen && yDegen) {
			throw std::runtime_error("null triangle. p1==p2==p3");
		} else if (xDegen && !yDegen) {

			lg << "TextureMapperLevy::getGrad::xDegen" << endl;

			Y = (p3 - p1);
			Y = Y / Y.norm();
			X = Y.cross(Vector3r(1, 0, 0));
			if (!X.norm()) {
				X = Y.cross(Vector3r(0, 1, 0));
			}
			X = X / X.norm();
		} else if (!xDegen && yDegen) {

			lg << "TextureMapperLevy::getGrad::yDegen" << endl;

			X = p2 - p1;
			X = X / X.norm();
			Y = X.cross(Vector3r(0, 1, 0));
			if (!Y.norm()) {
				Y = X.cross(Vector3r(1, 0, 0));
			}
			Y = Y / Y.norm();
		}
			} catch (std::runtime_error& e) {

				lg << "TextureMapperLevy::getGrad: " << e << endl;

				G(0, 0) = 0;	G(0, 1) = 0;	G(0, 2) = 0;
				G(1, 0) = 0;	G(1, 1) = 0;	G(1, 2) = 0;
			}
#else // 삼각형이 모두 넓이가 0보다 크다고 가정
	X = (p2 - p1) / (p2 - p1).norm();
	Y = X.cross(p3 - p1).cross(X);
	Y = Y / Y.norm();
#endif
	// Gradient Matrix 계산
	// "derivative of barycentric coordinate" 참조
	real x1 = (p1 - p1).dot(X); // =0
	real x2 = (p2 - p1).dot(X);
	real x3 = (p3 - p1).dot(X);
	real y1 = (p1 - p1).dot(Y); // =0
	real y2 = (p2 - p1).dot(Y); // =0
	real y3 = (p3 - p1).dot(Y);
	real d = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);
	G(0, 0) = (y2 - y3) / d;	G(0, 1) = (y3 - y1) / d;	G(0, 2) = (y1 - y2) / d;
	G(1, 0) = (x3 - x2) / d;	G(1, 1) = (x1 - x3) / d;	G(1, 2) = (x2 - x1) / d;
		}
void TextureMapperLevy::getGradWithArea(Matrix23r& G, int i) {
	// local coordinate system X, Y를 만듬
	// "local coordinate system axis" 참조
	Vector3r X;
	Vector3r Y;
	const Vector3r& p1 = m_points[m_triangles[i][0]];
	const Vector3r& p2 = m_points[m_triangles[i][1]];
	const Vector3r& p3 = m_points[m_triangles[i][2]];
#if NUM_DEBUG  // 넓이 0인 삼각형인지 체크
	try {
		bool xDegen = false;
		bool yDegen = false;
		if (!(p2 - p1).norm()) {
			xDegen = true;
		}
		if (!(p3 - p1).norm()) {
			yDegen = true;
		}
		if (!xDegen && !yDegen) {
			X = (p2 - p1);
			X = X / X.norm();
			Y = X.cross(p3 - p1).cross(X);
			if (!Y.norm()) {
				Y = X.cross(Vector3r(0, 1, 0));
			}
			if (!Y.norm()) {
				Y = X.cross(Vector3r(1, 0, 0));
			}
			Y = Y / Y.norm();
		} else if (xDegen && yDegen) {
			throw std::runtime_error("null triangle. p1==p2==p3");
		} else if (xDegen && !yDegen) {

			lg << "TextureMapperLevy::getGrad::xDegen" << endl;

			Y = (p3 - p1);
			Y = Y / Y.norm();
			X = Y.cross(Vector3r(1, 0, 0));
			if (!X.norm()) {
				X = Y.cross(Vector3r(0, 1, 0));
			}
		} else if (!xDegen && yDegen) {

			lg << "TextureMapperLevy::getGrad::yDegen" << endl;

			X = p2 - p1;
			X = X / X.norm();
			Y = X.cross(Vector3r(0, 1, 0));
			if (!Y.norm()) {
				Y = X.cross(Vector3r(1, 0, 0));
			}
		}
	} catch (std::runtime_error& e) {

		lg << "TextureMapperLevy::getGradWithArea: " << e << endl;

		G(0, 0) = 0;	G(0, 1) = 0;	G(0, 2) = 0;
		G(1, 0) = 0;	G(1, 1) = 0;	G(1, 2) = 0;
	}
#else // 삼각형이 모두 넓이가 0보다 크다고 가정
	X = (p2 - p1) / (p2 - p1).norm();
	Y = X.cross(p3 - p1).cross(X);
#endif
	// Gradient Matrix 계산
	// "derivative of barycentric coordinate" 참조
	// 여기서는 삼각형넓이 * 2를 더 곱해주므로 det = 1로 두었다.
	real x1 = (p1 - p1).dot(X); // 0
	real x2 = (p2 - p1).dot(X);
	real x3 = (p3 - p1).dot(X);
	real y1 = (p1 - p1).dot(Y); // 0
	real y2 = (p2 - p1).dot(Y); // 0
	real y3 = (p3 - p1).dot(Y);
	real d = 1.0;
	G(0, 0) = (y2 - y3) / d;	G(0, 1) = (y3 - y1) / d;	G(0, 2) = (y1 - y2) / d;
	G(1, 0) = (x3 - x2) / d;	G(1, 1) = (x1 - x3) / d;	G(1, 2) = (x2 - x1) / d;
}
void TextureMapperLevy::getFitMat(SparseMatrixr& mat) {
	vector<SparseCoefr> spCoefs; // sparse matrix coefficient (row, col, val)
	// A_fit 계산
	// "fitting cost" 참조
	int mCtrl = m_meshCtrlPoints.size(); // 얼굴에 찍은 점 갯수
	int nPoint = m_points.size(); // face mesh의 점 갯수
	for (int i = 0; i < mCtrl; i++) {
		const Vector3r& meshCtrlPoint = m_meshCtrlPoints[i];
		const auto& triangle = m_triangles[m_meshCtrlTriangles[i]];
		const auto& p1 = m_points[triangle[0]];
		const auto& p2 = m_points[triangle[1]];
		const auto& p3 = m_points[triangle[2]];
		Vector3r baryCoord;
		getBary(baryCoord, meshCtrlPoint, p1, p2, p3);

		spCoefs.push_back(SparseCoefr(i, triangle[0], baryCoord[0]));
		spCoefs.push_back(SparseCoefr(i, triangle[1], baryCoord[1]));
		spCoefs.push_back(SparseCoefr(i, triangle[2], baryCoord[2]));
		spCoefs.push_back(SparseCoefr(i + mCtrl, triangle[0] + nPoint, baryCoord[0]));
		spCoefs.push_back(SparseCoefr(i + mCtrl, triangle[1] + nPoint, baryCoord[1]));
		spCoefs.push_back(SparseCoefr(i + mCtrl, triangle[2] + nPoint, baryCoord[2]));
	}
	mat = SparseMatrixr(2 * mCtrl, 2 * nPoint);
	mat.setFromTriplets(spCoefs.begin(), spCoefs.end()); // sparse matrix coefficient로 행렬 생성
}
int getOther(const vector<int>& triangle, int v0, int v1) {
	for (int i = 0; i < triangle.size(); i++) {
		if (triangle[i] != v0 && triangle[i] != v1) {
			return triangle[i];
		}
	}
}
void TextureMapperLevy::getRegMat(SparseMatrixr& mat) {
	vector<SparseCoefr> spCoefs;
	EdgeToTriIdxMapper etm(m_triangles);
	// face mesh의 삼각형이 가지는 모든 edge 추출
	vector<tora::Edge> edges = etm.getEdges();
	int nEdge = edges.size();
	int nPoint = m_points.size();
	for (int i = 0; i < nEdge; i++) {
		// edges[i]를 포함하는 삼각형 idx 추출
		vector<int> triangles = etm.findTriIdxByEdge(edges[i]);
		int v0 = edges[i].v0;
		int v1 = edges[i].v1;

		// if there is only one triangle containing the edge, skip
		// edges[i]를 포함하는 삼각형이 1개라면 최외곽 edge이므로 계산하지 않음
		if (triangles.size() < 2) {
			continue;
		}

		/* calc */
		// "regularization cost" 참조
		const auto& triangle1 = m_triangles[triangles[0]];
		const auto& triangle2 = m_triangles[triangles[1]];
		int v2 = getOther(triangle1, v0, v1); // v0, v1을 제외한 나머지 point idx 추출
		int v2_ = getOther(triangle2, v0, v1); // v0, v1을 제외한 나머지 point idx 추출
		Matrix23r G1;
		Matrix23r G2;
		getGrad(G1, m_points[v0], m_points[v1], m_points[v2]); // Gradient Matrix를 구할때 (edge의 첫번째 점, edge의 두번째 점, 나머지 점) 순서로 점을 넣어줌.
		getGrad(G2, m_points[v0], m_points[v1], m_points[v2_]); // Gradient Matrix를 구할때 (edge의 첫번째 점, edge의 두번째 점, 나머지 점) 순서로 점을 넣어줌.

		spCoefs.push_back(SparseCoefr(i, v0, G1(1, 0) + G2(1, 0)));
		spCoefs.push_back(SparseCoefr(i, v1, G1(1, 1) + G2(1, 1)));
		spCoefs.push_back(SparseCoefr(i, v2, G1(1, 2)));
		spCoefs.push_back(SparseCoefr(i, v2_, G2(1, 2)));
		spCoefs.push_back(SparseCoefr(i + nEdge, v0 + nPoint, G1(1, 0) + G2(1, 0)));
		spCoefs.push_back(SparseCoefr(i + nEdge, v1 + nPoint, G1(1, 1) + G2(1, 1)));
		spCoefs.push_back(SparseCoefr(i + nEdge, v2 + nPoint, G1(1, 2)));
		spCoefs.push_back(SparseCoefr(i + nEdge, v2_ + nPoint, G2(1, 2)));
	}
	mat = SparseMatrixr(2 * nEdge, 2 * nPoint);
	mat.setFromTriplets(spCoefs.begin(), spCoefs.end()); // sparse matrix coefficient로 행렬 생성
}
void TextureMapperLevy::getConfMat(SparseMatrixr& mat) {
	vector<SparseCoefr> spCoefs;
	int nTriangle = m_triangles.size();
	int nPoint = m_points.size();
	// A_conf 계산
	// "conformal mapping cost" 참조
	for (int i = 0; i < nTriangle; i++) {
		const auto& triangle = m_triangles[i];
		Matrix23r G;
		getGradWithArea(G, i);
		spCoefs.push_back(SparseCoefr(i, triangle[0], G(0, 0)));
		spCoefs.push_back(SparseCoefr(i, triangle[1], G(0, 1)));
		spCoefs.push_back(SparseCoefr(i, triangle[2], G(0, 2)));

		spCoefs.push_back(SparseCoefr(i, triangle[0] + nPoint, -G(1, 0)));
		spCoefs.push_back(SparseCoefr(i, triangle[1] + nPoint, -G(1, 1)));
		spCoefs.push_back(SparseCoefr(i, triangle[2] + nPoint, -G(1, 2)));

		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[0], G(1, 0)));
		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[1], G(1, 1)));
		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[2], G(1, 2)));

		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[0] + nPoint, G(0, 0)));
		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[1] + nPoint, G(0, 1)));
		spCoefs.push_back(SparseCoefr(i + nTriangle, triangle[2] + nPoint, G(0, 2)));
	}
	mat = SparseMatrixr(2 * nTriangle, 2 * nPoint);
	mat.setFromTriplets(spCoefs.begin(), spCoefs.end()); // sparse matrix coefficient로 행렬 생성
}
void TextureMapperLevy::getBvec(VectorXr& bvec) {
	// "fitting cost" 참조
	// b_fit 생성
	int nCtrl = m_meshCtrlPoints.size();
	bvec = VectorXr(2 * nCtrl);
	for (int i = 0; i < nCtrl; i++) {
		bvec(i) = m_texCtrlPoints[i](0); // U 좌표
		bvec(i + nCtrl) = m_texCtrlPoints[i](1); // V 좌표
	}
}
