#version 400

uniform sampler2D FinalImage;
in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

void main()
{
	FragColor = texture(FinalImage, TexCoord);	
	//FragColor = vec4(1.0);
}