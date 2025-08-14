#version 400

layout(location = 0) out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D EntryPositions;
uniform sampler2D ExitPositions;

uniform sampler3D VolumeTex;
uniform sampler2D TransferFunc;
uniform float     StepSize;

uniform int		  MaxTexSize;
uniform vec3	  VolTexelSize;
uniform bool	  isShade;
uniform bool	  isFixedColor;
uniform int		  FixedColorIdx;
uniform mat4	  BMVP;
uniform bool	  isMIP;
uniform	bool	  isXRAY;
uniform vec3	  InvVolTexScale;
uniform float	  WindowLevel;
uniform float	  WindowWidth;

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;

float endDepth = 0;

vec2 GetTFIndex(float curr_intensity)
{
	ivec2 tf_texture_size = textureSize(TransferFunc, 0);
	float tf_x_increase = 1.0 / tf_texture_size.x;
	float tf_y_increase = 1.0 / tf_texture_size.y;

	float tf_x_index = curr_intensity * tf_x_increase;
	int tf_y_index = int(tf_x_index);
	return vec2(tf_x_index - tf_y_index, (tf_y_index + 0.5) * tf_y_increase);
}

float GetIntensity(vec3 pos) { return texture(VolumeTex, pos).r; }

vec3 GetGradient(vec3 pos)
{
	vec3 offset = (VolTexelSize);

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
	float GdotL = max(dot(G, L), 0.0);
	return kd* GdotL;
}

vec3 getSpecularColor(in vec3 ks, in vec3 G, in vec3 L)
{
	vec3 H = normalize(L);
	float GdotH = pow(max(dot(G, H), 0.0), 15.0);
	return ks * GdotH;
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
	return shadedColor;
}

vec4 raycasting(vec3 currPoint, vec4 dir_len, float deltaX)
{
	float lengthAccum = 0.0;
	vec3 deltaDir = (dir_len.xyz)*deltaX;
	vec3 lightDir = dir_len.xyz;
	vec4 realColor;
	vec2 TFidx;
	float currIntensity = 0.0;
	float len = dir_len.w;

	vec3 grad;
	vec3 shade;
	vec4 accumColor = vec4(0.0);

	float window_min = (WindowLevel - WindowWidth * 0.5) / 65535.0;
	float window_width = WindowWidth / 65535.0;

	for (; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0 + 0.5);
		realColor = texture(TransferFunc, GetTFIndex(currIntensity));

		if (realColor.a > 0.0)
		{
			// Heuristic!!! Why is this good???
			realColor.a = 1.0 - pow(1.0 - realColor.a, StepSize*200.0f);

			realColor.rgb = ((realColor.rgb - window_min) / window_width)*realColor.a;

			accumColor = accumColor + (1.0 - accumColor.a)*realColor;
		}

		if (accumColor.a > 0.9)
		{
			accumColor.a = 1.0;
			vec3 glV0 = vec3(BMVP*vec4(currPoint, 1.0));
			vec3 glV1 = vec3(BMVP*vec4(currPoint + dir_len.xyz*lengthAccum, 1.0));
			endDepth += abs(glV1.z - glV0.z)*0.5;
			break;
		}

		currPoint += deltaDir;
	}

	return accumColor;
}

vec4 shadeRaycasting(vec3 currPoint, vec4 dir_len, float deltaX)
{
	float lengthAccum = 0.0;
	vec3 deltaDir = (dir_len.xyz)*deltaX;
	vec3 lightDir = (dir_len.xyz);
	vec4 realColor;
	vec2 TFidx;
	float currIntensity = 0.0;
	float len = dir_len.w;

	vec3 grad;
	vec3 shade;
	vec4 accumColor = vec4(0.0);

	currIntensity = (texture(VolumeTex, currPoint).r*65535.0 + 0.5);
	realColor = texture(TransferFunc, GetTFIndex(currIntensity));
	float window_min = (WindowLevel - WindowWidth * 0.5) / 65535.0;
	float window_width = WindowWidth / 65535.0;

	if (realColor.a > 0.0)
	{
		realColor.rgb = ((realColor.rgb - window_min) / window_width)*realColor.a;
		accumColor = accumColor + (1.0 - accumColor.a)*realColor;
		lengthAccum += deltaX;
		currPoint += deltaDir;
	}

	for (; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0 + 0.5);//*TFxincrease;
		realColor = texture(TransferFunc, GetTFIndex(currIntensity));

		if (realColor.a > 0.0)
		{
			// Heuristic!!! Why is this good???
			realColor.a = 1.0 - pow(1.0 - realColor.a, StepSize*350.0f);

			grad = normalize(GetGradient(currPoint));

			shade = phongShading(grad, realColor.xyz*0.73f, vec3(0.12f), realColor.xyz*0.35f, lightDir);

			shade.rgb = ((shade.rgb - window_min) / window_width)*realColor.a;
			accumColor = accumColor + (1.0 - accumColor.a)*vec4(shade, realColor.a);
		}

		if (accumColor.a > 0.9)
		{
			accumColor.a = 1.0;
			vec3 glV0 = vec3(BMVP*vec4(currPoint, 1.0));
			vec3 glV1 = vec3(BMVP*vec4(currPoint + dir_len.xyz*lengthAccum, 1.0));
			endDepth += abs(glV1.z - glV0.z)*0.5;
			break;
		}

		currPoint += deltaDir;
	}

	return accumColor;
}

vec4 mipRaycasting(vec3 currPoint, vec4 dir_len, float deltaX)
{
	float lengthAccum = 0.0;
	vec3 deltaDir = (dir_len.xyz)*deltaX;
	vec3 lightDir = dir_len.xyz;
	vec4 realColor;
	vec2 TFidx;
	float currIntensity = 0.0;
	float len = dir_len.w;

	float maxIntensity = 0.0;
	float WindowMin = (WindowLevel - WindowWidth * 0.5) / 65535.0;
	for (; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0 + 0.5);

		if (currIntensity > maxIntensity)
		{
			maxIntensity = currIntensity;
		}

		currPoint += deltaDir;
	}

	maxIntensity = maxIntensity / 65535.0f;
	maxIntensity = (maxIntensity - WindowMin) / (WindowWidth / 65535.0f);

	return vec4(maxIntensity, maxIntensity, maxIntensity, maxIntensity);
}

vec4 XRaycasting(vec3 currPoint, vec4 dir_len, float deltaX)
{
	float lengthAccum = 0.0;
	vec3 deltaDir = (dir_len.xyz)*deltaX;
	float currIntensity = 0.0;
	float len = dir_len.w;
	float sumIntensity = 0.0;
	int sum_cnt = 0;

	float window_min = (WindowLevel - WindowWidth * 0.5) / 65535.0;
	float window_width = WindowWidth / 65535.0;
	float window_level = WindowLevel / 65535.0;

	for (; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0) + 0.5;
		currIntensity = currIntensity / 65535.0f;
		currIntensity = exp((currIntensity - window_min) / (window_width))*currIntensity;
		currIntensity = (currIntensity - window_min) / (window_width);
		currIntensity = max(0.0f, currIntensity);
		sumIntensity += currIntensity;
		sum_cnt++;
		currPoint += deltaDir;
	}

	float avgIntensity = sumIntensity / (sum_cnt);
	avgIntensity = clamp(avgIntensity, 0.0f, 1.0f);

	return vec4(avgIntensity, avgIntensity, avgIntensity, avgIntensity);
}

vec4 fixedColorRaycasting(vec3 currPoint, vec4 dir_len, float deltaX)
{
	float lengthAccum = 0.0;
	vec3 deltaDir = (dir_len.xyz)*deltaX;
	vec3 lightDir = (dir_len.xyz);
	vec4 realColor;
	vec2 TFidx;
	float currIntensity = 0.0;
	float len = dir_len.w;

	vec3 grad;
	vec3 shade;
	vec4 accumColor = vec4(0.0);
	float window_min = (WindowLevel - WindowWidth * 0.5) / 65535.0;
	float window_width = WindowWidth / 65535.0;

	vec3 color;
	if (FixedColorIdx == 0)
		color = vec3(1.0f, 0.1f, 0.03f);
	else if (FixedColorIdx == 1)
		color = vec3(0.09f, 0.01f, 0.9f);
	else if (FixedColorIdx == 2)
		color = vec3(0.0f, 0.65f, 0.31f);

	for (; lengthAccum < len; lengthAccum += deltaX)
	{
		currIntensity = (texture(VolumeTex, currPoint).r*65535.0 + 0.5);//*TFxincrease;
		realColor = texture(TransferFunc, GetTFIndex(currIntensity));

		if (realColor.a > 0.0)
		{
			// Heuristic!!! Why is this good???
			realColor.a = 1.0 - pow(1.0 - realColor.a, StepSize*200.0f);

			grad = normalize(GetGradient(currPoint));
			//shade = phongShading(grad, realColor.xyz*0.73f, vec3(0.12f), realColor.xyz*0.35f, lightDir);
			shade = phongShading(grad, color*0.73f, vec3(0.8f), color*0.55f, lightDir);

			shade.rgb = ((shade.rgb - window_min) / window_width)*realColor.a;

			accumColor = accumColor + (1.0 - accumColor.a)*vec4(shade, realColor.a);
		}

		if (accumColor.a > 0.9)
		{
			accumColor.a = 1.0;
			vec3 glV0 = vec3(BMVP*vec4(currPoint, 1.0));
			vec3 glV1 = vec3(BMVP*vec4(currPoint + dir_len.xyz*lengthAccum, 1.0));
			endDepth += abs(glV1.z - glV0.z)*0.5;
			return accumColor;
		}

		currPoint += deltaDir;
	}

	return accumColor;
}
vec4 calcDirection(const vec4 entryPoint, const vec4 exitPoint)
{
	vec3 dir = exitPoint.xyz - entryPoint.xyz;

	float len;

	if (dir == vec3(0.0f))
		len = 0.0f;
	else
	{
		len = length(dir);
		dir = normalize(dir);
	}

	return vec4(dir.x, dir.y, dir.z, len);
}

subroutine(RenderPassType)
void cuttingRayCasting()
{
}

subroutine(RenderPassType)
void surgeryRayCasting()
{
}

subroutine(RenderPassType)
void basicRayCasting()
{
	vec4 entryPoint = texture(EntryPositions, TexCoord);
	vec4 exitPoint = texture(ExitPositions, TexCoord);
	//vec4 dir_len = texture(RayDirections, TexCoord);
	vec4 dir_len = calcDirection(entryPoint, exitPoint);

	float deltaX = StepSize*pow(dot(dir_len.xyz*InvVolTexScale, dir_len.xyz), 0.8f);

	vec3 currPoint = entryPoint.xyz;
	vec4 accumColor = vec4(0.0); // The dest color

	if (entryPoint.w < exitPoint.w && dir_len.w > StepSize)
	{
		if (isFixedColor)
			accumColor += fixedColorRaycasting(currPoint, dir_len, deltaX);
		else if (isMIP)
			accumColor += mipRaycasting(currPoint, dir_len, deltaX);
		else if (isXRAY)
			accumColor += XRaycasting(currPoint, dir_len, deltaX);
		else if (isShade)
			accumColor += shadeRaycasting(currPoint, dir_len, deltaX);
		else
			accumColor += raycasting(currPoint, dir_len, deltaX);
	}
	else
		discard;

	if (endDepth == 0.0)
	{
		endDepth = 0.999; //depth clear 1.0
	}
	else
	{
		endDepth += entryPoint.w;
	}

	gl_FragDepth = endDepth;

	FragColor = accumColor;
}

void main()
{
	RenderPass();
}