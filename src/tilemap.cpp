#include "main.h"

#include "tilemap.h"
#include "assets.h"
#include "render.h"
#include "world.h"
#include "text_file_handler.h"

#include <stdio.h>

bool load_tilemap(Tilemap *tilemap, char *filepath) {
    Text_File_Handler handler;
    if (!start_file(&handler, filepath)) return false;
    defer(end_file(&handler));
    
    if (handler.version <= 0 || handler.version > TILEMAP_FILE_VERSION) {
        report_error(&handler, "Invalid value for version!");
        return false;
    }

    // Parse width.
    char *line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid tilemap file!", filepath);
        return false;
    }
    if (!starts_with(line, "width")) {
        report_error(&handler, "Missing width directive!");
        return false;
    }
    line += string_length("width");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'width'");
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int width = atoi(line);
    if (width <= 0) {
        report_error(&handler, "Invalid value for width!", filepath);
        return false;
    }

    // Parse height.
    line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid tilemap file!");
        return false;
    }
    if (!starts_with(line, "height")) {
        report_error(&handler, "Missing height directive!");
        return false;
    }
    line += string_length("height");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'height'");
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int height = atoi(line);
    if (height <= 0) {
        report_error(&handler, "Invalid value for height!");
        return false;
    }

    // Parse num_textures.
    line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid tilemap file!");
        return false;
    }
    if (!starts_with(line, "num_textures")) {
        report_error(&handler, "Missing num_textures directive!");
        return false;
    }
    line += string_length("num_textures");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'num_textures'");
        return false;
    }
    line += 1;
    line = trim_spaces(line);
    int num_textures = atoi(line);
    if (width <= 0) {
        report_error(&handler, "Invalid value for num_textures!");
        return false;
    }    

    // Parse textures.
    Texture **textures = new Texture*[num_textures];
    for (int i = 0; i < num_textures; i++) {
        line = consume_next_line(&handler);
        if (!line) {
            report_error(&handler, "File is too short to be considered a valid tilemap file!");
            delete[] textures;
            return false;
        }
        if (!starts_with(line, "texture")) {
            report_error(&handler, "Missing texture directive!");
            delete[] textures;
            return false;
        }
        line += string_length("texture");
        line = trim_spaces(line);
        if (!starts_with(line, "=")) {
            report_error(&handler, "Missing '=' after 'texture'");
            delete[] textures;
            return false;
        }
        line += 1;
        line = trim_spaces(line);

        char *texture_name = line;
        textures[i] = find_or_load_texture(texture_name);
    }

    // Parse collidable_tile_ids.
    line = consume_next_line(&handler);
    if (!line) {
        report_error(&handler, "File is too short to be considered a valid tilemap file!");
        delete[] textures;
        return false;
    }
    line = trim_spaces(line);
    if (!starts_with(line, "collidable_tile_ids")) {
        report_error(&handler, "Missing collidable_tile_ids directive!");
        delete[] textures;
        return false;
    }
    line += string_length("collidable_tile_ids");
    line = trim_spaces(line);
    if (!starts_with(line, "=")) {
        report_error(&handler, "Missing '=' after 'collidable_tile_ids'");
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
        lines[i] = consume_next_line(&handler);
        if (!lines[i]) {
            report_error(&handler, "File is too short to be considered a valid tilemap file!");
            delete[] textures;
            return false;
        }
    }

    u8 *tiles = new u8[width * height];
    for (int y = height - 1; y >= 0; y--) {
        line = lines[height - y - 1];

        Array <char *> id_strings;
        defer(for(char *s : id_strings) delete[] s);
        split_line(line, ',', id_strings);

        if (id_strings.count != width) {
            report_error(&handler, "Row doesn't contain valid number of ids!");
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

u8 get_tile_at(Tilemap *tilemap, float x, float y) {
    if (!tilemap) return 0;
    if (x < 0.0f || x > (float)tilemap->width)  return 0;
    if (y < 0.0f || y > (float)tilemap->height) return 0;

    int ix = (int)x;
    int iy = (int)y;
    
    return tilemap->tiles[iy * tilemap->width + ix];
}

bool is_tile_collidable(Tilemap *tilemap, u8 tile) {
    for (int i = 0; i < tilemap->num_collidable_tile_ids; i++) {
        if (tilemap->collidable_tile_ids[i] == tile) {
            return true;
        }
    }

    return false;
}
