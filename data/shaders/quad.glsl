OUT_IN vec4 v_color;
OUT_IN vec2 v_uv;

#ifdef VERTEX_SHADER

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

uniform mat4 view_proj;

void main(void) {
    gl_Position = view_proj * vec4(a_position, 0.0, 1.0);
    v_color     = a_color;
    v_uv        = a_uv;
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 o_color;

uniform sampler2D u_texture;

void main(void) {
    vec4 tex_color = texture(u_texture, v_uv);
    o_color = v_color * tex_color;
}

#endif
