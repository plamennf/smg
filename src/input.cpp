#include "main.h"

#include "input.h"

Key_State key_states[KEY_LAST];

bool is_key_down(int key_code) {
    bool result = key_states[key_code].is_down;
    return result;
}

bool is_key_pressed(int key_code) {
    bool result = key_states[key_code].is_down && key_states[key_code].changed;
    return result;
}

bool was_key_just_released(int key_code) {
    bool result = key_states[key_code].was_down && !key_states[key_code].is_down;
    return result;
}
