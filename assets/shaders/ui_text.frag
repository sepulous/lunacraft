#version 330 core

out vec4 out_color;

in vec4 v_color;
in vec2 v_uv;

uniform sampler2D u_font_atlas;

void main()
{
    out_color = vec4(texture(u_font_atlas, v_uv).r) * v_color;
}
