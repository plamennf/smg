#include "main.h"

#include "world.h"
#include "tilemap.h"
#include "input.h"
#include "entity_hero.h"
#include "entity_light.h"

void update_single_hero(Hero *hero, float dt, Tilemap *tilemap) {
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
    
    Entity *light_e = get_entity_by_id(hero->world, hero->light_id);
    if (light_e) {
        Light *light = (Light *)light_e;
        light->position = hero->position + (0.5f * hero->size);
    }
}
