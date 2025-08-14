#version 400


in vec4 Color;
uniform sampler3D VolumeTex; 
uniform int		  MinValue;
uniform int		  MaxIntensity;
//uniform sampler3D SegAirwayMaskTex;	// by jdk 160811

uniform float	  WindowLevel;	// by jdk 160811
uniform float	  WindowWidth;	// by jdk 160811

uniform bool	  isXray;

layout (location = 0) out vec4 FragColor;

void main()
{
	//FragColor = vec4(1.0f);

	if(Color.x < 0.0 || Color.x > 1.0 || Color.y < 0.0 || Color.y > 1.0 || Color.z < 0.0 || Color.z > 1.0)
	{
		FragColor = vec4(vec3(0.0), 1.0);
	}
	else
	{
		float val = 0.0f;

		if (isXray)
		{
			vec4 coord = Color;
			coord.z = 0.0f;
			for (int i = 0; i < 100; i++)	// temp range
			{
				val += (texture(VolumeTex, coord.xyz).x * 65535.0 - MinValue) / MaxIntensity;
				coord.z = coord.z + 0.01f;
			}
			val /= 100;
		}
		else
		{
			val = (texture(VolumeTex, Color.xyz).x * 65535.0 - MinValue) / MaxIntensity;
		}
		
		float minValue = WindowLevel - (WindowWidth / 2.0f);
		val = (val - minValue) / WindowWidth;
		FragColor = vec4(vec3(val), 1.0f);

		/*
		//float currSegMask = texture(SegAirwayMaskTex, Color.xyz).r;	// by jdk 160811

		//if (currSegMask == 0.0f)	// by jdk 160811
		{
			// window width, level
			float val = (texture(VolumeTex, Color.xyz).x * 65535.0 - MinValue) / MaxIntensity;
			float minValue = WindowLevel - (WindowWidth / 2.0f);
			val = (val - minValue) / WindowWidth;
			FragColor = vec4(vec3(val), 1.0f);
		}
		//else
		//{
		//	float val = (texture(VolumeTex, Color.xyz).x * 65535.0 - MinValue) / MaxIntensity * 0.5f;
		//	//FragColor = vec4(val + 0.2f, val, val + 0.5f, 1.0f);
		//	FragColor = vec4(0.3f, 0.0f, 0.7f, 1.0f);
		//}
		*/
	}
}