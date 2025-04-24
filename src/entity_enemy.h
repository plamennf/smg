#pragma once

#include "entity.h"

struct Tilemap;

enum Enemy_State {
    ENEMY_IDLE,
    ENEMY_CHASE,
    ENEMY_ATTACK,
    ENEMY_DEFEND,
    ENEMY_RETREAT,
};

struct Enemy : public Entity {
    float speed;
    u64 hero_id;
    u64 light_id;

    float attack_cooldown;
    Enemy_State state;
};

void update_single_enemy(Enemy *enemy, float dt);
