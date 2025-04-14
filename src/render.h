#pragma once

struct Texture;
struct Font;

struct Render_Vertex {
    Vector2 position;
    Vector4 color;
    Vector2 uv;
};

enum Render_Command_Type : u8 {
    RCT_Render_Command_Set_Render_Target,
    RCT_Render_Command_Render_Quads,
};

enum Render_Target {
    RENDER_TARGET_MAIN,
    RENDER_TARGET_LIGHTS,
    RENDER_TARGET_BACKBUFFER, // Always run backbuffer last, because it blits the main and light buffer onto the backbuffer.
};

struct Render_Target_Config {
    Render_Target render_target;
    bool clear_color;
    Vector4 color;
};

struct Render_Command_Set_Render_Target {
    Render_Target_Config config;
};

enum Blend_Mode {
    BLEND_MODE_OFF,
    BLEND_MODE_ALPHA,
    BLEND_MODE_SSLM,
};

struct Render_Setup {
    bool point_sample_textures;
    Blend_Mode blend_mode;
};

struct Render_Command_Render_Quads {
    Texture *texture;
    int num_vertices;
    int num_indices;
    int index_array_offset;
    Render_Setup setup;
};

struct Render_Commands {
    u32 max_size;
    u8 *base;
    u8 *at;

    Render_Command_Render_Quads *current_quads;
    Render_Setup current_setup;
    
    int max_vertices;
    Render_Vertex *vertices;
    int num_vertices;

    int max_indices;
    u32 *indices;
    int num_indices;
};

enum Flip_Mode {
    FLIP_MODE_NONE = Bit(0),
    FLIP_MODE_HORIZONTALLY = Bit(1),
    FLIP_MODE_VERTICALLY = Bit(2),
    FLIP_MODE_BOTH = FLIP_MODE_HORIZONTALLY | FLIP_MODE_VERTICALLY,
};

extern int render_width;
extern int render_height;

void render_init();

Texture *load_texture_from_memory(int width, int height, u8 *data);
Texture *load_texture_from_file(char *filepath);

Render_Commands *render_begin_frame();
void render_end_frame(Render_Commands *commands);

void set_render_target(Render_Commands *commands, Render_Target_Config config);

Render_Setup default_render_setup();
void set_render_setup(Render_Commands *rc, Render_Setup setup);

void render_quad(Render_Commands *rc, Texture *texture, Vector2 const &position, Vector2 const &size, Vector4 const &color);
void render_quad(Render_Commands *rc, Texture *texture, Vector2 const &position, Vector2 const &size, Rectangle2i *src_rect, Flip_Mode flip_mode, Vector4 const &color);
void render_quad(Render_Commands *rc, Texture *texture, Vector2 const &position, Vector2 const &size, float rotation, Vector4 const &color);

void render_circle(Render_Commands *rc, Vector2 const &center, float radius, Vector4 const &color);
void render_text(Render_Commands *rc, Font *font, char *text, int x, int y, Vector4 const &color);
