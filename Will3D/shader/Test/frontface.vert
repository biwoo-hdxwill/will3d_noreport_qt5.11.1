#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerTex;

out vec3 TexCoord;
out float depth;
uniform mat4 MVP;

void main()
{
    TexCoord = VerTex;
    gl_Position = MVP * vec4(VerPos, 1.0);
	
	depth = gl_Position.z / gl_Position.w * 0.5 + 0.5;
}
