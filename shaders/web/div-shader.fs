#version 300 es
precision mediump float;

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureVx;
uniform sampler2D textureVy;
uniform vec2 textureSize;      // cell-centered grid size (640, 320)
uniform float dt;

out float FragColor;

void main()
{
    vec2 uv = TexCoord;
    ivec2 ij = ivec2(uv * textureSize);

    // Divergence = u[i+1,j] - u[i,j] + v[i,j+1] - v[i,j]
    // u-texture is (textureSize.x+1) x textureSize.y, so u-face i maps directly
    // v-texture is textureSize.x x (textureSize.y+1), so v-face j maps directly
    float leftFlow   = texelFetch(textureVx, ivec2(ij.x,     ij.y), 0).r;
    float rightFlow  = texelFetch(textureVx, ivec2(ij.x + 1, ij.y), 0).r;
    float bottomFlow = texelFetch(textureVy, ivec2(ij.x, ij.y    ), 0).r;
    float topFlow    = texelFetch(textureVy, ivec2(ij.x, ij.y + 1), 0).r;

    FragColor = rightFlow - leftFlow + topFlow - bottomFlow;
}
