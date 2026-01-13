#version 330 core

out vec4 out_color;

in vec2 v_uv;

uniform sampler2D u_tex;
uniform float u_opacity;
uniform float u_darkness;

void main()
{
    vec4 tex_color = texture(u_tex, v_uv);
    out_color = vec4(tex_color.rgb * (1 - u_darkness), tex_color.a * u_opacity);
}
