
#version 400

uniform sampler2D MaskImage;
uniform sampler2D MaskImage2;
uniform bool		isThereSecondVolume;
uniform bool		isThisAfterFront;
in vec2 TexCoord;

void main()
{
	if(isThisAfterFront)
	{
		if(isThereSecondVolume)
		{
			float mask = texture(MaskImage, TexCoord).a;	
			float mask2 = texture(MaskImage2, TexCoord).a;	

			if(mask == 0.0 && mask2 == 0.0)
			{
				gl_FragDepth = 1.0;
			}
			else
			{
				gl_FragDepth = 0.0;
			}
		}
		else
		{
			float mask = texture(MaskImage, TexCoord).a;	
			if(mask == 0.0)
			{
				gl_FragDepth = 1.0;
			}
			else
			{
				gl_FragDepth = 0.0;
			}
		}
		

	}
	else
	{
		float mask = texture(MaskImage, TexCoord).a;	
		if(mask == 1.0)
		{
			gl_FragDepth = 0.0;
		}	
		else
		{
			gl_FragDepth = 1.0;
		}
	}
	
}