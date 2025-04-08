#pragma once

struct World;
struct Texture;

enum Entity_Type {
    ENTITY_TYPE_UNKNOWN,
    ENTITY_TYPE_HERO,
    ENTITY_TYPE_LIGHT,
};

struct Entity {
    u64 id;
    World *world;
    Entity_Type type;

    Vector2 position;
    Vector2 size;

    Texture *texture;
};
