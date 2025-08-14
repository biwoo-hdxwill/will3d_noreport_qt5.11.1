#version 400

in vec2 Color;

uniform sampler2D	ImageSlice;
uniform sampler2D	MaskSlice;
uniform float		minVal;
uniform float		normVal;
uniform bool		isCanalShown;
uniform bool		isNormalized;
uniform bool		isSecond;

layout (location = 0) out vec4 FragColor;


void main()
{
	float value;
	
	if(isCanalShown)
	{
		float mvalue = texture(MaskSlice, Color).x;

		if(mvalue > 0.5f)
		{
			FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0);
		}
		else
		{
			if(isNormalized)
			{
				value = texture(ImageSlice, Color).x;
			}
			else
			{
				value = (texture(ImageSlice, Color).x - minVal)/normVal;
			}
			

			FragColor = vec4(value, value, value, 1.0);
		}
	}
	else
	{
		if(isNormalized)
		{
			value = texture(ImageSlice, Color).x;
		}
		else
		{
			value = (texture(ImageSlice, Color).x - minVal)/normVal;
		}

		if(isSecond)
		{
			FragColor = vec4(value, value, 0.0, 0.5);
		}
		else
		{
			FragColor = vec4(value, value, value, 1.0);
		}
		
	}

	

	//float value = (texture(ImageSlice, Color).x + 1.0)/2.0f;

    
}
