#version 330 core

out vec4 out_color;

in vec2 v_tex;
in vec2 v_tile_origin;
in vec2 v_light;
in float v_view_dist;

uniform sampler2D u_block_texture;
uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;

void main()
{
    // Base color
    vec2 local_uv = fract(v_tex);
    vec2 atlas_uv = v_tile_origin + local_uv / 14.0; // atlas is 14x14
    vec4 tex_color = texture(u_block_texture, atlas_uv);
    vec2 light = clamp(v_light, 0.0, 1.0);
    tex_color = vec4(light.x * tex_color.r,
                     light.x * tex_color.g,
                     light.y * tex_color.b,
                     tex_color.a);

    // Fog
    float fog_factor = (v_view_dist - u_fog_start) / (u_fog_end - u_fog_start);
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    // Final
    out_color = mix(tex_color, u_fog_color, fog_factor * u_fog_color.a);
}
