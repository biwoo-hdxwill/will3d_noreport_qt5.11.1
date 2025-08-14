#version 400

layout(location = 0) in vec3 VerPos;

uniform mat4 MVP;

out float depth;

void main()
{
	gl_Position = MVP * vec4(VerPos, 1.0);
	depth = gl_Position.z / gl_Position.w;
}
