#include "MySolver.h"
#include <iostream>
using namespace std;

MySolver::MySolver(){
	maxIter = 800;
	tol = 1e-6;
	curIter = 0;
}
void MySolver::solve(VectorXr& x, const SparseMatrixr& A, const VectorXr& b, const VectorXr& x0){
	int dim = A.rows();
	/* solve cg */
	real tol2 = tol*tol;
	VectorXr Ap(dim);
	real rsold, alpha, rsnew;

	curIter = 0;
	x = x0;
	VectorXr r = b - A*x;
	VectorXr p = r;
	rsold = r.dot(r);
	for (int i = 0; i < maxIter && i < dim; i++){
		Ap = A*p;
		alpha = rsold / (Ap.dot(p));
		x += alpha*p;
		r -= alpha*Ap;
		rsnew = r.dot(r);
		if (rsnew < tol2){
			break;
		}
		p = r + rsnew / rsold*p;
		rsold = rsnew;
		curIter++;
	}
	
}
