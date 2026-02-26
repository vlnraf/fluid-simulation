#include "projectx.hpp"
#include <glm/glm.hpp>
#include <cmath>


#define GRID_SIZE 50
#define GRID_SIZE_Y 50
#define GRID_SIZE_X GRID_SIZE_Y * 2
#define CELL_SIZE 5

#define SIM_W 1280
#define SIM_H 720

glm::vec2 mousePos;
uint32_t generator = 1;
uint32_t hover = 0;
uint32_t active = 0;
Font f;
Font fSmall;
Font fBig;


struct Context{
    glm::vec2 pos;
    glm::vec2 size;
    glm::vec2 ctxSize;
    float padding;
    int items;
};

static Context c = {};

bool aabb(glm::vec2 mousePos, glm::vec2 widgetPos, glm::vec2 widgetSize){
    return (mousePos.x >= widgetPos.x && mousePos.x <= widgetPos.x + widgetSize.x &&
            mousePos.y >= widgetPos.y && mousePos.y <= widgetPos.y + widgetSize.y);
}

void drawRightPanel(Arena* arena, glm::vec2 panelPos, glm::vec2 panelSize, int padding, int columns){
    renderDrawFilledRect(panelPos, panelSize, 0, {0.3,0.3,0.3,1});
    glm::vec2 buttonSize = {(panelSize.x - padding * 2), (panelSize.y - padding * (columns + 1)) / (columns)};
    //column division
    glm::vec2 prevPos = {panelPos.x + padding , panelPos.y + panelSize.y - buttonSize.y - padding};
    for(int i = 0; i < columns; i++){
        glm::vec2 newPos = {prevPos.x, prevPos.y};
        renderDrawFilledRect(newPos, buttonSize, 0, {1.0,0.0,0.0,1});
        String8 text = pushString8F(arena, "Ciao %d", i);
        float tWidth = calculateTextWidth(&f, text.str, 1);
        float tHeight = calculateTextHeight(&f, text.str, 1);
        glm::vec2 textPos = (newPos + buttonSize * 0.5f);
        textPos.x -= tWidth * 0.5f;
        textPos.y -= (tHeight * 0.5f);
        renderDrawText2D(&f, text.str, textPos, 1);
        prevPos = {newPos.x, newPos.y - buttonSize.y - padding};
    }
}


//void drawTextBox(Arena* a, glm::vec2 pos, glm::vec2 size){
//    TempArena tmp = getTempArena(a);
//    Font* f = getFont("Roboto-Regular");
//    renderDrawFilledRect(pos, size, 0, {0.3,0.3,0.3,1});
//    String8 text;
//    if(aabb(mousePos, pos, size)){
//        if(isJustPressed(KEYS::A)){
//            text = pushString8F(tmp.arena, "%c", 'a');
//        }
//    }
//    String8 result = pushString8F(a, "%S", text);
//    renderDrawText2D(&f, result.str, pos, 1);
//    releaseTempArena(tmp);
//}

bool button(String8 label, glm::vec2 pos = {0,0}, glm::vec2 size = {0,0}){
    uint32_t id = generator++;

    float tWidth = calculateTextWidth(&f, label.str, 1);
    float tHeight = calculateTextHeight(&f, label.str, 1);
    glm::vec2 textPos = pos;
    if(glm::length(size) == 0){
        size = {tWidth, tHeight};
    }
    if(glm::length(pos) == 0){
        c.items++;
        pos = {c.pos.x , c.pos.y + c.ctxSize.y - c.padding - size.y};
        textPos = pos;
        c.ctxSize.y -= size.y + c.padding;
    }

    glm::vec4 color = {0.3,0.3,0.3,1};
    if(aabb(mousePos, pos, size)){
        hover = id;
    }
    if(hover == id){
        if(isMouseButtonJustPressed(MOUSE_BUTTON_LEFT)){
            color = {1.0f, 0.0f, 0.0f, 1.0f};
            active = id;
        }
    }
    renderDrawFilledRect(pos, size, 0, {0.5,0.5,0.5,1});
    renderDrawText2D(&f, label.str, textPos, 1);
    return active == id;
}

bool checkBox(String8 label, bool *value, glm::vec2 pos = {0,0}, glm::vec2 size = {0,0}){
    uint32_t id = generator++;

    float tWidth = calculateTextWidth(&f, label.str, 1);
    float tHeight = calculateTextHeight(&f, label.str, 1);
    glm::vec2 textPos = pos;
    if(glm::length(size) == 0){
        size = {tHeight, tHeight};
    }
    if(glm::length(pos) == 0){
        c.items++;
        pos = {c.pos.x + tWidth, c.pos.y + c.ctxSize.y - c.padding - size.y};
        textPos = {c.pos.x, c.pos.y + c.ctxSize.y - c.padding - size.y};
        c.ctxSize.y -= size.y + c.padding;
    }

    glm::vec4 color = {0.3,0.3,0.3,1};
    if(aabb(mousePos, pos, size)){
        hover = id;
    }
    if(hover == id){
        if(isMouseButtonJustPressed(MOUSE_BUTTON_LEFT)){
            color = {1.0f, 0.0f, 0.0f, 1.0f};
            active = id;
            *value = !(*value);
        }
    }
    renderDrawFilledRect(pos, size, 0, {0.5,0.5,0.5,1});
    if(*value){
        color = {1.0f, 1.0f, 0.0f, 1.0f};
        renderDrawFilledRect(pos + size * 0.5f * 0.5f, size * 0.5f, 0, color);
    }
    renderDrawText2D(&f, label.str, textPos, 1);
    return active == id;
}

bool slider(Arena* a, String8 label, float* value, float min, float max, glm::vec2 pos = {0,0}){
    uint32_t id = generator++;

    String8 text = pushString8F(a, "%.1f", *value);
    float tWidth = calculateTextWidth(&f, text.str, 1);
    float tHeight = calculateTextHeight(&f, text.str, 1);

    float labelWidth = calculateTextWidth(&f, label.str, 1);
    c.pos.x += labelWidth;
    glm::vec2 labelPos;

    glm::vec2 size = {c.ctxSize.x - labelWidth - c.padding, tHeight};
    if(glm::length(pos) == 0){
        c.items++;
        labelPos = {c.pos.x, c.pos.y + c.ctxSize.y - c.padding - size.y};
        pos = {c.pos.x + c.padding, c.pos.y + c.ctxSize.y - c.padding - size.y};
        c.ctxSize.y -= size.y + c.padding;
    }
    glm::vec4 color = {0.3,0.3,0.3,1};
    glm::vec2 sSize = {size.x * 0.1f , size.y};
    glm::vec2 sPos = {pos.x + ((*value / max) * (size.x)) - sSize.x * 0.5f, pos.y };
    if(aabb(mousePos, pos, size)){
        hover = id;
        color = {1, 0, 1, 1};
    }

    if(hover == id){
        if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            active = id;
            sPos.x = mousePos.x - sSize.x * 0.5f;
            float norm = ((sPos.x - pos.x + sSize.x * 0.5f) / size.x) * max;
            *value = glm::clamp(norm, min, max);
        }
    }
    glm::vec2 textPos = {pos.x + size.x * 0.5f - tWidth * 0.5f, pos.y};

    renderDrawText2D(&f, label.str, {labelPos.x - labelWidth, labelPos.y}, 1);
    renderDrawText2D(&f, text.str, {textPos.x, textPos.y}, 1);
    renderDrawFilledRect({pos.x, pos.y}, size, 0, color);
    renderDrawFilledRect({sPos.x, sPos.y}, sSize, 0, {1.0,1.0,1.0,0.5});
    c.pos.x -= labelWidth;

    return active == id;
}

bool sliderInt(Arena* a, String8 label, int* value, int min, int max, glm::vec2 pos = {0,0}){
    uint32_t id = generator++;

    String8 text = pushString8F(a, "%d", *value);
    float tWidth = calculateTextWidth(&f, text.str, 1);
    float tHeight = calculateTextHeight(&f, text.str, 1);

    float labelWidth = calculateTextWidth(&f, label.str, 1);
    c.pos.x += labelWidth;
    glm::vec2 labelPos;

    glm::vec2 size = {c.ctxSize.x - labelWidth - c.padding, tHeight};
    if(glm::length(pos) == 0){
        c.items++;
        labelPos = {c.pos.x, c.pos.y + c.ctxSize.y - c.padding - size.y};
        pos = {c.pos.x + c.padding, c.pos.y + c.ctxSize.y - c.padding - size.y};
        c.ctxSize.y -= size.y + c.padding;
    }
    glm::vec4 color = {0.3,0.3,0.3,1};
    glm::vec2 sSize = {size.x * 0.1f , size.y};
    glm::vec2 sPos = {pos.x + (((float)*value / max) * (size.x)) - sSize.x * 0.5f, pos.y };
    if(aabb(mousePos, pos, size)){
        hover = id;
        color = {1, 0, 1, 1};
    }

    if(hover == id){
        if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            active = id;
            sPos.x = mousePos.x - sSize.x * 0.5f;
            int norm = glm::floor(((sPos.x - pos.x + sSize.x * 0.5f) / size.x) * max);
            *value = (int)glm::clamp(norm, min, max);
        }
    }
    glm::vec2 textPos = {pos.x + size.x * 0.5f - tWidth * 0.5f, pos.y};

    renderDrawText2D(&f, label.str, {labelPos.x - labelWidth, labelPos.y}, 1);
    renderDrawText2D(&f, text.str, {textPos.x, textPos.y}, 1);
    renderDrawFilledRect({pos.x, pos.y}, size, 0, color);
    renderDrawFilledRect({sPos.x, sPos.y}, sSize, 0, {1.0,1.0,1.0,0.5});
    c.pos.x -= labelWidth;

    return active == id;
}

void beginPanel(String8 title, glm::vec2 pos, glm::vec2 size){
    c.pos = pos;
    c.size = size;
    c.padding = 10;
    glm::vec4 color = {0.3f, 0.3f, 0.3f, 1.0f};
    glm::vec4 barColor = {0.0f, 0.0f, 1.0f, 1.0f};
    float textHeight = calculateTextHeight(&f, title.str, 1);
    glm::vec2 barSize = {c.size.x, textHeight};
    glm::vec2 barPos = {c.pos.x, c.pos.y + c.size.y - barSize.y};
    c.ctxSize.x = c.size.x;
    c.ctxSize.y = c.size.y - barSize.y;
    //Content
    renderDrawFilledRect(c.pos, c.size, 0, color);
    //TitleBar
    renderDrawFilledRect(barPos, barSize, 0, barColor);
    renderDrawText2D(&f, title.str, barPos, 1);
}

void endPanel(){
    c.pos = {0,0};
    c.size = getScreenSize();
    c.ctxSize = getScreenSize();
    c.padding = 10;
    c.items = 0;
}

void drawHud(GameState* gs, Arena* a){
    generator = 1;
    active = 0;
    hover = 0;
    TempArena tmp = getTempArena(a);
    glm::vec2 screenSize = getScreenSize();
    //LOGINFO("%f, %f", screenSize.x, screenSize.y);

    glm::vec2 panelSize = {250,250};
    glm::vec2 panelPos = {screenSize.x - 300, screenSize.y * 0.5f - panelSize.y * 0.5f};
    float padding = 10;
    int columns = 5;
    beginScene(NO_DEPTH);
        //Right Panel
        //drawRightPanel(tmp.arena, panelPos, panelSize, padding, columns);
        //columns = 5;
        beginPanel(String8Lit("Test"), panelPos, panelSize);
            //slider(tmp.arena, &gs->radius, 1, 50, {50,100});
            //checkBox(&gs->dye, {100, 200}, {50,50});
            //checkBox(&gs->vel, {100, 400}, {50,50});
            slider(tmp.arena, String8Lit("radius"), &gs->radius, 1, 50);
            checkBox(String8Lit("Dye"), &gs->dye);
            checkBox(String8Lit("vel"), &gs->vel);
            sliderInt(tmp.arena, String8Lit("iterations"), &gs->iterations, 1, 1000);
            checkBox(String8Lit("CPU"), &gs->cpu);
            checkBox(String8Lit("Wind"), &gs->windTunnel);
            //if(button(String8Lit("Density"))){
            //    gs->s = SIM_DENSITY;
            //}
            //if(button(String8Lit("Pressure"))){
            //    gs->s = SIM_PRESSURE;
            //}
        endPanel();

        //drawTextBox(a, {500, 100}, {100,50});
    endScene();

    //horizontal layout
    //panelPos = {screenSize.x - 300, 50};
    //panelSize = {250,900};

    releaseTempArena(tmp);
}

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

    f = loadFont("Roboto-Regular", 24);
    fSmall = loadFont("Roboto-Regular", 12);
    fBig = loadFont("Roboto-Regular");
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
    gs->vel = false;
    gs->dye = false;
    gs->radius = 5;
    gs->iterations = 100;
    gs->cpu = false;
    gs->s = SIM_DENSITY;
}

GAME_API void gameUpdate(Arena* arena,float dt){
    GameState* gs=(GameState*)arena->memory;
    dt = 0.1f;
    mousePos = getMousePos();

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

    if(gs->cpu){
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

        step(&gs->fs, dt);

        beginScene();
        clearColor(0.1f,0.1f,0.1f,1);
        beginMode2D(gs->mainCamera);

        if(gs->s == SIM_DENSITY){
            drawDensity(&gs->fs);
        }else if( gs->s == SIM_PRESSURE){
            drawPressure(&gs->fs);
        }

        //drawGrid(&gs->fs);
        //drawVelocities(&gs->fs);
        //drawDivergence(&gs->fs);


        endMode2D();
        endScene();
    }else{
        Rect sourceRect = {.pos = {0,0}, .size{ gs->vx.texture.width, gs->vx.texture.height}};
        RenderTexture* srcVx = gs->pingPong ? &gs->vx : &gs->vxPrev;
        RenderTexture* dstVx = gs->pingPong ? &gs->vxPrev : &gs->vx;

        RenderTexture* srcVy = gs->pingPong ? &gs->vy : &gs->vyPrev;
        RenderTexture* dstVy = gs->pingPong ? &gs->vyPrev : &gs->vy;

        disableBlending();

        RenderTexture* srcDens = gs->pingPongDens ? &gs->densTexture : &gs->densTexturePrev;
        RenderTexture* dstDens = gs->pingPongDens ? &gs->densTexturePrev : &gs->densTexture;

        if(gs->windTunnel){
            beginTextureMode(srcDens, false);
                beginShaderMode(&gs->addSource);
                    setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W, SIM_H));
                    setUniform(&gs->addSource, "screenSize", getScreenSize());
                    setUniform(&gs->addSource, "radius", gs->radius);
                    setUniform(&gs->addSource, "mode", 3);
                    renderDrawFilledRect({1,SIM_H/2 - 25},{1,50},0,{1,1,1,1});
                endShaderMode();
            endTextureMode();
            beginTextureMode(srcVx, false);
                beginShaderMode(&gs->addSource);
                    setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W+1, SIM_H));
                    setUniform(&gs->addSource, "radius", gs->radius);
                    setUniform(&gs->addSource, "mode", 4);
                    setUniform(&gs->addSource, "screenSize", getScreenSize());
                    renderDrawFilledRect({0,0},{10,SIM_H},0,{1,1,1,1});
                endShaderMode();
            endTextureMode();
        }
        if(isMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            float quadLeft = getScreenSize().x / 2.0f - SIM_W / 2.0f;
            float quadBottom = getScreenSize().y / 2.0f + SIM_H / 2.0f;
            float mouseY = getScreenSize().y - mouseScreen.y;
            float texX = mouseScreen.x - quadLeft;
            float texY = quadBottom - mouseY;
            float deltax = mouseScreen.x - gs->mousePrevScreen.x;
            float deltay = mouseScreen.y - gs->mousePrevScreen.y;
            if(gs->dye){
                beginTextureMode(srcDens, false);
                    beginShaderMode(&gs->addSource);
                        setUniform(&gs->addSource, "mousePos", mouseScreen);
                        setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                        setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W, SIM_H));
                        setUniform(&gs->addSource, "screenSize", getScreenSize());
                        setUniform(&gs->addSource, "radius", gs->radius);
                        setUniform(&gs->addSource, "mode", 0);
                        renderDrawFilledRect({texX,texY},{gs->radius, gs->radius},0,{1,1,1,1});
                    endShaderMode();
                endTextureMode();
            }
            if(gs->vel){
                beginTextureMode(srcVx, false);
                    beginShaderMode(&gs->addSource);
                        setUniform(&gs->addSource, "mousePos", mouseScreen);
                        setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                        setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W+1, SIM_H));
                        setUniform(&gs->addSource, "radius", gs->radius);
                        setUniform(&gs->addSource, "mode", 1);
                        setUniform(&gs->addSource, "screenSize", getScreenSize());
                        renderDrawFilledRect({texX,texY},{gs->radius, gs->radius},0,{1,1,1,1});
                    endShaderMode();
                endTextureMode();
                
                beginTextureMode(srcVy, false);
                    beginShaderMode(&gs->addSource);
                        setUniform(&gs->addSource, "mousePos", mouseScreen);
                        setUniform(&gs->addSource, "mousePosPrev", gs->mousePrevScreen);
                        setUniform(&gs->addSource, "textureSize", glm::vec2(SIM_W, SIM_H+1));
                        setUniform(&gs->addSource, "radius", gs->radius);
                        setUniform(&gs->addSource, "mode", 2);
                        setUniform(&gs->addSource, "screenSize", getScreenSize());
                        renderDrawFilledRect({texX,texY},{gs->radius, gs->radius},0,{1,1,1,1});
                    endShaderMode();
                endTextureMode();
            }
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


        if(gs->windTunnel != gs->windTunnelPrev){
            gs->windTunnelPrev = gs->windTunnel;
            clearColor(0, 0, 0, 0);
            RenderTexture* toClear[] = {
                &gs->vx, &gs->vxPrev, &gs->vy, &gs->vyPrev,
                &gs->densTexture, &gs->densTexturePrev,
                &gs->pTexture, &gs->pTexturePrev, &gs->divTexture
            };
            for(auto* rt : toClear){
                beginTextureMode(rt, true);
                endTextureMode();
            }
        }

        int boundaryMode = gs->windTunnel ? 1 : 0;

        beginTextureMode(dstVx, false);
            beginShaderMode(&gs->advect);
            bindTextureToShader(&gs->advect, "textureVx", srcVx->texture.id, 0);
            bindTextureToShader(&gs->advect, "textureVy", srcVy->texture.id, 1);
            setUniform(&gs->advect, "textureSize", glm::vec2(SIM_W+1, SIM_H));
            setUniform(&gs->advect, "textureSizeOther", glm::vec2(SIM_W, SIM_H+1));
            setUniform(&gs->advect, "dt", dt);
            setUniform(&gs->advect, "mode", 0);
            setUniform(&gs->advect, "boundaryMode", boundaryMode);
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
            setUniform(&gs->advect, "boundaryMode", boundaryMode);
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

        gs->pingPongPressure = false;
        RenderTexture* pFinal = NULL;
        for(int i = 0; i < gs->iterations; i++){
            RenderTexture* psrc = (gs->pingPongPressure) ? &gs->pTexturePrev : &gs->pTexture;
            RenderTexture* pdst = (gs->pingPongPressure) ? &gs->pTexture     : &gs->pTexturePrev;

            beginTextureMode(pdst, false);
                beginShaderMode(&gs->pShader);
                bindTextureToShader(&gs->pShader, "divTexture", gs->divTexture.texture.id, 0);
                bindTextureToShader(&gs->pShader, "pPrev", psrc->texture.id, 1);
                setUniform(&gs->pShader, "textureSize", glm::vec2(SIM_W, SIM_H));
                setUniform(&gs->pShader, "dt", dt);
                setUniform(&gs->pShader, "boundaryMode", boundaryMode);
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
            setUniform(&gs->advect, "boundaryMode", boundaryMode);
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
            setUniform(&gs->advect, "boundaryMode", boundaryMode);
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
            setUniform(&gs->advect, "boundaryMode", boundaryMode);
            renderDrawFilledRect({0,0},{(float)dstDens->texture.width, (float)dstDens->texture.height},0,{1,1,1,1});
            endShaderMode();
        endTextureMode();
        gs->pingPongDens = !gs->pingPongDens;

        enableBlending();

        // Projected result is now in srcVx/srcVy, so DON'T flip pingPong
        // (next frame, src will be these same textures)

        // Render density as fullscreen quad (grayscale)
        if(gs->s == SIM_DENSITY){
            beginScene();
                clearColor(0.1f,0.1f,0.1f,1);
                beginShaderMode(&gs->test);
                bindTextureToShader(&gs->test, "textureIn", dstDens->texture.id, 0);
                setUniform(&gs->test, "mode", 0);
                glm::vec2 screen = getScreenSize();
                renderDrawFilledRect(
                    {screen.x / 2.0f - SIM_W / 2.0f, screen.y / 2.0f - SIM_H / 2.0f},
                    {(float)SIM_W*1, (float)SIM_H*1}, 0, {1,1,1,1}
                );
                endShaderMode();
            endScene();
        }else if(gs->s == SIM_PRESSURE){
            beginScene();
                clearColor(0.1f,0.1f,0.1f,1);
                beginShaderMode(&gs->test);
                //bindTextureToShader(&gs->test, "textureIn", gs->pTexture.texture.id, 0);
                bindTextureToShader(&gs->test, "textureIn", pFinal->texture.id, 0);
                setUniform(&gs->test, "mode", 1);
                glm::vec2 screen = getScreenSize();
                renderDrawFilledRect(
                    {screen.x / 2.0f - SIM_W / 2.0f, screen.y / 2.0f - SIM_H / 2.0f},
                    {(float)SIM_W*1, (float)SIM_H*1}, 0, {1,1,1,1}
                );
                endShaderMode();
            endScene();

        }
    }
    drawHud(gs, gs->arena);
}

GAME_API void gameRender(Arena*,float){}
GAME_API void gameStop(Arena*){}
