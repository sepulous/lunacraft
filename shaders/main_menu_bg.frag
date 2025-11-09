#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D tex;
uniform float opacity;

void main()
{
    FragColor = vec4(texture(tex, uv).rgb, opacity);
}
