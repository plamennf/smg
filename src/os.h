#pragma once

extern int os_window_width;
extern int os_window_height;
extern bool os_window_is_open;

void os_init();
void os_shutdown();

bool os_window_create(int width, int height, char *title);
void os_poll_events();
void *os_window_get_native();
bool os_window_was_resized();
void os_window_toggle_fullscreen();

u64 os_get_time_in_nanoseconds();

void os_show_and_unlock_cursor();
void os_hide_and_lock_cursor();
