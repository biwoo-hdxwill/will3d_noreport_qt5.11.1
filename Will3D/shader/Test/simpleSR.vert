#version 400

layout(location = 0) in vec3 VerPos;

out vec3 CoordIn3DTex;

out float depth;

uniform mat4 MVP;
uniform mat4 InverseScale;
uniform bool isYinvert;
uniform bool isXinvert;
uniform bool isScaled;
uniform bool isForBackFace;

void main()
{
	if(isForBackFace)
	{
		if(isScaled)
		{
			CoordIn3DTex = vec3(InverseScale*vec4(VerPos, 1.0f))*0.5 + 0.5;
		}
		else
		{
			CoordIn3DTex = VerPos*0.5 + 0.5;
		}
	
		if(isXinvert)
		{
			CoordIn3DTex.x = 1.0 - CoordIn3DTex.x;
		}
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