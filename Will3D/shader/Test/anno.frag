#version 400

uniform vec4 Color;
uniform float PlaneDepth;

layout(location = 0) out vec4 FragColor;

in float depth;

void main()
{
	vec4 finalColor = Color;

	///////////////////////////////////////////////////////////////////////////
	//
	//	PlaneDepth ���� depth�� ũ��(�ڿ� ������) ���� ��ä������ �����ϰ�
	//	blending�� ���� ���İ��� ����
	//	
	///////////////////////////////////////////////////////////////////////////
	if (depth > PlaneDepth + 0.001)
	{
		finalColor.rgb = vec3(1.0f, 1.0f, 1.0f);
		finalColor.a *= 0.5f;
	}

	FragColor = finalColor;
}
