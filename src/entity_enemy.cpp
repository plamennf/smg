#include "main.h"

#include "world.h"
#include "entity_enemy.h"
#include "entity_hero.h"
#include "entity_light.h"

#include <stdio.h>

void update_single_enemy(Enemy *enemy, float dt) {
    Entity *hero_e = get_entity_by_id(enemy->world, enemy->hero_id);
    if (!hero_e) return;
    Hero *hero = (Hero *)hero_e;

    float player_distance = fabsf(hero->position.x - enemy->position.x);

    if (enemy->attack_cooldown > 0.0f) {
        enemy->attack_cooldown -= dt;
    }

    switch (enemy->state) {
        case ENEMY_IDLE: {
            if (player_distance < 4.0f) {
                enemy->state = ENEMY_CHASE;
            }
        } break;

        case ENEMY_CHASE: {
            if (player_distance > 0.8f) {
                enemy->position.x = move_toward(enemy->position.x, hero->position.x, enemy->speed * dt);
            } else {
                enemy->state = ENEMY_ATTACK;
            }
        } break;

        case ENEMY_ATTACK: {
            if (enemy->attack_cooldown <= 0.0f) {
                Rectangle2 hero_aabb = {
                    hero->position.x,
                    hero->position.y,
                    hero->size.x,
                    hero->size.y,
                };

                Rectangle2 enemy_aabb = {
                    enemy->position.x,
                    enemy->position.y,
                    enemy->size.x,
                    enemy->size.y,
                };

                if (are_intersecting(hero_aabb, enemy_aabb)) {
                    printf("Enemy successfully attacked hero!\n");
                }
                enemy->attack_cooldown = 1.5f;
            } else {
                enemy->state = ENEMY_DEFEND;
                //printf("Enemy defending!\n");
            }
        } break;

        case ENEMY_DEFEND: {
            if (0/*hero->is_attacking*/) {
                // block();
            } else {
                enemy->state = ENEMY_IDLE;
            }
        } break;
    }

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
