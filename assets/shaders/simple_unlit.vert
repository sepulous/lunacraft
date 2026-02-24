#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

uniform mat4 u_mvp_matrix;

out vec2 v_uv;

void main()
{
    gl_Position = u_mvp_matrix * vec4(in_position, 1.0);
    v_uv = in_uv;
}
