cbuffer Per_Scene_Uniforms : register(b0) {
    float4x4 projection_matrix;
    float4x4 view_matrix;
    int do_hdr;
};

struct Sprite_Vertex_Input {
    float2 quad_position : POSITION;
    float2 quad_uv       : TEXCOORD;

    float2 position     : INSTANCE_POSITION;
    float2 size         : INSTANCE_SIZE;
    float4 color        : INSTANCE_COLOR;
    float2 uv_min       : INSTANCE_MIN_UV;
    float2 uv_max       : INSTANCE_MAX_UV;
};

SamplerState sampler_point  : register(s0);
SamplerState sampler_linear : register(s1);

Texture2D diffuse_texture : register(t0);
