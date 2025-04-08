#include "main.h"

#include "window.h"
#include "render.h"
#include "font.h"
#include "assets.h"

static void render_frame(Render_Commands *rc) {
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
    
        render_circle(rc, {50+64, 50+64}, 64, {1, 0.5f, 0.2f, 1});
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
        
        render_quad(rc, texture, {50.0f, 50.0f}, {128.0f, 128.0f}, {1, 1, 1, 1});
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

        char *text = "Giovanny Yatsuro!";
        int x = (window_width  - get_text_width(font, text)) / 2;
        int y = window_height - font->character_height; 
        
        render_text(rc, font, text, x, y, {1, 1, 1, 1});
    }
}

int main(int argc, char *argv[]) {
    if (!window_init(2080, 1080, "Swordsman Game!")) return 1;
    render_init();
    
    while (window_is_open) {
        window_poll_events();
        
        Render_Commands *render_commands = render_begin_frame();
        render_frame(render_commands);
        render_end_frame(render_commands);
        
        window_swap_buffers();
    }
    
    return 0;
}
