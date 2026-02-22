#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureIn;
uniform sampler2D textureIn2;

out vec4 FragColor;

void main()
{
    float v = texture(textureIn, TexCoord).r;
    //vec3 v = texture(textureIn, TexCoord).rgb;
    FragColor = vec4(v, v, v, 1.0);
}
