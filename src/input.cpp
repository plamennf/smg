#include "main.h"

static Key_State key_states[NUM_KEY_CODES];
static Mouse_Button_State mouse_button_states[NUM_MOUSE_BUTTONS];

int mouse_cursor_x = 0;
int mouse_cursor_y = 0;

int mouse_cursor_x_delta = 0;
int mouse_cursor_y_delta = 0;

int mouse_scroll_wheel_x_delta = 0;
int mouse_scroll_wheel_y_delta = 0;

bool is_key_down(Key_Code key_code) {
    return key_states[key_code].is_down;
}

bool is_key_pressed(Key_Code key_code) {
    return key_states[key_code].is_down && key_states[key_code].changed;
}

bool was_key_just_released(Key_Code key_code) {
    return key_states[key_code].was_down && !key_states[key_code].is_down;
}

void set_key_state(Key_Code key_code, bool is_down) {
    Key_State *state = &key_states[key_code];
    state->changed   = is_down != state->is_down;
    state->is_down   = is_down;
}

void clear_key_states() {
    for (int i = 0; i < ArrayCount(key_states); i++) {
        Key_State *state = &key_states[i];
        state->was_down  = state->is_down;
        state->changed   = false;
    }
}


bool is_mouse_button_down(Mouse_Button button) {
    return mouse_button_states[button].is_down;
}

bool is_mouse_button_pressed(Mouse_Button button) {
    return mouse_button_states[button].is_down && mouse_button_states[button].changed;    
}

bool was_mouse_button_just_released(Mouse_Button button) {
    return mouse_button_states[button].was_down && !mouse_button_states[button].is_down;
}

void set_mouse_button_state(Mouse_Button button, bool is_down) {
    Mouse_Button_State *state = &mouse_button_states[button];
    state->changed = state->is_down != is_down;
    state->is_down = is_down;
}

void clear_mouse_button_states() {
    for (int i = 0; i < ArrayCount(mouse_button_states); i++) {
        Mouse_Button_State *state = &mouse_button_states[i];
        state->was_down = state->is_down;
        state->changed  = false;
    }
}
