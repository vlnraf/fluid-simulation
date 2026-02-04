#include "projectx.hpp"

#define GRAVITY -9.8f
#define GRID_SIZE 50
#define CELL_SIZE 10

#define IX(i,j) ((i) + (GRID_SIZE + 2) * (j))
#define SWAP(x0,x) {float* tmp = x0; x0 = x; x = tmp;}

glm::vec4 clampColor(glm::vec4 color){
    return glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
}

void drawGrid(float* grid){
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f);

    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
            float xx = xo + (x * CELL_SIZE);// + (CELL_SIZE);
            float yy = yo + (y * CELL_SIZE);// + (CELL_SIZE);
            renderDrawRect({xx,yy}, {CELL_SIZE, CELL_SIZE}, {1,0,1,1}, 1.0f);
        }
    }
}

void drawGridVelocities(float* u, float *v){
    float xo = -((GRID_SIZE * CELL_SIZE * 0.5f) - (CELL_SIZE * 0.5f));
    float yo = -((GRID_SIZE * CELL_SIZE * 0.5f) - (CELL_SIZE * 0.5f));
    glm::vec4 color;

    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
            float xx = xo + (x * CELL_SIZE);
            float yy = yo + (y * CELL_SIZE);
            float ux = u[IX(x,y)];
            float vy = v[IX(x,y)];
            float scale = 100.0f;
            //int i = (y * GRID_SIZE) + x;
            //color = {grid[IX(x,y)], grid[IX(x,y)], grid[IX(x,y)], 1};
            //renderDrawFilledRectPro({xx,yy}, {CELL_SIZE, CELL_SIZE}, 0, {0.0f, 0.0f}, clampColor(color));
            renderDrawLine({xx, yy}, {xx + scale*ux , yy + scale*vy}, {1.0f, 0.0f, 0.0f, 1.0f}, 1.0f);
        }
    }
}

void drawGridContent(float* grid){
    float xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    float yo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    glm::vec4 color;

    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
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
    int size = (GRID_SIZE + 2) * (GRID_SIZE + 2);
    for(int i = 0; i < size; i++){
        x[i] += s[i] * dt;
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
    gameState->u = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->uPrev = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->v = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->vPrev = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->dens = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->densPrev = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->diff = 0.0001f;
    gameState->visc = 0.0f;
    gameState->restart = false;
    gameState->startSim = false;

    // Clear velocity
    for(int y = 1; y <= GRID_SIZE; y++){
        for(int x = 1; x <= GRID_SIZE; x++){
            gameState->uPrev[IX(x,y)] = 0.0f;
            gameState->vPrev[IX(x,y)] = 0.0f;
        }
    }

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
    if(isJustPressed(KEYS::T)){
        float force = 10.0f;
        for(int x = 1; x <= GRID_SIZE; x++){
            gameState->uPrev[IX(x,22)] = 1.0f * force;
            gameState->uPrev[IX(x,23)] = 1.0f * force;
            gameState->uPrev[IX(x,24)] = 1.0f * force;
            gameState->uPrev[IX(x,25)] = 1.0f * force;
            gameState->uPrev[IX(x,26)] = 1.0f * force;
            gameState->uPrev[IX(x,27)] = 1.0f * force;
            gameState->uPrev[IX(x,28)] = 1.0f * force;
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
    if(     ((int)mousePos.x >= xo+1) && ((int) mousePos.x <= xo + (GRID_SIZE * CELL_SIZE))
        &&  ((int)-mousePos.y >= yo+1) && ((int) -mousePos.y <= yo + (GRID_SIZE * CELL_SIZE))){ //if mouse is inside the grid

            if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                glm::vec2 mouseDelta = mousePos - gameState->mousePrev;
                float force = 10.0f;
                int cellX = ((int)mousePos.x / CELL_SIZE) + (GRID_SIZE / 2);
                int cellY = (int)-mousePos.y / CELL_SIZE + (GRID_SIZE / 2);
                gameState->densPrev[IX(((int)mousePos.x / CELL_SIZE) + (GRID_SIZE / 2), (int)-mousePos.y / CELL_SIZE + (GRID_SIZE / 2))] = 100.0f;
                gameState->uPrev[IX(cellX, cellY)] = mouseDelta.x * force;
                gameState->vPrev[IX(cellX, cellY)] = -mouseDelta.y * force;
            }
            LOGINFO("%d \t %d", ((int)mousePos.x / CELL_SIZE) + (GRID_SIZE / 2), ((int)-mousePos.y / CELL_SIZE) + (GRID_SIZE / 2));
        }

    gameState->mousePrev = mousePos;
    if(gameState->startSim){
        velStep(gameState->u, gameState->v, gameState->uPrev, gameState->vPrev, gameState->visc, dt);
        denseStep(gameState->dens, gameState->densPrev, gameState->u, gameState->v, gameState->diff, dt);

        beginScene();
            clearColor(0.2f, 0.3f, 0.3f, 1.0f);
            beginMode2D(gameState->mainCamera);
            drawGridContent(gameState->dens);
            drawGridVelocities(gameState->u, gameState->v);
            //drawGrid(gameState->dens);
            endMode2D();
        endScene();

        // Clear source arrays so they don't accumulate across frames
        int size = (GRID_SIZE + 2) * (GRID_SIZE + 2);
        for(int i = 0; i < size; i++){
            gameState->uPrev[i] = 0.0f;
            gameState->vPrev[i] = 0.0f;
            gameState->densPrev[i] = 0.0f;
        }
    }else{
        beginScene();
            clearColor(0.2f, 0.3f, 0.3f, 1.0f);
            beginMode2D(gameState->mainCamera);
            drawGridContent(gameState->dens);
            drawGridVelocities(gameState->u, gameState->v);
            //drawGrid(gameState->dens);
            endMode2D();
        endScene();
        velStep(gameState->u, gameState->v, gameState->uPrev, gameState->vPrev, gameState->visc, dt);
        //denseStep(gameState->dens, gameState->densPrev, gameState->u, gameState->v, gameState->diff, dt);
    }

}

GAME_API void gameStop(Arena* gameArena){
}