#include "main.h"

#include "world.h"
#include "entity_enemy.h"
#include "entity_hero.h"
#include "entity_light.h"

void update_single_enemy(Enemy *enemy, float dt) {
    Entity *hero_e = get_entity_by_id(enemy->world, enemy->hero_id);
    if (!hero_e) return;
    Hero *hero = (Hero *)hero_e;

    enemy->position.x = move_toward(enemy->position.x, hero->position.x, enemy->speed * dt);

    if (enemy->position.x < hero->position.x) {
        enemy->flip_mode = FLIP_MODE_NONE;
    } else {
        enemy->flip_mode = FLIP_MODE_HORIZONTALLY;
    }
    
    clamp(&enemy->position.x, 0.0f, (float)enemy->world->size.x - enemy->size.x);
    clamp(&enemy->position.y, 1.0f, (float)enemy->world->size.x - enemy->size.y);

    Entity *light_e = get_entity_by_id(enemy->world, enemy->light_id);
    if (light_e) {
        Light *light = (Light *)light_e;
        light->position = enemy->position + (0.5f * enemy->size);
    }
}
