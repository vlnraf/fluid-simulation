#include "projectx.hpp"

#define GRAVITY 9.8f
#define GRID_SIZE 10
#define CELL_SIZE 20

void drawGrid(float* grid){
    int xo = -(GRID_SIZE * CELL_SIZE * 0.5f);
    int yo = -(GRID_SIZE * CELL_SIZE * 0.5f);

    for(int y = 0; y < GRID_SIZE; y++){
        for(int x = 0; x < GRID_SIZE; x++){
            int xx = xo + (x * CELL_SIZE);
            int yy = yo + (y * CELL_SIZE);
            renderDrawRect({xx,yy}, {CELL_SIZE, CELL_SIZE}, {1,0,1,1});
        }
    }
}

void addSource(float* x, float* s, float dt){
    int size = (GRID_SIZE + 2) * (GRID_SIZE + 2);
    for(int i = 0; i < size; i++){
        x[i] += s[i] * dt;
    }
}

GAME_API void gameStart(Arena* gameArena){
    if(gameArena->index > 0){
        return;
    }
    GameState* gameState = arenaAllocStruct(gameArena, GameState);
    gameState->arena = gameArena;
    // Resolution-independent camera: shows 640 world units horizontally, 360 vertically, centered at origin
    // Bounds: -360 to +360 horizontally, -160 to +160 vertically
    gameState->restart = false;
    //gameState->mainCamera = createCamera(-1920.0f / 2, 1920.0f / 2, -1080.0f / 2, 1080.0f / 2);
    gameState->mainCamera = createCamera(-640.0f / 2, 640.0f / 2, -360.0f / 2, 360.0f / 2);
    setActiveCamera(&gameState->mainCamera);
    gameState->u = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
    gameState->source = arenaAllocArrayZero(gameState->arena, float, (GRID_SIZE + 2) * (GRID_SIZE + 2)); // +2 for boundary conditions
}

GAME_API void gameRender(Arena* gameArena, float dt){}

GAME_API void gameUpdate(Arena* gameArena, float dt){
    GameState* gameState = (GameState*)gameArena->memory;

    beginScene();
        clearColor(0.2f, 0.3f, 0.3f, 1.0f);
        beginMode2D(gameState->mainCamera);
        drawGrid(gameState->u);
        endMode2D();
    endScene();
}

GAME_API void gameStop(Arena* gameArena){
}