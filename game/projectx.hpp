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
#include "fluidsimulator.hpp"

struct GameState{
    Arena* arena;
    OrtographicCamera mainCamera;
    FluidSimulator fs;
    float* u;
    float* uPrev;
    float* v;
    float* vPrev;
    float* dens;
    float* densPrev;

    float *p;
    float *div;

    bool* solid;

    RenderTexture vx;
    RenderTexture vy;
    RenderTexture vxPrev;
    RenderTexture vyPrev;
    RenderTexture pTexture;
    RenderTexture pTexturePrev;
    RenderTexture divTexture;
    RenderTexture densTexture;
    RenderTexture densTexturePrev;
    bool pingPongDens = false;
    OrtographicCamera texCamera;
    Shader advect;
    Shader test;
    Shader divShader;
    Shader pShader;
    bool pingPong = false;
    bool pingPongPressure = false;
    void* imageVx;
    void* imageVy;
    void* imageDens;

    glm::vec2 mousePrev;
    glm::vec2 mousePrevScreen;

    float diff;
    float visc;

    bool startSim;

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