#pragma once

struct Texture;
struct Font;
//struct Animation;

#define TEXTURE_DIRECTORY "data/textures"
#define FONT_DIRECTORY "data/fonts"
//#define ANIMATION_DIRECTORY "data/animations"

Texture *find_or_load_texture(char *name);
Font *find_or_load_font(char *name, int character_height);
//Animation *find_or_load_animation(char *name);
