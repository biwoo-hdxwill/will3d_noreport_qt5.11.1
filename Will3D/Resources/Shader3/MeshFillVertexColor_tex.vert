#version 400

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;
layout(location = 2) in vec4 VertexColor;
out vec3 Normal;
out vec3 Point;
out vec4 Color;
uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
	gl_Position = P*V*M * vec4(VertexPosition, 1.0);
	Normal = normalize( vec3(M * vec4(VertexNormal, 0.0) ) );
	Point = vec3(M*vec4(VertexPosition, 1.0));
	Color = VertexColor;
}