#version 330 core

layout (location = 0) in vec4 u_coords; // (pos_x, pos_y, uv_x, uv_y)

out vec2 v_uv;

uniform mat4 u_ui_matrix;

void main()
{
    gl_Position = u_ui_matrix * vec4(u_coords.xy, 0.0, 1.0);
    v_uv = u_coords.zw;
}
