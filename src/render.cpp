#include "main.h"

#include "window.h"
#include "render.h"
#include "font.h"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

int render_width  = 16;
int render_height = 9;

static int prev_window_width;
static int prev_window_height;

struct Texture {
    int width;
    int height;

    GLuint id;
};

static Render_Commands the_render_commands;
static Texture *white_texture;

static GLuint main_fbo;
static GLuint main_fbo_color_texture;

static GLuint light_fbo;
static GLuint light_fbo_color_texture;

static GLuint the_vbo;
static GLuint the_ibo;

static GLuint the_fullscreen_vbo;
static GLuint the_fullscreen_ibo;

static GLuint the_quad_shader;
static GLint the_quad_shader_view_proj_loc;

static GLuint the_postprocess_shader;
static GLint the_postprocess_shader_proj_loc;

static void init_render_targets() {
    float optimal_window_width = (float)window_height * ((float)render_width / (float)render_height);
    float optimal_window_height = (float)window_width * ((float)render_height / (float)render_width);

    if ((float)window_width > optimal_window_width) {
        // Black bars on the sides.
        render_height = window_height;
        render_width  = (int)optimal_window_width;
    } else {
        render_width  = window_width;
        render_height = (int)optimal_window_height;
    }

    if (light_fbo_color_texture) {
        glDeleteTextures(1, &light_fbo_color_texture);
        light_fbo_color_texture = 0;
    }

    if (light_fbo) {
        glDeleteFramebuffers(1, &light_fbo);
        light_fbo = 0;
    }
    
    if (main_fbo_color_texture) {
        glDeleteTextures(1, &main_fbo_color_texture);
        main_fbo_color_texture = 0;
    }

    if (main_fbo) {
        glDeleteFramebuffers(1, &main_fbo);
        main_fbo = 0;
    }

    glGenFramebuffers(1, &main_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, main_fbo);

    glGenTextures(1, &main_fbo_color_texture);
    glBindTexture(GL_TEXTURE_2D, main_fbo_color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, render_width, render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, main_fbo_color_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Main Framebuffer is not complete!!!\n");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &light_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);

    glGenTextures(1, &light_fbo_color_texture);
    glBindTexture(GL_TEXTURE_2D, light_fbo_color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, render_width, render_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light_fbo_color_texture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Light Framebuffer is not complete!!!\n");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static GLuint load_shader_from_file(char *filepath) {
    char *file_data = read_entire_file(filepath);
    if (!file_data) {
        fprintf(stderr, "Failed to read file '%s'.\n", filepath);
        return 0;
    }
    defer(delete [] file_data);

    char *vertex_source[] = {
        "#version 330 core\n#define VERTEX_SHADER\n#define OUT_IN out\n#line 1 1\n",
        file_data
    };

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    defer(glDeleteShader(v));
    glShaderSource(v, ArrayCount(vertex_source), vertex_source, NULL);
    glCompileShader(v);
    int success = false;
    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[4096];
        glGetShaderInfoLog(v, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Failed to compile '%s' vertex shader:\n%s\n", filepath, info_log);
        return 0;
    }
    
    char *fragment_source[] = {
        "#version 330 core\n#define FRAGMENT_SHADER\n#define OUT_IN in\n#line 1 1\n",
        file_data
    };

    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    defer(glDeleteShader(f));
    glShaderSource(f, ArrayCount(fragment_source), fragment_source, NULL);
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[4096];
        glGetShaderInfoLog(f, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Failed to compile '%s' fragment shader:\n%s\n", filepath, info_log);
        return 0;
    }

    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[4096];
        glGetProgramInfoLog(p, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Failed to link '%s' shader program:\n%s\n", filepath, info_log);
        glDeleteProgram(p);
        return 0;
    }
    glValidateProgram(p);
    glGetProgramiv(p, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char info_log[4096];
        glGetProgramInfoLog(p, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "Failed to validate '%s' shader program:\n%s\n", filepath, info_log);
        glDeleteProgram(p);
        return 0;
    }

    return p;
}

void render_init() {
    the_render_commands.max_size = 65535;
    the_render_commands.base     = new u8[the_render_commands.max_size];
    the_render_commands.at       = the_render_commands.base;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_MULTISAMPLE);

    u32 max_quads = 20000;
    
    the_render_commands.max_vertices = max_quads * 4;
    the_render_commands.vertices     = new Render_Vertex[the_render_commands.max_vertices];
    the_render_commands.num_vertices = 0;
    
    glGenBuffers(1, &the_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, the_vbo);
    glBufferData(GL_ARRAY_BUFFER, the_render_commands.max_vertices * sizeof(Render_Vertex), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    the_render_commands.max_indices = max_quads * 6;
    the_render_commands.indices     = new u32[the_render_commands.max_indices];
    the_render_commands.num_indices = 0;
    
    glGenBuffers(1, &the_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, the_render_commands.max_indices * sizeof(u32), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    u32 fullscreen_indices[] = {
        0, 1, 2,
        0, 2, 3
    };
    
    glGenBuffers(1, &the_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, the_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Render_Vertex), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &the_fullscreen_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_fullscreen_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fullscreen_indices), fullscreen_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    the_quad_shader = load_shader_from_file("data/shaders/quad.glsl");
    the_quad_shader_view_proj_loc = glGetUniformLocation(the_quad_shader, "view_proj");

    the_postprocess_shader = load_shader_from_file("data/shaders/postprocess.glsl");
    the_postprocess_shader_proj_loc = glGetUniformLocation(the_postprocess_shader, "proj");
    
    //init_render_targets();

    u8 white_texture_data[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    white_texture = load_texture_from_memory(1, 1, white_texture_data);
}

Texture *load_texture_from_memory(int width, int height, u8 *data) {
    Texture *texture = new Texture();
    texture->width   = width;
    texture->height  = height;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

Texture *load_texture_from_file(char *filepath) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    u8 *data = stbi_load(filepath, &width, &height, &channels, 4);
    if (!data) {
        fprintf(stderr, "Failed to load image '%s'.\n", filepath);
        return NULL;
    }

    Texture *texture = load_texture_from_memory(width, height, data);
    return texture;
}

Render_Commands *render_begin_frame() {
    the_render_commands.at            = the_render_commands.base;

    the_render_commands.num_vertices  = 0;
    the_render_commands.num_indices   = 0;
    
    the_render_commands.current_quads = 0;
    
    return &the_render_commands;
}

void render_end_frame(Render_Commands *commands) {
    if (prev_window_width  != window_width ||
        prev_window_height != window_height) {
        init_render_targets();
        prev_window_width  = window_width;
        prev_window_height = window_height;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, the_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, commands->num_vertices * sizeof(Render_Vertex), commands->vertices);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, commands->num_indices * sizeof(u32), commands->indices);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    glm::mat4 proj = glm::orthoRH(0.0, (double)render_width, 0.0, (double)render_height, -1.0, 1.0);
    
    for (u8 *at = commands->base; at < commands->at;) {
        Render_Command_Type type = *(Render_Command_Type *)at;
        at++;

        switch (type) {
            case RCT_Render_Command_Set_Render_Target: {
                auto command = (Render_Command_Set_Render_Target *)at;
                at += sizeof(*command);

                auto config = command->config;

                if (config.render_target == RENDER_TARGET_BACKBUFFER) {
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    glDisable(GL_BLEND);
    
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glViewport(0, 0, window_width, window_height);

                    glClearColor(config.color.r, config.color.g, config.color.b, 1);
                    glClear(GL_COLOR_BUFFER_BIT);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, main_fbo_color_texture);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, light_fbo_color_texture);
    
                    proj = glm::orthoRH(0.0, (double)window_width, 0.0, (double)window_height, -1.0, 1.0);

                    float ox = 0.5f * (window_width  - render_width);
                    float oy = 0.5f * (window_height - render_height);
                    float ow = (float)render_width;
                    float oh = (float)render_height;
    
                    Render_Vertex fullscreen_vertices[] = {
                        { { ox,      oy },      { 1, 1, 1, 1 }, { 0, 0 } },
                        { { ox + ow, oy },      { 1, 1, 1, 1 }, { 1, 0 } },
                        { { ox + ow, oy + oh }, { 1, 1, 1, 1 }, { 1, 1 } },
                        { { ox,      oy + oh }, { 1, 1, 1, 1 }, { 0, 1 } },
                    };
    
                    glUseProgram(the_postprocess_shader);
                    glUniformMatrix4fv(the_postprocess_shader_proj_loc, 1, GL_FALSE, &proj[0][0]);
                    glUniform1i(glGetUniformLocation(the_postprocess_shader, "u_texture1"), 0);
                    glUniform1i(glGetUniformLocation(the_postprocess_shader, "u_texture2"), 1);

                    glBindBuffer(GL_ARRAY_BUFFER, the_fullscreen_vbo);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fullscreen_vertices), fullscreen_vertices);
    
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_fullscreen_ibo);

                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, position));
                    glEnableVertexAttribArray(0);
                
                    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, color));
                    glEnableVertexAttribArray(1);

                    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, uv));
                    glEnableVertexAttribArray(2);

                    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                    continue;
                }
                
                switch (config.render_target) {
                    case RENDER_TARGET_MAIN: {
                        glBindFramebuffer(GL_FRAMEBUFFER, main_fbo);
                    } break;

                    case RENDER_TARGET_LIGHTS: {
                        glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
                    } break;
                }
                
                glViewport(0, 0, render_width, render_height);
                
                GLenum clear_flags = 0;
                if (config.clear_color) {
                    glClearColor(config.color.r, config.color.g, config.color.b, config.color.a);
                    clear_flags |= GL_COLOR_BUFFER_BIT;
                }
                glClear(clear_flags);
            } break;

            case RCT_Render_Command_Render_Quads: {
                auto command = (Render_Command_Render_Quads *)at;
                at += sizeof(*command);

                switch (command->setup.blend_mode) {
                    case BLEND_MODE_OFF: {
                        glDisable(GL_BLEND);
                    } break;
                    
                    case BLEND_MODE_ALPHA: {
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    } break;

                    case BLEND_MODE_SSLM: {
                        glEnable(GL_BLEND);
                        glBlendFuncSeparate(GL_DST_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
                    } break;
                }
                
                glUseProgram(the_quad_shader);
                glUniformMatrix4fv(the_quad_shader_view_proj_loc, 1, GL_FALSE, &proj[0][0]);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, command->texture ? command->texture->id : 0);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                if (command->setup.point_sample_textures) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                } else {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
                
                glBindBuffer(GL_ARRAY_BUFFER, the_vbo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, the_ibo);

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, position));
                glEnableVertexAttribArray(0);
                
                glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, color));
                glEnableVertexAttribArray(1);

                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void *)offsetof(Render_Vertex, uv));
                glEnableVertexAttribArray(2);

                u64 offset = (u64)command->index_array_offset * sizeof(u32);
                glDrawElements(GL_TRIANGLES, command->num_indices, GL_UNSIGNED_INT, (void *)offset);
            } break;
        }
    }
}

#define add_render_command(commands, Type) (Type *)add_render_command_(commands, sizeof(Type), RCT_##Type)
void *add_render_command_(Render_Commands *commands, u32 data_size, Render_Command_Type type) {
    assert(commands->at + data_size + 1 <= commands->base + commands->max_size);

    commands->current_quads = NULL;
    
    *commands->at = (u8)type;
    commands->at++;

    void *result = (void *)commands->at;
    commands->at += data_size;
    return result;
}

void set_render_target(Render_Commands *commands, Render_Target_Config config) {
    auto command = add_render_command(commands, Render_Command_Set_Render_Target);
    assert(command);

    memcpy(&command->config, &config, sizeof(config));
}

Render_Setup default_render_setup() {
    Render_Setup setup;

    setup.point_sample_textures = false;
    setup.blend_mode = BLEND_MODE_OFF;
    
    return setup;
}

void set_render_setup(Render_Commands *rc, Render_Setup setup) {
    rc->current_setup = setup;
    rc->current_quads = NULL;
}

static Render_Command_Render_Quads *get_current_quads(Render_Commands *commands, int num_vertices, int num_indices, Texture *texture) {
    assert(commands->num_vertices + num_vertices <= commands->max_vertices);
    assert(commands->num_indices  + num_indices  <= commands->max_indices);

    if (commands->current_quads && commands->current_quads->texture != texture) {
        commands->current_quads = NULL;
    }
    
    if (!commands->current_quads) {
        commands->current_quads = add_render_command(commands, Render_Command_Render_Quads);
        commands->current_quads->texture = texture;
        commands->current_quads->num_vertices = 0;
        commands->current_quads->num_indices = 0;
        commands->current_quads->index_array_offset = commands->num_indices;
        commands->current_quads->setup = commands->current_setup;
    }

    return commands->current_quads;
}

static void push_quad(Render_Commands *rc, Render_Command_Render_Quads *command,
                      glm::vec2 const &p0, glm::vec4 const &c0, glm::vec2 const &uv0,
                      glm::vec2 const &p1, glm::vec4 const &c1, glm::vec2 const &uv1,
                      glm::vec2 const &p2, glm::vec4 const &c2, glm::vec2 const &uv2,
                      glm::vec2 const &p3, glm::vec4 const &c3, glm::vec2 const &uv3) {
    command->num_vertices += 4;
    command->num_indices  += 6;

    rc->vertices[rc->num_vertices + 0].position = p0;
    rc->vertices[rc->num_vertices + 0].color    = c0;
    rc->vertices[rc->num_vertices + 0].uv       = uv0;

    rc->vertices[rc->num_vertices + 1].position = p1;
    rc->vertices[rc->num_vertices + 1].color    = c1;
    rc->vertices[rc->num_vertices + 1].uv       = uv1;

    rc->vertices[rc->num_vertices + 2].position = p2;
    rc->vertices[rc->num_vertices + 2].color    = c2;
    rc->vertices[rc->num_vertices + 2].uv       = uv2;

    rc->vertices[rc->num_vertices + 3].position = p3;
    rc->vertices[rc->num_vertices + 3].color    = c3;
    rc->vertices[rc->num_vertices + 3].uv       = uv3;

    rc->indices[rc->num_indices + 0] = rc->num_vertices + 0;
    rc->indices[rc->num_indices + 1] = rc->num_vertices + 1;
    rc->indices[rc->num_indices + 2] = rc->num_vertices + 2;
    rc->indices[rc->num_indices + 3] = rc->num_vertices + 0;
    rc->indices[rc->num_indices + 4] = rc->num_vertices + 2;
    rc->indices[rc->num_indices + 5] = rc->num_vertices + 3;

    rc->num_vertices += 4;
    rc->num_indices  += 6;
}

void render_quad(Render_Commands *rc, Texture *texture, glm::vec2 const &position, glm::vec2 const &size, glm::vec4 const &color) {
    render_quad(rc, texture, position, size, NULL, FLIP_MODE_NONE, color);
}

void render_quad(Render_Commands *rc, Texture *texture, glm::vec2 const &position, glm::vec2 const &size, Rectangle2i *src_rect, Flip_Mode flip_mode, glm::vec4 const &color) {
    if (!texture) texture = white_texture;
    
    auto command = get_current_quads(rc, 4, 6, texture);
    assert(command);

    glm::vec2 p0 = position;
    glm::vec2 p1(position.x + size.x, position.y);
    glm::vec2 p2 = position + size;
    glm::vec2 p3(position.x, position.y + size.y);

    glm::vec2 uv0(0, 0);
    glm::vec2 uv1(1, 0);
    glm::vec2 uv2(1, 1);
    glm::vec2 uv3(0, 1);

    if (src_rect) {
        assert(texture->width > 0);
        assert(texture->height > 0);
        
        float min_uv_x = (float)src_rect->x / (float)texture->width;
        float max_uv_x = (float)(src_rect->x + src_rect->width) / (float)texture->width;

        float min_uv_y = (float)src_rect->y / (float)texture->height;
        float max_uv_y = (float)(src_rect->y + src_rect->height) / (float)texture->height;

        uv0 = {min_uv_x, min_uv_y};
        uv1 = {max_uv_x, min_uv_y};
        uv2 = {max_uv_x, max_uv_y};
        uv3 = {min_uv_x, max_uv_y};
    }

    if (flip_mode & FLIP_MODE_HORIZONTALLY) {
        float min_uv_x = uv0.x;
        float max_uv_x = uv1.x;

        uv0.x = max_uv_x;
        uv1.x = min_uv_x;
        uv2.x = min_uv_x;
        uv3.x = max_uv_x;
    }

    if (flip_mode & FLIP_MODE_VERTICALLY) {
        float min_uv_y = uv0.y;
        float max_uv_y = uv2.y;

        uv0.y = max_uv_y;
        uv1.y = max_uv_y;
        uv2.y = min_uv_y;
        uv3.y = min_uv_y;
    }

    push_quad(rc, command,
              p0, color, uv0,
              p1, color, uv1,
              p2, color, uv2,
              p3, color, uv3);    
}

void render_triangle(Render_Commands *rc, glm::vec2 const &p0, glm::vec2 const &p1, glm::vec2 const &p2, glm::vec4 const &color) {
    auto command = get_current_quads(rc, 3, 3, white_texture);
    assert(command);

    command->num_vertices += 3;
    command->num_indices  += 3;
    
    rc->vertices[rc->num_vertices + 0].position = p0;
    rc->vertices[rc->num_vertices + 0].color    = color;
    rc->vertices[rc->num_vertices + 0].uv       = {0, 0};

    rc->vertices[rc->num_vertices + 1].position = p1;
    rc->vertices[rc->num_vertices + 1].color    = color;
    rc->vertices[rc->num_vertices + 1].uv       = {0, 0};

    rc->vertices[rc->num_vertices + 2].position = p2;
    rc->vertices[rc->num_vertices + 2].color    = color;
    rc->vertices[rc->num_vertices + 2].uv       = {0, 0};

    rc->indices[rc->num_indices + 0] = rc->num_vertices + 0;
    rc->indices[rc->num_indices + 1] = rc->num_vertices + 1;
    rc->indices[rc->num_indices + 2] = rc->num_vertices + 2;

    rc->num_vertices += 3;
    rc->num_indices  += 3;
}

static glm::vec2 get_vec2(float theta) {
    float ct = glm::cos(theta);
    float st = glm::sin(theta);

    return glm::vec2(ct, st);
}

void render_circle(Render_Commands *rc, glm::vec2 const &center, float radius, glm::vec4 const &color) {
    const int NUM_TRIANGLES = 100;
    float dtheta = (2.0f * glm::pi<float>()) / NUM_TRIANGLES;
    float r = radius;

    for (int i = 0; i < NUM_TRIANGLES; i++) {
        float theta0 = i * dtheta;
        float theta1 = (i+1) * dtheta;

        glm::vec2 v0 = get_vec2(theta0);
        glm::vec2 v1 = get_vec2(theta1);

        glm::vec2 p0 = center;
        glm::vec2 p1 = center + r * v0;
        glm::vec2 p2 = center + r * v1;

        render_triangle(rc, p0, p1, p2, color);
    }
}

void render_text(Render_Commands *rc, Font *font, char *text, int x, int y, glm::vec4 const &color) {
    for (char *at = text; *at; at++) {
        Glyph *glyph = &font->glyphs[*at];
        if (!glyph) continue;

        if (*at == '\n') {
            printf("Reached new line in render_text: Stopping drawing!\n");
            break;
        }

        if (!is_space(*at)) {
            glm::vec2 pos;
            pos.x = (float)(x + glyph->bearing_x);
            pos.y = (float)(y - (glyph->size_y - glyph->bearing_y));

            glm::vec2 size;
            size.x = (float)glyph->size_x;
            size.y = (float)glyph->size_y;
            
            render_quad(rc, font->texture, pos, size, &glyph->src_rect, FLIP_MODE_VERTICALLY, color);
        }

        x += glyph->advance;
    }
}
