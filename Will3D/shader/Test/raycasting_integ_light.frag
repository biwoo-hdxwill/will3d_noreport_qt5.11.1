#version 400

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;
uniform sampler3D VolumeTex;
uniform sampler2D TransferFunc;
uniform sampler2D MeshDrawn;
uniform vec2 ScreenSize;
uniform int	MinValue;
uniform vec3 invVoltexScale;
uniform float TFxincrease;
uniform float TFyincrease;
uniform bool isShading;
uniform bool isMIP;
uniform float WindowMin;
uniform float WindowWidth;
uniform float MaxValue;
uniform float StepSize;
uniform vec3 texelSize;
uniform mat4 matFirstHitToDepth;
uniform mat4 BMVP;

uniform bool isFrontAndBack;
uniform bool isDepthFirstHit;
uniform bool isMeshSeparated;
uniform float opacity;

layout(location = 0) out vec4 FragColor;

vec2 GetTFIndex(float curr_intensity)
{
	float TFidx = curr_intensity * TFxincrease;
	int TFidy = int(TFidx);
	return vec2(TFidx - TFidy, (TFidy + 0.5)*TFyincrease);
}

float GetIntensity(vec3 pos)
{
	return texture(VolumeTex, pos).r;
}

vec3 GetGradient(vec3 pos)
{
	vec3 offset = (texelSize);

	float x0 = GetIntensity(pos + vec3(offset.x, 0.0, 0.0));
	float x1 = GetIntensity(pos + vec3(-offset.x, 0.0, 0.0));
	float x = x0 - x1;

	float y0 = GetIntensity(pos + vec3(0.0, offset.y, 0.0));
	float y1 = GetIntensity(pos + vec3(0.0, -offset.y, 0.0));
	float y = y0 - y1;

	float z0 = GetIntensity(pos + vec3(0.0, 0.0, offset.z));
	float z1 = GetIntensity(pos + vec3(0.0, 0.0, -offset.z));
	float z = z0 - z1;

	return vec3(x, y, z);
}

vec3 getDiffuseColor(in vec3 kd, in vec3 G, in vec3 L)
{
	return kd * max(dot(G, L), 0.0);
}

vec3 getSpecularColor(in vec3 ks, in vec3 G, in vec3 L)
{
	vec3 H = normalize(L);
	return ks * pow(max(dot(G, H), 0.0), 15.0);
}

vec3 getAmbientColor(in vec3 ka)
{
	return ka;
}

vec3 phongShading(in vec3 G, in vec3 kd, in vec3 ks, in vec3 ka, vec3 lightDir)
{
	vec3 shadedColor = vec3(0.0);
	shadedColor += getDiffuseColor(kd, G, lightDir);
	shadedColor += getSpecularColor(ks, G, lightDir);
	shadedColor += getAmbientColor(ka);
	return clamp(shadedColor, vec3(0.0), vec3(1.0));
}

void main()
{
	gl_FragDepth = isDepthFirstHit ? 1.0 : gl_FragCoord.z;

	vec4 MeshValue;
	if (isMeshSeparated)
		MeshValue = texture(MeshDrawn, TexCoord);

	vec4 entryPoint = texture(EntryPositions, TexCoord);
	vec4 exitPoint = texture(ExitPositions, TexCoord);
	vec3 dir = exitPoint.xyz;//exitPoint.xyz - entryPoint.xyz;
	float len = exitPoint.w;//length(dir);
	if (isFrontAndBack)
	{
		if (entryPoint.xyz == exitPoint.xyz)
		{
			dir = vec3(0.0);
			len = 0.0;
		}
		else
		{
			dir = exitPoint.xyz - entryPoint.xyz;
			len = length(dir);
			dir = dir / len;
		}
	}
	vec4 dir_len = vec4(dir, len);

	if (len == 0.0)
	{
		if (isMeshSeparated && MeshValue.w > 0.0)
		{
			FragColor = MeshValue * opacity;
			return;
		}
		discard;
	}

	float scalarDirTexScale = dot(dir*invVoltexScale, dir);
	scalarDirTexScale = (scalarDirTexScale < 1.0) ? 1.0 : scalarDirTexScale;

	float unitStep = StepSize * pow(scalarDirTexScale, 0.8);
	float deltaX = isMIP ? unitStep * 8.0 : unitStep;
	deltaX = unitStep;
	vec3 deltaDir = dir * deltaX;

	float lengthAccum = 0.0;
	bool isBlendedMesh = false;
	vec3 firstHitPos;
	vec3 currPoint = entryPoint.xyz;
	//////////////////////////////////////////
	//// MIP
	if (isMIP)
	{
		float currIntensity = 0.0, maxIntensity = 0.0;
		for (; lengthAccum < len; lengthAccum += deltaX)
		{
			currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;

			if (currIntensity > maxIntensity)
				maxIntensity = currIntensity;

			currPoint += deltaDir;
		}

		maxIntensity = maxIntensity / 65535.0;
		maxIntensity = (maxIntensity - WindowMin) / (WindowWidth);

		FragColor = vec4(maxIntensity, maxIntensity, maxIntensity, 1.0);
	}
	else
	{
		float currIntensity = 0.0;

		vec4 currColor = vec4(0.0);
		vec4 accumColor = vec4(0.0); // The dest color

		bool startFlag = false;
		if (isShading)
		{
			vec3 lightDir = normalize(dir);
			vec3 grad, shade;

			for (; lengthAccum < len; lengthAccum += deltaX)
			{
				if (isMeshSeparated && !isBlendedMesh)
				{
					float curr_depth = (BMVP*vec4(currPoint, 1.0)).z*0.5 + 0.5;
					float mesh_depth = MeshValue.w;

					if (mesh_depth < curr_depth && mesh_depth > 0.0)
					{
						accumColor = accumColor + (1.0 - accumColor.a)*vec4(MeshValue.xyz, opacity);
						isBlendedMesh = true;
					}
				}

				currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
				currColor = texture(TransferFunc, GetTFIndex(currIntensity));

				if (currColor.a > 0.0)
				{
					// Heuristic!!! Why is this good???
					currColor.a = 1.0 - pow(1.0 - currColor.a, StepSize*350.0);
					grad = normalize(GetGradient(currPoint));
					shade = phongShading(grad, currColor.xyz*0.73, vec3(0.12), currColor.xyz*0.35, lightDir);
					shade.rgb *= currColor.a;

					accumColor = accumColor + (1.0 - accumColor.a)*vec4(shade, currColor.a);

					if (!startFlag)
					{
						startFlag = true;

						if (isDepthFirstHit)
							firstHitPos = currPoint + deltaDir * 10.0;
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
			for (; lengthAccum < len; lengthAccum += deltaX)
			{
				if (isMeshSeparated && !isBlendedMesh)
				{
					float curr_depth = (BMVP*vec4(currPoint, 1.0)).z*0.5 + 0.5;
					float mesh_depth = MeshValue.w;

					if (mesh_depth < curr_depth && mesh_depth > 0.0)
					{
						accumColor = accumColor + (1.0 - accumColor.a)*vec4(MeshValue.xyz, opacity);
						isBlendedMesh = true;
					}
				}

				currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
				currColor = texture(TransferFunc, GetTFIndex(currIntensity));

				if (currColor.a > 0.0)
				{
					// Heuristic!!! Why is this good???
					currColor.a = 1.0 - pow(1.0 - currColor.a, StepSize*200.0);
					currColor.rgb *= currColor.a;
					accumColor = accumColor + (1.0 - accumColor.a)*currColor;

					if (!startFlag)
					{
						startFlag = true;

						if (isDepthFirstHit)
							firstHitPos = currPoint + deltaDir * 10.0;
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

		if (isMeshSeparated && !isBlendedMesh && MeshValue.w > 0.0)
			accumColor = accumColor + (1.0 - accumColor.a)*vec4(MeshValue.xyz, opacity);

		FragColor = accumColor;

		if (isDepthFirstHit)
		{
			vec4 depth = matFirstHitToDepth * vec4(firstHitPos, 1.0);
			gl_FragDepth = depth.z;
		}
	}

	//FragColor = entryPoint;
	//FragColor = exitPoint;
}