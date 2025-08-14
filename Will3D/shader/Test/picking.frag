#version 400

uniform int Index;

layout(location = 0) out vec4 FragColor;

void main()
{
	///////////////////////////////////////////////////////////////////////////
	//
	//	picking�� ���� �� object�� ������ ���� FragColor�� ���� �ٸ� index�� �Ҵ��ϰ�
	//	readpixel�� color�� �о� �� object�� ����
	//	
	///////////////////////////////////////////////////////////////////////////
	FragColor = vec4(Index / 255.0f);
}
