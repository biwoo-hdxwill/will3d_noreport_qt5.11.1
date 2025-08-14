#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;

uniform sampler3D VolumeTex;
uniform sampler2D TransferFunc;    
uniform float     StepSize; 
uniform float	  ThresholdAlpha;
uniform int		  MaxTexSize;


layout (location = 0) out vec4 FragColor;

vec2 GetTransferIdx(vec3 voxPos)
{
	float intensity = texture(VolumeTex, voxPos).r*65535.0 + 0.5;
	vec2 tffTexSize = textureSize(TransferFunc, 0);
	return vec2((intensity - (int(intensity) / MaxTexSize) * MaxTexSize) / (tffTexSize.x), (int(intensity) / MaxTexSize + 0.5) / (tffTexSize.y));
}

void GetHitPos(inout vec3 currPoint, vec3 deltaDir, float deltaX, float len,
inout vec3 firstHit)
{
	float lengthAccum = 0.0;
	float currIntensity = 0.0;
	float TFidx;
	int TFidy;
	vec2 TFid;
	vec4 currColor = vec4(0.0);
	vec4 accumColor = vec4(0.0);

	bool isFound = false;

	for(; lengthAccum < len; lengthAccum += deltaX)
	{		
		currColor = texture(TransferFunc, GetTransferIdx(currPoint));
		
		if (currColor.a > 0.0)
		{
			currColor.rgb *= currColor.a;
			accumColor = accumColor + (1.0 - accumColor.a)*currColor;

			if(!isFound)
			{
				isFound = true;
			}
		}
		
		if(accumColor.a > ThresholdAlpha)
		{
			firstHit = currPoint;
			break;
		}

		currPoint += deltaDir;
	}
}

void main()
{	 
	vec3 exitPoint = texture(ExitPositions, TexCoord).xyz;
	vec3 entryPoint = texture(EntryPositions, TexCoord).xyz;
	
	float entryDepth = (texture(EntryPositions, TexCoord)).w;
	float exitDepth = (texture(ExitPositions, TexCoord)).w;
	
	if(exitPoint == entryPoint)
		return;
	
	vec3 dir = (exitPoint - entryPoint);
	float len = length(dir);
	dir = dir/len;

	vec3 currPoint = entryPoint;
	vec3 deltaDir = dir*StepSize;
	
	vec3 firstHitPos, secondHitPos;
		
	firstHitPos = vec3(0.0);
	secondHitPos = vec3(0.0);
		
	GetHitPos(currPoint, deltaDir, StepSize, len, firstHitPos);

	bool bFirstHit = (firstHitPos != vec3(0.0)) ? true : false;	
		
	if(bFirstHit)
	{
		entryDepth = entryDepth + (exitDepth - entryDepth)*(length(firstHitPos - entryPoint)/len);
		gl_FragDepth = entryDepth;
		FragColor = vec4(firstHitPos, entryDepth);
	}
	else
	{
		gl_FragDepth = 1.0f;
		FragColor = vec4(exitPoint, 0.0);
	}
}
