#include "shader_globals.hlsl"

struct Vertex_Output {
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 uv       : TEXCOORD;
};

Vertex_Output vertex_main(Sprite_Vertex_Input input) {
    Vertex_Output result;

    float2 world_position = input.position + input.quad_position * input.size;
    
    result.position = mul(projection_matrix, mul(view_matrix, float4(world_position, 0.0, 1.0)));
    result.color    = input.color;
    result.uv       = lerp(input.uv_min, input.uv_max, input.quad_uv);

    return result;
}

float4 pixel_main(Vertex_Output input) : SV_TARGET {
    float4 tex_color = diffuse_texture.Sample(sampler_point, input.uv);
    return tex_color * input.color;
}
