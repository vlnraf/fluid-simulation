#include "projectx.hpp"
#include <glm/glm.hpp>
#include <cmath>


#define GRID_SIZE 50
#define GRID_SIZE_Y 50
#define GRID_SIZE_X GRID_SIZE_Y * 2
#define CELL_SIZE 5

#define IX(i,j)   ((i) + GRID_SIZE*(j))          // cell-centered
#define IX_U(i,j) ((i) + (GRID_SIZE+1)*(j))      // u: (N+1) x N
#define IX_V(i,j) ((i) + GRID_SIZE*(j))           // v: N x (N+1)

void drawDensity(float* d){
    float xo=-GRID_SIZE*CELL_SIZE*0.5f;
    float yo=-GRID_SIZE*CELL_SIZE*0.5f;
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float v=d[IX(i,j)];
            renderDrawFilledRect(
                {xo+i*CELL_SIZE, yo+j*CELL_SIZE},
                {CELL_SIZE, CELL_SIZE}, 0,
                {v,v,v,1}
            );
        }
    }
}

void drawVelocities(float* u,float* v){
    float xo=-GRID_SIZE*CELL_SIZE*0.5f;
    float yo=-GRID_SIZE*CELL_SIZE*0.5f;
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float cx = xo + (i+0.5f)*CELL_SIZE;
            float cy = yo + (j+0.5f)*CELL_SIZE;
            float ux = 0.5f*(u[IX_U(i,j)] + u[IX_U(i+1,j)]);
            float vy = 0.5f*(v[IX_V(i,j)] + v[IX_V(i,j+1)]);
            renderDrawLine({cx,cy},{cx+ux*5, cy+vy*5},{0,0,1,1},1);
        }
    }
}

void drawPressure(float* p){
    float xo=-GRID_SIZE*CELL_SIZE*0.5f;
    float yo=-GRID_SIZE*CELL_SIZE*0.5f;
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float val = p[IX(i,j)];
            float r = glm::clamp( val * 5.0f, 0.0f, 1.0f);
            float b = glm::clamp(-val * 5.0f, 0.0f, 1.0f);
            renderDrawFilledRect(
                {xo+i*CELL_SIZE, yo+j*CELL_SIZE},
                {CELL_SIZE, CELL_SIZE}, 0,
                {r,0,b,1}
            );
        }
    }
}

void drawSpeed(float* u, float* v){
    float xo=-GRID_SIZE*CELL_SIZE*0.5f;
    float yo=-GRID_SIZE*CELL_SIZE*0.5f;
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float ux = 0.5f*(u[IX_U(i,j)] + u[IX_U(i+1,j)]);
            float vy = 0.5f*(v[IX_V(i,j)] + v[IX_V(i,j+1)]);
            float speed = sqrtf(ux*ux + vy*vy);
            float t = glm::clamp(speed * 2.0f, 0.0f, 1.0f); // 0=still, 0.5=mid, 1=fast
            float r = glm::clamp(2.0f*t - 1.0f, 0.0f, 1.0f);
            float g = 1.0f - 2.0f*fabs(t - 0.5f);
            float b = glm::clamp(1.0f - 2.0f*t, 0.0f, 1.0f);
            renderDrawFilledRect(
                {xo+i*CELL_SIZE, yo+j*CELL_SIZE},
                {CELL_SIZE, CELL_SIZE}, 0,
                {r,g,b,1}
            );
        }
    }
}

float computeMaxDivergence(float* u, float* v) {
    float h = 1.0f / GRID_SIZE;
    float maxDiv = 0.0f;

    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float ur = u[IX_U(i+1,j)];
            float ul = u[IX_U(i,j)];
            float vt = v[IX_V(i,j+1)];
            float vb = v[IX_V(i,j)];
            float div = (ur - ul + vt - vb);
            maxDiv = glm::max(maxDiv, fabs(div));
        }
    }
    return maxDiv;
}

// FLUID CORE 
void computeDivergence(float* u,float* v,float* div,float* p){
    float h = 1.0f / GRID_SIZE;
    float maxDiv = 0.0f;

    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float ur = u[IX_U(i+1,j)];
            float ul = u[IX_U(i,j)];
            float vt = v[IX_V(i,j+1)];
            float vb = v[IX_V(i,j)];
            div[IX(i,j)] = (ur - ul + vt - vb);
            p[IX(i,j)] = 0.0f;
            maxDiv = glm::max(maxDiv, fabs(div[IX(i,j)]));
        }
    }
    LOGINFO("Divergence pre-solve: %f", maxDiv);
}

void solvePressure(float* p,float* div){
    for(int k=0;k<20;k++){
        for(int j=0;j<GRID_SIZE;j++){
            for(int i=0;i<GRID_SIZE;i++){
                float pL = (i > 0)            ? p[IX(i-1,j)] : p[IX(i+1,j)];
                float pR = (i < GRID_SIZE-1)  ? p[IX(i+1,j)] : p[IX(i-1,j)];
                float pB = (j > 0)            ? p[IX(i,j-1)] : p[IX(i,j+1)];
                float pT = (j < GRID_SIZE-1)  ? p[IX(i,j+1)] : p[IX(i,j-1)];
                p[IX(i,j)] = (pL + pR + pB + pT - div[IX(i,j)]) * 0.25f;
            }
        }
    }
}

void project(float* u,float* v,float* p){
    // u interior faces: i in [1, GRID_SIZE-1], j in [0, GRID_SIZE-1]
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=1;i<GRID_SIZE;i++){
            u[IX_U(i,j)] -= (p[IX(i,j)] - p[IX(i-1,j)]);
        }
    }
    // v interior faces: i in [0, GRID_SIZE-1], j in [1, GRID_SIZE-1]
    for(int j=1;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            v[IX_V(i,j)] -= (p[IX(i,j)] - p[IX(i,j-1)]);
        }
    }
}

void applyVelocityBoundaries(float* u,float* v){
    for(int j=0;j<GRID_SIZE;j++){
        u[IX_U(0,j)] = 0;
        u[IX_U(GRID_SIZE,j)] = 0;
        //u[IX_U(0,j)] = u[IX_U(1,j)];
        //u[IX_U(GRID_SIZE,j)] = u[IX_U(GRID_SIZE-1,j)];
    }
    for(int i=0;i<GRID_SIZE;i++){
        v[IX_V(i,0)] = 0;
        v[IX_V(i,GRID_SIZE)] = 0;
    }
}

void advectVelocity(float* u,float* v,float dt){
    float u0[(GRID_SIZE+1)*GRID_SIZE];
    float v0[GRID_SIZE*(GRID_SIZE+1)];
    memcpy(u0, u, sizeof(u0));
    memcpy(v0, v, sizeof(v0));

    float N = (float)GRID_SIZE;

    // advect u: lives at (i, j+0.5) in grid coords, i in [0,N], j in [0,N-1]
    for(int j=0;j<GRID_SIZE;j++){
        for(int i=1;i<GRID_SIZE;i++){
            // velocity at this u-face
            float ux = u0[IX_U(i,j)];
            // interpolate v to u-face location: average 4 neighboring v values
            float vy = 0.25f*(v0[IX_V(i-1,j)] + v0[IX_V(i,j)] + v0[IX_V(i-1,j+1)] + v0[IX_V(i,j+1)]);

            // backtrace in u-grid coords (u lives at integer i, integer j)
            float x = i - dt * ux * N;
            float y = j - dt * vy * N;

            x = glm::clamp(x, 0.5f, N-0.5f);
            y = glm::clamp(y, 0.0f, N-1.001f);

            int i0=(int)x, j0=(int)y;
            int i1=i0+1, j1=j0+1;
            i1 = glm::min(i1, GRID_SIZE);
            j1 = glm::min(j1, GRID_SIZE-1);

            float sx=x-i0, sy=y-j0;
            u[IX_U(i,j)] =
                (1-sx)*(1-sy)*u0[IX_U(i0,j0)] +
                sx*(1-sy)*u0[IX_U(i1,j0)] +
                (1-sx)*sy*u0[IX_U(i0,j1)] +
                sx*sy*u0[IX_U(i1,j1)];
        }
    }

    // advect v: lives at (i+0.5, j) in grid coords, i in [0,N-1], j in [0,N]
    for(int j=1;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            // interpolate u to v-face location: average 4 neighboring u values
            float ux = 0.25f*(u0[IX_U(i,j-1)] + u0[IX_U(i+1,j-1)] + u0[IX_U(i,j)] + u0[IX_U(i+1,j)]);
            float vy = v0[IX_V(i,j)];

            // backtrace in v-grid coords (v lives at integer i, integer j)
            float x = i - dt * ux * N;
            float y = j - dt * vy * N;

            x = glm::clamp(x, 0.0f, N-1.001f);
            y = glm::clamp(y, 0.5f, N-0.5f);

            int i0=(int)x, j0=(int)y;
            int i1=i0+1, j1=j0+1;
            i1 = glm::min(i1, GRID_SIZE-1);
            j1 = glm::min(j1, GRID_SIZE);

            float sx=x-i0, sy=y-j0;
            v[IX_V(i,j)] =
                (1-sx)*(1-sy)*v0[IX_V(i0,j0)] +
                sx*(1-sy)*v0[IX_V(i1,j0)] +
                (1-sx)*sy*v0[IX_V(i0,j1)] +
                sx*sy*v0[IX_V(i1,j1)];
        }
    }
}

void advectDensity(float* d,float* u,float* v,float dt){
    float d0[GRID_SIZE*GRID_SIZE];
    memcpy(d0, d, sizeof(d0));

    for(int j=0;j<GRID_SIZE;j++){
        for(int i=0;i<GRID_SIZE;i++){
            float ux = 0.5f*(u[IX_U(i,j)] + u[IX_U(i+1,j)]);
            float vy = 0.5f*(v[IX_V(i,j)] + v[IX_V(i,j+1)]);
            float x = i - dt * ux * GRID_SIZE;
            float y = j - dt * vy * GRID_SIZE;

            x = glm::clamp(x, 0.0f, GRID_SIZE-1.001f);
            y = glm::clamp(y, 0.0f, GRID_SIZE-1.001f);

            int i0=(int)x, j0=(int)y;
            int i1=i0+1, j1=j0+1;
            i1 = glm::min(i1, GRID_SIZE-1);
            j1 = glm::min(j1, GRID_SIZE-1);

            float sx=x-i0, sy=y-j0;
            d[IX(i,j)] =
                (1-sx)*(1-sy)*d0[IX(i0,j0)] +
                sx*(1-sy)*d0[IX(i1,j0)] +
                (1-sx)*sy*d0[IX(i0,j1)] +
                sx*sy*d0[IX(i1,j1)];
        }
    }
}

void step(float* d,float* u,float* v,float* p,float* div,float dt){
    // 1. advect velocity
    advectVelocity(u,v,dt);
    //applyVelocityBoundaries(u,v);

    //// 2. pressure projection (make divergence-free)
    //computeDivergence(u,v,div,p);
    //solvePressure(p,div);
    //project(u,v,p);
    //applyVelocityBoundaries(u,v);

    //float postDiv = computeMaxDivergence(u,v);
    //LOGINFO("Divergence post-project: %f", postDiv);

    //// 3. advect density
    //advectDensity(d,u,v,dt);
}

// ================= GAME =================
GAME_API void gameStart(Arena* arena){
    GameState* gs = arenaAllocStruct(arena, GameState);
    gs->arena = arena;
    gs->dens = arenaAllocArrayZero(arena,float,GRID_SIZE*GRID_SIZE);
    gs->u    = arenaAllocArrayZero(arena,float,(GRID_SIZE+1)*GRID_SIZE);
    gs->v    = arenaAllocArrayZero(arena,float,GRID_SIZE*(GRID_SIZE+1));
    gs->p    = arenaAllocArrayZero(arena,float,GRID_SIZE*GRID_SIZE);
    gs->div  = arenaAllocArrayZero(arena,float,GRID_SIZE*GRID_SIZE);

    //gs->mainCamera = createCamera(-200,200,-200,200);
    gs->mainCamera=createCamera(-640.0f/ 2, 640.0f/2, -320.0f/2, 320.0f/2);
    setActiveCamera(&gs->mainCamera);

    gs->mousePrev = {0,0};
    gs->fs = createFluidSimulator(gs->arena, GRID_SIZE_X, GRID_SIZE_Y, CELL_SIZE);

    for(int i = 0; i < gs->fs.gridSizeX; i++){
        //for(int j = gs->fs.gridSizeY/2-2; j < gs->fs.gridSizeX/2+2; j++){
        for(int j = 0; j < gs->fs.gridSizeX; j++){
            //gs->fs.u[uij(&gs->fs, i,j)] = 1.0f;
            //gs->fs.v[vij(&gs->fs, i,j)] = 1.0f;
            //for(int j = 0; j < gs->fs.gridSizeX; j++){
            //    gs->fs.u[ij(&gs->fs, i,j)] = 1.0f;
            //    gs->fs.v[ij(&gs->fs, i,j)] = 1.0f;
            //}
        }
    }
    loadFont("Roboto-Regular");
}

GAME_API void gameUpdate(Arena* arena,float dt){
    GameState* gs=(GameState*)arena->memory;
    dt = 0.1f;

    // mouse interaction
    glm::vec2 mouseScreen = getMousePos();
    glm::vec2 mouseWorld = screenToWorld(gs->mainCamera, getScreenSize(), mouseScreen);
    mouseWorld.y = -mouseWorld.y;

    //for(int i = 0; i < GRID_SIZE; i++){
    //    for(int j = GRID_SIZE/2-5; j < GRID_SIZE/2+5; j++){
    //        gs->u[IX_U(i,j)] = 1.0f;
    //        if(i < 3){
    //            gs->dens[IX(i,j)] = 1.0f;
    //        }
    //    }
    //}

    float xo = -(GRID_SIZE)*CELL_SIZE*0.5f;
    float yo = -(GRID_SIZE)*CELL_SIZE*0.5f;
    int mi = (int)((mouseWorld.x - xo) / CELL_SIZE);
    int mj = (int)((mouseWorld.y - yo) / CELL_SIZE);

    if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        glm::vec2 delta = (mouseWorld - gs->mousePrev) / (float)CELL_SIZE;
        float strength = 10.0f;
        int radius = 1;
        for(int dj=-radius;dj<=radius;dj++){
            for(int di=-radius;di<=radius;di++){
                int ci = mi+di, cj = mj+dj;
                if(ci>=0 && ci<GRID_SIZE && cj>=0 && cj<GRID_SIZE){
                    gs->u[IX_U(ci,cj)]   += delta.x * strength;
                    gs->u[IX_U(ci+1,cj)] += delta.x * strength;
                    gs->v[IX_V(ci,cj)]   += delta.y * strength;
                    gs->v[IX_V(ci,cj+1)] += delta.y * strength;
                    gs->dens[IX(ci,cj)] = 1.0f;

                    //gs->fs.u[uij(&gs->fs, ci,cj)] += delta.x * strength;
                    //gs->fs.v[vij(&gs->fs, ci,cj)] += delta.y * strength;
                }
            }
        }
    }

    xo = -(GRID_SIZE_X)*CELL_SIZE*0.5f;
    yo = -(GRID_SIZE_Y)*CELL_SIZE*0.5f;
    mi = (int)((mouseWorld.x - xo) / CELL_SIZE);
    mj = (int)((mouseWorld.y - yo) / CELL_SIZE);
    if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        glm::vec2 delta = (mouseWorld - gs->mousePrev) / (float)CELL_SIZE;
        float strength = 2.0f;
        int radius = 1;
        for(int j = -radius; j < radius; j++){
            for(int i = -radius; i < radius; i++){
                int cj = mj + j;
                int ci = mi + i;
                if(ci>=0 && ci<GRID_SIZE_X && cj>=0 && cj<GRID_SIZE_Y){
                    gs->fs.u[uij(&gs->fs, ci, cj)] += delta.x * strength;
                    gs->fs.v[vij(&gs->fs, ci, cj)] += delta.y * strength;
                    gs->fs.dens[ij(&gs->fs, ci, cj)] = 1.0f;
                }
            }
        }
    }
    gs->mousePrev = mouseWorld;

    //step(gs->dens,gs->u,gs->v,gs->p,gs->div,dt);

    step(&gs->fs, dt);

    beginScene();
    clearColor(0.1f,0.1f,0.1f,1);
    beginMode2D(gs->mainCamera);
    //drawDensity(gs->dens);
    //drawVelocities(gs->u,gs->v);
    //drawPressure(gs->p);
    //drawSpeed(gs->u, gs->v);

    //drawGrid(&gs->fs);
    //drawVelocities(&gs->fs);
    drawDensity(&gs->fs);
    //drawDivergence(&gs->fs);

    endMode2D();
    endScene();
}

GAME_API void gameRender(Arena*,float){}
GAME_API void gameStop(Arena*){}
