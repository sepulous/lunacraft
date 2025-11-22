#version 330 core

in vec4 color;
in vec2 uvCoords;

uniform sampler2D uFontAtlasTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(texture(uFontAtlasTexture, uvCoords).r) * color;
}
