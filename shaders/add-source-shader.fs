#version 330 core

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform vec2 textureSize;      
uniform vec2 mousePos;
uniform vec2 mousePosPrev;
uniform vec2 screenSize;
uniform float radius;
uniform int mode;

//out float FragColor;
out vec4 FragColor;

void main(){
    //float radius = 5;
    vec2 uv = TexCoord;
    ivec2 ij = ivec2(uv * TexCoord);
    vec2 mouseDelta = mousePos - mousePosPrev;
    float force = 0.2;
    if(mode == 0){
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }else if(mode == 1){
        //FragColor = mouseDelta.x * force;
        float v = float(mouseDelta.x * force);
        FragColor = vec4(v, 0, 0, 1);
    }else if (mode == 2){
        //FragColor = mouseDelta.y * force;
        float v = float(mouseDelta.y * force);
        FragColor = vec4(v, 0, 0, 1);
    }else if (mode == 3){
        FragColor = vec4(1.0, 0, 0, 1);
    }else if(mode == 4){
        FragColor = vec4(5.0, 0, 0, 1);
    }
}