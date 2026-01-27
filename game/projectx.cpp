#include "projectx.hpp"

#define GRAVITY 9.8f

void systemRenderSprites(){
    EntityArray entities = view(ECS_TYPE(TransformComponent), ECS_TYPE(SpriteComponent));

    for(size_t i = 0; i < entities.count; i++){
        Entity entity = entities.entities[i];
        TransformComponent* t= (TransformComponent*) getComponent(entity, TransformComponent);
        SpriteComponent* s= (SpriteComponent*) getComponent(entity, SpriteComponent);
        if(s->visible){
            //OrtographicCamera cam = gameState->mainCamera;
            OrtographicCamera* cam = getActiveCamera();
            // TODO: move this check in the renderer to cull everything that is not on screen
            // when we are in the world position, not in screen position
            if( (t->position.x <= (cam->position.x - (cam->width / 2))  || t->position.x >= (cam->position.x + (cam->width  / 2))) &&
                (t->position.y <= (cam->position.y - (cam->height / 2)) || t->position.y >= (cam->position.y + (cam->height / 2)))) continue; 
            // Calculate final size from sprite size * transform scale
            glm::vec2 size = s->size * glm::vec2(t->scale.x, t->scale.y);

            // Calculate position for rendering (center of sprite)
            glm::vec3 position = t->position;
            position.z = s->layer;

            // Use sourceRect if set, otherwise default to full texture
            Rect sourceRect = s->sourceRect;
            if(s->sourceRect.size.x == 0 || s->sourceRect.size.y == 0) {
                sourceRect = {.pos = {0, 0}, .size = {(float)s->texture->width, (float)s->texture->height}};
            }

            // Handle UV flipping for flipX/flipY
            if(s->flipX) {
                sourceRect.pos.x += sourceRect.size.x;
                sourceRect.size.x = -sourceRect.size.x;
            }
            if(s->flipY) {
                sourceRect.pos.y += sourceRect.size.y;
                sourceRect.size.y = -sourceRect.size.y;
            }

            // Call renderDrawQuadPro directly
            renderDrawQuadPro(
                position,
                size,
                t->rotation,
                sourceRect,
                {0.5f,0.5f},
                s->texture,
                s->color,
                s->ySort,
                s->ySortOffset  // Pass y-sort offset for depth sorting
            );
        }
    }
}

void systemRenderParticle(){
    EntityArray entities = view(ECS_TYPE(TransformComponent));

    for(size_t i = 0; i < entities.count; i++){
        Entity entity = entities.entities[i];
        TransformComponent* t= (TransformComponent*) getComponent(entity, TransformComponent);
        OrtographicCamera* cam = getActiveCamera();
        // TODO: move this check in the renderer to cull everything that is not on screen
        // when we are in the world position, not in screen position
        if( (t->position.x <= (cam->position.x - (cam->width / 2))  || t->position.x >= (cam->position.x + (cam->width  / 2))) &&
            (t->position.y <= (cam->position.y - (cam->height / 2)) || t->position.y >= (cam->position.y + (cam->height / 2)))) continue; 

        // Calculate position for rendering (center of sprite)
        glm::vec3 position = t->position;
        position.z = 0;

        renderDrawCirclePro({t->position.x,t->position.y}, 25, {0.5,0.5}, {1,1,1,1}, 1);
    }
}

void moveSystem(float dt){
    EntityArray entities = view(ECS_TYPE(TransformComponent), ECS_TYPE(VelocityComponent), ECS_TYPE(DirectionComponent));
    for(size_t i = 0; i < entities.count; i++){
        Entity e = entities.entities[i];
        TransformComponent* transform = getComponent(e, TransformComponent);
        VelocityComponent* velocity  = getComponent(e, VelocityComponent);
        DirectionComponent* direction  = getComponent(e, DirectionComponent);
        //transform->position += glm::vec3(direction->dir.x * velocity->vel.x * dt, direction->dir.y * velocity->vel.y * dt, 0.0f);
        transform->position += glm::vec3(0 * velocity->vel.x * GRAVITY * dt, (-1) * velocity->vel.y * GRAVITY * dt, 0.0f);
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

    Entity e = createEntity();
    TransformComponent t = {};
    t.position = {300,200,0};
    t.rotation = {0,0,0};
    t.scale = {1,1,1};
    DirectionComponent d = {.dir = {1,1}};
    VelocityComponent v = {.vel = {50, 50}};
    pushComponent(e, TransformComponent, &t);
    pushComponent(e, VelocityComponent, &d);
    pushComponent(e, DirectionComponent, &v);
}

GAME_API void gameRender(Arena* gameArena, float dt){}

GAME_API void gameUpdate(Arena* gameArena, float dt){

    moveSystem(dt);

    beginScene();
        clearColor(0.2f, 0.3f, 0.3f, 1.0f);
        renderDrawFilledRect({10,10}, {50,50}, 0, {1,0,0,1}, 1);
        beginMode2D(*getActiveCamera());
        renderDrawCirclePro({100,100}, 50, {0.5,0.5}, {1,1,1,1}, 1);
        systemRenderParticle();
        endMode2D();
    endScene();
}

GAME_API void gameStop(Arena* gameArena){
}