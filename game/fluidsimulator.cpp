#include "fluidsimulator.hpp"

int ij(FluidSimulator* fs, int i, int j){
    if(i< 0) i = 0;
    if(i > fs->gridSizeX - 1) i = fs->gridSizeX - 1;
    if(j< 0) j = 0;
    if(j > fs->gridSizeY - 1) j = fs->gridSizeY - 1;

    return i + fs->gridSizeX * j;
}

int uij(FluidSimulator* fs, int i, int j){
    if(i< 0) i = 0;
    if(i > fs->gridSizeX) i = fs->gridSizeX;
    if(j< 0) j = 0;
    if(j > fs->gridSizeY - 1) j = fs->gridSizeY - 1;

    return i + ((fs->gridSizeX + 1) * j);
}

int vij(FluidSimulator* fs, int i, int j){
    if(i< 0) i = 0;
    if(i > fs->gridSizeX - 1) i = fs->gridSizeX - 1;
    if(j< 0) j = 0;
    if(j > fs->gridSizeY) j = fs->gridSizeY;

    return i + (fs->gridSizeX  * j);
}

FluidSimulator createFluidSimulator(Arena* a, int countX, int countY, int cellSize){
    FluidSimulator fs = {};
    fs.u = arenaAllocArrayZero(a, float, (countX+1) * countY);
    fs.v = arenaAllocArrayZero(a, float, countX * (countY+1));
    fs.uTemp = arenaAllocArrayZero(a, float, (countX+1) * countY);
    fs.vTemp = arenaAllocArrayZero(a, float, countX * (countY+1));
    fs.p = arenaAllocArrayZero(a, float, countX * countY);
    fs.dens = arenaAllocArrayZero(a, float, countX * countY);
    fs.densTemp = arenaAllocArrayZero(a, float, countX * countY);
    fs.density = 1.0f;
    fs.gridSizeX = countX;
    fs.gridSizeY = countY;
    fs.cellSize = cellSize;
    fs.a = a;

    return fs;
}

float calculateDivergence(FluidSimulator* fs, int i, int j){
    float leftFlow   = fs->u[uij(fs, i, j)];
    float rigthFlow  = fs->u[uij(fs, i+1, j)];
    float bottomFlow = fs->v[vij(fs, i, j)];
    float topFlow    = fs->v[vij(fs, i, j+1)];
    float result     = rigthFlow - leftFlow + topFlow - bottomFlow;
    return result;
}

void advectVel(FluidSimulator* fs, float dt){
    memcpy(fs->uTemp, fs->u, sizeof(float) * (fs->gridSizeX + 1) * fs->gridSizeY);
    memcpy(fs->vTemp, fs->v, sizeof(float) * fs->gridSizeX * (fs->gridSizeY + 1));

    for(int j = 0; j < fs->gridSizeY; j++){
        for(int i = 0; i < fs->gridSizeX; i++){
            float vy = fs->vTemp[vij(fs, i, j)];
            //interpolate the horizontal velocity to the v-face at (i+0.5, j)
            float ux = (fs->uTemp[uij(fs, i, j-1)] + fs->uTemp[uij(fs, i+1, j-1)] + fs->uTemp[uij(fs, i, j)] + fs->uTemp[uij(fs, i+1, j)]) / 4;

            float x = i - (dt * ux); //advect new position on x axis
            float y = j - (dt * vy); //advect new position on y axis

            x = glm::clamp(x, 0.0f, (float)fs->gridSizeX);
            y = glm::clamp(y, 0.5f, (float)fs->gridSizeY);

            //bilinear interpolation
            int i0 = x, i1 = i0 + 1; //TODO: make sure that it's inside the grid???
            int j0 = y, j1 = j0 + 1; //TODO: make sure that it's inside the grid???
            float a = x - i0;
            float b = y - j0;
            float fj0j0 = (1 - a) * fs->vTemp[vij(fs, i0, j0)] + a * fs->vTemp[vij(fs, i1, j0)];
            float fj1j1 = (1 - a) * fs->vTemp[vij(fs, i0, j1)] + a * fs->vTemp[vij(fs, i1, j1)];
            fs->v[vij(fs, i, j)] = (1-b) * fj0j0 + b * fj1j1;
            //LOGINFO("%f", fs->v[vij(fs, i , j)]);
        }
    }

    for(int j = 0; j < fs->gridSizeY; j++){
        for(int i = 0; i < fs->gridSizeX; i++){
            float ux = fs->uTemp[uij(fs, i, j)];
            //interpolate the vertical velocity to the u-face at (i, j+0.5)
            float vy = (fs->vTemp[vij(fs, i-1, j)] + fs->vTemp[vij(fs, i, j)] + fs->vTemp[vij(fs, i-1, j+1)] + fs->vTemp[vij(fs, i, j+1)]) / 4;

            float x = i - (dt * ux); //advect new position on x axis
            float y = j - (dt * vy); //advect new position on y axis

            x = glm::clamp(x, 0.0f, (float)fs->gridSizeX);
            y = glm::clamp(y, 0.5f, (float)fs->gridSizeY);

            //bilinear interpolation
            int i0 = x, i1 = x + 1; //TODO: make sure that it's inside the grid???
            int j0 = y, j1 = y + 1; //TODO: make sure that it's inside the grid???
            float a = x - i0;
            float b = y - j0;
            float fj0j0 = (1 - a) * fs->uTemp[uij(fs, i0, j0)] + a * fs->uTemp[uij(fs, i1, j0)];
            float fj1j1 = (1 - a) * fs->uTemp[uij(fs, i0, j1)] + a * fs->uTemp[uij(fs, i1, j1)];
            fs->u[uij(fs, i, j)] = (1-b) * fj0j0 + b * fj1j1;
            //LOGINFO("%f", fs->u[uij(fs, i , j)]);
        }
    }

    //for(int i = 0; i < fs->gridSizeY; i++){
    //    for(int j = 0; j < fs->gridSizeX; j++){
    //        fs->u[uij(fs, i, j)] = fs->uTemp[uij(fs, i, j)];
    //        fs->v[vij(fs, i, j)] = fs->vTemp[vij(fs, i, j)];
    //    }
    //}
}

void pressureSolver(FluidSimulator*fs, float dt){
    //for(int j = 0; j < fs->gridSizeY; j++){
    //    for(int i = 0 ; i < fs->gridSizeX; i++){
    //        fs->p[ij(fs,i,j)] = 0.0f;
    //    }
    //}

    for(int k = 0; k < 4; k++){
        for(int j = 0; j < fs->gridSizeY; j++){
            for(int i = 0 ; i < fs->gridSizeX; i++){
                float div = calculateDivergence(fs, i, j);
                float s = 4; //TODO: change when adding solids
                float pLeft  = (i > 0) ? fs->p[ij(fs,i-1,j)] : 0.0f;
                float pRight = (i < fs->gridSizeX-1) ? fs->p[ij(fs,i+1,j)] : 0.0f;
                float pBot   = (j > 0) ? fs->p[ij(fs,i,j-1)] : 0.0f;
                float pTop   = (j < fs->gridSizeY-1) ? fs->p[ij(fs,i,j+1)] : 0.0f;
                float result = pRight + pLeft + pTop + pBot;
                fs->p[ij(fs,i,j)] = (result - fs->density * div) / s;
            }
        }
    }

    for(int j = 0; j < fs->gridSizeY; j++){
        for(int i = 0 ; i <= fs->gridSizeX; i++){
            float pRight = (i < fs->gridSizeX) ? fs->p[ij(fs, i, j)]   : 0.0f;
            float pLeft  = (i > 0)             ? fs->p[ij(fs, i-1, j)] : 0.0f;
            fs->u[uij(fs, i, j)] -= (pRight - pLeft) / fs->density;
        }
    }
    for(int j = 0; j <= fs->gridSizeY; j++){
        for(int i = 0 ; i < fs->gridSizeX; i++){
            float pTop    = (j < fs->gridSizeY) ? fs->p[ij(fs, i, j)]   : 0.0f;
            float pBottom = (j > 0)             ? fs->p[ij(fs, i, j-1)] : 0.0f;
            fs->v[vij(fs, i, j)] -= (pTop - pBottom) / fs->density;
        }
    }
}

void advectDensity(FluidSimulator* fs, float dt){
    memcpy(fs->densTemp, fs->dens, sizeof(float) * fs->gridSizeX * fs->gridSizeY);
    for(int j = 0; j < fs->gridSizeY; j++){
        for(int i = 0; i < fs->gridSizeX; i++){
            float ux = (fs->u[uij(fs, i, j)] + fs->u[uij(fs, i+1, j)]) / 2;
            float vy = (fs->v[vij(fs, i, j)] + fs->v[vij(fs, i, j+1)]) / 2;

            float x = i - (dt * ux); //advect new position on x axis
            float y = j - (dt * vy); //advect new position on y axis

            x = glm::clamp(x, 0.0f, (float)fs->gridSizeX-1);
            y = glm::clamp(y, 0.0f, (float)fs->gridSizeY-1);

            //bilinear interpolation
            int i0 = x, i1 = i0 + 1; //TODO: make sure that it's inside the grid???
            int j0 = y, j1 = j0 + 1; //TODO: make sure that it's inside the grid???
            float a = x - i0;
            float b = y - j0;
            float fj0j0 = (1 - a) * fs->densTemp[ij(fs, i0, j0)] + a * fs->densTemp[ij(fs, i1, j0)];
            float fj1j1 = (1 - a) * fs->densTemp[ij(fs, i0, j1)] + a * fs->densTemp[ij(fs, i1, j1)];
            fs->dens[ij(fs, i, j)] = (1-b) * fj0j0 + b * fj1j1;
            //LOGINFO("%f", fs->v[vij(fs, i , j)]);
        }
    }
}

void applyVelocityBoundaries(FluidSimulator* fs){
    for(int j=0;j<fs->gridSizeY;j++){
        fs->u[uij(fs,0,j)] = 0;
        fs->u[uij(fs,fs->gridSizeX,j)] = 0;
        //fs->u[uij(fs,0,j)] = fs->u[uij(fs,1,j)];
        //fs->u[uij(fs,fs->gridSizeX,j)] = fs->u[uij(fs,fs->gridSizeX-1,j)];
    }
    for(int i=0;i<fs->gridSizeX;i++){
        fs->v[vij(fs,i,0)] = 0;
        fs->v[vij(fs,i,fs->gridSizeY)] = 0;
    }
}

void step(FluidSimulator* fs, float dt){
    advectVel(fs, dt);
    applyVelocityBoundaries(fs);
    pressureSolver(fs, dt);
    applyVelocityBoundaries(fs);
    advectDensity(fs, dt);
}

//Draw functions

void drawGrid(FluidSimulator* fs){
    float xo=-fs->gridSizeX * fs->cellSize * 0.5f;
    float yo=-fs->gridSizeY * fs->cellSize * 0.5f;
    for(int j=0;j<fs->gridSizeY;j++){
        for(int i=0;i<fs->gridSizeX;i++){
            float cx = xo + i*fs->cellSize;
            float cy = yo + j*fs->cellSize;
            renderDrawRect({cx, cy}, {fs->cellSize, fs->cellSize}, {1.0f, 0.0f, 0.0f, 1.0f});
            //renderDrawLine({cx,cy},{cx+5, cy},{0,0,1,1},1);
        }
    }

}

void drawVelocities(FluidSimulator* fs){
    float xo=-fs->gridSizeX * fs->cellSize * 0.5f;
    float yo=-fs->gridSizeY * fs->cellSize * 0.5f;
    for(int j=0;j<fs->gridSizeY;j++){
        for(int i=0;i<fs->gridSizeX;i++){
            float cx = xo + (i+0.5f)*fs->cellSize;
            float cy = yo + (j+0.5f)*fs->cellSize;
            float ux = 0.5f*(fs->u[uij(fs, i,j)] + fs->u[uij(fs, i+1,j)]);
            float vy = 0.5f*(fs->v[vij(fs, i,j)] + fs->v[vij(fs, i,j+1)]);
            renderDrawLine({cx,cy},{cx+ux*fs->cellSize, cy+vy*fs->cellSize},{0,0,1,1},1);
            //renderDrawLine({cx,cy},{cx+5, cy},{0,0,1,1},1);
        }
    }
}

void drawDivergence(FluidSimulator* fs){
    Font* f = getFont("Roboto-Regular");
    TempArena tmp = getTempArena(fs->a);
    float scale = ((float)fs->cellSize - (fs->cellSize / 2 + 1)) / (float)f->characterSize;
    float xo=-fs->gridSizeX * fs->cellSize * 0.5f;
    float yo=-fs->gridSizeY * fs->cellSize * 0.5f;
    for(int j=0;j<fs->gridSizeY;j++){
        for(int i=0;i<fs->gridSizeX;i++){
            float div = calculateDivergence(fs, i, j);
            String8 t = cStringFromString8(tmp.arena, pushString8F(tmp.arena, "%.2f", div));
            float textWidth = calculateTextWidth(f, t.str, scale);
            float textHeight = calculateTextHeight(f, t.str, scale);

            float cx = xo + (fs->cellSize * 0.5f) + (i*fs->cellSize) - (textWidth * 0.5f);
            float cy = yo + (fs->cellSize * 0.5f) + (j*fs->cellSize) - (textHeight * 0.5f);
            //renderDrawText3D(f, t.str, {cx, cy, 0}, scale);
            renderDrawText2D(f, t.str, {cx, cy}, scale);
            //renderDrawLine({cx,cy},{cx+5, cy},{0,0,1,1},1);
        }
    }
    releaseTempArena(tmp);
}

void drawDensity(FluidSimulator* fs){
    float xo=-fs->gridSizeX * fs->cellSize * 0.5f;
    float yo=-fs->gridSizeY * fs->cellSize * 0.5f;
    for(int j=0;j<fs->gridSizeY;j++){
        for(int i=0;i<fs->gridSizeX;i++){
            float v = fs->dens[ij(fs,i,j)];
            renderDrawFilledRect({xo+i*fs->cellSize, yo+j*fs->cellSize}, {fs->cellSize, fs->cellSize}, 0, {v,v,v,1});
        }
    }
}