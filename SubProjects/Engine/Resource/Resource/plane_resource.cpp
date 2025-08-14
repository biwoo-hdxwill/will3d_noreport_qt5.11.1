#include "plane_resource.h"

using glm::vec3;
using glm::vec2;

PlaneResource::PlaneResource()
{
	vertices_ = { vec3(-1.0f, -1.0f, 0.0f) ,
		vec3(1.0f, -1.0f, 0.0f),
		vec3(1.0f, 1.0f, 0.0f),
		vec3(-1.0f, 1.0f, 0.0f)
	};

	tex_coord_ = { vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
		vec2(0.0f, 1.0f)
	};

	tex_coord_inverse_y_ = {
		vec2(0.0f, 1.0f),
		vec2(1.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 0.0f)
	};

	indices_ = { 0, 1, 3, 3, 1, 2}; 
}
