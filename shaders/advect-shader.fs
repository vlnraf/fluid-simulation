#version 330

in vec4 OutColor;
in vec2 TexCoord;
flat in int TexIndex;

uniform sampler2D textureVx;
uniform sampler2D textureVy;
uniform sampler2D pTexture;
uniform sampler2D densTexture;
uniform vec2 textureSize;      // size of the texture being advected (self)
uniform vec2 textureSizeOther; // size of the ross-component texture (or pressure size for projection)
uniform float dt;
uniform int mode;

//layout(binding = 0) uniform sampler2D sprite[16];
//uniform sampler2D sprite[16];

out float FragColor;

int isBoundary(ivec2 ij){
    if(ij.x == 0 || ij.x >= (textureSize.x - 1)){
        return 2;
    }
    if(ij.y == 0 || ij.y >= (textureSize.y - 1)){
        return 1;
    }

    float radius = 50;
    vec2 pos = vec2(200, textureSize.y / 2);
    if(length(ij - pos) <= radius){
        return 1;
    }

    //if((ij.x >= (pos-radius).x && ij.x <= (pos+radius).x) && (ij.y >= (pos-radius).y && ij.y <= (pos+radius).y)){
    //    return true;
    //}
    return 0;
}


// Interpolate u to a v-face location (called from mode 1, samples textureVx)
// CPU: u[i,j-1], u[i+1,j-1], u[i,j], u[i+1,j]
float getHorizontalVelocity(ivec2 ij){
    ivec2 current  = ij;                    // (i, j)
    ivec2 right    = ij + ivec2(1, 0);      // (i+1, j)
    ivec2 bottom   = ij + ivec2(0, -1);     // (i, j-1)
    ivec2 botRight = ij + ivec2(1, -1);     // (i+1, j-1)

    if(bottom.y < 0){
        bottom.y = 0;
    }
    if(botRight.y < 0){
        botRight.y = 0;
    }
    if(right.x >= textureSizeOther.x){
        right.x = int(textureSizeOther.x) - 1;
    }
    if(botRight.x >= textureSizeOther.x){
        botRight.x = int(textureSizeOther.x) - 1;
    }

    float velocityX = (texelFetch(textureVx, current, 0).r + texelFetch(textureVx, right, 0).r +
                texelFetch(textureVx, bottom, 0).r + texelFetch(textureVx, botRight, 0).r) * 0.25;

    return velocityX;

}

// Interpolate v to a u-face location (called from mode 0, samples textureVy)
// CPU: v[i-1,j], v[i,j], v[i-1,j+1], v[i,j+1]
float getVerticalVelocity(ivec2 ij){
    ivec2 top     = ij + ivec2(0,1);
    ivec2 topLeft = ij + ivec2(-1,1);
    ivec2 botLeft = ij + ivec2(-1,0);
    ivec2 current = ij;
    if(top.y >= textureSizeOther.y){
        top.y = int(textureSizeOther.y) - 1;
    }

    if(topLeft.x < 0){
        topLeft.x = 0;
    }
    if(topLeft.y >= textureSizeOther.y){
        topLeft.y = int(textureSizeOther.y) - 1;
    }

    if(botLeft.x < 0){
        botLeft.x = 0;
    }


    float velocityY = (texelFetch(textureVy, current, 0).r + texelFetch(textureVy, botLeft, 0).r +
                    texelFetch(textureVy, topLeft, 0).r + texelFetch(textureVy, top, 0).r) * 0.25;

    return velocityY;

}

void main()
{
    //FragColor = 5.0;
    if(mode == 0){          //horizontal advection
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);
        float velocityX = texelFetch(textureVx, ij, 0).r;
        
        //float velocityY = (texelFetch(textureVy, ij, 0).g + texelFetchOffset(textureVy, ij, 0, ivec2(-1,0)).g +
        //            texelFetchOffset(textureVy, ij, 0, ivec2(-1,1)).g + texelFetchOffset(textureVy, ij, 0, ivec2(0, 1)).g) * 0.25;
        float velocityY = getVerticalVelocity(ij);
        
        float x = float(ij.x) - dt * velocityX;
        float y = float(ij.y) - dt * velocityY;

        x = clamp(x, 0.5, textureSize.x-0.1);
        y = clamp(y, 0, textureSize.y-0.1);
        
        //bilinear interpolation;
        int i0 = int(x); int i1 = i0+1;
        int j0 = int(y); int j1 = j0+1;
        float a = x - float(i0);
        float b = y - float(j0);

        float i0i1 = mix(texelFetch(textureVx, ivec2(i0,j0), 0).r, texelFetch(textureVx, ivec2(i1,j0), 0).r, a);
        float j0j1 = mix(texelFetch(textureVx, ivec2(i0,j1), 0).r, texelFetch(textureVx, ivec2(i1,j1), 0).r, a);
        float res = mix(i0i1, j0j1, b);
        //isBoundary(ij) ? FragColor = 0 : FragColor = res;
        //(isBoundary(ij) == 1) ? FragColor = 0 : FragColor = res;
        if(isBoundary(ij) == 0){
            FragColor = res;
        }else if (isBoundary(ij) == 1){
            FragColor = 0;
        }else if(isBoundary(ij) == 2 && ij.x == 0){
            FragColor = texelFetch(textureVx, ivec2(ij.x + 1, ij.y), 0).r;
        }else if(isBoundary(ij) == 2 && ij.x >= textureSize.x-1){
            FragColor = texelFetch(textureVx, ivec2(ij.x - 1, ij.y), 0).r;
        }
        //FragColor = res;
        //FragColor = velocityX;
    }else if(mode == 1){    //vertical advection
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);
        float velocityY = texelFetch(textureVy, ij, 0).r;
        
        //float velocityX = (texelFetch(textureVx, ij, 0).r + texelFetchOffset(textureVx, ij, 0, ivec2(-1,0)).r +
        //            texelFetchOffset(textureVx, ij, 0, ivec2(-1,1)).r + texelFetchOffset(textureVx, ij, 0, ivec2(0, 1)).r) * 0.25;

        float velocityX = getHorizontalVelocity(ij);
        
        float x = float(ij.x) - dt * velocityX;
        float y = float(ij.y) - dt * velocityY;

        x = clamp(x, 0, textureSize.x-0.1);
        y = clamp(y, 0.5, textureSize.y-0.1);
        
        //bilinear interpolation;
        int i0 = int(x); int i1 = i0+1;
        int j0 = int(y); int j1 = j0+1;
        float a = x - float(i0);
        float b = y - float(j0);

        float i0i1 = mix(texelFetch(textureVy, ivec2(i0,j0), 0).r, texelFetch(textureVy, ivec2(i1,j0), 0).r, a);
        float j0j1 = mix(texelFetch(textureVy, ivec2(i0,j1), 0).r, texelFetch(textureVy, ivec2(i1,j1), 0).r, a);
        float res = mix(i0i1, j0j1, b);
        //(isBoundary(ij) == 1) ? FragColor = 0 : FragColor = res;
        if(isBoundary(ij) == 0){
            FragColor = res;
        }else if (isBoundary(ij) == 1){
            FragColor = 0;
        }else if(isBoundary(ij) == 2 && ij.y == 0){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y + 1), 0).r;
        }else if(isBoundary(ij) == 2 && ij.y >= textureSize.y-1){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y - 1), 0).r;
        }
        //FragColor = res;
        //FragColor = velocityY;
    }else if(mode == 2){
        // Passthrough: output vertex color's red channel
        FragColor = OutColor.r;
    }else if(mode == 3){
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);
        float velocityX = texelFetch(textureVx, ij, 0).r + 10;
        FragColor = velocityX;
    }else if(mode == 4){
        // Project u: u[i,j] -= p[i,j] - p[i-1,j]
        // textureSize = u-texture size (641, 320)
        // textureSizeOther = pressure size (640, 320)
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);

        // Boundary u-faces (i=0 and i=640) — skip, keep as-is
        if(ij.x <= 0 || ij.x >= int(textureSizeOther.x)){
            FragColor = texelFetch(textureVx, ij, 0).r;
        } else {
            // Interior u-face at i: pressure cells are (i-1) and (i)
            float pRight = texelFetch(pTexture, ivec2(ij.x, ij.y), 0).r;
            float pLeft  = texelFetch(pTexture, ivec2(ij.x - 1, ij.y), 0).r;
            FragColor = texelFetch(textureVx, ij, 0).r - (pRight - pLeft);
        }
        if (isBoundary(ij) == 1){
            FragColor = 0;
        }else if(isBoundary(ij) == 2 && ij.y == 0){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y + 1), 0).r;
        }else if(isBoundary(ij) == 2 && ij.y >= textureSize.y-1){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y - 1), 0).r;
        }
    }else if(mode == 5){
        // Project v: v[i,j] -= p[i,j] - p[i,j-1]
        // textureSize = v-texture size (640, 321)
        // textureSizeOther = pressure size (640, 320)
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);

        // Boundary v-faces (j=0 and j=320) — skip, keep as-is
        if(ij.y <= 0 || ij.y >= int(textureSizeOther.y)){
            FragColor = texelFetch(textureVy, ij, 0).r;
        } else {
            // Interior v-face at j: pressure cells are (j-1) and (j)
            float pTop    = texelFetch(pTexture, ivec2(ij.x, ij.y), 0).r;
            float pBottom = texelFetch(pTexture, ivec2(ij.x, ij.y - 1), 0).r;
            FragColor = texelFetch(textureVy, ij, 0).r - (pTop - pBottom);
        }
        if (isBoundary(ij) == 1){
            FragColor = 0;
        }else if(isBoundary(ij) == 2 && ij.y == 0){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y + 1), 0).r;
        }else if(isBoundary(ij) == 2 && ij.y >= textureSize.y-1){
            FragColor = texelFetch(textureVy, ivec2(ij.x, ij.y - 1), 0).r;
        }
    }else if(mode == 6){
        // Advect density (cell-centered)
        // textureSize = density size (640, 320)
        // textureVx = u velocity (641 x 320), textureVy = v velocity (640 x 321)
        vec2 uv = TexCoord;
        ivec2 ij = ivec2(uv * textureSize);

        // Interpolate velocity to cell center (i+0.5, j+0.5)
        // Average u-faces: u[i,j] and u[i+1,j]
        float ux = (texelFetch(textureVx, ivec2(ij.x, ij.y), 0).r +
                    texelFetch(textureVx, ivec2(ij.x + 1, ij.y), 0).r) * 0.5;
        // Average v-faces: v[i,j] and v[i,j+1]
        float vy = (texelFetch(textureVy, ivec2(ij.x, ij.y), 0).r +
                    texelFetch(textureVy, ivec2(ij.x, ij.y + 1), 0).r) * 0.5;

        // Backtrace
        float x = float(ij.x) - dt * ux;
        float y = float(ij.y) - dt * vy;

        x = clamp(x, 0.0, textureSize.x - 1.001);
        y = clamp(y, 0.0, textureSize.y - 1.001);

        // Bilinear interpolation
        int i0 = int(x); int i1 = i0 + 1;
        int j0 = int(y); int j1 = j0 + 1;
        float a = x - float(i0);
        float b = y - float(j0);

        float row0 = mix(texelFetch(densTexture, ivec2(i0, j0), 0).r,
                         texelFetch(densTexture, ivec2(i1, j0), 0).r, a);
        float row1 = mix(texelFetch(densTexture, ivec2(i0, j1), 0).r,
                         texelFetch(densTexture, ivec2(i1, j1), 0).r, a);
        float res = mix(row0, row1, b);
        //isBoundary(ij) ? FragColor = 0 : FragColor = res;
        //(isBoundary(ij) == 1) ? FragColor = 0 : FragColor = res;
        if(isBoundary(ij) == 0){
            FragColor = res;
        }else if (isBoundary(ij) == 1){
            FragColor = 0;
        }else if(isBoundary(ij) == 2 && ij.x == 0){
            FragColor = texelFetch(densTexture, ivec2(ij.x+1, ij.y), 0).r;
        }else if(isBoundary(ij) == 2 && ij.x >= textureSize.x-1){
            FragColor = texelFetch(densTexture, ivec2(ij.x-1, ij.y), 0).r;
        }else if(isBoundary(ij) == 2 && ij.y == 0){
            FragColor = texelFetch(densTexture, ivec2(ij.x, ij.y+1), 0).r;
        }else if(isBoundary(ij) == 2 && ij.y >= textureSize.y-1){
            FragColor = texelFetch(densTexture, ivec2(ij.x, ij.y-1), 0).r;
        }
        //FragColor = mix(row0, row1, b);
    }
}