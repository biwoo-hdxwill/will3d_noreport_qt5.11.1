#version 400

in vec2 TexCoord;
uniform sampler2D image_texture;

layout (location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(image_texture, TexCoord);
}