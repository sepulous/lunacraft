#version 330 core

layout (location = 0) in vec3 in_position;

out vec3 v_tex;

uniform mat4 u_view_projection;

void main()
{
    v_tex = in_position;
    vec4 pos = u_view_projection * vec4(in_position, 1.0);
    gl_Position = pos.xyww; // Forces z=1 (normalized devices coordinates)
}
