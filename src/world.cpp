#include "main.h"

#include "world.h"
#include "render.h"
#include "input.h"
#include "animation.h"

#include "mt19937-64.h"

#include "tilemap.h"
#include "entity_hero.h"
#include "entity_light.h"
#include "entity_enemy.h"

void world_init(World *world, Vector2i size) {
    unsigned long long init[] = {(u64)size.x, (u64)size.y};
    init_by_array64(init, ArrayCount(init));
    
    world->size = size;
}

void world_update(World *world, float dt) {
    for (int i = 0; i < world->entity_lookup.allocated; i++) {
        if (!world->entity_lookup.occupancy_mask[i]) continue;
        Entity *e = world->entity_lookup.buckets[i].value;

        switch (e->type) {
            case ENTITY_TYPE_HERO: {
                update_single_hero((Hero *)e, dt, world->tilemap);
            } break;

            case ENTITY_TYPE_ENEMY: {
                update_single_enemy((Enemy *)e, dt);
            } break;
        }

        if (e->current_animation) {
            update_animation(e->current_animation, dt);
        }
    }
}

void world_render(World *world, Render_Commands *rc) {
    if (world->background_texture) {
        // Y-Position is 1, because I assume that when we have a background texture, that means we are in an arena/fight and when you are in an arena there is going to be only one unit of platform underneath the player.
        Vector2 screen_space_position = world_space_to_screen_space(world, v2(0, 1));
        Vector2 screen_space_size     = world_space_to_screen_space(world, to_vec2(world->size));
        render_quad(rc, world->background_texture, screen_space_position, screen_space_size, v4(1, 1, 1, 1));
    }
    
    if (world->tilemap) {
        render_tilemap(rc, world->tilemap, world);
    }
    
    for (int i = 0; i < world->entity_lookup.allocated; i++) {
        if (!world->entity_lookup.occupancy_mask[i]) continue;
        Entity *e = world->entity_lookup.buckets[i].value;
        if (e->type == ENTITY_TYPE_LIGHT) continue;

        Animation *animation = e->current_animation;
        if (!animation) continue;

        Texture *texture = get_current_frame(animation);
        if (!texture) continue;
        
        Vector2 screen_space_position = world_space_to_screen_space(world, e->position);
        Vector2 screen_space_size     = world_space_to_screen_space(world, e->size);

        render_quad(rc, texture, screen_space_position, screen_space_size, NULL, e->flip_mode, {1, 1, 1, 1});
    }
}

void world_render_lights(World *world, Render_Commands *rc) {
    for (int i = 0; i < world->entity_lookup.allocated; i++) {
        if (!world->entity_lookup.occupancy_mask[i]) continue;
        Entity *e = world->entity_lookup.buckets[i].value;
        if (e->type != ENTITY_TYPE_LIGHT) continue;

        Light *light = (Light *)e;
        
        Vector2 screen_space_position = world_space_to_screen_space(world, e->position);
        float screen_space_radius = (light->radius / (float)world->size.y) * render_height;

        Vector4 color = v4(light->color, 1.0f);
        
        render_circle(rc, screen_space_position, screen_space_radius, color);
    }
}

Vector2 world_space_to_screen_space(World *world, Vector2 v) {
    assert(world->size.x > 0);
    assert(world->size.y > 0);
    
    Vector2 result = v;

    result.x /= (float)world->size.x;
    result.y /= (float)world->size.y;

    result.x *= (float)render_width;
    result.y *= (float)render_height;

    return result;
}

Vector2 screen_space_to_world_space(World *world, Vector2 v) {
    assert(render_width  > 0);
    assert(render_height > 0);

    Vector2 result = v;

    result.x /= (float)render_width;
    result.y /= (float)render_height;

    result.x *= (float)world->size.x;
    result.y *= (float)world->size.y;

    return result;
}

Entity *get_entity_by_id(World *world, u64 id) {
    Entity **_e = world->entity_lookup.find(id);
    if (!_e) return NULL;
    return *_e;
}

static u64 generate_id(World *world) {
    while (1) {
        u64 id = genrand64_int64();
        Entity **_e = world->entity_lookup.find(id);
        if (!_e) return id;
    }

    return 0;
}

static void register_entity(World *world, Entity *e, Entity_Type type) {
    u64 id = generate_id(world);

    e->id    = id;
    e->world = world;
    e->type  = type;

    e->flip_mode = FLIP_MODE_NONE;

    world->entity_lookup.add(id, e);
}

Hero *make_hero(World *world) {
    Hero *hero = new Hero();
    register_entity(world, hero, ENTITY_TYPE_HERO);
    load_animations_for_hero(hero);
    return hero;
}

Light *make_light(World *world) {
    Light *light = new Light();
    register_entity(world, light, ENTITY_TYPE_LIGHT);
    return light;
}

Enemy *make_enemy(World *world) {
    Enemy *enemy = new Enemy();
    register_entity(world, enemy, ENTITY_TYPE_ENEMY);
    return enemy;
}
