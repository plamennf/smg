#pragma once

#include "entity.h"

struct Render_Commands;
struct Texture;

struct Hero;
struct Light;
struct Enemy;

struct Tilemap;

struct World {
    Hash_Table<u64, Entity *> entity_lookup;
    Vector2i size;

    Texture *background_texture;
    
    Tilemap *tilemap;
};

void world_init(World *world, Vector2i size);
void world_update(World *world, float dt);
void world_render(World *world, Render_Commands *rc);
void world_render_lights(World *world, Render_Commands *rc);

Vector2 world_space_to_screen_space(World *world, Vector2 v);
Vector2 screen_space_to_world_space(World *world, Vector2 v);

Entity *get_entity_by_id(World *world, u64 id);

Hero *make_hero(World *world);
Light *make_light(World *world);
Enemy *make_enemy(World *world);
