#include "KnnWrapper.h"
using namespace std;
using namespace cv;

KnnWrapper::KnnWrapper() {
	kdtree = nullptr;
}

KnnWrapper::KnnWrapper(const std::vector< glm::vec3 >& points) {
	try {
		if (points.empty()) {
			throw std::runtime_error("points is empty");
		}
		matPoints = Mat(points.size(), 3, CV_32F);
		for (int i = 0; i < points.size(); i++) {
			for (int j = 0; j < 3; j++) {
				matPoints.at<float>(i, j) = points[i][j];
			}
		}
		// kdtree with random trees
		cv::flann::KDTreeIndexParams idxParams(5);

		// create idx
		kdtree = shared_ptr<cv::flann::Index>(new cv::flann::Index(matPoints, idxParams)
			, [](cv::flann::Index* ptr) { /* dont delete intentionally */ });
		// 이러면 메모리 leak이 생기지만 이렇게 안하면 cv내부적인 에러로 인해서 delete시에 에러가남
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::knnSearch: " << e.what() << endl;
	}
}

bool KnnWrapper::knnSearch(
	std::vector<std::vector<int>>& knearIdxs,
	std::vector<std::vector<float>>& dists,
	const std::vector<glm::vec3>& queryPoints,
	int k
) const {
	try {
		if (!kdtree) {
			throw std::runtime_error("not initialized yet");
		}
		int nPoints = queryPoints.size();
		Mat matQuerys(queryPoints.size(), 3, CV_32F);
		for (int i = 0; i < queryPoints.size(); i++) {
			for (int j = 0; j < 3; j++) {
				matQuerys.at<float>(i, j) = queryPoints[i][j];
			}
		}
		Mat matIdxs;
		Mat matDists;
		kdtree->knnSearch(matQuerys, matIdxs, matDists, k, cv::flann::SearchParams(64));
		knearIdxs.resize(nPoints, vector<int>(k));
		dists.resize(nPoints, vector<float>(k));
		for (int i = 0; i < nPoints; i++) {
			for (int j = 0; j < k; j++) {
				knearIdxs[i][j] = matIdxs.at<int>(i, j);
				dists[i][j] = matDists.at<float>(i, j);
			}
		}

		return true;
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::knnSearch: " << e.what() << endl;
		return false;
	}
}

bool KnnWrapper::knnSearch(
	std::vector<std::vector<int>>& knearIdxs,
	const std::vector<glm::vec3>& queryPoints,
	int k
) const {
	vector<vector<float>> dists;
	return knnSearch(knearIdxs, dists, queryPoints, k);
}

bool KnnWrapper::knnSearch(
	std::vector<int>& knearIdxs,
	std::vector<float>& dists,
	const std::vector<glm::vec3>& queryPoints
) const {
	try {
		if (!kdtree) {
			throw std::runtime_error("not initialized yet");
		}
		int nPoints = queryPoints.size();
		Mat matQuerys(queryPoints.size(), 3, CV_32F);
		for (int i = 0; i < queryPoints.size(); i++) {
			for (int j = 0; j < 3; j++) {
				matQuerys.at<float>(i, j) = queryPoints[i][j];
			}
		}
		Mat matIdxs;
		Mat matDists;
		kdtree->knnSearch(matQuerys, matIdxs, matDists, 1, cv::flann::SearchParams(64));
		knearIdxs.resize(nPoints);
		dists.resize(nPoints);
		for (int i = 0; i < nPoints; i++) {
			knearIdxs[i] = matIdxs.at<int>(i, 0);
			dists[i] = matDists.at<float>(i, 0);
		}

		return true;
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::knnSearch: " << e.what() << endl;
		return false;
	}
}

bool KnnWrapper::knnSearch(
	std::vector<int>& knearIdxs,
	const std::vector<glm::vec3>& queryPoints
) const {
	vector<float> dists;
	return knnSearch(knearIdxs, dists, queryPoints);
}

bool KnnWrapper::knnSearch(
	std::vector<int>& knearIdxs,
	std::vector<float>& dists,
	const glm::vec3& queryPoint,
	int k
) const {
	try {
		if (!kdtree) {
			throw std::runtime_error("not initialized yet");
		}
		Mat matQuerys(1, 3, CV_32F);
		matQuerys.at<float>(0, 0) = queryPoint[0];
		matQuerys.at<float>(0, 1) = queryPoint[1];
		matQuerys.at<float>(0, 2) = queryPoint[2];
		Mat matIdxs;
		Mat matDists;
		kdtree->knnSearch(matQuerys, matIdxs, matDists, k, cv::flann::SearchParams(64));
		knearIdxs.resize(k);
		dists.resize(k);
		for (int j = 0; j < k; j++) {
			knearIdxs[j] = matIdxs.at<int>(0, j);
			dists[j] = matDists.at<float>(0, j);
		}
		return true;
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::knnSearch: " << e.what() << endl;
		return false;
	}
}
bool KnnWrapper::knnSearch(
	std::vector<int>& knearIdxs,
	const glm::vec3& queryPoint,
	int k
) const {
	vector<float> dists;
	return knnSearch(knearIdxs, dists, queryPoint, k);
}
int KnnWrapper::knnSearch(
	const glm::vec3& queryPoint
) const {
	try {
		if (!kdtree) {
			throw std::runtime_error("not initialized yet");
		}
		Mat matQuerys(1, 3, CV_32F);
		matQuerys.at<float>(0, 0) = queryPoint[0];
		matQuerys.at<float>(0, 1) = queryPoint[1];
		matQuerys.at<float>(0, 2) = queryPoint[2];
		Mat matIdxs;
		Mat matDists;
		kdtree->knnSearch(matQuerys, matIdxs, matDists, 1, cv::flann::SearchParams(64));
		return matIdxs.at<int>(0, 0);
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::knnSearch: " << e.what() << endl;
	}
}

#if 0

KnnWrapper::KnnWrapper(const std::vector< glm::vec3 >& points) {
	try {
		if (points.empty()) {
			throw std::runtime_error("points.size=0");
		}
		matPoints = cv::Mat(points.size(), 3, CV_32F);
		for (int i = 0; i < points.size(); i++) {
			for (int j = 0; j < 3; j++) {
				matPoints.at<float>(i, j) = points[i][j];
			}
		}
		// kdtree with random trees
		cv::flann::KDTreeIndexParams indexParams(5);

		// create index
		kdtree = shared_ptr<cv::flann::Index>(new cv::flann::Index(matPoints, indexParams)
			, [](cv::flann::Index* ptr) { /* dont delete intentionally */ });
		// 이러면 메모리 leak이 생기지만 이렇게 안하면 cv내부적인 에러로 인해서 delete시에 에러가남
	} catch (std::runtime_error& e) {
		cout << "KnnWrapper::KnnWrapper: " << e.what() << endl;
	}
}

// idx_ret[i]	:	index vector of points which represents k nearest to queries[i]
//void KnnWrapper::knnSearch(std::vector< std::vector<int> >& idx_ret, const std::vector< glm::vec3 >& queries, int k){
//
//	Mat matQueries(queries.size(), 3, CV_32F);
//
//	for(int i = 0 ; i < queries.size() ; i ++){
//		for(int j = 0 ; j < 3 ; j ++){
//			matQueries.at<float>(i,j) = queries[i][j];
//		}
//	}
//
//	// kdtree with 5 random trees
//	cv::flann::KDTreeIndexParams indexParams(5);
//
//	// create index
//	cv::flann::Index kdtree(matPoints, indexParams);
//
//	// execute knn search
//	Mat indices;
//	Mat dists;
//	kdtree.knnSearch(matQueries, indices, dists, k, cv::flann::SearchParams(64));
//
//	// return idx
//	idx_ret.clear();
//	idx_ret.reserve(indices.rows);
//	for(int i = 0 ; i < indices.rows ; i ++){
//		std::vector<int> v;
//		for(int j = 0 ; j < indices.cols ; j ++){
//			v.push_back(indices.at<int>(i,j));
//		}
//		idx_ret.push_back(v);
//	}
//}

//void KnnWrapper::knnSearch(std::vector<int>& idx_ret, const glm::vec3& query, int k, bool isMember){
//	Mat matQueries(1, 3, CV_32F);
//
//	for(int i = 0 ; i < 1 ; i ++){
//		for(int j = 0 ; j < 3 ; j ++){
//			matQueries.at<float>(i,j) = query[j];
//		}
//	}
//
//	// kdtree with 5 random trees
//	cv::flann::KDTreeIndexParams indexParams(5);
//
//	// create index
//	cv::flann::Index kdtree(matPoints, indexParams);
//
//	// execute knn search
//	Mat indices;
//	Mat dists;
//	if(!isMember){
//		kdtree.knnSearch(matQueries, indices, dists, k, cv::flann::SearchParams(64));
//		// return idx
//		idx_ret.clear();
//		idx_ret.reserve(indices.cols);
//		for(int j = 0 ; j < indices.cols ; j ++){
//			idx_ret.push_back(indices.at<int>(0,j));
//		}
//	}
//	else{
//		kdtree.knnSearch(matQueries, indices, dists, k+1, cv::flann::SearchParams(64));
//		// return idx
//		idx_ret.clear();
//		idx_ret.reserve(indices.cols);
//		for(int j = 0 ; j < indices.cols ; j ++){
//			int idx = indices.at<int>(0,j);
//			glm::vec3 temp = glm::vec3(matPoints.at<float>(idx,0), matPoints.at<float>(idx,1), matPoints.at<float>(idx,2));
//
//			if(query != temp){
//				//cout << query[0] << " " << query[1] << " " << query[2] << " / " << temp[0] << " " << temp[1] << " " << temp[2] << endl;
//				idx_ret.push_back(idx);
//			}
//		}
//	}
//}

int KnnWrapper::knnSearch(const glm::vec3& query_point) const {
	Mat matQuery(1, 3, CV_32F);
	matQuery.at<float>(0, 0) = query_point[0];
	matQuery.at<float>(0, 1) = query_point[1];
	matQuery.at<float>(0, 2) = query_point[2];
	Mat idx;
	Mat dist;

	if (kdtree) {
		kdtree->knnSearch(matQuery, idx, dist, 1, cv::flann::SearchParams(64));
	} else {
		cout << "KnnSearch kdtree nullptr err" << endl;
	}
	return idx.at<int>(0, 0);
}

void KnnWrapper::knnSearch(vector<int>& knear_idx, const glm::vec3& query_point, int k) const {
	Mat matQuery(1, 3, CV_32F);
	matQuery.at<float>(0, 0) = query_point[0];
	matQuery.at<float>(0, 1) = query_point[1];
	matQuery.at<float>(0, 2) = query_point[2];
	Mat indices;
	Mat dists;

	// kdtree with 5 random trees
	//cv::flann::KDTreeIndexParams indexParams(5);

	// create index
	//cv::flann::Index kdtree(matPoints, indexParams);

	if (kdtree) {
		kdtree->knnSearch(matQuery, indices, dists, k, cv::flann::SearchParams(64));
	} else {
		cout << "KnnSearch kdtree nullptr err" << endl;
	}
	knear_idx.clear();
	knear_idx.reserve(k);
	for (int j = 0; j < indices.cols; j++) {
		int idx = indices.at<int>(0, j);
		knear_idx.push_back(idx);
	}
}

void KnnWrapper::knnSearch(vector<int>& knear_idx, vector<float>& dist, const glm::vec3& query_point, int k) const {
	Mat matQuery(1, 3, CV_32F);
	matQuery.at<float>(0, 0) = query_point[0];
	matQuery.at<float>(0, 1) = query_point[1];
	matQuery.at<float>(0, 2) = query_point[2];
	Mat indices;
	Mat dists;

	// kdtree with 5 random trees
	//cv::flann::KDTreeIndexParams indexParams(5);

	// create index
	//cv::flann::Index kdtree(matPoints, indexParams);

	if (kdtree) {
		kdtree->knnSearch(matQuery, indices, dists, k, cv::flann::SearchParams(64));
	} else {
		cout << "KnnSearch kdtree nullptr err" << endl;
	}
	knear_idx.clear();
	knear_idx.reserve(k);
	for (int j = 0; j < indices.cols; j++) {
		int idx = indices.at<int>(0, j);
		knear_idx.push_back(idx);
	}
	dist.clear();
	dist.reserve(k);
	for (int j = 0; j < dists.cols; j++) {
		dist.push_back(dists.at<float>(0, j));
	}
}

#endif

#if 1 // legacy
void KnnWrapper::knnSearch(std::vector<int>& knear_idx, int query_idx, int k) const {
	Mat matQuery = matPoints.row(query_idx);
	Mat indices;
	Mat dists;

	// kdtree with 5 random trees
	//cv::flann::KDTreeIndexParams indexParams(5);

	// create index
	//cv::flann::Index kdtree(matPoints, indexParams);

	if (kdtree) {
		kdtree->knnSearch(matQuery, indices, dists, k + 1, cv::flann::SearchParams(64));
	} else {
		cout << "KnnSearch kdtree nullptr err" << endl;
	}
	knear_idx.clear();
	knear_idx.reserve(k);
	for (int j = 0; j < indices.cols; j++) {
		int idx = indices.at<int>(0, j);
		if (query_idx != idx) {
			knear_idx.push_back(idx);
		}
	}
}
#endif
