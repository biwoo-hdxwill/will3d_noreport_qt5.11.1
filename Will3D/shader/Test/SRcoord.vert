#version 400

layout(location = 0) in vec3 VerPos;

out vec3 Coord;

uniform mat4 MVP;
uniform mat4 MODEL;

void main()
{
	vec4 tmp = MODEL*vec4(VerPos, 1.0);
	Coord = tmp.xyz;
	gl_Position = MVP*vec4(VerPos, 1.0);
}