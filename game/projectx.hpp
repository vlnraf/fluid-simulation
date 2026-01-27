#pragma once

#if defined(_WIN32)
    #ifdef GAME_EXPORT
        #define GAME_API __declspec(dllexport)
    #else
        #define GAME_API __declspec(dllimport)
    #endif
#else
    // Linux / macOS
    #ifdef GAME_EXPORT
        #define GAME_API __attribute__((visibility("default")))
    #else
        #define GAME_API
    #endif
#endif

#include "core.hpp"

struct GameState{
    Arena* arena;
    OrtographicCamera mainCamera;
    float* u;
    float* uPrev;
    float* v;
    float* vPrev;
    float* dens;
    float* densPrev;
    float* source;
    bool restart;

    bool pause = false;
};

extern "C" {
    GAME_API void gameStart(Arena* gameArena);
    GAME_API void gameRender(Arena* gameArena, float dt);
    GAME_API void gameUpdate(Arena* gameArena, float dt);
    //GAME_API GameState* gameReload(GameState* gameState, Renderer* renderer, const char* filePath);
    GAME_API void gameStop(Arena* gameArena);
}