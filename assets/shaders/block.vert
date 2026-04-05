#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_uv;
layout (location = 2) in vec3 in_normal;
layout (location = 3) in vec2 in_light;

out vec2 v_tex;
out vec2 v_tile_origin;
out vec2 v_light;
out float v_view_dist;

uniform mat4 u_view;
uniform mat4 u_proj;

void main()
{
    vec4 view_pos = u_view * vec4(in_position, 1.0);
    gl_Position = u_proj * view_pos;
    v_view_dist = -view_pos.z;
    v_tex = in_uv.xy;
    v_tile_origin = in_uv.zw;
    v_light = in_light;
}
