#version 400

//layout(location = 0) in vec3 VerPos;
//layout(location = 1) in vec3 VerTex;

//out vec3 TexCoord;

//uniform mat4 MVP;

//void main()
//{
//	TexCoord = VerTex;
//	gl_Position = MVP * vec4(VerPos, 1.0);
//}

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec2 VerTex;
out vec3 TexCoord;

uniform mat4 MVP;
uniform mat4 sliceModel;

void main()
{

	vec4 pos = sliceModel*vec4(VerPos, 1.0);
	TexCoord = pos.xyz*0.5 + 0.5;
	TexCoord.x = 1.0 - TexCoord.x;

	gl_Position = MVP * pos;
}
