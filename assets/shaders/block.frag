#version 330 core

out vec4 out_color;

in vec2 v_tex;
in vec2 v_tile_origin;
in vec3 v_normal;
in vec3 v_ws_position;
in vec2 v_light;

uniform sampler2D u_block_texture;
uniform vec3 u_ws_camera_position;
uniform vec4 u_fog_color;
uniform float u_fog_distance;

void main()
{
    vec2 local_uv = fract(v_tex);
    vec2 atlas_uv = v_tile_origin + local_uv / 14.0; // atlas is 14x14
    vec4 tex_color = texture(u_block_texture, atlas_uv);
    vec2 light = clamp(v_light, 0.0, 1.0);
    tex_color = vec4(light.x * tex_color.r,
                     light.x * tex_color.g,
                     light.y * tex_color.b,
                     tex_color.a);

    float view_distance = length(u_ws_camera_position - v_ws_position) - u_fog_distance;
    float fog_factor = clamp(exp(-0.1 * view_distance), 0, 1);

    //
    // This is just: mix(tex_color, mix(u_fog_color, tex_color, fog_factor), u_fog_color.a)
    //
    // But that has 4 adds/subs and 4 mults, while this only has 3 adds/subs and 2 mults
    //
    out_color = tex_color + u_fog_color.a * (u_fog_color - tex_color) * (1.0 - fog_factor);
}
