#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D divTexture;
uniform sampler2D pPrev;
uniform vec2 textureSize;
uniform float dt;
uniform int boundaryMode;

out float FragColor;

int isBoundary(ivec2 ij){
    if(ij.x == 0 || ij.x >= (textureSize.x - 1)){
        return 2;
    }
    if(ij.y == 0 || ij.y >= (textureSize.y - 1)){
        return 1;
    }

    if(boundaryMode == 1){
        float radius = 50;
        vec2 pos = vec2(200, textureSize.y / 2);
        if(length(ij - pos) <= radius){
            return 1;
        }
    }

    return 0;
}

ivec2 clampij(ivec2 coord){
    coord.x = clamp(coord.x, 0, int(textureSize.x) - 1);
    coord.y = clamp(coord.y, 0, int(textureSize.y) - 1);
    return coord;
}

void main()
{
    vec2 uv = TexCoord;
    ivec2 ij = ivec2(uv * textureSize);
    //if(isBoundary(ij) == 1 || isBoundary(ij) == 2){
    //    FragColor = 0;
    //}
    float left   = texelFetch(pPrev, clampij(ij + ivec2(-1, 0)), 0).r;
    float right  = texelFetch(pPrev, clampij(ij + ivec2( 1, 0)), 0).r;
    float top    = texelFetch(pPrev, clampij(ij + ivec2( 0, 1)), 0).r;
    float bottom = texelFetch(pPrev, clampij(ij + ivec2( 0,-1)), 0).r;
    float d = texelFetch(divTexture, ij, 0).r;
    float sum = left + right + top + bottom;
    FragColor = (sum - d) / 4.0;
}
