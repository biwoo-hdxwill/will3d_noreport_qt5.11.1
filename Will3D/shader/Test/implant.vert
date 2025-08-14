#version 400

layout(location = 0) in vec3 VerPos;
layout(location = 1) in vec3 VerNor;

out vec3 Normal;
out vec3 Position;

uniform mat4 MVP;		
uniform mat4 PositionMatrix; // bone density����� �� position�� texture ��ǥ��� ��ȯ��. �Ϲ������� MV.
uniform mat3 NormalMatrix;

uniform bool invert_x = false;
uniform bool isYinvert = false;

void main()
{
	
	Position = (PositionMatrix * vec4(VerPos, 1.0)).xyz;
	Normal = normalize(NormalMatrix * VerNor);

	vec4 position = MVP * vec4(VerPos, 1.0);

	if (isYinvert)
	{
		position.y *= -1.0;
	}
	if (invert_x)
	{
		position.x *= -1.0;
	}

	gl_Position = position;
}

//#version 400

//layout(location = 0) in vec3 VerPos;
//layout(location = 1) in vec3 VerNor;

//out vec4 color;
//out vec3 Normal;	// Texture coordinate
//out vec3 Tex;		// Texture coordinate
//out vec3 TexForShading;
//out vec3 NormalForShading;
//// shading �� texture coordinate

//uniform mat4 MVP;		// implant only or vr �� ���� �޶���
//uniform mat4 ModelToTexture;	// for BoneDensity
//uniform mat4 MatForNormal;
//uniform mat4 ModelToTextureForShading; // for Shading
//uniform mat4 MatForNormalShading; // for Shading
//uniform vec3 VolTexScale;

//uniform vec4 colorIn;


//uniform bool isWire;
//uniform bool isYinvert;
//uniform bool isBoneDensity;
//uniform bool isPano3D;

//void main()
//{
//	if(isWire)
//	{
//		color = colorIn;
//	}
//	else
//	{
		
//		if(isPano3D)
//		{
//			vec4 Normal4 = MatForNormalShading*vec4(VerNor, 0.0f);
//			NormalForShading = normalize(Normal4.xyz);

//			vec4 texCoord = ModelToTextureForShading*vec4(VerPos, 1.0);
//			TexForShading = texCoord.xyz;	
			
//			if(isBoneDensity)
//			{
//				Normal4 = MatForNormal*vec4(VerNor, 0.0f);
//				Normal = normalize(Normal4.xyz);
//				texCoord = ModelToTexture*vec4(VerPos, 1.0);
//				Tex = texCoord.xyz;
//			}
//			else
//			{
//				color = colorIn;	
//			}
					
//		}
//		else
//		{
//			vec4 Normal4 = MatForNormal*vec4(VerNor, 0.0f);
//			Normal = normalize(Normal4.xyz);

//			vec4 texCoord = ModelToTexture*vec4(VerPos, 1.0);
//			Tex = texCoord.xyz;	

//			NormalForShading = Normal;
//			TexForShading = Tex*VolTexScale;

//			if(!isBoneDensity)
//			{
//				color = colorIn;
//			}
//		}

//	}

//	if(isYinvert)
//	{
//		vec4 tmp = MVP*vec4(VerPos, 1.0);
//		tmp.y *= -1.0;
//		gl_Position = tmp; 
//	}
//	else
//	{
//		gl_Position = MVP*vec4(VerPos, 1.0);	
//	}
//}
