#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;
uniform sampler3D VolumeTex;
uniform sampler2D TransferFunc;
//uniform vec3	  invVolTexScale;
uniform vec3	  VolRange;
uniform float     StepSize;
uniform float	  MinValue;

layout (location = 0) out vec4 FragColor;

void main()
{
	vec4 entryPoint = texture(EntryPositions, TexCoord);
	vec4 exitPoint = texture(ExitPositions, TexCoord);
	
	vec3 dir = exitPoint.xyz;
	float len = exitPoint.w;
	
	dir = normalize(dir);
	//dir *= invVolTexScale;
	float deltaX = StepSize;

	vec3 deltaDir = dir*deltaX;
	
	float lengthAccum = 0.0;

    vec3 currPoint = entryPoint.xyz;
	//currPoint *= invVolTexScale;

	float currIntensity = 0.0;
    
	vec3 firstHit = vec3(0.0);

	for(; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0);
		if (currIntensity > MinValue)
		{
			firstHit = ((currPoint * 2.0) - 1.0) * VolRange;
			break;
		}

		currPoint += deltaDir;
	}

	firstHit.x = -firstHit.x;
	FragColor = vec4(firstHit, 1.0);
}

