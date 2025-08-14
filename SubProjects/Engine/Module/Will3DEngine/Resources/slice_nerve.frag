#version 400

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;

layout(location = 0) out vec4 FragColor;

in vec3 Position;
in float color_alpha;

void main()
{
	if(color_alpha < 0.000001)
		discard;

	FragColor = vec4(vec3(Material.Kd), color_alpha);
}

