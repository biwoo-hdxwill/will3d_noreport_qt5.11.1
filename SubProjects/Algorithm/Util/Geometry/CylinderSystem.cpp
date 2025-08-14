#include "CylinderSystem.h"
#include <exception>
#include <iostream>
#include <complex>
#include <algorithm>
using namespace std;

namespace geometry {
	CylinderSystem::CylinderSystem() {
		_center = glm::vec3(0, 0, 0);
		_xAxis = glm::vec3(1, 0, 0);
		_zAxis = glm::vec3(0, 0, 1);
		_yAxis = glm::cross(_zAxis, _xAxis);
		_minAngle = -M_PI;
		_maxAngle = M_PI;
	}
	CylinderSystem::CylinderSystem(const glm::vec3& center, const glm::vec3& zAxis, const glm::vec3& xAxis, float minAngle, float maxAngle, float minZ, float maxZ)
		: _center(center), _xAxis(xAxis), _zAxis(zAxis), _minAngle(minAngle), _maxAngle(maxAngle), _minZ(minZ), _maxZ(maxZ) {
		try {
			if (!glm::length(_zAxis)) {
				throw std::runtime_error("zAxis is (0,0,0)");
			}
			if (!glm::length(_xAxis)) {
				throw std::runtime_error("xAxis is (0,0,0)");
			}
			_zAxis /= glm::length(_zAxis);
			_xAxis = _xAxis - glm::dot(_zAxis, _xAxis) * _zAxis;
			if (!glm::length(_xAxis)) {
				throw std::runtime_error("xAxis normal to zAxis is (0,0,0) <-- zAxis and xAxis are in linear dependent direction");
			}
			_xAxis /= glm::length(_xAxis);
			_yAxis = glm::cross(_zAxis, _xAxis);
			if (!glm::length(_yAxis)) {
				throw std::runtime_error("yAxis is (0,0,0) <-- zAxis and xAxis are in linear dependent direction");
			}
			_yAxis /= glm::length(_yAxis);
		} catch (std::runtime_error& e) {
			cout << "CylinderSystem::CylinderSystem: " << e.what() << endl;
			_center = glm::vec3(0, 0, 0);
			_xAxis = glm::vec3(1, 0, 0);
			_zAxis = glm::vec3(0, 0, 1);
			_yAxis = glm::cross(_zAxis, _xAxis);
			abort(); //
		}
	}

	CylinderSystem::CylinderSystem(const glm::vec3& center, const glm::vec3& zAxis, const glm::vec3& xAxis, const glm::vec3& yAxis, float minAngle, float maxAngle, float minZ, float maxZ)
		: _center(center), _xAxis(xAxis), _zAxis(zAxis), _minAngle(minAngle), _maxAngle(maxAngle), _minZ(minZ), _maxZ(maxZ) {
		try {
			if (!glm::length(_zAxis)) {
				throw std::runtime_error("zAxis is (0,0,0)");
			}
			if (!glm::length(_xAxis)) {
				throw std::runtime_error("xAxis is (0,0,0)");
			}
			_zAxis /= glm::length(_zAxis);
			_xAxis = _xAxis - glm::dot(_zAxis, _xAxis) * _zAxis;
			if (!glm::length(_xAxis)) {
				throw std::runtime_error("xAxis normal to zAxis is (0,0,0) <-- zAxis and xAxis are in linear dependent direction");
			}
			_xAxis /= glm::length(_xAxis);
			_yAxis = glm::cross(_zAxis, _xAxis);
			if (!glm::length(_yAxis)) {
				throw std::runtime_error("yAxis is (0,0,0) <-- zAxis and xAxis are in linear dependent direction");
			}
			_yAxis /= glm::length(_yAxis);
			if (!glm::length(yAxis)) {
				throw std::runtime_error("input yAxis is (0,0,0)");
			}
			if (glm::dot(_yAxis, yAxis) < 0) {
				_yAxis = -_yAxis;
				cout << "[warning] CylinderSystem::CylinderSystem: " << "coordinate is left handed" << endl;
			}
		} catch (std::runtime_error& e) {
			cout << "CylinderSystem::CylinderSystem: " << e.what() << endl;
			_center = glm::vec3(0, 0, 0);
			_xAxis = glm::vec3(1, 0, 0);
			_zAxis = glm::vec3(0, 0, 1);
			_yAxis = glm::cross(_zAxis, _xAxis);
			abort(); //
		}
	}

	CylinderSystem::CylinderSystem(const vector<glm::vec3>& points, const glm::vec3& center, const glm::vec3& zAxis) {
		try {
			if (points.empty()) {
				throw std::runtime_error("points is empty");
			}
			glm::vec3 xAxis = glm::vec3(0, 0, 0);
			for (const auto& p : points) {
				xAxis += p - center;
			}
			if (glm::length(xAxis)) {
				xAxis /= glm::length(xAxis);
			} else if (glm::length(glm::cross(glm::vec3(1, 0, 0), zAxis))) {
				xAxis = glm::vec3(1, 0, 0);
			} else {
				xAxis = glm::vec3(0, 1, 0);
			}
			this->CylinderSystem::CylinderSystem(center, zAxis, xAxis); // to call constr must use "this->"

			// find minmax angle and minmax z
			auto minmaxAngle = std::minmax_element(points.cbegin(), points.cend(),
				[this](const glm::vec3& p1, const glm::vec3& p2)->bool {
				return pointToCylinder(p1)[1] < pointToCylinder(p2)[1];
			}
			);
			float minAngle = pointToCylinder(*minmaxAngle.first)[1];
			float maxAngle = pointToCylinder(*minmaxAngle.second)[1];
			auto minmaxZ = std::minmax_element(points.cbegin(), points.cend(),
				[this](const glm::vec3& p1, const glm::vec3& p2)->bool {
				return pointToCylinder(p1)[2] < pointToCylinder(p2)[2];
			}
			);
			float minZ = pointToCylinder(*minmaxZ.first)[2];
			float maxZ = pointToCylinder(*minmaxZ.second)[2];
			setMinAngle(minAngle).setMaxAngle(maxAngle).setMinZ(minZ).setMaxZ(maxZ);
		} catch (std::runtime_error& e) {
			_center = center;
			_zAxis = zAxis;
			if (glm::length(glm::cross(glm::vec3(1, 0, 0), zAxis))) {
				_xAxis = glm::vec3(1, 0, 0);
			} else {
				_xAxis = glm::vec3(0, 1, 0);
			}
			_yAxis = glm::cross(_zAxis, _xAxis);
			_minAngle = -M_PI;
			_maxAngle = M_PI;
			cout << "CylinderSystem::CylinderSystem: " << e.what() << endl;
		}
	}
	float CylinderSystem::getMinAngle() const {
		return _minAngle;
	}
	float CylinderSystem::getMaxAngle() const {
		return _maxAngle;
	}
	CylinderSystem& CylinderSystem::setMinAngle(float minAngle) {
		_minAngle = minAngle;
		return *this;
	}
	CylinderSystem& CylinderSystem::setMaxAngle(float maxAngle) {
		_maxAngle = maxAngle;
		return *this;
	}
	float CylinderSystem::getMinZ() const {
		return _minZ;
	}
	float CylinderSystem::getMaxZ() const {
		return _maxZ;
	}
	CylinderSystem& CylinderSystem::setMinZ(float minZ) {
		_minZ = minZ;
		return *this;
	}
	CylinderSystem& CylinderSystem::setMaxZ(float maxZ) {
		_maxZ = maxZ;
		return *this;
	}
	glm::vec3 CylinderSystem::getCenter() const {
		return _center;
	}
	glm::vec3 CylinderSystem::getXAxis() const {
		return _xAxis;
	}
	glm::vec3 CylinderSystem::getZAxis() const {
		return _zAxis;
	}
	// cylinderCoord	:	(r, th(radian), z)
	glm::vec3 CylinderSystem::cylinderToPoint(const glm::vec3& cylinderCoord) const {
		return _center + _xAxis * cylinderCoord[0] * cos(cylinderCoord[1]) + _yAxis * cylinderCoord[0] * sin(cylinderCoord[1]) + _zAxis * cylinderCoord[2];
	}
	glm::vec3 CylinderSystem::pointToCylinder(const glm::vec3& points) const {
		glm::vec3 pc = points - _center;
		float x = glm::dot(pc, _xAxis);
		float y = glm::dot(pc, _yAxis);
		float z = glm::dot(pc, _zAxis);
		complex<float> c = complex<float>(x, y);
		return glm::vec3(abs(c), arg(c), z);
	}

	glm::vec3 CylinderSystem::pointToCylinder360(const glm::vec3& points) const {
		glm::vec3 pc = points - _center;
		float x = glm::dot(pc, _xAxis);
		float y = glm::dot(pc, _yAxis);
		float z = glm::dot(pc, _zAxis);
		complex<float> c = complex<float>(x, y);
		float th = arg(c);
		return (th >= 0) ? glm::vec3(abs(c), arg(c), z)
			: glm::vec3(abs(c), arg(c) + 2 * M_PI, z);
	}
}
