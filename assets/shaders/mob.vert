#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 v_uv;
out float v_view_dist;

void main()
{
    vec4 view_pos = u_view * u_model * vec4(in_position, 1.0);
    gl_Position = u_proj * view_pos;
    v_uv = in_uv;
    v_view_dist = -view_pos.z;
}
