#version 400

layout(location = 0) out vec4 FragColor;

in vec3 Position;
in vec3 VolTexPosition;
in vec3 Normal;

uniform vec4 LightPosition;
uniform sampler3D VolumeTex;

uniform float  tfOffset;
uniform bool is_collide;
uniform vec3 texelSize;

float D1_range = 1000;
float D2_range = 1250 - 850;
float D3_range = 850 - 350;
float D4_range = 350 - 150;
float D5_range = 1150;

float D1 = 1250.0 + D1_range * 0.5;
float D2 = 850.0 + D2_range * 0.5;
float D3 = 350.0 + D3_range * 0.5;
float D4 = 150.0 + D4_range * 0.5;
float D5 = -1000.0 + D5_range * 0.5;

vec3 getBoneDensityColor(float intensity) {
	float intensityAppliedTF = intensity + tfOffset;

	vec3 boneColor = vec3(0.0);

	if (intensityAppliedTF >= D1) { // D1 blue -> white

		float BlueWhite = min(1.0, (intensityAppliedTF - D1) / (D1_range * 0.5));
		boneColor = vec3(BlueWhite, BlueWhite, 1.0);

	} else if (intensityAppliedTF >= D2 && intensityAppliedTF < D1) { // D2 green -> blue

		float GreenBlue = (intensityAppliedTF - D2) / ((D1_range + D2_range) * 0.5);
		boneColor = vec3(0.0, 1.0 - GreenBlue, GreenBlue);

	} else if (intensityAppliedTF >= D3 && intensityAppliedTF < D2) { // D3 yellow -> green

		float YellowGreen = (intensityAppliedTF - D3) / ((D2_range + D3_range) * 0.5);
		boneColor = vec3(YellowGreen, 1.0, 0.5);

	} else if (intensityAppliedTF >= D4 && intensityAppliedTF < D3) { // D4 orange -> yellow

		float OrangeYellow = (intensityAppliedTF - D4) / ((D3_range + D4_range) * 0.5);
		boneColor = vec3(1.0, OrangeYellow * 0.5 + 0.5, 0.0);

	} else if (intensityAppliedTF >= D5 && intensityAppliedTF < D4) { // D4 red -> orange

		float RedOrange = (intensityAppliedTF - D5) / ((D4_range + D5_range) * 0.5);
		boneColor = vec3(1.0, RedOrange * 0.5, 0.0);

	} else if (intensityAppliedTF < D5) { // D5 black -> red

		float minimun = D5 - D5_range * 0.5;
		float BlackRed = max(0.0, (intensityAppliedTF - minimun) / (D5_range * 0.5));
		boneColor = vec3(BlackRed, 0.0, 0.0);

	}

	return boneColor;
}

float getVolumeIntensity(vec3 position) {
	position.x = (position.x > 1.0) ? 1.0 : (position.x < 0.0) ? 0.0 : position.x;
	position.y = (position.y > 1.0) ? 1.0 : (position.y < 0.0) ? 0.0 : position.y;
	position.z = (position.z > 1.0) ? 1.0 : (position.z < 0.0) ? 0.0 : position.z;

	return texture(VolumeTex, position).r * 65535.0 + 0.5;
}

vec3 getBoneDensityAreaAverageColor(vec3 position) {
	vec3 offset = texelSize * 2.0;

	vec3 offsetX = vec3(offset.x, 0.0, 0.0);
	float intensityX = (getVolumeIntensity(position + offsetX) + getVolumeIntensity(position - offsetX)) * 0.5;

	vec3 offsetY = vec3(0.0, offset.y, 0.0);
	float intensityY = (getVolumeIntensity(position + offsetY) + getVolumeIntensity(position - offsetY)) * 0.5;

	vec3 offsetZ = vec3(0.0, 0.0, offset.z);
	float intensityZ = (getVolumeIntensity(position + offsetZ) + getVolumeIntensity(position - offsetZ)) * 0.5;

	float avgIntensity = (intensityX + intensityY + intensityZ) / 3.0;

	return getBoneDensityColor(avgIntensity);
}

void main() {
	vec3 color = getBoneDensityAreaAverageColor(VolTexPosition);

	vec3 n = Normal;
	if (!gl_FrontFacing) n = -n;
	vec3 s = normalize(vec3(LightPosition) - Position);

	//diffuse
	float diffuseCoefficient = max(0.0, dot(n, s));
	vec3 diffuse = diffuseCoefficient * color;

	//FragColor = vec4(color, 1.0);
	FragColor = vec4(diffuse, 1.0);
}