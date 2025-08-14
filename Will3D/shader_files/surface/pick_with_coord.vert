#version 400

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

uniform mat4 MVP;
out vec3 vertCoord;

void main()
{
	gl_Position = MVP * vec4(VertexPosition, 1.0);
	vertCoord = gl_Position.xyz / gl_Position.w * 0.5 + 0.5;
} 
