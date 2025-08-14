#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerNor;

out vec3 Position;
out vec3 Normal;
out vec3 VolTexPosition;

uniform mat4 MVP;		
uniform mat4 VolTexTransformMat;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

void main()
{
	
	VolTexPosition = (VolTexTransformMat * vec4(VerPos, 1.0)).xyz;
	Position = (ModelViewMatrix * vec4(VerPos, 1.0)).xyz;
	Normal = normalize(NormalMatrix * VerNor);

	gl_Position = MVP*vec4(VerPos, 1.0);
}
