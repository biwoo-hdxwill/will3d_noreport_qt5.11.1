#include "mxArrayWrapper.h"
#include <iostream>
#include <exception>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <boost/format.hpp>
#include "../Core/coutOverloading.h"
#include <iomanip>

#define UseMxArrayWrapperIdxDebug 1;
using namespace std;

/* mxComplexity
	mxREAL
	mxCOMPLEX
*/

/* mxClassID
	mxUNKNOWN_CLASS,
	mxCELL_CLASS,
	mxSTRUCT_CLASS,
	mxLOGICAL_CLASS,
	mxCHAR_CLASS,
	mxVOID_CLASS,
	mxDOUBLE_CLASS,
	mxSINGLE_CLASS,
	mxINT8_CLASS,
	mxUINT8_CLASS,
	mxINT16_CLASS,
	mxUINT16_CLASS,
	mxINT32_CLASS, <-- 메모리 에러 이상하게남. 상태가 좀 안좋은듯.
	mxUINT32_CLASS,
	mxINT64_CLASS,
	mxUINT64_CLASS,
	mxFUNCTION_CLASS
*/

void mxArrayWrapper::_init(){
	_ptr = nullptr;
	_ndims = 0;
	_nelems = 0;
	_classId = mxUNKNOWN_CLASS;
	_complexity = mxREAL;
	_nzmax = 0;
	//_percent_sparse = 0;
	name = "";
	string nameTemp = name;
	_deleter = [nameTemp](mxArray* ptr){
		// do nothing
		// mxArray* free operation will be done by MATLAB Engine // no
		// 1.
		// http://kr.mathworks.com/help/matlab/apiref/engputvariable.html
		// original, copy(MATLAB engine으로 넘겨준 copy) 중에서 copy는 MATLAB Engine이 알아서하지만
		// original은 app이 메모리관리에 대한 책임이 있다고함.
		// 고로 engPutVariable해도 mxDestroyArray 해줘야함
		// 2.
		// http://kr.mathworks.com/help/matlab/apiref/enggetvariable.html
		// engGetVariable된 것은 설명이 없네...
		//cout << "[" << nameTemp  << "]: mxArrayWrapper: _delete ... ";
		mxDestroyArray(ptr);
		//cout << "done" << endl;
	};
}
void mxArrayWrapper::_constrSparse(
	mwSize m,
	mwSize n,
	mxComplexity complexity,
	const std::string& name

){
	try{
		_init();
		_dims = vector<mwSize>();
		_dims.push_back(m);
		_dims.push_back(n);
		_ndims = 2;
		_nelems = std::accumulate(_dims.begin(), _dims.end(), uint64(1), std::multiplies<mwSize>());
		_classId = mxDOUBLE_CLASS;
		_complexity = complexity;
		double percent_sparse = min(1e4 / (m*n), 0.01);
		_nzmax = (mwSize)ceil(m*n*percent_sparse);
		_ptr = shared_ptr<mxArray>(mxCreateSparse(m, n, _nzmax, _complexity), _deleter);
		this->name = name;
	}
	catch (exception& e){
		_init();
		this->name = name;
		cout << "mxArrayWrapper::_constr: " << e.what() << endl;
	}

}
void mxArrayWrapper::_constr(
	const std::vector<mwSize>& dims,
	mxClassID classId,
	mxComplexity complexity, 
	const std::string& name
){
	try{
		_init();
		if (dims.empty()){
			throw exception("dims is empty");
		}
		for (auto dim : dims){
			if (dim < 0){
				throw exception("dim is negative");
			}
		}
		for (auto dim : dims){
			if (dim == 0){
				cout << "empty matrix (dims=" << dims << ")" << endl;
				break;
			}
		}
		
		_dims = dims;
		_ndims = _dims.size();
		_nelems = std::accumulate(_dims.begin(), _dims.end(), uint64(1), std::multiplies<mwSize>());
		_classId = classId;
		_complexity = complexity;
		_ptr = shared_ptr<mxArray>(mxCreateNumericArray(_ndims, _dims.data(), _classId, _complexity), _deleter);
		this->name = name;
	}
	catch (exception& e){
		_init();
		this->name = name;
		cout << "mxArrayWrapper::_constr: " << e.what() << endl;
	}
}

void mxArrayWrapper::_constr(mxArray* ptr, const std::string& name){
	try{
		if (!ptr){
			throw exception("ptr is nullptr err");
		}
		_init();
		_ptr = shared_ptr<mxArray>(ptr, _deleter);
		mwSize ndims = mxGetNumberOfDimensions(ptr);
		const mwSize* dims_ptr = mxGetDimensions(ptr);
		
		_dims.assign(dims_ptr, dims_ptr + ndims);
		_ndims = ndims;
		_nelems = std::accumulate(_dims.begin(), _dims.end(), uint64(1), std::multiplies<mwSize>());
		_classId = mxGetClassID(ptr);
		if (_classId == mxSTRUCT_CLASS		||
			_classId == mxFUNCTION_CLASS	|| 
			_classId == mxCELL_CLASS		||
			_classId == mxVOID_CLASS		){
			stringstream ss;
			ss << "classId(=" << getClassIdStr() << ") is not yet supported";
			throw exception(ss.str().c_str());
		}
		_complexity = mxIsComplex(ptr) ? mxCOMPLEX : mxREAL;
		this->name = name;
	}
	catch (exception& e){
		_init();
		this->name = name;
		cout << "mxArrayWrapper::_constr: " << e.what() << endl;
	}
}

#if UseMxArrayWrapperIdxDebug
bool mxArrayWrapper::_idxErrChk(const std::vector<mwIndex>& idxs) const{
	// mat == idxs
	stringstream ss;
	try{
		if (_ndims == idxs.size()){
			// 1. each idx need to meet each dim
			for (int i = 0; i < _ndims; i++){
				if (!(idxs[i] < _dims[i])){
					ss << "@at mat == idx case, @failed= 1. each idx need to meet each dim." << endl;
					ss << "\tidxs["<<i<<"](=" << idxs[i] << ") >= _dims["<<i<<"](=" << _dims[i] << ")";
					throw exception(ss.str().c_str());
					return false;
				}
			}
		}
		// mat > idxs
		else if (_ndims > idxs.size()){
			// 1. each idx need to meet each dim for idxs.size - 1
			for (int i = 0; i < idxs.size() - 1; i++){
				if (!(idxs[i] < _dims[i])){
					ss << "@at mat > idx case, @failed= 1. each idx need to meet each dim for idxs.size - 1." << endl;
					ss << "\tidxs[" << i << "](=" << idxs[i] << ") >= _dims[" << i << "](=" << _dims[i] << ")";
					throw exception(ss.str().c_str());
					return false;
				}
			}

			// 2. need that cumulated allIdx < _nelems
			mwIndex allIdx = 0;
			for (int i = 0; i < idxs.size(); i++){
				allIdx += std::accumulate(_dims.begin(), _dims.begin() + i, uint64(1), std::multiplies<mwIndex>())
					* idxs[i];
			}
			if (_nelems < allIdx){
				ss << "@at mat > idx case, @failed= 2. need that cumulated allIdx < _nelems." << endl;
				ss << "\t" << "allIdx(=" << allIdx << ") < " << "_nelems(=" << _nelems << ")";
				throw exception(ss.str().c_str());
				return false;
			}
		}
		// mat < idxs
		else if (_ndims < idxs.size()){
			// 1. each idx need to meet each dim
			for (int i = 0; i < _ndims; i++){
				if (!(idxs[i] < _dims[i])){
					ss << "@at  mat < idxs case, @failed= 1. each idx need to meet each dim." << endl;
					ss << "\t" << "idxs[" << i << "](=" << idxs[i] << ") >= _dims[" << i << "](=" << _dims[i] << ")";
					throw exception(ss.str().c_str());
					return false;
				}
			}

			// 2. beyond idxs need to be 0
			for (int i = _ndims; i < idxs.size(); i++){
				if (idxs[i] != 0){
					ss << "@at  mat < idxs case, @failed= 2. beyond idxs need to be 0." << endl;
					ss << "\t" << "idxs[" << i << "](=" << idxs[i] << ") != 0";
					throw exception(ss.str().c_str());
					return false;
				}
			}
		}
		return true;
	}
	catch (exception& e){
		cout << "mxArrayWrapper::_idxErrChk: " << e.what() << endl;
		return false;
	}
}
#else
bool mxArrayWrapper::_idxErrChk(const std::vector<mwIndex>& idxs) const{
	return true;
}
#endif

/////////////////////////////////////////////////////////// cretae & delete //////////////////////////////////////////////////
void mxArrayWrapper::clear(){
	_init();
}
mxArrayWrapper::~mxArrayWrapper(){

}

mxArrayWrapper::mxArrayWrapper(){
	_init();
}
mxArrayWrapper::mxArrayWrapper(mxArray* ptr, const std::string& name){
	_constr(ptr, name);
}
//mxArrayWrapper::mxArrayWrapper(const std::vector<mwSize>& dims, mxClassID classId, mxComplexity complexity){
//	_constr(dims, classId, complexity, "");
//}
mxArrayWrapper::mxArrayWrapper(const std::vector<mwSize>& dims, mxClassID classId, mxComplexity complexity, const std::string& name){
	_constr(dims, classId, complexity, name);
}
mxArrayWrapper::mxArrayWrapper(
	mwSize m,
	mwSize n,
	mxComplexity complexity,
	const std::string& name
){
	_constrSparse(m, n, complexity, name);
}
/////////////////////////////////////////////////////////// IO //////////////////////////////////////////////////
template<class V>
bool mxArrayWrapper::copy(const std::vector<V>& data){
	try{
		if (data.size() != this->size()){
			stringstream ss;
			ss << "data.size(=" << data.size() << ") != " << "mxArrayWrapper.size(=" << this->size() << ")";
			throw exception(ss.str().c_str());
		}
		void* cur = mxGetData(getPtr());
		V* dataPtr = static_cast<V*>(cur);
		for (int i = 0; i < data.size(); i++){
			dataPtr[i] = static_cast<V>(data[i]);
		}
		return true;
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::copy: " << e.what() << endl;
		return false;
	}
}
template<class V>
bool mxArrayWrapper::copy(const V* data, mwSize dataSize){
	try{
		if (dataSize != this->size()){
			stringstream ss;
			ss << "data.size(=" << dataSize << ") != " << "mxArrayWrapper.size(=" << this->size() << ")";
			throw exception(ss.str().c_str());
		}
		void* cur = mxGetData(getPtr());
		V* dataPtr = static_cast<V*>(cur);
		for (int i = 0; i < dataSize; i++){
			dataPtr[i] = static_cast<V>(data[i]);
		}
		return true;
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::copy: " << e.what() << endl;
		return false;
	}
}

/////////////////////////////////////////////////////////// access //////////////////////////////////////////////////

template<class V>
V mxArrayWrapper::atr(const std::vector<mwIndex>& idxs) const {
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			stringstream ss;
			ss << "type of " << getClassIdStr() << "is not supported for sparse matrix. "
				<< "sparse matrix only supports double type";
			throw exception(ss.str().c_str());
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::getr: " << e.what() << endl;
		return V(0);
	}
}

// template specification for double. double type is available for sparse matrix
template<>
double mxArrayWrapper::atr<double>(const std::vector<mwIndex>& idxs) const {
	typedef double V;
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			if (idxs.size() != 2){
				throw exception("sprase matrix idxs must be 2d");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex i_ = idxs[0];
			mwIndex j_ = (idxs.size() < 2) ? 0 : idxs[1]; // ?
			mwIndex* irs;
			mwIndex* jcs;
			irs = mxGetIr(getPtr());
			jcs = mxGetJc(getPtr());
			double* sr = mxGetPr(getPtr());
			mwSize nnz = jcs[_dims[1]];
			auto it = std::find(irs + jcs[j_], irs + jcs[j_ + 1], i_);
			if (it != irs + jcs[j_ + 1]){
				return sr[std::distance(irs, it)];
			}
			else return V(0);
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::getr: " << e.what() << endl;
		return V(0);
	}
}

template<class V>
V mxArrayWrapper::atr(mwIndex i0) const {
	vector<mwIndex> idxs(1);
	idxs[0] = i0;
	return atr<V>(idxs);
}
template<class V>
V mxArrayWrapper::atr(mwIndex i0, mwIndex i1) const {
	vector<mwIndex> idxs(2);
	idxs[0] = i0;
	idxs[1] = i1;
	return atr<V>(idxs);
}
template<class V>
V mxArrayWrapper::atr(mwIndex i0, mwIndex i1, mwIndex i2) const {
	vector<mwIndex> idxs(3);
	idxs[0] = i0;
	idxs[1] = i1;
	idxs[2] = i2;
	return atr<V>(idxs);
}
template<class V>
V mxArrayWrapper::ati(const std::vector<mwIndex>& idxs) const {
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetImagData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				return V(0);
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			stringstream ss;
			ss << "type of " << getClassIdStr() << "is not supported for sparse matrix. "
				<< "sparse matrix only supports double type";
			throw exception(ss.str().c_str());
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::geti: " << e.what() << endl;
		return V(0);
	}
}

template<>
double mxArrayWrapper::ati<double>(const std::vector<mwIndex>& idxs) const {
	typedef double V;
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetImagData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				return V(0);
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			if (idxs.size() != 2){
				throw exception("sprase matrix idxs must be 2d");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				return V(0);
			}
			mwIndex i_ = idxs[0];
			mwIndex j_ = (idxs.size() < 2) ? 0 : idxs[1]; // ?
			mwIndex* irs;
			mwIndex* jcs;
			irs = mxGetIr(getPtr());
			jcs = mxGetJc(getPtr());
			double* si = mxGetPi(getPtr());
			mwSize nnz = jcs[_dims[1]];
			auto it = std::find(irs + jcs[j_], irs + jcs[j_ + 1], i_);
			if (it != irs + jcs[j_ + 1]){
				return si[*it];
			}
			else return V(0);
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::geti: " << e.what() << endl;
		return V(0);
	}
}

template<class V>
V mxArrayWrapper::ati(mwIndex i0) const {
	vector<mwIndex> idxs(1);
	idxs[0] = i0;
	return ati<V>(idxs);
}
template<class V>
V mxArrayWrapper::ati(mwIndex i0, mwIndex i1) const {
	vector<mwIndex> idxs(2);
	idxs[0] = i0;
	idxs[1] = i1;
	return ati<V>(idxs);
}
template<class V>
V mxArrayWrapper::ati(mwIndex i0, mwIndex i1, mwIndex i2) const {
	vector<mwIndex> idxs(3);
	idxs[0] = i0;
	idxs[1] = i1;
	idxs[2] = i2;
	return ati<V>(idxs);
}

template<class V>
V& mxArrayWrapper::atr(const std::vector<mwIndex>& idxs){
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			stringstream ss;
			ss << "type of " << getClassIdStr() << "is not supported for sparse matrix. "
				<< "sparse matrix only supports double type";
			throw exception(ss.str().c_str());
			
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::setr: " << e.what() << endl;
		V dumy(0);
		return dumy;
	}
}


template<>
double& mxArrayWrapper::atr<double>(const std::vector<mwIndex>& idxs){
	typedef double V;
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			if (idxs.size() != 2){
				throw exception("sprase matrix idxs must be 2d");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			mwIndex i_ = idxs[0];
			mwIndex j_ = (idxs.size() < 2) ? 0 : idxs[1]; // ?
			mwIndex* irs;
			mwIndex* jcs;
			irs = mxGetIr(getPtr());
			jcs = mxGetJc(getPtr());
			double* sr = mxGetPr(getPtr());
			double* si = mxGetPi(getPtr());
			mwSize nnz = jcs[_dims[1]];
			auto it = std::find(irs + jcs[j_], irs + jcs[j_ + 1], i_);
			if (it != irs + jcs[j_ + 1]){
				return sr[*it];
			}
			else{
				// if capacity full
				if (nnz >= _nzmax){
					mwSize oldnzmax = _nzmax;
					_nzmax *= 2;
					if (oldnzmax == _nzmax){
						++_nzmax;
					}
					mxSetNzmax(getPtr(), _nzmax);
					mxSetPr(getPtr(), (double*)mxRealloc(sr, _nzmax*sizeof(double)));
					if (_complexity == mxCOMPLEX){
						mxSetPi(getPtr(), (double*)mxRealloc(si, _nzmax*sizeof(double)));
					}
					mxSetIr(getPtr(), (mwIndex*)mxRealloc(irs, _nzmax*sizeof(mwIndex)));
					sr = mxGetPr(getPtr());
					si = mxGetPi(getPtr());
					irs = mxGetIr(getPtr());
				}

				// insert value

				mwIndex insertedIdx = jcs[j_ + 1];
				for (int i = jcs[j_]; i < jcs[j_ + 1]; i++){
					if (i_ < irs[i]){
						insertedIdx = i;

						break;
					}
				}
				for (int j = j_ + 1; j <= _dims[1]; j++){
					++jcs[j];
				}
				for (int i = nnz; i > insertedIdx; i--){
					sr[i] = sr[i - 1];
					irs[i] = irs[i - 1];
				}
				sr[insertedIdx] = 0;
				irs[insertedIdx] = i_;
				if (_complexity == mxCOMPLEX){
					for (int i = nnz; i > insertedIdx; i--){
						si[i] = si[i - 1];
					}
					si[insertedIdx] = 0;
				}

				return sr[insertedIdx];
			}
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::setr: " << e.what() << endl;
		V dumy(0);
		return dumy;
	}
}

template<class V>
V& mxArrayWrapper::atr(mwIndex i0) {
	vector<mwIndex> idxs(1);
	idxs[0] = i0;
	return atr<V>(idxs);
}
template<class V>
V& mxArrayWrapper::atr(mwIndex i0, mwIndex i1) {
	vector<mwIndex> idxs(2);
	idxs[0] = i0;
	idxs[1] = i1;
	return atr<V>(idxs);
}
template<class V>
V& mxArrayWrapper::atr(mwIndex i0, mwIndex i1, mwIndex i2) {
	vector<mwIndex> idxs(3);
	idxs[0] = i0;
	idxs[1] = i1;
	idxs[2] = i2;
	return atr<V>(idxs);
}
template<class V>
V& mxArrayWrapper::ati(const std::vector<mwIndex>& idxs){
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetImagData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				throw exception("this object is not a COMPLEX");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			stringstream ss;
			ss << "type of " << getClassIdStr() << "is not supported for sparse matrix. "
				 << "sparse matrix only supports double type";
			throw exception(ss.str().c_str());
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::seti: " << e.what() << endl;
		V dumy(0);
		return dumy;
	}
}
template<>
double& mxArrayWrapper::ati<double>(const std::vector<mwIndex>& idxs){
	typedef double V;
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetImagData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				throw exception("this object is not a COMPLEX");
			}
			mwIndex allIdx = mxCalcSingleSubscript(getPtr(), idxs.size(), idxs.data());
			return static_cast<V*>(cur)[allIdx];
		}
		else{
			if (idxs.size() != 2){
				throw exception("sprase matrix idxs must be 2d");
			}
			if (!_idxErrChk(idxs)){
				throw exception("idx err");
			}
			if (_complexity != mxCOMPLEX){
				throw exception("this object is not a COMPLEX");
			}
			mwIndex i_ = idxs[0];
			mwIndex j_ = (idxs.size() < 2) ? 0 : idxs[1]; // ?
			mwIndex* irs;
			mwIndex* jcs;
			irs = mxGetIr(getPtr());
			jcs = mxGetJc(getPtr());
			double* sr = mxGetPr(getPtr());
			double* si = mxGetPi(getPtr());
			mwSize nnz = jcs[_dims[1]];
			auto it = std::find(irs + jcs[j_], irs + jcs[j_ + 1], i_);
			if (it != irs + jcs[j_ + 1]){
				return si[*it];
			}
			else{
				// if capacity full
				if (nnz >= _nzmax){
					mwSize oldnzmax = _nzmax;
					_nzmax *= 2;
					if (oldnzmax == _nzmax){
						_nzmax++;
					}
					mxSetNzmax(getPtr(), _nzmax);
					mxSetPr(getPtr(), (double*)mxRealloc(sr, _nzmax*sizeof(double)));
					if (_complexity == mxCOMPLEX){
						mxSetPi(getPtr(), (double*)mxRealloc(si, _nzmax*sizeof(double)));
					}
					mxSetIr(getPtr(), (mwIndex*)mxRealloc(irs, _nzmax*sizeof(mwIndex)));
					sr = mxGetPr(getPtr());
					si = mxGetPi(getPtr());
					irs = mxGetIr(getPtr());
				}

				// insert value

				mwIndex insertedIdx = jcs[j_ + 1];
				for (int i = jcs[j_]; i < jcs[j_ + 1]; i++){
					if (i_ < irs[i]){
						insertedIdx = i;

						break;
					}
				}
				for (int j = j_ + 1; j <= _dims[1]; j++){
					jcs[j] ++;
				}
				for (int i = nnz; i > insertedIdx; i--){
					sr[i] = sr[i - 1];
					si[i] = si[i - 1];
					irs[i] = irs[i - 1];
				}
				si[insertedIdx] = 0;
				irs[insertedIdx] = i_;

				return si[insertedIdx];
			}
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::seti: " << e.what() << endl;
		V dumy(0);
		return dumy;
	}
}
template<class V>
V& mxArrayWrapper::ati(mwIndex i0) {
	vector<mwIndex> idxs(1);
	idxs[0] = i0;
	return ati<V>(idxs);
}
template<class V>
V& mxArrayWrapper::ati(mwIndex i0, mwIndex i1) {
	vector<mwIndex> idxs(2);
	idxs[0] = i0;
	idxs[1] = i1;
	return ati<V>(idxs);
}
template<class V>
V& mxArrayWrapper::ati(mwIndex i0, mwIndex i1, mwIndex i2) {
	vector<mwIndex> idxs(3);
	idxs[0] = i0;
	idxs[1] = i1;
	idxs[2] = i2;
	return ati<V>(idxs);
}

void* mxArrayWrapper::datar(){
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			return cur;
		}
		else{
			throw exception("sparse matrix datar implemenation is not done yet."
				"dont use this function for sparse matrix");
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::datar: " << e.what() << endl;
		return nullptr;
	}
}

void* mxArrayWrapper::datai(){
	try{
		if (isNull()){
			throw exception("isNull err");
		}
		if (!isSparse()){
			void* cur = mxGetImagData(getPtr());
			if (!cur){
				throw exception("there is no value");
			}
			return cur;
		}
		else{
			throw exception("sparse matrix datar implemenation is not done yet."
				"dont use this function for sparse matrix");
		}
	}
	catch (exception& e){
		cout << "[varName=" << name << "]" << "mxArrayWrapper::datar: " << e.what() << endl;
		return nullptr;
	}
}


mxArrayWrapper::Val mxArrayWrapper::operator() (const std::vector<mwIndex>& idxs) const {
	return atr<Val>(idxs);
}
mxArrayWrapper::Val mxArrayWrapper::operator() (mwIndex i0) const {
	return atr<Val>(i0);
}
mxArrayWrapper::Val mxArrayWrapper::operator() (mwIndex i0, mwIndex i1) const {
	return atr<Val>(i0, i1);
}
mxArrayWrapper::Val mxArrayWrapper::operator() (mwIndex i0, mwIndex i1, mwIndex i2) const {
	return atr<Val>(i0, i1, i2);
}

mxArrayWrapper::Val& mxArrayWrapper::operator() (const std::vector<mwIndex>& idxs) {
	return atr<Val>(idxs);
}
mxArrayWrapper::Val& mxArrayWrapper::operator() (mwIndex i0) {
	return atr<Val>(i0);
}
mxArrayWrapper::Val& mxArrayWrapper::operator() (mwIndex i0, mwIndex i1) {
	return atr<Val>(i0, i1);
}
mxArrayWrapper::Val& mxArrayWrapper::operator() (mwIndex i0, mwIndex i1, mwIndex i2) {
	return atr<Val>(i0, i1, i2);
}

//mxArrayWrapper::Val mxArrayWrapper::getr(mwIndex i0, mwIndex i1) const { 
//	try{
//		vector<mwIndex> dims(2);
//		dims[0] = i0;
//		dims[1] = i1;
//		if (_idxErrChk(dims)){
//			throw exception("idx err");
//		}
//		
//		mwIndex idx = mxCalcSingleSubscript(getPtr(), dims.size(), dims.data());
//		void* cur = mxGetData(getPtr());
//		if (!cur){
//			throw exception("there is no value");
//		}
//		return static_cast<Val*>(cur)[idx];
//	}
//	catch (exception& e){
//		cout << "mxArrayWrapper::getr: " << e.what() << endl;
//		return Val(0);
//	}
//}
/////////////////////////////////////////////////////////// get info //////////////////////////////////////////////////
mxArray* mxArrayWrapper::getPtr(){
	return _ptr.get();
}
const mxArray* mxArrayWrapper::getPtr() const{
	return _ptr.get();
}
std::vector<mwSize> mxArrayWrapper::getDims() const{
	return _dims;
}
mwSize mxArrayWrapper::getNDims() const{
	return _ndims;
}
uint64 mxArrayWrapper::getNElems() const{
	return _nelems;
}
mxClassID mxArrayWrapper::getClassId() const{
	return _classId;
}
string mxArrayWrapper::getClassIdStr() const{
	switch (getClassId()){
	case mxUNKNOWN_CLASS:
		return "unknown";
	case mxCELL_CLASS:
		return "cell";
	case mxSTRUCT_CLASS:
		return "struct";
	case mxLOGICAL_CLASS:
		return "logical";
	case mxCHAR_CLASS:
		return "char";
	case mxVOID_CLASS:
		return "void";
	case mxDOUBLE_CLASS:
		return "double";
	case mxSINGLE_CLASS:
		return "single";
	case mxINT8_CLASS:
		return "int8";
	case mxUINT8_CLASS:
		return "uint8";
	case mxINT16_CLASS:
		return "int16";
	case mxUINT16_CLASS:
		return "uint16";
	case mxINT32_CLASS:
		return "int32";
	case mxUINT32_CLASS:
		return "uint32";
	case mxINT64_CLASS:
		return "int64";
	case mxUINT64_CLASS:
		return "uint64";
	case mxFUNCTION_CLASS:
		return "function";
	default:
		return "unknown(not a MATLAB ClassID)";
	}
}
mxComplexity mxArrayWrapper::getComplexity() const{
	return _complexity;
}
string mxArrayWrapper::getComplexityStr() const{
	switch (getComplexity()){
	case mxREAL:
		return "real";
	case mxCOMPLEX:
		return "complex";
	default:
		return "unknown(not a MATLAB Complexity)";
	}
}
bool mxArrayWrapper::isNull() const {
	return _ptr == nullptr;
}
bool mxArrayWrapper::isSparse() const {
	return mxIsSparse(getPtr());
}
bool mxArrayWrapper::isSparseValid() const {
	return getClassId() == mxDOUBLE_CLASS;
}
/////////////////////////////////////////////////////////// convenient get info ////////////////////////////////////
mwSize mxArrayWrapper::size() const {
	return getNElems();
}
mwSize mxArrayWrapper::size(int n) const{
	try{
		if (n >= getDims().size()){
			throw exception("out of idx err");
		}
		return getDims()[n];
	}
	catch (exception& e){
		return 0;
	}
}
/////////////////////////////////////////////////////////// clone //////////////////////////////////////////////////
mxArrayWrapper& mxArrayWrapper::copy(const mxArrayWrapper& o) {
	cout << "mxArrayWrapper::copy: not yet implemented" << endl;
	throw exception("mxArrayWrapper::copy: not yet implemented");
	//_constr(o._dims, o._classId, o._complexity, o.name);
	//for (mwIndex i = 0; i < _nelems; i++){
	//	setr(i) = o.getr(i);
	//	if (getComplexity() == mxCOMPLEX){
	//		seti(i) = o.geti(i);
	//	}
	//}
	return *this;
}
mxArrayWrapper mxArrayWrapper::clone() const{
	mxArrayWrapper o;
	o.copy(*this);
	return o;
}

/////////////////////////////////////////////////////////// cout //////////////////////////////////////////////////
std::ostream& operator << (std::ostream& o, const mxArrayWrapper& m){
	
	//typedef double V;
#define __code0(V)\
	try{\
		if (m.getNDims() > 3){\
			stringstream ss;\
			ss << "dimmension of [" << m.name << "] is too high (ndims=" << m.getNDims() << ")";\
			throw exception(ss.str().c_str());\
		}\
		o << "[varName=" << m.name << ", dims=" << m.getDims() << "]:" << endl;\
\
		if (m.getComplexity() == mxREAL){\
			auto& setw0 = std::setw(8);\
			auto& opt0 = std::left;\
			if (m.getNDims() == 0){\
				o << "[empty]" << endl;\
			}\
			else if (m.getNDims() == 1){\
				mwSize n0 = m.getDims()[0];\
				for (int i = 0; i < n0; i++){\
					o << setw0 << opt0 << m.atr<V>(i) << endl;\
				}\
			}\
			else if (m.getNDims() == 2){\
				mwSize n0 = m.getDims()[0];\
				mwSize n1 = m.getDims()[1];\
				for (int i = 0; i < n0; i++){\
					for (int j = 0; j < n1; j++){\
						o << setw0 << opt0 << m.atr<V>(i, j);\
					}\
					o << endl;\
				}\
			}\
			else if (m.getNDims() == 3){\
				mwSize n0 = m.getDims()[0];\
				mwSize n1 = m.getDims()[1];\
				mwSize n2 = m.getDims()[2];\
				for (int k = 0; k < n2; k++){\
					o << "## [" << m.name << "]: k=" << k << endl;\
					for (int i = 0; i < n0; i++){\
						for (int j = 0; j < n1; j++){\
							o << setw0 << opt0 << m.atr<V>(i, j, k);\
						}\
						o << endl;\
					}\
					o << endl;\
				}\
			}\
		}\
		else if(m.getComplexity() == mxCOMPLEX){\
			auto& setw0 = std::setw(8);\
			auto& opt0 = std::left;\
			if (m.getNDims() == 0){\
				o << "[]" << endl;\
			}\
			else if (m.getNDims() == 1){\
				mwSize n0 = m.getDims()[0];\
				for (int i = 0; i < n0; i++){\
					o << setw0 << opt0 << m.atr<V>(i) << " + i" << setw0 << opt0 << m.ati<V>(i) << endl;\
				}\
			}\
			else if (m.getNDims() == 2){\
				mwSize n0 = m.getDims()[0];\
				mwSize n1 = m.getDims()[1];\
				for (int i = 0; i < n0; i++){\
					for (int j = 0; j < n1; j++){\
						o << setw0 << opt0 << m.atr<V>(i, j) << " + i" << setw0 << opt0 << m.ati<V>(i, j);\
					}\
					o << endl;\
				}\
			}\
			else if (m.getNDims() == 3){\
				mwSize n0 = m.getDims()[0];\
				mwSize n1 = m.getDims()[1];\
				mwSize n2 = m.getDims()[2];\
				for (int k = 0; k < n2; k++){\
					o << "## [" << m.name << "]: k=" << k << endl;\
					for (int i = 0; i < n0; i++){\
						for (int j = 0; j < n1; j++){\
							o << setw0 << opt0 << m.atr<V>(i, j, k) << " + i" << setw0 << opt0 << m.ati<V>(i, j, k);\
						}\
						o << endl;\
					}\
					o << endl;\
				}\
			}\
		}\
		else{\
			throw exception("complexity is not mxREAL or mxCOMPLEX");\
		}\
		return o;\
	}\
	catch (exception& e){\
		cout << "[varName = " << m.name << "] " << "cout operator << (mxArrayWrapper) : " << e.what() << endl;\
		return o;\
	}

	__switch(m.getClassId(), __code0); // use macro template

}
