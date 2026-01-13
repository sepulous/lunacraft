#version 330 core

layout (location = 0) in vec4 in_coords; // (pos_x, pos_y, tex_x, tex_y)
layout (location = 1) in vec4 in_color;

out vec4 v_color;
out vec2 v_uv;

uniform mat4 u_ui_matrix;

void main()
{
    gl_Position = u_ui_matrix * vec4(in_coords.xy, 0.0, 1.0);
    v_color = in_color;
    v_uv = in_coords.zw;
}
