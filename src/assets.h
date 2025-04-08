#pragma once

struct Texture;
struct Font;

#define TEXTURE_DIRECTORY "data/textures"
#define FONT_DIRECTORY "data/fonts"

Texture *find_or_load_texture(char *name);
Font *find_or_load_font(char *name, int character_height);
