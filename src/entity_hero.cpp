#include "main.h"

#include "world.h"
#include "tilemap.h"
#include "input.h"
#include "entity_hero.h"
#include "entity_light.h"
#include "animation.h"
#include "assets.h"

#include <stdio.h>

const float GRAVITY = -30.0f;
const float MOVE_SPEED = 8.0f;
const float JUMP_FORCE = 15.0f;
const float MAX_FALL_SPEED = -25.0f;
const float FAST_FALL_MULTIPLIER = 1.5f;

const float DASH_SPEED = 20.0f;
const float DASH_DURATION = 0.2f;

static void update_animation(Hero *hero) {
    assert(hero->current_animation);

    if (!hero->current_animation->is_looping && !hero->current_animation->is_completed) return;

    Animation *animation = NULL;
    switch (hero->state) {
        case HERO_IDLE: {
            animation = hero->idle[hero->orientation];
        } break;

        case HERO_WALKING: {
            animation = hero->walk[hero->orientation];
        } break;

        case HERO_JUMPING: {
            animation = hero->jump[hero->orientation];
        } break;
    }

    if (animation == hero->current_animation) return;
    
    if (!animation) {
        fprintf(stderr, "Failed to get animation for current hero state(%d) and orientation(%d)! Resetting to idle!\n", hero->state, hero->orientation);
        return;
    }

    hero->current_animation = animation;
    reset_animation(hero->current_animation);
}

static void update_hero_movement_platformer(Hero *hero, float dt, Tilemap *tilemap) {
    World *world = hero->world;
    
    float input_x = 0.0f;
    if (is_key_down(KEY_A)) {input_x -= 1.0f; hero->orientation = HERO_LOOKING_LEFT;}
    if (is_key_down(KEY_D)) {input_x += 1.0f; hero->orientation = HERO_LOOKING_RIGHT;}
    
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

        if (is_key_down(KEY_W) && hero->on_ground) {
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

    if (hero->is_dashing)      hero->state = HERO_IDLE;
    else if (!hero->on_ground) hero->state = HERO_JUMPING;
    else if (hero->velocity.x != 0.0f) hero->state = HERO_WALKING;
    else hero->state = HERO_IDLE;

    update_animation(hero);
}

void update_single_hero(Hero *hero, float dt, Tilemap *tilemap) {
    if (hero->movement_type == HERO_MOVEMENT_PLATFORMER) {
        update_hero_movement_platformer(hero, dt, tilemap);
    }
    
    Entity *light_e = get_entity_by_id(hero->world, hero->light_id);
    if (light_e) {
        Light *light = (Light *)light_e;
        light->position = hero->position + (0.5f * hero->size);
    }
}

void load_animations_for_hero(Hero *hero) {
    memset(hero->idle, 0, sizeof(hero->idle));
    memset(hero->walk, 0, sizeof(hero->walk));
    memset(hero->jump, 0, sizeof(hero->jump));
    
    hero->idle[HERO_LOOKING_LEFT]  = find_or_load_animation("hero_idle_left");
    hero->idle[HERO_LOOKING_RIGHT] = find_or_load_animation("hero_idle_right");
    hero->idle[HERO_LOOKING_DOWN]  = find_or_load_animation("hero_idle_front");
    hero->idle[HERO_LOOKING_UP]    = find_or_load_animation("hero_idle_back");

    hero->walk[HERO_LOOKING_LEFT]  = find_or_load_animation("hero_walk_left");
    hero->walk[HERO_LOOKING_RIGHT] = find_or_load_animation("hero_walk_right");
    hero->walk[HERO_LOOKING_DOWN]  = find_or_load_animation("hero_walk_front");
    hero->walk[HERO_LOOKING_UP]    = find_or_load_animation("hero_walk_back");

    hero->jump[HERO_LOOKING_LEFT]  = find_or_load_animation("hero_jump_left");
    hero->jump[HERO_LOOKING_RIGHT] = find_or_load_animation("hero_jump_right");

    hero->current_animation = hero->idle[HERO_LOOKING_DOWN];
}
