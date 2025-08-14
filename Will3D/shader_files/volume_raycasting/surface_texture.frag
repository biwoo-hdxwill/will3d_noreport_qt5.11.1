#version 400

uniform sampler2D tex1;

in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;
uniform float alpha;

void main()
{
	FragColor = vec4(texture(tex1, TexCoord).rgb, alpha);	
}