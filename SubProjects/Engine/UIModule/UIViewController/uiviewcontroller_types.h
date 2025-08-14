#pragma once

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <GL/glm/vec3.hpp>
#endif

namespace UIViewController {
enum EVIEW_EVENT_TYPE {
	NO_EVENT = 0,
	MEASUREMENT,
	PAN,
	ZOOM,
	ZOOM_WHEEL,
	FIT,
	LIGHT,
	RESET,
	ROTATE,
	UPDATE,
};

typedef struct _PACK_VIEW_INFO_PROJ {
	int width = 0;
	int height = 0;
	float scale = 1.0f;
	float trans_x = 0.0f;
	float trans_y = 0.0f;
}PackViewProj;

typedef struct _TRANSLATE_PROJECTION {
	float dist_x = 0.0f;
	float dist_y = 0.0f;
} TranslateProjection;

const glm::vec3 kUpVector = glm::vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 kRightVector = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 kBackVector = glm::vec3(0.0f, 1.0f, 0.0f);
}
