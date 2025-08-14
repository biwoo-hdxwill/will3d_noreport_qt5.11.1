//#pragma message("## MatlabEngine/MatlabUtil.cpp visited")
//#include "MatlabUtil.h"
//#include <opencv2/opencv.hpp>
//using namespace std;
//
//
//template<class V>
//bool MatlabUtil::copy(mxArrayWrapper& mat, const std::vector<V>& data){
//	try{
//		if (data.size() != mat.size()){
//			stringstream ss;
//			ss << "data.size(=" << data.size() << ") != " << "mxArrayWrapper.size(=" << mat.size() << ")";
//			throw exception(ss.str().c_str());
//		}
//		void* cur = mxGetData(mat.getPtr());
//		Val* dataPtr = static_cast<Val*>(cur);
//		for (int i = 0; i < data.size(); i++){
//			dataPtr[i] = static_cast<Val>(data[i]);
//		}
//		return true;
//	}
//	catch (exception& e){
//		cout << "[varName=" << mat.name << "]" << "MatlabUtil::copy: " << e.what() << endl;
//		return false;
//	}
//}
//template<class V>
//bool MatlabUtil::copy(mxArrayWrapper& mat, const V* data, mwSize dataSize){
//	typedef mxArrayWrapper::Val Val;
//	try{
//		if (dataSize != mat.size()){
//			stringstream ss;
//			ss << "data.size(=" << dataSize << ") != " << "mxArrayWrapper.size(=" << mat.size() << ")";
//			throw exception(ss.str().c_str());
//		}
//		void* cur = mxGetData(mat.getPtr());
//		Val* dataPtr = static_cast<Val*>(cur);
//		for (int i = 0; i < dataSize; i++){
//			dataPtr[i] = static_cast<Val>(data[i]);
//		}
//		return true;
//	}
//	catch (exception& e){
//		cout << "[varName=" << mat.name << "]" << "MatlabUtil::copy: " << e.what() << endl;
//		return false;
//	}
//}
//template<class V>
//bool MatlabUtil::copy(mxArrayWrapper& mat, const cv::Mat& matcv){
//	typedef mxArrayWrapper::Val Val;
//	try{
//		if (mat.size() != matcv.total()){
//			stringstream ss;
//			ss << "matcv.size(=" << matcv.total() << ") != " << "mxArrayWrapper.size(=" << mat.size() << ")";
//			throw exception(ss.str().c_str());
//		}
//		void* cur = mxGetData(mat.getPtr());
//		Val* dataPtr = static_cast<Val*>(cur);
//		for (int i = 0; i < mat.size(); i++){
//			dataPtr[i] = static_cast<Val>(data[i]);
//		}
//		return true;
//	}
//	catch (exception& e){
//		cout << "[varName=" << mat.name << "]" << "MatlabUtil::copy: " << e.what() << endl;
//		return false;
//	}
//}
//template<class V>
//bool MatlabUtil::copy(std::vector<V>& data, const mxArrayWrapper& mat){
//	return true;
//}
//template<class V>
//bool MatlabUtil::copy(cv::Mat& matcv, const mxArrayWrapper& mat){
//	return true;
//}
