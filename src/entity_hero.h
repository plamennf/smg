#pragma once

#include "entity.h"

struct Tilemap;

enum Hero_State {
    HERO_IDLE,
    HERO_MOVING,
};

enum Hero_Orientation {
    HERO_LOOKING_DOWN,
    HERO_LOOKING_UP,
    HERO_LOOKING_LEFT,
    HERO_LOOKING_RIGHT,
};

enum Hero_Movement_Type {
    HERO_MOVEMENT_RPG,
    HERO_MOVEMENT_PLATFORMER,
};

struct Hero : public Entity {
    Hero_State state;
    Hero_Orientation orientation;
    Hero_Movement_Type movement_type;

    Vector2 velocity;

    bool on_ground = false;
    bool is_dashing = false;
    float dash_timer = 0.0f;
    
    u64 light_id;
};

void update_single_hero(Hero *hero, float dt, Tilemap *tilemap);
