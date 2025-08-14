#pragma once
#include <vector>
#include "../Core/util_global.h"
#include <gl/glm/glm.hpp>
#define _USE_MATH_DEFINES // for M_PI
#include <math.h> // for M_PI

namespace geometry{
	class UTIL_EXPORT CylinderSystem{
	private:
		glm::vec3 _center;
		glm::vec3 _xAxis;
		glm::vec3 _yAxis;
		glm::vec3 _zAxis;
		float _minAngle;
		float _maxAngle;
		float _minZ;
		float _maxZ;

	public:
		CylinderSystem();
		CylinderSystem(const glm::vec3& center, const glm::vec3& zAxis, const glm::vec3& xAxis, float minAngle = -M_PI, float maxAngle = M_PI, float minZ = -1, float maxZ = 1);
		CylinderSystem(const glm::vec3& center, const glm::vec3& zAxis, const glm::vec3& xAxis, const glm::vec3& yAxis, float minAngle = -M_PI, float maxAngle = M_PI, float minZ = -1, float maxZ = 1);
		CylinderSystem(const std::vector<glm::vec3>& points, const glm::vec3& center, const glm::vec3& zAxis);
		float getMinAngle() const;
		float getMaxAngle() const;
		CylinderSystem& setMinAngle(float minAngle);
		CylinderSystem& setMaxAngle(float maxAngle);
		float getMinZ() const;
		float getMaxZ() const;
		CylinderSystem& setMinZ(float minZ);
		CylinderSystem& setMaxZ(float maxZ);
		glm::vec3 getCenter() const;
		glm::vec3 getXAxis() const;
		glm::vec3 getZAxis() const;
		glm::vec3 cylinderToPoint(const glm::vec3& cylinderCoord) const;
		
		// (x,y,z) --> (r,th,z)
		// [caution] th is in [-pi, pi]
		glm::vec3 pointToCylinder(const glm::vec3& point) const;

		// (x,y,z) --> (r,th,z)
		// [caution] th is in [0, 2*pi]
		glm::vec3 pointToCylinder360(const glm::vec3& points) const;
	};
}
