#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;


uniform bool		isPlaneClipped;
uniform vec4		clipPlanes[16];
uniform int			numClipPlanes;


layout (location = 0) out vec4 FragColor_entry;
layout (location = 1) out vec4 FragColor_exit;

void ClippingPlane(inout vec3 entryPoint, inout vec3 exitPoint, inout float entryDepth, inout float exitDepth)
{
	for(int i = 0 ; i < numClipPlanes; i++)
	{
		vec3 PlaneNormal = normalize(clipPlanes[i].xyz);
		vec3 PlanePoint = vec3(0.5f) + PlaneNormal*clipPlanes[i].w*0.5f;
	
		vec3 dir = exitPoint - entryPoint;
		float len = length(dir);
		
		dir = dir/len;
		
		float distUnit = dot(dir, PlaneNormal);
		bool isFrontFace = (distUnit >= 0.0) ? true : false;
		
		float dist;
		vec3 texPlanePoint = PlanePoint;
		
		if(distUnit != 0.0)
			dist = (dot(PlaneNormal, texPlanePoint) - dot(PlaneNormal, entryPoint)) / distUnit;
		else
		{
			dist = dot(texPlanePoint - entryPoint, PlaneNormal);
			
			if(dist >= 0.0)
			{
				entryDepth = exitDepth = 0.0f;
				entryPoint = vec3(0.0f);
				exitPoint = vec3(0.0f);
				return;
			}
				
		}
		
		if(isFrontFace)
		{
			if(dist < len && dist > 0.0)
			{
				vec3 interPoint = entryPoint + dist*dir;
				entryDepth = entryDepth + (exitDepth - entryDepth)*(dist/len) - 0.00001f;
				entryPoint = interPoint;
			}
			else if( dist >= len)
			{
				entryDepth = exitDepth = 0.0f;
				entryPoint = vec3(0.0f);
				exitPoint = vec3(0.0f);
				return;
			}
		
		}
		else
		{
			if( dist <= len && dist > 0.0)
			{
				exitDepth = entryDepth + (exitDepth - entryDepth)*(dist/len) - 0.00001f;
				exitPoint = entryPoint + dir*dist;
			}
			else if( dist > len)
			{
			}
			else
			{
				entryDepth = exitDepth = 0.0f;
				entryPoint = vec3(0.0f);
				exitPoint = vec3(0.0f);
				return;
			}
		}
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
	
	if(isPlaneClipped)
		ClippingPlane(entryPoint, exitPoint, entryDepth, exitDepth);
	
	FragColor_entry = vec4(entryPoint, entryDepth);
	FragColor_exit = vec4(exitPoint, exitDepth);
	gl_FragDepth = entryDepth;
}
