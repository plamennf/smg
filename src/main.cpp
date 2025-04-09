#include "main.h"

#include "window.h"
#include "render.h"
#include "font.h"
#include "assets.h"
#include "input.h"
#include "world.h"

#include "tilemap.h"
#include "entity_hero.h"
#include "entity_light.h"

#include <stdio.h>

static int fps_cap = 60;
static World current_world;

static bool init_test_world() {
    world_init(&current_world, v2i(16, 9));

    current_world.tilemap = new Tilemap();
    if (!load_tilemap(current_world.tilemap, "data/tilemaps/test.tm")) {
        return false;
    }

    current_world.size = v2i(current_world.tilemap->width, current_world.tilemap->height);
    
    Hero *hero     = make_hero(&current_world);
    hero->position = v2(5, 5);
    hero->size     = v2(2, 2);
    hero->texture  = find_or_load_texture("BRAND_NEW_PACHI_SPRITE1");

    Light *light    = make_light(&current_world);
    light->position = hero->position;
    light->radius   = hero->size.y * 0.5f;
    light->color    = v3(1, 0.5f, 0.2f);

    hero->light_id = light->id;

    return true;
}

static void render_frame(Render_Commands *rc, float dt) {
    // Render Lights.
    {
        Render_Target_Config config = {};
        config.render_target = RENDER_TARGET_LIGHTS;
        config.clear_color   = true;
        config.color         = {19/255.0f, 24/255.0f, 98/255.0f, 1.0f};
        set_render_target(rc, config);
    
        Render_Setup setup = default_render_setup();
        setup.point_sample_textures = false;
        setup.blend_mode = BLEND_MODE_SSLM;
        set_render_setup(rc, setup);

        world_render_lights(&current_world, rc);
    }

    // Render Main Scene.
    {
        Render_Target_Config config = {};
        config.render_target = RENDER_TARGET_MAIN;
        config.clear_color   = true;
        config.color         = {0.2f, 0.5f, 0.8f, 1.0f};
        set_render_target(rc, config);
    
        Render_Setup setup = default_render_setup();
        setup.point_sample_textures = true;
        setup.blend_mode = BLEND_MODE_ALPHA;
        set_render_setup(rc, setup);

        world_render(&current_world, rc);
    }

    // Blit to back buffer and draw hud.
    {
        Render_Target_Config config = {};
        config.render_target = RENDER_TARGET_BACKBUFFER;
        config.clear_color   = true;
        config.color         = {0, 0, 0, 1};
        set_render_target(rc, config);

        int font_size = (int)(0.075f * window_height);
        Font *font = find_or_load_font("FANTASY MAGIST", font_size);

        char text[4096] = "Giovanny Yatsuro!";
        int x = (window_width - get_text_width(font, text)) / 2;
        int y = window_height - font->character_height; 
        
        render_text(rc, font, text, x, y, {1, 1, 1, 1});

        font_size = (int)(0.035f * window_height);
        font = find_or_load_font("OpenSans-Regular", font_size);
        
        snprintf(text, sizeof(text), "%.0lf fps", 1.0 / (double)dt);
        x = (window_width  - get_text_width(font, text)) - (window_width - render_width) / 2;
        y = window_height - font->character_height;

        render_text(rc, font, text, x, y, {1, 1, 1, 1});
    }
}

int main(int argc, char *argv[]) {
    if (!window_init(2080, 1080, "Swordsman Game!")) return 1;
    render_init();

    init_test_world();
    
    u64 last_time = get_time_nanoseconds();
    while (window_is_open) {
        u64 now_time = get_time_nanoseconds();
        double dt = (double)(now_time - last_time) / 1000000000.0;
        last_time = now_time;
        
        window_poll_events();
        
        if (is_key_pressed(KEY_F11)) window_toggle_fullscreen();
        
        world_update(&current_world, (float)dt);
        
        Render_Commands *render_commands = render_begin_frame();
        render_frame(render_commands, (float)dt);
        render_end_frame(render_commands);
        
        window_swap_buffers();
        window_sync(fps_cap);
    }
    
    return 0;
}
