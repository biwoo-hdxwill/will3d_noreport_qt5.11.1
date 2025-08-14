// 150923
// latest
#pragma once
#ifndef __XFormMaker_H__
#define __XFormMaker_H__
#include "../Core/util_global.h"
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif
#include <vector>
#include <functional>
using namespace std;
class UTIL_EXPORT XFormMaker {
	//static bool _isValid(float val);
	//static bool _isValid(double val);
	//static bool _isValid(int val);
	//static bool _isValid(const glm::vec3& p);
private:
	static void _getMinMaxLengthAndCenter(float& length, glm::vec3& center, const vector<glm::vec3>& points);
public:

	// get center of points and maximum diameter of points  among x, y, z axis
	static void getMinMaxLengthAndCenter(float& length, glm::vec3& center, const vector<glm::vec3>& points);

	// get min max normalization xform of points
	static glm::mat4 getMinMaxNormalize(const vector<glm::vec3>& points);

	// multiply xform to vec3 points
	static void execute(vector<glm::vec3>& points, const glm::mat4& xform);

	// multiply xform to vec2 points
	static void execute(vector<glm::vec2>& points, const glm::mat4& xform);

	// execute(points, criterion)
	// remove points which dont satisfy criterion
	// criterion(vec3) == 1 --> included,
	// criterion(vec3) == 0 --> removed
	static void execute(std::vector<glm::vec3>& points, const std::function<bool(const glm::vec3&)>& criterion);

	// remove points and triangles (mesh) which dont satisfy criterion
	static void execute(std::vector<glm::vec3>& points, std::vector<std::vector<int>>& triangles, const std::function<bool(const glm::vec3&)>& criterion);

	// get idxs of points satisfying criterion
	static void find(
		std::vector<int>& idxs,
		const std::vector<glm::vec3>& points,
		const std::function<bool(const glm::vec3)>& criterion
	);

	// convert vec3 points into vec2 points
	static void convert(vector<glm::vec2>& dst, const vector<glm::vec3>& src);

	/// convert vec2 points into vec3 points
	static void convert(vector<glm::vec3>& dst, const vector<glm::vec2>& src);

	// translate vec3 points in [0,1]^3 into [-0.5 0.5]^3
	static void textureToCenteredTexture(vector<glm::vec3>& points);

	// translate vec2 points in [0,1]^2 into [-0.5 0.5]^2
	static void textureToCenteredTexture(vector<glm::vec2>& points);

	// translate vec3 points in [-0.5 0.5]^3 into [0 1]^3
	static void centeredTextureToTexture(vector<glm::vec3>& points);

	// translate vec2 points in [-0.5 0.5]^2 into [0 1]^2
	static void centeredTextureToTexture(vector<glm::vec2>& points);

	// make centered texture ingredient. points in [-0.5 0.5]^3
	static void getCenteredTexture(vector<glm::vec3>& points, vector<vector<int>>& triangles, vector<glm::vec2>& textures);

	//static glm::mat4 getWeightCenterNormalize(const vector<glm::vec3>& points);

	//static bool isVaildData(const vector<glm::vec3>& points);
	//static vector<glm::vec3> getValidData(const vector<glm::vec3>& points);
};

#endif // __XFormMaker_H__
