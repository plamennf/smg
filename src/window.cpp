#include "main.h"

#include "window.h"
#include "input.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int window_width = 0;
int window_height = 0;
bool window_is_open = false;

extern Key_State key_states[KEY_LAST];

static GLFWwindow *window = NULL;
static u64 last_time;

static Vector2i prev_window_position = {};
static Vector2i prev_window_size = {};
static GLFWmonitor *monitor = {};

static void glfw_error_callback(int error_code, const char *description) {
    fprintf(stderr, "GLFW Error(%d): %s\n", error_code, description);
}

static void glfw_close_callback(GLFWwindow *window) {
    window_is_open = false;
}

static void glfw_resize_callback(GLFWwindow *window, int width, int height) {
    window_width  = width;
    window_height = height;
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    Key_State *state = &key_states[key];

    bool is_down = action == GLFW_PRESS || action == GLFW_REPEAT;

    state->changed = state->is_down != is_down;
    state->is_down = is_down;

    if (is_down && state->changed) {
        if (key == KEY_ENTER && (mods & GLFW_MOD_ALT)) {
            window_toggle_fullscreen();
        }
    }
}

static void glfw_center_window(GLFWwindow *window) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    int left, top, right, bottom;
    glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int x = (mode->width  - width)  / 2;
    int y = (mode->height - height) / 2;
    glfwSetWindowPos(window, x, y);
}

bool window_init(int width, int height, char *title) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW!\n");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create the window!\n");
        return false;
    }
    glfwMakeContextCurrent(window);

    glfw_center_window(window);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to load opengl extension functions!\n");
        return false;
    }

    glfwSetWindowCloseCallback(window, glfw_close_callback);
    glfwSetWindowSizeCallback(window, glfw_resize_callback);
    glfwSetKeyCallback(window, glfw_key_callback);

    glfwSwapInterval(1);
    
    window_width   = width;
    window_height  = height;
    window_is_open = true;

    monitor = glfwGetPrimaryMonitor();
    glfwGetWindowPos(window, &prev_window_position.x, &prev_window_position.y);
    glfwGetWindowSize(window, &prev_window_size.x, &prev_window_size.y);
    
    last_time = get_time_nanoseconds();
    
    return true;
}

void window_poll_events() {
    for (int i = 0; i < KEY_LAST; i++) {
        Key_State *state = &key_states[i];
        state->was_down  = state->is_down;
        state->changed   = false;
    }
    
    glfwPollEvents();
}

void window_swap_buffers() {
    glfwSwapBuffers(window);
}

void window_sync(int fps_cap) {
    u64 fps_cap_nanoseconds = 1000000000 / fps_cap;
    
    while (get_time_nanoseconds() <= last_time + fps_cap_nanoseconds) {
        // @TODO: Maybe sleep.
    }
    last_time += fps_cap_nanoseconds;
}

void window_toggle_fullscreen() {
    bool is_fullscreen = glfwGetWindowMonitor(window);
    bool should_fullscreen = !is_fullscreen;

    if (should_fullscreen) {
        glfwGetWindowPos(window, &prev_window_position.x, &prev_window_position.y);
        glfwGetWindowSize(window, &prev_window_size.x, &prev_window_size.y);

        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(window, NULL, prev_window_position.x, prev_window_position.y, prev_window_size.x, prev_window_size.y, 0);
    }
}
