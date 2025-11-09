#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D tex;
uniform float opacity;

void main()
{
    vec4 tex_color = texture(tex, uv);
    FragColor = vec4(tex_color.rgb, tex_color.a * opacity);
}
