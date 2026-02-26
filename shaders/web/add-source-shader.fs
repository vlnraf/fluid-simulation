#version 300 es
precision mediump float;

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform vec2 textureSize;      
uniform vec2 mousePos;
uniform vec2 mousePosPrev;
uniform vec2 screenSize;
uniform int mode;

out float FragColor;

void main(){
    float radius = 5.0;
    vec2 uv = TexCoord;
    ivec2 ij = ivec2(uv * TexCoord);
    vec2 mouseDelta = mousePos - mousePosPrev;
    float force = 0.5;
    if(mode == 0){
        FragColor = 1.0;
    }else if(mode == 1){
        FragColor = mouseDelta.x * force;
    }else if (mode == 2){
        FragColor = mouseDelta.y * force;
    }else if(mode == 3){
        FragColor = 1.0;
    }else if(mode == 4){
        FragColor = 5.0;
    }
}