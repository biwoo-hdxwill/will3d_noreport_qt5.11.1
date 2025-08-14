#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerClr;

out vec4 Color;

uniform mat4 invModel;
uniform mat4 MVP;

uniform bool isYinvert;

void main()
{

	Color = invModel*vec4(VerPos, 1.0);

	if(isYinvert)
	{
		vec4 tmp = MVP * vec4(VerPos, 1.0);
		tmp.y *= -1.0;
		gl_Position = tmp; 

		Color.y = Color.y*-1.0f + 1.0f;
	}
	else
	{
		gl_Position = MVP * vec4(VerPos, 1.0);
		
	}

}