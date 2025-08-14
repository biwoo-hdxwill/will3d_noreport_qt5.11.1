#version 400




in vec3 Normal;
in vec2 TexC;
in float depth;

uniform struct MaterialInfo {
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
} Material;

uniform bool isTexture;
uniform sampler2D FACEtexture;
uniform float	Alpha;

layout (location = 0) out vec4 FragColor;

void main()
{
	//FragColor = vec4(1.0);
	if(isTexture)
	{
		vec3 Norm = normalize(Normal);
		vec3 lightDir = vec3(0.0, 0.0, -1.0); // camera �� ���ϴ� ����
		float diffuse = 0.0;
		float spec = 0.0;
		vec3 baseColor = texture(FACEtexture, TexC).rgb;

		FragColor = vec4(baseColor*Alpha, depth);

	}
	else
	{	


		FragColor = vec4(Material.Ka, depth);

	}

	
}