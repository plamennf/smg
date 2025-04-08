#include "main.h"

#include "window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

int window_width = 0;
int window_height = 0;
bool window_is_open = false;

static GLFWwindow *window = NULL;

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

    glfwSwapInterval(1);
    
    window_width   = width;
    window_height  = height;
    window_is_open = true;
    
    return true;
}

void window_poll_events() {
    glfwPollEvents();
}

void window_swap_buffers() {
    glfwSwapBuffers(window);
}
