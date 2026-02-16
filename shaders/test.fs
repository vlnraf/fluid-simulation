#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureIn;
uniform vec2 texelSize;  // UV offset to sample neighboring pixels
uniform float dt;
uniform float diffusion;

out vec4 FragColor;

void main()
{
    vec4 c = texture(textureIn, TexCoord);
    vec4 l = texture(textureIn, TexCoord + vec2(-texelSize.x, 0.0));
    vec4 r = texture(textureIn, TexCoord + vec2( texelSize.x, 0.0));
    vec4 u = texture(textureIn, TexCoord + vec2(0.0,  texelSize.y));
    vec4 d = texture(textureIn, TexCoord + vec2(0.0, -texelSize.y));

    // Discrete Laplacian with h=1 (one pixel = one grid cell)
    vec4 laplacian = (l + r + u + d) - 4.0 * c;

    // Forward Euler: c_new = c + D * dt * laplacian
    vec4 result = c + diffusion * dt * laplacian;

    // For single-channel (R32F) textures, spread red to all channels for grayscale
    FragColor = vec4(result.r, result.r, result.r, 1.0);
}
