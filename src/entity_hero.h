#pragma once

#include "entity.h"

struct Tilemap;

struct Hero : public Entity {
    u64 light_id;
};

void update_single_hero(Hero *hero, float dt, Tilemap *tilemap);
