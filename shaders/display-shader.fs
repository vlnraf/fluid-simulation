#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureIn;
uniform sampler2D textureIn2;
uniform int mode;

out vec4 FragColor;

void main()
{
    if(mode == 0){
        float v = texture(textureIn, TexCoord).r;
        FragColor = vec4(v, v, v, 1.0);
    }else if(mode == 1){
        float v = texture(textureIn, TexCoord).r;
        if(v <= 0){
            FragColor = vec4(0,0,1,1);
        }else{
            FragColor = vec4(1,0,0,1);
        }
    }
    //vec3 v = texture(textureIn, TexCoord).rgb;
    //FragColor = vec4(v, 1.0);
}
