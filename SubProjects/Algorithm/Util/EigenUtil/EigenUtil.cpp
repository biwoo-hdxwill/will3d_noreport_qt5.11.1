#include "EigenUtil.h"
#pragma message("## Util/EigenUtil/EigenUtil.cpp visited")
#include <exception>
using namespace std;

void EigenUtil::getSpCoef(
	std::vector<SparseCoefr>& spCoef,
	const SparseXr& mat,
	int rOffset,
	int cOffset
){
	for (int k = 0; k < mat.outerSize(); k++){
		for (SparseXr::InnerIterator it(mat, k); it; ++it){
			spCoef.push_back(SparseCoefr(it.row() + rOffset, it.col() + cOffset, it.value()));
		}
	}
} 
