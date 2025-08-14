#include "MeshMove3d.h"
#include <algorithm>
#include <Util/Core/Logger.hpp>
#if defined(_WIN32)
#include <iterator>
#endif
#if defined(__APPLE__)
#include <eigen3/Eigen/Sparse>
#else
#include <Eigen/Sparse>
#endif
#include "DisplacementField.h"
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// sub-routines
///////////////////////////////////////////////////////////////////////////////////////////////////
glm::mat3 MeshMove3d::getKst(int s, int t) const {
	glm::mat3 res(0.f);
	if (s == t) {
		/* get tetrahedrons containing vertex s */
		auto subTetras = m_vtm.vert2Tetra(s);
		/*
		if s is dangling point (not contained to any tetrahedrons), identity mat.
		see Special case study (1) tetrahedron에 포함안된 dangling point s_0가 존재하는 경우
		*/
		if (subTetras.empty()) {
			res = glm::mat3(1.f);
		}
		/*
		[K]_ss = sigma_i ([K_i]_ss) for i = tetrahedron idx containing vertex s
		see <힘 행렬2>
		*/
		else {
			for (const auto& tetra : subTetras) {
				res = res + getKist(tetra, s, s);
			}
		}
	} else {
		/* get tetrahedrons containing edge (s,t) */
		auto subTetras = m_etm.edge2Tetra(tora::Edge(s, t));
		/*
		[K]_st = sigma_i ([K_i]_st) for i = tetrahedron idx containing edge (s,t)
		see <힘 행렬2>
		*/
		for (const auto& tetra : subTetras) {
			res = res + getKist(tetra, s, t);
		}
	}
	return res;
}

glm::mat3 MeshMove3d::getKist(const std::vector<int>& tetrahedron, int s, int t) const {
	glm::vec3 p[4];
	glm::vec3 v[4];
	for (int i = 0; i < 4; i++) {
		p[i] = m_points[tetrahedron[i]];
	}
	for (int i = 0; i < 4; i++) {
		v[i] = p[i] - p[0];
	}
	glm::mat3 V(v[1], v[2], v[3]);

	/* find M_is where i indicates tetrahedron's idx of "const std::vector<int>& tetrahedron" */
	auto Ms = getMvector(tetrahedron, s);
	/* find M_it */
	auto Mt = getMvector(tetrahedron, t);

	/*
	see <힘 행렬>
	cf) 6 * abs(glm::determinant(V)) = 36 * vol(tetrahedron)
	*/
	return 1.f / (6.f*abs(glm::determinant(V))) *
		(m_lambda * glm::outerProduct(Ms, Mt) +
			m_mu*glm::outerProduct(Mt, Ms) +
			m_mu*glm::dot(Ms, Mt) * glm::mat3(1.f));
}

glm::vec3 MeshMove3d::getMvector(const std::vector<int>& tetrahedron, const int pointIdx) const {
	/* tetrahedron points */
	glm::vec3 p[4];
	/* Tetrahedron local axis */
	glm::vec3 v[4];
	for (int i = 0; i < 4; i++) {
		p[i] = m_points[tetrahedron[i]];
	}
	for (int i = 0; i < 4; i++) {
		v[i] = p[i] - p[0];
	}
	glm::mat3 V(v[1], v[2], v[3]);

	/* find idx j in tetrahedron, s.t tetrahedron[j] = pointIdx */
	int j = getInnerIdx(tetrahedron, pointIdx);
	/* sgn operator */
	auto sgn = [](float val)->float {
		return (val > 0) - (val < 0);
	};
	/* mod operator s.t v --> {a, a+1, ... , b-1, b} */
	auto mod2 = [](int v, int a, int b) {
		return (v - a) % (b - a + 1) + a;
	};
	/* function to make v_j hat*/
	auto vh = [&mod2, &v](int j)->glm::vec3 {
		return glm::cross(v[mod2(j + 1, 1, 3)], v[mod2(j + 2, 1, 3)]);
	};

	if (j != 0) {
		return -sgn(glm::determinant(V)) * vh(j);
	}
	/*
	if j == 0
	see Lemmas for Barycentric interpolation in Tetrahedron
	*/
	else {
		return -sgn(glm::determinant(V)) * (-vh(1) - vh(2) - vh(3));
	}
}

int MeshMove3d::getInnerIdx(const std::vector<int>& tetrahedron, const int pointIdx) const {
	auto it = std::find(tetrahedron.begin(), tetrahedron.end(), pointIdx);
	return (it != tetrahedron.end()) ? std::distance(tetrahedron.begin(), it) : -1;
}

void MeshMove3d::constructStructure() {
	lg.tic("constr structure ...");
	/* construct each mapper */
	m_etm = EdgeTetraMapper(m_tetrahedrons);
	m_vem = VertEdgeMapper(m_tetrahedrons);
	m_vtm = VertTetraMapper(m_tetrahedrons);
	lg.toc();
}

void MeshMove3d::initFreeIdxs() {
	// init some obj
	lg.tic("init some obj ...");
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;

	freeIdxs.clear();
	{
		freeIdxs.reserve(nJoints);
		vector<int> allIdxs;
		allIdxs.reserve(nPoints);
		for (int i = 0; i < nPoints; i++) {
			allIdxs.push_back(i);
		}
		auto jointIdxsTemp = m_jointIdxs;
		std::sort(jointIdxsTemp.begin(), jointIdxsTemp.end());
		std::set_difference(allIdxs.begin(), allIdxs.end(), jointIdxsTemp.begin(), jointIdxsTemp.end(), std::back_inserter(freeIdxs));
	}
	lg.toc();
}

void MeshMove3d::initKcoef() {
	lg.tic("init kcoef ...");
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
	/* get all edges */
	const auto& edges = m_vem.getEdges();

	kcoef.clear();
	/* for all edges, calculate and save [K]_st */
	for (const auto& edge : edges) {
		kcoef.set(edge.v0, edge.v1, getKst(edge.v0, edge.v1));
		kcoef.set(edge.v1, edge.v0, getKst(edge.v1, edge.v0));
	}
	/* for all vertexs, calcuate and save [K]_ss */
	for (int iv = 0; iv < nPoints; iv++) {
		kcoef.set(iv, iv, getKst(iv, iv));
	}
	lg.toc();
}

void MeshMove3d::initK() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
	const auto& edges = m_vem.getEdges();

	// 1. K (3nPoints*3nPoints)
	K = SparseMatrixr(3 * nPoints, 3 * nPoints);
	{
		vector<SparseCoefr> spCoef;
		/*
		for all vertex i in vertexs
		K(3*i:3*i+2, 3*i:3*i+2) = kcoef.get(i,i);
		*/
		for (int i = 0; i < nPoints; i++) {
			Kcoef::Val mat3;
			if (kcoef.get(mat3, i, i)) {
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 3; k++) {
						spCoef.push_back(SparseCoefr(3 * i + j, 3 * i + k, mat3[k][j])); // glm::mat3[k][j] is mat3(j,k) element
					}
				}
			}
		}
		/*
		for all edge in edges
		(i0, i1) = (edge.v0, edge.v1)
		K(3*i0:3*i0+2, 3*i1:3*i1+2) = kcoef.get(i0,i1) = kcoef.get(i0, i1);
		*/
		for (const auto& edge : edges) {
			int i0 = edge.v0;
			int i1 = edge.v1;
			Kcoef::Val mat3;
			if (kcoef.get(mat3, i0, i1)) {
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 3; k++) {
						spCoef.push_back(SparseCoefr(3 * i0 + j, 3 * i1 + k, mat3[k][j])); // glm::mat3[k][j] is mat3(j,k) element
					}
				}
			}
		}
		/*
		for all edge in edges
		(i0, i1) = (edge.v1, edge.v0)
		K(3*i0:3*i0+2, 3*i1:3*i1+2) = kcoef.get(i0,i1) = kcoef.get(i0, i1);
		*/
		for (const auto& edge : edges) {
			int i0 = edge.v1;
			int i1 = edge.v0;
			Kcoef::Val mat3;
			if (kcoef.get(mat3, i0, i1)) {
				for (int j = 0; j < 3; j++) {
					for (int k = 0; k < 3; k++) {
						spCoef.push_back(SparseCoefr(3 * i0 + j, 3 * i1 + k, mat3[k][j])); // glm::mat3[k][j] is mat3(j,k) element
					}
				}
			}
		}

		K.setFromTriplets(spCoef.begin(), spCoef.end());
	}
}

void MeshMove3d::initZ() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
	const auto& edges = m_vem.getEdges();

	// 2. Z (3nPoints*3nFrees)
	Z = SparseMatrixr(3 * nPoints, 3 * nFrees);
	{
		vector<SparseCoefr> spCoef;
		/*
		for all j = 0, ..., nJoint-1
		i = freeIdxs(j) and
		Z(3*i:3*i+2, 3*j:3*j+2) = eye(3);
		*/
		for (int j = 0; j < nFrees; j++) {
			int i = freeIdxs[j];
			for (int k = 0; k < 3; k++) {
				spCoef.push_back(SparseCoefr(3 * i + k, 3 * j + k, 1));
			}
		}
		Z.setFromTriplets(spCoef.begin(), spCoef.end());
	}
}

void MeshMove3d::initb() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
	const auto& edges = m_vem.getEdges();

	// 3. make b vector (3nPoints * 1)
	b = MatrixXr::Zero(3 * nPoints, 1);
	for (int j = 0; j < nJoints; j++) {
		int i = m_jointIdxs[j];
		auto jointPoint = m_jointDisplacements[j];
		for (int k = 0; k < 3; k++) {
			b(3 * i + k, 0) = jointPoint[k];
		}
	}
}

void MeshMove3d::initC() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
	const auto& edges = m_vem.getEdges();

	// 4. C (3nFrees * 3nPoints) (= Z^T)
	C = SparseMatrixr(3 * nFrees, 3 * nPoints);
	{
		vector<SparseCoefr> spCoef;
		// i = freeIdxs(dd(j));
		// C(dd(3 * j:3 * j + 2), dd(3 * i:3 * i + 2)) = eye(3);
		for (int j = 0; j < nFrees; j++) {
			int i = freeIdxs[j];
			for (int k = 0; k < 3; k++) {
				spCoef.push_back(SparseCoefr(3 * j + k, 3 * i + k, 1));
			}
		}
		C.setFromTriplets(spCoef.begin(), spCoef.end());
	}
}

void MeshMove3d::setupCG() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;

#if USE_CUDA /* cuda cg */
	// setup cg
	lg.ticout() << "GPU CG setup (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;
	int maxIter = 1000;
	cg.m_maxIter = maxIter;
	cg.m_absTol = 5e-6;
	cg.m_relTol = 5e-6;
	Amat = C*K*Z;
	typedef cgsolver_real real;
	vector<Coef> coefs;
	coefs.reserve(Amat.nonZeros());
	for (int k = 0; k < Amat.outerSize(); k++) {
		for (SparseMatrixr::InnerIterator it(Amat, k); it; ++it) {
			coefs.push_back(Coef(it.row(), it.col(), it.value()));
		}
	}
	cg.compute(Amat.rows(), Amat.cols(), coefs);
	lg.toc();
#elif USE_CL /* cl cg */
	// setup cg
	lg.ticout() << "GPU CG setup (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;
	cg.m_maxIter = 1000;
	cg.m_absTol = 5e-6;
	cg.m_relTol = 5e-6;
	Amat = C*K*Z;
	typedef cgsolver_real real;
	vector<Coef> coefs;
	coefs.reserve(Amat.nonZeros());
	for (int k = 0; k < Amat.outerSize(); k++) {
		for (SparseMatrixr::InnerIterator it(Amat, k); it; ++it) {
			coefs.push_back(Coef(it.row(), it.col(), it.value()));
		}
	}
	cg.compute(Amat.rows(), Amat.cols(), coefs);
	lg.toc();
#else /* eigen cg */
	// setup cg
	lg.ticout() << "Eigen CG setup (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;
	int maxIter = 1000;
	float tol = 5e-6;

	lg << "maxIter is set to " << maxIter << endl;
	lg << "tol is set to " << tol << endl;
	cg.setMaxIterations(maxIter);
	cg.setTolerance(tol);

	Amat = C*K*Z;
	cg.compute(Amat);

	lg.toc();
#endif
}

void MeshMove3d::solveCG(const std::vector<cgsolver_real>& guess) {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;
#if USE_CUDA /* cuda cg */
	// solve cg
	lg.ticout() << "GPU CG solve (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;

	/* cg guess init */
	cg.m_x0 = guess;

	/*
	solve cg C*K*Z*x=-C*K*b
	Amat = C*K*Z
	bvec = -C*K*b
	x = u_sol_free
	(Z*x + b) = u_sol
	*/
	bvec = -C*K*b;
	typedef cgsolver_real real;
	vector<real> xSolved;
	vector<real> rhs(bvec.size());
	for (int i = 0; i < rhs.size(); i++) {
		rhs[i] = bvec[i];
	}

    lg.ticout() << "GPU CG solve only (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;
	cg.solve(xSolved, rhs);
	lg.toc();

	u_sol_free.resize(xSolved.size());
	for (int i = 0; i < xSolved.size(); i++) {
		u_sol_free[i] = xSolved[i];
	}
	u_sol = Z*u_sol_free + b;
	lg << "iter=" << cg.mr_iter << endl;
	lg << "abs err=" << cg.mr_absErr << endl;
	lg << "rel err=" << cg.mr_relErr << endl;
	lg.toc();
#elif USE_CL /* cl cg */
	// solve cg
	lg.ticout() << "GPU CG solve (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;

	/* cg guess init */
	cg.m_x0 = guess;

	/*
	solve cg C*K*Z*x=-C*K*b
	Amat = C*K*Z
	bvec = -C*K*b
	x = u_sol_free
	(Z*x + b) = u_sol
	*/
	bvec = -C*K*b;
	typedef cgsolver_real real;
	vector<real> xSolved;
	vector<real> rhs(bvec.size());
	for (int i = 0; i < rhs.size(); i++) {
		rhs[i] = bvec[i];
	}

	lg.ticout() << "GPU CG solve only (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;;
	cg.solve(xSolved, rhs);
	lg.toc();

	u_sol_free.resize(xSolved.size());
	for (int i = 0; i < xSolved.size(); i++) {
		u_sol_free[i] = xSolved[i];
	}
	u_sol = Z*u_sol_free + b;
	lg.toc();
#else /* eigen cg */
    // solve cg
    lg.ticout() << "Eigen CG solve (" << 3 * nFrees << "," << 3 * nFrees << ")..." << endl;

	VectorXr x0(3 * nFrees);
	int nFrees3 = 3 * nFrees;
	for (int i = 0; i < nFrees3; i++) {
		x0[i] = guess[i];
	}

	/*
	solve cg C*K*Z*x=-C*K*b
	Amat = C*K*Z
	bvec = -C*K*b
	x = u_sol_free
	(Z*x + b) = u_sol
	*/
	bvec = -C*K*b;
	u_sol_free = cg.solveWithGuess(bvec, x0);
    u_sol = Z*u_sol_free + b;
	lg << "iter=" << cg.iterations() << endl;
	lg << "estimated err=" << cg.error() << endl;
    lg.toc();
#endif
}

void MeshMove3d::chkForce() {
	int nPoints = m_points.size();
	int nJoints = m_jointIdxs.size();
	int nFrees = nPoints - nJoints;

	lg.tic("chk force ...");

	MatrixXr forceAll = MatrixXr::Zero(3 * nPoints, 1);
	for (int s = 0; s < nPoints; s++) {
		Eigen::Matrix3f mat;
		if (kcoef.get(mat, s, s)) {
			forceAll.block<3, 1>(3 * s, 0) += mat.cast<real>()*u_sol.block<3, 1>(3 * s, 0);
		}
		auto nei = m_vem.neighbor(s);
		for (auto t : nei) {
			if (kcoef.get(mat, s, t)) {
				forceAll.block<3, 1>(3 * s, 0) += mat.cast<real>()*u_sol.block<3, 1>(3 * t, 0);
			}
		}
	}
	//cout << "#" << indent() << "residual(rel)=" << (Amat*u_sol_free - Matrix(bvec)).norm() / bvec.norm() << endl;

	lg << "forceAll=" << forceAll.norm() << endl;
	lg << "forceAll(rel)=" << forceAll.norm() / (K*b).norm() << endl;

	MatrixXr forceFree = MatrixXr::Zero(3 * nFrees, 1);
	for (int j = 0; j < nFrees; j++) {
		int i = freeIdxs[j];
		forceFree.block<3, 1>(3 * j, 0) = forceAll.block<3, 1>(3 * i, 0);
	}

	lg << "forceFree=" << forceFree.norm() << endl;
	lg << "forceFree(rel)=" << forceFree.norm() / (C*K*b).norm() << endl;
	lg.toc();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// public methods
///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshMove3d::init() {
	m_points.clear();
	m_jointIdxs.clear();
	m_jointDisplacements.clear();
	m_tetrahedrons.clear();
	m_etm = EdgeTetraMapper();
	m_vem = VertEdgeMapper();
	m_vtm = VertTetraMapper();
	freeIdxs.clear();
	kcoef.clear();
	K = SparseMatrixr();
	Z = SparseMatrixr();
	b = VectorXr();
	Amat = SparseMatrixr();
	bvec = VectorXr();
#if USE_CUDA || USE_CL
	cg.release();
#endif
	u_sol_free = VectorXr();
	u_sol = VectorXr();
}
bool MeshMove3d::constr(
	const std::vector<glm::vec3>& points,
	const std::vector<int>& jointIdxs,
	const std::vector<std::vector<int>>& tetrahedrons,
	const float E,
	const float v
) {
	try {
		init();
		m_points = points;
		m_jointIdxs = jointIdxs;
		m_tetrahedrons = tetrahedrons;
		m_mu = E / (2 * (1 + v));
		/* Lame's first parameter */
		m_lambda = E / ((1 + v)*(1 - 2 * v));
		/* m_etm, m_vem, m_vtm 초기화 */
		constructStructure();
		/* free idx set 계산 */
		initFreeIdxs();
		/*
		K coefficient structure to make K sparse matrix.
		this only depends on tetrahedron structure (not depends on joint displacements).
		so, calculate K coefficient in constructor.
		*/
		initKcoef();
		/*
		K sparse matrix
		this only depends on tetrahedron structure (not depends on joint displacements).
		so, calculate K sparse matrix in constructor.
		*/
		initK();
		/*
		Z sparse matrix
		this only depends on tetrahedron structure and joint idx set (not depends on joint displacements).
		so, calculate Z sparse matrix in constructor.
		*/
		initZ();
		/*
		C (Z^T) sparse matrix
		this only depends on tetrahedron structure and joint idx set (not depends on joint displacements).
		so, calculate C (Z^T) sparse matrix in constructor.
		*/
		initC();
		/*
		setup Eigen cg solver
		*/
		setupCG();
		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshMove3d::constr: " << e << endl;

		return false;
	}
}
MeshMove3d::MeshMove3d() {
	init();
}

MeshMove3d::MeshMove3d(
	const std::vector<glm::vec3>& points,
	const std::vector<int>& jointIdxs,
	const std::vector<std::vector<int>>& tetrahedrons,
	const float E,
	const float v
) {
	constr(points, jointIdxs, tetrahedrons, E, v);
}

bool MeshMove3d::execute(
	std::vector<glm::vec3>& queryDisplacements,
	const std::vector<glm::vec3>& jointDisplacements,
	const std::vector<glm::vec3>& queryDisplacements0
) {
	try {
		m_jointDisplacements = jointDisplacements;
		int nPoints = m_points.size();
		int nJoints = m_jointIdxs.size();
		int nFrees = nPoints - nJoints;

		// constr K (3nPoints*3nPoints), Z (3nPoints*3nFrees), b(3nPoints*1), C (3nFrees*3n)
		lg.tic("constr b mat ...");

		/* init b */
		initb();
		lg.toc();

		/* solve cg */
		vector<cgsolver_real> guess(3 * nFrees);
		for (int i = 0; i < nFrees; i++) {
			const auto& p = queryDisplacements0[freeIdxs[i]];
			guess[3 * i + 0] = p[0];
			guess[3 * i + 1] = p[1];
			guess[3 * i + 2] = p[2];
		}
		solveCG(guess);

		/* chk force */
		// chkForce(); ///< for debug

		/* make result */
		lg.tic("make result ...");

		queryDisplacements.resize(m_points.size());
		for (int i = 0; i < nPoints; i++) {
			queryDisplacements[i] = glm::vec3(u_sol(3 * i + 0, 0), u_sol(3 * i + 1, 0), u_sol(3 * i + 2, 0));
		}
		lg.toc();
		return true;
	} catch (std::runtime_error& e) {
		lg << "MeshMove3d::execute: " << e << endl;
		return false;
	}
}

MeshMove3d::~MeshMove3d() {
	init();
}
