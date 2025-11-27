#version 330 core

layout (location = 0) in vec4 aCoords; // (pos_x, pos_y, uv_x, uv_y)

out vec2 uv;

uniform mat4 ui_matrix;

void main()
{
    gl_Position = ui_matrix * vec4(aCoords.xy, 0.0, 1.0);
    uv = aCoords.zw;
}
