#include "projectx.hpp"
#include <glm/glm.hpp>
#include <cmath>


#define GRID_SIZE 50
#define GRID_SIZE_Y 50
#define GRID_SIZE_X GRID_SIZE_Y * 2
#define CELL_SIZE 5

#define SIM_W 1280
#define SIM_H 720

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
    loadTexture("1");
    gs->vx = loadRenderTexture(SIM_W+1, SIM_H, TEXTURE_R32F);
    gs->vy = loadRenderTexture(SIM_W, SIM_H+1, TEXTURE_R32F);
    gs->vxPrev = loadRenderTexture(SIM_W+1, SIM_H, TEXTURE_R32F);
    gs->vyPrev = loadRenderTexture(SIM_W, SIM_H+1, TEXTURE_R32F);
    setTextureFilter(&gs->vx.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureFilter(&gs->vy.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureFilter(&gs->vxPrev.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureFilter(&gs->vyPrev.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->vx.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureWrap(&gs->vy.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureWrap(&gs->vxPrev.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureWrap(&gs->vyPrev.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    gs->advect = loadShader(gs->arena, "shaders/advect-shader.vs", "shaders/advect-shader.fs");
    gs->divShader = loadShader(gs->arena, "shaders/div-shader.vs", "shaders/div-shader.fs");
    gs->pShader = loadShader(gs->arena, "shaders/pressure-shader.vs", "shaders/pressure-shader.fs");
    gs->test = loadShader(gs->arena, "shaders/display-shader.vs", "shaders/display-shader.fs");
    gs->addSource = loadShader(gs->arena, "shaders/add-source-shader.vs", "shaders/add-source-shader.fs");
    gs->divTexture = loadRenderTexture(SIM_W, SIM_H, TEXTURE_R32F);
    gs->pTexture = loadRenderTexture(SIM_W, SIM_H, TEXTURE_R32F);
    gs->pTexturePrev = loadRenderTexture(SIM_W, SIM_H, TEXTURE_R32F);
    setTextureFilter(&gs->divTexture.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->divTexture.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureFilter(&gs->pTexture.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->pTexture.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureFilter(&gs->pTexturePrev.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->pTexturePrev.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);

    gs->densTexture = loadRenderTexture(SIM_W, SIM_H, TEXTURE_RGBA32F);
    gs->densTexturePrev = loadRenderTexture(SIM_W, SIM_H,TEXTURE_RGBA32F);
    setTextureFilter(&gs->densTexture.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->densTexture.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);
    setTextureFilter(&gs->densTexturePrev.texture, TEXTURE_FILTER_NEAREST, TEXTURE_FILTER_NEAREST);
    setTextureWrap(&gs->densTexturePrev.texture, TEXTURE_WRAP_CLAMP_TO_EDGE, TEXTURE_WRAP_CLAMP_TO_EDGE);

    #if 0 
    beginTextureMode(&gs->densTexture, false);
        renderDrawQuad2D({0,0}, gs->densTexture.texture.size, 0, getTextureByName("1"), {1,1,1,1});
    endTextureMode();

    beginTextureMode(&gs->densTexturePrev, false);
        renderDrawQuad2D({0,0}, gs->densTexturePrev.texture.size, 0, getTextureByName("1"), {1,1,1,1});
    endTextureMode();
    #endif

    gs->imageVx = arenaAllocArrayZero(gs->arena, float, (SIM_W+1)*SIM_H);
    gs->imageVy = arenaAllocArrayZero(gs->arena, float, SIM_W*(SIM_H+1));
    gs->imageDens = arenaAllocArrayZero(gs->arena, float, SIM_W*SIM_H);
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
    //LOGINFO("FPS: %f", getFPS());

    float xo = -(GRID_SIZE)*CELL_SIZE*0.5f;
    float yo = -(GRID_SIZE)*CELL_SIZE*0.5f;
    int mi = (int)((mouseWorld.x - xo) / CELL_SIZE);
    int mj = (int)((mouseWorld.y - yo) / CELL_SIZE);

    xo = -(GRID_SIZE_X)*CELL_SIZE*0.5f;
    yo = -(GRID_SIZE_Y)*CELL_SIZE*0.5f;
    mi = (int)((mouseWorld.x - xo) / CELL_SIZE);
    mj = (int)((mouseWorld.y - yo) / CELL_SIZE);
    if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        glm::vec2 delta = (mouseWorld - gs->mousePrev) / (float)CELL_SIZE;
        float strength = 2.0f;
        int radius = 5;
        for(int j = -radius; j < radius; j++){
            for(int i = -radius; i < radius; i++){
                int cj = mj + j;
                int ci = mi + i;
                if(ci>=1 && ci<GRID_SIZE_X-1 && cj>=1 && cj<GRID_SIZE_Y-1){
                    gs->fs.u[uij(&gs->fs, ci, cj)] += delta.x * strength;
                    gs->fs.v[vij(&gs->fs, ci, cj)] += delta.y * strength;
                    gs->fs.dens[ij(&gs->fs, ci, cj)] = 1.0f;
                }
            }
        }
    }
    gs->mousePrev = mouseWorld;

    //step(&gs->fs, dt);

    beginScene();
    clearColor(0.1f,0.1f,0.1f,1);
    beginMode2D(gs->mainCamera);

    //drawGrid(&gs->fs);
    //drawVelocities(&gs->fs);
    //drawDensity(&gs->fs);
    //drawPressure(&gs->fs);
    //drawDivergence(&gs->fs);


    endMode2D();
    endScene();

    #if 1

    Rect sourceRect = {.pos = {0,0}, .size{ gs->vx.texture.width, gs->vx.texture.height}};
    RenderTexture* srcVx = gs->pingPong ? &gs->vx : &gs->vxPrev;
    RenderTexture* dstVx = gs->pingPong ? &gs->vxPrev : &gs->vx;

    RenderTexture* srcVy = gs->pingPong ? &gs->vy : &gs->vyPrev;
    RenderTexture* dstVy = gs->pingPong ? &gs->vyPrev : &gs->vy;

    disableBlending();

    RenderTexture* srcDens = gs->pingPongDens ? &gs->densTexture : &gs->densTexturePrev;
    RenderTexture* dstDens = gs->pingPongDens ? &gs->densTexturePrev : &gs->densTexture;

    if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        #if 0
        float quadLeft = getScreenSize().x / 2.0f - SIM_W / 2.0f;
        float quadBottom = getScreenSize().y / 2.0f + SIM_H / 2.0f;
        float mouseY = getScreenSize().y - mouseScreen.y;
        float texX = mouseScreen.x - quadLeft;
        float texY = quadBottom - mouseY;
        beginTextureMode(srcDens, false);
            beginShaderMode(&gs->addSource);
                setUniform(&gs->addSource, "mousePos", mouseScreen);
                setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W, SIM_H));
                setUniform(&gs->addSource, "screenSize", getScreenSize());
                setUniform(&gs->addSource, "mode", 0);
                renderDrawFilledRect({texX,texY},{(float)10, (float)10},0,{1,1,1,1});
            endShaderMode();
        endTextureMode();

        beginTextureMode(srcVx, false);
            beginShaderMode(&gs->addSource);
                setUniform(&gs->addSource, "mousePos", mouseScreen);
                setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W+1, SIM_H));
                setUniform(&gs->addSource, "mode", 1);
                setUniform(&gs->addSource, "screenSize", getScreenSize());
                renderDrawFilledRect({texX,texY},{(float)10, (float)10},0,{1,1,1,1});
            endShaderMode();
        endTextureMode();
        
        beginTextureMode(srcVy, false);
            beginShaderMode(&gs->addSource);
                setUniform(&gs->addSource, "mousePos", mouseScreen);
                setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W, SIM_H+1));
                setUniform(&gs->addSource, "mode", 2);
                setUniform(&gs->addSource, "screenSize", getScreenSize());
                renderDrawFilledRect({mouseScreen.x,mouseScreen.y},{(float)10, (float)10},0,{1,1,1,1});
            endShaderMode();
        endTextureMode();
        #else
        float* vxData = (float*)gs->imageVx;
        float* vyData = (float*)gs->imageVy;
        float* densData = (float*)gs->imageDens;
        getImageFromTexture(vxData, &srcVx->texture, TEXTURE_R32F);
        getImageFromTexture(vyData, &srcVy->texture, TEXTURE_R32F);
        getImageFromTexture(densData, &srcDens->texture, TEXTURE_R32F);
        float quadLeft = getScreenSize().x / 2.0f - SIM_W / 2.0f;
        float quadBottom = getScreenSize().y / 2.0f + SIM_H / 2.0f;
        float mouseY = getScreenSize().y - mouseScreen.y;
        float texX = mouseScreen.x - quadLeft;
        float texY = quadBottom - mouseY;
        float deltax = mouseScreen.x - gs->mousePrevScreen.x;
        float deltay = mouseScreen.y - gs->mousePrevScreen.y;
        float force = 0.2f;
        int radius = 10;
        int velRadius = 10;

        int ci = (int)texX;
        int cj = (int)texY;

        for(int j = 0; j < SIM_H; j++){
            for(int i = 0; i <= SIM_W; i++){
                vxData[i + (SIM_W+1) * j] = 5.0f;
            }
        }
        for(int j = SIM_H / 2 - 25; j <= SIM_H / 2 + 25; j++){
            densData[1 + (SIM_W) * j] = 1.0f;
        }


        for(int dj = -velRadius; dj <= velRadius; dj++){
            for(int di = -velRadius; di <= velRadius; di++){
                int ix = ci + di;
                int jy = cj + dj;
                if(ix >= 0 && ix < SIM_W+1 && jy >= 0 && jy < SIM_H){
                    vxData[ix + (SIM_W+1) * jy] += deltax * force;
                }
            }
        }
        for(int dj = -velRadius; dj <= velRadius; dj++){
            for(int di = -velRadius; di <= velRadius; di++){
                int ix = ci + di;
                int jy = cj + dj;
                if(ix >= 0 && ix < SIM_W && jy >= 0 && jy < SIM_H+1){
                    vyData[ix + SIM_W * jy] += deltay * force;
                }
            }
        }
        // Inject density at cursor
        for(int dj = -radius; dj <= radius; dj++){
            for(int di = -radius; di <= radius; di++){
                int ix = ci + di;
                int jy = cj + dj;
                if(ix >= 0 && ix < SIM_W && jy >= 0 && jy < SIM_H){
                    densData[ix + SIM_W * jy] = 1.0f;
                }
            }
        }

        setImageToTexture(vxData, &srcVx->texture, TEXTURE_R32F);
        setImageToTexture(vyData, &srcVy->texture, TEXTURE_R32F);
        setImageToTexture(densData, &srcDens->texture, TEXTURE_R32F);
        #endif
    }
    gs->mousePrevScreen = mouseScreen;
    

    //beginTextureMode(srcVx, false);
    //    beginShaderMode(&gs->advect);
    //    bindTextureToShader(&gs->advect, "textureVx", dstVx->texture.id, 0);
    //    bindTextureToShader(&gs->advect, "textureVy", dstVy->texture.id, 1);
    //    setUniform(&gs->advect, "textureSize", glm::vec2(640, 320));
    //    setUniform(&gs->advect, "dt", dt);
    //    setUniform(&gs->advect, "mode", 3);
    //    renderDrawFilledRect({0,0},{(float)srcVx->texture.width, (float)srcVx->texture.height},0,{1,1,1,1});
    //    endShaderMode();
    //endTextureMode();


    beginTextureMode(dstVx, false);
        beginShaderMode(&gs->advect);
        bindTextureToShader(&gs->advect, "textureVx", srcVx->texture.id, 0);
        bindTextureToShader(&gs->advect, "textureVy", srcVy->texture.id, 1);
        setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W+1, SIM_H));
        setUniform(&gs->advect, "textureSizeOther", glm::vec2(SIM_W, SIM_H+1));
        setUniform(&gs->advect, "dt", dt);
        setUniform(&gs->advect, "mode", 0);
        renderDrawFilledRect({0,0},{(float)dstVx->texture.width, (float)dstVx->texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();

    beginTextureMode(dstVy, false);
        beginShaderMode(&gs->advect);
        bindTextureToShader(&gs->advect, "textureVx", srcVx->texture.id, 0);
        bindTextureToShader(&gs->advect, "textureVy", srcVy->texture.id, 1);
        setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W, SIM_H+1));
        setUniform(&gs->advect, "textureSizeOther", glm::vec2(SIM_W+1, SIM_H));
        setUniform(&gs->advect, "dt", dt);
        setUniform(&gs->advect, "mode", 1);
        renderDrawFilledRect({0,0},{(float)dstVy->texture.width, (float)dstVy->texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();

    // Divergence only needs to be computed once (doesn't change during Jacobi iterations)
    beginTextureMode(&gs->divTexture, false);
        beginShaderMode(&gs->divShader);
        bindTextureToShader(&gs->divShader, "textureVx", dstVx->texture.id, 0);
        bindTextureToShader(&gs->divShader, "textureVy", dstVy->texture.id, 1);
        setUniform(&gs->divShader, "textureSize", glm::vec2(SIM_W, SIM_H));
        setUniform(&gs->divShader, "dt", dt);
        renderDrawFilledRect({0,0},{(float)gs->divTexture.texture.width, (float)gs->divTexture.texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();

    RenderTexture* pFinal = NULL;
    for(int i = 0; i < 10; i++){
        RenderTexture* psrc = (gs->pingPongPressure) ? &gs->pTexturePrev : &gs->pTexture;
        RenderTexture* pdst = (gs->pingPongPressure) ? &gs->pTexture     : &gs->pTexturePrev;

        beginTextureMode(pdst, false);
            beginShaderMode(&gs->pShader);
            bindTextureToShader(&gs->pShader, "divTexture", gs->divTexture.texture.id, 0);
            bindTextureToShader(&gs->pShader, "pPrev", psrc->texture.id, 1);
            setUniform(&gs->pShader, "textureSize", glm::vec2(SIM_W, SIM_H));
            setUniform(&gs->pShader, "dt", dt);
            renderDrawFilledRect({0,0},{(float)pdst->texture.width, (float)pdst->texture.height},0,{1,1,1,1});
            endShaderMode();
        endTextureMode();
        pFinal = pdst;
        gs->pingPongPressure = !gs->pingPongPressure;
    }

    // Projection: subtract pressure gradient from velocity
    // Project u: dstVx -> srcVx (srcVx is free after advection)
    beginTextureMode(srcVx, false);
        beginShaderMode(&gs->advect);
        bindTextureToShader(&gs->advect, "textureVx", dstVx->texture.id, 0);
        bindTextureToShader(&gs->advect, "pTexture", pFinal->texture.id, 2);
        setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W+1, SIM_H));
        setUniform(&gs->advect, "textureSizeOther", glm::vec2(SIM_W, SIM_H));
        setUniform(&gs->advect, "dt", dt);
        setUniform(&gs->advect, "mode", 4);
        renderDrawFilledRect({0,0},{(float)srcVx->texture.width, (float)srcVx->texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();

    // Project v: dstVy -> srcVy (srcVy is free after advection)
    beginTextureMode(srcVy, false);
        beginShaderMode(&gs->advect);
        bindTextureToShader(&gs->advect, "textureVy", dstVy->texture.id, 1);
        bindTextureToShader(&gs->advect, "pTexture", pFinal->texture.id, 2);
        setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W, SIM_H+1));
        setUniform(&gs->advect, "textureSizeOther", glm::vec2(SIM_W, SIM_H));
        setUniform(&gs->advect, "dt", dt);
        setUniform(&gs->advect, "mode", 5);
        renderDrawFilledRect({0,0},{(float)srcVy->texture.width, (float)srcVy->texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();

    // Advect density: srcDens -> dstDens using projected velocity (srcVx/srcVy)
    beginTextureMode(dstDens, false);
        beginShaderMode(&gs->advect);
        bindTextureToShader(&gs->advect, "textureVx", srcVx->texture.id, 0);
        bindTextureToShader(&gs->advect, "textureVy", srcVy->texture.id, 1);
        bindTextureToShader(&gs->advect, "densTexture", srcDens->texture.id, 3);
        setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W, SIM_H));
        setUniform(&gs->advect, "dt", dt);
        setUniform(&gs->advect, "mode", 6);
        renderDrawFilledRect({0,0},{(float)dstDens->texture.width, (float)dstDens->texture.height},0,{1,1,1,1});
        endShaderMode();
    endTextureMode();
    gs->pingPongDens = !gs->pingPongDens;

    enableBlending();

    // Projected result is now in srcVx/srcVy, so DON'T flip pingPong
    // (next frame, src will be these same textures)

    // Render density as fullscreen quad (grayscale)
    beginScene();
        beginShaderMode(&gs->test);
        bindTextureToShader(&gs->test, "textureIn", dstDens->texture.id, 0);
        glm::vec2 screen = getScreenSize();
        renderDrawFilledRect(
            {screen.x / 2.0f - SIM_W / 2.0f, screen.y / 2.0f - SIM_H / 2.0f},
            {(float)SIM_W*1, (float)SIM_H*1}, 0, {1,1,1,1}
        );
        endShaderMode();
    endScene();
    #endif
}

GAME_API void gameRender(Arena*,float){}
GAME_API void gameStop(Arena*){}
