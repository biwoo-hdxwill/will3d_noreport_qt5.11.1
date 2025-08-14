#version 400

in vec3 TexCoord;

uniform sampler3D VolumeTex;
uniform int		  MinValue;
uniform int		  MaxIntensity;
uniform sampler3D SegAirwayMaskTex;	// by jdk 160811

uniform float	  WindowLevel;	// by jdk 160811
uniform float	  WindowWidth;	// by jdk 160811

layout (location = 0) out vec4 FragColor;

uniform bool isForFront;

void main()
{
	if(isForFront)
	{
		FragColor = vec4(TexCoord, 1.0);
	}
	else
	{

		if (TexCoord.x < 0.0 || TexCoord.x > 1.0 || TexCoord.y < 0.0 || TexCoord.y > 1.0 || TexCoord.z < 0.0 || TexCoord.z > 1.0)
		{
			FragColor = vec4(0.0);
		}
		else
		{
			float currSegMask = texture(SegAirwayMaskTex, TexCoord.xyz).r;	// by jdk 160811

			if (currSegMask == 0.0f)	// by jdk 160811
			{
				// by jdk 160906 window width, level
				float val = (texture(VolumeTex, TexCoord.xyz).x * 65535.0 - MinValue) / MaxIntensity;
				float minValue = WindowLevel - (WindowWidth / 2.0f);
				val = (val - minValue) / WindowWidth;
				FragColor = vec4(vec3(val), 0.9f);
				// jdk end

				//FragColor = vec4(vec3((texture(VolumeTex, TexCoord.xyz).x*65535.0 - MinValue) / MaxIntensity), 0.9);
			}
			else
			{
				float val = (texture(VolumeTex, TexCoord.xyz).x*65535.0 - MinValue) / MaxIntensity * 0.5f;
				//FragColor = vec4(val + 0.2f, val, val + 0.5f, 0.9f);
				FragColor = vec4(0.3f, 0.0f, 0.7f, 1.0f);
			}
		}
	}
}
