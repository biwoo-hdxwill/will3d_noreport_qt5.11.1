#version 400

layout (location = 0) in vec3 VerPos;
layout (location = 1) in vec2 VerTex;

out vec2 TexCoord;

uniform mat4 MVP;

void main()
{
    //TexCoord = vec2(VerTex.x, 1.0-VerTex.y);
	TexCoord = VerTex;
    gl_Position = MVP * vec4(VerPos,1.0);
}
