#include "XFormMaker.h"
#include <iostream>
#include <algorithm>
#if defined(__APPLE__)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#else
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtx/transform.hpp>
#endif
#include "VertexToTriIdxMapper.h"

void XFormMaker::_getMinMaxLengthAndCenter(float& length, glm::vec3& center, const vector<glm::vec3>& points) {
	if (points.empty()) {
		length = 0;
		center = glm::vec3(0, 0, 0);
		throw std::runtime_error("points is empty");
	}
	glm::vec3 minv = points[0];
	glm::vec3 maxv = points[0];
	for (const auto& p : points) {
		for (int i = 0; i < 3; i++) {
			if (p[i] < minv[i]) {
				minv[i] = p[i];
			}
			if (p[i] > maxv[i]) {
				maxv[i] = p[i];
			}
		}
	}
	glm::vec3 fovvec = maxv - minv;
	length = std::max(fovvec[0], std::max(fovvec[1], fovvec[2])); // 이게 맞음
	center = (maxv + minv) / 2.f;
}

void XFormMaker::getMinMaxLengthAndCenter(float& length, glm::vec3& center, const vector<glm::vec3>& points) {
	try {
		_getMinMaxLengthAndCenter(length, center, points);
	} catch (std::runtime_error& e) {
		cout << "XFormMaker::getMinMaxLengthAndCenter: " << e.what() << endl;
		length = 0;
		center = glm::vec3(0, 0, 0);
	}
}

glm::mat4 XFormMaker::getMinMaxNormalize(const vector<glm::vec3>& points) {
	try {
		//if(points.empty()){
		//	throw std::runtime_error("points is empty");
		//}
		//glm::vec3 minv = points[0];
		//glm::vec3 maxv = points[0];
		//for(const auto& p : points){
		//	for(int i = 0 ; i < 3 ; i ++){
		//		if(p[i] < minv[i]){
		//			minv[i] = p[i];
		//		}
		//		if(p[i] > maxv[i]){
		//			maxv[i] = p[i];
		//		}
		//	}
		//}

		//glm::vec3 fovvec = maxv - minv;
		//float length = std::max(fovvec[0], std::max(fovvec[1], fovvec[2])); // 이게 맞음
		//glm::vec3 center = (maxv+minv)/2.f;
		float length;
		glm::vec3 center;
		_getMinMaxLengthAndCenter(length, center, points);

		glm::mat4 xform_normalize = glm::scale(glm::vec3(1.0 / length, 1.0 / length, 1.0 / length))
			* glm::translate(glm::vec3(-center));

		return xform_normalize;
	} catch (std::runtime_error& e) {
		cout << "XFormMaker::getMinMaxNormalize: " << e.what() << endl;
		return glm::mat4(1.0);
	}
}

void XFormMaker::execute(vector<glm::vec3>& points, const glm::mat4& xform) {
	for (auto& p : points) {
		p = glm::vec3(xform * glm::vec4(p, 1.0));
	}
}

void XFormMaker::execute(vector<glm::vec2>& points, const glm::mat4& xform) {
	for (auto& p : points) {
		p = glm::vec2(xform * glm::vec4(p, 0.0, 1.0));
	}
}

void XFormMaker::execute(vector<glm::vec3>& points, const function<bool(const glm::vec3&)>& criterion) {
	//points.erase(std::remove_if(points.begin(), points.end(), criterion), points.end()); // this is reverse criterion case...
	vector<glm::vec3> pointsOut;
	pointsOut.reserve(points.size());
	for (const auto& p : points) {
		if (criterion(p)) {
			pointsOut.push_back(p);
		}
	}
	points = pointsOut;
}

void XFormMaker::execute(vector<glm::vec3>& points, vector<vector<int>>& triangles, const function<bool(const glm::vec3&)>& criterion) {
	VertexToTriIdxMapper vm(triangles);
	// make flag
	vector<unsigned char> f_points; f_points.reserve(points.size());
	vector<unsigned char> f_triangles(triangles.size(), 1);
	for (const auto& p : points) {
		f_points.push_back(criterion(p));
	}

	for (int i = 0; i < f_points.size(); i++) {
		auto f = f_points[i];
		if (!f) {
			for (auto it : vm.findTriIdxByVertexIdx(i)) {
				it;
				f_triangles[it] = 0;
			}
		}
	}

	// make remapping
	vector<int> remap_points(points.size(), -1);
	int cur = 0;
	for (int i = 0; i < remap_points.size(); i++) {
		if (f_points[i]) {
			remap_points[i] = cur++;
		} else {
			remap_points[i] = -1;
		}
	}

	vector<int> remap_triangles(triangles.size(), -1);
	cur = 0;
	for (int i = 0; i < remap_triangles.size(); i++) {
		if (f_triangles[i]) {
			remap_triangles[i] = cur++;
		} else {
			remap_triangles[i] = -1;
		}
	}

	// reduce points and triangles
	vector<glm::vec3> pointsOut(std::count(f_points.begin(), f_points.end(), 1));
	vector<vector<int>> trianglesOut(std::count(f_triangles.begin(), f_triangles.end(), 1));
	for (int i = 0; i < points.size(); i++) {
		if (remap_points[i] != -1) {
			pointsOut[remap_points[i]] = points[i];
		}
	}
	for (int i = 0; i < triangles.size(); i++) {
		if (remap_triangles[i] != -1) {
			auto& triOut = trianglesOut[remap_triangles[i]];
			const auto& tri = triangles[i];
			for (int j = 0; j < tri.size(); j++) {
				triOut.push_back(remap_points[tri[j]]);
			}
		}
	}

	// return
	points = pointsOut;
	triangles = trianglesOut;
}

void XFormMaker::find(std::vector<int>& idxs,
	const std::vector<glm::vec3>& points,
	const std::function<bool(const glm::vec3)>& criterion
) {
	idxs.clear();
	for (int i = 0; i < points.size(); i++) {
		const auto& p = points[i];
		if (criterion(p)) {
			idxs.push_back(i);
		}
	}
}

void XFormMaker::convert(vector<glm::vec2>& dst, const vector<glm::vec3>& src) {
	dst.clear();
	for (const auto& p : src) {
		dst.push_back(glm::vec2(p[0], p[1]));
	}
}

void XFormMaker::convert(vector<glm::vec3>& dst, const vector<glm::vec2>& src) {
	dst.clear();
	for (const auto& p : src) {
		dst.push_back(glm::vec3(p[0], p[1], 0));
	}
}

void XFormMaker::textureToCenteredTexture(vector<glm::vec3>& points) {
	execute(points, glm::translate(glm::vec3(-0.5, -0.5, 0)));
}

void XFormMaker::textureToCenteredTexture(vector<glm::vec2>& points) {
	execute(points, glm::translate(glm::vec3(-0.5, -0.5, 0)));
}

void XFormMaker::centeredTextureToTexture(vector<glm::vec3>& points) {
	execute(points, glm::translate(glm::vec3(0.5, 0.5, 0)));
}

void XFormMaker::centeredTextureToTexture(vector<glm::vec2>& points) {
	execute(points, glm::translate(glm::vec3(0.5, 0.5, 0)));
}

void XFormMaker::getCenteredTexture(vector<glm::vec3>& points, vector<vector<int>>& triangles, vector<glm::vec2>& textures) {
	points.clear();
	triangles.clear();
	textures.clear();
	points.push_back(glm::vec3(-0.5, -0.5, 0));
	points.push_back(glm::vec3(0.5, -0.5, 0));
	points.push_back(glm::vec3(0.5, 0.5, 0));
	points.push_back(glm::vec3(-0.5, 0.5, 0));

	vector<int> temp;
	temp.push_back(0);
	temp.push_back(1);
	temp.push_back(2);
	triangles.push_back(temp);
	temp.clear();
	temp.push_back(0);
	temp.push_back(2);
	temp.push_back(3);
	triangles.push_back(temp);

	textures.push_back(glm::vec2(0, 0));
	textures.push_back(glm::vec2(1, 0));
	textures.push_back(glm::vec2(1, 1));
	textures.push_back(glm::vec2(0, 1));
}

//
//bool XFormMaker::isVaildData(const vector<glm::vec3>& points){
//	int chk = true;
//	for(int i = 0 ; i < points.size() ; i ++){
//		const auto& p = points[i];
//		cout << "i=" << i << ", p=(" << p[0] << "," << p[1] << "," << p[2] << ")" << endl;
//		if( !_isValid(p) ){
//			chk = false;
//			//cout << "i=" << i << ", p=(" << p[0] << "," << p[1] << "," << p[2] << ")" << endl;
//			break;
//		}
//	}
//
//	return chk;
//}
//
//vector<glm::vec3> XFormMaker::getValidData(const vector<glm::vec3>& points){
//	vector<glm::vec3> res;
//	res.reserve(points.size());
//	for(const auto& p : points){
//		if(_isValid(p)){
//			res.push_back(p);
//		}
//	}
//	return res;
//}
//
//////////////////////////////////////////////// protected  /////////////////////////////////////
//bool XFormMaker::_isValid(float val){
//	//return (val>=0) || (val<0); // !( !(val>=0) && !(val<0) );
//	return  !( !(val>=0) && !(val<0) );
//}
//bool XFormMaker::_isValid(double val){
//	return (val>=0) || (val<0); // !( !(val>=0) && !(val<0) );
//}
//bool XFormMaker::_isValid(int val){
//	return (val>=0) || (val<0); // !( !(val>=0) && !(val<0) );
//}
//bool XFormMaker::_isValid(const glm::vec3& p){
//	return _isValid(p[0]) && _isValid(p[1]) && _isValid(p[2]);
//}
