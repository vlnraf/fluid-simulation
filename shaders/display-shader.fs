#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureIn;

out vec4 FragColor;

void main()
{
    float v = texture(textureIn, TexCoord).r;
    FragColor = vec4(v, v, v, 1.0);
}
