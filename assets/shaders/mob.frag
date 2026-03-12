#version 330 core

in vec2 v_uv;
in vec3 v_ws_position;

uniform sampler2D u_texture;
uniform vec3 u_ws_camera_position;
uniform vec4 u_color;
uniform vec4 u_fog_color;
uniform float u_fog_distance;

out vec4 out_color;

void main()
{
    vec4 tex_color = texture(u_texture, v_uv);
    float view_distance = length(u_ws_camera_position - v_ws_position) - u_fog_distance;
    float fog_factor = clamp(exp(-0.1 * view_distance), 0, 1);
    vec4 tex_with_fog = tex_color + u_fog_color.a * (u_fog_color - tex_color) * (1.0 - fog_factor);

    out_color = vec4(mix(tex_with_fog.rgb, u_color.rgb, u_color.a), 1.0);
}
