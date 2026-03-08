#include "main.h"

int main(int argc, char *argv[]) {
    init_temporary_storage(Megabytes(1));
    
    init_logging();
    defer { shutdown_logging(); };

    os_init();
    defer { os_shutdown(); };

    if (!os_window_create(0, 0, "Swordsman Game")) return 1;
    if (!renderer_init(true, false)) return 1;
    
    //os_window_toggle_fullscreen();

    Texture texture, texture2;
    if (!renderer_load_texture(&texture, "data/textures/cobblestone.png")) return 1;
    if (!renderer_load_texture(&texture2, "data/textures/dirt.png")) return 1;
    
    float fixed_update_dt = 1.0f / 60.0f;
    u64 last_time = os_get_time_in_nanoseconds();
    float accumulated_dt = 0.0f;
    bool should_show_cursor = true;
    while (os_window_is_open) {
        reset_temporary_storage();
        
        u64 now_time = os_get_time_in_nanoseconds();
        u64 dt_ns = now_time - last_time;
        last_time = now_time;

        float dt = (float)((double)dt_ns / 1000000000.0);
        accumulated_dt += dt;
        
        os_poll_events();

        if (is_key_down(KEY_ALT)) {
            if (is_key_pressed(KEY_F4)) {
                os_window_is_open = false;
            } else if (is_key_pressed(KEY_ENTER)) {
                os_window_toggle_fullscreen();
            }
        }
        
        if (is_key_pressed(KEY_F11)) {
            os_window_toggle_fullscreen();
        }

        if (is_key_pressed(KEY_ESCAPE)) {
            should_show_cursor = !should_show_cursor;
        }

        if (os_window_was_resized()) {
            renderer_resize();
        }
        
        if (should_show_cursor) {
            os_show_and_unlock_cursor();
        } else {
            os_hide_and_lock_cursor();
        }

        while (accumulated_dt >= fixed_update_dt) {
            if (!should_show_cursor) {
                //fixed_update_camera(&camera, CAMERA_TYPE_FPS, fixed_update_dt);
            }
            accumulated_dt -= fixed_update_dt;
        }

        Per_Scene_Uniforms uniforms;
        uniforms.projection_matrix = transpose(make_orthographic(0.0f, (float)render_width, 0.0f, (float)render_height, -1.0f, 1.0f));
        uniforms.view_matrix = matrix4_identity();
        uniforms.do_hdr      = renderer_is_hdr_used();
        
        renderer_begin_frame(v4(0.2f, 0.5f, 0.8f, 1.0f), uniforms);
        renderer_add_sprite(&texture, v2(50, 50), v2(64, 64), v4(1, 0.5f, 0.2f, 1));
        renderer_add_sprite(&texture2, v2(render_width - 50.0f - 64.0f, render_height - 50.0f - 64.0f), v2(64, 64), v4(1, 0.5f, 0.2f, 1));
        renderer_add_sprite_center(&texture, v2(render_width * 0.5f, render_height * 0.5f), v2(64, 64), v4(1, 1, 1, 1));

        int font_size = (int)(0.15f * render_height);
        Font *font = find_or_load_font("FANTASY MAGIST", font_size);
        char *text = "Hello, World!";
        int x = (render_width - get_text_width(font, text)) / 2;
        int y = render_height - font->character_height;
        renderer_add_text(font, text, x, y, v4(1, 1, 1, 1));
        
        renderer_end_frame();

        renderer_present();
    }
    
    return 0;
}
