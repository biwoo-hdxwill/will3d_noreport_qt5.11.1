#pragma once
#include "../Core/util_global.h"
#include <Eigen/src/Core/Matrix.h>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <iostream>
#include "../Core/typeDef.h"

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
	mxINT32_CLASS,
	mxUINT32_CLASS,
	mxINT64_CLASS,
	mxUINT64_CLASS,
	mxFUNCTION_CLASS
*/


class UTIL_EXPORT mxArrayWrapper{
protected:
	std::function<void(mxArray*)> _deleter;
	std::shared_ptr<mxArray> _ptr;
	std::vector<mwSize> _dims;	
	mwSize _ndims;
	uint64 _nelems;
	mxClassID _classId;
	mxComplexity _complexity;

	// for sparse matrix
	mwSize _nzmax;
	//double _percent_sparse;
protected:
	

	void _init();

	// for sparse matrix
	void _constrSparse(
		mwSize m, 
		mwSize n,
		mxComplexity complexity,
		const std::string& name
		);

	void _constr(
		const std::vector<mwSize>& dims, 
		mxClassID classId, 
		mxComplexity complexity,
		const std::string& name
		);
	void _constr(
		mxArray* ptr, 
		const std::string& name
		);
	bool _idxErrChk(const std::vector<mwIndex>& idxs) const;
public:
	typedef double Val; // default return type

	std::string name;

// create & delete
	void clear();
	~mxArrayWrapper();
	mxArrayWrapper();

	// make from mxArray*
	//mxArrayWrapper(
	//	mxArray* ptr,
	//	const std::string& name
	//	);
	mxArrayWrapper(
		mxArray* ptr,
		const std::string& name
		);

	//mxArrayWrapper(
	//	const std::vector<mwSize>& dims,
	//	mxClassID classId,
	//	mxComplexity complexity
	//	);

	// make dense mat
	//mxArrayWrapper(
	//	const std::vector<mwSize>& dims,
	//	mxClassID classId,
	//	mxComplexity complexity,
	//	const std::string& name
	//	);
	mxArrayWrapper(
		const std::vector<mwSize>& dims,
		mxClassID classId, 
		mxComplexity complexity, 
		const std::string& name
		);

	// make sparse 2d mat
	//mxArrayWrapper(
	//	mwSize m,
	//	mwSize n,
	//	mxComplexity complexity,
	//	const std::string& name
	//	);
	mxArrayWrapper(
		mwSize m,
		mwSize n,
		mxComplexity complexity,
		const std::string& name
		);

///////////////////////////////////////////////////////////// IO //////////////////////////////////////////////////
	template<class V>
	bool copy(const std::vector<V>& data);
	template<class V>
	bool copy(const V* data, mwSize dataSize);
	//bool copy(const double* data, const vector<mwSize> dims);
	//bool copy(const double* data, const vector<mwSize> dims);
///////////////////////////////////////////////////////////// access //////////////////////////////////////////////////
	template<class V>
	V atr(const std::vector<mwIndex>& idxs) const;
	template<class V>
	V atr(mwIndex i0) const;
	template<class V>
	V atr(mwIndex i0, mwIndex i1) const;
	template<class V>
	V atr(mwIndex i0, mwIndex i1, mwIndex i2) const;

	template<class V>
	V ati(const std::vector<mwIndex>& idxs) const;
	template<class V>
	V ati(mwIndex i0) const;
	template<class V>
	V ati(mwIndex i0, mwIndex i1) const;
	template<class V>
	V ati(mwIndex i0, mwIndex i1, mwIndex i2) const;

	template<class V>
	V& atr(const std::vector<mwIndex>& idxs);
	template<class V>
	V& atr(mwIndex i0);
	template<class V>
	V& atr(mwIndex i0, mwIndex i1);
	template<class V>
	V& atr(mwIndex i0, mwIndex i1, mwIndex i2);

	template<class V>
	V& ati(const std::vector<mwIndex>& idxs);
	template<class V>
	V& ati(mwIndex i0);
	template<class V>
	V& ati(mwIndex i0, mwIndex i1);
	template<class V>
	V& ati(mwIndex i0, mwIndex i1, mwIndex i2);
	
// raw data ptr
	void* datar();
	void* datai();

// access convenivent for real double
	Val operator() (const std::vector<mwIndex>& idxs) const;
	Val operator() (mwIndex i0) const;
	Val operator() (mwIndex i0, mwIndex i1) const;
	Val operator() (mwIndex i0, mwIndex i1, mwIndex i2) const;

	Val& operator() (const std::vector<mwIndex>& idxs);
	Val& operator() (mwIndex i0);
	Val& operator() (mwIndex i0, mwIndex i1);
	Val& operator() (mwIndex i0, mwIndex i1, mwIndex i2);
	
// get info
	mxArray* getPtr();
	const mxArray* getPtr() const;
	std::vector<mwSize> getDims() const;
	mwSize getNDims() const;
	uint64 getNElems() const;
	mxClassID getClassId() const;
	std::string getClassIdStr() const;
	mxComplexity getComplexity() const;
	std::string getComplexityStr() const;
	bool isNull() const;
	bool isSparse() const;
	bool isSparseValid() const;

// convenient get info
	uint64 size() const;
	mwSize size(int n) const;

// clone
	/*
		default operator = : shallow copy (_ptr sharing)
	*/
	mxArrayWrapper& copy(const mxArrayWrapper& o); // deep copy data
	mxArrayWrapper clone() const; // deep copy (_ptr recreating with MATLAB API)
	
};

//
//template <class Val>
//class UTIL_EXPORT mxArrayWrapperAccess : public mxArrayWrapper{
//public:
//	virtual Val getr(const std::vector<mwIndex>& idxs) const;
//	virtual Val getr(mwIndex i0) const;
//	virtual Val getr(mwIndex i0, mwIndex i1) const;
//	virtual Val getr(mwIndex i0, mwIndex i1, mwIndex i2) const;
//	
//	virtual Val geti(const std::vector<mwIndex>& idxs) const;
//	virtual Val geti(mwIndex i0) const;
//	virtual Val geti(mwIndex i0, mwIndex i1) const;
//	virtual Val geti(mwIndex i0, mwIndex i1, mwIndex i2) const;
//	
//	virtual Val& setr(const std::vector<mwIndex>& idxs);
//	virtual Val& setr(mwIndex i0);
//	virtual Val& setr(mwIndex i0, mwIndex i1);
//	virtual Val& setr(mwIndex i0, mwIndex i1, mwIndex i2);
//	
//	virtual Val& seti(const std::vector<mwIndex>& idxs);
//	virtual Val& seti(mwIndex i0);
//	virtual Val& seti(mwIndex i0, mwIndex i1);
//	virtual Val& seti(mwIndex i0, mwIndex i1, mwIndex i2);
//		
//	
//// access convenivent for real
//	virtual Val operator() (const std::vector<mwIndex>& idxs) const;
//	virtual Val operator() (mwIndex i0) const;
//	virtual Val operator() (mwIndex i0, mwIndex i1) const;
//	virtual Val operator() (mwIndex i0, mwIndex i1, mwIndex i2) const;
//	
//	virtual Val& operator() (const std::vector<mwIndex>& idxs);
//	virtual Val& operator() (mwIndex i0);
//	virtual Val& operator() (mwIndex i0, mwIndex i1);
//	virtual Val& operator() (mwIndex i0, mwIndex i1, mwIndex i2);
//};

UTIL_EXPORT std::ostream& operator << (std::ostream& o, const mxArrayWrapper& m);

// 여기서도 template <> 랑 template 은 분명 다름
//
//template UTIL_EXPORT bool mxArrayWrapper::copy<double>(const std::vector<double>& data);
//template UTIL_EXPORT bool mxArrayWrapper::copy<float>(const std::vector<float>& data);
//template UTIL_EXPORT bool mxArrayWrapper::copy<int>(const std::vector<int>& data);
//template UTIL_EXPORT bool mxArrayWrapper::copy<double>(const double* data, mwSize dataSize);
//template UTIL_EXPORT bool mxArrayWrapper::copy<float>(const float* data, mwSize dataSize);
//template UTIL_EXPORT bool mxArrayWrapper::copy<int>(const int* data, mwSize dataSize);

// define 이름이랑 ()이랑 떨어지면 컴파일 오류남.
#define __mxArrayWrapper_IO_macro_instantiation0__(V)\
template UTIL_EXPORT V mxArrayWrapper::atr<V>(const std::vector<mwIndex>& idxs) const; \
template UTIL_EXPORT V mxArrayWrapper::ati<V>(const std::vector<mwIndex>& idxs) const; \
template UTIL_EXPORT V& mxArrayWrapper::atr<V>(const std::vector<mwIndex>& idxs); \
template UTIL_EXPORT V& mxArrayWrapper::ati<V>(const std::vector<mwIndex>& idxs);

#define __mxArrayWrapper_IO_macro_instantiation1__(V)\
template UTIL_EXPORT V mxArrayWrapper::atr<V>(mwIndex i0) const; \
template UTIL_EXPORT V mxArrayWrapper::atr<V>(mwIndex i0, mwIndex i1) const; \
template UTIL_EXPORT V mxArrayWrapper::atr<V>(mwIndex i0, mwIndex i1, mwIndex i2) const; \
\
template UTIL_EXPORT V mxArrayWrapper::ati<V>(mwIndex i0) const; \
template UTIL_EXPORT V mxArrayWrapper::ati<V>(mwIndex i0, mwIndex i1) const; \
template UTIL_EXPORT V mxArrayWrapper::ati<V>(mwIndex i0, mwIndex i1, mwIndex i2) const; \
\
template UTIL_EXPORT V& mxArrayWrapper::atr<V>(mwIndex i0); \
template UTIL_EXPORT V& mxArrayWrapper::atr<V>(mwIndex i0, mwIndex i1); \
template UTIL_EXPORT V& mxArrayWrapper::atr<V>(mwIndex i0, mwIndex i1, mwIndex i2); \
\
template UTIL_EXPORT V& mxArrayWrapper::ati<V>(mwIndex i0); \
template UTIL_EXPORT V& mxArrayWrapper::ati<V>(mwIndex i0, mwIndex i1); \
template UTIL_EXPORT V& mxArrayWrapper::ati<V>(mwIndex i0, mwIndex i1, mwIndex i2); 

#define __mxArrayWrapper_IO_macro_specialize0__(V)\
template<> UTIL_EXPORT V mxArrayWrapper::atr<V>(const std::vector<mwIndex>& idxs) const; \
template<> UTIL_EXPORT V mxArrayWrapper::ati<V>(const std::vector<mwIndex>& idxs) const; \
template<> UTIL_EXPORT V& mxArrayWrapper::atr<V>(const std::vector<mwIndex>& idxs); \
template<> UTIL_EXPORT V& mxArrayWrapper::ati<V>(const std::vector<mwIndex>& idxs);

__mxArrayWrapper_IO_macro_specialize0__(double) // for space matrix IO
__mxArrayWrapper_IO_macro_instantiation0__(float)
__mxArrayWrapper_IO_macro_instantiation0__(int8)
__mxArrayWrapper_IO_macro_instantiation0__(int16)
__mxArrayWrapper_IO_macro_instantiation0__(int32)
__mxArrayWrapper_IO_macro_instantiation0__(int64)
__mxArrayWrapper_IO_macro_instantiation0__(uint8)
__mxArrayWrapper_IO_macro_instantiation0__(uint16)
__mxArrayWrapper_IO_macro_instantiation0__(uint32)
__mxArrayWrapper_IO_macro_instantiation0__(uint64)
__mxArrayWrapper_IO_macro_instantiation1__(double)
__mxArrayWrapper_IO_macro_instantiation1__(float)
__mxArrayWrapper_IO_macro_instantiation1__(int8)
__mxArrayWrapper_IO_macro_instantiation1__(int16)
__mxArrayWrapper_IO_macro_instantiation1__(int32)
__mxArrayWrapper_IO_macro_instantiation1__(int64)
__mxArrayWrapper_IO_macro_instantiation1__(uint8)
__mxArrayWrapper_IO_macro_instantiation1__(uint16)
__mxArrayWrapper_IO_macro_instantiation1__(uint32)
__mxArrayWrapper_IO_macro_instantiation1__(uint64)

#define __mxArrayWrapper_IO_macro_instantiation2__(V)\
template UTIL_EXPORT bool mxArrayWrapper::copy<V>(const std::vector<V>& data);\
template UTIL_EXPORT bool mxArrayWrapper::copy<V>(const V* data, mwSize dataSize);

__mxArrayWrapper_IO_macro_instantiation2__(double)
__mxArrayWrapper_IO_macro_instantiation2__(float)
__mxArrayWrapper_IO_macro_instantiation2__(int8)
__mxArrayWrapper_IO_macro_instantiation2__(int16)
__mxArrayWrapper_IO_macro_instantiation2__(int32)
__mxArrayWrapper_IO_macro_instantiation2__(int64)
__mxArrayWrapper_IO_macro_instantiation2__(uint8)
__mxArrayWrapper_IO_macro_instantiation2__(uint16)
__mxArrayWrapper_IO_macro_instantiation2__(uint32)
__mxArrayWrapper_IO_macro_instantiation2__(uint64)


// for multi value type macro template
#define __switch(id, code)\
switch(id){\
	case mxDOUBLE_CLASS:{\
		code(double);\
		break;\
	}\
	case mxSINGLE_CLASS:{\
		code(float);\
		break;\
	}\
	case mxINT8_CLASS:{\
		code(int8);\
		break;\
	}\
	case mxINT16_CLASS:{\
		code(int16);\
		break;\
	}\
	case mxINT32_CLASS:{\
		code(int32);\
		break;\
	}\
	case mxINT64_CLASS:{\
		code(int64);\
		break;\
	}\
	case mxUINT8_CLASS:{\
		code(uint8);\
		break;\
	}\
	case mxUINT16_CLASS:{\
		code(uint16);\
		break;\
	}\
	case mxUINT32_CLASS:{\
		code(uint32);\
		break;\
	}\
	case mxUINT64_CLASS:{\
		code(uint64);\
		break;\
	}\
	default:{\
		throw exception("__swtich macro unknown mxClassId");\
	}\
}
