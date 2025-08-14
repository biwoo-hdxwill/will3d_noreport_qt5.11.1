#version 400

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

layout(location = 0) out vec4 FragColor;

in vec3 Position;
in vec3 Normal;


vec3 phongModelDiffAndSpec()
{
	vec3 n = Normal;
	if (!gl_FrontFacing) n = -n;
	vec3 s = normalize(vec3(Light.Position) - Position);
	vec3 v = normalize(-Position.xyz);
	vec3 r = reflect(-s, n);
	float sDotN = max(dot(s, n), 0.0);
	vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
	vec3 spec = vec3(0.0);
	if (sDotN > 0.0)
		spec = Light.Intensity * Material.Ks *
		pow(max(dot(r, v), 0.0), Material.Shininess);

	return diffuse + spec;
}

void main()
{
	vec3 ambient = Light.Intensity * Material.Ka;
	vec3 diffAndSpec = phongModelDiffAndSpec();
    vec3 color = clamp(ambient+diffAndSpec, 0.0f, 1.0f);
    FragColor = vec4(color, alpha);
}

