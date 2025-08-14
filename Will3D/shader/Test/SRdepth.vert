#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerNormal;
layout(location = 2) in vec2 TexCoord;

out vec3 Normal;
out vec2 TexC;
out float depth;

uniform mat4 MVP;
uniform bool isTexture;
uniform bool isYinvert;

void main()
{
	vec4 Normal4 = MVP*vec4(VerNormal, 0.0f);
	Normal = normalize(Normal4.xyz);
	if(isTexture)
	{	
		TexC = TexCoord;
	}
	if(isYinvert)
	{
		vec4 tmp = MVP*vec4(VerPos, 1.0);
		tmp.y *= -1.0;
		gl_Position = tmp; 
	}
	else
	{
		gl_Position = MVP*vec4(VerPos, 1.0);	
	}

	depth = gl_Position.z / gl_Position.w * 0.5 + 0.5;
}