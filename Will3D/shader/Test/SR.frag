#version 400




in vec3 Normal;
in vec2 TexC;

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

		//diffuse = clamp(dot(Norm, lightDir), 0.0, 1.0);

		//baseColor = baseColor*pow(diffuse, 0.5)*3.0;// + 0.2*pow(diffuse, 100))*2.0;
		//baseColor = baseColor*(0.8*pow(diffuse, 0.5) + 0.2*pow(diffuse, 100))*2.0;

		//FragColor = vec4(baseColor, 1.0);


		FragColor = vec4(baseColor*Alpha, 1.0f);

		//FragColor = vec4(vec3(diffuse), 1.0f);
	}
	else
	{	
		vec3 Norm = normalize(Normal);
		vec3 lightDir = vec3(0.0, 0.0, -1.0);
		float diffuse = 0.0;
		float spec = 0.0;
		vec3 baseColor = vec3(1.0, 1.0, 1.0);


		diffuse = clamp(dot(Norm, lightDir), 0.0, 1.0);

		//baseColor *= (0.8*diffuse + 0.2*pow(diffuse, 100));

		baseColor *= (diffuse);


		FragColor = vec4(baseColor*Alpha, 1.0);

	}

	
}