//#pragma once
//#include <Util/Core/util_global.h>
//#include <functional>
//#include <memory>
//#include <matrix.h>
////class mxArray; 
//class UTIL_EXPORT mxArrayWrapper{
//	std::shared_ptr<mxArray> _data;
//	static std::function<void(mxArray*)> deleter;
//	void _init();
//public:
//	mxArrayWrapper();
//	mxArrayWrapper(mxArray* data); // with deleter
//	bool isEmpty() const;
//	// [caution]
//	// data will not be spreaded. mxArray* 하나로 여러 mxArrayWrapper에 assign하면 이중 delete로 인해서 메모리 오류나게 될 것임.
//	const std::shared_ptr<mxArray>& assign(mxArray* data);
//	// data will be shared
//	const std::shared_ptr<mxArray>& assign(const std::shared_ptr<mxArray>& data);
//	// data will be shared
//	const mxArrayWrapper& assign(const mxArrayWrapper& var);
//	mxArrayWrapper& operator = (mxArrayWrapper& var);
//	const mxArrayWrapper& operator = (const mxArrayWrapper& var);
//	std::shared_ptr<mxArray>& getSharedPtr();
//	const std::shared_ptr<mxArray>& getSharedPtr() const;
//	mxArray* getRawPtr();
//	const mxArray* getRawPtr() const;
//	const double& operator[](int i) const;
//	double& operator[](int i);
//};
