#version 400

layout(location = 0) in vec3 VerPos;

out vec2 TexCoord;

void main()
{
	TexCoord.x = (VerPos.x + 1.0)*0.5;
	TexCoord.y = (VerPos.y + 1.0f)*0.5;

    gl_Position = vec4(VerPos, 1.0);
}