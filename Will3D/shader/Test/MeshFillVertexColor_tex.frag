// current try white ...

#version 400

in vec3 Normal;
in vec3 Point;
in vec4 Color;
layout (location = 0) out vec3 Displacement;
layout (location = 1) out vec4 WorldCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	Displacement = vec3(Color);
	WorldCoord = vec4(Point, 1);
}