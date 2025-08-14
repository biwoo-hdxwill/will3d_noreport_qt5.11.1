#version 400

layout(location = 0) in vec3 VerPos;

out vec3 TexCoord;

out float depth;

uniform mat4 MVP;
uniform mat4 VolTexBias;
uniform mat4 VolTexTransformMat;

void main()
{
	vec4 VerTex = VolTexBias*VolTexTransformMat*vec4(VerPos, 1.0);

	TexCoord =  vec3(VerTex);
	gl_Position = MVP * vec4(VerPos, 1.0);
	depth = gl_Position.z / gl_Position.w * 0.5 + 0.5;
}
