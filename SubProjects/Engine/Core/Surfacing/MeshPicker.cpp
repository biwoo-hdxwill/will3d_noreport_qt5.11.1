#include "MeshPicker.h"

#include "../../Common/Common/W3Logger.h"

bool MeshPicker::pick(glm::vec3& point,
					  int& triangleId,
					  const std::vector<glm::vec3>& points,
					  const std::vector<std::vector<int>>& triangles,
					  const glm::vec2& normPickingPoint,
					  const glm::mat4& MVP) {
	std::vector<int> meshCtrlTriangleBuf;
	std::vector<glm::vec3> meshCtrlPointBuf;
	for (int i = 0; i < triangles.size(); i++) {
		const auto& triangle = triangles[i];
		glm::vec2 p0 = glm::vec2(MVP * glm::vec4(points.at(triangle[0]), 1));
		glm::vec2 p1 = glm::vec2(MVP * glm::vec4(points.at(triangle[1]), 1));
		glm::vec2 p2 = glm::vec2(MVP * glm::vec4(points.at(triangle[2]), 1));
		glm::vec2 v1 = p1 - p0;
		glm::vec2 v2 = p2 - p0;
		glm::mat2 V(v1[0], v1[1], v2[0], v2[1]);
		glm::vec2 temp = glm::inverse(V) * (glm::vec2(normPickingPoint - p0));
		glm::vec3 lambda(1.0 - temp[0] - temp[1], temp[0], temp[1]);
		// inner point
		if (0 <= lambda[0] && lambda[0] <= 1 &&
			0 <= lambda[1] && lambda[1] <= 1 &&
			0 <= lambda[2] && lambda[2] <= 1) {
			//meshCtrlTriangleBuf.push_back(triangle[0]); // err 이거때문에 하루날림
			meshCtrlTriangleBuf.push_back(i); // err
			meshCtrlPointBuf.push_back(
				lambda[0] * points[triangle[0]] +
				lambda[1] * points[triangle[1]] +
				lambda[2] * points[triangle[2]]);
		}
	}
	if (!meshCtrlTriangleBuf.empty()) {
		int idx = std::distance(meshCtrlPointBuf.begin(),
								std::min_element(meshCtrlPointBuf.begin(),
												 meshCtrlPointBuf.end(),
												 [&](const glm::vec3& p0, const glm::vec3& p1) {
			glm::vec3 pp0 = glm::vec3(MVP * glm::vec4(p0, 1));
			glm::vec3 pp1 = glm::vec3(MVP * glm::vec4(p1, 1));
			return pp0[2] < pp1[2];
		}));
		point = meshCtrlPointBuf[idx];
		triangleId = meshCtrlTriangleBuf[idx];
	} else {
		common::Logger::instance()->Print(common::LogType::ERR,
										  "MeshPicker::pick: picking failed");
		return false;
	}
	return true;
}
