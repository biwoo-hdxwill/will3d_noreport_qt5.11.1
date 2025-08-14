#pragma once


#if _WIN64
#include "solver_global.h"
#include <Util/GPUUtil/clsparse_global_setting.h>
#include "cgsolver_typedef.h"
#include "Coef.h"
#include <vector>
#include "CLWrapper.h"

class SOLVER_EXPORT CGSolverCL {
public: /* members */
	typedef cgsolver_real real;
	int m_maxIter;
	real m_relTol;
	real m_absTol;
	std::vector<real> m_x0;
protected: /* members */
	/* cl sparse matrix */
	bool isMatInitialized;
	CLWrapper m_clw;
	cl::Context m_context;
	cl::CommandQueue m_queue;
	clsparseCreateResult m_createResult;
	clsparseCsrMatrix A;
	bool A_isInitialized;
protected: /* methods */
	void _init();
	void _constr(int maxIter, float relTol2, float absTol2, const std::vector<real>& x0);
public: /* methods */
	CGSolverCL();
	~CGSolverCL();
	void releaseMat();
	void release();
	bool compute(int m, int n, std::vector<Coef> coefs);
	bool solve(std::vector<real>& xSolved, const std::vector<real>& b);
};
#else
#endif
