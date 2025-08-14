#if _WIN64

#include "CGSolverCL.h"
#include <iostream>
#include <exception>
#include <algorithm>
#define MATLAB_DEBUG 0
#if MATLAB_DEBUG
#include "IOParser.h"
#endif

using namespace std;
void CGSolverCL::_init() {
	m_maxIter = 0;
	m_relTol = 1e-4;
	m_absTol = 1e-6;
	m_x0 = vector<real>();
	isMatInitialized = false;
	cout << "CGSolverCL clw init" << endl;
#if defined(__APPLE__)
    m_clw = CLWrapper("Apple", CL_DEVICE_TYPE_GPU);
#else
	m_clw = CLWrapper("NVIDIA", CL_DEVICE_TYPE_GPU);
#endif
	cout << "CGSolverCL clw init done" << endl;
	m_context = m_clw.context;
	m_queue = m_clw.queue;
	cls(clsparseSetup());
}
void CGSolverCL::_constr(int maxIter, float relTol2, float absTol2, const std::vector<real>& x0) {
	m_maxIter = maxIter;
	m_relTol = relTol2;
	m_absTol = absTol2;
	m_x0 = x0;
}
CGSolverCL::CGSolverCL() {
	_init();
}
CGSolverCL::~CGSolverCL() {
	release();
}
void CGSolverCL::release() {
	cls(clsparseTeardown());
	releaseMat();
	_init();
}
void CGSolverCL::releaseMat() {
	if (isMatInitialized == true) {
		cls(clsparseReleaseControl(m_createResult.control));
		cls(clsparseCsrMetaDelete(&A));
		cle(clReleaseMemObject(A.values));
		cle(clReleaseMemObject(A.col_indices));
		cle(clReleaseMemObject(A.row_pointer));
		isMatInitialized = false;
	}
}
bool CGSolverCL::compute(int m, int n, std::vector<Coef> coefs) {
	try {
		if (m <= 0) {
			throw runtime_error("m <= 0");
		}
		if (n <= 0) {
			throw runtime_error("n <= 0");
		}
		cl_int err;
		/* setup clsparse matrix */
		releaseMat();
		m_createResult = clsparseCreateControl(m_queue());
		cls(m_createResult.status);
		clsparseInitCsrMatrix(&A);
#if MATLAB_DEBUG
		clsparseHeaderfromFile(&A.num_nonzeros, &A.num_rows, &A.num_cols, "data.mtx");
		cout << "m=" << m << ", A_num_row=" << A.num_rows << endl;
		cout << "n=" << n << ", A.num_col=" << A.num_cols << endl;
		cout << "nnz=" << coefs.size() << ", A.nnz=" << A.num_nonzeros << endl;
#else
		A.num_rows = m;
		A.num_cols = n;
		A.num_nonzeros = coefs.size();
#endif
		
		
#if MATLAB_DEBUG
		cle2(A.values = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(real)*A.num_nonzeros, nullptr, &err), err);
		cle2(A.col_indices = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(clsparseIdx_t)*A.num_nonzeros, nullptr, &err), err);
		cle2(A.row_pointer = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(clsparseIdx_t)*(A.num_rows + 1), nullptr, &err), err);
		cls(clsparseSCsrMatrixfromFile(&A, "data.mtx", m_createResult.control, true));
		/* debug */
		vector<real> c_values(A.num_nonzeros);
		vector<clsparseIdx_t> c_col_indices(A.num_nonzeros);
		vector<clsparseIdx_t> c_row_pointer(A.num_rows + 1);
		cout << "readback debug A.num_nonzeros=" << A.num_nonzeros << endl;
		cle(clEnqueueReadBuffer(m_queue(), A.values, CL_TRUE, 0, sizeof(real)*A.num_nonzeros, c_values.data(), 0, 0, 0));
		cle(clEnqueueReadBuffer(m_queue(), A.col_indices, CL_TRUE, 0, sizeof(real)*A.num_nonzeros, c_col_indices.data(), 0, 0, 0));
		cle(clEnqueueReadBuffer(m_queue(), A.row_pointer, CL_TRUE, 0, sizeof(real)*(A.num_rows + 1), c_row_pointer.data(), 0, 0, 0));
		IOParser::writeRaw1d<real, real>(c_values, "values.raw");
		IOParser::writeRaw1d<clsparseIdx_t, clsparseIdx_t>(c_col_indices, "col_indices.raw");
		IOParser::writeRaw1d<clsparseIdx_t, clsparseIdx_t>(c_row_pointer, "row_pointer.raw");
#else
		/* host malloc */
		vector<real> c_val(A.num_nonzeros);
		vector<clsparseIdx_t> c_col(A.num_nonzeros);
		vector<clsparseIdx_t> c_row(A.num_rows + 1);
		/* make c_data */
        auto coefsLess = [](const Coef& a, const Coef& b)->bool {
            return a.r < b.r ||
                (a.r == b.r && a.c < b.c);
        };

        if (!std::is_sorted(coefs.begin(), coefs.end(), coefsLess)) {
            std::sort(coefs.begin(), coefs.end(), coefsLess);
		}
		int curRow = 0;
		int curnnz = 0;
		c_row[0] = 0;
		for (int i = 0; i < coefs.size(); i++) {
			const auto& coef = coefs[i];
			for (; curRow < coef.r; curRow++) {
				c_row[curRow + 1] = curnnz;
			}
			c_col[i] = coef.c;
			c_val[i] = coef.val;
			++curnnz;
		}
		for (; curRow < m; curRow++) {
			c_row[curRow + 1] = curnnz;
		}
		/* dev malloc */
		cle2(A.values = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(real)*A.num_nonzeros, nullptr, &err), err);
		cle2(A.col_indices = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(clsparseIdx_t)*A.num_nonzeros, nullptr, &err), err);
		cle2(A.row_pointer = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(clsparseIdx_t)*(A.num_rows + 1), nullptr, &err), err);
		/* h2d cpy */
		cle(clEnqueueWriteBuffer(m_queue(), A.values, CL_TRUE, 0, sizeof(real)*A.num_nonzeros, c_val.data(), 0, 0, 0));
		cle(clEnqueueWriteBuffer(m_queue(), A.col_indices, CL_TRUE, 0, sizeof(clsparseIdx_t)*A.num_nonzeros, c_col.data(), 0, 0, 0));
		cle(clEnqueueWriteBuffer(m_queue(), A.row_pointer, CL_TRUE, 0, sizeof(clsparseIdx_t)*(A.num_rows + 1), c_row.data(), 0, 0, 0));
#endif

		/* setup clsparse matrix */
		cls(clsparseCsrMetaCreate(&A, m_createResult.control));

		/* A is initialized */
		isMatInitialized = true;

		return true;
	}
	catch (runtime_error& e) {
		cout << "CGSolverCL::compute: " << e.what() << endl;
		return false;
	}
}
bool CGSolverCL::solve(std::vector<real>& xSolved, const std::vector<real>& bvec){
	try {
		if (bvec.size() != A.num_rows) {
			stringstream ss;
			ss << "bvec.size " << bvec.size() << " != " << "A.num_rows " << A.num_rows << endl;
			throw runtime_error(ss.str().c_str());
		}
		if (!m_x0.empty() && m_x0.size() != A.num_cols) {
			stringstream ss;
			ss << "m_x0.size " << m_x0.size() << " != " << "A.num_cols " << A.num_cols << endl;
			throw runtime_error(ss.str().c_str());
		}
		cl_int err;
		cldenseVector x;
		cls(clsparseInitVector(&x));
		cldenseVector b;
		cls(clsparseInitVector(&b));

		/* setup x */
		x.num_values = A.num_cols;
		cle2(x.values = clCreateBuffer(m_context(), CL_MEM_READ_ONLY, sizeof(real)*x.num_values, nullptr, &err), err);
		if (m_x0.empty()) {
			real zero = 0;
			cle(clEnqueueFillBuffer(m_queue(), x.values, &zero, sizeof(real), 0, sizeof(real)*x.num_values, 0, 0, 0));
		}
		else {
			cle(clEnqueueWriteBuffer(m_queue(), x.values, CL_TRUE, 0, sizeof(real)*x.num_values, m_x0.data(), 0, 0, 0));
		}

		/* setup b*/
		b.num_values = A.num_rows;
		cle2(b.values = clCreateBuffer(m_context(), CL_MEM_READ_WRITE, sizeof(real)*b.num_values, nullptr, &err), err);
		cle(clEnqueueWriteBuffer(m_queue(), b.values, CL_TRUE, 0, sizeof(real)*b.num_values, bvec.data(), 0, 0, 0));
		clsparseCreateSolverResult solverResult = clsparseCreateSolverControl(DIAGONAL, m_maxIter, m_relTol, m_absTol);
		cls(solverResult.status);
		cls(clsparseSolverPrintMode(solverResult.control, NORMAL));

		/* solve */
		cls(clsparseScsrcg(&x, &A, &b, solverResult.control, m_createResult.control));
		
		/* solution d2h cpy */
		xSolved.resize(x.num_values);
		cle(clEnqueueReadBuffer(m_queue(), x.values, CL_TRUE, 0, sizeof(real)*x.num_values, xSolved.data(), 0, 0, 0));

		cls(clsparseReleaseSolverControl(solverResult.control));
		
		/* release */
		cle(clReleaseMemObject(x.values));
		cle(clReleaseMemObject(b.values));

		return true;
	}
	catch (runtime_error& e) {
		cout << "CGSolverCL::solver: " << e.what() << endl;
		return false;
	}
}
#else

#endif
