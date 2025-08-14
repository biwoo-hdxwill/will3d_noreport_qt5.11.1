#pragma once
#include "solver_global.h"
#include "cgsolver_typedef.h"
class SOLVER_EXPORT Coef {
public:
	typedef cgsolver_real real;
	int r;
	int c;
	real val;
	Coef();
	Coef(int r, int c, real val);
	bool operator< (const Coef& b);
};
