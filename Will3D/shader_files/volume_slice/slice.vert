#version 400

layout (location = 0) in vec3 VertexPosition;
//layout (location = 1) in vec3 VertexNormal;

out vec3 TexCoord;

uniform mat4 VolTexBias;
uniform mat4 VolTexTransformMat;
uniform mat4 MVP;

void main()
{
	vec4 VerTex = VolTexBias*VolTexTransformMat*vec4(VertexPosition, 1.0f);
	TexCoord =  vec3(VerTex);

	gl_Position = MVP * vec4(VertexPosition, 1.0f);
}