#pragma once

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/mat4x4.hpp>
#endif


namespace UIGLObjects {
	enum ObjCoordSysType {
		TYPE_VOLUME,
		TYPE_PANORAMA,
		TYPE_PANORAMA_SLICE
	};
	enum TransformType {
		TRANSLATE,
		ROTATE,
		SCALE,
		ARCBALL, //view matrix에 arcball이 곱해져 있다면 빼야함.
		REORIENTATION,
	};
	enum Orientation {
		CCW,
		CW,
	};

	const glm::vec3 kDefalutKa = glm::vec3(0.55f);
	const glm::vec3 kDefalutKd = glm::vec3(0.1f);
	const glm::vec3 kDefalutKs = glm::vec3(1.0f, 0.0f, 0.0f);
	const float kDefalutShinines = 15.0f;

	const glm::vec4 kDefaultLightPosition = glm::vec4(0.0f);
	const glm::vec3 kDefaultLightIntensity = glm::vec3(1.0f);

	typedef struct _MATERIALCOLOR {
		glm::vec3 Ka;
		glm::vec3 Kd;
		glm::vec3 Ks;
		float Shininess;

		_MATERIALCOLOR(const glm::vec3& ka,
				 const glm::vec3& kd,
				 const glm::vec3& ks,
				 const float& shininess){
			Ka = ka;
			Kd = kd;
			Ks = ks;
			Shininess = shininess;
		}

		_MATERIALCOLOR() :
		Ka(kDefalutKa), Kd(kDefalutKd), Ks(kDefalutKs), Shininess(kDefalutShinines) {}

	} Material;

	typedef struct _LIGHT_INFO {
		glm::vec4 position = kDefaultLightPosition;
		glm::vec3 intensity = kDefaultLightIntensity;
	} Light;

	typedef struct _TRANSFORM_MATRIX {
		glm::mat4 rotate;
		glm::mat4 scale;
		glm::mat4 translate;
		glm::mat4 arcball;
		glm::mat4 reorien;
	} TransformMat;
};
