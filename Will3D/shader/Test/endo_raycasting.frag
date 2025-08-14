#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;//
uniform sampler2D ExitPositions;//
uniform sampler2D RayCastingForSecond;//
uniform sampler3D VolumeTex;//
uniform sampler2D TransferFunc;  //
uniform sampler2D MeshDrawn;
uniform vec2      ScreenSize; //
uniform int		  MinValue; //
uniform vec3	  invVoltexScale;
uniform float	  TFxincrease;//
uniform float	  TFyincrease;//
uniform bool	  isShading;//
uniform bool	  isMIP;//
uniform float	  MaxValue;//
uniform float     AttenDistance;//
uniform float     StepSize;  //
uniform vec3      texelSize;//
uniform mat4		matFirstHitToDepth;
uniform bool		isThereSecondVolume;
uniform bool		isThisSecondVolume;

uniform bool		isFrontAndBack;
uniform bool		isDepthFirstHit;
uniform bool		isMeshSeparated;
uniform float		opacity;		
//uniform float	  gradMin;
//uniform float     gradMax;

layout (location = 0) out vec4 FragColor;

vec2 textureIdxLookup3D(vec3 voxPos)
{
	float intensity =  (texture(VolumeTex, voxPos).r*65535.0) + 0.5;

	float TFidx = intensity *TFxincrease;
	float TFidy = int(TFidx);
	vec2 TFid;
	TFid.x = TFidx - TFidy;
	TFid.y = (TFidy + 0.5)*TFyincrease;
	return TFid;
}

vec3 GetGradient(vec3 pos)
{
	vec3 offset = (texelSize)*2.0f;

	float x, y, z;
	
	float x0 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(offset.x, 0.0, 0.0))).a;
	float x1 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(-offset.x, 0.0, 0.0))).a;
	x = x0 - x1;
	
	float y0 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(0.0, offset.y, 0.0))).a;
	float y1 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(0.0, -offset.y, 0.0))).a;
	y = y0 - y1;
	
	float z0 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(0.0, 0.0, offset.z))).a;
	float z1 = texture(TransferFunc, textureIdxLookup3D(pos + vec3(0.0, 0.0, -offset.z))).a;
	z = z0 - z1;
	
	if(x == 0.0)
		x = texture(TransferFunc, textureIdxLookup3D(pos)).a - x1;
	if(y == 0.0)
		y = texture(TransferFunc, textureIdxLookup3D(pos)).a - y1;
	if(z == 0.0)
		z = texture(TransferFunc, textureIdxLookup3D(pos)).a - z1;
	
	vec3 gradient = vec3(x, y, z);
	return gradient;
}

vec3 getDiffuseColor(in vec3 kd, in vec3 G, in vec3 L) {
	float GdotL = max(dot(G, L), 0.0);
	return kd* GdotL;
}
vec3 getSpecularColor(in vec3 ks, in vec3 G, in vec3 L) {
	vec3 H = normalize(L);
	float GdotH = pow(max(dot(G, H), 0.0), 15.0);
	return ks * GdotH;
}
vec3 getAmbientColor(in vec3 ka) {
	return ka;
}

vec3 phongShading(in vec3 G, in vec3 kd, in vec3 ks, in vec3 ka, vec3 lightDir) {
	vec3 shadedColor = vec3(0.0);
	shadedColor += getDiffuseColor(kd, G, lightDir);
	shadedColor += getSpecularColor(ks, G, lightDir);
	shadedColor += getAmbientColor(ka);
	return shadedColor;
}


void main()
{
	/*FragColor = texture(ExitPositions, TexCoord);
	return;*/

	if(isDepthFirstHit)
	{
		gl_FragDepth = 1.0;
	}
	else
	{
		gl_FragDepth = gl_FragCoord.z;
	}
	

	vec4 entryPoint = texture(EntryPositions, TexCoord);
	vec3 firstHitPos;
	

	vec4 exitPoint = texture(ExitPositions, TexCoord);
	
	vec4 MeshValue;
	if(isMeshSeparated)
	{
		MeshValue = texture(MeshDrawn, TexCoord);
	}
	
	vec3 dir = exitPoint.xyz;//exitPoint.xyz - entryPoint.xyz;
	float len = exitPoint.w;//length(dir);

	if(isFrontAndBack)
	{
		if(entryPoint.xyz == exitPoint.xyz)
		{
			dir = vec3(0.0);
			len = 0.0;
		}
		else
		{
			dir = exitPoint.xyz - entryPoint.xyz;
			len = length(dir);
			dir = dir/len;
		}

	}

	// by jdk 151215 boolean operation
	//dir = normalize(dir);
	//dir = dir / length(dir);

	vec4 dir_len = vec4(dir, len);

	if(isThereSecondVolume)
	{
		vec2 TexCoord2 = TexCoord;
		TexCoord2.y = 1.0 - TexCoord2.y;
		vec4 resultForSecond = texture(RayCastingForSecond, TexCoord2);

		if(resultForSecond.w > 0.3)
		{
			FragColor = vec4(resultForSecond.xyz, 1.0);
			if(isMeshSeparated && MeshValue.w == 1.0)
			{
				FragColor = FragColor*(1.0f - opacity) + MeshValue*opacity;
			}
			return;

		}
		else
		{
			if(entryPoint.w == 0.0)
			{
				if(isMeshSeparated && MeshValue.w == 1.0)
				{
					FragColor = MeshValue*opacity;
					return;
				}
				discard;
			}
			if(len == 0.0)
			{
				if(isMeshSeparated && MeshValue.w == 1.0)
				{
					FragColor = MeshValue*opacity;
					return;
				}
				discard;
			}
		}
	}
	else
	{
		if(entryPoint.w == 0.0)
		{
			if(isMeshSeparated && MeshValue.w == 1.0)
			{
				FragColor = MeshValue*opacity;
				return;
			}
			discard;
		}
		if(len == 0.0)
		{
			if(isMeshSeparated && MeshValue.w == 1.0)
			{
				FragColor = MeshValue*opacity;
				return;
			}
			discard;
		}
	}

	float deltaX;
	
	float scalarDirTexScale = dot(dir*invVoltexScale, dir);
	scalarDirTexScale = (scalarDirTexScale < 1.0f) ? 1.0f : scalarDirTexScale;

	float unitStep = StepSize*pow(scalarDirTexScale, 0.8f);

	if(isMIP)
		deltaX = unitStep*4.0f;
	else
		deltaX = unitStep;
	

	vec3 deltaDir = dir*deltaX;
	
	float lengthAccum = 0.0;

    vec3 currPoint = entryPoint.xyz;

	//////////////////////////////////////////
	//// MIP
	if(isMIP)
	{
		float currIntensity = 0.0;
		float maxIntensity = 0.0;

		vec4 currColor = vec4(0.0);

		vec2 TFid;
		float TFidx;
		int TFidy;

		for(; lengthAccum < len; lengthAccum += deltaX)
		{
			currIntensity =  (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
			//TFidx = currIntensity*TFxincrease;
			//TFidy = int(TFidx);
			//
			//TFid.x = TFidx - TFidy;
			//TFid.y = (TFidy + 0.5)*TFyincrease;
			//
			//currColor = texture(TransferFunc, TFid);

			//if(currColor.a > 0.0) 
			{
				if(currIntensity > maxIntensity)
				{
					maxIntensity = currIntensity;	
				}	
			}

			currPoint += deltaDir;
		}

		maxIntensity = maxIntensity/MaxValue;//(32768.0f - MinValue);

		FragColor = vec4(maxIntensity, maxIntensity, maxIntensity, 1.0);
	}
	else
	{
		float currIntensity = 0.0;    
		vec4 currColor = vec4(0.0); 
		vec4 realColor = vec4(0.0);
		vec4 accumColor = vec4(0.0); // The dest color
		vec2 TFid;
		vec2 prevMinMax, currMinMax; 
	
		float TFidx;
		int TFidy;

		bool startFlag = false;
		bool isPass = false;
		
		if(isShading)
		{	
			vec3 lightDir = normalize(dir);
			vec3 grad;
			vec3 shade;
			// seg tmj mask	by jdk 160830
			currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;

			TFidx = currIntensity*TFxincrease;
			TFidy = int(TFidx);

			TFid.x = TFidx - TFidy;
			TFid.y = (TFidy + 0.5)*TFyincrease;

			currColor = texture(TransferFunc, TFid);

			if (currColor.a > 0.0)
			{
				currColor.rgb *= currColor.a;
				
				grad = normalize(GetGradient(currPoint));
				shade = phongShading(grad, currColor.xyz*0.73f, vec3(0.12f), currColor.xyz*0.35f, lightDir);
				shade.rgb *= currColor.a;
				
				if(length(currColor.rgb - shade.rgb) > 0.5f)
				{
					accumColor = accumColor + (1.0 - accumColor.a)*currColor;
				
					lengthAccum += deltaX;
					currPoint += deltaDir;
				}

				isPass = true;
			}
			else
			{
				isPass = false;
			}

			if (isPass)
			{
				if (!startFlag)
				{
					startFlag = true;

					if (isDepthFirstHit)
					{
						firstHitPos = currPoint + deltaDir * 10.0;
					}
				}
			}

			for(; lengthAccum < len; lengthAccum += deltaX)
			{	
				currIntensity =  (texture(VolumeTex, currPoint).r*65535.0) + 0.5;

				TFidx = currIntensity*TFxincrease;
				TFidy = int(TFidx);

				TFid.x = TFidx - TFidy;
				TFid.y = (TFidy + 0.5)*TFyincrease;

				currColor = texture(TransferFunc, TFid);

				if (currColor.a > 0.0)
				{
					// Heuristic!!! Why is this good???
					currColor.a = 1.0 - pow(1.0 - currColor.a, StepSize*350.0f);

					grad = normalize(GetGradient(currPoint));

					shade = phongShading(grad, currColor.xyz*0.73f, vec3(0.12f), currColor.xyz*0.35f, lightDir);

					shade.rgb *= currColor.a;
					
					accumColor = accumColor + (1.0 - accumColor.a)*vec4(shade, currColor.a);

					isPass = true;
				}
				else
				{
					isPass = false;
				}

				if (isPass)
				{
					if (!startFlag)
					{
						startFlag = true;

						if (isDepthFirstHit)
						{
							firstHitPos = currPoint + deltaDir * 10.0;
						}
					}
				}

				if (accumColor.a > 0.9)
				{
					accumColor.a = 1.0;
					break;
				}

				currPoint += deltaDir;
			}			
		}
		else
		{
			currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;

			TFidx = currIntensity*TFxincrease;
			TFidy = int(TFidx);

			TFid.x = TFidx - TFidy;
			TFid.y = (TFidy + 0.5)*TFyincrease;

			currColor = texture(TransferFunc, TFid);

			if (currColor.a > 0.0)
			{
				currColor.rgb *= currColor.a;
				accumColor = accumColor + (1.0 - accumColor.a)*currColor;

				isPass = true;

			}
			else
			{
				isPass = false;
			}
			
			if (isPass)
			{
				if (!startFlag)
				{
					startFlag = true;

					if (isDepthFirstHit)
					{
						firstHitPos = currPoint + deltaDir*10.0;
					}
				}

				realColor.a = (1.0 - exp(-realColor.a));

				accumColor = accumColor + (1.0 - accumColor.a)*realColor;
			}

			lengthAccum += deltaX;

			currPoint += deltaDir;

			for(; lengthAccum < len; lengthAccum += deltaX)
			{	
				currIntensity =  (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
				
				TFidx = currIntensity*TFxincrease;
				TFidy = int(TFidx);

				TFid.x = TFidx - TFidy;
				TFid.y = (TFidy + 0.5)*TFyincrease;

				currColor = texture(TransferFunc, TFid);
		
				if (currColor.a > 0.0)
				{
					// Heuristic!!! Why is this good???
					currColor.a = 1.0 - pow(1.0 - currColor.a, StepSize*200.0f);
			
					currColor.rgb *= currColor.a;
					
					accumColor = accumColor + (1.0 - accumColor.a)*currColor;
		
					isPass = true;

				}
				else
				{
					isPass = false;
				}
				
				if (isPass)
				{
					if (!startFlag)
					{
						startFlag = true;

						if (isDepthFirstHit)
						{
							firstHitPos = currPoint + deltaDir*10.0;
						}
					}

					realColor.a = (1.0 - exp(-realColor.a));

					accumColor = accumColor + (1.0 - accumColor.a)*realColor;
				}

				if (accumColor.a > 0.9)
				{
					accumColor.a = 1.0;
					break;
				}

				currPoint += deltaDir;
    		
			}
		}

		if(isThisSecondVolume)
		{
			//FragColor.xyza = accumColor.zyxa;
			FragColor = vec4(accumColor.b, accumColor.g, accumColor.r, accumColor.a);
		}
		else
		{
			FragColor = accumColor;
		}


		if(isMeshSeparated && MeshValue.w == 1.0)
		{
			FragColor = FragColor*(1.0f - opacity) + opacity*MeshValue;

		}

		if(isDepthFirstHit)
		{
			vec4 depth = matFirstHitToDepth*vec4(firstHitPos, 1.0);
			gl_FragDepth = depth.z;
		}
	}
}

