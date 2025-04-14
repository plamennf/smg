#include "main.h"

#include "world.h"
#include "tilemap.h"
#include "input.h"
#include "entity_hero.h"
#include "entity_light.h"

const float GRAVITY = -30.0f;
const float MOVE_SPEED = 8.0f;
const float JUMP_FORCE = 15.0f;
const float MAX_FALL_SPEED = -25.0f;
const float FAST_FALL_MULTIPLIER = 1.5f;

const float DASH_SPEED = 20.0f;
const float DASH_DURATION = 0.2f;

static void update_hero_movement_platformer(Hero *hero, float dt, Tilemap *tilemap) {
    World *world = hero->world;
    
    float input_x = 0.0f;
    if (is_key_down(KEY_A)) input_x -= 1.0f;
    if (is_key_down(KEY_D)) input_x += 1.0f;
    
    if (is_key_pressed(KEY_LEFT_SHIFT) && !hero->is_dashing && input_x != 0.0f) {
        hero->is_dashing = true;
        hero->dash_timer = DASH_DURATION;
        hero->velocity = v2(input_x * DASH_SPEED, 0.0f);
    }

    if (hero->is_dashing) {
        hero->dash_timer -= dt;
        if (hero->dash_timer <= 0) {
            hero->is_dashing = false;
        }
    } else {
        hero->velocity.x = input_x * MOVE_SPEED;

        if (is_key_down(KEY_SPACE) && hero->on_ground) {
            hero->velocity.y = JUMP_FORCE;
            hero->on_ground  = false;
        }

        hero->velocity.y += GRAVITY * dt;

        // Fast-fall.
        if (!hero->on_ground && is_key_pressed(KEY_S)) {
            hero->velocity.y += GRAVITY * (FAST_FALL_MULTIPLIER - 1.0f) * dt;
        }

        hero->velocity.y = Max(hero->velocity.y, MAX_FALL_SPEED);
    }

    hero->position += hero->velocity * dt;

    if (hero->position.y <= 1.0f) {
        hero->position.y = 1.0f;
        hero->velocity.y = 0.0f;
        hero->on_ground  = true;
    }

    clamp(&hero->position.x, 0.0f, (float)world->size.x - hero->size.x);
    clamp(&hero->position.y, 1.0f, (float)world->size.y - hero->size.y);
}

void update_single_hero(Hero *hero, float dt, Tilemap *tilemap) {
    if (hero->movement_type == HERO_MOVEMENT_PLATFORMER) {
        update_hero_movement_platformer(hero, dt, tilemap);
    }
    
    /*
    Vector2 move_dir = {};

    if (is_key_down(KEY_A)) move_dir.x -= 1.0f;
    if (is_key_down(KEY_D)) move_dir.x += 1.0f;
    
    if (is_key_down(KEY_W)) move_dir.y += 1.0f;
    if (is_key_down(KEY_S)) move_dir.y -= 1.0f;

    move_dir = normalize_or_zero(move_dir);

    float speed = 5.0f;
    
    Vector2 new_position = hero->position + move_dir * speed * dt;

    float tile_at_new_position_x = new_position.x;
    float tile_at_new_position_y = new_position.y;
    if (move_dir.x > 0.0f) {
        tile_at_new_position_x += hero->size.x;
    }
    if (move_dir.y > 0.0f) {
        tile_at_new_position_y += hero->size.y;
    }
    
    u8 tile_at_new_position = get_tile_at(tilemap, tile_at_new_position_x, tile_at_new_position_y);
    if (is_tile_collidable(tilemap, tile_at_new_position)) {
        if (move_dir.x > 0.0f) {
            hero->position.x = roundf(new_position.x);// - hero->size.x;
        } else if (move_dir.x < 0.0f) {
            hero->position.x = roundf(new_position.x);// + hero->size.x;
        }

        if (move_dir.y > 0.0f) {
            hero->position.y = roundf(new_position.y);
        } else if (move_dir.y < 0.0f) {
            hero->position.y = roundf(new_position.y);
        }
    } else {
        hero->position = new_position;
    }
    */
    
    Entity *light_e = get_entity_by_id(hero->world, hero->light_id);
    if (light_e) {
        Light *light = (Light *)light_e;
        light->position = hero->position + (0.5f * hero->size);
    }
}
