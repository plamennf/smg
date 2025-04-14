#pragma once

#include "entity.h"

struct Tilemap;

struct Enemy : public Entity {
    float speed;
    u64 hero_id;
};

void update_single_enemy(Enemy *enemy, float dt);
