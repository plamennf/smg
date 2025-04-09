#pragma once

#define TILEMAP_FILE_VERSION 1

struct Texture;
struct Render_Commands;
struct World;

struct Tilemap {
    int width;
    int height;
    
    u8 *tiles;

    int num_textures;
    Texture **textures;
    
    int num_collidable_tile_ids;
    u8 *collidable_tile_ids;
};

bool load_tilemap(Tilemap *tilemap, char *filepath);
void render_tilemap(Render_Commands *rc, Tilemap *tilemap, World *world);
