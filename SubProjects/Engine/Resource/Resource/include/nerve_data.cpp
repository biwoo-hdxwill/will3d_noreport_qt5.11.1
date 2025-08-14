#include "nerve_data.h"
#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#endif


#include <qmath.h>
#include "../../Common/Common/W3ElementGenerator.h"
#include "../../Common/Common/W3Logger.h"
namespace {
	const int kMeshCirclePoints = 36;
	const float kMeshCircleSegAngle = (2.0f*M_PI) / kMeshCirclePoints;
	const int kStacks = 50;
	const int kSlices = 50;

	void samplingPoints(const float divideLen, std::vector<glm::vec3>& points) {

		std::vector<glm::vec3> equiSpliPoints;
		equiSpliPoints.push_back(points.front());
		for (int i = 1; i < points.size(); i++) {
			glm::vec3 p0 = equiSpliPoints.back();
			glm::vec3 p1 = points[i];

			glm::vec3 v = p1 - p0;
			int length = glm::length(v);

			v /= length;

			for (int j = 0; j < length; j++)
				equiSpliPoints.push_back(p0 + v*((float)j + 1));
		}

		if (equiSpliPoints.size() > divideLen) {
			int splitStep;
			splitStep = (int)divideLen;

			points.clear();

			for (int i = 0; i < equiSpliPoints.size(); i += splitStep)
				points.push_back(equiSpliPoints[i]);

			glm::vec3 tailVec = points.back() - equiSpliPoints.back();
			int tailLen = glm::length(tailVec);
			if (tailLen <= splitStep) {
				points.pop_back();
				points.push_back((points.back() + equiSpliPoints.back())*0.5f);
			}

			points.push_back(equiSpliPoints.back());
		}
	}
}
using glm::vec3;
using glm::vec4;
using glm::mat4;

NerveData::NerveData() {
}

NerveData::~NerveData() {
}




void NerveData::GenerateMesh(const std::vector<vec3>& nerve_points_in_gl, const glm::vec3& nerve_radius_scale_in_gl) {


	mesh_vertices_.clear();
	mesh_indices_.clear();
	mesh_normals_.clear();
	std::vector<vec3> tmp_points = nerve_points_in_gl;
	if (tmp_points.size() < 2) {
		common::Logger::instance()->Print(common::LogType::ERR, "NerveData::GenerateMesh: nerve point invalid.");
		return;
	}

	samplingPoints(std::max(radius_*2.0f, 1.0f), tmp_points);

	int vertices_size = kMeshCirclePoints*tmp_points.size() + (kStacks*kSlices);
	mesh_vertices_.reserve(vertices_size);
	mesh_indices_.reserve(vertices_size * 6 - kMeshCirclePoints * 6);
	mesh_normals_.reserve(vertices_size);

	glm::vec3 tail = tmp_points.back() * 2.0f - tmp_points.at(tmp_points.size() - 2);
	tmp_points.push_back(tail);

	glm::vec3 unit = vec3(1.0f);
	for (int i = 0; i < tmp_points.size() - 1; i++) {
		vec3 p1 = tmp_points[i];
		vec3 p2 = tmp_points[i + 1];
	
		vec3 dir = normalize(p2 - p1);
		vec3 ortho = GetOrthoVector(dir, unit);
	
		int index = kMeshCirclePoints*i;
	
		vec4 ortho_rotate = vec4(ortho, 0.0f);
	
		for (int r = 0; r < kMeshCirclePoints; r++) {
			ortho_rotate = rotate(kMeshCircleSegAngle, dir)*ortho_rotate;
			mesh_vertices_.push_back(p1 + vec3(ortho_rotate)*nerve_radius_scale_in_gl);
		}
	
		if (i > tmp_points.size() - 3)
			continue;
	
		for (int r = 0; r < kMeshCirclePoints; r++) {
			uint i0 = index + r;
			uint i1 = index + (r + 1) % kMeshCirclePoints;
			uint i2 = index + r + kMeshCirclePoints;
			uint i3 = index + (r + 1) % kMeshCirclePoints + kMeshCirclePoints;
	
			mesh_indices_.push_back(i0);
			mesh_indices_.push_back(i1);
			mesh_indices_.push_back(i2);
	
			mesh_indices_.push_back(i2);
			mesh_indices_.push_back(i1);
			mesh_indices_.push_back(i3);
	
		}
	}

	auto func_gen_sphere = [&](const glm::vec3& position, const glm::vec3& dir){
		auto func_invalid_vertex = [&](const int& vertex_idx) -> bool{
			glm::vec3 v = glm::normalize(mesh_vertices_[vertex_idx] - position);			
			return (glm::dot(v, dir) >= 0.0f) ? false : true;
		};

		auto func_push_triangle = [&](const int& a, const int& b, const int& c) {
			if (func_invalid_vertex(a) &&
				func_invalid_vertex(b) &&
				func_invalid_vertex(c))
				return;

			mesh_indices_.push_back(a);
			mesh_indices_.push_back(b);
			mesh_indices_.push_back(c);
		};

		auto func_push_quad = [&](const int& a, const int& b, const int& c, const int& d) {
			if (func_invalid_vertex(a) &&
				func_invalid_vertex(b) &&
				func_invalid_vertex(c) &&
				func_invalid_vertex(d))
				return;

			mesh_indices_.push_back(a);
			mesh_indices_.push_back(b);
			mesh_indices_.push_back(c);
			mesh_indices_.push_back(a);
			mesh_indices_.push_back(c);
			mesh_indices_.push_back(d);
		};

		int begin_iter = mesh_vertices_.size();

		mesh_vertices_.push_back(position + nerve_radius_scale_in_gl * vec3(0.0f, 1.0f, 0.0f));

		for (uint32_t j = 0; j < kStacks - 1; ++j) {
			const float polar = (float)M_PI * float(j + 1) / float(kStacks);
			const float sp = std::sin(polar);
			const float cp = std::cos(polar);
			for (uint32_t i = 0; i < kSlices; ++i) {
				const float azimuth = (float)(2.0 * M_PI) * float(i) / float(kSlices);
				const float sa = std::sin(azimuth);
				const float ca = std::cos(azimuth);
				const float x = sp * ca;
				const float y = cp;
				const float z = sp * sa;
				mesh_vertices_.push_back(vec3(x, y, z) * nerve_radius_scale_in_gl + position);
				mesh_normals_.push_back(vec3(x, y, z));
			}
		}

		mesh_vertices_.push_back(position + nerve_radius_scale_in_gl * vec3(0.0f, -1.0f, 0.0f));

		for (int i = 0; i < kSlices; ++i) {
			const int a = begin_iter + i + 1;
			const int b = begin_iter + (i + 1) % kSlices + 1;
			func_push_triangle(begin_iter, b, a);
		}

		for (int j = 0; j < kStacks - 2; ++j) {
			const int start_a = begin_iter + j * kSlices + 1;
			const int start_b = begin_iter + (j + 1)*kSlices + 1;
			for (int i = 0; i < kSlices; ++i) {
				const int a = start_a + i;
				const int a1 = start_a + (i + 1) % kSlices;
				const int b = start_b + i;
				const int b1 = start_b + (i + 1) % kSlices;
				func_push_quad(a, a1, b1, b);
			}
		}

		for (int i = 0; i < kSlices; ++i) {
			const int a = begin_iter + i + kSlices * (kStacks - 2) + 1;
			const int b = begin_iter + (i + 1) % kSlices + kSlices * (kStacks - 2) + 1;
			func_push_triangle(mesh_vertices_.size() - 1, a, b);
		}
	};

	//시작과 끝에 반구모양으로 처리함.
	tmp_points.pop_back();
	int idx_back = tmp_points.size() - 1;
	func_gen_sphere(tmp_points[0], glm::normalize(tmp_points[0] - tmp_points[1]));
	func_gen_sphere(tmp_points[idx_back], glm::normalize(tmp_points[idx_back] - tmp_points[idx_back - 1]));
	//


	CW3ElementGenerator::GenerateSmoothNormals(mesh_vertices_, mesh_indices_, mesh_normals_);
}

inline vec3 NerveData::GetOrthoVector(const vec3 & dir, const vec3& unit) const {
	vec3 tmp = normalize(unit);
	const float offset = 0.00001f;

	float dot = glm::dot(dir, tmp);

	if (dot <= offset && dot >= -offset)
		tmp.z += offset;
	else if (dot <= 1 + offset && dot >= 1 - offset)
		tmp.z += offset;

	return normalize(cross(dir, tmp));
}


