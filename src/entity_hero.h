#pragma once

#include "entity.h"

struct Tilemap;

enum Hero_State {
    HERO_IDLE,
    HERO_WALKING,
    HERO_JUMPING,
};

enum Hero_Orientation {
    HERO_LOOKING_DOWN,
    HERO_LOOKING_UP,
    HERO_LOOKING_LEFT,
    HERO_LOOKING_RIGHT,
    HERO_ORIENTATION_TOTAL,
};

enum Hero_Movement_Type {
    HERO_MOVEMENT_RPG,
    HERO_MOVEMENT_PLATFORMER,
};

struct Hero : public Entity {
    Hero_State state;
    Hero_Orientation orientation;
    Hero_Movement_Type movement_type;

    Animation *idle[HERO_ORIENTATION_TOTAL];
    Animation *walk[HERO_ORIENTATION_TOTAL];
    Animation *jump[HERO_ORIENTATION_TOTAL];
    
    Vector2 velocity;

    bool on_ground = false;
    bool is_dashing = false;
    float dash_timer = 0.0f;
    
    u64 light_id;
};

void load_animations_for_hero(Hero *hero);
void update_single_hero(Hero *hero, float dt, Tilemap *tilemap);

void set_state(Hero *hero, Hero_State state);
void set_orientation(Hero *hero, Hero_Orientation orientation);
