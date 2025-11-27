#version 330 core

layout (location = 0) in vec4 aCoords; // (pos_x, pos_y, tex_x, tex_y)
layout (location = 1) in vec4 aColor;

out vec4 color;
out vec2 uvCoords;

uniform mat4 ui_matrix;

void main()
{
    gl_Position = ui_matrix * vec4(aCoords.xy, 0.0, 1.0);
    color = aColor;
    uvCoords = aCoords.zw;
}
