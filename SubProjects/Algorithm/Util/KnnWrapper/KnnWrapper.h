#pragma once
#ifndef __KnnWrapper_H__
#define __KnnWrapper_H__

#include <vector>
#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <opencv/cv.h>
#include <opencv2/flann.hpp>

#include "../Core/util_global.h"

class UTIL_EXPORT KnnWrapper {
protected:
	cv::Mat matPoints;
	std::shared_ptr<cv::flann::Index> kdtree;
public:
	KnnWrapper();
	KnnWrapper(const std::vector< glm::vec3 >& points);

	bool knnSearch(
		std::vector<std::vector<int>>& knearIdxs,
		std::vector<std::vector<float>>& dists,
		const std::vector<glm::vec3>& queryPoints,
		int k
	) const;
	bool knnSearch(
		std::vector<std::vector<int>>& knearIdxs,
		const std::vector<glm::vec3>& queryPoints,
		int k
	) const;

	bool knnSearch(
		std::vector<int>& knearIdxs,
		std::vector<float>& dists,
		const std::vector<glm::vec3>& queryPoints
	) const;
	bool knnSearch(
		std::vector<int>& knearIdxs,
		const std::vector<glm::vec3>& queryPoints
	) const;

	bool knnSearch(
		std::vector<int>& knearIdxs,
		std::vector<float>& dists,
		const glm::vec3& queryPoint,
		int k
	) const;
	bool knnSearch(
		std::vector<int>& knearIdxs,
		const glm::vec3& queryPoint,
		int k
	) const;

	int knnSearch(
		const glm::vec3& queryPoint
	) const;

	void knnSearch(std::vector<int>& knear_idx, int query_idx, int k) const; //
	//void knnSearch(std::vector<int>& knear_idx, const glm::vec3& query_point, int k) const;
	//void knnSearch(std::vector<int>& knear_idx, std::vector<float>& dist, const glm::vec3& query_point, int k) const; //
};

#endif
