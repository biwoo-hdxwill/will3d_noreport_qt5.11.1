#version 400

uniform sampler2D image_texture;

in vec2 TexCoord;

uniform float		WindowLevel;	
uniform float		WindowWidth;

layout (location = 0) out vec4 FragColor;

void main()
{
	
	float windowMin = WindowLevel - (WindowWidth / 2.0f);
	float windowMax = WindowLevel + (WindowWidth / 2.0f);
	
	float val = clamp(texture(image_texture, TexCoord).x * 65535.0f,
    min(windowMin, windowMax), max(windowMin, windowMax));

	val = (val - windowMin) / WindowWidth;

	FragColor = vec4(vec3(val), 1.0f);	
}