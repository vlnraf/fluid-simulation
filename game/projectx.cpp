#include "projectx.hpp"

#define GRAVITY -9.8f
#define GRID_SIZE 50
#define CELL_SIZE 5

#define IX(i,j) ((i) + GRID_SIZE * (j))
#define SWAP(x0,x) {float* tmp = x0; x0 = x; x = tmp;}

inline int clampI(int i){ return i < 0 ? 0 : (i >= GRID_SIZE ? GRID_SIZE-1 : i); }
inline int clampJ(int j){ return j < 0 ? 0 : (j >= GRID_SIZE ? GRID_SIZE-1 : j); }
#define IXC(i,j) IX(clampI(i), clampJ(j))

glm::vec4 clampColor(glm::vec4 color){
    return glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
}

void drawGrid(float* grid){
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f);

    for(int y = 0; y < GRID_SIZE; y++){
        for(int x = 0; x < GRID_SIZE; x++){
            float xx = xo + (x * CELL_SIZE);// + (CELL_SIZE);
            float yy = yo + (y * CELL_SIZE);// + (CELL_SIZE);
            renderDrawRect({xx,yy}, {CELL_SIZE, CELL_SIZE}, {1,0,1,1}, 1.0f);
        }
    }
}

void drawGridVelocities(float* u, float *v){
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f) + (CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f) + (CELL_SIZE * 0.5f);
    glm::vec4 color;

    for(int y = 0; y < GRID_SIZE; y++){
        for(int x = 0; x < GRID_SIZE; x++){
            float xx = xo + (x * CELL_SIZE);
            float yy = yo + (y * CELL_SIZE);
            float ux = u[IX(x,y)];
            float vy = v[IX(x,y)];
            float scale = 1.0f * CELL_SIZE;
            //scale = 1.0f;
            //int i = (y * GRID_SIZE) + x;
            //color = {grid[IX(x,y)], grid[IX(x,y)], grid[IX(x,y)], 1};
            //renderDrawFilledRectPro({xx,yy}, {CELL_SIZE, CELL_SIZE}, 0, {0.0f, 0.0f}, clampColor(color));
            renderDrawLine({xx, yy}, {xx + scale*ux , yy + scale*vy}, {0.0f, 0.0f, 1.0f, 1.0f}, 1.0f);
        }
    }
}

void drawGridContent(float* grid){
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    glm::vec4 color;

    for(int y = 0; y < GRID_SIZE; y++){
        for(int x = 0; x < GRID_SIZE; x++){
            float xx = xo + (x * CELL_SIZE);
            float yy = yo + (y * CELL_SIZE);
            //int i = (y * GRID_SIZE) + x;
            color = {grid[IX(x,y)], grid[IX(x,y)], grid[IX(x,y)], 1};
            //float d = grid[IX(x,y)];
            //color = {255, 255, 255, d};
            renderDrawFilledRect({xx,yy}, {CELL_SIZE, CELL_SIZE}, 0, clampColor(color));
        }
    }
}

void setBnd(int b, float* x){
    for(int i = 0; i <= GRID_SIZE; i++){
        x[IX(0,i)] = b == 1 ? -x[IX(1,i)] : x[IX(1,i)];
        x[IX(GRID_SIZE+1,i)] = b == 1 ? -x[IX(GRID_SIZE,i)] : x[IX(GRID_SIZE,i)];
        x[IX(i,0)] = b == 2 ? -x[IX(i,1)] : x[IX(i,1)];
        x[IX(i,GRID_SIZE+1)] = b == 2 ? -x[IX(i,GRID_SIZE)] : x[IX(i,GRID_SIZE)];
    }
    x[IX(0,0)] = 0.5f * (x[IX(1,0)] + x[IX(0,1)]);
    x[IX(0,GRID_SIZE+1)] = 0.5f * (x[IX(1,GRID_SIZE+1)] + x[IX(0,GRID_SIZE)]);
    x[IX(GRID_SIZE+1,0)] = 0.5f * (x[IX(GRID_SIZE,0)] + x[IX(GRID_SIZE+1,1)]);
    x[IX(GRID_SIZE+1,GRID_SIZE+1)] = 0.5f * (x[IX(GRID_SIZE,GRID_SIZE+1)] + x[IX(GRID_SIZE+1,GRID_SIZE)]);
}

void advect(int b, float* d, float* d0, float* u, float* v, float dt){
    int i0, j0, i1, j1; 
    float x, y, s0, t0, s1, t1, dt0; 
    dt0 = dt * GRID_SIZE; 
    for (int  i =1 ;  i <= GRID_SIZE ; i++) { 
        for (int j = 1 ; j <= GRID_SIZE; j++) { 
            x = i - dt0 * u[IX(i,j)];
            y = j - dt0 * v[IX(i,j)]; 
            if (x < 0.5) x = 0.5;
            if (x > GRID_SIZE + 0.5) {
                x = GRID_SIZE + 0.5; 
            }
            i0 = (int)x;
            i1 = i0+1; 
            if (y < 0.5) y = 0.5;
            if (y > GRID_SIZE + 0.5){
                y = GRID_SIZE + 0.5; 
            }
            j0 = (int)y; 
            j1 = j0 + 1; 

            s1 = x - i0;
            s0 = 1 - s1;
            t1 = y - j0;
            t0 = 1 - t1; 
            d[IX(i,j)] = s0 * (t0 * d0[IX(i0,j0)] + t1 * d0[IX(i0,j1)]) + s1 * (t0 * d0[IX(i1,j0)] + t1 * d0[IX(i1,j1)]); 
        } 
    } 
    setBnd(b, d); 
}

void diffuse(float diff, int b, float* x1, float* x0, float dt){
    float a = diff * GRID_SIZE * GRID_SIZE * dt;
    for(int k = 0; k < 20; k++){ //iterate 20 times to find the solution
        for(int y = 1; y <= GRID_SIZE; y++){
            for(int x = 1; x <= GRID_SIZE; x++){
                x1[IX(x,y)] = (x0[IX(x,y)] + a * (x1[IX(x-1,y)] + x1[IX(x+1,y)] + x1[IX(x,y-1)] + x1[IX(x,y+1)]))/(1+4*a);
            }
        }
    }
    setBnd(b, x1);

}

void addSource(float* x, float* s, float dt){
    int size = GRID_SIZE * GRID_SIZE;
    for(int i = 0; i < size; i++){
        x[i] += s[i] * dt;
        //x[i] = s[i] * dt;
    }
}

void denseStep(float *x1, float *x0, float* u, float *v, float diff, float dt){
    addSource(x1, x0, dt);
    SWAP(x0, x1);
    diffuse(diff, 0, x1, x0, dt);
    SWAP(x0, x1);
    advect(0, x1, x0, u, v, dt);
}

void project(float* u, float* v, float* p, float* div){
    float h;
    h = 1.0f/ GRID_SIZE;

    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
            div[IX(x,y)] = -0.5f * h * (u[IX(x + 1,y)] - u[IX(x-1, y)] + v[IX(x, y+1)] - v[IX(x, y-1)]);
            p[IX(x,y)] = 0;
        }
    }
    setBnd(0, div);
    setBnd(0, p);

    for(int k = 0; k < 20; k++){
        for(int y = 1; y <= GRID_SIZE; y++){
            for(int x = 1; x <= GRID_SIZE; x++){
                p[IX(x,y)] = (div[IX(x,y)] + p[IX(x-1, y)] + p[IX(x+1,y)] + p[IX(x, y-1)] + p[IX(x, y+1)]) / 4;
            }
        }
        setBnd(0, p);
    }

    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
            u[IX(x,y)] -= 0.5f * (p[IX(x+1, y)] - p[IX(x-1, y)]) / h;
            v[IX(x,y)] -= 0.5f * (p[IX(x, y+1)] - p[IX(x, y-1)]) / h;
        }
    }
    setBnd(1, u);
    setBnd(2, v);
}

void velStep(float* u, float* v, float *uPrev, float* vPrev, float visc, float dt){
    addSource(u, uPrev, dt);
    addSource(v, vPrev, dt);
    SWAP(uPrev, u);
    SWAP(vPrev, v);
    diffuse(visc, 1, u, uPrev, dt);
    diffuse(visc, 2, v, vPrev, dt);
    //advect(1, u, uPrev, uPrev, vPrev, dt);
    //advect(2, v, vPrev, uPrev, vPrev, dt);
    project(u, v, uPrev, vPrev);
    SWAP(uPrev, u);
    SWAP(vPrev, v);
    //diffuse(visc, 1, u, uPrev, dt);
    //diffuse(visc, 2, v, vPrev, dt);
    advect(1, u, uPrev, uPrev, vPrev, dt);
    advect(2, v, vPrev, uPrev, vPrev, dt);
    project(u, v, uPrev, vPrev);
}

void setBoundaries(bool* solid){
    for(int i = 0; i < GRID_SIZE; i++){
        for(int j = 0; j < GRID_SIZE; j++){
            solid[IX(i,j)] = false;
        }
    }

    for(int i = 0; i < GRID_SIZE; i++){
        solid[IX(i, 0)] = true;
        solid[IX(i, GRID_SIZE-1)] = true;
    }

    for(int j = 0; j < GRID_SIZE; j++){
        solid[IX(0, j)] = true;
        solid[IX(GRID_SIZE-1, j)] = true;
    }

    //int cx = GRID_SIZE / 2;
    //int cy = GRID_SIZE / 2;
    //int radius = 5;
    //for(int i = cx - radius; i < cx + radius; i++){
    //    for(int j = cy - radius; j < cy + radius; j++){
    //        int dx = i - cx;
    //        int dy = j - cy;
    //        if(dx * dx + dy * dy <= radius * radius){
    //            solid[IX(i,j)] = true;
    //        }
    //    }
    //}
    //solid[IX(15,22)] = true;
    //solid[IX(15,23)] = true;
    //solid[IX(15,24)] = true;
    //solid[IX(15,25)] = true;
    //solid[IX(15,26)] = true;
    //solid[IX(15,27)] = true;
    //solid[IX(16,22)] = true;
    //solid[IX(16,23)] = true;
    //solid[IX(16,24)] = true;
    //solid[IX(16,25)] = true;
    //solid[IX(16,26)] = true;
    //solid[IX(16,27)] = true;
}

float divergenceAtCell(int i, int j, float*u, float*v, bool* solid){
    float dx = 1.0f / GRID_SIZE;
    float uLeft  = solid[IXC(i-1,j)] ? 0 : u[IXC(i-1,j)];
    float uRight = solid[IXC(i+1,j)] ? 0 : u[IXC(i+1,j)];
    float vDown  = solid[IXC(i,j-1)] ? 0 : v[IXC(i,j-1)];
    float vUp    = solid[IXC(i,j+1)] ? 0 : v[IXC(i,j+1)];
    int countX = (solid[IXC(i-1,j)] ? 0 : 1) + (solid[IXC(i+1,j)] ? 0 : 1);
    int countY = (solid[IXC(i,j-1)] ? 0 : 1) + (solid[IXC(i,j+1)] ? 0 : 1);
    float divX = countX > 0 ? (uRight - uLeft) / (countX * dx) : 0;
    float divY = countY > 0 ? (vUp - vDown) / (countY * dx) : 0;
    return (divX + divY);
}

void calculateDivergence(float* p, float* div, float* u, float*v, bool* solid, float dt){
    for(int i = 0; i < GRID_SIZE; i++){
        for(int j = 0; j < GRID_SIZE; j++){
            p[IX(i,j)] = 0;
            div[IX(i,j)] = divergenceAtCell(i,j,u,v,solid);
        }
    }
}

//void diffuse2(float* x, float* x0){
//    float dx = 1.0f / GRID_SIZE;
//    for(int k = 0; k < 200; k++){
//        for(int i = 1; i <= GRID_SIZE; i++){
//            for(int j = 1; j <= GRID_SIZE; j++){
//                p[IX(i,j)] = ((-dx * dx * div[IX(i,j)]) + p[IX(i-1,j)] + p[IX(i+1,j)] + p[IX(i,j-1)] + p[IX(i,j+1)]) / 4;
//            }
//        }
//        setBoundary(p, 1);
//    }
//}

void diffuse2(float* x, float* x0, bool* solid, float dt){
    float a = 0.00001f * GRID_SIZE * GRID_SIZE * dt;
    for(int k = 0; k < 50; k++){
        for(int j = 0; j < GRID_SIZE; j++){
            for(int i = 0; i < GRID_SIZE; i++){
                if(solid[IX(i,j)]) continue;
                int sum = (solid[IXC(i-1,j)] ? 0 : 1) + (solid[IXC(i+1,j)] ? 0 : 1) + (solid[IXC(i,j-1)] ? 0 : 1) + (solid[IXC(i,j+1)] ? 0 : 1);
                if(sum == 0) continue;
                float xL = solid[IXC(i-1,j)] ? 0 : x[IXC(i-1,j)];
                float xR = solid[IXC(i+1,j)] ? 0 : x[IXC(i+1,j)];
                float xB = solid[IXC(i,j-1)] ? 0 : x[IXC(i,j-1)];
                float xT = solid[IXC(i,j+1)] ? 0 : x[IXC(i,j+1)];
                //x[IX(i,j)] = (x0[IX(i,j)] + a * (xL + xR + xB + xT))/(1+4*a);
                x[IX(i,j)] = (x0[IX(i,j)] + a * (xL + xR + xB + xT))/(1+sum*a);
            }
        }
    }

}


void project2(float *u, float* uPrev, float *v, float* vPrev, float* p, float* div, bool* solid, float dt){
    float h = 1.0f / GRID_SIZE;

    // Divergence — same as Stam but skip solid neighbors
    for(int j = 0; j < GRID_SIZE; j++){
        for(int i = 0; i < GRID_SIZE; i++){
            if(solid[IX(i,j)]){ div[IX(i,j)] = 0; p[IX(i,j)] = 0; continue; }
            float uRight = solid[IXC(i+1,j)] ? 0 : u[IXC(i+1,j)];
            float uLeft  = solid[IXC(i-1,j)] ? 0 : u[IXC(i-1,j)];
            float vTop   = solid[IXC(i,j+1)] ? 0 : v[IXC(i,j+1)];
            float vBot   = solid[IXC(i,j-1)] ? 0 : v[IXC(i,j-1)];
            div[IX(i,j)] = -0.5f * h * (uRight - uLeft + vTop - vBot);
            p[IX(i,j)] = 0;
        }
    }

    // Pressure solve — Jacobi, using sum of non-solid neighbors
    for(int k = 0; k < 50; k++){
        for(int j = 0; j < GRID_SIZE; j++){
            for(int i = 0; i < GRID_SIZE; i++){
                if(solid[IX(i,j)]) continue;
                float pL = solid[IXC(i-1,j)] ? p[IX(i,j)] : p[IXC(i-1,j)];
                float pR = solid[IXC(i+1,j)] ? p[IX(i,j)] : p[IXC(i+1,j)];
                float pB = solid[IXC(i,j-1)] ? p[IX(i,j)] : p[IXC(i,j-1)];
                float pT = solid[IXC(i,j+1)] ? p[IX(i,j)] : p[IXC(i,j+1)];
                p[IX(i,j)] = (div[IX(i,j)] + pL + pR + pB + pT) / 4;
            }
        }
    }

    // Gradient subtraction — same as Stam but skip solid neighbors
    for(int j = 0; j < GRID_SIZE; j++){
        for(int i = 0; i < GRID_SIZE; i++){
            if(solid[IX(i,j)]) continue;
            float pR = solid[IXC(i+1,j)] ? p[IX(i,j)] : p[IXC(i+1,j)];
            float pL = solid[IXC(i-1,j)] ? p[IX(i,j)] : p[IXC(i-1,j)];
            float pT = solid[IXC(i,j+1)] ? p[IX(i,j)] : p[IXC(i,j+1)];
            float pB = solid[IXC(i,j-1)] ? p[IX(i,j)] : p[IXC(i,j-1)];
            u[IX(i,j)] -= 0.5f * (pR - pL) / h;
            v[IX(i,j)] -= 0.5f * (pT - pB) / h;
        }
    }
}

void advect2(float* q, float* q0, float* u, float*v, bool* solid, float dt){
    //equation: q(x, t + dt) = q(x - u(x, t) * dt, t)
    int i0, j0, i1, j1;
    float x, y;
    for(int i = 0; i < GRID_SIZE; i++){
        for(int j = 0; j < GRID_SIZE; j++){
            if(solid[IX(i,j)]) continue;
            x = i - (dt * u[IX(i,j)]);
            y = j - (dt * v[IX(i,j)]);
            // Clamp backtrace position to grid bounds
            if(x < 0) x = 0;
            if(x > GRID_SIZE - 1.001f) x = GRID_SIZE - 1.001f;
            if(y < 0) y = 0;
            if(y > GRID_SIZE - 1.001f) y = GRID_SIZE - 1.001f;
            i0 = (int)x;
            i1 = i0 + 1;
            j0 = (int)y;
            j1 = j0 + 1;
            //bilinear interpolation
            float ax = (x - i0);
            float ay = (y - j0);
            float fxy1 = ((1 - ax) * q0[IX(i0, j0)]) + (ax * q0[IX(i1, j0)]);
            float fxy2 = ((1 - ax) * q0[IX(i0, j1)]) + (ax * q0[IX(i1, j1)]);
            float fxy = ((1- ay) * fxy1) + (ay * fxy2);
            q[IX(i,j)] = fxy;
        }
    }
    for(int i = 0; i < GRID_SIZE; i++){
        for(int j = 0; j < GRID_SIZE; j++){
            q0[IX(i,j)] = q[IX(i,j)];
        }
    }

}

void simulationStep(float* dens, float* densPrev, float* u, float* v, float* uPrev, float* vPrev, float* p, float* div, bool* solid, float dt){
    //addSource(uPrev, u, dt);
    //addSource(vPrev, v, dt);
    diffuse2(uPrev, u, solid, dt);
    diffuse2(vPrev, v, solid, dt);
    project2(u, uPrev, v, vPrev, p, div, solid, dt);

    advect2(uPrev, u, uPrev, vPrev, solid, dt);
    advect2(vPrev, v, uPrev, vPrev, solid, dt);
    project2(u, uPrev, v, vPrev, p, div, solid, dt);

    //addSource(densPrev, dens, dt);
    advect2(densPrev, dens, u, v, solid, dt);
    diffuse2(dens, densPrev, solid, dt);

    //advect2(dens, dens, uPrev, vPrev, dt);
    // Open boundaries: copy edge values into boundary ring
//for(int i = 1; i <= GRID_SIZE; i++){
//    u[IX(0,i)]           = u[IX(1,i)];
//    u[IX(GRID_SIZE+1,i)] = u[IX(GRID_SIZE,i)];
//    u[IX(i,0)]           = u[IX(i,1)];
//    u[IX(i,GRID_SIZE+1)] = u[IX(i,GRID_SIZE)];
//
//    v[IX(0,i)]           = v[IX(1,i)];
//    v[IX(GRID_SIZE+1,i)] = v[IX(GRID_SIZE,i)];
//    v[IX(i,0)]           = v[IX(i,1)];
//    v[IX(i,GRID_SIZE+1)] = v[IX(i,GRID_SIZE)];
//
//    dens[IX(0,i)]           = dens[IX(1,i)];
//    dens[IX(GRID_SIZE+1,i)] = dens[IX(GRID_SIZE,i)];
//    dens[IX(i,0)]           = dens[IX(i,1)];
//    dens[IX(i,GRID_SIZE+1)] = dens[IX(i,GRID_SIZE)];
//}

    // Clear velocity
    //for(int y = 1; y <= GRID_SIZE; y++){
    //    for(int x = 1; x <= GRID_SIZE; x++){
    //        uPrev[IX(x,y)] = 0.0f;
    //        vPrev[IX(x,y)] = 0.0f;
    //    }
    //}
}

GAME_API void gameStart(Arena* gameArena){
    if(gameArena->index > 0){
        return;
    }
    GameState* gameState = arenaAllocStruct(gameArena, GameState);
    gameState->arena = gameArena;
    // Resolution-independent camera: shows 640 world units horizontally, 360 vertically, centered at origin
    // Bounds: -360 to +360 horizontally, -160 to +160 vertically
    //gameState->mainCamera = createCamera(-1920.0f / 2, 1920.0f / 2, -1080.0f / 2, 1080.0f / 2);
    gameState->mainCamera = createCamera(-640.0f / 2, 640.0f / 2, -360.0f / 2, 360.0f / 2);
    setActiveCamera(&gameState->mainCamera);
    gameState->u = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->uPrev = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->v = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->vPrev = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->dens = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->densPrev = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->p = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->div = arenaAllocArrayZero(gameState->arena, float, GRID_SIZE * GRID_SIZE);
    gameState->solid = arenaAllocArrayZero(gameState->arena, bool, GRID_SIZE * GRID_SIZE);
    gameState->diff = 0.0001f;
    gameState->visc = 0.0f;
    gameState->restart = false;
    gameState->startSim = false;

    // Clear velocity
    for(int y = 0; y < GRID_SIZE; y++){
        for(int x = 0; x < GRID_SIZE; x++){
            gameState->uPrev[IX(x,y)] = 0.0f;
            gameState->vPrev[IX(x,y)] = 0.0f;
        }
    }

    setBoundaries(gameState->solid);

    // Set one or more density blobs in the center
    int cx = GRID_SIZE / 2;
    int cy = GRID_SIZE / 2;

    //for(int y = cy-3; y <= cy+3; y++){
    //    for(int x = cx-3; x <= cx+3; x++){
    //        gameState->densPrev[IX(x,y)] = 100.0f;
    //    }
    //}

    float force = 10.0f;
    //for(int x = 1; x <= GRID_SIZE; x++){
    //    gameState->uPrev[IX(x,25)] = 1;
    //    gameState->uPrev[IX(x,26)] = 1;
    //    gameState->uPrev[IX(x,27)] = 1;
    //    //gameState->vPrev[IX(x,y)] = 1;
    //}
    //for(int x = 1; x <= GRID_SIZE; x++){
    //    gameState->uPrev[IX(x,22)] = 1.0f * force;
    //    gameState->uPrev[IX(x,23)] = 1.0f * force;
    //    gameState->uPrev[IX(x,24)] = 1.0f * force;
    //    gameState->uPrev[IX(x,25)] = 1.0f * force;
    //    gameState->uPrev[IX(x,26)] = 1.0f * force;
    //    gameState->uPrev[IX(x,27)] = 1.0f * force;
    //    gameState->uPrev[IX(x,28)] = 1.0f * force;
    //    //gameState->vPrev[IX(x,25)] = 10.0f * force;
    //}
    //for(int y = 1; y <= GRID_SIZE; y++){
    //    for(int x = 1; x <= GRID_SIZE; x++){
    //        gameState->uPrev[IX(x,y)] = ((float)(rand() % 1000) / 1000.0f - 0.5f) * force;
    //        gameState->vPrev[IX(x,y)] = ((float)(rand() % 1000) / 1000.0f - 0.5f) * force;
    //    }
    //}
    //for(int y = 1; y <= GRID_SIZE; y++){
    //    for(int x = 1; x <= GRID_SIZE; x++){
    //        float dx = x - cx;
    //        float dy = y - cy;
    //        float dist = sqrtf(dx*dx + dy*dy) + 0.001f;
    //        float falloff = expf(-dist * dist / 100.0f);
    //        gameState->uPrev[IX(x,y)] = -dy / dist * force * falloff;
    //        gameState->vPrev[IX(x,y)] =  dx / dist * force * falloff;
    //    }
    //}
}

GAME_API void gameRender(Arena* gameArena, float dt){}

GAME_API void gameUpdate(Arena* gameArena, float dt){
    GameState* gameState = (GameState*)gameArena->memory;
    //dt = dt*0.9f;
    //dt = 0.5f;
    if(isJustPressed(KEYS::T)){
        float force = 10.0f;
        for(int x = 0; x < GRID_SIZE; x++){
            gameState->u[IX(x,21)] = 1.0f * force;
            gameState->u[IX(x,22)] = 1.0f * force;
            gameState->u[IX(x,23)] = 1.0f * force;
            gameState->u[IX(x,24)] = 1.0f * force;
            gameState->u[IX(x,25)] = 1.0f * force;
            gameState->u[IX(x,26)] = 1.0f * force;
            gameState->u[IX(x,27)] = 1.0f * force;
            //gameState->vPrev[IX(x,25)] = 10.0f * force;
        }
    }
    if(isJustPressed(KEYS::R)){
        gameState->startSim = true;
        //int size = (GRID_SIZE + 2) * (GRID_SIZE + 2);
        //for(int i = 0; i < size; i++){
        //    gameState->uPrev[i] = 0.0f;
        //    gameState->u[i] = 0.0f;
        //    gameState->vPrev[i] = 0.0f;
        //    gameState->v[i] = 0.0f;
        //    gameState->densPrev[i] = 0.0f;
        //}
        //int cx = GRID_SIZE / 2;
        //int cy = GRID_SIZE / 2;
        //float force = 50.0f;

        //for(int y = cy-3; y <= cy+3; y++){
        //    for(int x = cx-3; x <= cx+3; x++){
        //        gameState->densPrev[IX(x,y)] = 100.0f;
        //    }
        //}
    }

    glm::vec2 mousePos = screenToWorld(gameState->mainCamera, getScreenSize(), getMousePos());
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    if(     ((int)mousePos.x > xo) && ((int) mousePos.x < xo + (GRID_SIZE * CELL_SIZE))
        &&  ((int)-mousePos.y > yo) && ((int) -mousePos.y < yo + (GRID_SIZE * CELL_SIZE))){ //if mouse is inside the grid

            int cellX = clampI((int)((mousePos.x - xo) / CELL_SIZE));
            int cellY = clampJ((int)((-mousePos.y - yo) / CELL_SIZE));
            if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                glm::vec2 mouseDelta = mousePos - gameState->mousePrev;
                float force = 10.0f;
                //gameState->densPrev[IX(((int)mousePos.x / CELL_SIZE) + (GRID_SIZE / 2), (int)-mousePos.y / CELL_SIZE + (GRID_SIZE / 2))] = 100.0f;
                //gameState->uPrev[IX(cellX, cellY)] = mouseDelta.x * force;
                //gameState->vPrev[IX(cellX, cellY)] = -mouseDelta.y * force;

                gameState->dens[IX(cellX, cellY)] = 1.0f;
                //gameState->u[IX(cellX, cellY)] = mouseDelta.x * force;
                //gameState->v[IX(cellX, cellY)] = -mouseDelta.y * force;
                //LOGINFO("%f, %f", mouseDelta.x, mouseDelta.y);
            }
            //LOGINFO("%d \t %d", cellX, cellY);
        }

    //addSource(gameState->u, gameState->uPrev, dt);
    //addSource(gameState->v, gameState->vPrev, dt);

    gameState->mousePrev = mousePos;
    if(gameState->startSim){
        //velStep(gameState->u, gameState->v, gameState->uPrev, gameState->vPrev, gameState->visc, dt);
        //denseStep(gameState->dens, gameState->densPrev, gameState->u, gameState->v, gameState->diff, dt);
        simulationStep(gameState->dens, gameState->densPrev, gameState->u, gameState->v, gameState->uPrev, gameState->vPrev, gameState->p, gameState->div, gameState->solid, dt);

        beginScene();
            clearColor(0.2f, 0.3f, 0.3f, 1.0f);
            beginMode2D(gameState->mainCamera);
            drawGridContent(gameState->dens);
            drawGridVelocities(gameState->u, gameState->v);
            //drawGrid(gameState->dens);
            endMode2D();
        endScene();

        // Clear source arrays so they don't accumulate across frames
        //int size = (GRID_SIZE + 2) * (GRID_SIZE + 2);
        //for(int i = 0; i < size; i++){
        //    gameState->uPrev[i] = 0.0f;
        //    gameState->vPrev[i] = 0.0f;
        //    gameState->densPrev[i] = 0.0f;
        //}
    }else{
        beginScene();
            clearColor(0.2f, 0.3f, 0.3f, 1.0f);
            beginMode2D(gameState->mainCamera);
            drawGridContent(gameState->dens);
            drawGridVelocities(gameState->u, gameState->v);
            //drawGrid(gameState->dens);
            endMode2D();
        endScene();
        //velStep(gameState->u, gameState->v, gameState->uPrev, gameState->vPrev, gameState->visc, dt);
        //denseStep(gameState->dens, gameState->densPrev, gameState->u, gameState->v, gameState->diff, dt);
        // Clear velocity
        //for(int y = 1; y <= GRID_SIZE; y++){
        //    for(int x = 1; x <= GRID_SIZE; x++){
        //        gameState->uPrev[IX(x,y)] = 0.0f;
        //        gameState->vPrev[IX(x,y)] = 0.0f;
        //    }
        //}

    }

    for(int i = 0; i < GRID_SIZE; i++){
        for(int j = 0; j < GRID_SIZE; j++){
            float h = 1.0f / GRID_SIZE;
            float uR = gameState->solid[IXC(i+1,j)] ? 0 : gameState->u[IXC(i+1,j)];
            float uL = gameState->solid[IXC(i-1,j)] ? 0 : gameState->u[IXC(i-1,j)];
            float vT = gameState->solid[IXC(i,j+1)] ? 0 : gameState->v[IXC(i,j+1)];
            float vB = gameState->solid[IXC(i,j-1)] ? 0 : gameState->v[IXC(i,j-1)];
            float div = -0.5f * h * (uR - uL + vT - vB);
            //float div = gameState->div[IX(i,j)];
            if(div > 0.001f){
                LOGINFO("Divergence [%d,%d]: %f", i,j,div);
            }
        }
    }
}

GAME_API void gameStop(Arena* gameArena){
}