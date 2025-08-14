#version 400

layout(location = 0) in vec3 VertexPosition;
layout(location = 1) in vec3 VertexNormal;

out vec3 Normal;
out vec3 Position;
uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;

void main()
{
	Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
	Normal = normalize(NormalMatrix * VertexNormal);

	gl_Position = MVP * vec4(VertexPosition, 1.0);
}