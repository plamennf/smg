#pragma once

struct Font;

const int MAX_SPRITES = 1024;

const int RENDER_ASPECT_RATIO_X = 16;
const int RENDER_ASPECT_RATIO_Y = 9;

extern int render_width;
extern int render_height;

enum Flip_Mode {
    FLIP_MODE_NONE = 0,
    FLIP_MODE_HORIZONTALLY = Bit(1),
    FLIP_MODE_VERTICALLY = Bit(2),
    FLIP_MODE_BOTH = FLIP_MODE_HORIZONTALLY | FLIP_MODE_VERTICALLY
};

struct Texture {
    int width;
    int height;

#ifdef RENDER_D3D11
    struct ID3D11Texture2D *texture;
    struct ID3D11ShaderResourceView *srv;
#endif
};

struct Sprite_Render_Info {
    Texture *texture;
    Vector2 position;
    Vector2 size;
    Vector4 color;
    Vector2 min_uv;
    Vector2 max_uv;
    int layer;
};

struct Per_Scene_Uniforms {
    Matrix4 projection_matrix;
    Matrix4 view_matrix;
    int do_hdr;
    float padding[3];
};

bool renderer_init(bool vsync, bool do_hdr);
void renderer_present();
void renderer_resize();

bool renderer_is_hdr_used();

void renderer_begin_frame(Vector4 clear_color, Per_Scene_Uniforms const &uniforms);
void renderer_end_frame();

void renderer_add_sprite(Sprite_Render_Info const &info);
void renderer_add_sprite(Texture *texture, Vector2 position, Vector2 size, Vector4 color);
void renderer_add_sprite(Texture *texture, Vector2 position, Vector2 size, Rectangle2i *src_rect, Flip_Mode flip_mode, Vector4 color, int layer);
void renderer_add_sprite_center(Texture *texture, Vector2 center, Vector2 size, Vector4 color);
void renderer_add_text(Font *font, char *text, int x, int y, Vector4 color);

bool renderer_create_texture(Texture *texture, int width, int height, void *data);
bool renderer_load_texture(Texture *texture, char *filepath);
