#include "main.h"

#include "window.h"
#include "render.h"
#include "font.h"
#include "assets.h"
#include "input.h"

#include <stdio.h>

static int fps_cap = 60;
static Vector2 hero_position = {};
static Vector2 hero_size = {2, 2};

static void update_frame(float dt) {
    Vector2 move_dir = {};

    if (is_key_down(KEY_A)) move_dir.x -= 1.0f;
    if (is_key_down(KEY_D)) move_dir.x += 1.0f;
    
    if (is_key_down(KEY_W)) move_dir.y += 1.0f;
    if (is_key_down(KEY_S)) move_dir.y -= 1.0f;

    move_dir = normalize_or_zero(move_dir);

    float speed = 5.0f;
    
    hero_position += move_dir * speed * dt;
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

        Vector2 screen_hero_position = hero_position;

        screen_hero_position.x /= 16.0f;
        screen_hero_position.y /= 9.0f;

        screen_hero_position.x *= (float)render_width;
        screen_hero_position.y *= (float)render_height;

        Vector2 screen_hero_size = hero_size;

        screen_hero_size.x /= 16.0f;
        screen_hero_size.y /= 9.0f;

        screen_hero_size.x *= (float)render_width;
        screen_hero_size.y *= (float)render_height;
    
        render_circle(rc, v2(screen_hero_position.x + screen_hero_size.x * 0.5f, screen_hero_position.y + screen_hero_size.y * 0.5f), screen_hero_size.y * 0.5f, {1, 0.5f, 0.2f, 1});
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

        auto texture = find_or_load_texture("BRAND_NEW_PACHI_SPRITE1");

        Vector2 screen_hero_position = hero_position;

        screen_hero_position.x /= 16.0f;
        screen_hero_position.y /= 9.0f;

        screen_hero_position.x *= (float)render_width;
        screen_hero_position.y *= (float)render_height;

        Vector2 screen_hero_size = hero_size;

        screen_hero_size.x /= 16.0f;
        screen_hero_size.y /= 9.0f;

        screen_hero_size.x *= (float)render_width;
        screen_hero_size.y *= (float)render_height;
        
        render_quad(rc, texture, screen_hero_position, screen_hero_size, {1, 1, 1, 1});
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

    double last_time = get_time();
    while (window_is_open) {
        double now_time = get_time();
        double dt = now_time - last_time;
        last_time = now_time;
        
        window_poll_events();

        if (is_key_pressed(KEY_F11)) window_toggle_fullscreen();
        
        update_frame((float)dt);
        
        Render_Commands *render_commands = render_begin_frame();
        render_frame(render_commands, (float)dt);
        render_end_frame(render_commands);
        
        window_swap_buffers();
        window_sync(fps_cap);
    }
    
    return 0;
}
