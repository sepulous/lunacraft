#version 330 core

in vec2 v_uv;
in float v_view_dist;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform vec4 u_fog_color;
uniform float u_fog_start;
uniform float u_fog_end;
uniform float u_lighting;

out vec4 out_color;

void main()
{
    float fog_factor = (v_view_dist - u_fog_start) / (u_fog_end - u_fog_start);
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    vec4 tex_color = texture(u_texture, v_uv);
    tex_color = vec4(max(u_lighting, 0.8) * tex_color.rgb, tex_color.a);
    vec4 tex_with_fog = mix(tex_color, u_fog_color, fog_factor * u_fog_color.a);

    out_color = u_color * tex_with_fog;
}
