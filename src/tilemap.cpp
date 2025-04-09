#include "main.h"

#include "tilemap.h"
#include "assets.h"
#include "render.h"
#include "world.h"

#include <stdio.h>

bool load_tilemap(Tilemap *tilemap, char *filepath) {
    char *file_data = read_entire_file(filepath);
    if (!file_data) {
        fprintf(stderr, "Failed to read file '%s'.\n", filepath);
        return false;
    }
    defer(delete[] file_data);

    char *file_data_at = file_data;

    // Parse version.
    char *line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
        return false;
    }

    line = trim_spaces(line);
    if (!starts_with(line, "version")) {
        fprintf(stderr, "Error in file '%s': Missing version directive!\n", filepath);
        return false;
    }
    line += string_length("version");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'version'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int version = atoi(line);

    if (version <= 0 || version > TILEMAP_FILE_VERSION) {
        fprintf(stderr, "Error in file '%s': Invalid value for version!\n", filepath);
        return false;
    }

    // Parse width.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
        return false;
    }

    line = trim_spaces(line);
    if (!starts_with(line, "width")) {
        fprintf(stderr, "Error in file '%s': Missing width directive!\n", filepath);
        return false;
    }
    line += string_length("width");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'width'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int width = atoi(line);
    if (width <= 0) {
        fprintf(stderr, "Error in file '%s': Invalid value for width!\n", filepath);
        return false;
    }

    // Parse height.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
        return false;
    }

    line = trim_spaces(line);
    if (!starts_with(line, "height")) {
        fprintf(stderr, "Error in file '%s': Missing height directive!\n", filepath);
        return false;
    }
    line += string_length("height");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'height'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int height = atoi(line);
    if (width <= 0) {
        fprintf(stderr, "Error in file '%s': Invalid value for height!\n", filepath);
        return false;
    }

    // Parse num_textures.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
        return false;
    }

    line = trim_spaces(line);
    if (!starts_with(line, "num_textures")) {
        fprintf(stderr, "Error in file '%s': Missing num_textures directive!\n", filepath);
        return false;
    }
    line += string_length("num_textures");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'num_textures'\n", filepath);
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int num_textures = atoi(line);
    if (width <= 0) {
        fprintf(stderr, "Error in file '%s': Invalid value for num_textures!\n", filepath);
        return false;
    }    

    // Parse textures.
    Texture **textures = new Texture*[num_textures];
    for (int i = 0; i < num_textures; i++) {
        line = consume_next_line(&file_data_at);
        if (!line) {
            fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
            delete[] textures;
            return false;
        }
        line = trim_spaces(line);
        if (!starts_with(line, "texture")) {
            fprintf(stderr, "Error in file '%s': Missing texture directive!\n", filepath);
            delete[] textures;
            return false;
        }
        line += string_length("texture");
        line = trim_spaces(line);
        if (!starts_with(line, "=")) {
            fprintf(stderr, "Error in file '%s': Missing '=' after 'texture'\n", filepath);
            delete[] textures;
            return false;
        }
        line += 1;
        line = trim_spaces(line);

        char *texture_name = line;
        textures[i] = find_or_load_texture(texture_name);
    }

    // Parse collidable_tile_ids.
    line = consume_next_line(&file_data_at);
    if (!line) {
        fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
        delete[] textures;
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "collidable_tile_ids")) {
        fprintf(stderr, "Error in file '%s': Missing collidable_tile_ids directive!\n", filepath);
        delete[] textures;
        return false;
    }
    line += string_length("collidable_tile_ids");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        fprintf(stderr, "Error in file '%s': Missing '=' after 'collidable_tile_ids'\n", filepath);
        delete[] textures;
        return false;
    }
    line += 1;
    line = trim_spaces(line);

    Array <char *> collidable_tile_id_strings;
    defer(for(char *s : collidable_tile_id_strings) delete[] s);
    split_line(line, ',', collidable_tile_id_strings);

    int num_collidable_tile_ids = collidable_tile_id_strings.count;
    u8 *collidable_tile_ids = NULL;
    if (num_collidable_tile_ids) collidable_tile_ids = new u8[num_collidable_tile_ids];
    for (int i = 0; i < num_collidable_tile_ids; i++) {
        char *s = collidable_tile_id_strings[i];
        s = trim_spaces(s);
        collidable_tile_ids[i] = atoi(collidable_tile_id_strings[i]);
    }

    char **lines = new char*[height];
    defer(delete[] lines);

    for (int i = 0; i < height; i++) {
        lines[i] = consume_next_line(&file_data_at);
        if (!lines[i]) {
            fprintf(stderr, "File '%s' is too short to be considered a valid tilemap file!\n", filepath);
            delete[] textures;
            return false;
        }
    }

    u8 *tiles = new u8[width * height];
    for (int y = height - 1; y >= 0; y--) {
        line = lines[y];

        Array <char *> id_strings;
        defer(for(char *s : id_strings) delete[] s);
        split_line(line, ',', id_strings);

        if (id_strings.count != width) {
            fprintf(stderr, "Error in file '%s': Row doesn't contain valid number of ids!\n", filepath);
            delete[] textures;
            return false;
        }

        for (int x = 0; x < width; x++) {
            u8 tile_id = atoi(id_strings[x]);
            tiles[y * width + x] = tile_id;
        }
    }

    tilemap->width        = width;
    tilemap->height       = height;

    tilemap->tiles        = tiles;

    tilemap->num_textures = num_textures;
    tilemap->textures     = textures;

    tilemap->num_collidable_tile_ids = num_collidable_tile_ids;
    tilemap->collidable_tile_ids     = collidable_tile_ids;
    
    return true;
}

void render_tilemap(Render_Commands *rc, Tilemap *tilemap, World *world) {
    for (int y = 0; y < tilemap->height; y++) {
        for (int x = 0; x < tilemap->width; x++) {
            u8 tile = tilemap->tiles[y * tilemap->width + x];
            if (tile <= 0 || tile > tilemap->num_textures) continue;

            Texture *texture = tilemap->textures[tile - 1];

            Vector2 screen_space_position = world_space_to_screen_space(world, v2((float)x, (float)y));
            Vector2 screen_space_size = world_space_to_screen_space(world, v2(1, 1));
            
            render_quad(rc, texture, screen_space_position, screen_space_size, v4(1, 1, 1, 1));
        }
    }
}
