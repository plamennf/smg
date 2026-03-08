#pragma once


enum Key_Code {
    KEY_BACKSPACE       = 0x08,
    KEY_TAB             = 0x09,
    KEY_CLEAR           = 0x0C,
    KEY_ENTER           = 0x0D,
    KEY_SHIFT           = 0x10,
    KEY_CTRL            = 0x11,
    KEY_ALT             = 0x12,
    KEY_PAUSE           = 0x13,
    KEY_CAPS_LOCK       = 0x14,
    KEY_ESCAPE          = 0x1B,
    KEY_SPACE           = 0x20,
    KEY_PAGE_UP         = 0x21,
    KEY_PAGE_DOWN       = 0x22,
    KEY_END	            = 0x23,
    KEY_HOME            = 0x24,
    KEY_LEFT_ARROW      = 0x25,
    KEY_UP_ARROW        = 0x26,
    KEY_RIGHT_ARROW     = 0x27,
    KEY_DOWN_ARROW      = 0x28,
    KEY_SELECT          = 0x29,
    KEY_SNAPSHOT        = 0x2C,
    KEY_INSERT          = 0x2D,
    KEY_DELETE          = 0x2E,
    KEY_0               = 0x30,
    KEY_1               = 0x31,
    KEY_2               = 0x32,
    KEY_3               = 0x33,
    KEY_4               = 0x34,
    KEY_5               = 0x35,
    KEY_6               = 0x36,
    KEY_7               = 0x37,
    KEY_8               = 0x38,
    KEY_9               = 0x39,
    KEY_A               = 0x41,
    KEY_B               = 0x42,
    KEY_C               = 0x43,
    KEY_D               = 0x44,
    KEY_E               = 0x45,
    KEY_F               = 0x46,
    KEY_G               = 0x47,
    KEY_H               = 0x48,
    KEY_I               = 0x49,
    KEY_J               = 0x4A,
    KEY_K               = 0x4B,
    KEY_L               = 0x4C,
    KEY_M               = 0x4D,
    KEY_N               = 0x4E,
    KEY_O               = 0x4F,
    KEY_P               = 0x50,
    KEY_Q               = 0x51,
    KEY_R               = 0x52,
    KEY_S               = 0x53,
    KEY_T               = 0x54,
    KEY_U               = 0x55,
    KEY_V               = 0x56,
    KEY_W               = 0x57,
    KEY_X               = 0x58,
    KEY_Y               = 0x59,
    KEY_Z               = 0x5A,
    KEY_LMETA           = 0x5B,
    KEY_RMETA           = 0x5C,
    KEY_SLEEP           = 0x5F,
    KEY_NP0             = 0x60,
    KEY_NP1             = 0x61,
    KEY_NP2             = 0x62,
    KEY_NP3             = 0x63,
    KEY_NP4             = 0x64,
    KEY_NP5             = 0x65,
    KEY_NP6             = 0x66,
    KEY_NP7             = 0x67,
    KEY_NP8             = 0x68,
    KEY_NP9             = 0x69,
    KEY_NP_MULTIPLY     = 0x6A,
    KEY_NP_ADD          = 0x6B,
    KEY_NP_SEPARATOR    = 0x6C,
    KEY_NP_SUBTRACT     = 0x6D,
    KEY_NP_DECIMAL      = 0x6E,
    KEY_NP_DIVIDE       = 0x6F,
    KEY_F1  = 0x70,
    KEY_F2  = 0x71,
    KEY_F3  = 0x72,
    KEY_F4  = 0x73,
    KEY_F5  = 0x74,
    KEY_F6  = 0x75,
    KEY_F7  = 0x76,
    KEY_F8  = 0x77,
    KEY_F9  = 0x78,
    KEY_F10 = 0x79,
    KEY_F11 = 0x7A,
    KEY_F12 = 0x7B,
    KEY_F13 = 0x7C,
    KEY_F14 = 0x7D,
    KEY_F15 = 0x7E,
    KEY_F16 = 0x7F,
    KEY_F17 = 0x80,
    KEY_F18 = 0x81,
    KEY_F19 = 0x82,
    KEY_F20 = 0x83,
    KEY_F21 = 0x84,
    KEY_F22 = 0x85,
    KEY_F23 = 0x86,
    KEY_F24 = 0x87,
    KEY_NUM_LOCK = 0x90,
    KEY_SCROLL_LOCK = 0x91,
    KEY_LSHIFT = 0xA0,
    KEY_RSHIFT = 0xA1,
    KEY_LCTRL  = 0xA2,
    KEY_RCTRL  = 0xA3,
    //KEY_LMETA  = 0xA4,
    //KEY_RMETA  = 0xA5,
    KEY_SEMICOLON_COLON = 0xBA,
    KEY_EQUALS_PLUS = 0xBB,
    KEY_COMMA_LESS_THAN = 0xBC,
    KEY_DASH_UNDERSCORE = 0xBD,
    KEY_PERIOD_GREATER_THAN = 0xBE,
    KEY_FORWARD_SLASH_QUESTION_MARK = 0xBF,
    KEY_GRAVE_ACCENT_TILDE = 0xC0,

    KEY_GAMEPAD_A = 0xC3,
    KEY_GAMEPAD_B = 0xC4,
    KEY_GAMEPAD_X = 0xC5,
    KEY_GAMEPAD_Y = 0xC6,
    KEY_GAMEPAD_RIGHT_SHOULDER = 0xC7,
    KEY_GAMEPAD_LEFT_SHOULDER = 0xC8,
    KEY_GAMEPAD_LEFT_TRIGGER = 0xC9,
    KEY_GAMEPAD_RIGHT_TRIGGER = 0xCA,
    KEY_GAMEPAD_DPAD_UP = 0xCB,
    KEY_GAMEPAD_DPAD_DOWN = 0xCC,
    KEY_GAMEPAD_DPAD_LEFT = 0xCD,
    KEY_GAMEPAD_DPAD_RIGHT = 0xCE,
    KEY_GAMEPAD_START = 0xCF,
    KEY_GAMEPAD_BACK = 0xD0,
    KEY_GAMEPAD_LEFT_THUMBSTICK_BUTTON = 0xD1,
    KEY_GAMEPAD_RIGHT_THUMBSTICK_BUTTON = 0xD2,
    KEY_GAMEPAD_LEFT_THUMBSTICK_UP = 0xD3,
    KEY_GAMEPAD_LEFT_THUMBSTICK_DOWN = 0xD4,
    KEY_GAMEPAD_LEFT_THUMBSTICK_RIGHT = 0xD5,
    KEY_GAMEPAD_LEFT_THUMBSTICK_LEFT = 0xD6,
    KEY_GAMEPAD_RIGHT_THUMBSTICK_UP = 0xD7,
    KEY_GAMEPAD_RIGHT_THUMBSTICK_DOWN = 0xD8,
    KEY_GAMEPAD_RIGHT_THUMBSTICK_RIGHT = 0xD9,
    KEY_GAMEPAD_RIGHT_THUMBSTICK_LEFT = 0xDA,

    KEY_LEFT_BRACE  = 0xDB,
    KEY_BACKSLASH   = 0xDC,
    KEY_RIGHT_BRACE = 0xDD,
    KEY_APOSTROPHE_DOUBLE_QUOTATION_MARK = 0xDE,

    NUM_KEY_CODES,
};

enum Mouse_Button {
    MOUSE_BUTTON_LEFT   = 0x01,
    MOUSE_BUTTON_RIGHT  = 0x02,
    MOUSE_BUTTON_MIDDLE = 0x04,
    MOUSE_BUTTON_X1     = 0x05,
    MOUSE_BUTTON_X2     = 0x06,

    NUM_MOUSE_BUTTONS,
};

struct Key_State {
    bool is_down;
    bool was_down;
    bool changed;
};

struct Mouse_Button_State {
    bool is_down;
    bool was_down;
    bool changed;
};

extern int mouse_cursor_x;
extern int mouse_cursor_y;

extern int mouse_cursor_x_delta;
extern int mouse_cursor_y_delta;

extern int mouse_scroll_wheel_x_delta;
extern int mouse_scroll_wheel_y_delta;

bool is_key_down(Key_Code key_code);
bool is_key_pressed(Key_Code key_code);
bool was_key_just_released(Key_Code key_code);

void set_key_state(Key_Code key_code, bool is_down);
void clear_key_states();

bool is_mouse_button_down(Mouse_Button button);
bool is_mouse_button_pressed(Mouse_Button button);
bool was_mouse_button_just_released(Mouse_Button button);

void set_mouse_button_state(Mouse_Button button, bool is_down);
void clear_mouse_button_states();
