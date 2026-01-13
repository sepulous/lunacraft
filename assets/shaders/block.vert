#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec4 in_uv;
layout (location = 2) in vec3 in_normal;

out vec2 v_tex;
out vec2 v_tile_origin;
out vec3 v_normal;
out vec3 v_ws_position;

uniform mat4 u_view_projection;

void main()
{
    v_ws_position = in_position;
    gl_Position = u_view_projection * vec4(in_position, 1.0);
    v_tex = in_uv.xy;
    v_tile_origin = in_uv.zw;
    v_normal = in_normal;
}
