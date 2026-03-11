#version 330 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec2 in_uv;

uniform mat4 u_vp_matrix;
uniform mat4 u_model_matrix;

out vec2 v_uv;
out vec3 v_ws_position;

void main()
{
    gl_Position = u_vp_matrix * u_model_matrix * vec4(in_position, 1.0);
    v_uv = in_uv;
    v_ws_position = vec3(u_model_matrix * vec4(in_position, 1.0));
}
