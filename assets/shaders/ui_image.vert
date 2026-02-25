#version 330 core

layout (location = 0) in vec4 in_coords; // (pos_x, pos_y, uv_x, uv_y)

uniform mat4 u_ui_matrix;
uniform mat4 u_model_matrix;

out vec2 v_uv;

void main()
{
    gl_Position = u_ui_matrix * u_model_matrix * vec4(in_coords.xy, 0.0, 1.0);
    v_uv = in_coords.zw;
}
