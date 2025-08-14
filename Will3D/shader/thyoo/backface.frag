#version 400

in vec3 TexCoord;
in float depth;
layout(location = 0) out vec4 FragColor;


void main()
{
	FragColor = vec4(TexCoord, depth);
}