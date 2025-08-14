#version 400

uniform sampler2D ExitPositions;
uniform sampler2D EntryPositions;

in vec2 TexCoord;

void main()
{
	vec4 exit = texture(ExitPositions, TexCoord);	
	vec4 entry = texture(EntryPositions, TexCoord);	

	if(exit == entry)
	{
		gl_FragDepth = 0.0;
	}
	else
	{
		gl_FragDepth = 1.0;
	}
}