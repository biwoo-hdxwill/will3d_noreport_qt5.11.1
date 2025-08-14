#version 400

layout(location = 0) out vec4 FragColor;
uniform int index;
in vec3 vertCoord;

void main()
{
	FragColor = vec4(vertCoord, index / 255.0);
} 
