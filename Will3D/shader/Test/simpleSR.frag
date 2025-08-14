#version 400


in vec3 CoordIn3DTex;
in float depth;

layout (location = 0) out vec4 FragColor;
/*layout (location = 1) out vec4 FragColor_backface;
layout (location = 2) out vec4 FragColor_frontface;*/

uniform vec3 meshcolor;
uniform bool isForBackFace;

void main()
{
	if(isForBackFace)
	{
		FragColor = vec4(CoordIn3DTex, depth);
	}
	else
	{
		FragColor = vec4(meshcolor, depth);
	}


	//FragColor_color = vec4(meshcolor, 1.0);
	//FragColor_backface = vec4(CoordIn3DTex, 1.0);
	////FragColor_frontface = vec4(CoordIn3DTex, 0.0);
	//FragColor_frontface = vec4(0.0);
	
}