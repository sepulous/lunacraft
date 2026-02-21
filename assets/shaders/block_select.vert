#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

out vec2 v_uv;

uniform mat4 u_view_projection;
uniform vec3 u_position;

void main()
{
    gl_Position = u_view_projection * vec4(in_position + u_position, 1.0);
    v_uv = in_uv;
}
