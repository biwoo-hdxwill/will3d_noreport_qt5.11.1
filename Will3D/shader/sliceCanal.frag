#version 400


in vec4 Color;
uniform sampler3D VolumeTex; 
uniform int		  MinValue;
uniform int		  MaxIntensity;
uniform vec3	  VolScale;

uniform float	  WindowLevel;	// by jdk 160811
uniform float	  WindowWidth;	// by jdk 160811

layout (location = 0) out vec4 FragCoord;
layout (location = 1) out float FragColor;

void main()
{
	//FragColor = vec4(1.0f);

	//if(Color.x < 0.5/200.0 || Color.x > 199.5/200.0 || Color.y < 0.5/200.0 || Color.y > 199.5/200.0 || Color.z < 0.5/100.0 || Color.z > 99.5/100.0)
	
	if(Color.x < 0.0 || Color.x > 1.0 || Color.y < 0.0 || Color.y > 1.0 || Color.z < 0.0 || Color.z > 1.0)
	{
		FragColor = 0.0;
		FragCoord = vec4(Color.xyz*VolScale, 1.0);// ;
	}
	else
	{
		// window width, level
		float val = (texture(VolumeTex, Color.xyz).x * 65535.0 - MinValue) / MaxIntensity;
		float minValue = WindowLevel - (WindowWidth / 2.0f);
		val = (val - minValue) / WindowWidth;
		FragColor = val;

		//FragColor = (texture(VolumeTex, Color.xyz).x * 65535.0 - MinValue) / MaxIntensity;
		FragCoord = vec4(Color.xyz * VolScale, 1.0); //Color.xyz*VolScale;
	}

	
}