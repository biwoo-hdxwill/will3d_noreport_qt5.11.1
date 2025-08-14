#pragma once
#ifndef __KnnWrapper_H__
#define __KnnWrapper_H__

#include "../Core/util_global.h"
#include <glm/glm.hpp>
#include <vector>
#include <opencv/cv.h>
#include <opencv2/flann.hpp>
#include <memory>





class UTIL_EXPORT KnnWrapper{
protected:
	cv::Mat matPoints;
	std::shared_ptr<cv::flann::Index> kdtree;
public:
	KnnWrapper();
	//KnnWrapper(KnnWrapper& o);
	//KnnWrapper(KnnWrapper&& o);
	//KnnWrapper& operator= (KnnWrapper& o);
	//KnnWrapper& operator= (KnnWrapper&& o);
	//~KnnWrapper();
	KnnWrapper(const std::vector< glm::vec3 >& points);

	void knnSearch(
		std::vector<std::vector<int>>& knearIdxs,
		std::vector<std::vector<float>>& dists,
		const std::vector<glm::vec3>& queryPoints,
		int k
		) const;
	void knnSearch(
		std::vector<std::vector<int>>& knearIdxs, 
		const std::vector<glm::vec3>& queryPoints, 
		int k
		) const;
	void knnSearch(
		std::vector<int>& knearIdxs, 
		const std::vector<glm::vec3>& queryPoints
		) const;

	void knnSearch(
		std::vector<int>& knearIdxs,
		std::vector<float>& dists,
		const glm::vec3& queryPoint,
		int k
		) const;
	void knnSearch(
		std::vector<int>& knearIdxs,
		const glm::vec3& queryPoint,
		int k
		) const;
	int knnSearch(
		const glm::vec3& query_point
		) const;

#if 1 // legacy
	void knnSearch(std::vector<int>& knear_idx, int query_idx, int k) const; //
	//void knnSearch(std::vector<int>& knear_idx, const glm::vec3& query_point, int k) const;
	//void knnSearch(std::vector<int>& knear_idx, std::vector<float>& dist, const glm::vec3& query_point, int k) const; //
#endif
};


#endif 
