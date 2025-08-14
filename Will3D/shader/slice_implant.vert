#version 400

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 Normal;
out vec3 Position;
out float color_alpha;

uniform mat3 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform vec4 clip_plane;
uniform vec4 front_plane;
uniform vec4 back_plane;
uniform bool use_clipping = false;
uniform float alpha;

void main()
{
	if(use_clipping){
		vec3 model_position =  (ModelMatrix * vec4(VertexPosition, 1.0)).xyz;

		if(dot(vec4(model_position, 1.0), front_plane) < 0.0f &&
		dot(vec4(model_position, 1.0), back_plane) > 0.0f) {
			  float inside = dot(vec4(model_position, 1.0), clip_plane);
			  if(inside < 0.0f){
			  	color_alpha = alpha;
			  } else color_alpha = 1.0f;
		}
		else{
			color_alpha = 0.0;
		}

	} else color_alpha = alpha;

	Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
	Normal = normalize(NormalMatrix * VertexNormal);

	gl_Position = MVP * vec4(VertexPosition, 1.0);
}
