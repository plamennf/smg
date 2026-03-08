#include "shader_globals.hlsl"

struct Vertex_Input {
    float2 position : POSITION;
    float2 uv       : TEXCOORD;
};

struct Vertex_Output {
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD;
};

Vertex_Output vertex_main(Vertex_Input input) {
    Vertex_Output result;

    result.position = float4(input.position, 0.0, 1.0);
    result.uv       = input.uv;
    
    return result;
}

float4 pixel_main(Vertex_Output input) : SV_TARGET {
    float exposure = 1.0;
    
    float4 tex_color = diffuse_texture.Sample(sampler_point, input.uv);

    float3 mapped = tex_color.rgb;
    if (do_hdr) {
        mapped = 1.0 - exp(-tex_color.rgb * exposure);
    }
        
    return float4(mapped, 1.0);
}
