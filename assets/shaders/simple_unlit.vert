#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 v_uv;

void main()
{
    gl_Position = u_proj * u_view * u_model * vec4(in_position, 1.0);
    v_uv = in_uv;
}
