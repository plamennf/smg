#pragma once

struct World;
struct Animation;

#include "render.h"

enum Entity_Type {
    ENTITY_TYPE_UNKNOWN,
    ENTITY_TYPE_HERO,
    ENTITY_TYPE_LIGHT,
    ENTITY_TYPE_ENEMY,
    ENTITY_TYPE_BUILDING,
};

struct Entity {
    u64 id;
    World *world;
    Entity_Type type;

    Vector2 position;
    Vector2 size;
    Flip_Mode flip_mode;
    
    Animation *current_animation;
};
