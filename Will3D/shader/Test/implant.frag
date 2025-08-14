#version 400

layout (location = 0) out vec4 FragColor;
layout (location = 1) out float implant_id;

in vec3 Position;
in vec3 Normal;

uniform struct LightInfo {
	vec4 Position;
	vec3 Intensity;
} Light;

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;

uniform float alpha;

uniform sampler3D VolumeTex;
uniform float id;

uniform bool isBoneDensity;
uniform bool isWire;

uniform int  tfOffset;
uniform vec3 meshColor;
uniform vec3 texelSize;

vec3 phongModelDiffAndSpec(vec3 kd, vec3 ks)
{
	vec3 n = Normal;
	if (!gl_FrontFacing) n = -n;
	vec3 s = normalize(vec3(Light.Position) - Position);
	vec3 v = normalize(-Position.xyz);
	vec3 r = reflect(-s, n);
	float sDotN = max(dot(s, n), 0.0);
	vec3 diffuse = Light.Intensity * kd * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = Light.Intensity * ks *
		pow(max(dot(r, v), 0.0), Material.Shininess);

	return diffuse + spec;
}

vec3 getBoneDensityColor(float intensity)
{
	float intensityAppliedTF = intensity + tfOffset;

	vec3 boneColor = vec3(0.0);

	float D1_range = 1000;
	float D2_range = 1250 - 850;
	float D3_range = 850 - 350;
	float D4_range = 350 - 150;
	float D5_range = 1150;

	if (intensityAppliedTF >= 1250)//D1 �Ķ��� -> ���
	{
		float BlueWhite = (intensityAppliedTF - 1250) / D1_range;
		boneColor = vec3(BlueWhite, BlueWhite, 1.0);
	}
	else if (intensityAppliedTF >= 850 && intensityAppliedTF < 1250)//D2 ��� -> �Ķ���
	{
		float GreenBlue = (intensityAppliedTF - 850) / D2_range;
		boneColor = vec3(0.0, 1.0 - GreenBlue, GreenBlue);
	}
	else if (intensityAppliedTF >= 350 && intensityAppliedTF < 850)//D3 ����� -> ���
	{
		float YellowGreen = (intensityAppliedTF - 350) / D3_range;
		boneColor = vec3(YellowGreen, 1.0, 0.5);
	}
	else if (intensityAppliedTF >= 150 && intensityAppliedTF < 350)//D4 ������-> �����
	{
		float RedYellow = (intensityAppliedTF - 150) / D4_range;
		boneColor = vec3(1.0, RedYellow, 0.5);
	}
	else if (intensityAppliedTF < 150) //D5 ������->������
	{
		float BlackRed = (max(0.0, intensityAppliedTF) + 1150) / D5_range;
		boneColor = vec3(BlackRed, 0.0, 0.0);
	}

	return boneColor;
}

float getVolumeIntensity(vec3 position)
{
	position.x = (position.x > 1.0) ? 1.0 : (position.x < 0.0) ? 0.0 : position.x;
	position.y = (position.y > 1.0) ? 1.0 : (position.y < 0.0) ? 0.0 : position.y;
	position.z = (position.z > 1.0) ? 1.0 : (position.z < 0.0) ? 0.0 : position.z;

	return texture(VolumeTex, position).r*65535.0 + 0.5;
}

vec3 getBoneDensityAreaAverageColor(vec3 position)
{
	vec3 offset = (texelSize)*2.0;
	
	vec3 offsetX = vec3(offset.x, 0.0, 0.0);
	float intensityX = (getVolumeIntensity(position + offsetX) + getVolumeIntensity(position - offsetX))*0.5;

	vec3 offsetY = vec3(0.0, offset.y, 0.0);
	float intensityY = (getVolumeIntensity(position + offsetY) + getVolumeIntensity(position - offsetY))*0.5;
	
	vec3 offsetZ = vec3(0.0, 0.0, offset.z);
	float intensityZ = (getVolumeIntensity(position + offsetZ) + getVolumeIntensity(position - offsetZ))*0.5;

	float avgIntensity = (intensityX + intensityY + intensityZ) / 3.0;

	return getBoneDensityColor(avgIntensity);
}

void main()
{
	if(isWire)
	{
		FragColor = vec4(meshColor, 1.0);
	}
	else if(isBoneDensity)
	{
		vec3 color = getBoneDensityAreaAverageColor(Position);

		vec3 n = Normal;
		if (!gl_FrontFacing) n = -n;
		vec3 s = normalize(vec3(Light.Position) - Position);
		
		//diffuse
		float diffuseCoefficient = max(0.0, dot(n, s));
		vec3 diffuse = diffuseCoefficient * color * Light.Intensity;

		FragColor = vec4(diffuse, alpha); 

	}
	else
	{
		vec3 ambient = Light.Intensity * Material.Ka;
		vec3 diffAndSpec = phongModelDiffAndSpec(Material.Kd, Material.Ks);

		vec3 color = clamp(ambient + diffAndSpec, 0.0, 1.0);
		FragColor = vec4(color, alpha);

		//FragColor = vec4((diffAndSpec + ambient)*alpha, alpha);
	}

	implant_id = id;
}