#include "main.h"

#include "assets.h"
#include "render.h"
#include "font.h"

#include <stdio.h>

template <typename T>
struct Resource_Info {
    u64 name_hash;
    T *data;
};

static Array<Resource_Info<Texture>> loaded_textures;
static Array<Resource_Info<Font>> loaded_fonts;

Texture *find_or_load_texture(char *name) {
    u64 hash = get_hash(name);
    
    for (auto info : loaded_textures) {
        if (info.name_hash == hash) {
            return info.data;
        }
    }

    char *extensions[] = {
        "png",
        "jpg",
        "bmp",
    };

    char full_path[4096];
    bool full_path_exists = false;
    for (int i = 0; i < ArrayCount(extensions); i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s.%s", TEXTURE_DIRECTORY, name, extensions[i]);
        if (file_exists(full_path)) {
            full_path_exists = true;
            break;
        }
    }

    if (!full_path_exists) {
        fprintf(stderr, "No file '%s' in '%s'!\n", name, TEXTURE_DIRECTORY);
        return NULL;
    }
    
    Texture *texture = load_texture_from_file(full_path);
    if (!texture) return NULL;

    loaded_textures.add({hash, texture});
    return texture;
}

Font *find_or_load_font(char *name, int character_height) {
    u64 hash = get_hash(name);
    
    for (auto info : loaded_fonts) {
        if (info.name_hash == hash && info.data->character_height == character_height) {
            return info.data;
        }
    }

    char *extensions[] = {
        "ttf",
        "otf",
    };
    
    char full_path[4096];
    bool full_path_exists = false;
    for (int i = 0; i < ArrayCount(extensions); i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s.%s", FONT_DIRECTORY, name, extensions[i]);
        if (file_exists(full_path)) {
            full_path_exists = true;
            break;
        }
    }

    if (!full_path_exists) {
        fprintf(stderr, "No file '%s' in '%s'!\n", name, FONT_DIRECTORY);
        return NULL;
    }
    
    Font *font = new Font();
    if (!load_font(font, full_path, character_height)) return NULL;

    loaded_fonts.add({hash, font});
    return font;
}
