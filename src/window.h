#pragma once

extern int window_width;
extern int window_height;
extern bool window_is_open;

bool window_init(int width, int height, char *title);
void window_poll_events();
void window_swap_buffers();
void window_sync(int fps_cap);
void window_toggle_fullscreen();
