#include "main.h"

#ifdef RENDER_D3D11

#include "renderer.h"
#include "font.h"

#include <d3d11.h>
#include <dxgi1_6.h>
#include <float.h>
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define SHADER_DIRECTORY "data/shaders/compiled"

#define SafeRelease(ptr) if (ptr) { ptr->Release(); ptr = NULL; }

static bool do_hdr = false;

enum Render_Vertex_Type {
    RENDER_VERTEX_SPRITE,
    RENDER_VERTEX_FULLSCREEN_QUAD,
};

struct Sprite_Vertex {
    Vector2 position;
    Vector2 uv;
};

struct Sprite_Region {
    Texture *texture;
    int instance_offset;
    int num_instances;
};

int render_width = 0;
int render_height = 0;

static bool should_vsync;

static ID3D11Device *device;
static ID3D11DeviceContext *device_context;
static IDXGISwapChain *swap_chain;
static u32 swap_chain_flags;

static ID3D11Texture2D *back_buffer_texture;
static ID3D11RenderTargetView *back_buffer_rtv;

static ID3D11Texture2D *offscreen_buffer_texture;
static ID3D11RenderTargetView *offscreen_buffer_rtv;
static ID3D11ShaderResourceView *offscreen_buffer_srv;

static ID3D11BlendState *blend_enabled_bs;
static ID3D11RasterizerState *default_rasterizer_state;
static ID3D11DepthStencilState *depth_disabled_dss;

static ID3D11SamplerState *sampler_point        = NULL;
static ID3D11SamplerState *sampler_linear       = NULL;
static ID3D11SamplerState *sampler_point_clamp  = NULL;
static ID3D11SamplerState *sampler_linear_clamp = NULL;

static ID3D11InputLayout *sprite_input_layout;
static ID3D11InputLayout *fullscreen_quad_input_layout;

static ID3D11VertexShader *sprite_vertex_shader;
static ID3D11PixelShader *sprite_pixel_shader;

static ID3D11VertexShader *resolve_vertex_shader;
static ID3D11PixelShader *resolve_pixel_shader;

static ID3D11Buffer *per_scene_uniforms_cb;

static ID3D11Buffer *fullscreen_quad_vb;
static ID3D11Buffer *fullscreen_quad_ib;

static ID3D11Buffer *sprite_quad_vb;
static ID3D11Buffer *sprite_quad_ib;
static ID3D11Buffer *sprite_instance_buffer;

static Vector4 clear_color;
static Per_Scene_Uniforms per_scene_uniforms;
static Sprite_Render_Info *sprite_instances;
static Sprite_Region *sprite_regions;
static int num_sprite_instances;

static bool load_shader(ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, char *filename, Render_Vertex_Type render_vertex_type);

static bool init_shaders() {
#define LOAD_SHADER(name, vertex_type) if (!load_shader(&name##_vertex_shader, &name##_pixel_shader, #name, vertex_type)) return false;

    LOAD_SHADER(sprite, RENDER_VERTEX_SPRITE);
    LOAD_SHADER(resolve, RENDER_VERTEX_FULLSCREEN_QUAD);

#undef LOAD_SHADER

    return true;
}

static void init_back_buffer() {
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer_texture));

    D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {};
    rtv_desc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    device->CreateRenderTargetView(back_buffer_texture, &rtv_desc, &back_buffer_rtv);

    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width              = render_width;
    texture_desc.Height             = render_height;
    texture_desc.MipLevels          = 1;
    texture_desc.ArraySize          = 1;

    if (do_hdr) {
        texture_desc.Format             = DXGI_FORMAT_R16G16B16A16_FLOAT;
    } else {
        texture_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    }
    
    texture_desc.SampleDesc.Count   = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage              = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    device->CreateTexture2D(&texture_desc, NULL, &offscreen_buffer_texture);

    rtv_desc.Format = texture_desc.Format;
    device->CreateRenderTargetView(offscreen_buffer_texture, &rtv_desc, &offscreen_buffer_rtv);

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format                    = texture_desc.Format;
    srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels       = 1;
    device->CreateShaderResourceView(offscreen_buffer_texture, &srv_desc, &offscreen_buffer_srv);
}

static void release_back_buffer() {
    SafeRelease(offscreen_buffer_srv);
    SafeRelease(offscreen_buffer_rtv);
    SafeRelease(offscreen_buffer_texture);

    SafeRelease(back_buffer_rtv);
    SafeRelease(back_buffer_texture);
}

bool renderer_init(bool vsync, bool _do_hdr) {
    do_hdr = _do_hdr;
    
    should_vsync = vsync;
    if (!should_vsync) {
        swap_chain_flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    }
    
    DXGI_SWAP_CHAIN_DESC swap_chain_desc = {};
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.SampleDesc.Count  = 1;
    swap_chain_desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.BufferCount       = 2;
    swap_chain_desc.OutputWindow      = (HWND)os_window_get_native();
    swap_chain_desc.Windowed          = TRUE;
    swap_chain_desc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.Flags             = swap_chain_flags;

    IDXGIFactory6 *factory = NULL;
    defer { SafeRelease(factory); };
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    IDXGIAdapter1 *adapter = NULL;
    defer { SafeRelease(adapter); };
    
    factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
    
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0 };

    UINT create_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef BUILD_DEBUG
    create_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    D3D11CreateDeviceAndSwapChain(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, create_flags, feature_levels, ArrayCount(feature_levels), D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain, &device, NULL, &device_context);

    if (!swap_chain) {
        logprintf("Failed to create dxgi swap chain!\n");
        return false;
    }

    if (!device) {
        logprintf("Failed to create d3d11 device!\n");
        return false;
    }
    
    if (!device_context) {
        logprintf("Failed to create d3d11 device context!\n");
        return false;
    }

    //
    // Create blend states
    //
    {
        D3D11_BLEND_DESC blend_desc = {};
        blend_desc.RenderTarget[0].BlendEnable = TRUE;
        blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        blend_desc.AlphaToCoverageEnable = FALSE;
        device->CreateBlendState(&blend_desc, &blend_enabled_bs);
        if (!blend_enabled_bs) {
            logprintf("Failed to create blend enabled blend state!\n");
            return false;
        }
    }

    //
    // Create rasterizer states
    //
    {
        D3D11_RASTERIZER_DESC rasterizer_desc = {};
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = TRUE;
        rasterizer_desc.DepthClipEnable       = TRUE;
        rasterizer_desc.ScissorEnable         = FALSE;
        device->CreateRasterizerState(&rasterizer_desc, &default_rasterizer_state);
        if (!default_rasterizer_state) {
            logprintf("Failed to create default rasterizer state!\n");
            return false;
        }
    }
    
    //
    // Create depth stencil states
    //
    {
        D3D11_DEPTH_STENCIL_DESC depth_stencil_desc = {};
        depth_stencil_desc.DepthEnable    = FALSE;
        depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        depth_stencil_desc.DepthFunc      = D3D11_COMPARISON_LESS_EQUAL;
        device->CreateDepthStencilState(&depth_stencil_desc, &depth_disabled_dss);
        if (!depth_disabled_dss) {
            logprintf("Failed to create depth disabled dss!\n");
            return false;
        }
    }

    //
    // Create sampler states
    //
    {
        D3D11_SAMPLER_DESC sampler_desc = {};
        sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        sampler_desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MinLOD         = -FLT_MAX;
        sampler_desc.MaxLOD         = +FLT_MAX;
        sampler_desc.MipLODBias     = 0.0f;
        sampler_desc.MaxAnisotropy  = 1;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.BorderColor[0] = 1;
        sampler_desc.BorderColor[1] = 1;
        sampler_desc.BorderColor[2] = 1;
        sampler_desc.BorderColor[3] = 1;
        device->CreateSamplerState(&sampler_desc, &sampler_point);

        sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        device->CreateSamplerState(&sampler_desc, &sampler_linear);

        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        device->CreateSamplerState(&sampler_desc, &sampler_linear_clamp);

        sampler_desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        device->CreateSamplerState(&sampler_desc, &sampler_point_clamp);
    }

    //
    // Create constant buffers
    //
    {
        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth      = sizeof(Per_Scene_Uniforms);
        buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
        buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (device->CreateBuffer(&buffer_desc, NULL, &per_scene_uniforms_cb) != S_OK) {
            logprintf("Failed to create d3d11 per scene uniforms constant buffer!\n");
            return false;
        }

        if (!per_scene_uniforms_cb) {
            logprintf("Failed to create d3d11 per scene uniforms constant buffer!\n");
            return false;
        }
    }
    
    //
    // Create fullscreen quad buffers
    //
    {
        Sprite_Vertex vertices[] = {
            { { -1.0f, -1.0f }, { 0.0f, 1.0f } },
            { { +1.0f, -1.0f }, { 1.0f, 1.0f } },
            { { +1.0f, +1.0f }, { 1.0f, 0.0f } },
            { { -1.0f, +1.0f }, { 0.0f, 0.0f } },
        };

        u32 indices[] = {
            0, 1, 2,
            0, 2, 3,
        };

        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srd = {};
        srd.pSysMem = vertices;

        if (device->CreateBuffer(&buffer_desc, &srd, &fullscreen_quad_vb) != S_OK) {
            logprintf("Failed to create d3d11 fullscreen quad vertex buffer!\n");
            return false;
        }

        if (!fullscreen_quad_vb) {
            logprintf("Failed to create d3d11 fullscreen quad vertex buffer!\n");
            return false;
        }

        buffer_desc.ByteWidth = sizeof(indices);
        buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        srd.pSysMem = indices;

        if (device->CreateBuffer(&buffer_desc, &srd, &fullscreen_quad_ib) != S_OK) {
            logprintf("Failed to create d3d11 fullscreen quad index buffer!\n");
            return false;
        }

        if (!fullscreen_quad_ib) {
            logprintf("Failed to create d3d11 fullscreen quad index buffer!\n");
            return false;
        }
    }

    //
    // Create sprite buffers
    //
    {
        Sprite_Vertex vertices[] = {
            { { -0.5f, -0.5f }, { 0.0f, 0.0f } },
            { { +0.5f, -0.5f }, { 1.0f, 0.0f } },
            { { +0.5f, +0.5f }, { 1.0f, 1.0f } },
            { { -0.5f, +0.5f }, { 0.0f, 1.0f } },
        };

        u32 indices[] = {
            0, 1, 2,
            0, 2, 3,
        };

        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth = sizeof(vertices);
        buffer_desc.Usage     = D3D11_USAGE_IMMUTABLE;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA srd = {};
        srd.pSysMem = vertices;

        if (device->CreateBuffer(&buffer_desc, &srd, &sprite_quad_vb) != S_OK) {
            logprintf("Failed to create d3d11 sprite quad vertex buffer!\n");
            return false;
        }

        if (!sprite_quad_vb) {
            logprintf("Failed to create d3d11 sprite quad vertex buffer!\n");
            return false;
        }

        buffer_desc.ByteWidth = sizeof(indices);
        buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        srd.pSysMem = indices;

        if (device->CreateBuffer(&buffer_desc, &srd, &sprite_quad_ib) != S_OK) {
            logprintf("Failed to create d3d11 sprite quad index buffer!\n");
            return false;
        }

        if (!sprite_quad_ib) {
            logprintf("Failed to create d3d11 sprite quad index buffer!\n");
            return false;
        }

        buffer_desc.ByteWidth      = sizeof(Sprite_Render_Info) * MAX_SPRITES;
        buffer_desc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
        buffer_desc.Usage          = D3D11_USAGE_DYNAMIC;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (device->CreateBuffer(&buffer_desc, NULL, &sprite_instance_buffer) != S_OK) {
            logprintf("Failed to create d3d11 sprite instance buffer!\n");
            return false;
        }

        if (!sprite_instance_buffer) {
            logprintf("Failed to create d3d11 sprite instance buffer!\n");
            return false;
        }

        sprite_instances = new Sprite_Render_Info[MAX_SPRITES];
        sprite_regions   = new Sprite_Region[MAX_SPRITES];
        num_sprite_instances = 0;
    }

    if (!init_shaders()) return false;
    
    renderer_resize();
    
    return true;
}

bool renderer_is_hdr_used() {
    return do_hdr;
}

void renderer_begin_frame(Vector4 _clear_color, Per_Scene_Uniforms const &uniforms) {
    num_sprite_instances = 0;
    
    clear_color        = _clear_color;
    per_scene_uniforms = uniforms;
}

static int sprite_compare(const void *_a, const void *_b) {
    const Sprite_Render_Info *a = (const Sprite_Render_Info *)_a;
    const Sprite_Render_Info *b = (const Sprite_Render_Info *)_b;

    if (a->layer != b->layer) return a->layer - b->layer;
    if (a->texture != b->texture) return (int)((intptr_t)a->texture - (intptr_t)b->texture);

    return 0;
}

void renderer_end_frame() {
    device_context->ClearRenderTargetView(offscreen_buffer_rtv, &clear_color.x);
    device_context->OMSetRenderTargets(1, &offscreen_buffer_rtv, NULL);

    device_context->RSSetState(default_rasterizer_state);
    device_context->OMSetDepthStencilState(depth_disabled_dss, 0);
    device_context->OMSetBlendState(blend_enabled_bs, NULL, 0xFFFFFFFF);

    ID3D11SamplerState *samplers[] = { sampler_point, sampler_linear, sampler_point_clamp, sampler_linear_clamp };
    device_context->PSSetSamplers(0, ArrayCount(samplers), samplers);

    device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetInputLayout(sprite_input_layout);

    D3D11_VIEWPORT viewport = {};
    viewport.Width    = (float)render_width;
    viewport.Height   = (float)render_height;
    viewport.MaxDepth = 1.0f;
    device_context->RSSetViewports(1, &viewport);

    device_context->VSSetShader(sprite_vertex_shader, NULL, 0);
    device_context->PSSetShader(sprite_pixel_shader, NULL, 0);

    qsort(sprite_instances, num_sprite_instances, sizeof(Sprite_Render_Info), sprite_compare);
    
    D3D11_MAPPED_SUBRESOURCE msr;
    device_context->Map(sprite_instance_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, sprite_instances, sizeof(Sprite_Render_Info) * num_sprite_instances);
    device_context->Unmap(sprite_instance_buffer, 0);

    device_context->Map(per_scene_uniforms_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    memcpy(msr.pData, &per_scene_uniforms, sizeof(per_scene_uniforms));
    device_context->Unmap(per_scene_uniforms_cb, 0);

    ID3D11Buffer *cbos[] = { per_scene_uniforms_cb };
    device_context->VSSetConstantBuffers(0, ArrayCount(cbos), cbos);
    device_context->PSSetConstantBuffers(0, ArrayCount(cbos), cbos);
    
    UINT strides[] = {
        sizeof(Sprite_Vertex),
        sizeof(Sprite_Render_Info),
    };

    UINT offsets[] = {
        0, 0
    };

    ID3D11Buffer *buffers[] = {
        sprite_quad_vb,
        sprite_instance_buffer,
    };

    device_context->IASetVertexBuffers(0, ArrayCount(buffers), buffers, strides, offsets);
    device_context->IASetIndexBuffer(sprite_quad_ib, DXGI_FORMAT_R32_UINT, 0);

    int num_sprite_regions = 0;
    Sprite_Region *current_sprite_region = NULL;
    for (int i = 0; i < num_sprite_instances; i++) {
        Sprite_Render_Info *info = &sprite_instances[i];
        if (current_sprite_region) {
            if (current_sprite_region->texture != info->texture) {
                current_sprite_region = NULL;
            }
        }

        if (!current_sprite_region) {
            current_sprite_region = &sprite_regions[num_sprite_regions++];
            current_sprite_region->texture = info->texture;
            current_sprite_region->instance_offset = i;
            current_sprite_region->num_instances   = 0;
        }

        current_sprite_region->num_instances++;
    }

    for (int i = 0; i < num_sprite_regions; i++) {
        Sprite_Region *region = &sprite_regions[i];
        
        device_context->PSSetShaderResources(0, 1, &region->texture->srv);
        device_context->DrawIndexedInstanced(6, region->num_instances, 0, 0, region->instance_offset);
    }
    
    // Resolve
    float back_buffer_clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    device_context->ClearRenderTargetView(back_buffer_rtv, back_buffer_clear_color);
    device_context->OMSetRenderTargets(1, &back_buffer_rtv, NULL);

    viewport.TopLeftX = (float)(os_window_width  - render_width) * 0.5f;
    viewport.TopLeftY = (float)(os_window_height - render_height) * 0.5f;
    viewport.Width    = (float)render_width;
    viewport.Height   = (float)render_height;
    device_context->RSSetViewports(1, &viewport);

    device_context->VSSetShader(resolve_vertex_shader, NULL, 0);
    device_context->PSSetShader(resolve_pixel_shader, NULL, 0);
    device_context->IASetInputLayout(fullscreen_quad_input_layout);

    device_context->PSSetShaderResources(0, 1, &offscreen_buffer_srv);
    
    UINT stride = sizeof(Sprite_Vertex);
    UINT offset = 0;
    device_context->IASetVertexBuffers(0, 1, &fullscreen_quad_vb, &stride, &offset);
    device_context->IASetIndexBuffer(fullscreen_quad_ib, DXGI_FORMAT_R32_UINT, 0);

    device_context->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView *null_srv[1] = { NULL };
    device_context->PSSetShaderResources(0, 1, null_srv);
}

void renderer_present() {
    if (should_vsync) {
        swap_chain->Present(1, 0);
    } else {
        swap_chain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    }
}

void renderer_resize() {
    if (!swap_chain) return;

    Rectangle2i render_area = aspect_ratio_fit(os_window_width, os_window_height, RENDER_ASPECT_RATIO_X, RENDER_ASPECT_RATIO_Y);

    render_width  = render_area.width;
    render_height = render_area.height;
    
    release_back_buffer();
    swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, swap_chain_flags);
    init_back_buffer();
}

static char *read_entire_binary_file(const char *filepath, s64 *length_pointer) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        logprintf("Failed to read file '%s'\n", filepath);
        return NULL;
    }
    defer { fclose(file); };

    fseek(file, 0, SEEK_END);
    auto length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *result = new char[length];
    auto num_read = fread(result, 1, length, file);
    if (length_pointer) *length_pointer = num_read;

    return result;
}

static bool load_shader(ID3D11VertexShader **vertex_shader, ID3D11PixelShader **pixel_shader, char *filename, Render_Vertex_Type render_vertex_type) {
    char vertex_full_path[256];
    snprintf(vertex_full_path, sizeof(vertex_full_path), "%s/%s.vertex.fxc", SHADER_DIRECTORY, filename);

    s64 vertex_data_size;
    char *vertex_data = read_entire_binary_file(vertex_full_path, &vertex_data_size);
    if (!vertex_data) return false;
    defer { delete [] vertex_data; };

    if (device->CreateVertexShader(vertex_data, vertex_data_size, NULL, vertex_shader) != S_OK) {
        logprintf("Failed to create '%s' vertex shader\n", vertex_full_path);
        return false;
    }
    
    char pixel_full_path[256];
    snprintf(pixel_full_path, sizeof(pixel_full_path), "%s/%s.pixel.fxc", SHADER_DIRECTORY, filename);

    s64 pixel_data_size;
    char *pixel_data = read_entire_binary_file(pixel_full_path, &pixel_data_size);
    if (!pixel_data) return false;
    defer { delete [] pixel_data; };
    
    if (device->CreatePixelShader(pixel_data, pixel_data_size, NULL, pixel_shader) != S_OK) {
        logprintf("Failed to create '%s' pixel shader\n", pixel_full_path);
        return false;
    }

    switch (render_vertex_type) {
        case RENDER_VERTEX_SPRITE: {
            D3D11_INPUT_ELEMENT_DESC ieds[7] = {};
                
            ieds[0].SemanticName      = "POSITION";
            ieds[0].Format            = DXGI_FORMAT_R32G32_FLOAT;
            ieds[0].AlignedByteOffset = offsetof(Sprite_Vertex, position);
            ieds[0].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

            ieds[1].SemanticName      = "TEXCOORD";
            ieds[1].Format            = DXGI_FORMAT_R32G32_FLOAT;
            ieds[1].AlignedByteOffset = offsetof(Sprite_Vertex, uv);
            ieds[1].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

            ieds[2].SemanticName         = "INSTANCE_POSITION";
            ieds[2].Format               = DXGI_FORMAT_R32G32_FLOAT;
            ieds[2].InputSlot            = 1;
            ieds[2].AlignedByteOffset    = offsetof(Sprite_Render_Info, position);
            ieds[2].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
            ieds[2].InstanceDataStepRate = 1;

            ieds[3].SemanticName         = "INSTANCE_SIZE";
            ieds[3].Format               = DXGI_FORMAT_R32G32_FLOAT;
            ieds[3].InputSlot            = 1;
            ieds[3].AlignedByteOffset    = offsetof(Sprite_Render_Info, size);
            ieds[3].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
            ieds[3].InstanceDataStepRate = 1;

            ieds[4].SemanticName         = "INSTANCE_COLOR";
            ieds[4].Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
            ieds[4].InputSlot            = 1;
            ieds[4].AlignedByteOffset    = offsetof(Sprite_Render_Info, color);
            ieds[4].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
            ieds[4].InstanceDataStepRate = 1;

            ieds[5].SemanticName         = "INSTANCE_MIN_UV";
            ieds[5].Format               = DXGI_FORMAT_R32G32_FLOAT;
            ieds[5].InputSlot            = 1;
            ieds[5].AlignedByteOffset    = offsetof(Sprite_Render_Info, min_uv);
            ieds[5].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
            ieds[5].InstanceDataStepRate = 1;

            ieds[6].SemanticName         = "INSTANCE_MAX_UV";
            ieds[6].Format               = DXGI_FORMAT_R32G32_FLOAT;
            ieds[6].InputSlot            = 1;
            ieds[6].AlignedByteOffset    = offsetof(Sprite_Render_Info, max_uv);
            ieds[6].InputSlotClass       = D3D11_INPUT_PER_INSTANCE_DATA;
            ieds[6].InstanceDataStepRate = 1;

            if (!sprite_input_layout) {
                if (device->CreateInputLayout(ieds, ArrayCount(ieds), vertex_data, vertex_data_size, &sprite_input_layout) != S_OK) {
                    logprintf("Failed to create sprite vertex input layout with shader '%s'\n", vertex_full_path);
                return false;
                }
            }
        } break;

        case RENDER_VERTEX_FULLSCREEN_QUAD: {
            D3D11_INPUT_ELEMENT_DESC ieds[2] = {};
                
            ieds[0].SemanticName      = "POSITION";
            ieds[0].Format            = DXGI_FORMAT_R32G32_FLOAT;
            ieds[0].AlignedByteOffset = offsetof(Sprite_Vertex, position);
            ieds[0].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

            ieds[1].SemanticName      = "TEXCOORD";
            ieds[1].Format            = DXGI_FORMAT_R32G32_FLOAT;
            ieds[1].AlignedByteOffset = offsetof(Sprite_Vertex, uv);
            ieds[1].InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;

            if (!fullscreen_quad_input_layout) {
                if (device->CreateInputLayout(ieds, ArrayCount(ieds), vertex_data, vertex_data_size, &fullscreen_quad_input_layout) != S_OK) {
                    logprintf("Failed to create fullscreen quad vertex input layout with shader '%s'\n", vertex_full_path);
                return false;
                }
            }
        } break;
    }
    
    return true;
}

bool renderer_create_texture(Texture *texture, int width, int height, void *initial_data) {
    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width              = width;
    texture_desc.Height             = height;
    texture_desc.MipLevels          = 1;
    texture_desc.ArraySize          = 1;
    texture_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    texture_desc.SampleDesc.Count   = 1;
    texture_desc.SampleDesc.Quality = 0;
    texture_desc.Usage              = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA srd = {};
    srd.pSysMem     = initial_data;
    srd.SysMemPitch = width * 4 * sizeof(u8);

    if (device->CreateTexture2D(&texture_desc, initial_data ? &srd : NULL, &texture->texture) != S_OK) {
        logprintf("Failed to create d3d11 texture\n");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format                    = texture_desc.Format;
    srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels       = 1;

    if (device->CreateShaderResourceView(texture->texture, &srv_desc, &texture->srv) != S_OK) {
        logprintf("Failed to create d3d11 shader resource view\n");
        return false;
    }

    texture->width           = width;
    texture->height          = height;
    
    return true;
}

bool renderer_load_texture(Texture *texture, char *filepath) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data = stbi_load(filepath, &width, &height, &channels, 4);
    if (!data) {
        logprintf("Failed to load image '%s'\n", filepath);
        return false;
    }
    defer { stbi_image_free(data); };

    if (!renderer_create_texture(texture, width, height, data)) {
        return false;
    }

    return true;
}

void renderer_add_sprite(Sprite_Render_Info const &info) {
    Assert(num_sprite_instances < MAX_SPRITES);
    sprite_instances[num_sprite_instances++] = info;
}

void renderer_add_sprite(Texture *texture, Vector2 position, Vector2 size, Vector4 color) {
    Sprite_Render_Info info;

    info.texture  = texture;
    info.position = position + size * 0.5f;
    info.size     = size;
    info.color    = color;
    info.min_uv   = v2(0, 0);
    info.max_uv   = v2(1, 1);
    info.layer    = 0;

    renderer_add_sprite(info);
}

void renderer_add_sprite(Texture *texture, Vector2 position, Vector2 size, Rectangle2i *src_rect, Flip_Mode flip_mode, Vector4 color, int layer) {
    Sprite_Render_Info info;

    Vector2 min_uv = v2(0, 0), max_uv = v2(1, 1);
    if (src_rect) {
        min_uv.x = (float)src_rect->x / (float)texture->width;
        min_uv.y = (float)src_rect->y / (float)texture->height;

        max_uv.x = (float)(src_rect->x + src_rect->width)  / (float)texture->width;
        max_uv.y = (float)(src_rect->y + src_rect->height) / (float)texture->height;
    }

    if (flip_mode & FLIP_MODE_HORIZONTALLY) {
        float min_uv_x = min_uv.x;
        min_uv.x = max_uv.x;
        max_uv.x = min_uv_x;
    }

    if (flip_mode & FLIP_MODE_VERTICALLY) {
        float min_uv_y = min_uv.y;
        min_uv.y = max_uv.y;
        max_uv.y = min_uv_y;
    }
    
    info.texture  = texture;
    info.position = position + size * 0.5f;
    info.size     = size;
    info.color    = color;
    info.min_uv   = min_uv;
    info.max_uv   = max_uv;
    info.layer    = layer;

    renderer_add_sprite(info);    
}

void renderer_add_sprite_center(Texture *texture, Vector2 center, Vector2 size, Vector4 color) {
    Sprite_Render_Info info;

    info.texture  = texture;
    info.position = center;
    info.size     = size;
    info.color    = color;
    info.min_uv   = v2(0, 0);
    info.max_uv   = v2(1, 1);
    info.layer    = 0;

    renderer_add_sprite(info);
}

void renderer_add_text(Font *font, char *text, int x, int y, Vector4 color) {
    for (char *at = text; *at; at++) {
        Glyph *glyph = &font->glyphs[*at];
        if (!glyph) continue;

        if (*at == '\n') {
            logprintf("Reached new line in render_text: Stopping drawing!\n");
            break;
        }

        if (!is_space(*at)) {
            Vector2 pos;
            pos.x = (float)(x + glyph->bearing_x);
            pos.y = (float)(y - (glyph->size_y - glyph->bearing_y));

            Vector2 size;
            size.x = (float)glyph->size_x;
            size.y = (float)glyph->size_y;
            
            renderer_add_sprite(font->texture, pos, size, &glyph->src_rect, FLIP_MODE_VERTICALLY, color, 1);
        }

        x += glyph->advance;
    }
}

#endif
