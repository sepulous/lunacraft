#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D tex;
uniform float opacity;
uniform float darkness;

void main()
{
    vec4 tex_color = texture(tex, uv);
    FragColor = vec4(tex_color.rgb * (1 - darkness), tex_color.a * opacity);
}
