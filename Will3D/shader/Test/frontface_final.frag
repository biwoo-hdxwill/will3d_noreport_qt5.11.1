#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;
uniform sampler3D VolumeTex;
uniform sampler2D TransferFunc;
uniform float     StepSize;
uniform float	  TFxincrease;
uniform float	  TFyincrease;

uniform bool		isPlaneClipped;
uniform vec4		clipPlanes[16];
uniform int			numClipPlanes;

uniform bool		isDepthFirstHit;
uniform bool		isThisSecond;

uniform bool		isPerspective;

layout (location = 0) out vec4 FragColor_entry;
layout (location = 1) out vec4 FragColor_exit;

void GetHitPos(inout vec3 currPoint, vec3 deltaDir, float deltaX, float len,
inout vec3 firstHit, inout vec3 secondHit) {
	float lengthAccum = 0.0;
	float currIntensity = 0.0;
	float TFidx;
	int TFidy;
	vec2 TFid;
	vec4 currColor = vec4(0.0);
	vec4 accumColor = vec4(0.0);

	bool isFound = false;
	for(; lengthAccum < len; lengthAccum += deltaX) {		
		currIntensity =  (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
		
		TFidx = currIntensity*TFxincrease;
		TFidy = int(TFidx);

		TFid.x = TFidx - TFidy;
		TFid.y = (TFidy + 0.5)*TFyincrease;

		currColor = texture(TransferFunc, TFid);
		
		if (currColor.a > 0.0) {
			currColor.rgb *= currColor.a;
			accumColor = accumColor + (1.0 - accumColor.a)*currColor;

			if(!isFound) {
				isFound = true;
				firstHit = currPoint;
			}
		}
		
		if(accumColor.a > 0.9) {
			secondHit = currPoint;
			break;
		}

		currPoint += deltaDir;
	}
}

void main() {
	vec3 exitPoint = texture(ExitPositions, TexCoord).xyz;
	vec3 entryPoint = texture(EntryPositions, TexCoord).xyz;
	
	float entryDepth = texture(EntryPositions, TexCoord).w;
	float exitDepth = texture(ExitPositions, TexCoord).w;

	gl_FragDepth = entryDepth;
	
	if(entryDepth >= exitDepth) {
		gl_FragDepth = 1.0f;
		FragColor_entry = vec4(exitPoint, 0.0);
		FragColor_exit = vec4(exitPoint, 0.0);
		return;
	}

	bool isEntryClipped = false;
	if(isPlaneClipped) {
		for(int i = 0 ; i < numClipPlanes; i++) {
			vec3 PlaneNormal = clipPlanes[i].xyz;
			vec3 PlanePoint = vec3(0.5f) + PlaneNormal*(clipPlanes[i].w)*0.5f;
	
			vec3 dir = exitPoint - entryPoint;
			float len = length(dir);
			dir = dir/len;

			float distUnit = dot(dir, PlaneNormal);
			float dist;
			if(distUnit != 0.0)
				dist = (dot(PlaneNormal, PlanePoint) - dot(PlaneNormal, entryPoint)) / distUnit;
			else {
				dist = dot(PlanePoint - entryPoint, PlaneNormal);
				if(dist >= 0.0) {
					gl_FragDepth = 1.0f;
					FragColor_entry = vec4(exitPoint, 1.0);
					FragColor_exit = vec4(exitPoint, 0.0);
					return;
				}
			}

			bool isFrontFace = (distUnit >= 0.0) ? true : false;
			if(isFrontFace) {
				if(dist < len && dist > 0.0) {
					gl_FragDepth = entryDepth + (exitDepth - entryDepth)*(dist/len);

					//projection?? perspective?? ?? ???? ??? ??????? ????.. ??? ???? ??? ????
					if(isPerspective)  {
						gl_FragDepth += 0.15f;
					}

					entryDepth = gl_FragDepth;
					entryPoint = entryPoint + dist*dir;
					isEntryClipped = true;
				} else if( dist >= len) {
					gl_FragDepth = 1.0f;
					FragColor_entry = vec4(exitPoint, 1.0);
					FragColor_exit = vec4(exitPoint, 0.0);
					return;
				}
			} else {
				if( dist < len && dist > 0.0) {
					exitPoint = entryPoint + dir*dist;
					exitDepth = entryDepth + (exitDepth - entryDepth)*(dist/len);
				} else if( dist >= len) {
				} else {
					gl_FragDepth = 1.0f;
					FragColor_entry = vec4(exitPoint, 1.0);
					FragColor_exit = vec4(exitPoint, 0.0);
					return;
				}
			}
		}
	}

	float mask_clipped = 0.0f;
	if(isEntryClipped) {
		float currIntensity = (texture(VolumeTex, entryPoint).r*65535.0) + 0.5;

		float TFidx = currIntensity*TFxincrease;
		int TFidy = int(TFidx);

		vec2 TFid;
		TFid.x = TFidx - TFidy;
		TFid.y = (TFidy + 0.5)*TFyincrease;

		if(texture(TransferFunc, TFid).a > 0.0f)
			mask_clipped = 1.0f;
	}

	vec3 dir = exitPoint - entryPoint;
	float len = length(dir);
	dir = dir/len;

	if(isDepthFirstHit) {
		vec3 currPoint = entryPoint;
		vec3 deltaDir = dir*StepSize;
	
		vec3 firstHitPos = vec3(0.0);
		vec3 secondHitPos = vec3(0.0);
		GetHitPos(currPoint, deltaDir, StepSize, len, firstHitPos, secondHitPos);

		bool bFirstHit = (firstHitPos != vec3(0.0)) ? true : false;
		if(bFirstHit) {
			vec3 d = exitPoint - firstHitPos;
			float l = length(d);
			d = d/l;
		
			FragColor_entry = vec4(firstHitPos, mask_clipped);
			FragColor_exit = vec4(d, l);
		}
		
		bool bSecondHit = (secondHitPos != vec3(0.0)) ? true : false;	
		if(bSecondHit) {
			gl_FragDepth = entryDepth + (exitDepth - entryDepth)*(length(secondHitPos - entryPoint)/len);
		} else {
			gl_FragDepth = 1.0f;
		
			FragColor_entry = vec4(exitPoint, mask_clipped);
			FragColor_exit = vec4(dir, len);
		}

		float offset = 0.0f;
		if(isThisSecond || isPlaneClipped)
			offset = 0.001f;
			
		gl_FragDepth += offset;
	} else {
		FragColor_entry = vec4(entryPoint, mask_clipped);
		FragColor_exit = vec4(dir, len);
	}
}
